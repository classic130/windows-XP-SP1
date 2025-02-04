// Copyright (c) 1996-1999 Microsoft Corporation

// --------------------------------------------------------------------------
//
//  MENU.CPP
//
//  Menu class, for system menus and app menus.
//
//	There are four classes here. 
//	CMenu is the class that knows how to deal with menu bar objects. These
//	have children that are CMenuItem objects, or just children (rare - 
//	this is when you have a command right on the menu bar).
//	CMenuItem is something that when you click on it opens a popup.
//	It has 1 child that is a CMenuPopupFrame.
//	CMenuPopupFrame is the HWND that pops up when you click on a CMenuItem. It
//	has 1 child, a CMenuPopup.
//  CMenuPopup objects represent the client area of a CMenuPopupFrame HWND.
//  It has children that are menu items (little m, little i), separators, and 
//	CMenuItems (when you have cascading menus).
//
//  Issues that came up during design/implementation:
//      (1) How do we select/focus menu items while in menu mode?
//      (2) How do we choose an item (default action)?
//          For menu bars, we use SendInput to send Alt+Shortcut key to 
//          open or execute an item or command. Send just Alt to close 
//          an item that is already open.
//      (3) How do we handle popup menus?  
//          As discussed above, we treat them very strangely. There are ways
//          to get the children in a popup whether it is visible or not.
//      (4) What about "system menu" popups on tray?
//          
//		(5) In general, what about "context menus"? This may need to be
//			exposed by the app itself. We can't do everything!
// 
//  History:
//	written by Laura Butler, early 1996
//	complete re-write by Steve Donie, August 1996-January 1997
//  doDefaultAction changed to use keypresses rather than mouse clicks 3-97
// --------------------------------------------------------------------------

#include "oleacc_p.h"
#include "default.h"
#include "classmap.h"
#include "ctors.h"
#include "window.h"
#include "client.h"
#include "menu.h"

#include "propmgr_util.h"

#define MI_NONE         -1

#ifndef MFS_HOTTRACK
#define MFS_HOTTRACK        0x00000100L
#endif // !MFS_HOTTRACK





// --------------------------------------------------------------------------
// prototypes for local functions
// --------------------------------------------------------------------------
HWND GetSubMenuWindow (HMENU hSubMenuToFind);
long FindItemIDThatOwnsThisMenu (HMENU hMenuOwned,HWND* phwndOwner,
                                 BOOL* pfPopup,BOOL* pfSysMenu);
STDAPI  WindowFromAccessibleObjectEx(IAccessible* pacc, HWND* phwnd);

BOOL    MyGetMenuString( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, long id, LPTSTR lpszBuf, UINT cchMax, BOOL fAllowGenerated );

BOOL    TryMSAAMenuHack( IAccessible * pTheObj, HWND hWnd, DWORD_PTR dwItemData, LPTSTR lpszBuf, UINT cchMax );
BOOL    GetShellOwnerDrawMenu( HWND hwnd, DWORD_PTR dwItemData, LPTSTR lpszBuf, UINT cchMax );

UINT    GetMDIButtonIndex( HMENU hMenu, DWORD idPos );
BOOL    GetMDIMenuDescriptionString( HMENU hMenu, DWORD idPos, BSTR * pbstr );
BOOL    GetMDIMenuString( HWND hwnd, HMENU hMenu, DWORD idPos, LPTSTR lpszBuf, UINT cchMax );

HMENU   MyGetSystemMenu( HWND hwnd );

HRESULT GetMenuItemName( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id, BSTR * pszName );
TCHAR   GetMenuItemHotkey( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id, DWORD fOptions );
HRESULT GetMenuItemShortcut( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id,
                             BOOL fIsMenuBar, BSTR * pszShortcut );

enum
{
    GMIH_ALLOW_INITIAL     = 0x01,
    GMIH_ALLOW_SYS_SPACE   = 0x02
};



// --------------------------------------------------------------------------
//
//  CreateSysMenuBarObject()
//
//  EXTERNAL for CreateStdAcessibleObject
//
//	Parameters: 
//	hwnd		IN		window handle of the window that owns this menu
//	idChildCur	IN		id of the current child. Will be 0 when creating the 
//						system menu bar and application menu bar. Will be the 
//						id of a child when calling CMenu::Clone()
//	riid		IN		the id of the interface asked for
//	ppvMenu		OUT		ppvMenu holds the indirect pointer to the menu 
//						object. 
//
//	Return Value:
//		S_OK if the interface is supported, E_NOINTERFACE if not, 
//		E_OUTOFMEMORY if not enough memory to create the menu object,
//		E_FAIL if the hwnd is invalid.
//
// --------------------------------------------------------------------------
HRESULT CreateSysMenuBarObject(HWND hwnd, long idObject, REFIID riid,
    void** ppvMenu)
{
    UNUSED(idObject);

    if (!IsWindow(hwnd))
        return(E_FAIL);

    return(CreateMenuBar(hwnd, TRUE, 0, riid, ppvMenu));
}

// --------------------------------------------------------------------------
//
//  CreateMenuBarObject()
//
//  EXTERNAL for CreateStdAcessibleObject
//
//	Parameters: 
//	hwnd		IN		window handle of the window that owns this menu
//	idChildCur	IN		id of the current child. Will be 0 when creating the 
//						system menu bar and application menu bar. Will be the 
//						id of a child when calling CMenu::Clone()
//	riid		IN		the id of the interface asked for (like IAccessible,
//						IEnumVARIANT,IDispatch...)
//	ppvMenu		OUT		ppvMenu is where QueryInterface will return the 
//						indirect pointer to the menu object (caller casts
//						this to be a pointer to the interface they asked for)
//
//	Return Value:
//		S_OK if the interface is supported, E_NOINTERFACE if not, 
//		E_OUTOFMEMORY if not enough memory to create the menu object,
//		E_FAIL if the hwnd is invalid.
//
// --------------------------------------------------------------------------
HRESULT CreateMenuBarObject(HWND hwnd, long idObject, REFIID riid, void** ppvMenu)
{
    UNUSED(idObject);

    if (!IsWindow(hwnd))
        return(E_FAIL);

    return(CreateMenuBar(hwnd, FALSE, 0, riid, ppvMenu));
}



// --------------------------------------------------------------------------
//
//  CreateMenuBar()
//
//	Parameters: 
//	hwnd		IN		window handle of the window that owns this menu
//	fSysMenu	IN		true if this is a system menu, false if app menu
//	idChildCur	IN		id of the current child. Will be 0 when creating the 
//						system menu bar and application menu bar. Will be the 
//						id of a child when calling CMenu::Clone()
//	riid		IN		the id of the interface asked for
//	ppvMenu		OUT		ppvMenu is where QueryInterface will return the 
//						indirect pointer to the menu object
//
//	Return Value:
//		S_OK if the interface is supported, E_NOINTERFACE if not, 
//		E_OUTOFMEMORY if not enough memory to create the menu object.
//
//	Called By:
//	CreateMenuBarObject and CMenu::Clone
// --------------------------------------------------------------------------
HRESULT CreateMenuBar(HWND hwnd, BOOL fSysMenu, long idChildCur,
    REFIID riid, void** ppvMenu)
{
HRESULT     hr;
CMenu*      pmenu;

    InitPv(ppvMenu);

    pmenu = new CMenu(hwnd, fSysMenu, idChildCur);
    if (!pmenu)
        return(E_OUTOFMEMORY);

    hr = pmenu->QueryInterface(riid, ppvMenu);
    if (!SUCCEEDED(hr))
        delete pmenu;

    return(hr);
}

// --------------------------------------------------------------------------
//
//  CMenu::CMenu()
//
//	Constructor for the CMenu class. Initializes the member variables with
//	the passed in parameters. It is only called by CreateMenuBar.
//
// --------------------------------------------------------------------------
CMenu::CMenu(HWND hwnd, BOOL fSysMenu, long idChildCur)
    : CAccessible( CLASS_MenuObject )
{
    m_hwnd = hwnd;
    m_fSysMenu = fSysMenu;
    m_idChildCur = idChildCur;
	m_hMenu = NULL;
	// m_hMenu is filled in by SetupChildren()
	// m_cChildren is filled in by SetupChildren()
}

// --------------------------------------------------------------------------
//
//  CMenu::SetupChildren()
//
//	This uses the object's window handle to get the handle to the appropriate
//	menu (hmenu type menu handle) and the count of the children in that menu.
//	It uses GetMenuBarInfo, a private function in USER, to do this.
//	These values are kept as member variables of the CMenu object.
//
// --------------------------------------------------------------------------
void CMenu::SetupChildren(void)
{
MENUBARINFO mbi;

    if (!MyGetMenuBarInfo(m_hwnd, (m_fSysMenu ? OBJID_SYSMENU : OBJID_MENU), 
        0, &mbi))
    {
        m_hMenu = NULL;
    }
    else
    {
        m_hMenu = mbi.hMenu;
    }

    if (!m_hMenu)
        m_cChildren = 0;
    else
    {
        m_cChildren = GetMenuItemCount(m_hMenu);

        if( m_cChildren == -1 )
        {
            // Paranoia in case we get an invalid HMENU
            m_cChildren = 0;
        }
    }
}

// --------------------------------------------------------------------------
//
//  CMenu::get_accChild()
//
//	What we want this do do is return (in ppdisp) an IDispatch pointer to 
//  the child specified by varChild. The children of a CMenu are either
//	menu commands (rare to have a command on the menu bar) and CMenuItems.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::get_accChild(VARIANT varChild, IDispatch** ppdisp)
{
HMENU	hSubMenu;

    InitPv(ppdisp);

    if (!ValidateChild(&varChild))
        return (E_INVALIDARG);
    
    if (varChild.lVal == CHILDID_SELF)
        return(E_INVALIDARG);

	Assert (m_hMenu);

    hSubMenu = GetSubMenu(m_hMenu, varChild.lVal-1);

	if (hSubMenu == NULL)
	{
        // This returns false - for commands on the menu bar, we do not create a child
        // object - the parent is able to answer all the questions.
		return (S_FALSE);
	}

    return(CreateMenuItem((IAccessible*)this, m_hwnd, m_hMenu,hSubMenu,
        varChild.lVal,  0, TRUE, IID_IDispatch, (void**)ppdisp));
}

// --------------------------------------------------------------------------
//
//  CMenu::get_accName()
//
//	Pass in a VARIANT with type VT_I4 and lVal equal to the 1-based position
// of the item you want the name for. Pass in a pointer to a string and the
// string will be filled with the name.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

	if (varChild.lVal != CHILDID_SELF)
	{
        return GetMenuItemName( this, m_hwnd, m_hMenu, varChild.lVal, pszName );
	}

    if (m_fSysMenu)
        return HrCreateString(STR_SYSMENU_NAME, pszName);	// in English = "System"
    else
        return HrCreateString(STR_MENUBAR_NAME, pszName);	// in English, this is "Application"
}

// --------------------------------------------------------------------------
//
//  CMenu::get_accDescription()
//
// get a string with the description of this menu item. For CMenu, this
// is something like "contains commands to manipulate the window" or
// "Contains commands to manipulate the current view or document" 
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::get_accDescription(VARIANT varChild, BSTR* pszDesc)
{

    InitPv(pszDesc);

    if (! ValidateChild(&varChild))
        return E_INVALIDARG;

	// Check if they are asking about the menu bar itself or a child.
    // If asking about a child, return S_FALSE, because we don't have 
    // descriptions for items, just the system and app menu bars.
    //
	if (varChild.lVal != CHILDID_SELF)
    {
        if( GetMDIMenuDescriptionString( m_hMenu, varChild.lVal - 1, pszDesc ) )
        {
            return S_OK;
        }

		return S_FALSE;
    }
    else if (m_fSysMenu)
    {
        return HrCreateString(STR_SYSMENUBAR_DESCRIPTION, pszDesc);
    }
    else
    {
        return HrCreateString(STR_MENUBAR_DESCRIPTION, pszDesc);
    }
}




// --------------------------------------------------------------------------
//
//  CMenu::get_accRole()
//
// get the role - this is either menu item or menu bar
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

	if (varChild.lVal != CHILDID_SELF)
    {
        if( GetMDIButtonIndex( m_hMenu, varChild.lVal - 1 ) != 0 )
        {
            // Special case for MDI child buttons - they are actually implemented
            // as menu items, but appear as buttons.
    		pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
        }
        else
        {
    		pvarRole->lVal = ROLE_SYSTEM_MENUITEM;
        }
    }
	else
		pvarRole->lVal = ROLE_SYSTEM_MENUBAR;

    return(S_OK);
}




// --------------------------------------------------------------------------
//
//  CMenu::get_accState()
//
// get the state of the child specified. returned in a variant VT_I4
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    MENUBARINFO mbi;

    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (!m_hMenu || !MyGetMenuBarInfo(m_hwnd, (m_fSysMenu ? OBJID_SYSMENU : OBJID_MENU),
        varChild.lVal, &mbi))
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
    }
    else if( varChild.lVal && GetMDIButtonIndex( m_hMenu, varChild.lVal - 1 ) != 0 )
    {
        // For MDI button elements, just leave the state as normal, to be consistent
        // with the top-level restore/close/minimize buttons.

        // Do nothing here.
    }
    else
    {
        // Non-MDI-button menu items, or CHILDID_SELF...

        // smd 1-29-97 - change from OFFSCREEN to INVISIBLE
        if (IsRectEmpty(&mbi.rcBar))
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

        if (GetForegroundWindow() == m_hwnd)
            pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;

        if (mbi.fFocused)
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;
        
        if (varChild.lVal)
        {
            MENUITEMINFO    mi;

            //
            // Get menu item flags.  NOTE:  Can't use GetMenuState().  It whacks
            // random stuff in for hierarchicals.
            //
            mi.cbSize = SIZEOF_MENUITEMINFO;
            mi.fMask = MIIM_STATE | MIIM_SUBMENU;

            if (!GetMenuItemInfo(m_hMenu, varChild.lVal-1, TRUE, &mi))
            {
                pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
                return(S_FALSE);
            }

            if (mi.fState & MFS_GRAYED)
                pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;

            if (mi.fState & MFS_CHECKED)
                pvarState->lVal |= STATE_SYSTEM_CHECKED;

            if (mi.fState & MFS_DEFAULT)
                pvarState->lVal |= STATE_SYSTEM_DEFAULT;

            if (mbi.fFocused)
            {
                pvarState->lVal |= STATE_SYSTEM_HOTTRACKED;
			    if (mi.fState & MFS_HILITE)
                    pvarState->lVal |= STATE_SYSTEM_FOCUSED;
            }

            if (mi.hSubMenu)
                pvarState->lVal |= STATE_SYSTEM_HASPOPUP;
        
        }
    }

    return(S_OK);
}



// --------------------------------------------------------------------------
//
//  CMenu::get_accKeyboardShortcut()
//
// returns a string with the menu shortcut to the child asked for
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    if (varChild.lVal == CHILDID_SELF)
    {
        if (!m_hMenu)
            return S_FALSE;

        if (m_fSysMenu)
        {
            // Alt+Space/Hyphen is the system menu...
            TCHAR szFormat[16];
            LoadString(hinstResDll, STR_MENU_SHORTCUT_FORMAT, szFormat, ARRAYSIZE(szFormat));

            TCHAR szKey[16];
            LoadString(hinstResDll, ((GetWindowLong(m_hwnd, GWL_STYLE) & WS_CHILD) ?
                STR_CHILDSYSMENU_KEY : STR_SYSMENU_KEY), szKey, ARRAYSIZE(szKey));

            TCHAR szHotKey[32];
            szHotKey[ 0 ] = '\0';
            wsprintf(szHotKey, szFormat, szKey);

            if (*szHotKey)
            {
                *pszShortcut = TCharSysAllocString(szHotKey);
                if (! *pszShortcut)
                    return E_OUTOFMEMORY;

                return S_OK;
            }

            return S_FALSE;
        }
        else
        {
            // "Alt" is the menu bar
            return HrCreateString(STR_MENU_SHORTCUT, pszShortcut);
        }
    }
    else
    {
        // Get menu item shortcut - TRUE means use "Alt+" format.
        return GetMenuItemShortcut( this, m_hwnd, m_hMenu, varChild.lVal, TRUE, pszShortcut );
    }
}



// --------------------------------------------------------------------------
//
//  CMenu::get_accFocus()
//
//  This fills in pvarFocus with the ID of the child that has the focus.
//	So when say you just hit "Alt" (File is now highlighted) and then call 
//	get_accFocus(), pvarFocus will have VT_I4 and lVal = 1.
//
//	If we are not in menu mode, then we certainly don't have the focus. 
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::get_accFocus(VARIANT* pvarFocus)
{
GUITHREADINFO	GuiThreadInfo;
MENUITEMINFO	mii;
int				i;

	// set it to empty
    InitPvar(pvarFocus);

    //
    // Are we in menu mode?  If not, nothing.
    //
	if (!MyGetGUIThreadInfo (NULL,&GuiThreadInfo))
		return(S_FALSE);

	if (GuiThreadInfo.flags & GUI_INMENUMODE)
	{
		// do I have to loop through all of them to see which
		// one is hilited?? Looks like it...
		mii.cbSize = SIZEOF_MENUITEMINFO;
		mii.fMask = MIIM_STATE;

		SetupChildren();
		for (i=0;i < m_cChildren;i++)
		{
			GetMenuItemInfo (m_hMenu,i,TRUE,&mii);
			if (mii.fState & MFS_HILITE)
			{
				pvarFocus->vt = VT_I4;
				pvarFocus->lVal = i+1;
				return (S_OK);
			}
		}

		// I don't think this should happen
		return(S_FALSE);
	}

    return(S_FALSE);
}



// --------------------------------------------------------------------------
//
//  CMenu::get_accDefaultAction()
//
//  Menu bars have no defaults.  However, items do.  Hierarchical items
//  drop down/pop up their hierarchical.  Non-hierarchical items execute
//  their command.
//
//  doDefaultAction follows from this. It has to do whatever getDefaultAction
//  says it is going to do. We use keystrokes to do this for menu bars. 
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::get_accDefaultAction(VARIANT varChild, BSTR* pszDefA)
{
GUITHREADINFO   gui;
HMENU           hSubMenu;

    InitPv(pszDefA);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(E_NOT_APPLICABLE);


    if( GetMDIButtonIndex( m_hMenu, varChild.lVal - 1 ) != 0 )
    {
        // Special case for MDI child buttons - they are actually implemented
        // as menu items, but appear as buttons.
        return HrCreateString(STR_BUTTON_PUSH, pszDefA);
    }


    // Only if this window is active can we do the default.
    // There is a slight danger here that an Always On Top window
    // could be covering us, but this is small.
    //
    if (!MyGetGUIThreadInfo(0, &gui))
        return(E_NOT_APPLICABLE);

    if (m_hwnd != gui.hwndActive)
        return(E_NOT_APPLICABLE);

    varChild.lVal--;

    // Is this item enabled?
    if (GetMenuState(m_hMenu, varChild.lVal, MF_BYPOSITION) & MFS_GRAYED)
        return(E_NOT_APPLICABLE);

    // Now check if this item has a submenu that is displayed.
    // If there is, the action is hide, if not, the action is show. 
    // If it doesn't have a submenu, the action is execute.
    if (hSubMenu = GetSubMenu(m_hMenu, varChild.lVal))
    {
        if (GetSubMenuWindow(hSubMenu))
            return(HrCreateString(STR_DROPDOWN_HIDE, pszDefA));
        else
            return(HrCreateString(STR_DROPDOWN_SHOW, pszDefA));
    }
    else
        return(HrCreateString(STR_EXECUTE, pszDefA));
}

// --------------------------------------------------------------------------
//
//  CMenu::accSelect()
//
//  We only accept TAKE_FOCUS. What I wanted this to do is to just put the
//  app into menu mode (if it isn't already - more on this later) and then
//  select the item specified - don't open it or anything, just select it.
//
//  But that was a pain in the butt, so no I just use doDefaultAction to
//  do the work. Maybe I'll fix it for 1.1
//
//  If we are already in menu mode, and a popup is open, then we should 
//  close the popup(s) and select the item. If in menu mode and no popups
//  are up, just select the item. 
//
//  If the app is just setting focus to the menu bar itself, and not already
//  in menu mode, just put us into menu mode (automatically selects first
//  item). If we are already in menu mode, do nothing.
//
//  I want to try to do all this without generating a whole mess of extra
//  events!
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::accSelect(long flagsSel, VARIANT varChild)
{
LPARAM          lParam;
GUITHREADINFO   GuiThreadInfo;

    if (!ValidateChild(&varChild) || !ValidateSelFlags(flagsSel))
        return(E_INVALIDARG);

    if (flagsSel != SELFLAG_TAKEFOCUS)
        return(E_NOT_APPLICABLE);

    // if this window is not the active window, fail.
	MyGetGUIThreadInfo (NULL,&GuiThreadInfo);
    if (GuiThreadInfo.hwndActive != m_hwnd)
        return (E_NOT_APPLICABLE);

#ifdef _DEBUG
    if (!m_hMenu)
    {
        //DBPRINTF (TEXT("null hmenu at 1\r\n"));
        Assert (m_hMenu);
    }
#endif

    if (varChild.lVal == CHILDID_SELF)
    {
        if (!m_fSysMenu)
            lParam = NULL;
        else if (GetWindowLong(m_hwnd, GWL_STYLE) & WS_CHILD)
            lParam = MAKELONG('-', 0);
        else
            lParam = MAKELONG(' ', 0);

        PostMessage(m_hwnd, WM_SYSCOMMAND, SC_KEYMENU, lParam);
        return (S_OK);
    }
    else if (GetSubMenu(m_hMenu, varChild.lVal-1))
    {
        // for version 1.0, I'll just do this. Safe, even though it's not 100%
        // what I want it to do.
        return (accDoDefaultAction (varChild));
    }

    return (E_FAIL);
}



// --------------------------------------------------------------------------
//
//  CMenu::accLocation()
//
// get the location of the child. left,top,width,height
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    MENUBARINFO mbi;

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

	if (!MyGetMenuBarInfo(m_hwnd, (m_fSysMenu ? OBJID_SYSMENU : OBJID_MENU),
        varChild.lVal, &mbi))
        return(S_FALSE);

    *pcxWidth = mbi.rcBar.right - mbi.rcBar.left;
    *pcyHeight = mbi.rcBar.bottom - mbi.rcBar.top;

    *pxLeft = mbi.rcBar.left;
    *pyTop = mbi.rcBar.top;

    return(S_OK);
}



// --------------------------------------------------------------------------
//
//  CMenu::accHitTest()
//
// if the point is in a menu bar, return the child the point is over
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::accHitTest(long x, long y, VARIANT* pvarHit)
{
    InitPvar(pvarHit);
    SetupChildren();

    if (SendMessage(m_hwnd, WM_NCHITTEST, 0, MAKELONG(x, y)) == (m_fSysMenu ? HTSYSMENU : HTMENU))
    {
        pvarHit->vt = VT_I4;
        pvarHit->lVal = 0;

        if (m_cChildren)
        {
            if (m_fSysMenu)
                pvarHit->lVal = 1;
            else
            {
                POINT   pt;

                pt.x = x;
                pt.y = y;

				// MenuItemFromPoint conveniently returns -1 if we are not
				// over any menu item, so that gets returned as 0 (CHILDID_SELF)
				// while others get bumped by 1 to be 1..n. Cool!
                pvarHit->lVal = MenuItemFromPoint(m_hwnd, m_hMenu, pt) + 1;
            }

            if (pvarHit->lVal)
            {
                IDispatch* pdispChild;

                pdispChild = NULL;
                get_accChild(*pvarHit, &pdispChild);
                if (pdispChild)
                {
                    pvarHit->vt = VT_DISPATCH;
                    pvarHit->pdispVal = pdispChild;
                }
            }
        }

        return(S_OK);
    }

    return(S_FALSE);
}



// --------------------------------------------------------------------------
//
//  CMenu::accNavigate()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd)
{
long		lEnd = 0;
HMENU		hSubMenu;

    InitPvar(pvarEnd);	

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD)
        dwNavDir = NAVDIR_NEXT;
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        varStart.lVal = m_cChildren + 1;
        dwNavDir = NAVDIR_PREVIOUS;
    }
    else if (varStart.lVal == CHILDID_SELF)
        return(GetParentToNavigate((m_fSysMenu ? OBJID_SYSMENU : OBJID_MENU),
            m_hwnd, OBJID_WINDOW, dwNavDir, pvarEnd));

	// when we get to here, navdir was either firstchild
	// or lastchild (now changed to either next or previous)
	// OR
	// we were starting from something other than the parent object
    switch (dwNavDir)
    {
        case NAVDIR_RIGHT:
        case NAVDIR_NEXT:
            lEnd = varStart.lVal + 1;
            if (lEnd > m_cChildren)
                lEnd = 0;
            break;

        case NAVDIR_LEFT:
        case NAVDIR_PREVIOUS:
            lEnd = varStart.lVal - 1;
            break;

        case NAVDIR_UP:
        case NAVDIR_DOWN:
            lEnd = 0;
            break;
    }

    if (lEnd)
    {
		// we should give the child object back!!
#ifdef _DEBUG
        if (!m_hMenu)
        {
            //DBPRINTF (TEXT("null hmenu at 2\r\n"));
            Assert (m_hMenu);
        }
#endif

		hSubMenu = GetSubMenu (m_hMenu,lEnd-1);
		if (hSubMenu)
		{
			pvarEnd->vt=VT_DISPATCH;
			return(CreateMenuItem((IAccessible*)this, m_hwnd, m_hMenu, hSubMenu,
				lEnd,  0, FALSE, IID_IDispatch, (void**)&pvarEnd->pdispVal));
		}
		// just return VT_I4 if it does not have a submenu.
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;

        return(S_OK);
    }

    return(S_FALSE);
}




// --------------------------------------------------------------------------
//
//  CMenu::accDoDefaultAction()
//
//  Menu bars have no defaults.  However, items do.  Hierarchical items
//  drop down/pop up their hierarchical.  Non-hierarchical items execute
//  their command. To Open something that is closed or to Execute a command, 
//  we use SendInput to send Alt+ShortcutKey. To Close something, we just 
//  send Alt.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::accDoDefaultAction(VARIANT varChild)
{
GUITHREADINFO   gui;
TCHAR           chHotKey;
HMENU	        hSubMenu;
int             i,n;
int             nTries;
#define MAX_TRIES 20

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(E_NOT_APPLICABLE);


    if( GetMDIButtonIndex( m_hMenu, varChild.lVal - 1 ) != 0 )
    {
        // Special case for MDI child buttons - they are actually implemented
        // as menu items, but appear as buttons.

        // Active them by posting a WM_COMMAND with the command id of the
        // corresponding menu item.
        //
        // This WM_COMMAND gets picked up by the MDIClient window, and it then
        // minimizes/restores/closes the currently active child. (If we did
        // a WM_SYSCOMMAND instead of WM_COMMAND, the overall app window
        // would minimize/restore/close instead!)
        int id = GetMenuItemID( m_hMenu, varChild.lVal - 1 );
        PostMessage(m_hwnd, WM_COMMAND, id, 0L);
        return S_OK;
    }


    //
    // Only if this window is active can we do the default.
	//
    if (!MyGetGUIThreadInfo(0, &gui))
        return(E_FAIL);

    if (m_hwnd != gui.hwndActive)
        return(E_NOT_APPLICABLE);

    // If disabled, fail
    if (GetMenuState(m_hMenu, varChild.lVal-1, MF_BYPOSITION) & MFS_GRAYED)
        return(E_NOT_APPLICABLE);

#ifdef _DEBUG
    if (!m_hMenu)
    {
        //DBPRINTF (TEXT("null hmenu at 3\r\n"));
        Assert (m_hMenu);
    }
#endif

    // First check if this item has a sub menu, and if it is open.
    // If it has, and it is, then close it.
    if (hSubMenu = GetSubMenu(m_hMenu, varChild.lVal-1))
    {
        if (GetSubMenuWindow(hSubMenu))
        {
            MyBlockInput (TRUE);
            SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
            SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);
            MyBlockInput (FALSE);
            return (S_OK);
        }
    }

    // when we get here, either it doesn't have a submenu and we need
    // to execute, or the submenu is closed and we need to open it.
    // Our actions are the same in either case. - send Alt+Letter if
    // there is a letter, if not a letter....

    // special case for system menus
    if (m_fSysMenu)
    {
    LPARAM  lParam;

        if (GetWindowLong(m_hwnd, GWL_STYLE) & WS_CHILD)
            lParam = MAKELONG('-', 0);
        else
            lParam = MAKELONG(' ', 0);

        PostMessage(m_hwnd, WM_SYSCOMMAND, SC_KEYMENU, lParam);
        return (S_OK);
    }

    //
    // Get menu item string; get & character.
    //

    chHotKey = GetMenuItemHotkey( this, m_hwnd, m_hMenu, varChild.lVal, 0 );

    if (chHotKey)
    {
        MyBlockInput (TRUE);
        SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
        SendKey (KEYPRESS,VK_CHAR,0,chHotKey);
        SendKey (KEYRELEASE,VK_CHAR,0,chHotKey);
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);
        MyBlockInput (FALSE);
        return (S_OK);
    }
    else
    {
        // Bad Apps don't define hot keys. We can try to move the selection
        // to that item and then hit enter. An easier way would be to just
        // hit Alt+FirstLetter, but if there are more than 1 item with that
        // letter, it will always do the first one. Not optimal, may lead to
        // unexpected side-effects. Better to do nothing than to do that.
        //
        // We need to put ourselves in menu mode if we aren't already, then
        // send right arrow keys to put us on the right one, then hit Enter.
        // If we are already in menu mode, take us out of menu mode to close
        // the heirarchy, then go back into menu mode and continue.
        MyBlockInput (TRUE);
        if (gui.flags & GUI_INMENUMODE)
        {
            SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
            SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);
        }

        // now go into menu mode and send right arrows until the one we 
        // want is highlighted.
        SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);

        // calculate how many right arrows to hit:
        n = varChild.lVal-1;
        // if this menu is the menu of an MDI window and the window
        // is maximized, then the thing now highlighted is the MDI
        // Doc Sys menu, and we'll have to go 1 farther than we think.
        // To see if this is the case, we'll check if the first item
        // in the menu is something with a submenu and it is a bitmap menu.
        if (GetSubMenu(m_hMenu,0) &&
            (GetMenuState(m_hMenu, 0, MF_BYPOSITION) & MF_BITMAP))
            n++;

        for (i = 0; i < n;i++)
        {
            SendKey (KEYPRESS,VK_VIRTUAL,VK_RIGHT,0);
            SendKey (KEYRELEASE,VK_VIRTUAL,VK_RIGHT,0);
        }
        MyBlockInput (FALSE);        

        // check if it is highlighted now. If so, hit enter to activate.
        // try several times - 
        nTries = 0;
        while ( ((GetMenuState(m_hMenu, varChild.lVal-1, MF_BYPOSITION) & MF_HILITE) == 0) &&
                (nTries < MAX_TRIES))
        {
            Sleep(55);
            nTries++;
        }

        if (GetMenuState(m_hMenu, varChild.lVal-1, MF_BYPOSITION) & MF_HILITE)
        {
            MyBlockInput (TRUE);        
            SendKey (KEYPRESS,VK_VIRTUAL,VK_RETURN,0);
            SendKey (KEYRELEASE,VK_VIRTUAL,VK_RETURN,0);
            MyBlockInput (FALSE);        
            return (S_OK);
        }
        else
            return (E_FAIL);
    }
}



// --------------------------------------------------------------------------
//
//  CMenu::Clone()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenu::Clone(IEnumVARIANT** ppenum)
{
    return(CreateMenuBar(m_hwnd, m_fSysMenu, m_idChildCur, IID_IEnumVARIANT,
            (void**)ppenum));
}


STDMETHODIMP CMenu::GetIdentityString (
    DWORD	    dwIDChild,
    BYTE **     ppIDString,
    DWORD *     pdwIDStringLen
)
{
    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    BYTE * pKeyData = (BYTE *) CoTaskMemAlloc( HMENUKEYSIZE );
    if( ! pKeyData )
    {
        return E_OUTOFMEMORY;
    }

    DWORD dwpid;
    GetWindowThreadProcessId( m_hwnd, & dwpid );
    MakeHmenuKey( pKeyData, dwpid, m_hMenu, dwIDChild );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HMENUKEYSIZE;

    return S_OK;
}




/////////////////////////////////////////////////////////////////////////////
//
//  MENU ITEMS
//
/////////////////////////////////////////////////////////////////////////////


// --------------------------------------------------------------------------
//
//  CreateMenuItem()
//
//  This creates a child object for a menu item that has a sub menu.
//
//	Parameters:
//	paccMenu	IN		pointer to the parent's IAccessible
//	hwnd		IN		the hwnd of the window that owns the parent menu
//	hMenu		IN		the hmenu of the menu that owns this item.
//	hSubMenu	IN		the hMenu of the submenu this menu item opens
//	ItemID		IN		the menu item ID. Position (1..n).
//	iCurChild	IN		ID of the current child in the enumeration
//	fPopup		IN		is this menu item in a popup or on a menu bar?
//	riid		IN		what interface are we asking for on this item?
//	ppvItem		OUT		the pointer to the interface asked for.
//
// --------------------------------------------------------------------------
HRESULT CreateMenuItem(IAccessible* paccMenu, HWND hwnd, HMENU hMenu, 
	HMENU hSubMenu, long ItemID, long iCurChild, BOOL fPopup, REFIID riid, 
	void** ppvItem)
{
    HRESULT hr;
    CMenuItem* pmenuitem;

    InitPv(ppvItem);

    pmenuitem = new CMenuItem(paccMenu, hwnd, hMenu, hSubMenu, ItemID, iCurChild, fPopup);
    if (! pmenuitem)
        return(E_OUTOFMEMORY);

    hr = pmenuitem->QueryInterface(riid, ppvItem);
    if (!SUCCEEDED(hr))
        delete pmenuitem;

    return(hr);
}



// --------------------------------------------------------------------------
//
//  CMenuItem::CMenuItem()
//
//  We hang on to our parent object so we can forward methods up to him.
//  Therefore we must bump up the ref count.
//
// --------------------------------------------------------------------------
CMenuItem::CMenuItem(IAccessible* paccParent, HWND hwnd, HMENU hMenu,
    HMENU hSubMenu, long ItemID, long iCurChild, BOOL fPopup)
    : CAccessible( CLASS_MenuItemObject )
{
    m_hwnd = hwnd;
    m_hMenu = hMenu;
	m_hSubMenu = hSubMenu;
    m_ItemID = ItemID;
    m_idChildCur = iCurChild;
    m_fInAPopup = fPopup;

    m_paccParent = paccParent;
    paccParent->AddRef();
}



// --------------------------------------------------------------------------
//
//  CMenuItem::~CMenuItem()
//
//  We hung on to our parent, so we must release it on destruction.
//
// --------------------------------------------------------------------------
CMenuItem::~CMenuItem()
{
    m_paccParent->Release();
}


// --------------------------------------------------------------------------
//
//  SetupChildren()
//
//  CMenuItems have 1 child. That one child is either a CMenuPopupFrame or
//  a CMenuPopup (depending if the menu is visible).
//
// --------------------------------------------------------------------------
void CMenuItem::SetupChildren(void)
{
    m_cChildren = 1;
}

// --------------------------------------------------------------------------
//
//  CMenuItem::get_accParent()
//
// Pass it on back to the parent.
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::get_accParent(IDispatch** ppdispParent)
{
    InitPv(ppdispParent);

    return(m_paccParent->QueryInterface(IID_IDispatch, (void**)ppdispParent));
}



// --------------------------------------------------------------------------
//
//  CMenuItem::get_accChild()
//
//  The menu item's child is either a CMenuPopupFrame (if the popup window
//  is visible and belongs to this CMenuItem) or a CMenuPopup. This allows
//  someone to enumerate the commands whether or not the popup is visible.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::get_accChild(VARIANT varChild, IDispatch** ppdispChild)
{
HWND    hwndSubMenu;
HRESULT hr;

    InitPv(ppdispChild);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);
	
    if (varChild.lVal != CHILDID_SELF)
    {
        // In order to create the accessible object representing the child,
        // we have to find the popup menu window.
        // Once we have found it, we check if it is visible. If so, then
        // our child is a CMenuPopupFrame, which we will create by
        // calling CreateMenuPopupWindow.
        // If the popup window is not visible, or if it does not belong
        // to this CMenuItem, then our child is a CMenuPopup, which we
        // will create by calling CreateMenuPopup.
        //
        hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
        if (hwndSubMenu)
            return (CreateMenuPopupWindow (hwndSubMenu,0,IID_IDispatch, (void**)ppdispChild));
        else
        {
            // this is where we create 'invisible' popups so apps can
            // walk down and see all of the commands (most, at least).
            // Since it is invisible, we have to tell it more about who
            // it's parent is.
            hr = CreateMenuPopupClient (NULL,0,IID_IDispatch,(void**)ppdispChild);
            if (SUCCEEDED (hr))
                ((CMenuPopup*)*ppdispChild)->SetParentInfo((IAccessible*)this,
                        m_hSubMenu,varChild.lVal);

            return(hr);
        }
    }

    return(E_INVALIDARG);
}


// --------------------------------------------------------------------------
//
//  CMenuItem::get_accName()
//
//  The name for the child (CMenuPopup or CMenuPopupFrame) is the same as
//  the name of the Parent/Self, so whether we are asked for id=self or
//  id = child (1), we return the same thing.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    return GetMenuItemName( this, m_hwnd, m_hMenu, m_ItemID, pszName );
}



// --------------------------------------------------------------------------
//
//  CMenuItem::get_accRole()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
MENUITEMINFO mi;

    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    if (varChild.lVal == CHILDID_SELF)
    {
        mi.cbSize = SIZEOF_MENUITEMINFO;
        mi.fMask = MIIM_TYPE | MIIM_SUBMENU;
        mi.cch = 0;
        mi.dwTypeData = 0;

        GetMenuItemInfo(m_hMenu, m_ItemID-1, TRUE, &mi);
        if (mi.fType & MFT_SEPARATOR)
            pvarRole->lVal = ROLE_SYSTEM_SEPARATOR;
        else
            pvarRole->lVal = ROLE_SYSTEM_MENUITEM;
    }
    else
    {
        pvarRole->lVal = ROLE_SYSTEM_MENUPOPUP;
    }
    return(S_OK);
}



// --------------------------------------------------------------------------
//
//  CMenuItem::get_accState()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::get_accState(VARIANT varChild, VARIANT* pvarState)
{
HWND    hwndSubMenu;

    InitPvar(pvarState);

	if (!ValidateChild (&varChild))
        return(E_INVALIDARG);

	// We do this because sometimes we'll be asked for our own info,
	// and the caller will just call us item 0 (CHILDID_SELF) and we 
	// have to make sure when we call our parent to tell her who
	// we are (m_ItemID). 
	if (varChild.lVal == CHILDID_SELF)
    {
		varChild.lVal = m_ItemID;
        return(m_paccParent->get_accState(varChild, pvarState));
    }
    else
    {
        // If the popup (our only child) is not showing or it belongs to 
        // another menu item, set the state to invisible.
        // If it is showing and belongs to us, set the state to normal.
        
        // This starts by assuming that it is invisible, and clearing the
        // state if we find a visible menu that belongs to us.
        pvarState->vt = VT_I4;
        pvarState->lVal = 0 | STATE_SYSTEM_INVISIBLE;

        hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
        if (hwndSubMenu)
            pvarState->lVal = 0;
    }
    return (S_OK);
}

// --------------------------------------------------------------------------
//
//  CMenuItem::get_accKeyboardShortcut()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

    if (! ValidateChild(&varChild))
        return E_INVALIDARG;

    if (varChild.lVal == CHILDID_SELF)
    {
        // Get menu item shortcut - use "Alt+" format for menu bars...
        BOOL fIsMenuBar = m_hwnd && ::GetMenu( m_hwnd ) == m_hMenu;

        return GetMenuItemShortcut( this, m_hwnd, m_hMenu, m_ItemID, fIsMenuBar, pszShortcut );
    }

    return S_FALSE;

}



// --------------------------------------------------------------------------
//
//  CMenuItem::get_accFocus()
//
//  If focus is us or our popup, great.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::get_accFocus(VARIANT* pvarFocus)
{
HRESULT         hr;
HWND            hwndSubMenu;
IDispatch*      pdispChild;

    // Ask our parent who has the focus.  Is it us?
    hr = m_paccParent->get_accFocus(pvarFocus);
    if (!SUCCEEDED(hr))
        return(hr);

    // No, so return nothing.
    if ((pvarFocus->vt != VT_I4) || (pvarFocus->lVal != m_ItemID))
    {
        VariantClear(pvarFocus);
        pvarFocus->vt = VT_EMPTY;
        return(S_FALSE);
    }

    // Is the currently active popup our child?
    // If so, then we should return an IDispatch to
    // the window frame object.
    hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
    if (hwndSubMenu)
    {
        hr = CreateMenuPopupWindow (hwndSubMenu,0,IID_IDispatch,(void**)&pdispChild);

        if (!SUCCEEDED(hr))
            return (hr);
        pvarFocus->vt = VT_DISPATCH;
        pvarFocus->pdispVal = pdispChild;
        return (S_OK);
    }
    
    pvarFocus->lVal = 0;
    return(S_OK);
}




// --------------------------------------------------------------------------
//
//  CMenuItem::get_accDefaultAction()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::get_accDefaultAction(VARIANT varChild, BSTR* pszDefA)
{
    InitPv(pszDefA);

	if (!ValidateChild (&varChild))
        return(E_INVALIDARG);

	// We do this because sometimes we'll be asked for our own info,
	// and the caller will just call us item 0 (CHILDID_SELF) and we 
	// have to make sure when we call our parent to tell her who
	// we are (m_ItemID). 
    // But sometimes, we will be asked for info about our child - There 
    // is no default action for our child.
	if (varChild.lVal == CHILDID_SELF)
    {
		varChild.lVal = m_ItemID;
        return(m_paccParent->get_accDefaultAction(varChild, pszDefA));
    }
    return (E_NOT_APPLICABLE);
}



// --------------------------------------------------------------------------
//
//  CMenuItem::accSelect()
//
//  We just let our parent take care of this for us. Tell her who we are by
//  setting varChild.lVal to our ItemID.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::accSelect(long flagsSel, VARIANT varChild)
{
    if (!ValidateChild (&varChild) ||
        !ValidateSelFlags(flagsSel))
        return (E_INVALIDARG);

	if (varChild.lVal == CHILDID_SELF)
		varChild.lVal = m_ItemID;
    return(m_paccParent->accSelect(flagsSel, varChild));
}

// --------------------------------------------------------------------------
//
//  CMenuItem::accLocation()
//
//  Sometimes we are asked for the location of a peer object. This is 
//  kinda screwy. This happens when we are asked to navigate next or prev,
//	and then let our parent navigate for us. The caller then starts thinking
//	we know about our peers. 
//  Since this is the only case where something like this happens, we'll
//	have to do some sort of hack. 
//  Problem is, when they ask for a child 0 (self) we are OK.
//	But when we are asked for child 1, is it the popup or peer 1?
//  I am going to assume that it is always the peer.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::accLocation(long* pxLeft, long* pyTop,
    long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
	// we just call this to translate empty values - not going to 
	// check the return value.
	ValidateChild (&varChild);

	if (varChild.lVal == CHILDID_SELF)
		varChild.lVal = m_ItemID;

    return(m_paccParent->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));
}



// --------------------------------------------------------------------------
//
//  CMenuItem::accNavigate()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT* pvarEnd)
{
HWND        hwndSubMenu;

	InitPvar(pvarEnd);

    if (!ValidateChild(&varStart))
        return (E_INVALIDARG);

    if (!ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir >= NAVDIR_FIRSTCHILD) // this means firstchild or lastchild
    {
        hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
        if (hwndSubMenu)
        {
            pvarEnd->vt = VT_DISPATCH;
            return (CreateMenuPopupWindow (hwndSubMenu,0,IID_IDispatch, (void**)&(pvarEnd->pdispVal)));
        }

        return(S_FALSE);
    }
    else
    {
		if (varStart.lVal == CHILDID_SELF)
			varStart.lVal = m_ItemID;
        return(m_paccParent->accNavigate(dwNavDir, varStart, pvarEnd));
    }
}



// --------------------------------------------------------------------------
//
//  CMenuItem::accHitTest()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::accHitTest(long x, long y, VARIANT* pvarHit)
{
HRESULT hr;
HWND    hwndSubMenu;
RECT    rc;
POINT   pt;

    InitPvar(pvarHit);

    hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
    if (hwndSubMenu)
    {
        // Is point in our popup menu window child?
        MyGetRect(hwndSubMenu, &rc, TRUE);

        pt.x = x;
        pt.y = y;

        if (PtInRect(&rc, pt))
        {
            // need to set the parent
            pvarHit->vt = VT_DISPATCH;
            return (CreateMenuPopupWindow (hwndSubMenu,0,IID_IDispatch, (void**)pvarHit->pdispVal));
        }
    }

    // Is point in us?
    hr = m_paccParent->accHitTest(x, y, pvarHit);
    // #11150, CWO, 1/24/97, changed from !SUCCEEDED to !S_OK
    if ((hr != S_OK) || (pvarHit->vt == VT_EMPTY))
        return(hr);

	pvarHit->vt = VT_I4;
	pvarHit->lVal = CHILDID_SELF;

    return(S_OK);
}



// --------------------------------------------------------------------------
//
//  CMenuItem::accDoDefaultAction()
//
//  We just let our parent take care of this for us. Tell her who we are by
//  setting varChild.lVal to our ItemID.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::accDoDefaultAction(VARIANT varChild)
{
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    Assert(varChild.vt == VT_I4);
	if (varChild.lVal == CHILDID_SELF)
		varChild.lVal = m_ItemID;
    return(m_paccParent->accDoDefaultAction(varChild));
}



// --------------------------------------------------------------------------
//
//  CMenuItem::Clone()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuItem::Clone(IEnumVARIANT** ppenum)
{
    return(CreateMenuItem(m_paccParent, m_hwnd, m_hMenu, m_hSubMenu,m_ItemID, 
		m_idChildCur, FALSE, IID_IEnumVARIANT, (void**)ppenum));
}


STDMETHODIMP CMenuItem::GetIdentityString (
    DWORD	    dwIDChild,
    BYTE **     ppIDString,
    DWORD *     pdwIDStringLen
)
{
    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    if( dwIDChild != CHILDID_SELF )
    {
        // CMenuItems have 1 child - that one child is either a CMenuPopupFrame or
        // a CMenuPopup (depending if the menu is visible).
        // We're not going to support geting the IDs of these thourhg the parent,
        // a client should get the interface pointers to those objects themselves,
        // and ask them for their string.
        return E_INVALIDARG;
    }



    // Weird stuff alert:
    //
    // CMenuItem repesents a menu item that has an associated popup. (ie. it is
    // not a command leaf node) There are two options for representing this item:
    //
    //     as a child of its parent menu,
    // or
    //     as the parent (CHILDID_SELF) of its own submenu
    //
    // While the HWND-based controls use the latter option, here we are going for
    // the former - this keeps the menu item with its siblings, regardless of
    // whether they are menu items with popups, or leaf-node commands.
    // We can do this here since we do know what our parent HMENU is and what our
    // child id is in that parent menu.
    // HWND-based proxies as generally don't have this information available to them,
    // so the former option is not really an option for them.

    // This request is for the item itself - represent it as a child of
    // our parent menu

    BYTE * pKeyData = (BYTE *) CoTaskMemAlloc( HMENUKEYSIZE );
    if( ! pKeyData )
    {
        return E_OUTOFMEMORY;
    }

    // Need to find pid of process that the menu belongs to. Can't use the
    // pid of the popup menu, since that's a shared/reused system window.
    // Instead, we assume that since the menu is present, it belongs to the
    // current foreground thread, which is what GetGUIThreadInfo(NULL) gets us.
    GUITHREADINFO	GuiThreadInfo;
    if( ! MyGetGUIThreadInfo( NULL, & GuiThreadInfo ) )
        return E_FAIL;
    DWORD dwPid = 0;
    GetWindowThreadProcessId( GuiThreadInfo.hwndActive, & dwPid );
    if( dwPid == 0 )
        return E_FAIL;

    MakeHmenuKey( pKeyData, dwPid, m_hMenu, m_ItemID );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HMENUKEYSIZE;

    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
//
//  MENU POPUPS
//
/////////////////////////////////////////////////////////////////////////////



// --------------------------------------------------------------------------
//
//  CreateMenuPopupClient()
//
//  EXTERNAL for CreateClientObject...
//
// --------------------------------------------------------------------------
HRESULT CreateMenuPopupClient(HWND hwnd, long idChildCur,
    REFIID riid, void** ppvPopup)
{
    CMenuPopup*     ppopup;
    HRESULT         hr;

    ppopup = new CMenuPopup(hwnd, idChildCur);
    if (!ppopup)
        return(E_OUTOFMEMORY);

    hr = ppopup->QueryInterface(riid, ppvPopup);
    if (!SUCCEEDED(hr))
        delete ppopup;

    return(hr);
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::CMenuPopup()
//
// --------------------------------------------------------------------------
CMenuPopup::CMenuPopup(HWND hwnd, long idChildCur)
    : CClient( CLASS_MenuPopupClient )
{
    Initialize(hwnd, idChildCur);
    m_hMenu = NULL;
    m_ItemID = 0;
    m_hwndParent = NULL;
    m_fSonOfPopup = 0;
    m_fSysMenu = 0;

    // this only works if there is a window handle.
    if (hwnd)
    {
        m_hMenu = (HMENU)SendMessage (m_hwnd,MN_GETHMENU,0,0);
        // if we didn't get back an HMENU, that means that the window
        // is probably invisible. Don't try to set other values. 
        // SetupChildren will see this and set m_cChildren to 0.
        if (m_hMenu)
        {
	        m_ItemID = FindItemIDThatOwnsThisMenu (m_hMenu,&m_hwndParent,
                &m_fSonOfPopup,&m_fSysMenu);
        }
    }
}

// --------------------------------------------------------------------------
//
// The CMenuPopup objects need to know their parent when they are invisible, 
// so after one is created, the creator should call SetParentInfo.
//
// --------------------------------------------------------------------------
void CMenuPopup::SetParentInfo(IAccessible* paccParent,HMENU hMenu,long ItemID)
{
    m_paccParent = paccParent;
    m_hMenu = hMenu;
    m_ItemID= ItemID;
    if (paccParent)
        paccParent->AddRef();
}

// --------------------------------------------------------------------------
//
//  CMenuPopup::~CMenuPopup()
//
// --------------------------------------------------------------------------
CMenuPopup::~CMenuPopup(void)
{
    if (m_paccParent)
        m_paccParent->Release();
}

// --------------------------------------------------------------------------
//
//  CMenuPopup::SetupChildren()
//
// --------------------------------------------------------------------------
void CMenuPopup::SetupChildren(void)
{
    // we need to be able to set up our children whether the popup is 
    // displayed or not. So we have a m_hMenu variable, it just needs
    // to be set when the thing is made - It is either set by the 
    // constructor (if we are visible) or by the dude that called the create
    // function if we are invisible.
    // PROBLEM - sometimes CMenuPopups are created by a call to 
    // AccessibleObjectFromEvent, and the hwnd isn't always able to 
    // give us back a good m_hMenu. So we will just set m_cChildren to 0.
    if (m_hMenu)
    {
        m_cChildren = GetMenuItemCount(m_hMenu);

        if( m_cChildren == -1 )
        {
            // Paranoia in case we get an invalid HMENU
            m_cChildren = 0;
        }
    }
    else
        m_cChildren = 0;
}

// --------------------------------------------------------------------------
//
//  CMenuPopup::get_accParent()
//
//  The parent of a CMenuPopup is either a CMenuPopupFrame or a CMenuItem.
//  If the popup is visible, it will have an hwnd, and lots of other stuff
//  will also be set. If it is not visible, it will not have an hwnd.
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::get_accParent(IDispatch** ppdispParent)
{
    if (m_paccParent)
    {
        return (m_paccParent->QueryInterface(IID_IDispatch,(void**)ppdispParent));
    }
    else if (m_hwnd)
    {
        // try to create a parent for us...
        return (CreateMenuPopupWindow (m_hwnd,0,IID_IDispatch,(void**)ppdispParent));
    }
    else
        return (E_FAIL);
}

// --------------------------------------------------------------------------
//
//  CMenuPopup::get_accChild()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::get_accChild(VARIANT varChild, IDispatch** ppdispChild)
{
HMENU	hSubMenu;

    InitPv(ppdispChild);

    if (!ValidateChild(&varChild) || varChild.lVal == CHILDID_SELF)
        return(E_INVALIDARG);

    //
    // Is this item a hierarchical?
    //
    Assert (m_hMenu);
	hSubMenu = GetSubMenu(m_hMenu, varChild.lVal-1);
    if (!hSubMenu)
        return(S_FALSE);

    //
    // Yes.
    //
	return(CreateMenuItem((IAccessible*)this, m_hwnd, m_hMenu, hSubMenu,
		varChild.lVal,  0, FALSE, IID_IDispatch, (void**)ppdispChild));
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::get_accName()
//
//  The name of the popup is the name of the item it hangs off of.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::get_accName(VARIANT varChild, BSTR* pszName)
{
HWND            hwndOwner;
TCHAR           szClassName[50];
HRESULT         hr;
IAccessible*    paccParent;

    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
    {
        // If we popped up from a menu bar or as another popup,
		// then our name is the name of the thing that popped us
		// up. If we are a floating popup, then our name is...?
		//
        // We implement this by either:
        // 1. calling our parent object, OR
        // 2. creating a parent object on the fly that we can 
        //    ask the name of, OR
		// 3. Looking for the name of the owner window, OR
        // 4. Checking if we are the child of the start button. 
        // If all else fails, we'll just call ourselves "context menu".
        if (m_paccParent && m_ItemID)
        {
            varChild.vt = VT_I4;
            varChild.lVal = m_ItemID;
            return (m_paccParent->get_accName (varChild,pszName));
        }
        if (m_hwndParent && m_ItemID)
        {
            varChild.vt = VT_I4;
            varChild.lVal = m_ItemID;

            if (m_fSonOfPopup)
                hr = CreateMenuPopupClient(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            else if (m_fSysMenu)
                hr = CreateSysMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            else
                hr = CreateMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);

            if (SUCCEEDED(hr))
            {
                hr = paccParent->get_accName (varChild,pszName);
                paccParent->Release();
            }
            return (hr);
        }
        else
		{
			// Try to get the owner window and use that for a name
			// This doesn't seem to work on anything I have ever found,
            // but it should work if anything has an owner, so i'll
            //leav it in. If it starts breaking, just rip it out.
			if (m_hwnd)
			{
			IAccessible*	pacc;
			HRESULT			hr;

				hwndOwner = ::GetWindow (m_hwnd,GW_OWNER);
				hr = AccessibleObjectFromWindow (hwndOwner, OBJID_WINDOW, IID_IAccessible, (void**)&pacc);
				if (SUCCEEDED(hr))
				{
					hr = pacc->get_accName(varChild,pszName);
                    pacc->Release();
					if (SUCCEEDED(hr))
                    {
						return (hr);
                    }
				}
			}

			// check if the start button has focus
			hwndOwner = MyGetFocus();
			if (InTheShell(hwndOwner, SHELL_TRAY))
			{
                GetClassName(hwndOwner,szClassName,ARRAYSIZE(szClassName));
                if (lstrcmp(szClassName,TEXT("Button")) == 0)
                {
                    return (HrCreateString(STR_STARTBUTTON,pszName));
                }
			}
			// at least return this for a name
            return (HrCreateString (STR_CONTEXT_MENU,pszName));
		} // end else we don't have m_paccparent && m_itemid
    } // end if childid_self
    else // not childid self, childid > 0
    {
        return GetMenuItemName( this, m_hwnd, m_hMenu, varChild.lVal, pszName );
    }

    return(S_OK);
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::get_accDescription()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::get_accDescription(VARIANT varChild, BSTR* pszDesc)
{
    InitPv(pszDesc);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(CClient::get_accDescription(varChild, pszDesc));

    return(E_NOT_APPLICABLE);
}




// --------------------------------------------------------------------------
//
//  CMenuPopup::get_accRole()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    if (varChild.lVal == CHILDID_SELF)
        pvarRole->lVal = ROLE_SYSTEM_MENUPOPUP;
    else
    {
        MENUITEMINFO mi;

        mi.cbSize = SIZEOF_MENUITEMINFO;
        mi.fMask = MIIM_TYPE;
        mi.cch = 0;
        mi.dwTypeData = 0;

        if (GetMenuItemInfo(m_hMenu, varChild.lVal-1, TRUE, &mi) &&
               (mi.fType & MFT_SEPARATOR))
            pvarRole->lVal = ROLE_SYSTEM_SEPARATOR;
        else
            pvarRole->lVal = ROLE_SYSTEM_MENUITEM;
    }

    return(S_OK);
}




// --------------------------------------------------------------------------
//
//  CMenuPopup::get_accState()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (varChild.lVal == CHILDID_SELF)
        return(CClient::get_accState(varChild, pvarState));
    else
    {
        // GetMenuBarInfo fails if the menu isn't currently present.
        // We don't fail outright when this happens, since we still want
        // to collect other info using GetMenuItemInfo below.
        MENUBARINFO     mbi;
        if( MyGetMenuBarInfo(m_hwnd, OBJID_CLIENT, varChild.lVal, &mbi) )
        {
            if (mbi.fFocused)
            {
                pvarState->lVal |= STATE_SYSTEM_FOCUSED | STATE_SYSTEM_HOTTRACKED;
            }
        }

        //
        // Get menu item flags.  NOTE:  Can't use GetMenuState().  It whacks
        // random stuff in for hierarchicals.
        //
        MENUITEMINFO    mi;
        mi.cbSize = SIZEOF_MENUITEMINFO;
        mi.fMask = MIIM_STATE | MIIM_SUBMENU;

        if (!GetMenuItemInfo(m_hMenu, varChild.lVal-1, TRUE, &mi))
        {
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
            return(S_FALSE);
        }

        if (mi.fState & MFS_GRAYED)
            pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;

        if (mi.fState & MFS_CHECKED)
            pvarState->lVal |= STATE_SYSTEM_CHECKED;

        if (mi.fState & MFS_DEFAULT)
            pvarState->lVal |= STATE_SYSTEM_DEFAULT;

        if (mi.hSubMenu)
            pvarState->lVal |= STATE_SYSTEM_HASPOPUP;
    }

    return(S_OK);
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::get_accKeyboardShortcut()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    if (varChild.lVal == CHILDID_SELF)
        return CClient::get_accKeyboardShortcut(varChild, pszShortcut);

    // Get menu item shortcut. FALSE means don't use ALT+ form.
    return GetMenuItemShortcut( this, m_hwnd, m_hMenu, varChild.lVal, FALSE, pszShortcut );
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::get_accFocus()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::get_accFocus(VARIANT* pvarFocus)
{
GUITHREADINFO	GuiThreadInfo;
MENUITEMINFO	mii;
int				i;

	// set it to empty
    if (IsBadWritePtr(pvarFocus,sizeof(VARIANT*)))
        return (E_INVALIDARG);

    InitPvar(pvarFocus);

    //
    // Are we in menu mode?  If not, nothing.
    //
	if (!MyGetGUIThreadInfo (NULL,&GuiThreadInfo))
		return(S_FALSE);

	if (GuiThreadInfo.flags & GUI_INMENUMODE)
	{
		// do I have to loop through all of them to see which
		// one is hilited?? Looks like it...
		mii.cbSize = SIZEOF_MENUITEMINFO;
		mii.fMask = MIIM_STATE;

		SetupChildren();
		for (i=0;i < m_cChildren;i++)
		{
			GetMenuItemInfo (m_hMenu,i,TRUE,&mii);
			if (mii.fState & MFS_HILITE)
			{
				pvarFocus->vt = VT_I4;
				pvarFocus->lVal = i+1;
				return (S_OK);
			}
		}

		// I don't think this should happen
		return(S_FALSE);
	}

    return(S_FALSE);
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::get_accDefaultAction()
//
//  Popups have no defaults.  However, items do.  Hierarchical items
//  drop down/pop up their hierarchical.  Non-hierarchical items execute
//  their command.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::get_accDefaultAction(VARIANT varChild, BSTR* pszDefA)
{
HMENU   hSubMenu;

    InitPv(pszDefA);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(E_NOT_APPLICABLE);

    varChild.lVal--;

    // Is this item enabled?
    if (GetMenuState(m_hMenu, varChild.lVal, MF_BYPOSITION) & MFS_GRAYED)
        return(E_NOT_APPLICABLE);


    // Now check if this item has a submenu that is displayed.
    // If there is, the action is hide, if not, the action is show. 
    // If it doesn't have a submenu, the action is execute.
#ifdef _DEBUG
    if (!m_hMenu)
    {
        //DBPRINTF ("null hmenu at 4\r\n");
        Assert (m_hMenu);
    }
#endif

    if (hSubMenu = GetSubMenu(m_hMenu, varChild.lVal))
    {
        if (GetSubMenuWindow(hSubMenu))
            return(HrCreateString(STR_DROPDOWN_HIDE, pszDefA));
        else
            return(HrCreateString(STR_DROPDOWN_SHOW, pszDefA));
    }
    else
        return(HrCreateString(STR_EXECUTE, pszDefA));
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::accSelect()
//
//  We only accept TAKEFOCUS.  
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::accSelect(long flagsSel, VARIANT varChild)
{
    if (!ValidateChild(&varChild) || !ValidateSelFlags(flagsSel))
        return E_INVALIDARG;

    if (flagsSel != SELFLAG_TAKEFOCUS)
        return E_NOT_APPLICABLE;

    return E_NOTIMPL;
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::accLocation()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
MENUBARINFO mbi;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

    if (!MyGetMenuBarInfo(m_hwnd, OBJID_CLIENT, varChild.lVal, &mbi))
        return(S_FALSE);

    *pcyHeight = mbi.rcBar.bottom - mbi.rcBar.top;
    *pcxWidth = mbi.rcBar.right - mbi.rcBar.left;

    *pyTop = mbi.rcBar.top;
    *pxLeft = mbi.rcBar.left;

    return(S_OK);
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::accHitTest()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::accHitTest(long x, long y, VARIANT* pvarHit)
{
HRESULT hr;

    // first make sure we are pointing to our own client area
    hr = CClient::accHitTest(x, y, pvarHit);
    // #11150, CWO, 1/27/97, Replaced !SUCCEEDED with !S_OK
    if ((hr != S_OK) || (pvarHit->vt != VT_I4) || (pvarHit->lVal != 0))
        return(hr);

    // now we can see which child is at this point.
    SetupChildren();

    if (m_cChildren)
    {
        POINT   pt;

        pt.x = x;
        pt.y = y;

        pvarHit->lVal = MenuItemFromPoint(m_hwnd, m_hMenu, pt) + 1;

        if (pvarHit->lVal)
        {
            IDispatch* pdispChild;

            pdispChild = NULL;
            get_accChild(*pvarHit, &pdispChild);
            if (pdispChild)
            {
                pvarHit->vt = VT_DISPATCH;
                pvarHit->pdispVal = pdispChild;
            }
        }
            
        return(S_OK);
    }

    return(S_FALSE);
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::accNavigate()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd)
{
long            lEnd = 0;
MENUITEMINFO    mi;

    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD)
        dwNavDir = NAVDIR_NEXT;
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        varStart.lVal = m_cChildren + 1;
        dwNavDir = NAVDIR_PREVIOUS;
    }
    else if (!varStart.lVal)
	{
        return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));
	}

    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
        case NAVDIR_DOWN:
            lEnd = varStart.lVal + 1;
            if (lEnd > m_cChildren)
                lEnd = 0;
            break;

        case NAVDIR_PREVIOUS:
        case NAVDIR_UP:
            lEnd = varStart.lVal - 1;
            break;

        case NAVDIR_LEFT:
        case NAVDIR_RIGHT:
            lEnd = 0;
            break;
    }

    if (lEnd)
    {
		// we should give the child object back!!
		// can't use getSubMenu here because it seems to ignore
		// separators??
		//hSubMenu = GetSubMenu (m_hMenu,lEnd-1);

        mi.cbSize = SIZEOF_MENUITEMINFO;
        mi.fMask = MIIM_SUBMENU;
        mi.cch = 0;
        mi.dwTypeData = 0;
		GetMenuItemInfo (m_hMenu,lEnd-1,TRUE,&mi);
		if (mi.hSubMenu)
		{
			pvarEnd->vt=VT_DISPATCH;
			return(CreateMenuItem((IAccessible*)this, m_hwnd, m_hMenu, mi.hSubMenu,
				lEnd,  0, FALSE, IID_IDispatch, (void**)&pvarEnd->pdispVal));
		}
		// just return VT_I4 if it does not have a submenu.
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;

        return(S_OK);
    }

    return(S_FALSE);
}




// --------------------------------------------------------------------------
//
//  CMenuPopup::accDoDefaultAction()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::accDoDefaultAction(VARIANT varChild)
{
RECT		rcLoc;
HRESULT		hr;

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(CClient::accDoDefaultAction(varChild));

    // If disabled, fail
    if (GetMenuState(m_hMenu, varChild.lVal-1, MF_BYPOSITION) & MFS_GRAYED)
        return(E_NOT_APPLICABLE);

	hr = accLocation(&rcLoc.left,&rcLoc.top,&rcLoc.right,&rcLoc.bottom,varChild);
	if (!SUCCEEDED (hr))
		return (hr);
	
	// this will check if WindowFromPoint at the click point is the same
	// as m_hwnd, and if not, it won't click. Cool!
	if (ClickOnTheRect(&rcLoc,m_hwnd,FALSE))
		return (S_OK);
	else
		return (E_NOT_APPLICABLE);
}



// --------------------------------------------------------------------------
//
//  CMenuPopup::Clone()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopup::Clone(IEnumVARIANT **ppenum)
{
HRESULT hr;

    hr = CreateMenuPopupClient(m_hwnd, m_idChildCur, IID_IEnumVARIANT,
        (void**)ppenum);
    if (SUCCEEDED(hr))
        ((CMenuPopup*)*ppenum)->SetParentInfo((IAccessible*)this,m_hMenu,m_ItemID);
    return(hr);
}



STDMETHODIMP CMenuPopup::GetIdentityString (
    DWORD	    dwIDChild,
    BYTE **     ppIDString,
    DWORD *     pdwIDStringLen
)
{
    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    BYTE * pKeyData = (BYTE *) CoTaskMemAlloc( HMENUKEYSIZE );
    if( ! pKeyData )
    {
        return E_OUTOFMEMORY;
    }

    // Need to find pid of process that the menu belongs to. Can't use the
    // pid of the popup menu, since that's a shared/reused system window.
    // Instead, we assume that since the menu is present, it belongs to the
    // current foreground thread, which is what GetGUIThreadInfo(NULL) gets us.
    GUITHREADINFO	GuiThreadInfo;
    if( ! MyGetGUIThreadInfo( NULL, & GuiThreadInfo ) )
        return E_FAIL;
    DWORD dwPid = 0;
    GetWindowThreadProcessId( GuiThreadInfo.hwndActive, & dwPid );
    if( dwPid == 0 )
        return E_FAIL;

    MakeHmenuKey( pKeyData, dwPid, m_hMenu, dwIDChild );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HMENUKEYSIZE;

    return S_OK;
}









// ==========================================================================
//
//  POPUP WINDOW FRAMES 
//
// ==========================================================================

// --------------------------------------------------------------------------
//
//  CreateMenuPopupWindow()
//
//  This creates a child object that represents the Window object for a 
//  popup menu. It has no members, but has one child (a cMenuPopup)
//
// --------------------------------------------------------------------------
HRESULT CreateMenuPopupWindow(HWND hwnd, long idChildCur, REFIID riid, void** ppvMenuPopupW)
{
CMenuPopupFrame*    pPopupFrame;
HRESULT             hr;

    InitPv(ppvMenuPopupW);

    pPopupFrame = new CMenuPopupFrame(hwnd,idChildCur);
    if (!pPopupFrame)
        return(E_OUTOFMEMORY);

    hr = pPopupFrame->QueryInterface(riid, ppvMenuPopupW);
    if (!SUCCEEDED(hr))
        delete pPopupFrame;

    return(hr);
}


// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::CMenuPopupFrame()
//
// --------------------------------------------------------------------------
CMenuPopupFrame::CMenuPopupFrame(HWND hwnd,long idChildCur)
    : CWindow( CLASS_MenuPopupWindow )
{

    Initialize(hwnd, idChildCur);
    m_hMenu = NULL;
    m_ItemID = 0;
    m_hwndParent = NULL;
    m_fSonOfPopup = 0;
    m_fSysMenu = 0;

	m_hMenu = (HMENU)SendMessage (m_hwnd,MN_GETHMENU,0,0);
	m_ItemID = FindItemIDThatOwnsThisMenu (m_hMenu,&m_hwndParent,
        &m_fSonOfPopup,&m_fSysMenu);
}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::~CMenuPopupFrame()
//
// --------------------------------------------------------------------------
CMenuPopupFrame::~CMenuPopupFrame()
{
}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::SetupChildren()
//
//  Frames have 1 child. That one child is the CMenuPopup.
//
// --------------------------------------------------------------------------
void CMenuPopupFrame::SetupChildren(void)
{
	m_cChildren = 1;
}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::get_accParent()
//
//  Parent of a popupmenuframe is the CMenuItem that created it (if any).
//  To create one of those we need the grandparent. So we will create the 
//  grandparent (either a CMenuPopup, or a CMenu) temporarily, then we will 
//  create our parent CMenuItem based on that.
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopupFrame::get_accParent(IDispatch** ppdispParent)
{
IAccessible* paccGrandParent;
HRESULT      hr;
CMenu*       pMenu;
CMenuPopup*  pMenuPopup;

    InitPv(ppdispParent);

    if (m_fSonOfPopup)
    {
        hr = CreateMenuPopupClient(m_hwndParent,0,IID_IAccessible,(void**)&paccGrandParent);
        if (SUCCEEDED(hr))
        {
            pMenuPopup = (CMenuPopup*)paccGrandParent;
            hr = CreateMenuItem (paccGrandParent,   //	paccMenu	IN		pointer to the parent's IAccessible
                                 m_hwndParent,      //	hwnd		IN		the hwnd of the window that owns the parent menu
                                 pMenuPopup->GetMenu(), //	hMenu		IN		the hmenu of the menu that owns this item.
                                 m_hMenu,           //	hSubMenu	IN		the hMenu of the submenu this menu item opens
                                 m_ItemID,          //	ItemID		IN		the menu item ID. Position (1..n).
                                 0,                 //	iCurChild	IN		ID of the current child in the enumeration
                                 m_fSonOfPopup,     //	fPopup		IN		is this menu item in a popup or on a menu bar?
                                 IID_IDispatch,     //	riid		IN		what interface are we asking for on this item?
                                 (void**)ppdispParent); //	ppvItem		OUT		the pointer to the interface asked for.
            paccGrandParent->Release();
            return (hr);
        }
    }
    else if (m_fSysMenu)
    {
        hr = CreateSysMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccGrandParent);
        if (SUCCEEDED(hr))
        {
            pMenu = (CMenu*)paccGrandParent;
            pMenu->SetupChildren();
            hr = CreateMenuItem (paccGrandParent,   //	paccMenu	IN		pointer to the parent's IAccessible
                                 m_hwndParent,      //	hwnd		IN		the hwnd of the window that owns the parent menu
                                 pMenu->GetMenu(), //	hMenu		IN		the hmenu of the menu that owns this item.
                                 m_hMenu,           //	hSubMenu	IN		the hMenu of the submenu this menu item opens
                                 m_ItemID,          //	ItemID		IN		the menu item ID. Position (1..n).
                                 0,                 //	iCurChild	IN		ID of the current child in the enumeration
                                 m_fSonOfPopup,     //	fPopup		IN		is this menu item in a popup or on a menu bar?
                                 IID_IDispatch,     //	riid		IN		what interface are we asking for on this item?
                                 (void**)ppdispParent); //	ppvItem		OUT		the pointer to the interface asked for.
            paccGrandParent->Release();
            return (hr);
        }
    }
    else
    {
        hr = CreateMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccGrandParent);
        if (SUCCEEDED(hr))
        {
            pMenu = (CMenu*)paccGrandParent;
            pMenu->SetupChildren();
            hr = CreateMenuItem (paccGrandParent,   //	paccMenu	IN		pointer to the parent's IAccessible
                                 m_hwndParent,      //	hwnd		IN		the hwnd of the window that owns the parent menu
                                 pMenu->GetMenu(), //	hMenu		IN		the hmenu of the menu that owns this item.
                                 m_hMenu,           //	hSubMenu	IN		the hMenu of the submenu this menu item opens
                                 m_ItemID,          //	ItemID		IN		the menu item ID. Position (1..n).
                                 0,                 //	iCurChild	IN		ID of the current child in the enumeration
                                 m_fSonOfPopup,     //	fPopup		IN		is this menu item in a popup or on a menu bar?
                                 IID_IDispatch,     //	riid		IN		what interface are we asking for on this item?
                                 (void**)ppdispParent); //	ppvItem		OUT		the pointer to the interface asked for.
            paccGrandParent->Release();
            return (hr);
        }
    }
    return (hr);
}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::get_accChild()
//
//	What we want this do do is return (in ppdisp) an IDispatch pointer to 
//  the child specified by varChild. The 1 child of a CMenuPopupFrame is
//  a cMenuPopup.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopupFrame::get_accChild(VARIANT varChild, IDispatch** ppdisp)
{
    InitPv(ppdisp);

    if (!ValidateChild(&varChild) || varChild.lVal == CHILDID_SELF)
        return(E_INVALIDARG);

    return (CreateMenuPopupClient(m_hwnd, 0,IID_IDispatch, (void**)ppdisp));
}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::get_accName
//
//  Has very similar logic to CMenuPopup::get_accName
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopupFrame::get_accName(VARIANT varChild, BSTR* pszName)
{
HWND            hwndOwner;
TCHAR           szClassName[50];
HRESULT         hr;
IAccessible*    paccParent;

    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
    {
        // If we popped up from a menu bar or as another popup,
		// then our name is the name of the thing that popped us
		// up. If we are a floating popup, then our name is...?
		//
        // We implement this by either:
        // 1. creating a parent object on the fly that we can 
        //    ask the name of, OR
		// 2. Looking for the name of the owner window, OR
        // 3. Checking if we are the child of the start button. 
        // If all else fails, we'll just call ourselves "context menu".
        if (m_hwndParent && m_ItemID)
        {
            varChild.vt = VT_I4;
            varChild.lVal = m_ItemID;

            if (m_fSonOfPopup)
                hr = CreateMenuPopupClient(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            else if (m_fSysMenu)
                hr = CreateSysMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            else
                hr = CreateMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            if (SUCCEEDED(hr))
            {
                hr = paccParent->get_accName (varChild,pszName);
                paccParent->Release();
            }
            return (hr);
        }
        else
		{
			// Try to get the owner window and use that for a name
			// This doesn't seem to work on anything I have ever found,
            // but it should work if anything has an owner, so i'll
            //leave it in. If it starts breaking, just rip it out.
			if (m_hwnd)
			{
			IAccessible*	pacc;
			HRESULT			hr;

				hwndOwner = ::GetWindow (m_hwnd,GW_OWNER);
				hr = AccessibleObjectFromWindow (hwndOwner, OBJID_WINDOW, IID_IAccessible, (void**)&pacc);
				if (SUCCEEDED(hr))
				{
					hr = pacc->get_accName(varChild,pszName);
                    pacc->Release();
					if (SUCCEEDED(hr))
						return (hr);
				}
			}
			// check if the start button has focus
			hwndOwner = MyGetFocus();
			if (InTheShell(hwndOwner, SHELL_TRAY))
			{
                GetClassName(hwndOwner,szClassName,ARRAYSIZE(szClassName));
                if (lstrcmp(szClassName,TEXT("Button")) == 0)
                    return (HrCreateString(STR_STARTBUTTON,pszName));
			}
			// at least return this for a name
            return (HrCreateString (STR_CONTEXT_MENU,pszName));
		} // end else we don't have m_paccparent && m_itemid
    } // end if childid_self
    else
    {
        // not asking for name of the menupopupframe itself. We do not support asking for
        // name of our child - have to talk to the child itself
        return (E_INVALIDARG);
    }

}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::accHitTest()
//
//  We just need to return VARIANT with var.pDispVal set to be our one child, 
//  the CMenuPopup.
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopupFrame::accHitTest(long x, long y, VARIANT* pvarHit)
{
IDispatch*  pdispChild;
HRESULT     hr;

    InitPvar(pvarHit);
    SetupChildren();

    pvarHit->vt = VT_I4;
    pvarHit->lVal = CHILDID_SELF;

    if (SendMessage(m_hwnd, WM_NCHITTEST, 0, MAKELONG(x, y)) == HTCLIENT)
    {
        hr = CreateMenuPopupClient (m_hwnd,0,IID_IDispatch,(void**)&pdispChild);
        if (SUCCEEDED (hr))
        {
            pvarHit->vt = VT_DISPATCH;
            pvarHit->pdispVal = pdispChild;
        }
        return(hr);
    }

    return(S_OK);
}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::get_accFocus()
//
// This fills in pvarFocus with the child that has the focus.
// Since we only have one child, We'll return an IDispatch to that child.
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopupFrame::get_accFocus(VARIANT* pvarFocus)
{
HRESULT     hr;
IDispatch*  pdispChild;

    InitPvar(pvarFocus);
    hr = CreateMenuPopupClient(m_hwnd, 0,IID_IDispatch, (void**)&pdispChild);
    if (!SUCCEEDED(hr))
        return (hr);

    pvarFocus->vt = VT_DISPATCH;
    pvarFocus->pdispVal = pdispChild;
    return (S_OK);
}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::accLocation()
//
//  Location of Self and Child is the same.
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopupFrame::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(CWindow::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));
}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::accNavigate()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopupFrame::accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd)
{
    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD || dwNavDir == NAVDIR_LASTCHILD)
    {
        pvarEnd->vt = VT_DISPATCH;
        return (CreateMenuPopupClient (m_hwnd,0,IID_IDispatch, (void**)&(pvarEnd->pdispVal)));
    }

    return (S_FALSE);
}
                                          
// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::Clone()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopupFrame::Clone(IEnumVARIANT **ppenum)
{
    return (CreateMenuPopupWindow(m_hwnd, m_idChildCur, IID_IEnumVARIANT,
        (void**)ppenum));
}

// --------------------------------------------------------------------------
//
//  CMenuPopupFrame::Next()
//
// --------------------------------------------------------------------------
STDMETHODIMP CMenuPopupFrame::Next(ULONG celt, VARIANT* rgvar, ULONG* pceltFetched)
{
    VARIANT* pvar;
    long    cFetched;

    // Can be NULL
    if (pceltFetched)
        *pceltFetched = 0;

    pvar = rgvar;
    cFetched = 0;

    // we only have one child, so we can only return it if m_idChildCur == 0
    if (m_idChildCur == 0)
    {
        cFetched++;
        m_idChildCur++;
        pvar->vt = VT_DISPATCH;
        CreateMenuPopupClient (m_hwnd,0,IID_IDispatch, (void**)&(pvar->pdispVal));
    }

    //
    // Fill in the number fetched
    //
    if (pceltFetched)
        *pceltFetched = cFetched;

    //
    // Return S_FALSE if we grabbed fewer items than requested
    //
    return((cFetched < (long)celt) ? S_FALSE : S_OK);
}



// --------------------------------------------------------------------------
//
// This is a private function used to get the window handle that contains
// a given hSubMenu.
//
// --------------------------------------------------------------------------

HWND GetSubMenuWindow (HMENU hSubMenuToFind)
{
HWND    hwndSubMenu;
BOOL    bFound;
HMENU   hSubMenuTemp;

    hwndSubMenu = FindWindow (TEXT("#32768"),NULL);
    if (hwndSubMenu == NULL)
        return (NULL);    // random error condition - shouldn't happen
    
    if (!IsWindowVisible(hwndSubMenu))
        return (NULL);

    bFound = FALSE;
    while (hwndSubMenu)
    {
        hSubMenuTemp = (HMENU)SendMessage (hwndSubMenu,MN_GETHMENU,0,0);
        if (hSubMenuTemp == hSubMenuToFind)
        {
            bFound = TRUE;
            break;
        }
        hwndSubMenu = FindWindowEx (NULL,hwndSubMenu,TEXT("#32768"),NULL);
    } // end while hwndSubMenu

    if (bFound)
    {
        return(hwndSubMenu);
    }
    return (NULL);
}



// --------------------------------------------------------------------------
// This looks at each item in the Active window's menu and any other menu 
// windows, until it finds one that has an hSubMenu that matches the hMenu 
// we are trying to find. It then returns the ID of that thing (1..n) and
// fills in the window handle of the owner, and whether that window is a top
// level window or a popup menu.
// --------------------------------------------------------------------------
long FindItemIDThatOwnsThisMenu (HMENU hMenuOwned,HWND* phwndOwner,
                                 BOOL* pfPopup,BOOL *pfSysMenu)
{
HWND            hwndMenu;
HMENU           hMenu;
int             cItems;
int             i;

    if (IsBadWritePtr(phwndOwner,sizeof(HWND*)) || 
        IsBadWritePtr (pfPopup,sizeof(BOOL*))   ||
        IsBadWritePtr (pfSysMenu,sizeof(BOOL*)))
        return 0;

    *pfPopup = FALSE;
    *pfSysMenu = FALSE;
    *phwndOwner = NULL;

    GUITHREADINFO	GuiThreadInfo;
    if( ! MyGetGUIThreadInfo (NULL,&GuiThreadInfo) )
        return 0;
    
    // check if it is from the sys menu first
    MENUBARINFO     mbi;
    if( MyGetMenuBarInfo(GuiThreadInfo.hwndActive, OBJID_SYSMENU, 0, &mbi) 
        && mbi.hMenu != NULL )
    {
        hMenu = mbi.hMenu;

        if (GetSubMenu(hMenu,0) == hMenuOwned)
        {
            *pfSysMenu = TRUE;
            *pfPopup = FALSE;
            *phwndOwner = GuiThreadInfo.hwndActive;
            return (1);
        }
    }

    // if not from the sys menu, check the window's menu bar
    hMenu = GetMenu (GuiThreadInfo.hwndActive);
    if (hMenu)
    {
        cItems = GetMenuItemCount (hMenu);
        for (i=0;i<cItems;i++)
        {
#ifdef _DEBUG
            if (!hMenu)
            {
                //DBPRINTF ("null hmenu at 5\r\n");
                Assert (hMenu);
            }
#endif

            if (GetSubMenu(hMenu,i) == hMenuOwned)
            {
                *pfPopup = FALSE;
                *phwndOwner = GuiThreadInfo.hwndActive;
                return (i+1);
            }
        }
    }

	// Okay, it doesn't belong to the active window's menu bar, maybe
	// it belongs to a submenu of that...
    hwndMenu = FindWindow (TEXT("#32768"),NULL);
    while (hwndMenu)
    {
        hMenu = (HMENU)SendMessage (hwndMenu,MN_GETHMENU,0,0);
        if (hMenu)
        {
            cItems = GetMenuItemCount (hMenu);
            for (i=0;i<cItems;i++)
            {
                if (GetSubMenu(hMenu,i) == hMenuOwned)
			    {
                    *pfPopup = TRUE;
                    *phwndOwner = hwndMenu;
                    return (i+1);
                }
            }
        }
        hwndMenu = FindWindowEx (NULL,hwndMenu,TEXT("#32768"),NULL);
    } // end while hwndMenu
	
	// if we still haven't returned, then this menu is either a context
	// menu, or belongs to the start button
	return 0;
}



// --------------------------------------------------------------------------
//
//  INTERNAL
//  MyGetMenuString()
//
//  This tries to get the text of menu items.  If they are ownerdraw, this
//  will hack around in shell structures to get the text.
//
//  Parameters:
//  hwnd            IN  the hwnd that owns the menu 
//  hMenu           IN  the hMenu to talk to
//  id              IN  the ID of the item to get (1..n)
//  fShell          IN  TRUE if this is a shell owned menu - tells the function
//                      to hack into the shell's memory
//  lpszBuf         OUT gets filled in with the string
//  cchMax          IN  number of characters in lpszBuf
//  fAllowGenerated IN  If TRUE, generated names (eg. those for MDI items)
//                      are allowed.
//
//  Returns:
//  TRUE if string was filled in, FALSE otherwise
//
//  Returns:
//  S_OK if string returned, S_FALSE for missing string, COM error code
//  otherwise.
//
//  'Generated names' are menu item names that do not actually correspond
//  to text on the menu item - eg. "Close"/"Restore" for the MDI buttons.
//  Typically you do want this text if you are looking for the name, but you
//  do not want this text if you are looking to extract a shortcut key.
// --------------------------------------------------------------------------

BOOL MyGetMenuString( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, long id, 
                      LPTSTR lpszBuf, UINT cchMax, BOOL fAllowGenerated )
{
    --id;
    *lpszBuf = 0;

    MENUITEMINFO mii;
    mii.cbSize = SIZEOF_MENUITEMINFO;
    mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_DATA | MIIM_ID;
    mii.dwTypeData = NULL;
    mii.cch = 0;

    if (!GetMenuItemInfo(hMenu, id, TRUE, &mii))
        return FALSE;

    // Is this a separator?  If so, bail.
    if (mii.fType & MFT_SEPARATOR)
        return FALSE;

    // For MDI windows - the min/restore/close buttons of the child window are
    // actually menu items. Have to check for them here...
    if( fAllowGenerated && hwnd && GetMDIMenuString( hwnd, hMenu, id, lpszBuf, cchMax ) )
        return TRUE;

    // If it's owner-draw, check if it supports the the 'dwData is ptr to
    // MSAA data' workaround.
    if( ( mii.fType & MFT_OWNERDRAW )
        && TryMSAAMenuHack( pTheObj, hwnd, mii.dwItemData, lpszBuf, cchMax ) )
        return TRUE;

    // Try to get the text.
    //
    // GetMenuString does the right thing for Win95/NT close/min/max
    // system menu items - whereas mii.dwTypeData only contains a bitmap
    // number (apparently - although cchSize does give the character
    // count as though it were a string...)
    if (GetMenuString(hMenu, id, lpszBuf, cchMax, MF_BYPOSITION))
        return TRUE;

    // is this a shell owner menu item?
    // Check that (a) this is a shell menu, (b) it's ownerdraw, and
    // (c) it has non-0 ownerdraw itemdata,
    if ( InTheShell( hwnd, SHELL_PROCESS ) && ( mii.fType & MFT_OWNERDRAW ) && mii.dwItemData )
    {
        if( GetShellOwnerDrawMenu( hwnd, mii.dwItemData, lpszBuf, cchMax ) )
            return TRUE;
    }

    // We've tried everything, but didn't get any name...
    return FALSE;
}




// --------------------------------------------------------------------------
//
//  INTERNAL
//  GetShellOwnerDrawMenu
//
//  Pull text out of shell's internal data structs.
//  This is used by the SendTo menu. (Also used to be used by the old
//  pre-IE4 start menus, before they changed to being Toolbar32 based.)
//
//	Parameters:
//	hwnd	IN		hwnd of menu
//	dwItemData	IN	dwItemData from the menu
//	lpszBuf	IN/OUT	gets filled in with the string
//	cchMax	in		number of characters in lpszBuf
//
//	Returns:
//	TRUE if string was filled in, FALSE otherwise
// --------------------------------------------------------------------------

BOOL GetShellOwnerDrawMenu( HWND hwnd, DWORD_PTR dwItemData, LPTSTR lpszBuf, UINT cchMax )
{
    DWORD idProcess = 0;
    GetWindowThreadProcessId(hwnd, &idProcess);
    if (!idProcess)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, idProcess);
    if (!hProcess)
        return FALSE;

    //
    // Try to read FILEMENUITEM's worth of stuff.
    //
    FILEMENUITEM  fmi;
    SIZE_T        cbRead;
    if (ReadProcessMemory(hProcess, (LPCVOID)dwItemData, &fmi, sizeof(fmi), &cbRead) &&
        (cbRead == sizeof(fmi)))
    {
        //
        // Is there a string pointer in here?
        //
        if (fmi.psz)
        {
            ReadProcessMemory(hProcess, fmi.psz, lpszBuf, cchMax, &cbRead);
            lpszBuf[cchMax-1] = 0;
        }
        else if (fmi.pidl)
        {
            ITEMIDLIST id;

            // No, we have to grovel inside of the PIDL
            if (ReadProcessMemory(hProcess, fmi.pidl, &id, sizeof(ITEMIDLIST), &cbRead) &&
                (cbRead == sizeof(id)))
            {
                id.cbTotal -= OFFSET_SZFRIENDLYNAME;
                cchMax = min((DWORD)id.cbTotal, cchMax);
                cchMax = max(cchMax, 1);

                ReadProcessMemory(hProcess, (LPBYTE)fmi.pidl + OFFSET_SZFRIENDLYNAME,
                    lpszBuf, cchMax, &cbRead);
                lpszBuf[cchMax-1] = 0;

                //
                // Are the last 4 characters ".lnk"? or ".pif"?
                // or .??? - we'll cut 'em all off.
                //
                cchMax = lstrlen(lpszBuf);
                if ((cchMax >= 4) && (lpszBuf[cchMax-4] == '.'))
                    lpszBuf[cchMax-4] = 0;
            }
        }
    }

    CloseHandle(hProcess);

    return *lpszBuf != 0;
}





// --------------------------------------------------------------------------
//
//  INTERNAL
//  GetMDIButtonIndex
//
//  Returns appropritate INDEX_TITLEBAR_nnn for the given menu item, if it
//  is really an MDI child button (restore/minimize/close).
//
//  Returns 0 otherwise.
//
// --------------------------------------------------------------------------


UINT GetMDIButtonIndex( HMENU hMenu, DWORD idPos )
{
    switch( GetMenuItemID( hMenu, idPos ) )
    {
        case SC_MINIMIZE:   return INDEX_TITLEBAR_MINBUTTON;
        case SC_RESTORE:    return INDEX_TITLEBAR_RESTOREBUTTON;
        case SC_CLOSE:      return INDEX_TITLEBAR_CLOSEBUTTON;
        default:            return 0; // INDEX_TITLEBAR_SELF
    }
}


// --------------------------------------------------------------------------
//
//  INTERNAL
//  GetMDIChildMenuString
//
//  Check if this is a MDI menu - return strings for the document menu,
//  and the min/restore/close buttons
//
//	Parameters:
//	hmenu   IN	    handle of menu
//	id      IN	    0-based index of the menu item
//	lpszBuf	IN/OUT	gets filled in with the string
//	cchMax	in		number of characters in lpszBuf
//
//	Returns:
//	TRUE if string was filled in, FALSE otherwise
// --------------------------------------------------------------------------

BOOL GetMDIMenuString( HWND hwnd, HMENU hMenu, DWORD idPos, LPTSTR lpszBuf, UINT cchMax )
{
    // For MDI windows - the min/restore/close buttons of the child window are
    // actually owner-draw menu items. Have to check for them here...

    UINT iIndex = GetMDIButtonIndex( hMenu, idPos );
    if( iIndex )
    {
        return LoadString( hinstResDll, iIndex + STR_TITLEBAR_NAME, lpszBuf, cchMax ) != 0;
    }

    // Detect the document system menu by checking that it has a submenu
    // which contains the Restore item (GetMenuState returns -1 if not found...)
    // but which is not the actual system menu (since that also has a restore item).
    HMENU hSub = GetSubMenu( hMenu, idPos );
    if( hSub && GetMenuState( hSub, SC_RESTORE, MF_BYCOMMAND ) != -1
        && hSub != MyGetSystemMenu( hwnd ) )
    {
        return LoadString( hinstResDll, STR_DOCMENU_NAME, lpszBuf, cchMax ) != 0;
    }

    return FALSE;
}




// --------------------------------------------------------------------------
//
//  INTERNAL
//  GetMDIMenuDescriptionString
//
//  Check if this is a MDI menu - return description strings for the document
//  menu, and the min/restore/close buttons
//
//	Parameters:
//	hmenu   IN	    handle of menu
//	idPos   IN	    0-based index of the menu item
//	pbstr   OUT     returns the description of the item
//
//	Returns:
//	TRUE if item is a MDI element and pbstr was set, FALSE otherwise.
// --------------------------------------------------------------------------

BOOL GetMDIMenuDescriptionString( HMENU hMenu, DWORD idPos, BSTR * pbstr )
{
    UINT iIndex = GetMDIButtonIndex( hMenu, idPos );
    if( iIndex )
    {
        return HrCreateString( iIndex + STR_TITLEBAR_DESCRIPTION, pbstr ) == S_OK;
    }
    else
    {
        return FALSE;
    }
}


// --------------------------------------------------------------------------
//
//  INTERNAL
//  TryMSAAMenuHack()
//
//  Checks if a menu supports the 'dwData is ptr to MSAA data' workaround.
//
//	Parameters:
//	pTheObj	IN		the hwnd that owns the menu (used to get window handle
//                  if hWnd is NULL)
//	hWnd	IN		hwnd of menu, NULL if not known (eg. invisible 'fake'
//                  popups)
//	dwItemData	IN	dwItemData from the menu
//	lpszBuf	IN/OUT	gets filled in with the string
//	cchMax	in		number of characters in lpszBuf
//
//	Returns:
//	TRUE if string was filled in, FALSE otherwise
// --------------------------------------------------------------------------

BOOL TryMSAAMenuHack( IAccessible *  pTheObj,
                      HWND           hWnd,
                      DWORD_PTR      dwItemData,
                      LPTSTR         lpszBuf,
                      UINT           cchMax )
{
    BOOL bGotIt = FALSE;

    if( ! hWnd )
    {
        // It's an invisible 'fake' popup menu (CPopuMenu created to expose
        // a HMENU, but no menu, and therefore no popup window, is currenly
        // visible).
        // Need a window handle so we can get the process id...
        if( WindowFromAccessibleObjectEx( pTheObj, & hWnd ) != S_OK || hWnd == NULL )
            return FALSE;
    }

    // ...now get process id...
    DWORD idProcess = 0;
    GetWindowThreadProcessId( hWnd, &idProcess );
    if( !idProcess )
        return FALSE;

    // Open that process so we can read its memory...
    HANDLE hProcess = OpenProcess( PROCESS_VM_READ, FALSE, idProcess );
    if( hProcess )
    {
        // Treat dwItemData as an address, and try to read a
        // MSAAMENUINFO struct from there...
        MSAAMENUINFO menuinfo;
        SIZE_T cbRead;

        if( ReadProcessMemory( hProcess, (LPCVOID)dwItemData, (LPVOID) & menuinfo, sizeof( menuinfo ), &cbRead ) 
            && ( cbRead == sizeof( menuinfo ) ) )
        {

            // Check signature...
            if( menuinfo.dwMSAASignature == MSAA_MENU_SIG )
            {
                // Work out len of UNICODE string to copy (+1 for terminating NUL)
                DWORD copyLen = ( menuinfo.cchWText + 1 ) * sizeof( WCHAR );

                WCHAR * pAlloc = (LPWSTR) LocalAlloc( LPTR, copyLen );
                if( pAlloc )
                {

                    // Do the copy... also fail if we read less than expected, or terminating NUL missing...
                    if( ReadProcessMemory( hProcess, (LPCVOID)menuinfo.pszWText, pAlloc, copyLen, &cbRead ) 
                            && ( cbRead == copyLen )
                            && ( pAlloc[ menuinfo.cchWText ] == '\0' ) )
                    {

#ifdef UNICODE
						// Copy text to output buffer...
						if( cchMax > 0 )
						{
							UINT cchCopy = menuinfo.cchWText;
							if( cchCopy > cchMax - 1 )
								cchCopy = cchMax - 1; // -1 for terminating NUL
							memcpy( lpszBuf, pAlloc, cchCopy * sizeof( TCHAR ) );
							lpszBuf[ cchCopy ] = L'\0';
							bGotIt = TRUE;
						}
#else
                        // Convert (and copy) UNICODE to ANSI...
                        if( WideCharToMultiByte( CP_ACP, 0, pAlloc, -1, lpszBuf, cchMax, NULL, NULL ) != 0 )
                        {
                            bGotIt = TRUE;
                        }
#endif
                    }

                    LocalFree( pAlloc );
                } // pAlloc
            } // m_Signature
        } // ReadProcessMemory

        CloseHandle( hProcess );
    } // hProcess

    return bGotIt;
}






// --------------------------------------------------------------------------
//
//  WindowFromAccessibleObjectEx()
//
//  This walks UP the ancestor chain until we find something who responds to
//  IOleWindow().  Then we get the HWND from it.
//
//  This is effectively a local version of WindowFromAccessibleObject
//  This version doesn't stop till it runs out of objects, it gets a valid
//  hwnd. The non-ex version stops even if it getgs a NULL hwnd.
//  This allows us to navigate up through menupopups which have no hwnd
//  (return NULL), but which do have parents, which eventually leads us to
//  the owning hWnd.
//
// --------------------------------------------------------------------------
STDAPI WindowFromAccessibleObjectEx( IAccessible* pacc, HWND* phwnd )
{
IAccessible* paccT;
IOleWindow* polewnd;
IDispatch* pdispParent;
HRESULT     hr;

    //CWO: 12/4/96, Added check for NULL object
    //CWO: 12/13/96, Removed NULL check, replaced with IsBadReadPtr check (#10342)
    if (IsBadWritePtr(phwnd,sizeof(HWND*)) || IsBadReadPtr(pacc, sizeof(void*)))
        return (E_INVALIDARG);

    *phwnd = NULL;
    paccT = pacc;
    hr = S_OK;

    while (paccT && SUCCEEDED(hr))
    {
        polewnd = NULL;
        hr = paccT->QueryInterface(IID_IOleWindow, (void**)&polewnd);
        if (SUCCEEDED(hr) && polewnd)
        {
            hr = polewnd->GetWindow(phwnd);
            polewnd->Release();

            // Don't quit if we just got a NULL hwnd...
            // (this is the only change from WindowFromAccessibleObject(), which
            // just unconditionally returned when it got here...)
            if( *phwnd != NULL )
            {
                //
                // Release an interface we obtained on our own, but not the one
                // passed in.
                //
                if (paccT != pacc)
                {
                    paccT->Release();
                    paccT = NULL;
                }
                break;
            }
        }

        //
        // Get our parent.
        //
        pdispParent = NULL;
        hr = paccT->get_accParent(&pdispParent);

        //
        // Release an interface we obtained on our own, but not the one
        // passed in.
        //
        if (paccT != pacc)
        {
            paccT->Release();
        }

        paccT = NULL;

        if (SUCCEEDED(hr) && pdispParent)
        {
            hr = pdispParent->QueryInterface(IID_IAccessible, (void**)&paccT);
            pdispParent->Release();
        }
    }

    return(hr);
}






// --------------------------------------------------------------------------
//
//  INTERNAL
//  GetMenuItemName()
//
//  Returns the BSTR name for a menu item.
//
//  Parameters:
//  pTheObj         IN      the hwnd that owns the menu (used to get window
//                          handle if hWnd is NULL)
//  hWnd            IN      hwnd of menu, NULL if not known (eg. invisible
//                          'fake' popups)
//  hMenu           IN      Menu handle
//  id              IN      1-based id of the menu item (idChild)
//  pszName         OUT     returns string containing text for menu item.
//
//  Returns:
//  S_OK if string returned, S_FALSE for missing string, COM error code
//  otherwise.
//
//  Catches special cases " " and "-", which get mapped to "System" and
//  "Document window" respectively. These are the top-level and child window
//  system menus.
//
// --------------------------------------------------------------------------

HRESULT GetMenuItemName( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id, BSTR * pszName )
{
    Assert( hMenu );

    TCHAR szItemName[256];

    // TRUE -> allow generated names (eg. for MDI buttons)
    if( MyGetMenuString( pTheObj, hwnd, hMenu, id, szItemName, ARRAYSIZE( szItemName ), TRUE ) )
    {
        StripMnemonic( szItemName );

        if( lstrcmp( szItemName, TEXT(" ") ) == 0 )
        {
            return HrCreateString( STR_SYSMENU_NAME, pszName ); // "System"
        }

        if( lstrcmp( szItemName, TEXT("-") ) == 0 )
        {
            return HrCreateString( STR_DOCMENU_NAME, pszName ); // "Document window"
        }

        *pszName = TCharSysAllocString(szItemName);
        if( ! *pszName )
            return E_OUTOFMEMORY;

        return S_OK;
    }
    else
    {
        *szItemName = '\0';
        *pszName = NULL;
        return S_FALSE;
    }
}




// --------------------------------------------------------------------------
//
//  INTERNAL
//  GetMenuItemShortcut()
//
//  Returns the BSTR name for a menu shortcut.
//
//  Parameters:
//  pTheObj     IN      the hwnd that owns the menu (used to get window
//                      handle if hWnd is NULL)
//  hWnd        IN      hwnd of menu, NULL if not known (eg. invisible
//                      'fake' popups)
//  hMenu       IN      Menu handle
//  id          IN      1-based id of the menu item (idChild)
//  fIsMenuBar  IN      TRUE if the menu is a menubar; false for a popup
//  pszShortcut OUT     returns string containing kbshortcut for menu item.
//
//  Returns:
//  S_OK if string returned, S_FALSE for missing string, COM error code
//  otherwise.
//
//  Catches special case " " for system menu.
//  If fMenuBar is TRUE, uses "Alt+%c" form.
//
// --------------------------------------------------------------------------

HRESULT GetMenuItemShortcut( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id,
                             BOOL fIsMenuBar, BSTR * pszShortcut )
{
    TCHAR szHotKey[32];
    szHotKey[0] = GetMenuItemHotkey( pTheObj, hwnd, hMenu, id,
                                     GMIH_ALLOW_INITIAL | GMIH_ALLOW_SYS_SPACE );
    szHotKey[1] = 0;

    if ( szHotKey[0] == ' ' )
    {
        // Expand space character ' ' to the string "Space"
        szHotKey[ 0 ] = '\0';
        LoadString( hinstResDll, STR_SYSMENU_KEY, szHotKey, ARRAYSIZE( szHotKey ) );
    }

    // Fall through... This gives us c -> Alt+c if it was a single char,
    // or ' ' -> "Space" -> "Alt+Space" if it was a space char (for sys menu).

    if ( *szHotKey )
    {
        // If this is a menu bar, use the ALT+ form...
        if ( fIsMenuBar )
        {
            // Make a string of the form "Alt+ch".
            return HrMakeShortcut( szHotKey, pszShortcut );
        }
        else
        {
            // otherwise use just the key
            *pszShortcut = TCharSysAllocString( szHotKey );
            if ( ! *pszShortcut )
            {
                return E_OUTOFMEMORY;
            }

            return S_OK;
        }
    }

    *pszShortcut = NULL;
    return S_FALSE;
}





// --------------------------------------------------------------------------
//
//  INTERNAL
//  GetMenuItemHotkey()
//
//  Returns the TCHAR hotkey for a menu, if one exists.
//
//  Parameters:
//  pTheObj     IN      the hwnd that owns the menu (used to get window
//                      handle if hWnd is NULL)
//  hWnd        IN      hwnd of menu, NULL if not known (eg. invisible
//                      'fake' popups)
//  hMenu       IN      Menu handle
//  id          IN      1-based id of the menu item (idChild)
//  fOption     IN      Option flags - see below.
//
//  Returns:
//  Hotkey character of menu item, or '\0' if item has no hotkey.
//
//  Options:
//
//    GMIH_ALLOW_INITIAL
//      If set, allows the initial character of the menu item string to be
//      returned as the shortcut key (provided that no other items also use
//      that key as their menonic.)
//
//    GMIH_ALLOW_SYS_SPACE   = 0x02
//      If set, returns ' ' as the shortcut key of the system menu item.
//
// --------------------------------------------------------------------------

TCHAR GetMenuItemHotkey( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id, DWORD fOptions )
{
    TCHAR szItemName[ 256 ];

    // FALSE -> disallow generated names (eg. for MDI buttons)
    if( ! MyGetMenuString( pTheObj, hwnd, hMenu, id, szItemName, ARRAYSIZE( szItemName ), FALSE ) )
    {
        return '\0';
    }

    // Check for menu name being " " - caller will want to treat this as a special hotkey of "Space".
    // (eg. givein "Alt+Space" as the entire hotkey string.)
    if( ( fOptions & GMIH_ALLOW_SYS_SPACE )
        && lstrcmp( szItemName, TEXT(" ") ) == 0 )
    {
        return ' ';
    }

    TCHAR ch = StripMnemonic( szItemName );

    // Can stop here if caller doesn't want initial chars (ie. &-mnemonics only)
    if( ! ( fOptions & GMIH_ALLOW_INITIAL ) )
    {
        return ch;
    }



    // Did we get a hot-key? If so, use it.
    if( ch != '\0' )
    {
        return ch;
    }

    // Try initial letter instead...
    LPTSTR pScanCh = szItemName;
    while( *pScanCh == ' ' )
    {
        *pScanCh++;
    }

    // Obscure USER32 menu thing - was used to right-justify Help items in a previous version?
    // Anyhow, USER skips over it to find the real first letter. So we do likewise...
    if( *pScanCh == '\x08' )
    {
        pScanCh++;
    }

    // Just in case there's no initial letter... (all spaces)
    if( *pScanCh == '\0' )
    {
        return '\0';
    }

    // Hotkeys are always returned as lowercase...
    CharLowerBuff( pScanCh, 1 );
    ch = *pScanCh;



    // Now compare against all other menu items - if one other item has this
    // initial letter as its mnemonic, then we can't use it for this one.
    // (Mnemonic takes precedence over initial letters)

    // GetMenuItemHotkey index is 1-based (ie. idChild), as is id, so also
    // using a 1-based index here.
    int cItems = GetMenuItemCount( hMenu );
    for( int iScan = 1 ; iScan <= cItems ; iScan++ )
    {
        // Don't compare against this item!
        if( iScan != id )
        {
            if( ch == GetMenuItemHotkey( pTheObj, hwnd, hMenu, iScan, NULL ) )
            {
                // Some other item has a mnemonic that is the same as this
                // item's initial char - it mnemonic takes precedence, so
                // this item has no kbshortcut.
                return '\0';
            }
        }
    }

    // No item use this item's initial char as a mnemonic - we're clear to use
    // it as this item's kbshortcut.
    return ch;
}


// --------------------------------------------------------------------------
//
//  INTERNAL
//  MyGetSystemMenu()
//
//  Returns the system HMENU for the given HWND.
//
//  Can't use the Win32 API GetSystemMenu, since that modifies the system
//  HMENU for the window.
//
// --------------------------------------------------------------------------

HMENU MyGetSystemMenu( HWND hwnd )
{
    MENUBARINFO mbi;
    if ( ! MyGetMenuBarInfo( hwnd, OBJID_SYSMENU, 0, &mbi ) )
    {
        return NULL;
    }
    // GetMenuBarInfo returns a menu containing the sysmenu as its only
    // submenu. Use GetSubMenu to access that...
    return GetSubMenu( mbi.hMenu, 0 );
}
