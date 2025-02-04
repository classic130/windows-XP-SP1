/////////////////////////////////////////////////////////////////////////////
//
// Copyright(C) 1997-2000 Microsoft Corporation all rights reserved.
//
// Module:      sdoclient.h
//
// Project:     Everest
//
// Description: IAS Server Data Object - Client Object Definition
//
// Author:      TLP 1/23/98
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_IAS_SDO_CLIENT_H_
#define _INC_IAS_SDO_CLIENT_H_

#include "resource.h"    // main symbols
#include <ias.h>
#include <sdoiaspriv.h>
#include "sdo.h"         // SDO base class


/////////////////////////////////////////////////////////////////////////////
// CSdoclient
/////////////////////////////////////////////////////////////////////////////

class CSdoClient : public CSdo 
{

public:

////////////////////
// ATL Interface Map
////////////////////
BEGIN_COM_MAP(CSdoClient)
	COM_INTERFACE_ENTRY(ISdo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_SDO_FACTORY(CSdoClient);

	/////////////////////////////////////////////////////////////////////////////
	CSdoClient() { }

	/////////////////////////////////////////////////////////////////////////////
    ~CSdoClient() { }

private:

	CSdoClient(const CSdoClient& rhs);
	CSdoClient& operator = (CSdoClient& rhs);
};

#endif // _INC_IAS_SDO_CLIENT_H_

