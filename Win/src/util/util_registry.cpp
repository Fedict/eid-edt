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
int registryLogAces(PACL pDacl);
void RegistryLogAceMask(DWORD aceMask);
void RegistryLogAceFlags(DWORD AceFlags);
void RegistryLogAceSidStart(PSID pSid);
void RegistryLogGeneralRIDS(LPTSTR stringIdentifierAuthority, DWORD sidSubAuthority);

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
					registryLogAces(pDacl);
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

int registryLogAces(PACL pDacl)
{
	int iReturnCode = EDT_OK;
	DWORD i = 0;
	LPVOID pAce = NULL;
	PACE_HEADER pAceheader=NULL;

	LOG_ENTER();
	for(;i<pDacl->AceCount;i++)
	{
		GetAce(pDacl,i,&pAce);
		pAceheader = (PACE_HEADER)pAce;
		pAceheader->AceType;
		pAceheader->AceSize;
		LOG(L"-------------------\n",pAceheader->AceFlags);
		LOG(L"AceFlags are 0x%.2x\n",pAceheader->AceFlags);
		RegistryLogAceFlags(pAceheader->AceFlags);
		LOG(L"AceSize  is  0x%.2x\n",pAceheader->AceSize);
		LOG(L"AceType  is  0x%.2x\n",pAceheader->AceType);
		switch(pAceheader->AceType)
		{
		case ACCESS_ALLOWED_ACE_TYPE:
		case ACCESS_DENIED_ACE_TYPE:
		case SYSTEM_AUDIT_ACE_TYPE:
		case SYSTEM_ALARM_ACE_TYPE:
			//case SYSTEM_MANDATORY_LABEL_ACE:
			{
				PACCESS_ALLOWED_ACE pAccessAllowedAce = (PACCESS_ALLOWED_ACE)pAce;
				LOG(L"AceMask  is 0x%.8x, this means access permissions are:\n",pAccessAllowedAce->Mask);
				RegistryLogAceMask(pAccessAllowedAce->Mask);
				LOG(L"The above access permissions are given to following SID'S:\n");
				RegistryLogAceSidStart( &(pAccessAllowedAce->SidStart));
			}
			break;
		case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
		case ACCESS_DENIED_OBJECT_ACE_TYPE:
		case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
		case SYSTEM_ALARM_OBJECT_ACE_TYPE:
			{
				PACCESS_ALLOWED_OBJECT_ACE pAccessAllowedAce = (PACCESS_ALLOWED_OBJECT_ACE)pAce;
				LOG(L"AceMask  is 0x%.8x, this means access permissions are:\n",pAccessAllowedAce->Mask);
				RegistryLogAceMask(pAccessAllowedAce->Mask);
				LOG(L"The above access permissions are given to following SID'S:\n");
				RegistryLogAceSidStart( &(pAccessAllowedAce->SidStart));
			}
			break;

		default:
			LOG_BASIC(L"AceType unknow\n");
			LOG(L"Don't know the ACE type, cannot parse the ACE");
			break;
		};
	}

	LOG_EXIT(iReturnCode);
	return iReturnCode;
}

void RegistryLogAceMask(DWORD aceMask)
{
	LOG_INC_INDENT();

	if(GENERIC_ALL & aceMask)
		LOG(L"GENERIC_ALL\n");
	if(GENERIC_READ & aceMask)
		LOG(L"GENERIC_READ\n");
	if(GENERIC_WRITE & aceMask)
		LOG(L"SDDL_GENERIC_WRITE\n");
	if(GENERIC_EXECUTE & aceMask)
		LOG(L"SDDL_GENERIC_EXECUTE\n");
	//Standard access rights
	if(READ_CONTROL & aceMask)
		LOG(L"READ_CONTROL\n");
	if(DELETE & aceMask)
		LOG(L"DELETE\n");
	if(WRITE_DAC & aceMask)
		LOG(L"WRITE_DAC\n");
	if(WRITE_OWNER & aceMask)
		LOG(L"SDDL_WRITE_OWNER\n");
	//Directory service object access rights
	/*if(ADS_RIGHT_DS_READ_PROP & aceMask)
	LOG(L"ADS_RIGHT_DS_READ_PROP\n");
	if(ADS_RIGHT_DS_WRITE_PROP & aceMask)
	LOG(L"ADS_RIGHT_DS_WRITE_PROP\n");
	if(ADS_RIGHT_DS_CREATE_CHILD & aceMask)
	LOG(L"ADS_RIGHT_DS_CREATE_CHILD\n");
	if(ADS_RIGHT_DS_DELETE_CHILD & aceMask)
	LOG(L"ADS_RIGHT_DS_DELETE_CHILD\n");
	if(ADS_RIGHT_ACTRL_DS_LIST & aceMask)
	LOG(L"ADS_RIGHT_ACTRL_DS_LIST\n");
	if(ADS_RIGHT_DS_SELF & aceMask)
	LOG(L"ADS_RIGHT_DS_SELF\n");
	if(ADS_RIGHT_DS_LIST_OBJECT & aceMask)
	LOG(L"ADS_RIGHT_DS_LIST_OBJECT\n");
	if(ADS_RIGHT_DS_DELETE_TREE & aceMask)
	LOG(L"ADS_RIGHT_DS_DELETE_TREE\n");
	if(ADS_RIGHT_DS_CONTROL_ACCESS & aceMask)
	LOG(L"ADS_RIGHT_DS_CONTROL_ACCESS\n");*/
	//File access rights
	if(FILE_ALL_ACCESS & aceMask)
		LOG(L"FILE_ALL_ACCESS\n");
	if(FILE_GENERIC_READ & aceMask)
		LOG(L"FILE_GENERIC_READ\n");
	if(FILE_GENERIC_WRITE & aceMask)
		LOG(L"FILE_GENERIC_WRITE\n");
	if(FILE_GENERIC_EXECUTE & aceMask)
		LOG(L"FILE_GENERIC_EXECUTE\n");
	//Registry key access rights
	if(KEY_ALL_ACCESS & aceMask)
		LOG(L"KEY_ALL_ACCESS\n");
	if(KEY_READ & aceMask)
		LOG(L"KEY_READ\n");
	if(KEY_WRITE & aceMask)
		LOG(L"KEY_WRITE\n");
	if(KEY_EXECUTE & aceMask)
		LOG(L"KEY_EXECUTE\n");
	//Mandatory label rights
	if(SYSTEM_MANDATORY_LABEL_NO_READ_UP & aceMask)
		LOG(L"SYSTEM_MANDATORY_LABEL_NO_READ_UP\n");
	if(SYSTEM_MANDATORY_LABEL_NO_WRITE_UP & aceMask)
		LOG(L"SYSTEM_MANDATORY_LABEL_NO_WRITE_UP\n");
	if(SYSTEM_MANDATORY_LABEL_NO_EXECUTE_UP & aceMask)
		LOG(L"SYSTEM_MANDATORY_LABEL_NO_EXECUTE_UP\n");

	LOG_DEC_INDENT();
}

void RegistryLogAceSidStart(PSID SidStart)
{
	LOG_INC_INDENT();

	if (FALSE == IsValidSid(SidStart))
	{
		LOG(L"Invalid Sid given, cannot parse\n");
		LOG_DEC_INDENT();
		return;
	}
	LPTSTR stringSid = NULL;
	if(FALSE == ConvertSidToStringSid(SidStart,&stringSid))
	{
		LOG(L"Could not convert SID to SIDString\n");
		LOG_DEC_INDENT();
		return;
	}
	LOG(L"SID = %s\n",stringSid);

	PSID_IDENTIFIER_AUTHORITY sidia = GetSidIdentifierAuthority(SidStart);

	PUCHAR pSubAuthorityCount = GetSidSubAuthorityCount(SidStart);

	UCHAR counter = 0;
	for(; counter < *pSubAuthorityCount ; counter++)
	{
		PDWORD pSidSubAuthority = GetSidSubAuthority(SidStart,counter);
		BYTE nullauthorityValue[6] = SECURITY_NULL_SID_AUTHORITY;
		BYTE worldauthorityValue[6] = SECURITY_WORLD_SID_AUTHORITY;
		BYTE localauthorityValue[6] = SECURITY_LOCAL_SID_AUTHORITY;
		BYTE creatorauthorityValue[6] = SECURITY_CREATOR_SID_AUTHORITY;
		BYTE ntauthorityValue[6] = SECURITY_NT_AUTHORITY;

		if(memcmp(sidia->Value,nullauthorityValue,6) == 0)
		{
			switch(*pSidSubAuthority)
			{
			case SECURITY_NULL_RID:
				LOG(L"SECURITY_NULL\n");
				break;
			default:
				RegistryLogGeneralRIDS(L"SECURITY_NULL_SID_AUTHORITY",*pSidSubAuthority);
				break;
			};
		}
		else if (memcmp(sidia->Value,worldauthorityValue,6) == 0)
		{
			switch(*pSidSubAuthority)
			{
			case SECURITY_WORLD_RID:
				LOG(L"EVERYONE\n");
				break;	
			default:
				RegistryLogGeneralRIDS(L"SECURITY_WORLD_SID_AUTHORITY",*pSidSubAuthority);
				break;
			};
		}
		else if (memcmp(sidia->Value,localauthorityValue,6) == 0)
		{
			switch(*pSidSubAuthority)
			{	
			case SECURITY_LOCAL_RID:
				LOG(L"SECURITY_LOCAL_SID_AUTHORITY SECURITY_LOCAL_RID\n");
				break;
			case SECURITY_LOCAL_LOGON_RID:
				LOG(L"SECURITY_LOCAL_SID_AUTHORITY SECURITY_LOCAL_LOGON_RID\n");
				break;		
			default:
				RegistryLogGeneralRIDS(L"SECURITY_LOCAL_SID_AUTHORITY",*pSidSubAuthority);
				break;
			};
		}
		else if (memcmp(sidia->Value,creatorauthorityValue,6) == 0)
		{
			switch(*pSidSubAuthority)
			{	
			case SECURITY_CREATOR_OWNER_RID:
				LOG(L"CREATOR_OWNER\n");
				//LOG(L"SECURITY_CREATOR_SID_AUTHORITY SECURITY_CREATOR_OWNER_RID\n");
				break;
			case SECURITY_CREATOR_GROUP_RID:
				LOG(L"CREATOR_GROUP\n");
				//LOG(L"SECURITY_CREATOR_SID_AUTHORITY SECURITY_CREATOR_GROUP_RID\n");
				break;		
			default:
				RegistryLogGeneralRIDS(L"SECURITY_CREATOR_SID_AUTHORITY",*pSidSubAuthority);
				break;
			};
		}
		else if (memcmp(sidia->Value,ntauthorityValue,6) == 0)
		{
			switch(*pSidSubAuthority)
			{	
			case SECURITY_DIALUP_RID:
				LOG(L"DIALUP\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_DIALUP_RID: Users who log on to terminals using a dial-up modem. This is a group identifier\n");
				break;
			case SECURITY_NETWORK_RID:
				LOG(L"NETWORK\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_NETWORK_RID: Users who log on across a network. This is a group identifier\n");
				break;		
			case SECURITY_BATCH_RID:
				LOG(L"BATCH\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_BATCH_RID: Users who log on using a batch queue facility. This is a group identifier\n");
				break;
			case SECURITY_INTERACTIVE_RID:
				LOG(L"INTERACTIVE\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_INTERACTIVE_RID: Users who log on for interactive operation. This is a group identifier\n");
				break;	
			case SECURITY_LOGON_IDS_RID:
				LOG(L"LOGON_IDS\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_LOGON_IDS_RID: A logon session\n");
				break;
			case SECURITY_SERVICE_RID:
				LOG(L"SERVICE\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_SERVICE_RID: Accounts authorized to log on as a service. This is a group identifier\n");
				break;		
			case SECURITY_ANONYMOUS_LOGON_RID:
				LOG(L"ANONYMOUS\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_ANONYMOUS_LOGON_RID: Anonymous logon, or null session logon\n");
				break;
			case SECURITY_PROXY_RID:
				LOG(L"PROXY\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_PROXY_RID: Proxy\n");
				break;	
			case SECURITY_ENTERPRISE_CONTROLLERS_RID:
				LOG(L"ENTERPRISE_CONTROLLERS\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_ENTERPRISE_CONTROLLERS_RID: Enterprise controllers\n");
				break;
			case SECURITY_PRINCIPAL_SELF_RID:
				LOG(L"PRINCIPAL_SELF\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_PRINCIPAL_SELF_RID: The PRINCIPAL_SELF security identifier\n");
				break;		
			case SECURITY_AUTHENTICATED_USER_RID:
				LOG(L"AUTHENTICATED_USER\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_AUTHENTICATED_USER_RID: The authenticated users\n");
				break;
			case SECURITY_RESTRICTED_CODE_RID:
				LOG(L"RESTRICTED_CODE\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_RESTRICTED_CODE_RID: Restricted code\n");
				break;	
			case SECURITY_TERMINAL_SERVER_RID:
				LOG(L"TERMINAL_SERVER\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_TERMINAL_SERVER_RID: Terminal Services\n");
				break;
			case SECURITY_LOCAL_SYSTEM_RID:
				LOG(L"LOCAL_SYSTEM\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_LOCAL_SYSTEM_RID: A special account used by the operating system\n");
				break;		
			case SECURITY_NT_NON_UNIQUE:
				LOG(L"NT_NON_UNIQUE\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_NT_NON_UNIQUE: SIDS are not unique\n");
				break;
			case SECURITY_BUILTIN_DOMAIN_RID:
				LOG(L"BUILTIN_DOMAIN\n");
				//LOG(L"SECURITY_NT_AUTHORITY SECURITY_BUILTIN_DOMAIN_RID: Buildin Domain\n");
				break;	
			default:
				RegistryLogGeneralRIDS(L"SECURITY_NT_AUTHORITY",*pSidSubAuthority);
				break;
			};
		}
	}
	LOG_DEC_INDENT();
}

void RegistryLogGeneralRIDS(LPTSTR stringIdentifierAuthority, DWORD sidSubAuthority)
{
	switch(sidSubAuthority)
	{
	case SECURITY_ANONYMOUS_LOGON_RID:
		LOG(L"ANONYMOUS_LOGON\n");
		break;
	case DOMAIN_ALIAS_RID_ACCOUNT_OPS:
		LOG(L"ACCOUNT_OPERATORS\n");
		break;
	case SECURITY_AUTHENTICATED_USER_RID:
		LOG(L"AUTHENTICATED_USERS\n");
		break;
	case DOMAIN_ALIAS_RID_ADMINS:
		LOG(L"BUILTIN_ADMINISTRATORS\n");
		break;
	case DOMAIN_ALIAS_RID_GUESTS:
		LOG(L"BUILTIN_GUESTS\n");
		break;
	case DOMAIN_ALIAS_RID_BACKUP_OPS:
		LOG(L"BACKUP_OPERATORS\n");
		break;
	case DOMAIN_ALIAS_RID_USERS:
		LOG(L"BUILTIN_USERS\n");
		break;
	case DOMAIN_GROUP_RID_CERT_ADMINS:
		LOG(L"CERT_SERV_ADMINISTRATORS\n");
		break;
	case DOMAIN_ALIAS_RID_CERTSVC_DCOM_ACCESS_GROUP:
		LOG(L"CERTSVC_DCOM_ACCESS\n");
		break;
	case SECURITY_CREATOR_GROUP_RID:
		LOG(L"CREATOR_GROUP\n");
		break;
	case SECURITY_CREATOR_OWNER_RID:
		LOG(L"CREATOR_OWNER\n");
		break;
	case DOMAIN_GROUP_RID_ADMINS:
		LOG(L"DOMAIN_ADMINISTRATORS\n");
		break;
	case DOMAIN_GROUP_RID_COMPUTERS:
		LOG(L"DOMAIN_COMPUTERS\n");
		break;
	case DOMAIN_GROUP_RID_CONTROLLERS:
		LOG(L"DOMAIN_DOMAIN_CONTROLLERS\n");
		break;
	case DOMAIN_GROUP_RID_GUESTS:
		LOG(L"DOMAIN_GUESTS\n");
		break;
	case DOMAIN_GROUP_RID_USERS:
		LOG(L"DOMAIN_USERS\n");
		break;
	case DOMAIN_GROUP_RID_ENTERPRISE_ADMINS:
		LOG(L"ENTERPRISE_ADMINS\n");
		break;
	case SECURITY_SERVER_LOGON_RID:
		LOG(L"ENTERPRISE_DOMAIN_CONTROLLERS\n");
		break;
	case SECURITY_MANDATORY_HIGH_RID:
		LOG(L"ML_HIGH\n");
		break;
	case SECURITY_INTERACTIVE_RID:
		LOG(L"INTERACTIVE\n");
		break;	
	case DOMAIN_USER_RID_ADMIN:
		LOG(L"LOCAL_ADMIN\n");
		break;
	case DOMAIN_USER_RID_GUEST:
		LOG(L"LOCAL_GUEST\n");
		break;
	case SECURITY_LOCAL_SERVICE_RID:
		LOG(L"LOCAL_SERVICE\n");
		break;
	case SECURITY_MANDATORY_LOW_RID:
		LOG(L"ML_LOW\n");
		break;
	case SECURITY_MANDATORY_MEDIUM_RID:
		LOG(L"MLMEDIUM\n");
		break;
		//Performance Monitor users.?
	case DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS:
		LOG(L"NETWORK_CONFIGURATION_OPS\n");
		break;
	case SECURITY_NETWORK_SERVICE_RID:
		LOG(L"NETWORK_SERVICE\n");
		break;
	case SECURITY_NETWORK_RID:
		LOG(L"NETWORK\n");
		break;
	case DOMAIN_GROUP_RID_POLICY_ADMINS:
		LOG(L"GROUP_POLICY_ADMINS\n");
		break;
	case DOMAIN_ALIAS_RID_PRINT_OPS:
		LOG(L"PRINTER_OPERATORS\n");
		break;
	case SECURITY_PRINCIPAL_SELF_RID:
		LOG(L"PERSONAL_SELF\n");
		break;
	case DOMAIN_ALIAS_RID_POWER_USERS:
		LOG(L"POWER_USERS\n");
		break;
	case SECURITY_RESTRICTED_CODE_RID:
		LOG(L"RESTRICTED_CODE\n");
		break;
	case DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS:
		LOG(L"REMOTE_DESKTOP\n");
		break;
	case DOMAIN_ALIAS_RID_REPLICATOR:
		LOG(L"REPLICATOR\n");
		break;
	case DOMAIN_GROUP_RID_ENTERPRISE_READONLY_DOMAIN_CONTROLLERS:
		LOG(L"ENTERPRISE_READONLY_DOMAIN_CONTROLLERS\n");
		break;
	case DOMAIN_ALIAS_RID_RAS_SERVERS:
		LOG(L"RAS_SERVERS\n");
		break;
	case DOMAIN_ALIAS_RID_PREW2KCOMPACCESS:
		LOG(L"ALIAS_PREW2KCOMPACC\n");
		break;
	case DOMAIN_GROUP_RID_SCHEMA_ADMINS:
		LOG(L"SCHEMA_ADMINISTRATORS\n");
		break;
	case SECURITY_MANDATORY_SYSTEM_RID:
		LOG(L"ML_SYSTEM\n");
		break;
	case DOMAIN_ALIAS_RID_SYSTEM_OPS:
		LOG(L"SERVER_OPERATORS\n");
		break;
	case SECURITY_SERVICE_RID:
		LOG(L"SERVICE\n");
		break;
	case SECURITY_LOCAL_SYSTEM_RID:
		LOG(L"LOCAL_SYSTEM\n");
		break;
	default:
		LOG(L"%s, sidSubAuthority = %d\n",stringIdentifierAuthority,sidSubAuthority);
		break;
	};
}

void RegistryLogAceFlags(DWORD AceFlags)
{
	LOG_INC_INDENT();

	if(AceFlags & CONTAINER_INHERIT_ACE)
		LOG(L"CONTAINER_INHERIT_ACE\n");
	if(AceFlags & OBJECT_INHERIT_ACE)
		LOG(L"OBJECT_INHERIT_ACE\n");
	if(AceFlags & NO_PROPAGATE_INHERIT_ACE)
		LOG(L"NO_PROPAGATE_INHERIT_ACE\n");
	if(AceFlags & INHERIT_ONLY_ACE)
		LOG(L"INHERIT_ONLY_ACE\n");
	if(AceFlags & INHERITED_ACE)
		LOG(L"INHERITED_ACE\n");
	if(AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG)
		LOG(L"SUCCESSFUL_ACCESS_ACE_FLAG\n");
	if(AceFlags & FAILED_ACCESS_ACE_FLAG)
		LOG(L"FAILED_ACCESS_ACE_FLAG\n");

	LOG_DEC_INDENT();
}