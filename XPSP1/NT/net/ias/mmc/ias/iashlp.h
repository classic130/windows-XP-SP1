//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, 1999 - 1999
//
//  File:       iashlp.h
//
//--------------------------------------------------------------------------

#define	IDH_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT	800010052
#define	IDH_BUTTON_CLIENT_PAGE1__FIND	800010077
#define	IDH_BUTTON_LOCAL_FILE_LOGGING_PAGE2__BROWSE	800010134
#define	IDH_BUTTON_REALMS_ADD	800010003
#define	IDH_BUTTON_REALMS_EDIT	800010005
#define	IDH_BUTTON_REALMS_MOVE_DOWN	800010002
#define	IDH_BUTTON_REALMS_MOVE_UP	800010001
#define	IDH_BUTTON_REALMS_REMOVE	800010004
#define	IDH_BUTTON_RESOLVE_DNS_NAME__RESOLVE	800010201
#define	IDH_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS	800010079
#define	IDH_CHECK_LOCAL_FILE_LOGING_PAGE1__ACCOUNTING	800010100
#define	IDH_CHECK_LOCAL_FILE_LOGING_PAGE1__AUTHENTICATION	800010101
#define	IDH_CHECK_LOCAL_FILE_LOGING_PAGE1__INTERIM_ACCT	800010102
#define	IDH_CHECK_SERVER_PAGE1__CAPTURE_DEBUG_PACKETS	800010152
#define	IDH_CHECK_SERVER_PAGE1__CAPTURE_MALFORMED_PACKETS	800010151
#define	IDH_COMBO_ADD_CLIENT__PROTOCOL	800010051
#define	IDH_COMBO_CLIENT_PAGE1__MANUFACTURER	800010078
#define	IDH_COMBO_RESOLVE_DNS_NAME__RESULT	800010202
#define	IDH_EDIT_ADD_CLIENT__NAME	800010050
#define	IDH_EDIT_CLIENT_PAGE1__ADDRESS	800010076
#define	IDH_EDIT_CLIENT_PAGE1__NAME	800010075
#define	IDH_EDIT_CLIENT_PAGE1__SHARED_SECRET	800010080
#define	IDH_EDIT_CLIENT_PAGE1__SHARED_SECRET_CONFIRM	800010081
#define	IDH_EDIT_LOCAL_FILE_LOGGING_PAGE2__DIRECTORY	800010133
#define	IDH_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE	800010132
#define	IDH_EDIT_REALMS_FIND	800010025
#define	IDH_EDIT_REALMS_REPLACE	800010026
#define	IDH_EDIT_RESOLVE_DNS_NAME__DNS_NAME	800010200
#define	IDH_EDIT_SERVER_PAGE1__NAME	800010150
#define	IDH_EDIT_SERVER_PAGE2_ACCOUNTING_PORT	800010176
#define	IDH_EDIT_SERVER_PAGE2_AUTHENTICATION_PORT	800010175
#define	IDH_EDIT_STARTUP_WIZARD_CONNECT__COMPUTER_NAME	800010277
#define	IDH_LIST_REALMS_REPLACEMENTS	800010000
#define	IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__DAILY	800010127
#define	IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_IAS1	800010126
#define	IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_ODBC	800010125
#define	IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__LOG_LIMIT	800010131
#define	IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__MONTHLY	800010129
#define	IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__UNLIMITED	800010130
#define	IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__WEEKLY	800010128
#define	IDH_RADIO_STARTUP_WIZARD_CONNECT__ANOTHER_COMPUTER	800010276
#define	IDH_RADIO_STARTUP_WIZARD_CONNECT__LOCAL_COMPUTER	800010275

#define IDH_BUTTON_RESOLVE_DNS_NAME__USE_THIS_IP 900000000

const DWORD g_aHelpIDs_IDD_PROPPAGE_SERVER3[]=
{
	IDC_LIST_REALMS_REPLACEMENTS,IDH_LIST_REALMS_REPLACEMENTS,
	IDC_BUTTON_REALMS_MOVE_UP,IDH_BUTTON_REALMS_MOVE_UP,
	IDC_BUTTON_REALMS_MOVE_DOWN,IDH_BUTTON_REALMS_MOVE_DOWN,
	IDC_BUTTON_REALMS_ADD,IDH_BUTTON_REALMS_ADD,
	IDC_BUTTON_REALMS_REMOVE,IDH_BUTTON_REALMS_REMOVE,
	IDC_BUTTON_REALMS_EDIT,IDH_BUTTON_REALMS_EDIT,
	0,0
};


const DWORD g_aHelpIDs_IDD_REPLACE_REALMS[]=
{
	IDC_EDIT_REALMS_FIND,IDH_EDIT_REALMS_FIND,
	IDC_EDIT_REALMS_REPLACE,IDH_EDIT_REALMS_REPLACE,
	0,0
};

const DWORD g_aHelpIDs_IDD_ADD_CLIENT[]=
{
	IDC_EDIT_ADD_CLIENT__NAME,IDH_EDIT_ADD_CLIENT__NAME,
	IDC_COMBO_ADD_CLIENT__PROTOCOL,IDH_COMBO_ADD_CLIENT__PROTOCOL,
	IDC_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT,IDH_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT,
	0,0
};

const DWORD g_aHelpIDs_IDD_PROPPAGE_CLIENT1[]=
{
	IDC_EDIT_CLIENT_PAGE1__NAME,IDH_EDIT_CLIENT_PAGE1__NAME,
	IDC_EDIT_CLIENT_PAGE1__ADDRESS,IDH_EDIT_CLIENT_PAGE1__ADDRESS,
	IDC_BUTTON_CLIENT_PAGE1__FIND,IDH_BUTTON_CLIENT_PAGE1__FIND,
	IDC_COMBO_CLIENT_PAGE1__MANUFACTURER,IDH_COMBO_CLIENT_PAGE1__MANUFACTURER,
	IDC_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS_SENDS_SIGNATURE,IDH_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS,
	IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET,IDH_EDIT_CLIENT_PAGE1__SHARED_SECRET,
	IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET_CONFIRM,IDH_EDIT_CLIENT_PAGE1__SHARED_SECRET_CONFIRM,
	0,0
};

const DWORD g_aHelpIDs_IDD_PROPPAGE_LOCAL_FILE_LOGGING1[]=
{
	IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_ACCOUNTING_PACKETS,IDH_CHECK_LOCAL_FILE_LOGING_PAGE1__ACCOUNTING,
	IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_AUTHENTICATION_PACKETS,IDH_CHECK_LOCAL_FILE_LOGING_PAGE1__AUTHENTICATION,
	IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_INTERIM_ACCOUNTING_PACKETS,IDH_CHECK_LOCAL_FILE_LOGING_PAGE1__INTERIM_ACCT,
	0,0
};

const DWORD g_aHelpIDs_IDD_PROPPAGE_LOCAL_FILE_LOGGING2[]=
{
	IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_ODBC,IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_ODBC,
	IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_IAS1,IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_IAS1,
	IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__DAILY,IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__DAILY,
	IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WEEKLY,IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__WEEKLY,
	IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__MONTHLY,IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__MONTHLY,
	IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__UNLIMITED,IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__UNLIMITED,
	IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_REACHES,IDH_RADIO_LOCAL_FILE_LOGGING_PAGE2__LOG_LIMIT,
	IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE,IDH_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE,
	IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_DIRECTORY,IDH_EDIT_LOCAL_FILE_LOGGING_PAGE2__DIRECTORY,
	IDC_BUTTON_LOCAL_FILE_LOGGING_PAGE2__BROWSE,IDH_BUTTON_LOCAL_FILE_LOGGING_PAGE2__BROWSE,
	0,0
};

const DWORD g_aHelpIDs_IDD_PROPPAGE_SERVER1[]=
{
	IDC_EDIT_SERVER_PAGE1__NAME,IDH_EDIT_SERVER_PAGE1__NAME,
	IDC_CHECK_SERVER_PAGE1__CAPTURE_MALFORMED_PACKETS,IDH_CHECK_SERVER_PAGE1__CAPTURE_MALFORMED_PACKETS,
	IDC_CHECK_SERVER_PAGE1__CAPTURE_DEBUG_PACKETS,IDH_CHECK_SERVER_PAGE1__CAPTURE_DEBUG_PACKETS,
	0,0
};

const DWORD g_aHelpIDs_IDD_PROPPAGE_SERVER2[]=
{
	IDC_EDIT_SERVER_PAGE2_AUTHENTICATION_PORT,IDH_EDIT_SERVER_PAGE2_AUTHENTICATION_PORT,
	IDC_EDIT_SERVER_PAGE2_ACCOUNTING_PORT,IDH_EDIT_SERVER_PAGE2_ACCOUNTING_PORT,
	0,0
};

const DWORD g_aHelpIDs_IDD_RESOLVE_DNS_NAME[]=
{
	IDC_EDIT_RESOLVE_DNS_NAME__DNS_NAME,IDH_EDIT_RESOLVE_DNS_NAME__DNS_NAME,
	IDC_BUTTON_RESOLVE_DNS_NAME__RESOLVE,IDH_BUTTON_RESOLVE_DNS_NAME__RESOLVE,
	IDC_COMBO_RESOLVE_DNS_NAME__RESULT,IDH_COMBO_RESOLVE_DNS_NAME__RESULT,
	IDOK, IDH_BUTTON_RESOLVE_DNS_NAME__USE_THIS_IP ,
	0,0
};

const DWORD g_aHelpIDs_IDD_WIZPAGE_ADD_CLIENT1[]=
{
	IDC_EDIT_ADD_CLIENT__NAME,IDH_EDIT_ADD_CLIENT__NAME,
	IDC_COMBO_ADD_CLIENT__PROTOCOL,IDH_COMBO_ADD_CLIENT__PROTOCOL,
	0,0
};

const DWORD g_aHelpIDs_IDD_WIZPAGE_ADD_CLIENT2[]=
{
	IDC_EDIT_CLIENT_PAGE1__ADDRESS,IDH_EDIT_CLIENT_PAGE1__ADDRESS,
	IDC_BUTTON_CLIENT_PAGE1__FIND,IDH_BUTTON_CLIENT_PAGE1__FIND,
	IDC_COMBO_CLIENT_PAGE1__MANUFACTURER,IDH_COMBO_CLIENT_PAGE1__MANUFACTURER,
	IDC_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS_SENDS_SIGNATURE,IDH_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS,
	IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET,IDH_EDIT_CLIENT_PAGE1__SHARED_SECRET,
	IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET_CONFIRM,IDH_EDIT_CLIENT_PAGE1__SHARED_SECRET_CONFIRM,
	0,0
};

const DWORD g_aHelpIDs_IDD_WIZPAGE_STARTUP_CONECT_TO_MACHINE[]=
{
	IDC_RADIO_STARTUP_WIZARD_CONNECT__LOCAL_COMPUTER,IDH_RADIO_STARTUP_WIZARD_CONNECT__LOCAL_COMPUTER,
	IDC_RADIO_STARTUP_WIZARD_CONNECT__ANOTHER_COMPUTER,IDH_RADIO_STARTUP_WIZARD_CONNECT__ANOTHER_COMPUTER,
	IDC_EDIT_STARTUP_WIZARD_CONNECT__COMPUTER_NAME,IDH_EDIT_STARTUP_WIZARD_CONNECT__COMPUTER_NAME,
	0,0
};
