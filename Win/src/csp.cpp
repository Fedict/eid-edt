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

#include "edt.h"
#include <Wincrypt.h>
#include "csp.h"
//#include "log.h"



////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_CSP_FindCards(void);
int EDT_CSP_EnumProviders(void);
int EDT_CSP_ReadParam(HCRYPTPROV hProv,DWORD dwParam);

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_StartCSPLog()
{
	int iReturnCode = EDT_OK;
	int iFunctionCode = EDT_OK;
	bool pkcs11Available = false;
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_CSPLOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"testing csp...\r\n");

	LOG(L"CSP test\n");
	iReturnCode = EDT_CSP_EnumProviders();

	iFunctionCode = EDT_CSP_FindCards();
	if( (iReturnCode == EDT_OK) && (iFunctionCode!=EDT_OK) )
		iReturnCode = iFunctionCode;

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_CSPLOG_STOP>\n");
	LOG(EDT_LINE_BREAK);

	return iReturnCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

int EDT_CSP_EnumProviders()
{
	DWORD       dwIndex = 0;
	DWORD		dwProvType;
	DWORD       dwProvName;
	wchar_t     *pwcProvName = NULL; 

	LOG(L"CSP Providers found:\n");

	while(CryptEnumProviders(dwIndex, NULL, 0, &dwProvType, NULL, &dwProvName) == TRUE)
	{
		//dwProvName returns the length of pwcProvName
		pwcProvName = (wchar_t*)malloc(dwProvName);
		if(pwcProvName == NULL)
		{
			LOG_ERROR(L"EDT_CSP_EnumProviders : malloc failed\n");
			return EDT_ERR_INTERNAL;
		}
		if (CryptEnumProviders(dwIndex, NULL, 0, &dwProvType, pwcProvName, &dwProvName) == TRUE)
		{
			LOG(L"%d) name:%s type:%d\n",dwIndex,pwcProvName,dwProvType);
		}
		else
		{
			LOG_ERROR(L"CryptEnumProviders failed\n");
			return EDT_ERR_INTERNAL;
		}
		free(pwcProvName);
		dwIndex++;
	}
	return EDT_OK;
}


int EDT_CSP_FindCards()
{
	int iRetVal = EDT_OK;
	DWORD err;
	HCRYPTPROV hProv = NULL;
	//Get a context
	if (CryptAcquireContext(&hProv, NULL, L"Belgium Identity Card CSP", PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) != TRUE) 
	{
		err = GetLastError();
		LOG(L"CryptAcquireContext \"Belgium Identity Card CSP\" failed with %d\n",err);
		//our old csp failed, try the minidriver
		if (CryptAcquireContext(&hProv, NULL, L"Microsoft Base Smart Card Crypto Provider", PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) != TRUE) 
		{
			err = GetLastError();
			LOG(L"CryptAcquireContext \"Microsoft Base Smart Card Crypto Provider\" failed with %d\n",err);
			iRetVal = EDT_ERR_CSP_FAILED;
		}
		if(iRetVal == EDT_OK)
		{
			EDT_CSP_ReadParam(hProv,PP_SMARTCARD_READER);	
			EDT_CSP_ReadParam(hProv,PP_SMARTCARD_GUID);
			EDT_CSP_ReadParam(hProv,PP_USER_CERTSTORE);			
		}
	}
	if (hProv!=NULL && !CryptReleaseContext(hProv, 0))
	{
		LOG_LASTERROR(L"CryptReleaseContext failed");
		iRetVal = EDT_ERR_CSP_FAILED;
	}
	return iRetVal;
} 

int EDT_CSP_ReadParam(HCRYPTPROV hProv,DWORD dwParam)
{
	int iRetVal = EDT_OK;
	BYTE *pbData = NULL;
	DWORD dwDataLen = 0;
	DWORD dwFlags = 0;

	if (CryptGetProvParam(hProv, dwParam, NULL, &dwDataLen, dwFlags) == TRUE )
	{
		pbData = (BYTE *)malloc(dwDataLen);
		if(pbData != NULL)
		{
			if (CryptGetProvParam(hProv, dwParam, pbData, &dwDataLen, dwFlags) == TRUE )
			{
				GUID *pTheGuid;
				HCERTSTORE *phCertStore;
				PCCERT_CONTEXT pPrevCertContext = NULL;
				switch(dwParam)
				{
				case PP_SMARTCARD_READER:
					LOG(L"SmartCard Reader: %hs\n",pbData);
					break;
				case PP_SMARTCARD_GUID:
					pTheGuid = (GUID *)pbData;
					LOG(L"SmartCard GUID: ");
					LOG_BYTE_ARRAY(pTheGuid->Data4,sizeof(pTheGuid->Data4));
					break;
				case PP_USER_CERTSTORE:
					phCertStore = (HCERTSTORE*)pbData;
					pPrevCertContext = CertEnumCertificatesInStore(*phCertStore,pPrevCertContext);
					while(pPrevCertContext != NULL)
					{
						LOG(L"Found certificate with length %d\n",pPrevCertContext->cbCertEncoded);
						pPrevCertContext = CertEnumCertificatesInStore(*phCertStore,pPrevCertContext);						
					}
					if ( CertCloseStore(*phCertStore,0) == FALSE )
					{
						LOG_ERROR(L"CertCloseStore failed");
					}
					break;
				default:
					break;
				}
			}
			else
			{
				LOG_LASTERROR(L"CryptGetProvParam failed 2e \n");
			}
			free(pbData);
		}
		else
		{
			LOG_ERROR(L"malloc failed\n");
		}
	}
	else
	{
		LOG_LASTERROR(L"CryptGetProvParam failed\n");
	}
	return iRetVal;
}
