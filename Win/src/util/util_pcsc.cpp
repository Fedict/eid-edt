/* ****************************************************************************

* EDT Project.
* Copyright (C) 2010-2011 FedICT.
*
* This is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License version
* 3.0 as published by the Free Software Foundation.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, see
* http://www.gnu.org/licenses/.

**************************************************************************** */

#include <edt.h>
#include "pcsc.h"
#include "log.h"
#include "util_pcsc.h"
//#include "progress.h"

typedef LONG(__stdcall *fSCardEstablishContext)(
	DWORD dwScope,
	LPCVOID pvReserved1,
	LPCVOID pvReserved2,
	LPSCARDCONTEXT phContext
	);
static fSCardEstablishContext g_fSCardEstablishContext=NULL;
#define FCTNAME_SCARD_ESTABLISHCONTEXT "SCardEstablishContext"

typedef LONG(__stdcall *fSCardReleaseContext)(
	SCARDCONTEXT hContext
	);
static fSCardReleaseContext g_fSCardReleaseContext=NULL;
#define FCTNAME_SCARD_RELEASECONTEXT "SCardReleaseContext"

typedef LONG(__stdcall *fSCardConnect)(
	SCARDCONTEXT hContext,
	LPCTSTR szReader,
	DWORD dwShareMode,
	DWORD dwPreferredProtocols,
	LPSCARDHANDLE phCard,
	LPDWORD pdwActiveProtocol
	);
static fSCardConnect g_fSCardConnect=NULL;
#ifdef WIN32
#define FCTNAME_SCARD_CONNECT "SCardConnectW"
#elif __APPLE__
#define FCTNAME_SCARD_CONNECT "SCardConnect"
#endif	

typedef LONG(__stdcall *fSCardDisconnect)(
	SCARDHANDLE hCard,
	DWORD dwDisposition
	);
static fSCardDisconnect g_fSCardDisconnect=NULL;
#define FCTNAME_SCARD_DISCONNECT "SCardDisconnect"

typedef LONG(__stdcall *fSCardReconnect)(
	SCARDHANDLE hCard,
	DWORD dwShareMode,
	DWORD dwPreferredProtocols,
	DWORD dwInitialization,
	LPDWORD pdwActiveProtocol
);
static fSCardReconnect g_fSCardReconnect=NULL;
#define FCTNAME_SCARD_RECONNECT "SCardReconnect"

#ifdef WIN32
typedef LONG(__stdcall *fSCardFreeMemory)(
	SCARDCONTEXT hContext,
	LPCVOID pvMem
	);
static fSCardFreeMemory g_fSCardFreeMemory=NULL;
#define FCTNAME_SCARD_FREEMEMORY "SCardFreeMemory"
#endif

typedef LONG(__stdcall *fSCardListReaders)(
	SCARDCONTEXT hContext,
	LPCTSTR mszGroups,
	LPTSTR mszReaders,
	LPDWORD pcchReaders
	);	
static fSCardListReaders g_fSCardListReaders=NULL;
#ifdef WIN32
#define FCTNAME_SCARD_LISTREADERS "SCardListReadersW"
#elif __APPLE__
#define FCTNAME_SCARD_LISTREADERS "SCardListReaders"
#endif	

#ifdef WIN32
typedef LONG(__stdcall *fSCardGetAttrib)(
	SCARDHANDLE hCard,
	DWORD dwAttrId,
	LPBYTE pbAttr,
	LPDWORD pcbAttrLen
	);
static fSCardGetAttrib g_fSCardGetAttrib;
#define FCTNAME_SCARD_GETATTRIB "SCardGetAttrib"
#endif

typedef LONG(__stdcall *fSCardStatus)(
	SCARDHANDLE hCard,
	LPTSTR szReaderName,
	LPDWORD pcchReaderLen,
	LPDWORD pdwState,
	LPDWORD pdwProtocol,
	LPBYTE pbAtr,
	LPDWORD pcbAtrLen
	);
static fSCardStatus g_fSCardStatus=NULL;
#ifdef WIN32
#define FCTNAME_SCARD_STATUS "SCardStatusW"
#elif __APPLE__
#define FCTNAME_SCARD_STATUS "SCardStatus"
#endif	

typedef LONG(__stdcall *fSCardTransmit)(
	SCARDHANDLE hCard,
	LPCSCARD_IO_REQUEST pioSendPci,
	LPCBYTE pbSendBuffer,
	DWORD cbSendLength,
	LPSCARD_IO_REQUEST pioRecvPci,
	LPBYTE pbRecvBuffer,
	LPDWORD pcbRecvLength
	);
static fSCardTransmit g_fSCardTransmit=NULL;
#define FCTNAME_SCARD_TRANSMIT "SCardTransmit"

typedef LONG(__stdcall *fSCardBeginTransaction)(
	SCARDHANDLE hCard
	);
static fSCardBeginTransaction g_fSCardBeginTransaction=NULL;
#define FCTNAME_SCARD_BEGINTRANSACTION "SCardBeginTransaction"

typedef LONG(__stdcall *fSCardEndTransaction)(
	SCARDHANDLE hCard,
	DWORD dwDisposition
	);
static fSCardEndTransaction g_fSCardEndTransaction=NULL;
#define FCTNAME_SCARD_ENDTRANSACTION "SCardEndTransaction"

static HMODULE g_hWinscardLib=NULL;

static SCARD_IO_REQUEST g_ioSendPci;
static SCARD_IO_REQUEST g_ioRecvPci;

#define DEFAULT_CONNECT_DELAY 200
#define DEFAULT_TRANSMIT_DELAY 30

static int g_ConnectDelay = DEFAULT_CONNECT_DELAY;
static int g_TransmitDelay = DEFAULT_TRANSMIT_DELAY;
static bool g_TransactionStarted = false;

const BYTE REF_ID_DF[]		= {0xDF, 0x01};
const BYTE REF_ID[]			= {0x40, 0x31};
const BYTE REF_ADDRESS[]	= {0x40, 0x33};
const BYTE REF_PICTURE[]	= {0x40, 0x35};
const BYTE REF_VERSION[]	= {0x50, 0x32};
const BYTE REF_CERTRRN[]	= {0x50, 0x3C};
const BYTE REF_CERTROOT[]	= {0x50, 0x3B};
const BYTE REF_CERTCA[]		= {0x50, 0x3A};
const BYTE REF_CERTSIGN[]	= {0x50, 0x39};
const BYTE REF_CERTAUTH[]	= {0x50, 0x38};

#define BEID_FIELD_TAG_ID_Surname				0x07
#define BEID_FIELD_TAG_ID_FirstName_1			0x08
#define BEID_FIELD_TAG_ID_FirstName_2			0x09
#define BEID_FIELD_TAG_ADDR_Street				0x01

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void *loadPCSCLib();
int Connect(const SCARDCONTEXT hSC, const wchar_t *csReader, SCARDHANDLE *phCard,int *pConnectDelay=NULL);
int Disconnect(SCARDHANDLE hCard, bool reset);
int EDT_pcscLogCardReaderList(bool logCard);

long CardRecover(SCARDHANDLE hCard);
int BeginTransaction( SCARDHANDLE hCard );
int EndTransaction( SCARDHANDLE hCard );
int SelectApplet( SCARDHANDLE hCard, int *pTransmitDelay );
int SelectDF( SCARDHANDLE hCard, BYTE *selectapdu , DWORD selectapduLen);
int SendApdu(SCARDHANDLE hCard, LPBYTE pbSendBuffer, DWORD dwSendBufLen, LPBYTE  pbRecvBuffer, LPDWORD pdwRecvBufLen);
int ReadCardFile(SCARDHANDLE hCard, const BYTE *Path, const int PathLen, DWORD *pFileLen);
//int GetTlvValue(const std::vector<BYTE>&FileContent, int Tag, std::wstring *Value);
int GetATR(SCARDHANDLE hCard, std::wstring *atr);
int LogCardData(SCARDHANDLE hCard);
int ReadCard(SCARDHANDLE hCard);
//int SelectMF(SCARDHANDLE hCard, int * const TransmitDelay);
/*
int TestConnect(Card_ID id, bool *success, int ConnectDelay);

int pcscFillReaderList(Reader_LIST *readersList, Card_LIST *cardList);
*/
////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_pcscLoad()
{
	int iReturnCode = EDT_OK;
	LOG_ENTER();
	HMODULE hWinscardLib = (HMODULE)loadPCSCLib();

	if (hWinscardLib == NULL) 
	{
		LOG_ERROR(L"EDT_pcscLoad EDT_ERR_LIBRARY_NOT_FOUND");
		return EDT_ERR_LIBRARY_NOT_FOUND;
	}

	LOG(L"EDT_pcscLoad succeeded\n");

	LOG_EXIT(iReturnCode);
	return EDT_OK;
} 

int EDT_pcscEstablishContext()
{
	int iReturnCode = EDT_OK;
	LOG_ENTER();
	HMODULE hWinscardLib = (HMODULE)loadPCSCLib();

	if (hWinscardLib == NULL) 
	{
		LOG_EXIT_ERROR(L"loadPCSCLib EDT_ERR_LIBRARY_NOT_FOUND");
		return EDT_ERR_LIBRARY_NOT_FOUND;
	}
	LOG(L"EDT_pcscLoad succeeded\n");

	SCARDCONTEXT hSC=NULL;

	int err=SCARD_S_SUCCESS;

	if (SCARD_S_SUCCESS != (err = g_fSCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hSC)))
	{
		LOG_EXIT_ERRORCODE(L"SCardEstablishContext failed\n",err);
		return EDT_ERR_PCSC_CONTEXT_FAILED;
	}
	else
	{
		LOG(L"SCardEstablishContext succeeded\n");
	}

	if (SCARD_S_SUCCESS != (err = g_fSCardReleaseContext(hSC)))
	{
		LOG_ERRORCODE(L"SCardReleaseContext failed\n",err);
		return EDT_ERR_PCSC_CONTEXT_FAILED;
	}
	else
	{
		LOG(L"SCardReleaseContext succeeded\n");
	}
	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int BeginTransaction( SCARDHANDLE hCard )
{
	int err = SCARD_S_SUCCESS;
	LOG_ENTER();
	if (SCARD_S_SUCCESS != (err = g_fSCardBeginTransaction(hCard))) 
	{
		LOG_EXIT_ERRORCODE(L"SCardBeginTransaction failed",err);
		return EDT_ERR_INTERNAL;
	} 
	else
	{
		g_TransactionStarted = true;
	}
	LOG_EXIT(EDT_OK);
	return EDT_OK;
}
int EndTransaction( SCARDHANDLE hCard )
{
	int err = SCARD_S_SUCCESS;
	LOG_ENTER();
	if (SCARD_S_SUCCESS != (err = g_fSCardEndTransaction(hCard, SCARD_LEAVE_CARD))) 
	{
		LOG_EXIT_ERRORCODE(L"SCardEndTransaction failed",err);
		return EDT_ERR_INTERNAL;
	}
	else
	{
		g_TransactionStarted = false;
	}
	LOG_EXIT(EDT_OK);
	return EDT_OK;
}
int SelectApplet( SCARDHANDLE hCard )
{
	int iReturnCode = EDT_OK;
	BYTE select_applet[] = {0x00,0xA4,0x04,0x00,0x0F,0xA0,0x00,0x00,0x00,0x30,0x29,0x05,0x70,0x00,0xAD,0x13,0x10,0x01,0x01,0xFF};
	LOG_ENTER();

	BYTE pbRecvBuffer[2];
	DWORD dwRecvBufLen = 2;
	// select applet	
	if(EDT_OK != (iReturnCode = SendApdu( hCard, select_applet,sizeof(select_applet), pbRecvBuffer, &dwRecvBufLen)))
	{
		LOG_EXIT_ERRORCODE(L"SelectApplet failed",iReturnCode);
		return iReturnCode;
	}

	if (pbRecvBuffer[0] != 0x90 || pbRecvBuffer[1] != 0x00) 
	{
		LOG_EXIT_ERROR(L"SelectApplet failed");
		return EDT_ERR_PCSC_TRANSMIT_FAILED;
	}
	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

int SelectDF( SCARDHANDLE hCard , BYTE* selectDF, DWORD selectDFLen)
{
	int iReturnCode = EDT_OK;
	LOG_ENTER();

	BYTE pbRecvBuffer[2];
	DWORD dwRecvBufLen = 2;
	// select applet	
	if(EDT_OK != (iReturnCode = SendApdu( hCard, selectDF,selectDFLen, pbRecvBuffer, &dwRecvBufLen)))
	{
		LOG_EXIT_ERRORCODE(L"SelectDF failed",iReturnCode);
		return iReturnCode;
	}

	if (pbRecvBuffer[0] != 0x90 || pbRecvBuffer[1] != 0x00) 
	{
		LOG_EXIT_ERROR(L"SelectDF failed");
		return EDT_ERR_PCSC_TRANSMIT_FAILED;
	}
	LOG_EXIT(iReturnCode);

	return iReturnCode;
}

void *loadPCSCLib()
{
	if(g_hWinscardLib)
		return g_hWinscardLib;

#ifdef WIN32
	const wchar_t *pcscLib = L"winscard.dll";
#elif __APPLE__
	const wchar_t *pcscLib = L"/System/Library/Frameworks/PCSC.framework/PCSC";
#endif
	LOG_ENTER();
	LOG(L"searching pcscLib '%ls'\n",pcscLib);

	if(NULL == (g_hWinscardLib = LoadLibrary(pcscLib)))
	{
		LOG_LASTERROR(L"LoadLibrary failed");
		return g_hWinscardLib;
	}

	if(NULL == (g_fSCardEstablishContext = reinterpret_cast<fSCardEstablishContext>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_ESTABLISHCONTEXT))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardEstablishContext failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	if(NULL == (g_fSCardReleaseContext = reinterpret_cast<fSCardReleaseContext>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_RELEASECONTEXT))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardReleaseContext failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	if(NULL == (g_fSCardListReaders = reinterpret_cast<fSCardListReaders>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_LISTREADERS))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardListReaders failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	if(NULL == (g_fSCardConnect = reinterpret_cast<fSCardConnect>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_CONNECT))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardConnect failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	if(NULL == (g_fSCardDisconnect = reinterpret_cast<fSCardDisconnect>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_DISCONNECT))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardDisconnect failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	if(NULL == (g_fSCardReconnect = reinterpret_cast<fSCardReconnect>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_RECONNECT))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardReconnect failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

#ifdef WIN32
	if(NULL == (g_fSCardFreeMemory = reinterpret_cast<fSCardFreeMemory>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_FREEMEMORY))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardFreeMemory failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	//Not available on Mac OS Tiger
	if(NULL == (g_fSCardGetAttrib = reinterpret_cast<fSCardGetAttrib>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_GETATTRIB))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardGetAttrib failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}
#endif

	if(NULL == (g_fSCardStatus = reinterpret_cast<fSCardStatus>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_STATUS))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardStatus failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	if(NULL == (g_fSCardTransmit = reinterpret_cast<fSCardTransmit>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_TRANSMIT))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardTransmit failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	if(NULL == (g_fSCardBeginTransaction = reinterpret_cast<fSCardBeginTransaction>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_BEGINTRANSACTION))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardBeginTransaction failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	if(NULL == (g_fSCardEndTransaction = reinterpret_cast<fSCardEndTransaction>(GetProcAddress(g_hWinscardLib, FCTNAME_SCARD_ENDTRANSACTION))))
	{
		LOG_LASTERROR(L"GetProcAddress on fSCardEndTransaction failed");
		FreeLibrary(g_hWinscardLib);
		g_hWinscardLib=NULL;
		LOG_EXIT_ERROR(L"loadPCSCLib failed\n");
		return NULL;
	}

	LOG_EXIT(0);
	return g_hWinscardLib;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int Connect(const SCARDCONTEXT hSC, const wchar_t *csReader, SCARDHANDLE *phCard, int *pConnectDelay)
{
	int iReturnCode = EDT_OK;

	LOG_ENTER();
	if((void*)hSC == NULL || csReader == NULL || wcslen(csReader) == 0 || phCard == NULL)
	{
		LOG_EXIT_ERROR(L"Connect failed");
		return EDT_ERR_BAD_PARAM;
	}

	*phCard = NULL;
	DWORD dwProtocol = 1;
	unsigned int err=SCARD_S_SUCCESS;
	int counter = 0;

	for( ;counter < EDT_PCSC_MAX_RETRIES; counter++)
	{

#ifdef WIN32
		if (SCARD_S_SUCCESS != (err = g_fSCardConnect(hSC, csReader,							  SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, phCard, &dwProtocol)))
#elif __APPLE__
		if (SCARD_S_SUCCESS != (err = g_fSCardConnect(hSC, string_From_wstring(csReader).c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_Tx, phCard, &dwProtocol)))
#endif
		{
			if (SCARD_E_NO_SMARTCARD == err
				|| SCARD_W_REMOVED_CARD == err)
			{
				LOG_EXIT_ERRORCODE(L"SCardConnect failed, card not found",err);
				return EDT_ERR_CARD_NOT_FOUND;
			}
			else
			{
				LOG_EXIT_ERRORCODE(L"SCardConnect failed",err);
				return EDT_ERR_PCSC_CONNECT_FAILED;
			}
		}
		else {
			break;
		}
		Sleep(EDT_PCSC_RETRY_MS);
	}

	if( err == SCARD_S_SUCCESS)
	{
		counter++;
		LOG(L"SCardConnect succeeded after %d try, ",counter);
		LOG(L"dwProtocol = %d\n",dwProtocol);
		g_ioSendPci.dwProtocol = dwProtocol;
		g_ioSendPci.cbPciLength = sizeof(SCARD_IO_REQUEST);
		g_ioRecvPci.dwProtocol = dwProtocol;
		g_ioRecvPci.cbPciLength = sizeof(SCARD_IO_REQUEST);

		// If you do an SCardTransmit() too fast after an SCardConnect(),
		// some cards/readers will return an error (e.g. 0x801002f)
		Sleep(pConnectDelay==NULL?g_ConnectDelay:*pConnectDelay);
	}
	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int Disconnect(SCARDHANDLE hCard, bool reset)
{
	int iReturnCode = EDT_OK;
	LOG_ENTER();
	if((void*)hCard == NULL)
	{
		LOG_EXIT_ERROR(L"Disconnect failed hCard = NULL\n");
		return EDT_ERR_BAD_PARAM;
	}

	DWORD dwDisposition = (reset ? /*SCARD_UNPOWER_CARD*/ SCARD_RESET_CARD : SCARD_LEAVE_CARD);
	int err=SCARD_S_SUCCESS;

	if (SCARD_S_SUCCESS != (err = g_fSCardDisconnect((SCARDHANDLE) hCard, dwDisposition)))
	{
		LOG_EXIT_ERRORCODE(L"SCardDisconnect failed",err);
		return EDT_ERR_INTERNAL;
	}
	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

int EDT_pcscLogReaderList()
{
	return EDT_pcscLogCardReaderList(false);
}

int EDT_pcscLogCardList()
{
	return EDT_pcscLogCardReaderList(true);
}

int EDT_pcscLogCardReaderList(bool logCard)
{
	int iReturnCode = EDT_OK;
	SCARDCONTEXT hSC=NULL;
	unsigned int err=SCARD_S_SUCCESS;
	DWORD dwReaders = SCARD_AUTOALLOCATE;
	WCHAR*	pReaders = NULL;
	WCHAR*	pReader = NULL;

	LOG_ENTER();
	HMODULE hWinscardLib = (HMODULE)loadPCSCLib();
	if (hWinscardLib == NULL) 
	{
		return EDT_ERR_LIBRARY_NOT_FOUND;
	}

	if (SCARD_S_SUCCESS != (err = g_fSCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hSC)))
	{
		LOG_EXIT_ERRORCODE(L"SCardEstablishContext failed",err);
		return EDT_ERR_PCSC_CONTEXT_FAILED;
	}
	else
	{
		LOG(L"SCardEstablishContext succeeded\n");
	}

	if (SCARD_S_SUCCESS != (err = g_fSCardListReaders(hSC,NULL,(LPWSTR)&pReaders,&dwReaders))) 
	{
		if(SCARD_E_NO_READERS_AVAILABLE != err)
		{
			LOG_ERRORCODE(L"SCardListReaders failed",err);
			iReturnCode = EDT_ERR_INTERNAL;
		}
		else
		{
			LOG_ERROR(L"SCardListReaders failed, no readers available");
			iReturnCode = EDT_ERR_READER_NOT_FOUND;
		}
	}
	else
	{
		LOG(L"SCardListReaders succeeded listlength in bytes = %d\n",dwReaders);
		if (dwReaders > 0) 
		{
			pReader = pReaders;

			while(*pReader != 0) 
			{
				LOG(L"Reader Found: %s\n",pReader);
				if(logCard == true)
				{
					SCARDHANDLE hCard = NULL;
					if(EDT_OK == Connect(hSC, pReader, &hCard))
					{
						std::wstring atr;
						if(EDT_OK == (iReturnCode = GetATR(hCard, &atr)))
						{
							//LOG(L"GetATR succeeded, ATR = %s\n",atr.c_str());
							//Do the card tests in sequence here
							LOG(L"LogCardData start\n");
							if(EDT_OK == LogCardData(hCard))
							{
								LOG(L"LogCardData succeeded\n");
							}
							else
							{
								LOG_ERROR(L"LogCardData failed");
							}
							LOG(L"ReadCard start\n");
							if(EDT_OK == ReadCard(hCard))
							{
								LOG(L"ReadCard succeeded\n");
							}
							else
							{
								LOG_ERROR(L"ReadCard failed");
							}





						}
						else
						{
							LOG_ERRORCODE(L"GetATR failed",iReturnCode);
						}
						Disconnect(hCard, false);
					}
				}
				pReader += wcslen((wchar_t *)pReader) + 1;
			}
		}
		if(SCARD_S_SUCCESS != (err = g_fSCardFreeMemory( hSC, pReaders )) )
		{
			LOG_ERRORCODE(L"SCardFreeMemory failed",err);
			iReturnCode = EDT_ERR_INTERNAL;
		}
		else
		{
			LOG(L"SCardFreeMemory succeeded\n");
		}
	}

	if (SCARD_S_SUCCESS != (err = g_fSCardReleaseContext(hSC)))
	{
		LOG_ERRORCODE(L"SCardReleaseContext failed",err);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	else
	{
		LOG(L"SCardReleaseContext succeeded\n");
	}
	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int ReadCard(SCARDHANDLE hCard)
{
	int iReturnCode = EDT_OK;
	DWORD pFileLen;

	LOG_ENTER();


	if(EDT_OK != (iReturnCode = BeginTransaction(hCard)))
	{
		LOG_EXIT_ERROR(L"BeginTransaction failed");
		return iReturnCode;
	}

	if(EDT_OK != (iReturnCode = SelectApplet(hCard)))
	{
		LOG_ERROR(L"SelectApplet failed");
	}

	BYTE selectIDDF[] = {0x00,0xA4,0x02,0x0C,0x02,0xDF,0x01};
	if(EDT_OK == SelectDF(hCard, selectIDDF, sizeof(selectIDDF)))
	{
		LOG(L"select ID directory file succeeded\n");
	}
	else
	{
		LOG(L"failed to select ID directory file\n");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	
	if(EDT_OK == ReadCardFile(hCard, REF_ID, sizeof(REF_ID), &pFileLen))
	{
		LOG(L"ReadCardFile REF_ID read, size is %d bytes \n",pFileLen);
		/*
		GetTlvValue(Content,BEID_FIELD_TAG_ID_Surname,&info.LastName);
		std::wstring FirstName;
		GetTlvValue(Content,BEID_FIELD_TAG_ID_FirstName_1,&FirstName);
		info.FirstName.assign(FirstName);
		GetTlvValue(Content,BEID_FIELD_TAG_ID_FirstName_2,&FirstName);*/
	}
	else
	{
		LOG(L"failed to read CardFile REF_ID size read is %d bytes \n",pFileLen);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(EDT_OK == ReadCardFile(hCard, REF_ADDRESS, sizeof(REF_ADDRESS), &pFileLen))
	{
		LOG(L"ReadCardFile REF_ADDRESS read, size is %d bytes \n",pFileLen);
	}
	else
	{
		LOG(L"failed to read CardFile REF_ADDRESS size read is %d bytes \n",pFileLen);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(EDT_OK == ReadCardFile(hCard, REF_PICTURE, sizeof(REF_PICTURE), &pFileLen))
	{
		LOG(L"ReadCardFile REF_PICTURE read, size is %d bytes \n",pFileLen);
	}
	else
	{
		LOG(L"failed to read CardFile REF_PICTURE size read is %d bytes \n",pFileLen);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	/*if(EDT_OK == ReadCardFile(hCard, REF_VERSION, sizeof(REF_VERSION), &pFileLen))
	{
		LOG(L"ReadCardFile REF_VERSION read, size is %d bytes \n",pFileLen);
	}
	else
	{
		LOG(L"failed to read CardFile REF_VERSION size read is %d bytes \n",pFileLen);
		iReturnCode = EDT_ERR_INTERNAL;
	}*/
	BYTE select_belpicDF[] = {0x00,0xA4,0x04,0x0C,0x0C,0xA0,0x00,0x00,0x01,0x77,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};
	if(EDT_OK != SelectDF( hCard,select_belpicDF, sizeof(select_belpicDF) ))
	{
		LOG_ERROR(L"SelectBelpicDF failed");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(EDT_OK == ReadCardFile(hCard, REF_CERTRRN, sizeof(REF_CERTRRN), &pFileLen))
	{
		LOG(L"ReadCardFile REF_CERTRRN read, size is %d bytes \n",pFileLen);
	}
	else
	{
		LOG(L"failed to read CardFile REF_CERTRRN size read is %d bytes \n",pFileLen);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(EDT_OK == ReadCardFile(hCard, REF_CERTROOT, sizeof(REF_CERTROOT), &pFileLen))
	{
		LOG(L"ReadCardFile REF_CERTROOT read, size is %d bytes \n",pFileLen);
	}
	else
	{
		LOG(L"failed to read CardFile REF_CERTROOT size read is %d bytes \n",pFileLen);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(EDT_OK == ReadCardFile(hCard, REF_CERTCA, sizeof(REF_CERTCA), &pFileLen))
	{
		LOG(L"ReadCardFile REF_CERTCA read, size is %d bytes \n",pFileLen);
	}
	else
	{
		LOG(L"failed to read CardFile REF_CERTCA size read is %d bytes \n",pFileLen);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(EDT_OK == ReadCardFile(hCard, REF_CERTSIGN, sizeof(REF_CERTSIGN), &pFileLen))
	{
		LOG(L"ReadCardFile REF_CERTSIGN read, size is %d bytes \n",pFileLen);
	}
	else
	{
		LOG(L"failed to read CardFile REF_CERTSIGN size read is %d bytes \n",pFileLen);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(EDT_OK == ReadCardFile(hCard, REF_CERTAUTH, sizeof(REF_CERTAUTH), &pFileLen))
	{
		LOG(L"ReadCardFile REF_CERTAUTH read, size is %d bytes \n",pFileLen);
	}
	else
	{
		LOG(L"failed to read CardFile REF_CERTAUTH size read is %d bytes \n",pFileLen);
		iReturnCode = EDT_ERR_INTERNAL;
	}

	if (EDT_OK != (iReturnCode = EndTransaction(hCard)))
	{
		LOG_EXIT_ERROR(L"EndTransaction failed");
		return iReturnCode;
	}

	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

long CardRecover(SCARDHANDLE hCard)
{
	//try to recover when the card is not responding (properly) anymore

	DWORD ap = 0;
	int i = 0;
	long lRet = SCARD_F_INTERNAL_ERROR;

	LOG_ERROR( L"Card is not responding properly, trying to recover...");
	LOG_ENTER();

	for (i = 0; (i < 10) && (lRet != SCARD_S_SUCCESS); i++)
	{
		if (i != 0)
		{
			LOG_SCREEN(L"Card Confused.. sleeping a second before retrying\r\n");
			Sleep(1000);
		}
		lRet = g_fSCardReconnect(hCard, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, SCARD_RESET_CARD, &ap);
		if ( lRet != SCARD_S_SUCCESS )
		{
			LOG(L"[%d] SCardReconnect errorcode: [0x%.2x]", i, lRet);
			LOG_ERROR(L"SCardReconnect error");
			continue;
		}
		else
		{
			LOG(L"[%d] SCardReconnect succeeded \n", i);
		}

		// transaction is lost after an SCardReconnect()
		if(g_TransactionStarted == true)
		{
			lRet = g_fSCardBeginTransaction(hCard);
			if ( lRet != SCARD_S_SUCCESS )
			{
				LOG(L"[%d] SCardBeginTransaction errorcode: [0x%02X]", i, lRet);
				LOG_ERROR(L"SCardBeginTransaction error");
				if(i == 9)
				{
					LOG_ERROR(L"transaction lost due to reconnect");
					g_TransactionStarted = false;//we lost the transaction
				}
				continue;
			}
			else
			{
				LOG(L"SCardBeginTransaction succeeded \n");
			}
		}
		LOG(L"Card recovered in loop %d", i);
	}
	if ( lRet != SCARD_S_SUCCESS )
	{
		LOG_ERROR(L"CardRecover failed.. stop trying..");
	}
	LOG_EXIT(lRet);
	return lRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int SendApdu(SCARDHANDLE hCard, LPBYTE pbSendBuffer, DWORD dwSendBufLen, LPBYTE  pbRecvBuffer, LPDWORD pdwRecvBufLen) 
{
	int iReturnCode = EDT_OK;

	LOG_ENTER();
	if((void*)hCard == NULL || pbSendBuffer == NULL || pbRecvBuffer == NULL)
	{
		LOG_EXIT_ERROR(L"SendApdu failed due too bad parameter provided");
		return EDT_ERR_BAD_PARAM;
	}

	BYTE SW1;
	BYTE SW2;
	LOG(L"SCardTransmit: \n");
	LOG_APDU(pbSendBuffer,dwSendBufLen);

	int pcscErr=SCARD_S_SUCCESS;
	DWORD dwRecvBufLen = *pdwRecvBufLen;

	pcscErr = g_fSCardTransmit(hCard,&g_ioSendPci,pbSendBuffer,dwSendBufLen,&g_ioRecvPci,pbRecvBuffer,pdwRecvBufLen);
	if (pcscErr != SCARD_S_SUCCESS)
	{
		if(pcscErr == SCARD_E_COMM_DATA_LOST || pcscErr == SCARD_E_NOT_TRANSACTED )
		{
			LOG_ERRORCODE(L"SCardTransmit failed with %d , trying to recover",pcscErr);
			pcscErr = CardRecover(hCard);
			if (pcscErr != SCARD_S_SUCCESS)
			{
				LOG_EXIT_ERROR(L"Cardrecover failed ");
				return EDT_ERR_PCSC_TRANSMIT_FAILED;
			}
			else
			{
				// try to select the applet
				const BYTE Cmd[] = {0x00,0xA4,0x04,0x00,0x0F,0xA0,0x00,0x00,0x00,0x30,0x29,0x05,0x70,0x00,0xAD,0x13,0x10,0x01,0x01,0xFF};
				LOG(L"selecting the applet\ncalling SCardTransmit, APDU = {0x00,0xA4,0x04,0x00,0x0F,0xA0,0x00,0x00,0x00,0x30,0x29,0x05,0x70,0x00,0xAD,0x13,0x10,0x01,0x01,0xFF}\n");
				*pdwRecvBufLen = dwRecvBufLen;
				pcscErr = g_fSCardTransmit(hCard,&g_ioSendPci,Cmd,sizeof(Cmd),&g_ioRecvPci,pbRecvBuffer,pdwRecvBufLen);
				if (pcscErr != SCARD_S_SUCCESS)
				{
					LOG_EXIT_ERROR(L"selecting the applet failed ");
					return EDT_ERR_PCSC_TRANSMIT_FAILED;
				}

				if( (*pdwRecvBufLen == 2) && 
					( (pbRecvBuffer[0] == 0x61) || ((pbRecvBuffer[0] == 0x90) && (pbRecvBuffer[1] == 0x00)) ) )
				{
					//try the initial command again, now that the card has been reset
					LOG(L"Retrying command\n");
					LOG_APDU(pbSendBuffer,dwSendBufLen);
					*pdwRecvBufLen = dwRecvBufLen;
					pcscErr = g_fSCardTransmit(hCard,&g_ioSendPci,pbSendBuffer,dwSendBufLen,&g_ioRecvPci,pbRecvBuffer,pdwRecvBufLen);
					{
						if (pcscErr != SCARD_S_SUCCESS)
						{
							LOG_EXIT_ERRORCODE(L"SCardTransmit failed (again) with pcsc error %d ",pcscErr);
							return EDT_ERR_PCSC_TRANSMIT_FAILED;
						}
					}
				}
			}
		}
		LOG_EXIT_ERRORCODE(L"SCardTransmit failed with 0x%.8x ",pcscErr);
		return EDT_ERR_PCSC_TRANSMIT_FAILED;
	}

	LOG(L"SCardTransmit returns SCARD_S_SUCCESS \n");

	if (*pdwRecvBufLen < 2) 
	{
		LOG(L"SCardTransmit receive buffer length = %d\n",*pdwRecvBufLen);
		LOG_EXIT_ERROR(L"SCardTransmit error : received too few bytes");
		return EDT_ERR_PCSC_TRANSMIT_FAILED; 
	}

	SW1 = pbRecvBuffer[(*pdwRecvBufLen)-2];
	SW2 = pbRecvBuffer[(*pdwRecvBufLen)-1];
	LOG(L"received buffer length = %d, SW1 = 0x%.2x, SW2 = 0x%.2x \n",*pdwRecvBufLen,SW1,SW2);

	if (*pdwRecvBufLen == 2)
	{
		// If SW1 = 0x61, then SW2 indicates the maximum value to be given to the
		// short Le  field (length of extra/ data still available) in a GET RESPONSE.
		if (SW1 == 0x61)
		{
			BYTE pbNewSendBuffer[5] = {0x00, 0xC0, 0x00, 0x00, 0x00};
			pbNewSendBuffer[4] = SW2;
			LOG(L"sending 'Get Response' APDU:  {0x00, 0xC0, 0x00, 0x00, 0x%.2x} \n",SW2);
			Sleep(25);
			*pdwRecvBufLen = dwRecvBufLen;
			LOG_EXIT(iReturnCode);
			return SendApdu( hCard, pbNewSendBuffer, 5, pbRecvBuffer, pdwRecvBufLen);
            //return SendAPDU(0xC0, 0x00, 0x00, oResp.GetByte(1)); // Get Response
		}
		// If SW1 = 0x6c, then SW2 indicates the value to be given to the short
		// Le field (exact length of requested data) when re-issuing the same command.
		if (SW1 == 0x6c)
		{
			if(dwSendBufLen < 5)
			{
				LOG(L"received SW=0x6C, but sendbuffer lenght is too short : %d bytes\n",dwSendBufLen); 
				LOG_EXIT_ERROR(L"received too few bytes"); 
				return EDT_ERR_PCSC_TRANSMIT_FAILED;
			}
			pbSendBuffer[4] = SW2;
			Sleep(25);

			LOG(L"resending APDU with le = %d \n",SW2);
			*pdwRecvBufLen = dwRecvBufLen;
			LOG_EXIT(iReturnCode);
			return SendApdu( hCard, pbSendBuffer, dwSendBufLen, pbRecvBuffer, pdwRecvBufLen);
		}
	}
	
	if( (SW1 != 0x90) && (SW2 != 0x00) &&
		(SW1 != 0x61) &&
		(SW1 != 0x6C) )
	{
		Sleep(25);
	}
	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int ReadCardFile(SCARDHANDLE hCard, const BYTE *Path, const int PathLen, DWORD *pFileLen) 
{
	int iReturnCode = EDT_OK;
	LOG_ENTER();

	if((void*)hCard == NULL || Path == NULL || PathLen <= 0 )
	{
		LOG_EXIT_ERROR(L"ReadCardFile : bad parameter supplied");
		return EDT_ERR_BAD_PARAM;
	}
	if(PathLen > 250)
	{
		LOG(L"PathLen = %d\n",PathLen);
		LOG_EXIT_ERROR(L"ReadCardFile : path Length too long");
		return EDT_ERR_INTERNAL;
	}

	//Build the 'select file' apdu
	DWORD selectFileCommandLen = PathLen+5;
	BYTE *pSelectFileCommand = (BYTE*)malloc(selectFileCommandLen);
	
	BYTE SelectFileCommand[4] = {0x00, 0xA4, 0x02, 0x0C};
	BYTE *pCommand = pSelectFileCommand+4;
	*pCommand = PathLen;
	pCommand++;
	memcpy(pSelectFileCommand,SelectFileCommand,sizeof(SelectFileCommand));
	memcpy(pCommand,Path,PathLen);

	BYTE precvBuf[2];
	DWORD recvBufLen = sizeof(precvBuf);

	//Run the 'select file'
	if(EDT_OK != (iReturnCode = SendApdu(hCard,pSelectFileCommand,selectFileCommandLen,precvBuf, &recvBufLen )))
	{
		LOG_ERROR(L"ReadCardFile 'SendApdu' failed");
		free(pSelectFileCommand);
		LOG_EXIT(iReturnCode);
		return iReturnCode;
	}
	free(pSelectFileCommand);
	if (precvBuf[0] != 0x90 || precvBuf[1] != 0x00) 
	{
		LOG_ERROR(L"SendApdu 'Select file' failed");
		LOG_EXIT(EDT_ERR_INTERNAL);
		return EDT_ERR_INTERNAL;
	}

	//Build the 'read binary' apdu
	BYTE SelectReadBinary[5] = {0x00, 0xB0, 0x00, 0x00, 0xF8};
	BYTE SelectReadBinaryBuf[250];//F8 hex = 248 dec + 2 SW
	DWORD SelectReadBinaryBufLen = sizeof(SelectReadBinaryBuf);

	BYTE SW1 = 0;
	BYTE SW2 = 0;
	*pFileLen = 0;
	int tries = 0;

	while( tries < 200 )
	{
		tries++;
		SelectReadBinaryBufLen = sizeof(SelectReadBinaryBuf);
		SelectReadBinary[2] = ((*pFileLen & 0xFF00) >> 8); // P1 = HiByte(*pFileLen) 
		SelectReadBinary[3] = (*pFileLen & 0xFF); // P2 = LoByte(*pFileLen) }
		if(EDT_OK != (iReturnCode = SendApdu(hCard,SelectReadBinary,sizeof(SelectReadBinary),SelectReadBinaryBuf,&SelectReadBinaryBufLen)))
		{
			LOG_ERROR(L"SendApdu 'Read binary' failed");
			LOG_EXIT(iReturnCode);
			return iReturnCode;
		}
		SW1 = SelectReadBinaryBuf[SelectReadBinaryBufLen-2];
		SW2 = SelectReadBinaryBuf[SelectReadBinaryBufLen-1];

		// offset is passed EOF 
		if (SW1 == 0x6B && SW2 == 0x00)
		{
			LOG(L"SendApdu 'Read binary' response: SW1 = 0x6B && SW2 = 0x00\n   'offset is passed EOF' \n");
			break;
		}

		// read final part -> already tried in sendapdu function
		if (SW1 == 0x6C) 
		{
			LOG(L"SendApdu 'Read binary' response: SW1 = 0x6C && SW2 = 0x%.2x\n 'read final part' \n");
			SelectReadBinary[4] = SW2; // gebruik juiste lengte voor Le 
			continue;
		}
		// read MAX_READ_SIZE bytes 
		if (SW1 != 0x90 || SW2 != 0x00)
		{
			LOG(L"SendApdu 'Read binary' response : SW1 = 0x%.2x && SW2 = 0x%.2x\n",SW1,SW2);
			LOG_ERROR(L"SendApdu 'Read binary' failed");
			iReturnCode = EDT_ERR_PCSC_TRANSMIT_FAILED;
			break;
		}
		else
		{
			*pFileLen += SelectReadBinaryBufLen-2;
			LOG(L"read another part; part %d\n",tries);
			if(SelectReadBinaryBufLen < sizeof(SelectReadBinaryBuf))
			{
				LOG(L"final part read\n");
				break;
			}
		}
	}

	LOG_EXIT(iReturnCode);
	return iReturnCode;
}
/*
////////////////////////////////////////////////////////////////////////////////////////////////
int GetTlvValue(const std::vector<BYTE>&FileContent, int Tag, std::wstring *Value) 
{
int iReturnCode = EDT_OK;

if(Value == NULL)
{
return EDT_ERR_BAD_PARAM;
}

Value->clear();

std::string strValue = "";

int ip;
int currentTag;
int size;

ip = 0;
while (ip < static_cast<int> (FileContent.size())) 
{
currentTag = FileContent.at(ip++);
size = FileContent.at(ip++);
if (size == 255) 
{
size = (size << 8) | FileContent.at(ip++);
}
if (currentTag == Tag) 
{
int j = ip;
for (int i = 0; i < size; ++i) 
{
strValue.push_back(FileContent.at(j++));
}
break;
}
ip += size;
}

Value->append(wstring_From_string(strValue));

return iReturnCode;
}*/

////////////////////////////////////////////////////////////////////////////////////////////////
int GetATR(SCARDHANDLE hCard, std::wstring *atr)
{
	int iReturnCode = EDT_OK;
	LOG_ENTER();

	if((void*)hCard == NULL || atr == NULL)
	{
		return EDT_ERR_BAD_PARAM;
	}

	atr->clear();

	DWORD dwReaderLen = 0;
	DWORD dwState, dwProtocol;
	unsigned char ucAtr[64];
	DWORD dwATRLen = sizeof(ucAtr);

	int err=SCARD_S_SUCCESS;

	if (SCARD_S_SUCCESS != (err = g_fSCardStatus(hCard, NULL, &dwReaderLen, &dwState, &dwProtocol, ucAtr, &dwATRLen))) 
	{
		LOG_EXIT_ERRORCODE(L"SCardStatus failed",err);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	else
	{
		wchar_t buff[3];
		for(DWORD i=0;i<dwATRLen;i++)
		{
			if(-1==swprintf_s(buff,3,L"%02X",ucAtr[i]))
			{
				LOG_ERROR(L"swprintf_s failed");
				iReturnCode = EDT_ERR_INTERNAL;
				break;
			}
			else
			{
				atr->append(buff);
			}
		}
		LOG(L"ATR = %s\n",atr->c_str());
	}
	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int LogCardData(SCARDHANDLE hCard)
{
	int iReturnCode = EDT_OK;
	LOG_ENTER();

	if((void*)hCard == NULL)
	{
		LOG_EXIT_ERRORCODE(L"LogCardData failed, hCard = NULL",EDT_ERR_BAD_PARAM);
		return EDT_ERR_BAD_PARAM;
	}

	BYTE getCardData[] = {0x80, 0xE4, 0x00, 0x00, 0x1C};//unsigned
	//responce is 0x1C long -> 28 bytes +2 SW
	BYTE pbRecvBuf[30];
	DWORD dwRecvBufLen = 30;

	if(EDT_OK != (iReturnCode = BeginTransaction(hCard)))
	{
		LOG_EXIT_ERROR(L"BeginTransaction failed");
		return iReturnCode;
	}

	if(EDT_OK != (iReturnCode = SelectApplet(hCard)))
	{
		LOG_ERROR(L"SelectApplet failed");
	}
	//getCardData

	if(EDT_OK != (iReturnCode = SendApdu(hCard,getCardData, sizeof(getCardData),pbRecvBuf,&dwRecvBufLen)))
	{
		LOG_ERROR(L"SendApdu 'getCardData' failed");
		if(EDT_OK != EndTransaction(hCard))
		{
			LOG_ERROR(L"EndTransaction failed");
		}
		LOG_EXIT(iReturnCode);
		return iReturnCode;
	}
	BYTE SW1 = pbRecvBuf[dwRecvBufLen-2];
	BYTE SW2 = pbRecvBuf[dwRecvBufLen-1];

	if (SW1 != 0x90 || SW2 != 0x00) 
	{
		LOG(L"SendApdu 'getCardData' failed statusbytes are SW1 = 0x%.2x, SW2 = 0x%.2x\n",SW1,SW2);
		LOG_ERROR(L"SendApdu 'getCardData' failed");
		if(EDT_OK != (iReturnCode = EndTransaction(hCard)))
		{
			LOG_ERROR(L"EndTransaction failed");
		}
		LOG_EXIT(iReturnCode);
		return EDT_ERR_INTERNAL;
	}
	LOG(L"SendApdu 'getCardData' succeeded\n");
	if(dwRecvBufLen < 30)
	{
		LOG(L"getCardData apdu returned not 28, but %d bytes\n",dwRecvBufLen);
		LOG_ERROR(L"getCardData apdu returned too few bytes");
	}
	else
	{
		BYTE *pbRecvBufData = pbRecvBuf;
		LOG(L"Serial Number = ");
		LOG_BYTE_ARRAY(pbRecvBufData,16);
		pbRecvBufData += 16;
		LOG(L"Component code = 0x%.2x\n",*pbRecvBufData);
		pbRecvBufData++;
		LOG(L"OS number = 0x%.2x\n",*pbRecvBufData);
		pbRecvBufData++;
		LOG(L"OS version = 0x%.2x\n",*pbRecvBufData);
		pbRecvBufData++;
		LOG(L"Softmask number = 0x%.2x\n",*pbRecvBufData);
		pbRecvBufData++;
		LOG(L"Softmask version = 0x%.2x\n",*pbRecvBufData);
		pbRecvBufData++;
		LOG(L"Application version = 0x%.2x\n",*pbRecvBufData);
		pbRecvBufData++;
		LOG(L"Global OS version = 0x%.2x 0x%.2x\n",*pbRecvBufData,*(pbRecvBufData+1));
		pbRecvBufData+=2;
		LOG(L"Application interface version = 0x%.2x\n",*pbRecvBufData);
		pbRecvBufData++;
		LOG(L"PKCS#1 support = 0x%.2x\n",*pbRecvBufData);
		pbRecvBufData++;
		LOG(L"Key exchange version = 0x%.2x\n",*pbRecvBufData);
		pbRecvBufData++;
		LOG(L"Application Life cycle = 0x%.2x\n",*pbRecvBufData);
	}

	if(EDT_OK != (iReturnCode = EndTransaction(hCard)))
	{
		LOG_ERROR(L"EndTransaction failed");
	}
	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*int SelectMF(SCARDHANDLE hCard, int * const TransmitDelay)
{
int iReturnCode = EDT_OK;

if((void*)hCard == NULL)
{
return EDT_ERR_BAD_PARAM;
}

if (EDT_OK != (iReturnCode = BeginTransaction(hCard)))
{
return iReturnCode;
}

if(EDT_OK != (iReturnCode = SelectApplet(hCard, TransmitDelay)))
{
LOG_ERROR(L"SelectMF 'SelectApplet' failed");
if(EDT_OK != (iReturnCode = EndTransaction(hCard)))
{
LOG_ERROR(L"SelectMF 'EndTransaction' failed");
}
return iReturnCode;
}

BYTE SW1=0xFF;
BYTE SW2=0xFF;

const BYTE SELECT_MF_APDU[] = {0x00, 0xA4, 0x02, 0x0C, 0x02, 0x3F, 0x00};

std::vector<BYTE> SelectMfCommand;

for (unsigned int i = 0; i < sizeof (SELECT_MF_APDU); ++i)
SelectMfCommand.push_back(SELECT_MF_APDU[i]);

std::vector<BYTE> Response;
//Run the 'select file'
if(EDT_OK != (iReturnCode = SendApdu(hCard,SelectMfCommand,&Response,&SW1,&SW2,TransmitDelay)))
{
LOG_ERROR(L"SendApdu 'Select MF' failed");
if(EDT_OK != (iReturnCode = EndTransaction(hCard)))
{
LOG_ERROR(L"EndTransaction failed");
}
return iReturnCode;
}

if (SW1 != 0x90 || SW2 != 0x00) 
{
LOG_PCSCERROR(L"SendApdu 'Select MF' failed",0,SW1,SW2);
if (EDT_OK != (iReturnCode = EndTransaction(hCard)))
{
LOG_ERROR(L"EndTransaction 'Select MF' failed");
}
return RETURN_LOG_INTERNAL_ERROR;
}

if (EDT_OK != (iReturnCode = EndTransaction(hCard)))
{
LOG_ERROR(L"EndTransaction 'Select MF' failed");
}

return iReturnCode;
}
*/