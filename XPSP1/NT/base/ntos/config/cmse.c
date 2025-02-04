/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    cmse.c

Abstract:

    This module implements security routines for the configuration manager.

Author:

    John Vert (jvert) 20-Jan-1992

Revision History:

    Richard Ward (richardw) 14-Apr-1992  Changed ACE_HEADER

--*/
#include "cmp.h"


//
// Function prototypes private to this module
//

//
// Dragos: modified to use the security cache
//
BOOLEAN
CmpFindMatchingDescriptorCell(
    IN PCMHIVE CmHive,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Type,
    OUT PHCELL_INDEX MatchingCell,
    OUT OPTIONAL PCM_KEY_SECURITY_CACHE *CachedSecurityPointer
    );

////////////////
NTSTATUS
CmpSetSecurityDescriptorInfo(
    IN PCM_KEY_CONTROL_BLOCK kcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

NTSTATUS
CmpQuerySecurityDescriptorInfo(
    IN PCM_KEY_CONTROL_BLOCK kcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    );

PCM_KEY_SECURITY
CmpGetKeySecurity(
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Key,
    OUT PHCELL_INDEX SecurityCell OPTIONAL
    );

NTSTATUS
CmpGetObjectSecurity(
    IN HCELL_INDEX Cell,
    IN PHHIVE Hive,
    OUT PCM_KEY_SECURITY *Security,
    OUT PHCELL_INDEX SecurityCell OPTIONAL
    );

BOOLEAN
CmpInsertSecurityCellList(
    IN PHHIVE Hive,
    IN HCELL_INDEX NodeCell,
    IN HCELL_INDEX SecurityCell
    );

VOID
CmpRemoveSecurityCellList(
    IN PHHIVE Hive,
    IN HCELL_INDEX SecurityCell
    );

ULONG
CmpSecurityExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    );

//
// This macro takes a PSECURITY_DESCRIPTOR and returns the size of the
// hive cell required to contain the entire security descriptor.
//

#define SECURITY_CELL_LENGTH(pDescriptor) \
    FIELD_OFFSET(CM_KEY_SECURITY,Descriptor) + \
    RtlLengthSecurityDescriptor(pDescriptor)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpSecurityMethod )
#pragma alloc_text(PAGE,CmpSetSecurityDescriptorInfo)
#pragma alloc_text(PAGE,CmpAssignSecurityDescriptor)
#pragma alloc_text(PAGE,CmpQuerySecurityDescriptorInfo)
#pragma alloc_text(PAGE,CmpCheckCreateAccess)
#pragma alloc_text(PAGE,CmpCheckNotifyAccess)
#pragma alloc_text(PAGE,CmpGetObjectSecurity)
#pragma alloc_text(PAGE,CmpGetKeySecurity)
#pragma alloc_text(PAGE,CmpHiveRootSecurityDescriptor)
#pragma alloc_text(PAGE,CmpFreeSecurityDescriptor)
#pragma alloc_text(PAGE,CmpInsertSecurityCellList)
#pragma alloc_text(PAGE,CmpRemoveSecurityCellList)
#pragma alloc_text(PAGE,CmpSecurityExceptionFilter)
#endif

ULONG
CmpSecurityExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    )

/*++

Routine Description:

    Debug code to find registry security exceptions that are being swallowed

Return Value:

    EXCEPTION_EXECUTE_HANDLER

--*/

{
#ifndef _CM_LDR_
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CM: Registry security exception %lx, ExceptionPointers = %p\n",
            ExceptionPointers->ExceptionRecord->ExceptionCode,
            ExceptionPointers);
#endif //_CM_LDR_
    
    //
    // This is a request from the base test team; no dbg should be hit on the free builds 
    // at the client; after RC2 is shipped we should enable this on free builds too.
    //
#if DBG
    try {
        DbgBreakPoint();
    } except (EXCEPTION_EXECUTE_HANDLER) {

        //
        // no debugger enabled, just keep going
        //

    }
#endif

    return(EXCEPTION_EXECUTE_HANDLER);
}

NTSTATUS
CmpSecurityMethod (
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )

/*++

Routine Description:

    This is the security method for registry objects.  It is responsible for
    retrieving, setting, and deleting the security descriptor of a registry
    object.  It is not used to assign the original security descriptor to an
    object (use SeAssignSecurity for that purpose).


    IT IS ASSUMED THAT THE OBJECT MANAGER HAS ALREADY DONE THE ACCESS
    VALIDATIONS NECESSARY TO ALLOW THE REQUESTED OPERATIONS TO BE PERFORMED.

Arguments:

    Object - Supplies a pointer to the object being used.

    OperationCode - Indicates if the operation is for setting, querying, or
        deleting the object's security descriptor.

    SecurityInformation - Indicates which security information is being
        queried or set.  This argument is ignored for the delete operation.

    SecurityDescriptor - The meaning of this parameter depends on the
        OperationCode:

        QuerySecurityDescriptor - For the query operation this supplies the
            buffer to copy the descriptor into.  The security descriptor is
            assumed to have been probed up to the size passed in in Length.
            Since it still points into user space, it must always be
            accessed in a try clause in case it should suddenly disappear.

        SetSecurityDescriptor - For a set operation this supplies the
            security descriptor to copy into the object.  The security
            descriptor must be captured before this routine is called.

        DeleteSecurityDescriptor - It is ignored when deleting a security
            descriptor.

        AssignSecurityDescriptor - For assign operations this is the
            security descriptor that will be assigned to the object.
            It is assumed to be in kernel space, and is therefore not
            probed or captured.

    CapturedLength - For the query operation this specifies the length, in
        bytes, of the security descriptor buffer, and upon return contains
        the number of bytes needed to store the descriptor.  If the length
        needed is greater than the length supplied the operation will fail.
        It is ignored in the set and delete operation.

        This parameter is assumed to be captured and probed as appropriate.

    ObjectsSecurityDescriptor - For the Set operation this supplies the address
        of a pointer to the object's current security descriptor.  This routine
        will either modify the security descriptor in place or deallocate/
        allocate a new security descriptor and use this variable to indicate
        its new location.  For the query operation it simply supplies
        the security descriptor being queried.

    PoolType - For the set operation this specifies the pool type to use if
        a new security descriptor needs to be allocated.  It is ignored
        in the query and delete operation.

    GenericMapping - Passed only for the set operation, this argument provides
        the mapping of generic to specific/standard access types for the object
        being accessed.  This mapping structure is expected to be safe to
        access (i.e., captured if necessary) prior to be passed to this routine.

Return Value:

    NTSTATUS - STATUS_SUCCESS if the operation is successful and an
        appropriate error status otherwise.

--*/

{
    PCM_KEY_CONTROL_BLOCK   kcb;
    NTSTATUS                Status;
    CM_KEY_REFERENCE        Key;
    PCM_KEY_NODE            TempNode;

    //
    //  Make sure the common parts of our input are proper
    //

    PAGED_CODE();
    ASSERT_KEY_OBJECT(Object);

    ASSERT( (OperationCode == SetSecurityDescriptor) ||
            (OperationCode == QuerySecurityDescriptor) ||
            (OperationCode == AssignSecurityDescriptor) ||
            (OperationCode == DeleteSecurityDescriptor) );

    //
    // Lock hive for shared or exclusive, depending on what we need
    // to do.
    //
    if (OperationCode == QuerySecurityDescriptor) {
        CmpLockRegistry();
    } else {
        CmpLockRegistryExclusive();
#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif //CHECK_REGISTRY_USECOUNT
    }

    if (((PCM_KEY_BODY)Object)->KeyControlBlock->Delete) {
        //
        // Key has been deleted, performing security operations on
        // it is Not Allowed.
        //
        CmpUnlockRegistry();
        return(STATUS_KEY_DELETED);
    }

    kcb = ((PCM_KEY_BODY)Object)->KeyControlBlock;

    try {

        //
        //  This routine simply cases off of the operation code to decide
        //  which support routine to call
        //

        switch (OperationCode) {

        case SetSecurityDescriptor:

            //
            //  check the rest of our input and call the set security
            //  method
            //
            ASSERT( (PoolType == PagedPool) || (PoolType == NonPagedPool) );

            Status = CmpSetSecurityDescriptorInfo( kcb,
                                                   SecurityInformation,
                                                   SecurityDescriptor,
                                                   ObjectsSecurityDescriptor,
                                                   PoolType,
                                                   GenericMapping );

            //
            // this is the one and only path on which a user could change
            // a security descriptor, therefore, report such changes for
            // notification here.
            //
            if (NT_SUCCESS(Status)) {
                CmpReportNotify(kcb,
                                kcb->KeyHive,
                                kcb->KeyCell,
                                REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_SECURITY);
    
            }

            break;

        case QuerySecurityDescriptor:

            //
            //  check the rest of our input and call the default query security
            //  method
            //
            ASSERT( CapturedLength != NULL );
            Status = CmpQuerySecurityDescriptorInfo( kcb,
                                                     SecurityInformation,
                                                     SecurityDescriptor,
                                                     CapturedLength,
                                                     ObjectsSecurityDescriptor );
            break;

        case DeleteSecurityDescriptor:

            //
            // Nobody should ever call the delete method.  When the key is
            // freed, the security descriptor associated with it is
            // explicitly freed (CmpFreeSecurityDescriptor)
            //
            ASSERT(FALSE);

            break;

        case AssignSecurityDescriptor:

            //
            // Set the SecurityDescriptor field in the object's header to
            // NULL.  This indicates that our security method needs to be
            // called for any security descriptor operations.
            //

            Status = ObAssignObjectSecurityDescriptor(Object, NULL, PagedPool);

            ASSERT( NT_SUCCESS( Status ));

            TempNode = (PCM_KEY_NODE)HvGetCell(kcb->KeyHive, kcb->KeyCell);
            if( TempNode == NULL ) {
                //
                // we couldn't map the bin containing this cell
                //
                Status = STATUS_INSUFFICIENT_RESOURCES;
                // step thru exit
                break;
            }
            
            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
            // release the cell right here as we are holding the reglock exclusive
            HvReleaseCell(kcb->KeyHive, kcb->KeyCell);
            //
            // Assign the actual descriptor.
            //
            Status = CmpAssignSecurityDescriptor( kcb->KeyHive,
                                                  kcb->KeyCell,
                                                  TempNode,
                                                  SecurityDescriptor );
            //
            // Security has been changed, update the cache.
            //
            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
            CmpAssignSecurityToKcb(kcb,TempNode->Security);

            break;

        default:

            //
            //  Bugcheck on any other operation code,  We won't get here if
            //  the earlier asserts are still checked.
            //
            CM_BUGCHECK( REGISTRY_ERROR,BAD_SECURITY_METHOD,1,kcb,OperationCode);

        }

    } except (CmpSecurityExceptionFilter(GetExceptionInformation())) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmpSecurityMethod: code:%08lx\n", GetExceptionCode()));
        Status = GetExceptionCode();
    }

    CmpUnlockRegistry();
    return(Status);

}

NTSTATUS
CmpSetSecurityDescriptorInfo(
    IN PCM_KEY_CONTROL_BLOCK Key,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )
/*++

Routine Description:

    This routine will set a node's security descriptor.  The input
    security descriptor must be previously captured.

Arguments:

    Key - Supplies a pointer to the KEY_CONTROL_BLOCK for the node whose
        security descriptor will be set.

    SecurityInformation - Indicates which security information is
        to be applied to the object.  The value(s) to be assigned are
        passed in the SecurityDescriptor parameter.

    ModificationDescriptor - Supplies the input security descriptor to be
        applied to the object.  The caller of this routine is expected
        to probe and capture the passed security descriptor before calling
        and release it after calling.

    ObjectsSecurityDescriptor - Supplies the address of a pointer to
        the objects security descriptor that is going to be altered by
        this procedure

    PoolType - Specifies the type of pool to allocate for the objects
        security descriptor.

    GenericMapping - This argument provides the mapping of generic to
        specific/standard access types for the object being accessed.
        This mapping structure is expected to be safe to access
        (i.e., captured if necessary) prior to be passed to this routine.

Return Value:

    NTSTATUS - STATUS_SUCCESS if successful and an appropriate error
        value otherwise

--*/

{
    NTSTATUS                Status;
    HCELL_INDEX             SecurityCell;
    HCELL_INDEX             MatchSecurityCell;
    HCELL_INDEX             NewCell;
    HCELL_INDEX             OldCell;
    PCM_KEY_SECURITY        Security;
    PCM_KEY_SECURITY        NewSecurity;
    PCM_KEY_SECURITY        FlinkSecurity;
    PCM_KEY_SECURITY        BlinkSecurity;
    PCM_KEY_NODE            Node;
    ULONG                   DescriptorLength;
    PSECURITY_DESCRIPTOR    DescriptorCopy;
    PSECURITY_DESCRIPTOR    OldDescriptorCopy;
    ULONG                   Type;
    LARGE_INTEGER           SystemTime;
    PHHIVE                  Hive;
    PCM_KEY_SECURITY_CACHE  CachedSecurity;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpSetSecurityDescriptorInfo:\n"));

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    Node = (PCM_KEY_NODE)HvGetCell(Key->KeyHive, Key->KeyCell);
    if( Node == NULL ) {
        //
        // we couldn't map the bin containing this cell;
        // this shouldn't happen as we are about to modify the cell
        // (i.e. it should be dirty/pinned by this time)
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // release the cell right here as we are holding the reglock exclusive
    HvReleaseCell(Key->KeyHive, Key->KeyCell);

    //
    // Map in the hive cell for the security descriptor before we make
    // the call to SeSetSecurityDescriptorInfo.  This prevents us from
    // changing its security descriptor and then being unable to bring
    // the hive cell into memory for updating.
    //
    Security = CmpGetKeySecurity(Key->KeyHive,
                                 Node,
                                 &SecurityCell);
    if( Security == NULL ) {
        //
        // couldn't map view inside
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // SeSetSecurityDescriptorInfo takes a pointer to the original
    // descriptor. This pointer is not freed, but a new pointer will
    // be returned.
    //
    DescriptorCopy = &Security->Descriptor;
    Status = SeSetSecurityDescriptorInfo( NULL,
                                          SecurityInformation,
                                          ModificationDescriptor,
                                          &DescriptorCopy,
                                          PoolType,
                                          GenericMapping );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    //
    // Set Security operation succeeded, so we update the security
    // descriptor in the hive.
    //
    DescriptorLength = RtlLengthSecurityDescriptor(DescriptorCopy);
    Type = HvGetCellType(Key->KeyCell);
    Hive = Key->KeyHive;

    if (! (HvMarkCellDirty(Hive, Key->KeyCell) &&
           HvMarkCellDirty(Hive, SecurityCell)))
    {
        ExFreePool(DescriptorCopy);
        return STATUS_NO_LOG_SPACE;
    }

    //
    // Try to find an existing security descriptor that we can share.
    //
    if (CmpFindMatchingDescriptorCell((PCMHIVE)Hive, DescriptorCopy, Type, &MatchSecurityCell,&CachedSecurity)) {
        //
        // A match was found.
        //
        if( MatchSecurityCell == SecurityCell ) {
            //
            // Whoops !!!; what we want to set is already here ! bail out
            // (office instalation does this !!!!)
            //
            ExFreePool(DescriptorCopy);

            //
            // Update the LastWriteTime of the key. Do we need to do that? ==> Ask John.
            //
#pragma message ("Dragos ==> John - Do we need to update the time even though nothing changed?")

            KeQuerySystemTime(&SystemTime);
            Node->LastWriteTime = SystemTime;
            // update the time in kcb too, to keep the cache in sync
            Key->KcbLastWriteTime = SystemTime;

            return STATUS_SUCCESS;
        } else {
            if (!HvMarkCellDirty(Hive, MatchSecurityCell)) {
                ExFreePool(DescriptorCopy);
                return(STATUS_NO_LOG_SPACE);
            }
            if (Security->ReferenceCount == 1) {
                //
                // No more references to the old security cell, so we can free it now.
                //
                if (! (HvMarkCellDirty(Hive, Security->Flink) &&
                       HvMarkCellDirty(Hive, Security->Blink))) {
                    ExFreePool(DescriptorCopy);
                    return(STATUS_NO_LOG_SPACE);
                }
                CmpRemoveSecurityCellList(Hive, SecurityCell);
                HvFreeCell(Hive, SecurityCell);
            } else {

                //
                // Just decrement the count on the old security cell
                //
                Security->ReferenceCount -= 1;
            }

            //
            // Set the node to point at the matching security cell.
            //
            Security = (PCM_KEY_SECURITY)HvGetCell(Hive, MatchSecurityCell);
            if( Security == NULL ) {
                //
                // we couldn't map the bin containing this cell
                // this should not happen as we just marked the cell dirty
                //
                ASSERT( FALSE );
                ExFreePool(DescriptorCopy);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            // release the cell right here as we are holding the reglock exclusive
            HvReleaseCell(Hive, MatchSecurityCell);

            Security->ReferenceCount += 1;
            Node->Security = MatchSecurityCell;
        }
    } else {

        //
        // No match was found, we need to create a new cell.
        //
        if (Security->ReferenceCount > 1) {

            //
            // We can't change the existing security cell, since it is shared
            // by multiple keys.  Allocate a new cell and decrement the existing
            // one's reference count.
            //
            NewCell = HvAllocateCell(Key->KeyHive,
                                     SECURITY_CELL_LENGTH(DescriptorCopy),
                                     Type,
                                     HCELL_NIL);
            if (NewCell == HCELL_NIL) {
                ExFreePool(DescriptorCopy);
                return(STATUS_INSUFFICIENT_RESOURCES);
            }

            if (! HvMarkCellDirty(Key->KeyHive, Security->Flink)) {
                ExFreePool(DescriptorCopy);
                return STATUS_NO_LOG_SPACE;
            }

            Security->ReferenceCount -= 1;

            //
            // Map in the new cell and insert it into the linked list.
            //
            NewSecurity = (PCM_KEY_SECURITY) HvGetCell(Key->KeyHive, NewCell);
            if( NewSecurity == NULL ) {
                //
                // we couldn't map the bin containing this cell
                //
                ExFreePool(DescriptorCopy);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            // release the cell right here as we are holding the reglock exclusive
            HvReleaseCell(Key->KeyHive, NewCell);

            NewSecurity->Blink = SecurityCell;
            NewSecurity->Flink = Security->Flink;
            FlinkSecurity = (PCM_KEY_SECURITY) HvGetCell(Key->KeyHive, Security->Flink);
            if( FlinkSecurity == NULL ) {
                //
                // we couldn't map the bin containing this cell
                //
                ExFreePool(DescriptorCopy);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            // release the cell right here as we are holding the reglock exclusive
            HvReleaseCell(Key->KeyHive, Security->Flink);

            Security->Flink = FlinkSecurity->Blink = NewCell;

            //
            // initialize new cell
            //
            NewSecurity->Signature = CM_KEY_SECURITY_SIGNATURE;
            NewSecurity->ReferenceCount = 1;
            NewSecurity->DescriptorLength = DescriptorLength;
            Security=NewSecurity;

            //
            // copy the descriptor
            //
            RtlCopyMemory( &(Security->Descriptor),
                           DescriptorCopy,
                           DescriptorLength );

            //
            // Add the new created security cell to the cache
            //
            if( !NT_SUCCESS(CmpAddSecurityCellToCache( (PCMHIVE)Key->KeyHive,NewCell,FALSE)) ) {
                //
                // we couldn't map the bin containing this cell
                // this shouldn't happen as we just allocated (marked dirty) the cell
                //
                ASSERT( FALSE );
                ExFreePool(DescriptorCopy);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            //
            // Update the pointer in the node cell.
            //
            Node->Security = NewCell;

        } else {
            //
            // when this is FALSE, the new cell is ADDED to cache;
            // Otherwise (the cell index and size did not change), 
            // the new sd is copied over the one in cache
            //
            BOOLEAN UpdateCache;

            if (DescriptorLength != Security->DescriptorLength) {

                //
                // The security descriptor's size has changed, and it is not shared
                // by any other cells, so reallocate the cell.
                //
                if (! (HvMarkCellDirty(Key->KeyHive, Security->Flink) &&
                       HvMarkCellDirty(Key->KeyHive, Security->Blink))) {
                    ExFreePool(DescriptorCopy);
                    return(STATUS_INSUFFICIENT_RESOURCES);
                }

                DCmCheckRegistry((PCMHIVE)(Key->KeyHive));
                OldCell = SecurityCell;
                SecurityCell = HvReallocateCell( Key->KeyHive,
                                                 SecurityCell,
                                                 SECURITY_CELL_LENGTH(DescriptorCopy) );
                if (SecurityCell == HCELL_NIL) {
                    ExFreePool(DescriptorCopy);
                    return(STATUS_INSUFFICIENT_RESOURCES);
                }

                //
                // remove the old cell from security cache and signal that the new one should be added
                //
                CmpRemoveFromSecurityCache ((PCMHIVE)Key->KeyHive,OldCell);
                UpdateCache = FALSE;

                //
                // Update the Node's security data.
                //
                Node->Security = SecurityCell;

                //
                // Update Security to point to where the new security object is
                //
                Security = (PCM_KEY_SECURITY) HvGetCell(Key->KeyHive, SecurityCell);
                if( Security == NULL ) {
                    //
                    // we couldn't map the bin containing this cell
                    // this shouldn't happen as we just allocated this cell
                    // (i.e. it should be pinned into memory at this point)
                    //
                    ASSERT( FALSE );
                    ExFreePool(DescriptorCopy);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                // release the cell right here as we are holding the reglock exclusive
                HvReleaseCell(Key->KeyHive, SecurityCell);

                ASSERT_SECURITY(Security);

                //
                // Update other list references to the node
                //
                if (Security->Flink == OldCell) {
                    Security->Flink = SecurityCell; // point to new self
                } else {
                    FlinkSecurity = (PCM_KEY_SECURITY) HvGetCell(
                                                            Key->KeyHive,
                                                            Security->Flink
                                                            );
                    if( FlinkSecurity == NULL ) {
                        //
                        // we couldn't map the bin containing this cell
                        //
                        ExFreePool(DescriptorCopy);
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }

                    // release the cell right here as we are holding the reglock exclusive
                    HvReleaseCell(Key->KeyHive, Security->Flink);

                    FlinkSecurity->Blink = SecurityCell;
                }

                if (Security->Blink == OldCell) {
                    Security->Blink = SecurityCell; // point to new self
                } else {
                    BlinkSecurity = (PCM_KEY_SECURITY) HvGetCell(
                                                            Key->KeyHive,
                                                            Security->Blink
                                                            );
                    if( BlinkSecurity == NULL ) {
                        //
                        // we couldn't map the bin containing this cell
                        //
                        ExFreePool(DescriptorCopy);
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }

                    // release the cell right here as we are holding the reglock exclusive
                    HvReleaseCell(Key->KeyHive,Security->Blink);

                    BlinkSecurity->Flink = SecurityCell;
                }

                //
                // Finally, update the length field in the cell
                //
                Security->DescriptorLength = DescriptorLength;
                DCmCheckRegistry((PCMHIVE)(Key->KeyHive));

            } else {

                //
                // Size hasn't changed, and it's not shared by any other cells, so
                // we can just write the new bits over the old bits.
                //

                //
                // new bits should be copied over the cached security 
                // descriptor too, to keep cache consistency
                //
                //
                // get the cached security structure for this security cell
                //
                ULONG Index;

                if( CmpFindSecurityCellCacheIndex ((PCMHIVE)Hive,SecurityCell,&Index) == FALSE ) {
                    //
                    // this cannot happen !!!
                    //
                    CM_BUGCHECK( REGISTRY_ERROR,BAD_SECURITY_CACHE,2,Key,SecurityCell);
                } 
                CachedSecurity = ((PCMHIVE)Hive)->SecurityCache[Index].CachedSecurity;

                UpdateCache = TRUE;
            }

            RtlCopyMemory( &(Security->Descriptor),
                           DescriptorCopy,
                           DescriptorLength );

            if( UpdateCache == TRUE ) {
                //
                // we just need to copy the descriptor over the existing one
                // (keep the security cache in sync !!!)
                //
                RtlCopyMemory( &(CachedSecurity->Descriptor),
                                DescriptorCopy,
                                DescriptorLength );
                //
                // recalculate the conv key and insert the sd in the proper place in the hash
                //
                CmpRemoveEntryList(&(CachedSecurity->List));
                CachedSecurity->ConvKey = CmpSecConvKey(DescriptorLength,(PULONG)(DescriptorCopy));
                InsertTailList( &(((PCMHIVE)Hive)->SecurityHash[CachedSecurity->ConvKey % CmpSecHashTableSize]),
                                &(CachedSecurity->List)
                              );

            
            } else {
                //
                // add new cell to the security cache
                //
                if( !NT_SUCCESS(CmpAddSecurityCellToCache( (PCMHIVE)Hive,SecurityCell,FALSE)) ) {
                    //
                    // we couldn't map the bin containing this cell
                    // this shouldn't happen as we just allocated (marked dirty) the cell
                    //
                    ASSERT( FALSE );
                    ExFreePool(DescriptorCopy);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }    
    }


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"\tObject's SD has been changed\n"));
    //CmpDumpSecurityDescriptor(DescriptorCopy, "NEW DESCRIPTOR\n");

    ExFreePool(DescriptorCopy);

    //
    // Update the LastWriteTime of the key.
    //
    KeQuerySystemTime(&SystemTime);
    Node->LastWriteTime = SystemTime;

    // update the time in kcb too, to keep the cache in sync
    Key->KcbLastWriteTime = SystemTime;

    //
    // Security has changed, update the cache.
    //
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    CmpAssignSecurityToKcb(Key,Node->Security);

    return(STATUS_SUCCESS);
}

NTSTATUS
CmpAssignSecurityDescriptor(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PCM_KEY_NODE Node,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

/*++

Routine Description:

    This routine assigns the given security descriptor to the specified
    node in the configuration tree.

Arguments:

    Hive - Supplies a pointer to the Hive for the node whose security
           descriptor will be assigned.

    Cell - Supplies the HCELL_INDEX of the node whose security descriptor
           will be assigned.

    Node - Supplies a pointer to the node whose security descriptor will
           be assigned.

    SecurityDescriptor - Supplies a pointer to the security descriptor to
           be assigned to the node.

    PoolType - Supplies the type of pool the SecurityDescriptor was a
           allocated from.

Return Value:

    NTSTATUS - STATUS_SUCCESS if successful and an appropriate error value
        otherwise

--*/

{
    HCELL_INDEX SecurityCell;
    PCM_KEY_SECURITY Security;
    ULONG DescriptorLength;
    ULONG Type;

    PAGED_CODE();
    //
    // Map the node that we need to assign the security descriptor to.
    //
    if (! HvMarkCellDirty(Hive, Cell)) {
        return STATUS_NO_LOG_SPACE;
    }
    ASSERT_NODE(Node);

    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

#if DBG
    {
        UNICODE_STRING Name;

        Name.MaximumLength = Name.Length = Node->NameLength;
        Name.Buffer = Node->Name;
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpAssignSecurityDescriptor: '%wZ' (H %p C %lx)\n",&Name,Hive,Cell ));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"\tSecurityCell = %lx\n",Node->Security));
    }
#endif

    ASSERT(Node->Security==HCELL_NIL);

    //
    // This is a CreateKey, so the registry node has just been created and
    // the security descriptor we have been passed needs to be associated
    // with the new registry node and inserted into the hive.
    //
    //CmpDumpSecurityDescriptor(SecurityDescriptor, "ASSIGN DESCRIPTOR\n");

    //
    // Try to find an existing security descriptor that matches this one.
    // If successful, then we don't need to allocate a new cell, we can
    // just point to the existing one and increment its reference count.
    //
    Type = HvGetCellType(Cell);
    if (!CmpFindMatchingDescriptorCell( (PCMHIVE)Hive,
                                        SecurityDescriptor,
                                        Type,
                                        &SecurityCell,
                                        NULL)) {
        //
        // No matching descriptor found, allocate and initialize a new one.
        //
        SecurityCell = HvAllocateCell(Hive,
                                      SECURITY_CELL_LENGTH(SecurityDescriptor),
                                      Type,
                                      HCELL_NIL);
        if (SecurityCell == HCELL_NIL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        //
        // Map the security cell
        //
        Security = (PCM_KEY_SECURITY) HvGetCell(Hive, SecurityCell);
        if( Security == NULL ) {
            //
            // we couldn't map the bin containing this cell
            // this shouldn't happen as we just allocated this cell
            // (i.e. it should be PINNED into memory at this point)
            //
            ASSERT( FALSE );
            HvFreeCell(Hive, SecurityCell);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        // release the cell right here as we are holding the reglock exclusive
        HvReleaseCell(Hive, SecurityCell);

        //
        // Initialize the security cell
        //
        DescriptorLength = RtlLengthSecurityDescriptor(SecurityDescriptor);

        Security->Signature = CM_KEY_SECURITY_SIGNATURE;
        Security->ReferenceCount = 1;
        Security->DescriptorLength = DescriptorLength;
        RtlCopyMemory( &(Security->Descriptor),
                       SecurityDescriptor,
                       DescriptorLength );

        //
        // Insert the new security descriptor into the list of security
        // cells; takes care of cache too
        //
        if (!CmpInsertSecurityCellList(Hive,Cell,SecurityCell))
        {
            HvFreeCell(Hive, SecurityCell);
            return STATUS_NO_LOG_SPACE;
        }

    } else {

        //
        // Found identical descriptor already existing.  Map it in and
        // increment its reference count.
        //
        if (! HvMarkCellDirty(Hive, SecurityCell)) {
            return STATUS_NO_LOG_SPACE;
        }
        Security = (PCM_KEY_SECURITY) HvGetCell(Hive, SecurityCell);
        if( Security == NULL ) {
            //
            // we couldn't map the bin containing this cell
            // this shouldn't happen as we just marked the cell dirty
            // (dirty means PIN !)
            //
            ASSERT( FALSE );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        // release the cell right here as we are holding the reglock exclusive
        HvReleaseCell(Hive, SecurityCell);

        Security->ReferenceCount += 1;
    }

    //
    // Initialize the reference in the node cell
    //
    Node->Security = SecurityCell;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"\tSecurityCell = %lx\n",Node->Security));

    return(STATUS_SUCCESS);
}


NTSTATUS
CmpQuerySecurityDescriptorInfo(
    IN PCM_KEY_CONTROL_BLOCK kcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    )

/*++

Routine Description:

    This routine will extract the desired information from the
    passed security descriptor and return the information in
    the passed buffer as a security descriptor in absolute format.

Arguments:

    Key - Supplies a pointer to the CM_KEY_REFERENCE for the node whose
        security descriptor will be deleted.

    SecurityInformation - Specifies what information is being queried.

    SecurityDescriptor - Supplies the buffer to output the requested
        information into.

        This buffer has been probed only to the size indicated by
        the Length parameter.  Since it still points into user space,
        it must always be accessed in a try clause.

    Length - Supplies the address of a variable containing the length of
        the security descriptor buffer.  Upon return this variable will
        contain the length needed to store the requested information.

    ObjectsSecurityDescriptor - Supplies the address of a pointer to
        the objects security descriptor.  The passed security descriptor
        must be in self-relative format.


Return Value:

    NTSTATUS - STATUS_SUCCESS if successful and an appropriate error value
        otherwise

Note:
    
      In the new implementation this function looks just in the security cache

--*/

{
    NTSTATUS                Status;
    PSECURITY_DESCRIPTOR    CellSecurityDescriptor;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpQuerySecurityDescriptorInfo:\n"));



    CellSecurityDescriptor = &(kcb->CachedSecurity->Descriptor);

    Status = SeQuerySecurityDescriptorInfo( SecurityInformation,
                                            SecurityDescriptor,
                                            Length,
                                            &CellSecurityDescriptor );

    return Status;
}


BOOLEAN
CmpCheckCreateAccess(
    IN PUNICODE_STRING RelativeName,
    IN PSECURITY_DESCRIPTOR Descriptor,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE PreviousMode,
    IN ACCESS_MASK AdditionalAccess,
    OUT PNTSTATUS AccessStatus
    )

/*++

Routine Description:

    This routine checks to see if we are allowed to create a sub-key in the
    given key, and performs auditing as appropriate.

Arguments:

    RelativeName - Supplies the relative name of the key being created.

    Descriptor - Supplies the security descriptor of the key in which
        the sub-key is to be created.

    CreateAccess - The access mask corresponding to create access for
        this directory type.

    AccessState - Checks for traverse access will typically be incidental
        to some other access attempt.  Information on the current state of
        that access attempt is required so that the constituent access
        attempts may be associated with each other in the audit log.

    PreviousMode - The previous processor mode.

    AdditionalAccess - access rights in addition to KEY_CREATE_SUB_KEY
            that are required.  (e.g. KEY_CREATE_LINK)

    AccessStatus - Pointer to a variable to return the status code of the
        access attempt.  In the case of failure this status code must be
        propagated back to the user.

Return Value:

    BOOLEAN - TRUE if access is allowed and FALSE otherwise.  AccessStatus
    contains the status code to be passed back to the caller.  It is not
    correct to simply pass back STATUS_ACCESS_DENIED, since this will have
    to change with the advent of mandatory access control.

--*/

{
    BOOLEAN AccessAllowed;
    ACCESS_MASK GrantedAccess = 0;
    BOOLEAN AuditPerformed = FALSE;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpCheckCreateAccess:\n"));

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

    AccessAllowed = SeAccessCheck(
                        Descriptor,
                        &AccessState->SubjectSecurityContext,
                        TRUE,                              // Token is read locked
                        (KEY_CREATE_SUB_KEY | AdditionalAccess),
                        0,
                        NULL,
                        &CmpKeyObjectType->TypeInfo.GenericMapping,
                        PreviousMode,
                        &GrantedAccess,
                        AccessStatus
                        );

    SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"Create access %s\n",AccessAllowed ? "granted" : "denied"));
/*
#if DBG
    if (!AccessAllowed) {
        CmpDumpSecurityDescriptor(Descriptor, "DENYING DESCRIPTOR");
    }
#endif
*/
    return(AccessAllowed);
}


BOOLEAN
CmpCheckNotifyAccess(
    IN PCM_NOTIFY_BLOCK NotifyBlock,
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Node
    )
/*++

Routine Description:

    Check whether the subject process/thread/user specified by the
    security data in the NotifyBlock has required access to the
    key specified by Hive.Cell.

Arguments:

    NotifyBlock - pointer to structure that describes the notify
                  operation, including the identity of the subject
                  that opened the notify.

    Hive - Supplies pointer to hive containing Node.

    Node - Supplies pointer to key of interest.

Return Value:

    TRUE if RequiredAccess is in fact possessed by the subject,
    else FALSE.

Note:

    In the new implementation get the sd from the security cache.

--*/
{
    PCM_KEY_SECURITY        Security;
    PSECURITY_DESCRIPTOR    SecurityDescriptor;
    BOOLEAN                 AccessAllowed;
    NTSTATUS                Status;
    ACCESS_MASK             GrantedAccess = 0;
    ULONG                   Index;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpCheckAccessForNotify:\n"));

    if( CmpFindSecurityCellCacheIndex ((PCMHIVE)Hive,Node->Security,&Index) == FALSE ) {
        return FALSE;
    }


    SeLockSubjectContext( &NotifyBlock->SubjectContext );

    SecurityDescriptor = &(((PCMHIVE)Hive)->SecurityCache[Index].CachedSecurity->Descriptor);


    AccessAllowed = SeAccessCheck( SecurityDescriptor,
                                   &NotifyBlock->SubjectContext,
                                   TRUE,
                                   KEY_NOTIFY,
                                   0,
                                   NULL,
                                   &CmpKeyObjectType->TypeInfo.GenericMapping,
                                   UserMode,
                                   &GrantedAccess,
                                   &Status );

    SeUnlockSubjectContext( &NotifyBlock->SubjectContext );

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"Notify access %s\n",AccessAllowed ? "granted" : "denied"));
/*
#if DBG
    if (!AccessAllowed) {
        CmpDumpSecurityDescriptor(SecurityDescriptor, "DENYING DESCRIPTOR");
    }
#endif
*/
    return AccessAllowed;
}


NTSTATUS
CmpGetObjectSecurity(
    IN HCELL_INDEX Cell,
    IN PHHIVE Hive,
    OUT PCM_KEY_SECURITY *Security,
    OUT PHCELL_INDEX SecurityCell OPTIONAL
    )

/*++

Routine Description:

    This routine maps in the security cell of a registry object.

Arguments:

    Cell - Supplies the cell index of the object.

    Hive - Supplies the hive the object's cell is in.

    Security - Returns a pointer to the security cell of the object.

    SecurityCell - Returns the index of the security cell

Return Value:

    NTSTATUS.

--*/

{
    HCELL_INDEX CellIndex;
    PCM_KEY_NODE Node;

    PAGED_CODE();
    //
    // Map the node we need to get the security descriptor for
    //
    Node = (PCM_KEY_NODE) HvGetCell(Hive, Cell);

    if( Node == NULL ) {
        //
        // we couldn't map the bin containing this cell
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

#if DBG
    {
        UNICODE_STRING Name;

        Name.MaximumLength = Name.Length = Node->NameLength;
        Name.Buffer = Node->Name;
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpGetObjectSecurity for: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"%wZ\n", &Name));
    }
#endif

    *Security = CmpGetKeySecurity(Hive,Node,SecurityCell);

    HvReleaseCell(Hive, Cell);

    if( *Security == NULL ) {
        //
        // couldn't map view inside
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

PCM_KEY_SECURITY
CmpGetKeySecurity(
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Key,
    OUT PHCELL_INDEX SecurityCell OPTIONAL
    )

/*++

Routine Description:

    This routine returns the security of a registry key.

Arguments:

    Hive - Supplies the hive the object's cell is in.

    Key - Supplies a pointer to the key node.

    SecurityCell - Returns the index of the security cell

Return Value:

    Returns a pointer to the security cell of the object
    
    NULL, if resources problem
--*/

{
    HCELL_INDEX CellIndex;
    PCM_KEY_SECURITY Security;

    PAGED_CODE();

    ASSERT(Key->Signature == CM_KEY_NODE_SIGNATURE);
    ASSERT_NODE(Key);

#if DBG
    {
        UNICODE_STRING Name;

        Name.MaximumLength = Name.Length = Key->NameLength;
        Name.Buffer = Key->Name;
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpGetObjectSecurity for: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"%wZ\n", &Name));
    }
#endif

    CellIndex = Key->Security;

    //
    // Map in the security descriptor cell
    //
    Security = (PCM_KEY_SECURITY) HvGetCell(Hive, CellIndex);
    if( Security == NULL ) {
        //
        // we couldn't map the bin containing this cell
        //
        return NULL;
    }
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    HvReleaseCell(Hive, CellIndex);
    ASSERT_SECURITY(Security);

    if (ARGUMENT_PRESENT(SecurityCell)) {
        *SecurityCell = CellIndex;
    }

    return(Security);
}

PSECURITY_DESCRIPTOR
CmpHiveRootSecurityDescriptor(
    VOID
    )
/*++

Routine Description:

    This routine allocates and initializes the default security descriptor
    for a system-created registry key.

    The caller is responsible for freeing the allocated security descriptor
    when he is done with it.

Arguments:

    None

Return Value:

    Pointer to an initialized security descriptor if successful.

    Bugcheck otherwise.

--*/

{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR SecurityDescriptor=NULL;
    PACL Acl=NULL;
    PACL AclCopy;
    PSID WorldSid=NULL;
    PSID RestrictedSid=NULL;
    PSID SystemSid=NULL;
    PSID AdminSid=NULL;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    ULONG AceLength;
    ULONG AclLength;
    PACE_HEADER AceHeader;

    PAGED_CODE();

    //
    // Allocate and initialize the SIDs we will need.
    //
    WorldSid  = ExAllocatePool(PagedPool, RtlLengthRequiredSid(1));
    RestrictedSid  = ExAllocatePool(PagedPool, RtlLengthRequiredSid(1));
    SystemSid = ExAllocatePool(PagedPool, RtlLengthRequiredSid(1));
    AdminSid  = ExAllocatePool(PagedPool, RtlLengthRequiredSid(2));
    if ((WorldSid  == NULL) ||
        (RestrictedSid == NULL) ||
        (SystemSid == NULL) ||
        (AdminSid  == NULL)) {

        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 1, 0, 0);
    }

    if ((!NT_SUCCESS(RtlInitializeSid(WorldSid, &WorldAuthority, 1))) ||
        (!NT_SUCCESS(RtlInitializeSid(RestrictedSid, &NtAuthority, 1))) ||
        (!NT_SUCCESS(RtlInitializeSid(SystemSid, &NtAuthority, 1))) ||
        (!NT_SUCCESS(RtlInitializeSid(AdminSid, &NtAuthority, 2)))) {
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 2, 0, 0);
    }

    *(RtlSubAuthoritySid(WorldSid, 0)) = SECURITY_WORLD_RID;

    *(RtlSubAuthoritySid(RestrictedSid, 0)) = SECURITY_RESTRICTED_CODE_RID;

    *(RtlSubAuthoritySid(SystemSid, 0)) = SECURITY_LOCAL_SYSTEM_RID;

    *(RtlSubAuthoritySid(AdminSid, 0)) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid(AdminSid, 1)) = DOMAIN_ALIAS_RID_ADMINS;

    ASSERT(RtlValidSid(WorldSid));
    ASSERT(RtlValidSid(RestrictedSid));
    ASSERT(RtlValidSid(SystemSid));
    ASSERT(RtlValidSid(AdminSid));

    //
    // Compute the size of the ACE list
    //

    AceLength = (SeLengthSid(WorldSid)  -
                 sizeof(ULONG)          +
                 sizeof(ACCESS_ALLOWED_ACE))
              + (SeLengthSid(RestrictedSid)  -
                 sizeof(ULONG)          +
                 sizeof(ACCESS_ALLOWED_ACE))
              + (SeLengthSid(SystemSid) -
                 sizeof(ULONG)          +
                 sizeof(ACCESS_ALLOWED_ACE))
              + (SeLengthSid(AdminSid)  -
                 sizeof(ULONG)          +
                 sizeof(ACCESS_ALLOWED_ACE));

    //
    // Allocate and initialize the ACL
    //

    AclLength = AceLength + sizeof(ACL);
    Acl = ExAllocatePool(PagedPool, AclLength);
    if (Acl == NULL) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: couldn't allocate ACL\n"));

        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 3, 0, 0);
    }

    Status = RtlCreateAcl(Acl, AclLength, ACL_REVISION);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: couldn't initialize ACL\n"));
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 4, Status, 0);
    }

    //
    // Now add the ACEs to the ACL
    //
    Status = RtlAddAccessAllowedAce(Acl,
                                    ACL_REVISION,
                                    KEY_ALL_ACCESS,
                                    SystemSid);
    if (NT_SUCCESS(Status)) {
        Status = RtlAddAccessAllowedAce(Acl,
                                        ACL_REVISION,
                                        KEY_ALL_ACCESS,
                                        AdminSid);
    }
    if (NT_SUCCESS(Status)) {
        Status = RtlAddAccessAllowedAce(Acl,
                                        ACL_REVISION,
                                        KEY_READ,
                                        WorldSid);
    }
    if (NT_SUCCESS(Status)) {
        Status = RtlAddAccessAllowedAce(Acl,
                                        ACL_REVISION,
                                        KEY_READ,
                                        RestrictedSid);
    }
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: RtlAddAce failed status %08lx\n", Status));

        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 5, Status, 0);
    }

    //
    // Make the ACEs inheritable
    //
    Status = RtlGetAce(Acl,0,&AceHeader);
    ASSERT(NT_SUCCESS(Status));
    AceHeader->AceFlags |= CONTAINER_INHERIT_ACE;

    Status = RtlGetAce(Acl,1,&AceHeader);
    ASSERT(NT_SUCCESS(Status));
    AceHeader->AceFlags |= CONTAINER_INHERIT_ACE;

    Status = RtlGetAce(Acl,2,&AceHeader);
    ASSERT(NT_SUCCESS(Status));
    AceHeader->AceFlags |= CONTAINER_INHERIT_ACE;

    Status = RtlGetAce(Acl,3,&AceHeader);
    ASSERT(NT_SUCCESS(Status));
    AceHeader->AceFlags |= CONTAINER_INHERIT_ACE;
    //
    // We are finally ready to allocate and initialize the security descriptor
    // Allocate enough space to hold both the security descriptor and the
    // ACL.  This allows us to free the whole thing at once when we are
    // done with it.
    //

    SecurityDescriptor = ExAllocatePool(
                            PagedPool,
                            sizeof(SECURITY_DESCRIPTOR) + AclLength
                            );

    if (SecurityDescriptor == NULL) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: Couldn't allocate Sec. Desc.\n"));
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 6, 0, 0);
    }

    AclCopy = (PACL)((PISECURITY_DESCRIPTOR)SecurityDescriptor+1);
    RtlCopyMemory(AclCopy, Acl, AclLength);

    Status = RtlCreateSecurityDescriptor( SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: CreateSecDesc failed %08lx\n",Status));
        ExFreePool(SecurityDescriptor);
        SecurityDescriptor=NULL;
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 7, Status, 0);
    }

    Status = RtlSetDaclSecurityDescriptor( SecurityDescriptor,
                                           TRUE,
                                           AclCopy,
                                           FALSE );
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpHiveRootSecurityDescriptor: SetDacl failed %08lx\n",Status));
        ExFreePool(SecurityDescriptor);
        SecurityDescriptor=NULL;
        CM_BUGCHECK(REGISTRY_ERROR, ALLOCATE_SECURITY_DESCRIPTOR, 8, Status, 0);
    }

    //
    // free any allocations we made
    //
    if (WorldSid!=NULL) {
        ExFreePool(WorldSid);
    }
    if (RestrictedSid!=NULL) {
        ExFreePool(RestrictedSid);
    }
    if (SystemSid!=NULL) {
        ExFreePool(SystemSid);
    }
    if (AdminSid!=NULL) {
        ExFreePool(AdminSid);
    }
    if (Acl!=NULL) {
        ExFreePool(Acl);
    }

    return(SecurityDescriptor);
}

VOID
CmpFreeSecurityDescriptor(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    )

/*++

Routine Description:

    Frees the security descriptor associated with a particular node.  This
    can only happen when the node is actually being deleted from the
    registry.

    NOTE:   Caller is expected to have already marked relevent cells dirty.

Arguments:

    Hive - Supplies thepointer to hive control structure for hive of interest

    Cell - Supplies index for cell to free storage for (the target)

Return Value:

    None.

--*/

{
    PCELL_DATA Node;
    PCELL_DATA Security;
    HCELL_INDEX SecurityCell;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpFreeSecurityDescriptor for cell %ld\n",Cell));

    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);
    //
    // Map in the cell whose security descriptor is being freed
    //
    Node = HvGetCell(Hive, Cell);
    if( Node == NULL ) {
        //
        // we couldn't map the bin containing this cell
        // Sorry, we cannot free the descriptor
        return;
    }

    ASSERT_NODE(&(Node->u.KeyNode));

    //
    // Map in the cell containing the security descriptor.
    //
    SecurityCell = Node->u.KeyNode.Security;
    Security = HvGetCell(Hive, SecurityCell);
    if( Security == NULL ) {
        //
        // we couldn't map the bin containing this cell
        // Sorry, we cannot free the descriptor
        HvReleaseCell(Hive, Cell);
        return;
    }

    ASSERT_SECURITY(&(Security->u.KeySecurity));


    if (Security->u.KeySecurity.ReferenceCount == 1) {

        //
        // This is the only cell that references this security descriptor,
        // so it is ok to free it now.
        //
        CmpRemoveSecurityCellList(Hive, SecurityCell);
        HvFreeCell(Hive, SecurityCell);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpFreeSecurityDescriptor: freeing security cell\n"));
    } else {

        //
        // More than one node references this security descriptor, so
        // just decrement the reference count.
        //
        Security->u.KeySecurity.ReferenceCount -= 1;
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpFreeSecurityDescriptor: decrementing reference count\n"));
    }

    //
    // Zero out the pointer to the security descriptdr in the main cell
    //
    Node->u.KeyNode.Security = HCELL_NIL;
    // release the cells
    HvReleaseCell(Hive, Cell);
    HvReleaseCell(Hive, SecurityCell);
}

BOOLEAN
CmpInsertSecurityCellList(
    IN PHHIVE Hive,
    IN HCELL_INDEX NodeCell,
    IN HCELL_INDEX SecurityCell
    )
/*++

Routine Description:

    Inserts a newly-created security cell into the per-hive linked list of
    security cells.

    NOTE:   Assumes that NodeCell and SecurityCell have already been
            marked dirty.

Arguments:

    Hive - Supplies a pointer to the hive control structure.

    NodeCell - Supplies the cell index of the node that owns the security cell

    SecurityCell - Supplies the cell index of the security cell.

Return Value:

    TRUE - it worked

    FALSE - some failure - generally STATUS_NO_LOG_SPACE

--*/

{
    PCM_KEY_SECURITY    FlinkCell;
    PCM_KEY_SECURITY    BlinkCell;
    PCM_KEY_SECURITY    Cell;
    PCM_KEY_NODE        Node;
    PCM_KEY_NODE        ParentNode;

    PAGED_CODE();
    //
    // If the new cell's storage type is Volatile, simply make it the
    //  anchor of it's own list.  (Volatile security entries will disappear
    //  at reboot, restore, etc, so we don't need the list to hunt them
    //  down at those times.)
    //
    // Else, the storage type is Stable.
    //   Map in the node that owns the new security cell.  If it is a root
    //   cell, then we are creating the hive for the first time, so this is
    //   the only security cell in the list.  If it is not a root cell, then
    //   we simply find its parent's security cell and stick the new security
    //   cell into the list immediately after it.
    //
    //
    // we have the lock exclusive or nobody is operating inside this hive
    //
    //ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

    Cell = (PCM_KEY_SECURITY) HvGetCell(Hive, SecurityCell);
    if( Cell == NULL ) {
        //
        // we couldn't map the bin containing this cell
        // 
        return FALSE;
    }

    // release the cell as we hold the reglock exclusive
    HvReleaseCell(Hive, SecurityCell);

    ASSERT_SECURITY(Cell);

    if (HvGetCellType(SecurityCell) == Volatile) {

        Cell->Flink = Cell->Blink = SecurityCell;

    } else {

        Node = (PCM_KEY_NODE) HvGetCell(Hive, NodeCell);
        if( Node == NULL ) {
            //
            // we couldn't map the bin containing this cell
            // 
            return FALSE;
        }

        // release the cell as we hold the reglock exclusive
        HvReleaseCell(Hive, NodeCell);

        ASSERT_NODE(Node);

        if (Node->Flags & KEY_HIVE_ENTRY) {
            //
            // This must be the hive creation, so this cell becomes the anchor
            // for the list.
            //
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpInsertSecurityCellList: hive creation\n"));
            Cell->Flink = Cell->Blink = SecurityCell;

        } else {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpInsertSecurityCellList: insert at parent\n"));
            //
            // Map in the node's parent's security cell, so we can hook into
            // the list there.
            //
            ParentNode = (PCM_KEY_NODE) HvGetCell(Hive, Node->Parent);
            if( ParentNode == NULL ) {
                //
                // we couldn't map the bin containing this cell
                // 
                return FALSE;
            }
            // release the cell as we hold the reglock exclusive
            HvReleaseCell(Hive, Node->Parent);

            ASSERT_NODE(ParentNode);
            BlinkCell = (PCM_KEY_SECURITY) HvGetCell(
                                            Hive,
                                            ParentNode->Security
                                            );
            if( BlinkCell == NULL ) {
                //
                // we couldn't map the bin containing this cell
                // 
                return FALSE;
            }
            // release the cell as we hold the reglock exclusive
            HvReleaseCell(Hive, ParentNode->Security);

            ASSERT_SECURITY(BlinkCell);

            //
            // Map in the Flink of the parent's security cell.
            //
            FlinkCell = (PCM_KEY_SECURITY) HvGetCell(
                                            Hive,
                                            BlinkCell->Flink
                                            );
            if( FlinkCell == NULL ) {
                //
                // we couldn't map the bin containing this cell
                // 
                return FALSE;
            }
            // release the cell as we hold the reglock exclusive
            HvReleaseCell(Hive, BlinkCell->Flink);

            ASSERT_SECURITY(FlinkCell);

            if (! (HvMarkCellDirty(Hive, ParentNode->Security) &&
                   HvMarkCellDirty(Hive, BlinkCell->Flink)))
            {
                return FALSE;
            }

            //
            // Insert the new security cell in between the Flink and Blink cells
            //
            Cell->Flink = BlinkCell->Flink;
            Cell->Blink = FlinkCell->Blink;
            BlinkCell->Flink = SecurityCell;
            FlinkCell->Blink = SecurityCell;
        }
    }

    //
    // add the new security cell to the hive's security cache
    //
    if( !NT_SUCCESS( CmpAddSecurityCellToCache ( (PCMHIVE)Hive,SecurityCell,FALSE) ) ) {
        return FALSE;
    }

    return TRUE;
}


VOID
CmpRemoveSecurityCellList(
    IN PHHIVE Hive,
    IN HCELL_INDEX SecurityCell
    )
/*++

Routine Description:

    Removes a security cell from the per-hive linked list of security cells.
    (This means the cell is going to be deleted!)

    NOTE:   Caller is expected to have already marked relevent cells dirty

Arguments:

    Hive - Supplies a pointer to the hive control structure

    SecurityCell - Supplies the cell index of the security cell to be
           removed

Return Value:

    None.

--*/

{
    PCM_KEY_SECURITY FlinkCell;
    PCM_KEY_SECURITY BlinkCell;
    PCM_KEY_SECURITY Cell;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpRemoveSecurityCellList: index %ld\n",SecurityCell));

    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

    Cell = (PCM_KEY_SECURITY) HvGetCell(Hive, SecurityCell);
    if( Cell == NULL ) {
        //
        // we couldn't map the bin containing one of these cells
        // 
        return;
    }

    FlinkCell = (PCM_KEY_SECURITY) HvGetCell(Hive, Cell->Flink);
    if( FlinkCell == NULL ) {
        //
        // we couldn't map the bin containing one of these cells
        // 
        HvReleaseCell(Hive, SecurityCell);
        return;
    }

    BlinkCell = (PCM_KEY_SECURITY) HvGetCell(Hive, Cell->Blink);
    if( BlinkCell == NULL ) {
        //
        // we couldn't map the bin containing one of these cells
        // 
        HvReleaseCell(Hive, SecurityCell);
        HvReleaseCell(Hive, Cell->Flink);
        return;
    }

    ASSERT(FlinkCell->Blink == SecurityCell);
    ASSERT(BlinkCell->Flink == SecurityCell);

    FlinkCell->Blink = Cell->Blink;
    BlinkCell->Flink = Cell->Flink;

    //
    // finally, remove the security cell from cache, as it'll be freed
    //
    CmpRemoveFromSecurityCache ( (PCMHIVE)Hive,SecurityCell);

    //
    // release used cells
    //
    HvReleaseCell(Hive, Cell->Blink);
    HvReleaseCell(Hive, Cell->Flink);
    HvReleaseCell(Hive, SecurityCell);
}

