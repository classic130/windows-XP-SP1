/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    elflpc.c

Abstract:

    This file contains the routines that deal with the LPC port in the
    eventlog service.

Author:

    Rajen Shah  (rajens)    10-Jul-1991

Revision History:



--*/

//
// INCLUDES
//

#include <eventp.h>
#include <ntiolog.h>    // For IO_ERROR_LOG_[MESSAGE/PACKET]
#include <ntiologc.h>   // QUOTA error codes
#include <elfkrnl.h>
#include <stdlib.h>
#include <memory.h>
#include <elfextrn.h>   // Computername

#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <ntdef.h>
#include <ntstatus.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <lmcons.h>
#include <string.h>
#include <lmerr.h>
#include <elfmsg.h>


//
//  Global value for the "system" module
//

PLOGMODULE SystemModule = NULL;

NTSTATUS
SetUpLPCPort(
    VOID
    )

/*++

Routine Description:

    This routine sets up the LPC port for the service.

Arguments:

    None

Return Value:

--*/
{
    NTSTATUS status;
    UNICODE_STRING SystemString;
    UNICODE_STRING unicodePortName;
    OBJECT_ATTRIBUTES objectAttributes;
    PORT_MESSAGE connectionRequest;

    ELF_LOG0(LPC,
             "SetUpLPCPort: Enter\n");

    //
    // We're going to need this every time, so just get it once
    //
    ASSERT(SystemModule == NULL);

    //
    // Get the system module to log driver events
    //
    RtlInitUnicodeString(&SystemString, ELF_SYSTEM_MODULE_NAME);
    SystemModule = GetModuleStruc(&SystemString);

    //
    // The System log and its default module should have been created by now.
    //
    ASSERT(_wcsicmp(SystemModule->ModuleName, ELF_SYSTEM_MODULE_NAME) == 0);

    //
    // Initialize the handles to zero so that we can determine what to do
    // if we need to clean up.
    //
    ElfConnectionPortHandle = NULL;
    ElfCommunicationPortHandle = NULL;

    //
    // Create the LPC port.
    //
    RtlInitUnicodeString( &unicodePortName, ELF_PORT_NAME_U );

    InitializeObjectAttributes(
            &objectAttributes,
            &unicodePortName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    status = NtCreatePort(&ElfConnectionPortHandle,
                          &objectAttributes,
                          0,
                          ELF_PORT_MAX_MESSAGE_LENGTH,
                          ELF_PORT_MAX_MESSAGE_LENGTH * 32);

    if (!NT_SUCCESS(status))
    {
        ELF_LOG2(ERROR,
                 "SetUpLPCPort: Error creating LPC port %ws %#x\n",
                 ELF_PORT_NAME_U,
                 status);
    }

    ELF_LOG1(LPC,
             "SetUpLPCPort: Exiting with status %#x\n",
             status);

    return status;
}



LPWSTR
ElfpCopyString(
    LPWSTR Destination,
    LPWSTR Source,
    ULONG Length
    )

/*++

Routine Description:

    Copies a string to the destination.  Correctly NUL terminates
    the string.


Arguments:

    Destination - place where string is to be copied
    
    Source - string that may or may not be NUL terminated
    
    Length - length in bytes of string being copied.  May include NUL

Return Value:

    LPWSTR to first WCHAR past NUL

--*/
{
    //
    //  Copy the data
    //
    RtlMoveMemory(Destination, Source, Length);

    //
    //  Make sure it's NULL terminated
    //
    if (Length != 0)
    {
        Destination += Length / sizeof(WCHAR) - 1;

        if (*Destination != L'\0')
        {
            Destination++;
            *Destination = L'\0';
        }
    }
    else
    {
        *Destination = L'0';
    }

    return Destination + 1;
}


NTSTATUS
ElfProcessIoLPCPacket( 
    ULONG                 PacketLength,
    PIO_ERROR_LOG_MESSAGE pIoErrorLogMessage
    )

/*++

Routine Description:

    This routine takes the packet received from the LPC port and processes it.
    The logfile will be system, the module name will be the driver that 
    generated the packet, the SID will always be NULL and
    there will always be one string, which will be the device name.

    It extracts the information from the LPC packet, and then calls the
    common routine to do the work of formatting the data into
    an event record and writing it out to the log file.


Arguments:

    pIoErrorLogMessage - Pointer to the data portion of the packet just
                         received through the LPC port.


Return Value:

    Status of this operation.

--*/

{
    NTSTATUS status;
    ELF_REQUEST_RECORD  Request;
    WRITE_PKT WritePkt;

    ULONG RecordLength;
    PEVENTLOGRECORD EventLogRecord;
    LPWSTR DestinationString, SourceString;
    PBYTE BinaryData;
    ULONG PadSize;
    LARGE_INTEGER Time;
    ULONG TimeWritten;
    PULONG pEndLength;
    ULONG i = 0;
    PWCHAR pwch;
    PWCHAR pwStart;
    PWCHAR pwEnd;
    ULONG StringLength;
	WCHAR LocalComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	ULONG  ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
	BOOL bOK;
    PacketLength = min(pIoErrorLogMessage->Size, PacketLength);

    try
    {
        // Get the computer name

    	bOK = GetComputerNameW(LocalComputerName, &ComputerNameLength);
		if(bOK == FALSE)
		{
            ELF_LOG1(ERROR,
                 "ElfProcessIoLPCPacket: failed calling GetComputerNameW, last error 0x%x\n",
                  GetLastError());
            return STATUS_UNSUCCESSFUL;    
		}
    	ComputerNameLength = (ComputerNameLength+1)*sizeof(WCHAR); // account for the NULL
    	
        //
        // Validate the packet, First make sure there are the correct
        // number of NULL terminated strings, and remember the
        // total number of bytes to copy
        //
        pwStart = pwch = (PWCHAR) ((PBYTE) pIoErrorLogMessage +
                                       pIoErrorLogMessage->EntryData.StringOffset);

        pwEnd = (PWCHAR) ((PBYTE) pIoErrorLogMessage + PacketLength);

        while (pwch < pwEnd
                &&
               i < pIoErrorLogMessage->EntryData.NumberOfStrings)
        {
                if (*pwch == L'\0')
                {
                    i++;
                }

                pwch++;
        }

        StringLength = (ULONG) (pwch - pwStart) * sizeof(WCHAR);

        //
        // Now make sure everything in the packet is true
        //

        if ((i != pIoErrorLogMessage->EntryData.NumberOfStrings)
              ||
            (pIoErrorLogMessage->DriverNameOffset 
                 + pIoErrorLogMessage->DriverNameLength >= PacketLength)
              ||
            (pIoErrorLogMessage->EntryData.StringOffset >= PacketLength)
              ||
            (FIELD_OFFSET(IO_ERROR_LOG_MESSAGE, EntryData) 
                 + FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) 
                 + (ULONG) pIoErrorLogMessage->EntryData.DumpDataSize >= PacketLength))
        {    
            //
            // It's a bad packet, log it and return
            //
            ELF_LOG0(ERROR,
                     "ElfProcessIoLPCPacket: Bad LPC packet -- dumping it to System log\n");

            ElfpCreateElfEvent(EVENT_BadDriverPacket,
                               EVENTLOG_ERROR_TYPE,
                               0,                    // EventCategory
                               0,                    // NumberOfStrings
                               NULL,                 // Strings
                               pIoErrorLogMessage,   // Data
                               PacketLength,         // Datalength
                               0,
                               FALSE);                   // flags

            return STATUS_UNSUCCESSFUL;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        //
        // It's a bad packet, log it and return
        //
        ELF_LOG1(ERROR,
                 "ElfProcessIoLPCPacket: Exception %#x caught processing I/O LPC packet\n",
                 GetExceptionCode());

        ElfpCreateElfEvent(EVENT_BadDriverPacket,
                           EVENTLOG_ERROR_TYPE,
                           0,                    // EventCategory
                           0,                    // NumberOfStrings
                           NULL,                 // Strings
                           NULL,                 // Data
                           0,                    // Datalength
                           0,
                           FALSE);                   // flags

        return STATUS_UNSUCCESSFUL;
    }

    //
    // The packet should be an IO_ERROR_LOG_MESSAGE
    //
    ASSERT(pIoErrorLogMessage->Type == IO_TYPE_ERROR_MESSAGE);

    //
    // Set up write packet in request packet
    //
    Request.Pkt.WritePkt = &WritePkt;
    Request.Flags = 0;

    //
    // Generate any additional information needed in the record.
    //

    //
    // TIMEWRITTEN
    // We need to generate a time when the log is written. This
    // gets written in the log so that we can use it to test the
    // retention period when wrapping the file.
    //
    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(
                        &Time,
                        &TimeWritten
                        );

    //
    // Determine how big a buffer is needed for the eventlog record.
    //
    RecordLength = sizeof(EVENTLOGRECORD)
                       + ComputerNameLength                   // computer name
                       + 2 * sizeof(WCHAR)                    // terminating NULLs
                       + PacketLength
                       - FIELD_OFFSET(IO_ERROR_LOG_MESSAGE, EntryData)
                       + sizeof(RecordLength);                // final len

    //
    // Determine how many pad bytes are needed to align to a DWORD
    // boundary.
    //
    PadSize = sizeof(ULONG) - (RecordLength % sizeof(ULONG));

    RecordLength += PadSize;    // True size needed

    //
    // Allocate the buffer for the Eventlog record
    //
    EventLogRecord = (PEVENTLOGRECORD) ElfpAllocateBuffer(RecordLength);

    if (EventLogRecord != (PEVENTLOGRECORD) NULL)
    {
        //
        // Fill up the event record
        //
        EventLogRecord->Length = RecordLength;

        RtlTimeToSecondsSince1970(&pIoErrorLogMessage->TimeStamp,
                                  &EventLogRecord->TimeGenerated);

        EventLogRecord->Reserved    = ELF_LOG_FILE_SIGNATURE;
        EventLogRecord->TimeWritten = TimeWritten;
        EventLogRecord->EventID     = pIoErrorLogMessage->EntryData.ErrorCode;

        //
        // Set EventType based on the high order nibble of
        // pIoErrorLogMessage->EntryData.ErrorCode
        //
        if (NT_INFORMATION(pIoErrorLogMessage->EntryData.ErrorCode))
        {
            EventLogRecord->EventType = EVENTLOG_INFORMATION_TYPE;
        }
        else if (NT_WARNING(pIoErrorLogMessage->EntryData.ErrorCode))
        {
            EventLogRecord->EventType = EVENTLOG_WARNING_TYPE;
        }
        else if (NT_ERROR(pIoErrorLogMessage->EntryData.ErrorCode))
        {
            EventLogRecord->EventType = EVENTLOG_ERROR_TYPE;
        }
        else
        {
            //
            // Unknown, set to error
            //
            ELF_LOG1(LPC,
                     "ElfProcessIoLPCPacket: Unknown EventType (high nibble of ID %#x)\n",
                     EventLogRecord->EventID);

            EventLogRecord->EventType = EVENTLOG_ERROR_TYPE;
        }

        EventLogRecord->NumStrings    = pIoErrorLogMessage->EntryData.NumberOfStrings;
        EventLogRecord->EventCategory = pIoErrorLogMessage->EntryData.EventCategory;
        EventLogRecord->StringOffset  = sizeof(EVENTLOGRECORD)
                                            + pIoErrorLogMessage->DriverNameLength
                                            + ComputerNameLength;

        EventLogRecord->DataLength    = FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData)
                                            + pIoErrorLogMessage->EntryData.DumpDataSize;

        EventLogRecord->DataOffset    = EventLogRecord->StringOffset + StringLength;

        //
        // Quota events contain a SID.
        //
        if (pIoErrorLogMessage->EntryData.ErrorCode == IO_FILE_QUOTA_LIMIT
             ||
            pIoErrorLogMessage->EntryData.ErrorCode == IO_FILE_QUOTA_THRESHOLD)
        {
            PFILE_QUOTA_INFORMATION pFileQuotaInformation =
                (PFILE_QUOTA_INFORMATION) pIoErrorLogMessage->EntryData.DumpData;

            ELF_LOG0(LPC,
                     "ElfProcessIoLPCPacket: Event is a Quota event\n");

            EventLogRecord->UserSidLength = pFileQuotaInformation->SidLength;
            EventLogRecord->UserSidOffset = EventLogRecord->DataOffset
                                                + FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData)
                                                + FIELD_OFFSET(FILE_QUOTA_INFORMATION, Sid);

            EventLogRecord->DataLength    = EventLogRecord->UserSidOffset -
                                                EventLogRecord->DataOffset;
        }
        else
        {
            EventLogRecord->UserSidLength = 0;
            EventLogRecord->UserSidOffset = 0;
        }

        //
        // Fill in the variable-length fields
        //

        //
        // MODULENAME
        //
        // Use the driver name as the module name, since its location is
        // described by an offset from the start of the IO_ERROR_LOG_MESSAGE
        // turn it into a pointer
        //
        DestinationString = (LPWSTR) ((LPBYTE) EventLogRecord + sizeof(EVENTLOGRECORD));
        SourceString = (LPWSTR) ((LPBYTE) pIoErrorLogMessage
                                      + pIoErrorLogMessage->DriverNameOffset);

        DestinationString = ElfpCopyString(DestinationString,
                                           SourceString,
                                           pIoErrorLogMessage->DriverNameLength);

        //
        // COMPUTERNAME
        //
        DestinationString = ElfpCopyString(DestinationString,
                                           LocalComputerName,
                                           ComputerNameLength);

        //
        // STRINGS
        //
        DestinationString = ElfpCopyString(DestinationString, pwStart, StringLength);

        //
        // BINARY DATA
        //
        BinaryData = (LPBYTE) DestinationString;

        RtlMoveMemory(BinaryData, 
                      &pIoErrorLogMessage->EntryData,
                      FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) 
                          + pIoErrorLogMessage->EntryData.DumpDataSize);

        //
        // LENGTH at end of record
        //
        pEndLength = (PULONG) ((LPBYTE) EventLogRecord + RecordLength - sizeof(ULONG));
        *pEndLength = RecordLength;

        //
        // Set up request packet.
        // Link event log record into the request structure.
        //
        Request.Module  = SystemModule;
        Request.LogFile = Request.Module->LogFile;
        Request.Command = ELF_COMMAND_WRITE;

        Request.Pkt.WritePkt->Buffer   = (PVOID) EventLogRecord;
        Request.Pkt.WritePkt->Datasize = RecordLength;

        //
        // Perform the operation
        //
        ElfPerformRequest( &Request );

        //
        // Replicate the event if part of a cluster
        //
        ElfpReplicateEvent(SystemModule, EventLogRecord, RecordLength);

        //
        // Free up the buffer
        //
        ElfpFreeBuffer(EventLogRecord);

        status = Request.Status;                // Set status of WRITE
    }
    else
    {
        ELF_LOG0(ERROR,
                 "ElfProcessIoLPCPacket: Unable to allocate memory for EventLogRecord\n");

        status = STATUS_NO_MEMORY;
    }

    return status;
}



NTSTATUS
ElfProcessSmLPCPacket(
    ULONG PacketLength,
    PSM_ERROR_LOG_MESSAGE SmErrorLogMessage
    )

/*++

Routine Description:

    This routine takes the packet received from the LPC port and processes it.
    The packet is an SM_ERROR_LOG_MESSAGE.  The logfile will be system, the 
    module name will be SMSS, the SID will always be NULL and
    there will always be one string, which will be the filename

    It extracts the information from the LPC packet, and then calls the
    common routine to do the work of formatting the data into
    an event record and writing it out to the log file.


Arguments:

    SmErrorLogMessage - Pointer to the data portion of the packet just
                        received through the LPC port.


Return Value:

    Status of this operation.

--*/

{
    NTSTATUS status;
    ELF_REQUEST_RECORD  Request;
    WRITE_PKT WritePkt;

    ULONG RecordLength;
    PEVENTLOGRECORD EventLogRecord;
    LPWSTR DestinationString, SourceString;
    PBYTE BinaryData;
    ULONG PadSize;
    LARGE_INTEGER Time;
    ULONG TimeWritten;
    PULONG pEndLength;
	WCHAR LocalComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	ULONG  ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
	BOOL bOK;

    try
    {
    	// Get the computer name

		bOK = GetComputerNameW(LocalComputerName, &ComputerNameLength);
		if(bOK == FALSE)
		{
        	ELF_LOG1(ERROR,
                 "ElfProcessIoLPCPacket: failed calling GetComputerNameW, last error 0x%x\n",
                  GetLastError());
            return STATUS_UNSUCCESSFUL;    
		}
    	ComputerNameLength = (ComputerNameLength+1)*sizeof(WCHAR);
        //
        //  Validate the packet.  
        //
        if (PacketLength < sizeof(SM_ERROR_LOG_MESSAGE) 

                ||
            
            // 
            //  Offset begins before header
            //
             
            SmErrorLogMessage->StringOffset < sizeof(*SmErrorLogMessage)
                
                ||

            //
            //  Offset begins after packet
            //

            SmErrorLogMessage->StringOffset >= PacketLength

                ||

            //
            //  Length of string longer than packet
            //

            SmErrorLogMessage->StringLength > PacketLength

                ||

            //
            //  String end after end of packet
            //

            SmErrorLogMessage->StringOffset
                + SmErrorLogMessage->StringLength > PacketLength

             )
        {
            RtlRaiseStatus(STATUS_UNSUCCESSFUL);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        //
        // It's a bad packet, log it and return
        //
        ELF_LOG1(ERROR,
                 "ElfProcessSmLPCPacket: Exception %#x caught processing SMSS LPC packet\n",
                 GetExceptionCode());

        ELF_LOG3(ERROR,
                 "SmErrorLogMessage->StringOffset %#x\n"
                     "\tPacketLength %#x\n"
                     "\tSmErrorLogMessage->StringLength %#x\n",
                 SmErrorLogMessage->StringOffset,
                 PacketLength,
                 SmErrorLogMessage->StringLength);

        ElfpCreateElfEvent(EVENT_BadDriverPacket,
                           EVENTLOG_ERROR_TYPE,
                           0,                    // EventCategory
                           0,                    // NumberOfStrings
                           NULL,                 // Strings
                           NULL,                 // Data
                           0,                    // Datalength
                           0,
                           FALSE);                   // flags

        return STATUS_UNSUCCESSFUL;
    }

    //
    // Set up write packet in request packet
    //
    Request.Pkt.WritePkt = &WritePkt;
    Request.Flags = 0;

    //
    // Generate any additional information needed in the record.
    //

    //
    //  Determine how big a buffer is needed for the eventlog record.  
    //  We overestimate string lengths rather than probing for 
    //  terminating NUL's
    //
    RecordLength = sizeof(EVENTLOGRECORD)
                       + sizeof(L"system")
                       + ComputerNameLength + sizeof(WCHAR)
                       + SmErrorLogMessage->StringLength + sizeof(WCHAR)
                       + sizeof(RecordLength);

    //
    //  Since the RecordLength at the end must be ULONG aligned, we round 
    //  up the total size to be ULONG aligned.
    //
    RecordLength += sizeof(ULONG) - (RecordLength % sizeof(ULONG));

    //
    // Allocate the buffer for the Eventlog record
    //
    EventLogRecord = (PEVENTLOGRECORD) ElfpAllocateBuffer(RecordLength);

    if (EventLogRecord == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfProcessSmLPCPacket: Unable to allocate memory for EventLogRecord\n");

        return STATUS_NO_MEMORY;
    }

    //
    // Fill up the event record
    //
    EventLogRecord->Length   = RecordLength;
    EventLogRecord->Reserved = ELF_LOG_FILE_SIGNATURE;

    RtlTimeToSecondsSince1970(&SmErrorLogMessage->TimeStamp,
                              &EventLogRecord->TimeGenerated);

    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time, &EventLogRecord->TimeWritten);
    EventLogRecord->EventID = SmErrorLogMessage->Status;

    //
    // set EventType based on the high order nibble of
    // the eventID
    //
    if (NT_INFORMATION(EventLogRecord->EventID))
    {
        EventLogRecord->EventType =  EVENTLOG_INFORMATION_TYPE;
    }
    else if (NT_WARNING(EventLogRecord->EventID))
    {
        EventLogRecord->EventType =  EVENTLOG_WARNING_TYPE;
    }
    else if (NT_ERROR(EventLogRecord->EventID))
    {
        EventLogRecord->EventType = EVENTLOG_ERROR_TYPE;
    }
    else
    {
        //
        // Unknown, set to error
        //
        ELF_LOG1(LPC,
                 "ElfProcessSmLPCPacket: Unknown EventType (high nibble of ID %#x)\n",
                 EventLogRecord->EventID);

        EventLogRecord->EventType = EVENTLOG_ERROR_TYPE;
    }

    //
    //  There is a single string;  it is the name of the file being 
    //  replaced
    //
    EventLogRecord->NumStrings    = 1;
    EventLogRecord->EventCategory = ELF_CATEGORY_SYSTEM_EVENT;

    //
    //  Nothing for ReservedFlags
    //  Nothing for ClosingRecordNumber
    //
    EventLogRecord->StringOffset = sizeof(EVENTLOGRECORD) 
                                       + sizeof( L"system" )
                                       + ComputerNameLength;

    //
    //  No SID's present
    //
    EventLogRecord->UserSidLength = 0;
    EventLogRecord->UserSidOffset = 0;
    EventLogRecord->DataLength    = 0;
    EventLogRecord->DataOffset    = 0;

    //
    // Fill in the variable-length fields
    //
    // MODULENAME
    //
    // SMSS
    //
    DestinationString = (LPWSTR) ((LPBYTE) EventLogRecord + sizeof(EVENTLOGRECORD));

    DestinationString = ElfpCopyString(DestinationString, 
                                       L"system", 
                                       sizeof(L"system"));

    //
    // COMPUTERNAME
    //
    DestinationString = ElfpCopyString(DestinationString,
                                       LocalComputerName,
                                       ComputerNameLength);

    //
    // STRING
    //
    SourceString = (LPWSTR) ((LPBYTE) SmErrorLogMessage + SmErrorLogMessage->StringOffset);

    ELF_LOG2(LPC,
             "ElfProcessSmLPCPacket: String is '%*ws'\n",
             SmErrorLogMessage->StringLength,
             SourceString);

    DestinationString = ElfpCopyString(DestinationString,
                                       SourceString,
                                       SmErrorLogMessage->StringLength);

    //
    // LENGTH at end of record
    //
    pEndLength = (PULONG) ((LPBYTE) EventLogRecord + RecordLength - sizeof(ULONG));
    *pEndLength = RecordLength;

    //
    // Set up request packet.
    // Link event log record into the request structure.
    //
    Request.Module  = SystemModule;
    Request.LogFile = Request.Module->LogFile;
    Request.Command = ELF_COMMAND_WRITE;

    Request.Pkt.WritePkt->Buffer   = (PVOID) EventLogRecord;
    Request.Pkt.WritePkt->Datasize = RecordLength;

    //
    // Perform the operation
    //
    ElfPerformRequest( &Request );


    //
    // Replicate the event if part of a cluster
    //
    ElfpReplicateEvent(SystemModule, EventLogRecord, RecordLength);

    //
    // Free up the buffer
    //
    ElfpFreeBuffer( EventLogRecord );

    return Request.Status;
}



NTSTATUS
ElfProcessLPCCalls(
    VOID
    )

/*++

Routine Description:

    This routine waits for messages to come through the LPC port to
    the system thread. When one does, it calls the appropriate routine to
    handle the API, then replies to the system thread indicating that the
    call has completed if the message was a request, if it was a datagram,
    it just waits for the next message.

Arguments:


Return Value:

--*/

{
    NTSTATUS status;

    BOOL SendReply = FALSE;

    ELF_REPLY_MESSAGE   replyMessage;
    PELF_PORT_MSG       receiveMessage;
    PHANDLE             PortConnectionHandle;

    //
    // Loop dispatching API requests.
    //
    receiveMessage = ElfpAllocateBuffer(ELF_PORT_MAX_MESSAGE_LENGTH + sizeof(PORT_MESSAGE));

    if (!receiveMessage)
    {
        ELF_LOG0(ERROR,
                 "ElfProcessLPCCalls: Unable to allocate memory for receiveMessage\n");

        return STATUS_NO_MEMORY;
    }

    while (TRUE)
    {
        //
        // On the first call to NtReplyWaitReceivePort, don't send a
        // reply since there's nobody to whom to reply.  However, on
        // subsequent calls send a reply to the message from the prior
        // time if that message wasn't an LPC_DATAGRAM.
        //
        status = NtReplyWaitReceivePort(
                                       ElfConnectionPortHandle,
                     (PVOID)           &PortConnectionHandle,
                     (PPORT_MESSAGE)   (SendReply ? &replyMessage : NULL),
                     (PPORT_MESSAGE)   receiveMessage
                 );

        if (!NT_SUCCESS(status))
        {
            ELF_LOG1(ERROR,
                     "ElfProcessLPCCalls: NtReplyWaitReceivePort failed %#x\n",
                     status);

            return status;
        }

        ELF_LOG0(LPC,
                 "ElfProcessLPCCalls: Received message\n");

        //
        // Take the record received and perform the operation.  Strip off
        // the PortMessage and just send the packet.
        //

        //
        // Set up the response message to be sent on the next call to
        // NtReplyWaitReceivePort if this wasn't a datagram.
        // 'status' contains the status to return from this call.
        // Only process messages that are LPC_REQUEST or LPC_DATAGRAM
        //
        if (receiveMessage->PortMessage.u2.s2.Type == LPC_REQUEST
             ||
            receiveMessage->PortMessage.u2.s2.Type == LPC_DATAGRAM)
        {
            ELF_LOG1(LPC,
                     "ElfProcessLPCCalls: LPC message type = %ws\n",
                     (receiveMessage->PortMessage.u2.s2.Type == LPC_REQUEST ? "LPC_REQUEST" :
                                                                              "LPC_DATAGRAM"));

            if (receiveMessage->MessageType == IO_ERROR_LOG)
            {
                ELF_LOG0(LPC,
                         "ElfProcessLPCCalls: SM_IO_LOG\n");

                status =  ElfProcessIoLPCPacket(receiveMessage->PortMessage.u1.s1.DataLength, 
                                                &receiveMessage->u.IoErrorLogMessage);
            }
            else if (receiveMessage->MessageType == SM_ERROR_LOG)
            {
                ELF_LOG0(LPC,
                         "ElfProcessLPCCalls: SM_ERROR_LOG\n");

                status = ElfProcessSmLPCPacket(receiveMessage->PortMessage.u1.s1.DataLength, 
                                               &receiveMessage->u.SmErrorLogMessage);
            }
            else
            {
                ELF_LOG1(ERROR,
                         "ElfProcessLPCCalls: Unknown MessageType %#x\n",
                         receiveMessage->MessageType);
                status = STATUS_UNSUCCESSFUL;
            }

            if (receiveMessage->PortMessage.u2.s2.Type == LPC_REQUEST)
            {
                replyMessage.PortMessage.u1.s1.DataLength  = sizeof(replyMessage)
                                                                 - sizeof(PORT_MESSAGE);

                replyMessage.PortMessage.u1.s1.TotalLength = sizeof(replyMessage);
                replyMessage.PortMessage.u2.ZeroInit       = 0;

                replyMessage.PortMessage.ClientId 
                    = receiveMessage->PortMessage.ClientId;

                replyMessage.PortMessage.MessageId 
                    = receiveMessage->PortMessage.MessageId;

                replyMessage.Status = status;
    
                SendReply = TRUE;
            }
            else
            {
                SendReply = FALSE;
            }
        }
        else if (receiveMessage->PortMessage.u2.s2.Type == LPC_CONNECTION_REQUEST)
        {
            PHANDLE pSavedHandle = NULL;
            BOOLEAN Accept       = TRUE;

            ELF_LOG0(LPC,
                     "ElfProcessLPCCalls: Processing connection request\n");

            pSavedHandle = ElfpAllocateBuffer(sizeof (HANDLE));

            if (pSavedHandle)
            {
                status = NtAcceptConnectPort(pSavedHandle,
                                             pSavedHandle,
                                             &receiveMessage->PortMessage,
                                             Accept,
                                             NULL,
                                             NULL);
            } else {

               ELF_LOG0(ERROR, "ElfProcessLPCCalls: Unable to allocate LPC handle\n");
               status = STATUS_NO_MEMORY;

            }

            if (!Accept)
            {
                if(pSavedHandle)
                {
                  ElfpFreeBuffer(pSavedHandle);
                  pSavedHandle = NULL;
                }

                continue;
            }

            if (NT_SUCCESS(status))
            {
                status = NtCompleteConnectPort(*pSavedHandle);

                if (!NT_SUCCESS(status))
                {
                    ELF_LOG1(ERROR,
                             "ElfProcessLPCCalls: NtAcceptConnectPort failed %#x\n",
                             status);

                    NtClose(*pSavedHandle);
                }
            }

            if (!NT_SUCCESS(status))
            {
                ELF_LOG1(ERROR,
                         "ElfProcessLPCCalls: Cleaning up failed connect\n", status);

                if(pSavedHandle)
                {
                  ElfpFreeBuffer(pSavedHandle);
                  pSavedHandle = NULL;
                }
            }
        }
        else if (receiveMessage->PortMessage.u2.s2.Type == LPC_PORT_CLOSED)
        {
            ELF_LOG0(LPC,
                     "ElfProcessLPCCalls: Processing port closed\n");

            ASSERT(PortConnectionHandle != NULL);

            NtClose(*PortConnectionHandle);
            ElfpFreeBuffer(PortConnectionHandle);


        }
        else
        {
            //
            // We received a message type we didn't expect, probably due to
            // error.
            //
            ELF_LOG1(ERROR,
                     "ElfProcessLPCCalls: Unknown message type %#x received on LPC port\n",
                     receiveMessage->PortMessage.u2.s2.Type);
        }
    }

} // ElfProcessLPCCalls



DWORD
MainLPCThread(
    LPVOID      LPCThreadParm
    )

/*++

Routine Description:

    This is the main thread that monitors the LPC port from the I/O system.
    It takes care of creating the LPC port, and waiting for input, which
    it then transforms into the right operation on the event log.


Arguments:

    NONE

Return Value:

    NONE

--*/

{
    NTSTATUS    Status;

    ELF_LOG0(LPC,
             "MainLPCThread: Inside LPC thread\n");

    Status = SetUpLPCPort();

    if (NT_SUCCESS(Status))
    {
        //
        // Loop forever. This thread will be killed when the service terminates.
        //
        while (TRUE)
        {
            Status = ElfProcessLPCCalls ();
        }
    }

    ELF_LOG1(ERROR,
             "MainLPCThread: SetUpLPCPort failed %#x\n",
             Status);

    return Status;

    UNREFERENCED_PARAMETER(LPCThreadParm);
}



BOOL
StartLPCThread(
    VOID
    )

/*++

Routine Description:

    This routine starts up the thread that monitors the LPC port.

Arguments:

    NONE

Return Value:

    TRUE if thread creation succeeded, FALSE otherwise.

Note:


--*/
{
    DWORD       error;
    DWORD       ThreadId;

    ELF_LOG0(LPC,
             "StartLPCThread: Start up the LPC thread\n");

    //
    // Start up the actual thread.
    //
    LPCThreadHandle = CreateThread(NULL,               // lpThreadAttributes
                                   4096,               // dwStackSize
                                   MainLPCThread,      // lpStartAddress
                                   NULL,               // lpParameter
                                   0L,                 // dwCreationFlags
                                   &ThreadId);         // lpThreadId

    if (LPCThreadHandle == NULL)
    {
        error = GetLastError();

        ELF_LOG1(ERROR,
                 "MainLPCThread: CreateThread failed %d\n",
                 error);

        return FALSE;
    }

    return TRUE;
}
