
/****************************** Module Header ******************************\
* Module Name: DDE.C (Extensible Compound Documents -DDE)
*
* Copyright (c) 1985 - 1991 Microsoft Corporation
*
* PURPOSE: Handles all API routines for the dde sub-dll of the ole dll.
*
* History:
*   Raor,Srinik  (../../90,91)  Designed and coded
*   curts created portable version for WIN16/32
*
\***************************************************************************/

#include <windows.h>
#include "dde.h"
#include "dll.h"

/* #define GRAPHBUG */


// ### may not need seperate wndproc for system topic!
HANDLE  GetDDEDataHandle (DDEDATA FAR *, UINT, HANDLE);

extern  ATOM        aSystem;
extern  ATOM        aOle;
extern  HANDLE      hInstDLL;


// DocWndProc: Window procedure used to document DDE conversations

LRESULT FAR PASCAL DocWndProc(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
){
    PEDIT_DDE   pedit = NULL;
    LPOBJECT_LE lpobj  = NULL;



    Puts("DocWndProc");

    if (lpobj  = (LPOBJECT_LE) GetWindowLongPtr (hwnd, 0))
    {
        pedit = lpobj->pDocEdit;
    }
    else
    {
        // Can't cope so just pass this message on
        DEBUG_OUT ("SYS: doc conv block missing",0);
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    switch (message){

        case WM_DDE_ACK:
#ifdef  FIREWALLS
        ASSERT (pedit, "Doc conv channel missing");
#endif
            DEBUG_OUT ("WM_DDE_ACK ", 0);
            if (pedit->bTerminating){
                // ### this error recovery may not be correct.
                DEBUG_OUT ("No action due to termination process",0)
                break;
            }

            switch(pedit->awaitAck){

                case AA_INITIATE:
                    HandleAckInitMsg (pedit, (HWND)wParam);
                    if (LOWORD(lParam))
                        GlobalDeleteAtom (LOWORD(lParam));
                    if (HIWORD(lParam))
                        GlobalDeleteAtom (HIWORD(lParam));
                    break;

                case AA_REQUEST:
                    Puts("Request");
                    HandleAck (lpobj, pedit, wParam, lParam);
                    break;

                case AA_UNADVISE:
                    Puts("Unadvise");
                    HandleAck (lpobj, pedit, wParam, lParam);
                    break;

                case AA_EXECUTE:
                    Puts("Execute");
                    HandleAck (lpobj, pedit, wParam, lParam);
                    break;

                case AA_ADVISE:
                    Puts("Advise");
                    HandleAck (lpobj, pedit, wParam, lParam);
                    break;

                case AA_POKE:

                    // freeing pokedata is done in handleack
                    Puts("Poke");
                    HandleAck (lpobj, pedit, wParam, lParam);
                    break;

                default:
                    DEBUG_OUT ("received ACK We don't know how to handle ",0)
                    break;

            } // end of switch
            break;

        case WM_TIMER:
#ifdef  FIREWALLS
        ASSERT (pedit, "Doc conv channel missing");
#endif
            HandleTimerMsg (lpobj, pedit);
            break;

        case WM_DDE_DATA:
#ifdef  FIREWALLS
        ASSERT (pedit, "Doc conv channel missing");
#endif
            DEBUG_OUT ("WM_DDE_DATA",0);
            HandleDataMsg (lpobj, GET_WM_DDE_DATA_HDATA(wParam,lParam),
                                  GET_WM_DDE_DATA_ITEM(wParam,lParam));
            DDEFREE(message, lParam);
            break;

        case WM_DDE_TERMINATE:

#ifdef  FIREWALLS
        ASSERT (pedit, "Doc conv channel missing");
#endif
            DEBUG_OUT ("WM_DDE_TERMINATE",0);
            HandleTermMsg (lpobj, pedit, (HWND)wParam, TRUE);
            break;

        case WM_DESTROY:

#ifdef  FIREWALLS
        ASSERT (pedit, "Doc conv channel missing");
#endif
            DEBUG_OUT ("Client window being destroyed", 0)
            pedit->hClient = NULL;
            break;

        default:
            return DefWindowProc (hwnd, message, wParam, lParam);

    }
    return 0L;
}



// SrvrWndProc: Window Procedure for System Topic DDE conversations
// wndproc for system topic

LRESULT FAR PASCAL SrvrWndProc(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
){
    PEDIT_DDE   pedit = NULL;
    LPOBJECT_LE lpobj  = NULL;

    Puts("SysWndProc");

    if (lpobj  = (LPOBJECT_LE) GetWindowLongPtr (hwnd, 0))
    {
        pedit = lpobj->pSysEdit;
    }
    else
    {
        // Can't cope so just pass this message on
        DEBUG_OUT ("SYS: conv edit block missing",0);
        return DefWindowProc (hwnd, message, wParam, lParam);
    }


    switch (message){

       case WM_DDE_ACK:

#ifdef  FIREWALLS
            ASSERT (pedit, "sys conv edit block missing");
#endif

            DEBUG_OUT ("SYS: WM_DDE_ACK",0);

            if(pedit->bTerminating){
                //### Error recovery may not be OK.
                DEBUG_OUT ("No action due to termination process",0)
                break;
            }

            switch (pedit->awaitAck) {


                case AA_INITIATE:

#ifdef      HISTORY
                    if (GETWINDOWUINT((HWND)wParam, GWW_HINSTANCE) == pedit->hInst ||
                            IsSrvrDLLwnd ((HWND)wParam, pedit->hInst)) {
                        // For exact instance match or for
                        // DLL instance match, keep the new one
#ifdef  FIREWALLS
        ASSERT (!pedit->hServer, "Two instances are matching");
#endif

                        pedit->hServer = (HWND)wParam;
                    } else {

                        ++pedit->extraTerm;
                        // This post directly is alright since we are
                        // terminating extra initiates.

                        PostMessage ((HWND)wParam,
                                WM_DDE_TERMINATE, hwnd, 0);
                    }
#else

                    HandleAckInitMsg (pedit, (HWND)wParam);
#endif
                    if (LOWORD(lParam))
                        GlobalDeleteAtom (LOWORD(lParam));
                    if (HIWORD(lParam))
                        GlobalDeleteAtom (HIWORD(lParam));

                    break;

                case AA_EXECUTE:
                    HandleAck(lpobj, pedit, wParam, lParam);
                    break;


                default:
                    DEBUG_OUT ("received ACK We don't know how to handle ",0)
                    break;


            }

            break;

       case WM_TIMER:
#ifdef  FIREWALLS
            ASSERT (pedit, "sys conv edit block missing");
#endif

            HandleTimerMsg (lpobj, pedit);
            break;

       case WM_DDE_TERMINATE:

#ifdef  FIREWALLS
        ASSERT (pedit, "sys conv edit block missing");
#endif
            HandleTermMsg (lpobj, pedit, (HWND)wParam, FALSE);
            break;

       case WM_DESTROY:
#ifdef  FIREWALLS
        ASSERT (pedit, "sys conv edit block missing");
#endif
            DEBUG_OUT ("destroy window for the sys connection", 0);
            pedit->hClient = NULL;
            break;


       default:
            return DefWindowProc (hwnd, message, wParam, lParam);

       }
       return 0L;
}

void    INTERNAL    HandleTimerMsg (
    LPOBJECT_LE lpobj,
    PEDIT_DDE   pedit
){


    // Since there is only one timer for each client, just
    // repost the message and delete the timer.

    KillTimer (pedit->hClient, 1);
    pedit->wTimer = 0;

    if (PostMessageToServer(pedit, pedit->msg, pedit->lParam))
        return ; // return something.

    // Postmessage failed. We need to getback to the main stream of
    // commands for the object.
    HandleAck (lpobj, pedit, (WPARAM)NULL, pedit->lParam);
    return ;
}


void INTERNAL   HandleTermMsg (lpobj, pedit, hwndPost, bDoc)
    LPOBJECT_LE     lpobj;
    PEDIT_DDE       pedit;
    HWND            hwndPost;
    BOOL            bDoc;
{
    UINT    asyncCmd;
    BOOL    bBusy;

    if (pedit->hServer != hwndPost){
        DEBUG_OUT ("Got terminate for extra conversation",0)
        if (--pedit->extraTerm == 0 && pedit->bTerminating)
            ScheduleAsyncCmd (lpobj);
        return;

    }

    if (!pedit->bTerminating){

        // If we are waiting for any ack, then goto next step with error

        // delete any data if we were in busy mode.
        bBusy = DeleteBusyData (lpobj, pedit);

        asyncCmd = lpobj->asyncCmd;
        PostMessageToServer(pedit, WM_DDE_TERMINATE, 0);
        pedit->hServer = NULL;
        if (pedit->awaitAck || bBusy) {
            // Set error and goto next step.
            lpobj->subErr = OLE_ERROR_COMM;
            pedit->awaitAck = 0;
            ScheduleAsyncCmd (lpobj);
        }

        // If the command is delete, do not delete
        // the edit blocks. It will be deleted
        // in the OleLnkDelete routine and for delete it is
        // possible that by the time we come here, the object
        // may not exist at all.

        if (asyncCmd != OLE_DELETE){
            // QueryOpen() is done because excel is sending WM_DDE_TERMINATE
            // for system without sending for doc in case of failure.

            if (bDoc || QueryOpen (lpobj)) {
                // if the termination is for document and no async command
                // terminate the server conversation also.
                if ((asyncCmd == OLE_NONE) || (asyncCmd == OLE_REQUESTDATA)
                       || (asyncCmd == OLE_OTHER) || (asyncCmd == OLE_SETDATA)
                       || (asyncCmd == OLE_RUN) || (asyncCmd == OLE_SHOW)
                       || (asyncCmd == OLE_SETUPDATEOPTIONS)) {
                    if (lpobj->pDocEdit && lpobj->pDocEdit->awaitAck)
                        // we are waiting for an ack on Doc channel. So start
                        // the unlaunch process after we get the ack.
                        lpobj->bUnlaunchLater = TRUE;
                    else
                        CallEmbLnkDelete (lpobj);
                } else {
                    if (bDoc)
                        DeleteDocEdit (lpobj);

                }
            }else
                DeleteSrvrEdit (lpobj);

        }
    } else {
        pedit->hServer = NULL;
        if (pedit->extraTerm == 0)
            ScheduleAsyncCmd (lpobj);
    }
}

#ifdef FIREWALLS
BOOL INTERNAL CheckAtomValid (ATOM aItem)
{
    char    buffer[MAX_ATOM];
    int     len, val;


    Puts("CheckAtomValid");

    if (aItem == NULL)
             return TRUE;

    val = GlobalGetAtomName (aItem, buffer, MAX_ATOM);
    len = lstrlen (buffer);
    return ((val != 0) && (val == len));
}
#endif




//  HandleAckInitMsg: Handles WM_DDE_ACKs received while in initiate state.  If
//  this is the first reply, save its window handle.  If multiple replies
//  are received, take the one with the prefered instance, if there is
//  one.  Keep a count of WM_DDE_TERMINATEs we send so that we don't shut
//  the window until we get all of the responses for  WM_DDE_TERMINATEs.

void INTERNAL HandleAckInitMsg (
    PEDIT_DDE      pedit,
    HWND           hserver
){

    Puts("HandleAckInitMsg");

    if (pedit->hServer){
        // just take the very first one. Direct post is OK
        PostMessage (hserver, WM_DDE_TERMINATE, (WPARAM)pedit->hClient, 0);
        ++pedit->extraTerm;
    } else
        pedit->hServer = hserver;

}


// HandleAck: returns 0 if <ack> is not positive, else non-0.  Should probably be
//  a macro.

BOOL INTERNAL HandleAck (
    LPOBJECT_LE     lpobj,
    PEDIT_DDE       pedit,
    WPARAM          wParam,
    LPARAM          lParam
){
    WORD    wStatus = GET_WM_DDE_ACK_STATUS(wParam,lParam);
    HANDLE  hData  = NULL;
    BOOL    retval = TRUE;

    UNREFERENCED_PARAMETER(wParam);

    // check for busy bit
    if ((wStatus & 0x4000) && ContextCallBack ((LPOLEOBJECT)lpobj, OLE_QUERY_RETRY)){
        // we got busy from the server. create a timer and wait for time out.

        // We do not need makeprocinstance since, DLLs are single insance, all
        // we need to do is export for this function.

        if ((pedit->wTimer = SetTimer (pedit->hClient, 1, 3000, NULL)))
            return TRUE;
    }

    // even if the client got terminate we have to go thru this path.

    if (pedit->wTimer) {
        KillTimer (pedit->hClient, 1);
        pedit->wTimer = 0;
    }

    if (pedit->awaitAck == AA_POKE)
        // We have to free the data first. Handleack can trigger
        // another Poke (like pokehostnames)
        FreePokeData (lpobj, pedit);

    if (pedit->awaitAck == AA_EXECUTE) {
        hData = GET_WM_DDE_EXECACK_HDATA(wParam,lParam);
        if (hData) GlobalFree (hData);
    } else {
        ATOM aItem = GET_WM_DDE_ACK_ITEM(wParam,lParam);

        ASSERT (CheckAtomValid(aItem),"Invalid atom in ACK")

        if (aItem)
            GlobalDeleteAtom (aItem);
    }

    if (!(wStatus & 0x8000)) {
        // error case. set the error
        DEBUG_OUT ("DDE ACK with failure", 0)

        if (lpobj->errHint){
            lpobj->subErr = lpobj->errHint;
            lpobj->errHint = OLE_OK;
        } else
            lpobj->subErr = OLE_ERROR_COMM;

        retval = FALSE;

        if (pedit->awaitAck == AA_ADVISE) {

#ifdef  ASSERT
        ASSERT (pedit->hopt, "failed advise, options block missing");
#endif
           GlobalFree (pedit->hopt);
        }
    }

    pedit->hopt = NULL;
    pedit->awaitAck = 0;
    ScheduleAsyncCmd (lpobj);
    return retval;
}

// HandleDataMsg: Called for WM_DDE_DATA message.  If data is from an
//  ADVISE-ON-CLOSE and this is there are no more outstanding
//  ADVISE-ON-CLOSE requests, close the document and end the
//  conversation.

void INTERNAL HandleDataMsg (
    LPOBJECT_LE     lpobj,
    HANDLE          hdata,
    ATOM            aItem
){
    DDEDATA         far *lpdata = NULL;
    BOOL            fAck;
    BOOL            fRelease;
    int             options;
    PEDIT_DDE       pedit;

    Puts("HandleDataMsg");

    if (ScanItemOptions (aItem, (int far *)&options) != OLE_OK) {
        DEBUG_OUT (FALSE, "Improper item options");
        return;
    }

    pedit = lpobj->pDocEdit;

    if (hdata) {
        if (!(lpdata = (DDEDATA FAR *) GlobalLock(hdata)))
            return;

        fAck = lpdata->fAckReq;
        fRelease = lpdata->fRelease;

        if (pedit->bTerminating) {
            DEBUG_OUT ("Got DDE_DATA in terminate sequence",0)
            fRelease = TRUE;
        }
        else {
            if ((OLECLIPFORMAT)lpdata->cfFormat == cfBinary && aItem == aStdDocName) {
                ChangeDocName (lpobj, (LPSTR)lpdata->Value);
            }
            else
                SetData (lpobj, hdata, options);

            #ifdef  FIREWALLS
                ASSERT (IsWindowValid(pedit->hServer),
                    "Server window missing in HandleDataMsg")
                ASSERT (CheckAtomValid(aItem),"HandleDataMsg invalid atom")
            #endif

            // important that we post the acknowledge first. Otherwist the
            // messages are not in sync.

            if (fAck)
            {
                LPARAM lparamNew = MAKE_DDE_LPARAM(WM_DDE_ACK,POSITIVE_ACK,aItem);
                PostMessageToServer (pedit, WM_DDE_ACK, lparamNew);
            }
            else if (aItem)
                GlobalDeleteAtom (aItem);

            if ((lpdata->fResponse) && (pedit->awaitAck == AA_REQUEST)) {
                // we sent the request. So, schedule next step.
                pedit->awaitAck = 0;
                ScheduleAsyncCmd (lpobj);
            }
        }

        GlobalUnlock (hdata);
        if (fRelease)
            GlobalFree (hdata);
    }
    else {
        if (CanCallback (lpobj, options)) {
            if (options != OLE_CLOSED)
                ContextCallBack ((LPOLEOBJECT)lpobj, options);
            else
                lpobj->bSvrClosing = FALSE;

        }
    }

    if (options == OLE_CLOSED && (lpobj->pDocEdit->nAdviseClose <= 2)
            && (lpobj->asyncCmd == OLE_NONE)) {
        InitAsyncCmd (lpobj, OLE_SERVERUNLAUNCH, EMBLNKDELETE);
        EmbLnkDelete (lpobj);
    }
}


HANDLE  GetDDEDataHandle (
   DDEDATA far     *lpdata,
   UINT            cfFormat,
   HANDLE          hdata
){

    if (cfFormat == CF_METAFILEPICT) {
#ifdef _WIN64
        return (*(void* _unaligned*)lpdata->Value);
#else
        return LongToHandle(*(LONG*)lpdata->Value);
#endif
    }

    if (cfFormat == CF_BITMAP || cfFormat == CF_ENHMETAFILE)
         return LongToHandle(*(LONG*)lpdata->Value);

    if (cfFormat == CF_DIB)
        return GlobalReAlloc (LongToHandle(*(LONG*)lpdata->Value), 0L,
                    GMEM_MODIFY|GMEM_SHARE);

    return CopyData (((LPSTR)lpdata)+4, (DWORD)(GlobalSize (hdata) - 4));
}

// SetData: Given the DDEDATA structure from a WM_DDE_DATA message, set up the
//  appropriate data in lpobj.  If the native is in native format, add
//  that field, otherwise, if it is in picture format, ask the picture
//  to add it itself.

void INTERNAL SetData (
    LPOBJECT_LE     lpobj,
    HANDLE          hdata,
    int             options
){
    DDEDATA far     *lpdata   = NULL;
    OLESTATUS       retVal = OLE_ERROR_MEMORY;
    HANDLE          hdataDDE;

    Puts("SetData");

    if (!(lpdata = (DDEDATA far *) (GlobalLock (hdata))))
        goto errrtn;


    if (!(hdataDDE =  GetDDEDataHandle (lpdata, lpdata->cfFormat, hdata)))
        goto errrtn;

    if ((OLECLIPFORMAT)lpdata->cfFormat == cfNative) {
        retVal = (*lpobj->head.lpvtbl->ChangeData) ( (LPOLEOBJECT)lpobj,
                        hdataDDE,
                        lpobj->head.lpclient,
                        TRUE);  // use this data, don't copy

    }
    else if ((BOOL)lpdata->cfFormat && (lpdata->cfFormat == (int)GetPictType (lpobj))) {

            retVal = (*lpobj->lpobjPict->lpvtbl->ChangeData) (lpobj->lpobjPict,
                        hdataDDE,
                        lpobj->head.lpclient,
                        lpdata->fRelease);

    } else {
        // case of extra data in the object.
        DeleteExtraData (lpobj);
        lpobj->cfExtra = lpdata->cfFormat;
        lpobj->hextraData = hdataDDE;
        goto end;
    }

    if (retVal == OLE_OK) {
        SetExtents (lpobj);
        if (CanCallback (lpobj, options)) {
            if (options == OLE_CLOSED) {
                ContextCallBack ((LPOLEOBJECT)lpobj, OLE_CHANGED);
                ContextCallBack ((LPOLEOBJECT)lpobj, OLE_CLOSED);
                lpobj->bSvrClosing = FALSE;
            }
            else
                ContextCallBack ((LPOLEOBJECT)lpobj, options);
        }
    }

end:
errrtn:
    if (lpdata)
        GlobalUnlock (hdata);

    return;
}


// SysStartConvDDE: Starts a system conversation.  Returns a handle to that
//  conversation, or NULL.

BOOL INTERNAL InitSrvrConv (
    LPOBJECT_LE     lpobj,
    HANDLE          hInst
){
    HANDLE      hedit = NULL;
    PEDIT_DDE   pedit = NULL;

    Puts("InitSrvrConv");

    if (!lpobj->hSysEdit) {
        hedit = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (EDIT_DDE));

        if (hedit == NULL || ((pedit = (PEDIT_DDE) LocalLock (hedit)) == NULL))
            goto errRtn;

    } else {
#ifdef  FIREWALLS
    ASSERT (!lpobj->pSysEdit->hClient, "Sys conv lptr is present");
#endif
        hedit  =  lpobj->hSysEdit;
        pedit =   lpobj->pSysEdit;
        UtilMemClr ((PSTR) pedit, sizeof (EDIT_DDE));
    }

    if((pedit->hClient = CreateWindow ("OleSrvrWndClass", "",
        WS_OVERLAPPED,0,0,0,0,NULL,NULL, hInstDLL, NULL)) == NULL)
        goto errRtn;


    lpobj->hSysEdit     = hedit;
    lpobj->pSysEdit     = pedit;
    pedit->hInst        = hInst;
    pedit->awaitAck     = AA_INITIATE;

    SetWindowLongPtr (pedit->hClient, 0, (LONG_PTR)lpobj);
    SendMessage ((HWND)-1, WM_DDE_INITIATE, (WPARAM)pedit->hClient,
             MAKELPARAM (lpobj->app, aOle));

    ASSERT (CheckAtomValid(aOle),"systopic invalid atom")

    pedit->awaitAck    = 0;
    if (pedit->hServer == NULL) {
        pedit->awaitAck    = AA_INITIATE;
        // Now try the System topic
        SendMessage ((HWND)-1, WM_DDE_INITIATE, (WPARAM)pedit->hClient,
                 MAKELPARAM (lpobj->app, aSystem));

        ASSERT (CheckAtomValid(aSystem),"systopic invalid atom")

        pedit->awaitAck    = 0;
        if (pedit->hServer == NULL) {
            DEBUG_OUT ("Srver connection failed", 0);
            goto errRtn;
        }
    }

    // Put the long ptr handle in the object.
    return TRUE;

errRtn:

    if (pedit) {
        if (pedit->hClient)
            DestroyWindow (pedit->hClient);
        LocalUnlock (hedit);
    }

    if (hedit)
        LocalFree (hedit);

    lpobj->hSysEdit     = NULL;
    lpobj->pSysEdit    = NULL;

    return FALSE;
}


// TermSrvrConv: Ends conversation indicated by hedit.
void INTERNAL TermSrvrConv (LPOBJECT_LE lpobj)
{
    PEDIT_DDE pedit;

    Puts("TermSrvrConv");


    if (!(pedit = lpobj->pSysEdit))
        return;

    if (PostMessageToServer (pedit, WM_DDE_TERMINATE, 0)){
        lpobj->bAsync        = TRUE;
        pedit->bTerminating = TRUE;
    } else {
        pedit->bTerminating = FALSE;
        lpobj->subErr = OLE_ERROR_TERMINATE;
    }
    return;
}


void INTERNAL  DeleteAbortData (
   LPOBJECT_LE    lpobj,
   PEDIT_DDE     pedit
){
    UNREFERENCED_PARAMETER(lpobj);

    // kill if any timer active.
    if (pedit->wTimer) {
        KillTimer (pedit->hClient, 1);
        pedit->wTimer = 0;
    }
    return;


}

BOOL INTERNAL   DeleteBusyData (
    LPOBJECT_LE    lpobj,
    PEDIT_DDE     pedit
){
    UNREFERENCED_PARAMETER(lpobj);

    // kill if any timer active.
    if (pedit->wTimer) {
        KillTimer (pedit->hClient, 1);
        pedit->wTimer = 0;

        if (pedit->hData) {
            GlobalFree (pedit->hData);
            pedit->hData = NULL;
        }

        if (pedit->hopt) {
            GlobalFree (pedit->hopt);
            pedit->hopt = NULL;
        }

        if (pedit->awaitAck && (HIWORD(pedit->lParam))) {
            if (pedit->awaitAck == AA_EXECUTE) {
		HANDLE hData = GET_WM_DDE_EXECACK_HDATA(pedit->wParam, pedit->lParam);
	        if (hData) GlobalFree (hData);
	    } else {
                ASSERT (CheckAtomValid(HIWORD(pedit->lParam)),
                    "Invalid atom in ACK")
                if (HIWORD(pedit->lParam))
                    GlobalDeleteAtom (HIWORD(pedit->lParam));
            }

            // we want to wipe out the HIWORD of lParam
            pedit->lParam &= 0x0000FFFF;
        }

        return TRUE;
    }

    return FALSE;
}

void INTERNAL   DeleteSrvrEdit (
   LPOBJECT_LE    lpobj
){

    PEDIT_DDE pedit;

    Puts("deleteSrvrEdit");

    if (!(pedit = lpobj->pSysEdit))
        return;


    // delete any data if we were in busy mode.
    DeleteBusyData (lpobj, pedit);

    if (pedit->hClient)
        DestroyWindow (pedit->hClient);

    if (lpobj->pSysEdit)
        LocalUnlock (lpobj->hSysEdit);

    if (lpobj->hSysEdit)
        LocalFree (lpobj->hSysEdit);

    lpobj->hSysEdit  = NULL;
    lpobj->pSysEdit = NULL;

    return;
}


void INTERNAL   SendStdExit (
   LPOBJECT_LE    lpobj
){


    Puts("SendSrvrExit");

    if (!lpobj->pSysEdit)
        return;

    SrvrExecute (lpobj, MapStrToH ("[StdExit]"));

}

void INTERNAL   SendStdClose (
    LPOBJECT_LE    lpobj
){


    Puts("SendDocClose");

    if (!lpobj->pDocEdit)
        return;

    DocExecute (lpobj, MapStrToH ("[StdCloseDocument]"));

}


// SrvrExecute: Sends execute command to system conversation.
BOOL INTERNAL SrvrExecute (
    LPOBJECT_LE lpobj,
    HANDLE      hdata
){
    PEDIT_DDE   pedit = NULL;
    int         retval = FALSE;

    Puts("SrvrExecute");

#ifdef  FIREWALLS

    ASSERT (lpobj->hSysEdit, "Sys conv handle missing");
    ASSERT (lpobj->pSysEdit, "sys conv lptr is missing");

#endif
    pedit = lpobj->pSysEdit;

    if (hdata == NULL || pedit == NULL) {
        lpobj->subErr = OLE_ERROR_MEMORY;
        return FALSE;
    }

#ifdef  FIREWALLS
    ASSERT (!pedit->bTerminating, "In terminate state")
    ASSERT (pedit->awaitAck == 0, "trying to Post msg while waiting for ack")
#endif

    if (lpobj->bOldLink) {
        GlobalFree (hdata);
        return TRUE;
    }


    if (PostMessageToServer (pedit, WM_DDE_EXECUTE, (LPARAM)hdata)) {
        // data is being freed in the acknowledge
        lpobj->bAsync    = TRUE;
        pedit->awaitAck = AA_EXECUTE;
        return TRUE;
    } else {
        lpobj->subErr = OLE_ERROR_COMMAND;
        GlobalFree (hdata);
        return FALSE;
    }
}

// StartConvDDE: Starts the document conversation for an object based on
// .app and .topic atoms.
BOOL FARINTERNAL InitDocConv (
    LPOBJECT_LE lpobj,
    BOOL        fNetDlg
){

    // ### This routine looks very similar to IitSrvrConv
    // combine with the it

    HANDLE      hedit = NULL;
    PEDIT_DDE   pedit = NULL;
    char        buf[MAX_NET_NAME];
    int         nDrive = 2;     // drive C
    char        cOldDrive;

    Puts("InitDocConv");

    if (QueryOpen (lpobj)){
        DEBUG_OUT ("Attempt to start already existing conversation",0);
        return FALSE;
    }

    cOldDrive = lpobj->cDrive;
    if (CheckNetDrive (lpobj, fNetDlg) != OLE_OK)
        return FALSE;

    if (!lpobj->pDocEdit) {
        hedit = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (EDIT_DDE));

        if (hedit == NULL || ((pedit = (PEDIT_DDE) LocalLock (hedit)) == NULL)){
            lpobj->subErr = OLE_ERROR_MEMORY;
            goto errRtn;
        }
    } else {
#ifdef  FIREWALLS
    ASSERT (!lpobj->pDocEdit->hClient, "Doc conv lptr is present");
#endif
        hedit  =  lpobj->hDocEdit;
        pedit =  lpobj->pDocEdit;
        UtilMemClr ((PSTR) pedit, sizeof (EDIT_DDE));
    }

    if ((pedit->hClient = CreateWindow ("OleDocWndClass", "Window Name",
        WS_OVERLAPPED,0,0,0,0,NULL,NULL, hInstDLL, NULL)) == NULL) {
        lpobj->subErr = OLE_ERROR_MEMORY;
        goto errRtn;
    }
    lpobj->hDocEdit     = hedit;
    lpobj->pDocEdit     = pedit;
    SetWindowLongPtr (pedit->hClient, 0, (LONG_PTR)lpobj);

    // buf will filled by netname in the first call to SetNextNetDrive()
    buf[0] = '\0';
    do {
        pedit->awaitAck = AA_INITIATE;

        // !!! Where are the atom counts bumped?

        SendMessage ((HWND)-1, WM_DDE_INITIATE, (WPARAM)pedit->hClient,
                MAKELPARAM (lpobj->app, lpobj->topic));

        pedit->awaitAck = 0;

        if (pedit->hServer) {
            if ((cOldDrive != lpobj->cDrive)
                    && (lpobj->asyncCmd != OLE_CREATEFROMFILE))
                ContextCallBack ((LPOLEOBJECT)lpobj, OLE_RENAMED);
            return TRUE;
        }

    } while ((lpobj->head.ctype == CT_LINK) && (lpobj->aNetName)
                && SetNextNetDrive (lpobj, &nDrive, buf)) ;

errRtn:
    if (cOldDrive != lpobj->cDrive) {
        // put back the old drive
        lpobj->cDrive = cOldDrive;
        ChangeTopic (lpobj);
    }

    if (pedit && pedit->hClient)
        DestroyWindow (pedit->hClient);

    if (pedit)
        LocalUnlock (hedit);

    if (hedit)
        LocalFree (hedit);

    lpobj->hDocEdit     = NULL;
    lpobj->pDocEdit     = NULL;
    return FALSE;
}


// Execute: Sends an execute string WM_DDE_EXECUTE to the document conversation.
BOOL INTERNAL DocExecute(
    LPOBJECT_LE lpobj,
    HANDLE      hdata
){
    PEDIT_DDE   pedit;

    Puts("DocExecute");
    pedit = lpobj->pDocEdit;

    if (hdata == NULL || pedit == NULL)
        return FALSE;


#ifdef  FIREWALLS
    ASSERT (!pedit->bTerminating, "Execute: terminating")
    ASSERT (pedit->hClient, "Client null")
    ASSERT (IsWindowValid(pedit->hClient),"Invalid client")
    ASSERT (pedit->awaitAck == 0, "trying to Post msg while waiting for ack")
#endif

    if (lpobj->bOldLink) {
        GlobalFree (hdata);
        return TRUE;
    }

    if (PostMessageToServer (pedit, WM_DDE_EXECUTE, (LPARAM)hdata)) {
        // data is being freed in the execute command
        pedit->awaitAck    = AA_EXECUTE;
        lpobj->bAsync       = TRUE;
        return TRUE;
    } else {
        lpobj->subErr    = OLE_ERROR_COMMAND;
        GlobalFree (hdata);
        return FALSE;
    }
}


// EndConvDDE: terminates the doc level conversation.
void INTERNAL TermDocConv (
    LPOBJECT_LE    lpobj
){
    PEDIT_DDE pedit;

    Puts ("TermDocConv");

    DEBUG_OUT ("About to terminate convs from destroy",0)

    if (!(pedit = lpobj->pDocEdit))
        return;

    if (PostMessageToServer (pedit, WM_DDE_TERMINATE, 0)) {
        pedit->bTerminating = TRUE;
        lpobj->bAsync        = TRUE;
    } else
        lpobj->subErr = OLE_ERROR_TERMINATE;

    return;

}

// Deletes the document conversdation memory.
void INTERNAL DeleteDocEdit (lpobj)
LPOBJECT_LE    lpobj;
{

    PEDIT_DDE pedit;

    Puts ("DeleteDocEdit");

    if (!(pedit = lpobj->pDocEdit))
        return;

    // delete any data if we were in busy mode.
    DeleteBusyData (lpobj, pedit);

    // Delete if any data blocks.
    if (pedit->hClient)
        DestroyWindow (pedit->hClient);

    if (lpobj->pDocEdit)
        LocalUnlock (lpobj->hDocEdit);

    if (lpobj->hDocEdit)
        LocalFree (lpobj->hDocEdit);

    lpobj->hDocEdit  = NULL;
    lpobj->pDocEdit = NULL;

    return;
}


// LeLauchApp: Launches app based on the ClassName in lpobj.
// History:
// curts changed LoadModule calls to WinExec
//

HANDLE INTERNAL  LeLaunchApp (LPOBJECT_LE lpobj)
{
//    struct CMDSHOW
//    {
//        WORD first;
//        WORD second;
//    } cmdShow = {2, SW_SHOWNORMAL};
//
//    struct
//    {
//        WORD wEnvSeg;
//        LPSTR lpcmdline;
//        struct CMDSHOW FAR *lpCmdShow;
//        DWORD dwReserved;
//    } paramBlock;

    WORD    cmdShow = SW_SHOWNORMAL;
    char    cmdline[MAX_STR];
    char    exeName[MAX_STR];
    char    lpstr[2*MAX_STR];
    HANDLE  hInst;

    #define EMB_STR     " -Embedding "

    Puts("LeLaunchApp");

    GlobalGetAtomName (lpobj->aServer, exeName, MAX_STR);

    cmdline[0] = ' ';
    if (lpobj->bOldLink) {
        cmdShow = SW_SHOWMINIMIZED;
        GlobalGetAtomName (lpobj->topic, cmdline + 1, MAX_STR);
    } else {
        lstrcpy ((LPSTR)cmdline+1, (LPSTR) EMB_STR);

        // For all link servers we want to give the filename on the command
        // line. But Excel is not registering the document before returning
        // from WinMain, if it has auto load macros. So, we want send StdOpen
        // for the old servers, instead of giving the file name on the command
        // line.

        if (lpobj->bOleServer && (lpobj->fCmd & LN_MASK) == LN_LNKACT)
            GlobalGetAtomName (lpobj->topic, cmdline+sizeof(EMB_STR),
                    MAX_STR-sizeof(EMB_STR));
        if (lpobj->fCmd & ACT_MINIMIZE)
            cmdShow = SW_SHOWMINIMIZED;
        else if (!(lpobj->fCmd & (ACT_SHOW | ACT_DOVERB))
                    // we want to launch with show in create invisible case
                    // even though ACT_SHOW flag will be false
                    && ((lpobj->fCmd & LN_MASK) != LN_NEW))
            cmdShow = SW_HIDE;
    }

//    paramBlock.wEnvSeg      = NULL;
//    paramBlock.lpcmdline    = (LPSTR)cmdline;
//    paramBlock.lpCmdShow    = &cmdShow;
//    paramBlock.dwReserved   = NULL;

    lstrcpy(lpstr,exeName);
    lstrcat(lpstr,cmdline);

    if ((hInst =  (HANDLE)ULongToPtr(WinExec(lpstr, cmdShow))) < (HANDLE)32)
        hInst = NULL;

    if (!hInst) {
        LPSTR   lptmp;
        char    ch;

        // strip off the path and try again
        lptmp = (LPSTR)exeName;
        lptmp += lstrlen ((LPSTR) exeName);
        ch = *lptmp;
        while (ch != '\\' && ch != ':') {
            if (lptmp == (LPSTR) exeName) {
                // exe did not have path in it's name. we already tried
                // loading and it failed, no point trying again.
                return NULL;
            }
            else
                ch = *--lptmp;
        }

        lstrcpy(lpstr,++lptmp);
        lstrcat(lpstr,cmdline);

        if ((hInst =  (HANDLE)ULongToPtr(WinExec(lpstr,cmdShow))) < (HANDLE)32)
            hInst = NULL;
    }

    return hInst;
}



//ScanItemOptions: Scan for the item options like Close/Save etc.

int INTERNAL ScanItemOptions (
   ATOM    aItem,
   int far *lpoptions
){

    ATOM    aModifier;

    LPSTR   lpbuf;
    char    buf[MAX_STR];

    *lpoptions = OLE_CHANGED;

    if (!aItem) {
        // NULL item with no modifier means OLE_CHANGED for NULL item
        return OLE_OK;
    }

    GlobalGetAtomName (aItem, (LPSTR)buf, MAX_STR);
    lpbuf = (LPSTR)buf;

    while ( *lpbuf && *lpbuf != '/')
           lpbuf++;

    // no modifier same as /change

    if (*lpbuf == '\0')
        return OLE_OK;

    *lpbuf++ = '\0';        // seperate out the item string
                            // We are using this in the caller.

    if (!(aModifier = GlobalFindAtom (lpbuf)))
        return OLE_ERROR_SYNTAX;

    if (aModifier == aChange)
        return OLE_OK;

    // Is it a save?
    if (aModifier == aSave){
        *lpoptions = OLE_SAVED;
        return  OLE_OK;
    }
    // Is it a Close?
    if (aModifier == aClose){
        *lpoptions = OLE_CLOSED;
        return OLE_OK;
    }

    // unknown modifier
    return OLE_ERROR_SYNTAX;

}

void   INTERNAL   ChangeDocName (
    LPOBJECT_LE lpobj,
    LPSTR       lpdata

){
    ATOM      aOldTopic;
    OLESTATUS retVal;

    aOldTopic = lpobj->topic;
    lpobj->topic = GlobalAddAtom (lpdata);
    if ((retVal = SetNetName (lpobj)) != OLE_OK) {
        if (lpobj->topic)
            GlobalDeleteAtom (lpobj->topic);
        lpobj->topic = aOldTopic;
        return;
        // !!! what should we do in case of error? Currently, we will not
        // change the topic if SetNetName fails.
    }

    if (aOldTopic)
        GlobalDeleteAtom (aOldTopic);

    // Delete the link data block
    if (lpobj->hLink) {
        GlobalFree (lpobj->hLink);
        lpobj->hLink = NULL;
    }

    ContextCallBack ((LPOLEOBJECT)lpobj, OLE_RENAMED);



}


BOOL INTERNAL CanCallback (
    LPOBJECT_LE lpobj,
    int         options
){
    LPINT    lpCount;

    if (options == OLE_CLOSED) {
        lpobj->bSvrClosing = TRUE;
        lpCount = &(lpobj->pDocEdit->nAdviseClose);
    }
    else if (options == OLE_SAVED) {
        if (lpobj->head.ctype == CT_LINK)
            return TRUE;
        lpCount = &(lpobj->pDocEdit->nAdviseSave);
    }
    else {
        // it must be due to request
        if ((lpobj->pDocEdit->awaitAck == AA_REQUEST)
                && lpobj->pDocEdit->bCallLater)
            return FALSE;

        return TRUE;
    }

    switch (*lpCount) {
        case 1:
            break;

        case 2:
            ++(*lpCount);
            return FALSE;

        case 3:
            --(*lpCount);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


void  FARINTERNAL CallEmbLnkDelete (
    LPOBJECT_LE lpobj
){
    InitAsyncCmd (lpobj, OLE_SERVERUNLAUNCH,EMBLNKDELETE);
    EmbLnkDelete (lpobj);

    if (lpobj->head.ctype == CT_EMBEDDED) {
        lpobj->bSvrClosing = TRUE;
        ContextCallBack ((LPOLEOBJECT)lpobj, OLE_CLOSED);
        if (FarCheckObject ((LPOLEOBJECT)lpobj))
            lpobj->bSvrClosing = FALSE;
    }
}
