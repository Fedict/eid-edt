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
#include <EDT.h>

#include "util_registry.h"
#include "log.h"
#include "Sddl.h"

#define G_BUFFER_SIZE 32767
static BYTE g_buffer[G_BUFFER_SIZE];

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
// hRootKey values are HKEY_CLASSES_ROOT   HKEY_CURRENT_USER    HKEY_LOCAL_MACHINE    HKEY_USERS 
////////////////////////////////////////////////////////////////////////////////////////////////
int registryGetValue(HKEY hRootKey, const wchar_t *wzKey, const wchar_t *wzName, std::wstring *ValueStr)
{
	int iReturnCode = EDT_OK;
	int err = ERROR_SUCCESS;

    HKEY hRegKey;
	LOG_TIME(L"Ask for registry value (%ls\\%ls@%ls) --> ",hRootKey==HKEY_CURRENT_USER?L"HKCU":L"HKLM",wzKey,wzName);
	if(ValueStr == NULL)
		return EDT_ERR_BAD_PARAM;

	if(ERROR_SUCCESS != (err = RegOpenKeyEx(hRootKey, wzKey, 0L, KEY_READ , &hRegKey)))
	{
		if(err != ERROR_FILE_NOT_FOUND)
		{
			LOG_ERRORCODE(L"RegOpenKeyEx failed",err);
			return EDT_ERR_REGISTRY_READ_FAILED;
		}
		else
		{
			LOG(L"NOT FOUND\n");
			return EDT_ERR_REGISTRY_NOT_FOUND;
		}
    }
	ValueStr->clear();
    //--- get the value
    DWORD dwType = REG_NONE;
	DWORD dwValDatLen = G_BUFFER_SIZE; 
    if(ERROR_SUCCESS != (err = RegQueryValueEx(hRegKey, wzName, 0L, &dwType, g_buffer, &dwValDatLen)))
	{     
 		LOG_ERRORCODE(L"RegQueryValueEx failed",err);
		iReturnCode = EDT_ERR_REGISTRY_READ_FAILED;
	}
	else
	{
		switch(dwType)
		{
		case REG_SZ:
		case REG_EXPAND_SZ:
			ValueStr->append((wchar_t*)g_buffer);
			LOG(L"DONE\n");
			break;
		case REG_DWORD:
			wchar_t buf[16];
			if(-1==swprintf_s(buf,16,L"%ld",*(DWORD*)g_buffer))
			{
				LOG_ERROR(L"swprintf_s failed");
			}
			else
			{
				ValueStr->append(buf);
			}
			LOG(L"DONE\n");
			break;
		default:
			LOG_ERROR(L"Unmanaged data type");
			iReturnCode = EDT_ERR_NOT_AVAILABLE;
		}
	}

	if(ERROR_SUCCESS != (err = RegCloseKey(hRegKey)))
	{
		LOG_ERRORCODE(L"RegCloseKey failed",err);
	}

	return iReturnCode;
} 

int registryLogPermissions(HKEY hRootKey, const wchar_t *wzKey)
{
	int iReturnCode = EDT_OK;
	int err = ERROR_SUCCESS;

    HKEY hRegKey;
	LOG_ENTER();
	LOG_TIME(L"registry key (%ls\\%ls) --> \n",hRootKey==HKEY_CURRENT_USER?L"HKCU":L"HKLM",wzKey);

	if(ERROR_SUCCESS != (err = RegOpenKeyEx(hRootKey, wzKey, 0L, KEY_READ , &hRegKey)))
	{
		if(err != ERROR_FILE_NOT_FOUND)
		{
			LOG_ERRORCODE(L"RegOpenKeyEx failed",err);
			return EDT_ERR_REGISTRY_READ_FAILED;
		}
		else
		{
			LOG_EXIT(L"NOT FOUND\n");
			return EDT_ERR_REGISTRY_NOT_FOUND;
		}
    }
	PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
	DWORD lcbSecurityDescriptor = 0;

	if(ERROR_INSUFFICIENT_BUFFER == (err = RegGetKeySecurity(hRegKey, DACL_SECURITY_INFORMATION, pSecurityDescriptor, &lcbSecurityDescriptor)))
	{     
		pSecurityDescriptor = (PSECURITY_DESCRIPTOR)malloc(lcbSecurityDescriptor);

		if(ERROR_SUCCESS != (err = RegGetKeySecurity(hRegKey, DACL_SECURITY_INFORMATION, pSecurityDescriptor, &lcbSecurityDescriptor)))
		{
 			LOG_ERRORCODE(L"RegQueryValueEx failed",err);
			iReturnCode = EDT_ERR_REGISTRY_READ_FAILED;
		}
		else
		{
			BOOL bDaclPresent = FALSE;
			BOOL bDaclDefaulted = FALSE;
			PACL pDacl = NULL;

			GetSecurityDescriptorDacl(pSecurityDescriptor, &bDaclPresent, &pDacl, &bDaclDefaulted);
			if(bDaclPresent == TRUE)
			{
				if(pDacl == NULL)
				{
					LOG(L"A NULL discretionary access control list (DACL) found \nA NULL DACL implicitly allows all access to an object.\n");
				}
				else
				{
					LOG(L"A discretionary access control list (DACL) was found with Length = %d\n",pDacl->AclSize);
					LOG(L"Number of Access Control Elements (ACE's): %d\n",pDacl->AceCount);
					DWORD i = 0;
					LPVOID pAce = NULL;
					PACE_HEADER pAceheader=NULL;
					for(;i<pDacl->AceCount;i++)
					{
						GetAce(pDacl,i,&pAce);
						pAceheader = (PACE_HEADER)pAce;
						pAceheader->AceType;
						ACCESS_ALLOWED_ACE;
						//ACCESS_ALLOWED_ACE*  pTempAce;
					}
				}
			}
			else
			{
				LOG(L"No discretionary access control list (DACL) found \n");
			}

			LPTSTR StringSecurityDescriptor;
			ULONG StringSecurityDescriptorLen;

			ConvertSecurityDescriptorToStringSecurityDescriptor(pSecurityDescriptor,SDDL_REVISION_1,DACL_SECURITY_INFORMATION,
				&StringSecurityDescriptor,&StringSecurityDescriptorLen);


			LOG(L"%s\n",(const wchar_t*)StringSecurityDescriptor);
			//parse info see http://msdn.microsoft.com/en-us/library/aa379570%28v=vs.85%29.aspx

			LocalFree(StringSecurityDescriptor);

		}
		free(pSecurityDescriptor);
	}
	else
	{
		LOG_ERRORCODE(L"RegGetKeySecurity failed",err);
	}

	if(ERROR_SUCCESS != (err = RegCloseKey(hRegKey)))
	{
		LOG_ERRORCODE(L"RegCloseKey failed",err);
	}

	LOG_EXIT(iReturnCode);
	return iReturnCode;
} 




