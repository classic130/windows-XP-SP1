#include "stdinc.h"
#include <conio.h>

//#include <atlbase.h>
CComModule _Module;
#include <atlcom.h>
#include <atlimpl.cpp>

#define __TESTFF_CPP__
#include <fixprop.h>

GET_DEFAULT_DOMAIN_NAME_FN pfnGetDefaultDomainName = NULL;
HANDLE  g_hProcessImpersonationToken = NULL;

#define	KEY_STORAGEFILE	"StorageFileName"
#define KEY_MAXBLOCKS	"MaxBlocks"
#define KEY_GROUPNAME	"GroupName"
#define KEY_NAMELEN		"GroupNameLen"
#define KEY_LOWMARK		"LowWaterMark"
#define KEY_HIGHMARK	"HighWaterMark"
#define KEY_ARTICLECOUNT "ArticleCount"
#define KEY_GROUPID		"GroupId"
#define KEY_CREATELOW	"CreateLow"
#define KEY_CREATEHIGH 	"CreateHigh"
#define KEY_ISSPECIAL	"IsSpecial"
#define KEY_READONLY	"ReadOnly"
#define KEY_LOOP		"Loop"

#define PROGID_NO_DRIVER L"TestNT.NoDriver"
#define VRPATH L""

CHAR g_szStorageFile[MAX_PATH+1];
LPSTR g_lpstrSectionName = "testfix";
DWORD g_dwHandlePoolSize = 0xffffffff;
DWORD g_dwMaxBlocks = 0;
DWORD g_dwLoop = 0;
WCHAR g_wszMBPath[MAX_PATH+1];
CHAR g_szMBPath[MAX_PATH+1];
CHAR g_szGroupList[1024];
CHAR g_szVarGroupList[1024];

NNTPVROOTPTR g_pvroot;
CNNTPVRootTable *g_pVRTable;
CNewsTreeCore *g_pNewsTree;
// global count of errors
LONG g_cErrors;

void Verify(BOOL fTest, const char *szError, const char *szContext, DWORD dwLine) {
	if (fTest) return;
	InterlockedIncrement(&g_cErrors);
	printf("ERROR: line %i -- %s (%s)\n", dwLine, szError, szContext);
	_ASSERT(FALSE);
}


#define V(__f__, __szError__, __szContext__) Verify(__f__, __szError__, __szContext__, __LINE__)

void StartHintFunction( void )
{}

DWORD INNHash( PBYTE pb, DWORD )
{ return 0; }

BOOL fTestComponents( LPCSTR what )
{ return TRUE;
}

VOID
CopyUnicodeStringIntoAscii(
                IN LPSTR AsciiString,
                IN LPWSTR UnicodeString
            )
{
    while ( (*AsciiString++ = (CHAR)*UnicodeString++) != '\0');

} // CopyUnicodeStringIntoAscii

void Initialize(BOOL fDeleteGroupList) {
	HRESULT hr;
	BOOL fFatal;
	static IMSAdminBaseW *pMB;
	METADATA_HANDLE hmRoot;
	
	// initialize COM and the metabase
	printf("initializing metabase\n");
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	V(SUCCEEDED(hr), "CoInitializeEx", NULL);

	hr = CoCreateInstance(CLSID_MSAdminBase_W, NULL, CLSCTX_ALL, 
						  IID_IMSAdminBase_W, (LPVOID *) &pMB);
	V(hr == S_OK, "CoCreateInstance", NULL);

	// create a vroot entry for the root.  we don't have any other 
	// vroot entries
	printf("creating virtual root hierarchy\n");
	hr = pMB->OpenKey(METADATA_MASTER_ROOT_HANDLE, 
				      L"", 
				      METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
				      100,
				      &hmRoot);
	V(hr == S_OK, "metabase OpenKey(root)", NULL);

	// first we delete whatever exists in the metabase under this path,
	// then we create a new area to play in
	pMB->DeleteKey(hmRoot, g_wszMBPath);
	printf("creating key %s\n", g_wszMBPath);
	hr = pMB->AddKey(hmRoot, g_wszMBPath);
	V(hr == S_OK, "AddKey failed", g_szMBPath);
	// we configure one root vroot with parameters set to create no driver
	METADATA_RECORD mdrProgID = { 
		MD_VR_DRIVER_PROGID, 
		METADATA_INHERIT, 
		ALL_METADATA, 
		STRING_METADATA, 
		(lstrlenW(PROGID_NO_DRIVER)+1) * sizeof(WCHAR), 
		(BYTE *) PROGID_NO_DRIVER, 
		0 
	};
	printf("setting MD_VR_DRIVER_PROGID = \"%S\"\n", PROGID_NO_DRIVER);
	hr = pMB->SetData(hmRoot, g_wszMBPath, &mdrProgID);
	V(hr == S_OK, "SetData(MD_VR_DRIVER_PROGID) failed", g_szMBPath);

	METADATA_RECORD mdrVRPath = { 
		MD_VR_PATH, 
		METADATA_INHERIT, 
		ALL_METADATA, 
		STRING_METADATA, 
		(lstrlenW(VRPATH)+1) * sizeof(WCHAR), 
		(BYTE *) VRPATH, 
		0 
	};
	printf("setting MD_VR_PATH = \"%S\"\n", VRPATH);
	hr = pMB->SetData(hmRoot, g_wszMBPath, &mdrVRPath);
	V(hr == S_OK, "SetData(MD_VR_PATH) failed", g_szMBPath);

	pMB->CloseKey(hmRoot);
	pMB->Release();

	// initialize our news tree
	printf("initializing newstree object\n");
	g_pNewsTree = new CNewsTreeCore();
	g_pVRTable = new CNNTPVRootTable(g_pNewsTree->GetINewsTree(), 
						CNewsTreeCore::VRootRescanCallback);
	g_pNewsTree->Init(g_pVRTable, fFatal, 100, TRUE);

	// initialize the vroot table
	printf("initializing vroot table\n");
	hr = g_pVRTable->Initialize(g_szMBPath);
	printf("g_pVRTable->Initialize(\"%s\") returned %x\n", g_szMBPath, hr);
	V(hr == S_OK, "g_pVRTable->Initialize() failed", g_szMBPath);

	// load the newstree from disk
	// figure out a group.lst filename
	GetEnvironmentVariable("temp", g_szGroupList, 1024);
	lstrcat(g_szGroupList, "\\group.lst");
	printf("using group.lst path %s\n", g_szGroupList);
	if (fDeleteGroupList) DeleteFile(g_szGroupList);
	GetEnvironmentVariable("temp", g_szVarGroupList, 1024);
	lstrcat(g_szVarGroupList, "\\groupvar.lst");
	printf("using groupvar.lst path %s\n", g_szVarGroupList);
	if (fDeleteGroupList) DeleteFile(g_szVarGroupList);
	//g_pNewsTree->LoadTree(g_szGroupList, g_szVarGroupList);	
}



#ifdef DEBUG
BOOL EnumCallback( DATA_BLOCK& dbBuffer, PVOID pvContext, DWORD dw, BOOL b )
{
	printf ( "****************************\n" );
	printf ( "Enumerated: \n" );
	printf ( "Group name: %s\n", dbBuffer.szGroupName );
	printf ( "Group name len: %d\n", dbBuffer.dwGroupNameLen );
	printf ( "blah, blah ...\n" );

	return TRUE;
}

VOID ReadINI( LPSTR szINIFile )
{
	DWORD 	dw;
	CHAR	szFileName[MAX_PATH+1];
	CHAR	szTempPath[MAX_PATH+1];	

	// retrieve storage file name
	dw = GetPrivateProfileString(	g_lpstrSectionName,
									KEY_STORAGEFILE,
									"default",
									szFileName,
									MAX_PATH,
									szINIFile );
	_ASSERT( dw > 0 );

	// we'll always automatically put the file under temp dir
	dw = GetTempPath( MAX_PATH, szTempPath );
	_ASSERT( dw > 0 );
	sprintf( g_szStorageFile, "%s%s", szTempPath, szFileName );

	// read max blocks to expand to
	g_dwMaxBlocks = GetPrivateProfileInt( 	g_lpstrSectionName,
											KEY_MAXBLOCKS,
											64,
											szINIFile );
	_ASSERT( g_dwMaxBlocks > 0 );		

	// read loop number
	g_dwLoop = GetPrivateProfileInt( 	g_lpstrSectionName,
										KEY_LOOP,
										0,
										szINIFile );
	_ASSERT( g_dwLoop >= 0 );
}

VOID LoadProperty( CNewsGroupCore *pGroup, LPSTR szINIFile, LPSTR szSectionName )
{
	DWORD	dwBuffer=0xffffffff;
	CHAR	szGroupName[MAX_PATH+1];
	DWORD	dwLen = 0;
	FILETIME ft;

	// Load group name, group len is also set 
	dwBuffer = GetPrivateProfileString(	szSectionName,
										KEY_GROUPNAME,
										"microsoft.public.ins",
										szGroupName,
										MAX_PATH,
										szINIFile );

	dwBuffer = 0xffffffff;
	dwBuffer = GetPrivateProfileInt( 	szSectionName,
										KEY_GROUPID,
										0,
										szINIFile );
	_ASSERT( 0xffffffff != dwBuffer );

	pGroup->Init( 	szGroupName,
					NULL,
					dwBuffer,
					g_pvroot );
						
	dwBuffer = 0xffffffff;
	dwBuffer = GetPrivateProfileInt(	szSectionName,
										KEY_LOWMARK,
										0,
										szINIFile );
	_ASSERT( 0xffffffff != dwBuffer );

	pGroup->SetLowWatermark( dwBuffer );

	dwBuffer = 0xffffffff;
	dwBuffer = GetPrivateProfileInt(	szSectionName,
										KEY_HIGHMARK,
										0,
										szINIFile );
	_ASSERT( 0xffffffff != dwBuffer );

	pGroup->SetHighWatermark( dwBuffer );
	
	dwBuffer = 0xffffffff;
	dwBuffer = GetPrivateProfileInt(	szSectionName,
										KEY_ARTICLECOUNT,
										0,
										szINIFile );
	_ASSERT( 0xffffffff != dwBuffer );

	pGroup->SetMessageCount( dwBuffer );
	
	dwBuffer = 0xffffffff;
	dwBuffer = GetPrivateProfileInt(	szSectionName,
										KEY_CREATELOW,
										0,
										szINIFile );
	_ASSERT( 0xffffffff != dwBuffer );

	ft.dwLowDateTime = dwBuffer;

	dwBuffer = 0xffffffff;
	dwBuffer = GetPrivateProfileInt(	szSectionName,
										KEY_CREATEHIGH,
										0,
										szINIFile );
	_ASSERT( 0xffffffff != dwBuffer );

	ft.dwHighDateTime = dwBuffer;

	pGroup->SetCreateDate( ft );
	
	dwBuffer = 0xffffffff;
	dwBuffer = GetPrivateProfileInt(	szSectionName,
										KEY_ISSPECIAL,
										0,
										szINIFile );
//        _ASSERT( SUCCEEDED( hr ) );

	pGroup->SetSpecial( ( dwBuffer != 0 ) );
	
	dwBuffer = 0xffffffff;
	dwBuffer = GetPrivateProfileInt(	szSectionName,
										KEY_READONLY,
										0,
										szINIFile );
	_ASSERT( 0xffffffff != dwBuffer );

	pGroup->SetReadOnly( (dwBuffer != 0 ) );
}

VOID TestAdd( INNTPPropertyBag *pPropBag, CFixPropPersist& store )
{
	DWORD i;
	
	for ( i = 0; i < g_dwMaxBlocks; i++ ) {
		if ( !store.AddGroup( pPropBag ) ) {
			printf ( "Adding group fail %d\n", GetLastError() );
			exit( 1 );
		}
	}
}

VOID TestDelete( INNTPPropertyBag *pPropBag, CFixPropPersist& store )
{
	DWORD i;

	for ( i = 0; i < g_dwMaxBlocks; i++ ) {
		if ( !store.RemoveGroup( pPropBag ) ) {
			printf ( "Removing group fail %d\n", GetLastError() );
			exit( 1 );
		}
	}
}

VOID TestSet( INNTPPropertyBag *pPropBag, CFixPropPersist& store, DWORD dwFlag )
{

    if ( !store.SetGroup( pPropBag, dwFlag ) ) {
	    printf ( "Setting group fail %d\n", GetLastError() );
        exit( 1 );
   }
}

VOID TestGet( INNTPPropertyBag *pPropBag, CFixPropPersist& store, DWORD dwFlag )
{
	if( !store.GetGroup( pPropBag, dwFlag ) ) {
		printf ( "Getting group fail %d\n", GetLastError() );
		exit( 1 );
	}
}
#endif

void Shutdown() {
	g_pNewsTree->StopTree();
	//g_pNewsTree->TermTree();
	delete g_pVRTable;
	delete g_pNewsTree;
}

int __cdecl main( int argc, char **argv )
{
#ifdef DEBUG
	DWORD i;
	CNewsGroupCore *pGroup1, *pGroup2;
	INNTPPropertyBag *pPropBag1, *pPropBag2;
	DWORD	dwIdHigh = 0;

	InitAsyncTrace();

    // 
    // Initialize global heap
    //
    ExchMHeapCreate( NUM_EXCHMEM_HEAPS, 0, 100 * 1024, 0 ) ;


	if ( argc != 3 ) {
		printf( "[usage] testfix *.ini section\n" );
		goto FailExit;
	}

	printf( "CFixPropPersist unit test is starting ...\n" );

	// Read ini file parameters
	ReadINI( argv[1] );
	strcpy( g_szMBPath, "LM/testfix" );
	lstrcpyW( g_wszMBPath, L"LM/testfix" );

	CVRootTable::GlobalInitialize();

	_Module.Init(NULL, (HINSTANCE) INVALID_HANDLE_VALUE);

	Initialize( TRUE );

	g_pNewsTree = new CNewsTreeCore;
	if ( !g_pNewsTree ) {
		printf ( "Out of memory\n" );
        ExchMHeapDestroy();
		exit( 1 );
	}

	g_pvroot = new CNNTPVRoot;
	if ( !g_pvroot ) {
		printf ( "Out of memory\n" );
        ExchMHeapDestroy();
		exit( 1 );
	}

    g_pvroot->Init( NULL, "name", (CVRootTable*)g_pVRTable, L"name" );

	for ( i = 0; i < g_dwLoop; i++ ) {

		// If the file already exists, delete it
		DeleteFile( g_szStorageFile );

		// Initialize the store
		CFixPropPersist store( g_szStorageFile);
		printf( "Init handle %d\n", g_dwHandlePoolSize );
		if( !store.Init( 	TRUE, NULL, &dwIdHigh, EnumCallback )) {
			printf( "Store initialization failed with %d\n", GetLastError() );
			goto FailExit;		
		}

		pGroup1 = new CNewsGroupCore( g_pNewsTree ); // no newstree
		if ( NULL == pGroup1 ) {
			printf ( "Create group core instance fail %d\n", GetLastError() );
			goto FailExit;
		}

		LoadProperty( pGroup1, argv[1], "samplegroup" );

		pPropBag1 = pGroup1->GetPropertyBag();
		if ( NULL == pPropBag1 ) {
			printf  ( "Get property bag fail %d\n", GetLastError() );
			goto FailExit;
		}
	
		// Test adding groups with the same name
		TestAdd( pPropBag1, store );
	
		// Test remove all those groups by name
		pGroup2 = new CNewsGroupCore( g_pNewsTree ); 
		if ( NULL == pGroup2 ) {
			printf ( "Create group core fail %d\n", GetLastError() );
			goto FailExit;
		}

		pGroup2->Init( pGroup1->GetName(), NULL, pGroup1->GetGroupId(),
						g_pvroot );

		pPropBag2 = pGroup2->GetPropertyBag();
		if ( NULL == pPropBag2 ) {
			printf ("Get property bag fail %d\n", GetLastError() );
			goto FailExit;
		}
		
		TestDelete( pPropBag2, store );

		// Add one more group into it, after which there should be only
		// one group
		if ( !store.AddGroup( pPropBag1 ) ) {
			printf ( "Add group fail %d\n", GetLastError() );
			goto FailExit;
		}

		// Set this group with another bag
		LoadProperty( pGroup2, argv[1], "samplegroup2" );
		if ( !store.SetGroup( pPropBag2, ALL_BUT_NAME_AND_LEN ) ) {
			printf ( "Set group fail %d\n", GetLastError() );
			goto FailExit;
		}

		pPropBag1->Release();
		pPropBag2->Release();
		//if ( pGroup1 ) delete pGroup1;
		//if ( pGroup2 ) delete pGroup2;

		if ( !store.Term() ) {
			printf ( "Store termination failed with %d\n", GetLastError() );
			goto FailExit;
		}
	} // for

	_Module.Term();

	Shutdown();

    delete g_pvroot;
    ExchMHeapDestroy();
	TermAsyncTrace();
	return 0;

FailExit:

    delete g_pvroot;
    ExchMHeapDestroy();
	TermAsyncTrace();
	return -1;
	
#else
	printf("This test doesn't work in retail builds\n");
	return -1;
#endif
}
