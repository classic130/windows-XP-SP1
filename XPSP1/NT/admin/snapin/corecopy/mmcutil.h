//____________________________________________________________________________
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996 - 1997.
//
//  File:       MMCUtil.h
//
//  Contents:
//
//  Classes:
//
//  Functions:  WriteToStream
//              ReadFromStream
//              ListView_GetItemData
//
//  History:    12/4/1996   RaviR   Created
//____________________________________________________________________________
//

#ifndef _MMCUTIL_H_
#define _MMCUTIL_H_

#define MMC_CLSCTX_INPROC (CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER)


inline void WriteToStream(IStream& stream, void const* pv, ULONG cbToWrite)
{
    ULONG bytesWritten;
    HRESULT hr = stream.Write(pv, cbToWrite, &bytesWritten);
    ANT(SUCCEEDED(hr) && bytesWritten == cbToWrite,
                                        COMEX(hr, UnableToWriteToStream));
}


inline void ReadFromStream(IStream& stream, void* pv, ULONG cbToRead)
{
    unsigned long bytesRead;
    HRESULT hr = stream.Read(pv, cbToRead, &bytesRead);
    ANT(SUCCEEDED(hr) && bytesRead == cbToRead, COMEX(hr, UnableToLoad));
}


inline ListView_GetItemData(HWND hwnd, int iItem)
{
    LV_ITEM lvi; 
    ZeroMemory(&lvi, sizeof(lvi));

    if (iItem >= 0)
    {
        lvi.iItem  = iItem;
        lvi.mask = LVIF_PARAM;
        VERIFY(::SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM)&lvi) == TRUE);
    }

    return lvi.lParam;
}




#endif // _MMCUTIL_H_


