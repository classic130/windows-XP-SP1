/*++

Copyright (C) 1999-2001 Microsoft Corporation

Module Name:

Abstract:

History:

--*/

// Perfgen.h

#ifndef _PERFGEN_H_
#define _PERFGEN_H_

//
//	Definition macros
//

#define TIMEOUTVAL	600000	// 10 minutes

#define EVILPERF_FUNCTION_FAIL					0x00000001
#define EVILPERF_OVERWRITE_PREFIX_GUARDBYTES	0x00000002
#define EVILPERF_OVERWRITE_SUFFIX_GUARDBYTES	0x00000004
#define EVILPERF_RETURN_EMPTY_BLOB				0x00000008
#define EVILPERF_BAD_OBJECT_TOTAL_LENGTH		0x00000010
#define EVILPERF_BAD_OBJECT_DEFINITION_LENGTH	0x00000020
#define EVILPERF_BAD_OBJECT_HEADER_LENGTH		0x00000040
#define EVILPERF_BAD_INSTANCE_DEF_LENGTH		0x00000080
#define EVILPERF_BAD_COUNTER_DEF_LENGTH			0x00000100
#define EVILPERF_THROW_EXCEPTION				0x00000200
#define	EVILPERF_DUPLICATE_PROP_INDEX_NON_BASE	0x00000400
#define EVILPERF_DUPLICATE_PROP_INDEX_BASE		0x00000800
#define EVILPERF_MULTIBASE						0x00001000
#define EVILPERF_FIRST_COUNTER_IS_BASE			0x00002000 
#define EVILPERF_SECOND_COUNTER_IS_BASE			0x00004000 
#define EVILPERF_DUPLICATE_SPACE_OBJECTS		0x00008000
#define EVILPERF_TIMEOUT						0x00010000
#define EVILPERF_OBJECTS_WITH_ZERO_LEN_BLOB		0x00020000

//
//  Function Prototypes
//
//      these are used to insure that the data collection functions
//      accessed by Perflib will have the correct calling format.
//

PM_OPEN_PROC    OpenEvilPerformanceData;
PM_COLLECT_PROC CollectEvilPerformanceData;
PM_CLOSE_PROC   CloseEvilPerformanceData;


#endif	//_PERFGEN_H_