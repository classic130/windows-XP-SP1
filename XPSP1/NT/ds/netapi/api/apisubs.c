/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

    apisubs.c

Abstract:

    Subroutines for LAN Manager APIs.

Author:

    Chuck Lenzmeier (chuckl) 25-Jul-90

Revision History:

    08-Sept-1992    Danl
        Dll Cleanup routines used to be called for DLL_PROCESS_DETACH.
        Thus they were called for FreeLibrary or ExitProcess reasons.
        Now they are only called for the case of a FreeLibrary.  ExitProcess
        will automatically clean up process resources.

    03-Aug-1992     JohnRo
        Use FORMAT_ and PREFIX_ equates.

--*/

// These must be included first:
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NOMINMAX                // Avoid stdlib.h vs. windows.h warnings.
#include <windows.h>
#include <lmcons.h>
#include <ntsam.h>
#include <netdebug.h>

// These may be included in any order:
#include <accessp.h>
#include <configp.h>
#include <lmerr.h>
#include <netdebug.h>
#include <netlib.h>
#include <prefix.h>     // PREFIX_ equates.
#include <secobj.h>
#include <stdarg.h>
#include <stdio.h>
#include <rpcutil.h>
#include <thread.h>
#include <stdlib.h>
#include <netbios.h>
#include <dfsp.h>
#include <winsock2.h>  // needed by dsgetdcp.h
#include <dsgetdc.h>   // needed by dsgetdcp.h
#include <dsgetdcp.h>  // DCNameInitialize/Close


BOOLEAN
NetapipInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN LPVOID lpReserved OPTIONAL
    )
{
    //
    // Handle attaching netapi.dll to a new process.
    //

    if (Reason == DLL_PROCESS_ATTACH) {

        NET_API_STATUS NetStatus;
        NTSTATUS Status;

        if ( !DisableThreadLibraryCalls( DllHandle ) )
        {
            NetpKdPrint((
                    PREFIX_NETAPI "DisableThreadLibraryCalls failed: "
                    FORMAT_API_STATUS "\n", GetLastError()));
        }

        //
        // Initialize Netbios
        //

        NetbiosInitialize(DllHandle);

        //
        // Initialize the NetGetDCName PDC Name cache
        //

        if (( NetStatus = DCNameInitialize()) != NERR_Success) {
            NetpKdPrint(( "[netapi.dll] Failed initialize DCName APIs%lu\n",
                          NetStatus));
            return FALSE;
        }

        //
        // Initialize the NetDfsXXX API Critical Section
        //
        InitializeCriticalSection( &NetDfsApiCriticalSection );
        NetDfsApiInitialize();

    //
    // When DLL_PROCESS_DETACH and lpReserved is NULL, then a FreeLibrary
    // call is being made.  If lpReserved is Non-NULL, and ExitProcess is
    // in progress.  These cleanup routines will only be called when
    // a FreeLibrary is being called.  ExitProcess will automatically
    // clean up all process resources, handles, and pending io.
    //
    } else if ((Reason == DLL_PROCESS_DETACH) &&
               (lpReserved == NULL)) {

        NetbiosDelete();

        DCNameClose();

        //
        // Delete the NetDfsXXX API critical section
        //
        DeleteCriticalSection( &NetDfsApiCriticalSection );
    }

    return TRUE;

} // NetapipInitialize

