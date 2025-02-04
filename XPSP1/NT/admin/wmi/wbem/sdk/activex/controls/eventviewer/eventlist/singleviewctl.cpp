// Copyright (c) 1997-2001 Microsoft Corporation, All Rights Reserved
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "precomp.h"
#include "singleviewctl.h"

/////////////////////////////////////////////////////////////////////////////
// CSingleViewCtl

IMPLEMENT_DYNCREATE(CSingleViewCtl, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CSingleViewCtl properties

CString CSingleViewCtl::GetNameSpace()
{
	CString result;
	GetProperty(0x1, VT_BSTR, (void*)&result);
	return result;
}

void CSingleViewCtl::SetNameSpace(LPCTSTR propVal)
{
	SetProperty(0x1, VT_BSTR, propVal);
}

long CSingleViewCtl::GetReadyState()
{
	long result;
	GetProperty(DISPID_READYSTATE, VT_I4, (void*)&result);
	return result;
}

long CSingleViewCtl::GetPropertyFilter()
{
	long result;
	GetProperty(0x2, VT_I4, (void*)&result);
	return result;
}

void CSingleViewCtl::SetPropertyFilter(long propVal)
{
	SetProperty(0x2, VT_I4, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CSingleViewCtl operations

long CSingleViewCtl::GetEditMode()
{
	long result;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

void CSingleViewCtl::SetEditMode(long lEditMode)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 lEditMode);
}

long CSingleViewCtl::RefreshView()
{
	long result;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

void CSingleViewCtl::NotifyWillShow()
{
	InvokeHelper(0x6, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long CSingleViewCtl::DeleteInstance()
{
	long result;
	InvokeHelper(0x7, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

void CSingleViewCtl::ExternInstanceCreated(LPCTSTR szObjectPath)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x8, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 szObjectPath);
}

void CSingleViewCtl::ExternInstanceDeleted(LPCTSTR szObjectPath)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x9, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 szObjectPath);
}

long CSingleViewCtl::QueryCanCreateInstance()
{
	long result;
	InvokeHelper(0xa, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long CSingleViewCtl::QueryCanDeleteInstance()
{
	long result;
	InvokeHelper(0xb, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long CSingleViewCtl::QueryNeedsSave()
{
	long result;
	InvokeHelper(0xc, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long CSingleViewCtl::QueryObjectSelected()
{
	long result;
	InvokeHelper(0xd, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

CString CSingleViewCtl::GetObjectPath(long lPosition)
{
	CString result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		lPosition);
	return result;
}

long CSingleViewCtl::StartViewEnumeration(long lWhere)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xf, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lWhere);
	return result;
}

long CSingleViewCtl::GetTitle(BSTR* pszTitle, LPDISPATCH* lpPictureDisp)
{
	long result;
	static BYTE parms[] =
		VTS_PBSTR VTS_PDISPATCH;
	InvokeHelper(0x10, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		pszTitle, lpPictureDisp);
	return result;
}

CString CSingleViewCtl::GetViewTitle(long lPosition)
{
	CString result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x11, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		lPosition);
	return result;
}

long CSingleViewCtl::NextViewTitle(long lPositon, BSTR* pbstrTitle)
{
	long result;
	static BYTE parms[] =
		VTS_I4 VTS_PBSTR;
	InvokeHelper(0x12, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lPositon, pbstrTitle);
	return result;
}

long CSingleViewCtl::PrevViewTitle(long lPosition, BSTR* pbstrTitle)
{
	long result;
	static BYTE parms[] =
		VTS_I4 VTS_PBSTR;
	InvokeHelper(0x13, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lPosition, pbstrTitle);
	return result;
}

long CSingleViewCtl::SelectView(long lPosition)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x14, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lPosition);
	return result;
}

long CSingleViewCtl::StartObjectEnumeration(long lWhere)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x15, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lWhere);
	return result;
}

CString CSingleViewCtl::GetObjectTitle(long lPos)
{
	CString result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x16, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		lPos);
	return result;
}

long CSingleViewCtl::SaveData()
{
	long result;
	InvokeHelper(0x17, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long CSingleViewCtl::AddContextRef(long lCtxtHandle)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x18, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lCtxtHandle);
	return result;
}

long CSingleViewCtl::ReleaseContext(long lCtxtHandle)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x19, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lCtxtHandle);
	return result;
}

long CSingleViewCtl::RestoreContext(long lCtxtHandle)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1a, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lCtxtHandle);
	return result;
}

long CSingleViewCtl::GetContext(long* plCtxthandle)
{
	long result;
	static BYTE parms[] =
		VTS_PI4;
	InvokeHelper(0x1b, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		plCtxthandle);
	return result;
}

long CSingleViewCtl::NextObject(long lPosition)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1c, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lPosition);
	return result;
}

long CSingleViewCtl::PrevObject(long lPosition)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1d, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lPosition);
	return result;
}

long CSingleViewCtl::SelectObjectByPath(LPCTSTR szObjectPath)
{
	long result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x1e, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		szObjectPath);
	return result;
}

long CSingleViewCtl::SelectObjectByPosition(long lPosition)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1f, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lPosition);
	return result;
}

long CSingleViewCtl::SelectObjectByPointer(LPUNKNOWN lpunkWbemServices, LPUNKNOWN lpunkClassObject, long bExistsInDatabase)
{
	long result;
	static BYTE parms[] =
		VTS_UNKNOWN VTS_UNKNOWN VTS_I4;
	InvokeHelper(0x20, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		lpunkWbemServices, lpunkClassObject, bExistsInDatabase);
	return result;
}

long CSingleViewCtl::CreateInstance(LPCTSTR szClassName)
{
	long result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x21, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		szClassName);
	return result;
}

long CSingleViewCtl::CreateInstanceOfCurrentClass()
{
	long result;
	InvokeHelper(0x22, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

void CSingleViewCtl::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}
