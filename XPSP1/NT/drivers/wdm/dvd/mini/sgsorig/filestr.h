#ifndef __FILESTR_H
#define __FILESTR_H
//----------------------------------------------------------------------------
// FILESTR.H
//----------------------------------------------------------------------------
// Description : small description of the goal of the module
//----------------------------------------------------------------------------
// Copyright SGS Thomson Microelectronics  !  Version alpha  !  Jan 1st, 1995
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//                               Include files
//----------------------------------------------------------------------------
#include "stdio.h"
#include "stdefs.h"
#include "error.h"

//----------------------------------------------------------------------------
//                             Exported Constants
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//                               Exported Types
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//                             Exported Variables
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//                             Exported Functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// One line function description (same as in .C)
//----------------------------------------------------------------------------
// In     :
// Out    :
// InOut  :
// Global :
// Return :
//----------------------------------------------------------------------------
ERRORCODE OpenFile(PCHAR FileName);
U16 ReadBuffer(P_U8 Buffer, U16 Size);
VOID RewindFile(VOID);
VOID CloseFile(VOID);
FILE *GetFilePtr(VOID); // FILE should be redefined !

//------------------------------- End of File --------------------------------
#endif // #ifndef __FILESTR_H

