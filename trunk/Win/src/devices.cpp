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

#include "edt.h" //includes <windows.h>
#include "log.h"
#include "devices.h"
#include "Setupapi.h"
#include "util\util_registry.h"
#include <Cfgmgr32.h>//for CM_DEVCAP_Xxx constants

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_DEV_LogAlldevices();
//caution: when ppBuf != NULL, the returned buffer needs to be freed by the caller, even when an error is returned
int EDT_DEV_LogdeviceProperty(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pdevInfoData, DWORD devProperty,DWORD type, const wchar_t *devPropLabel,  PUCHAR* ppBuf = NULL );
BOOL EDT_DEV_DeviceIsSmartcardRelated(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pdevInfoData);
int EDT_DEV_LogdeviceCapabilities(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pdevInfoData);

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


int EDT_StartDevicesLog()
{
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_DEVICESLOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"gathering devices info...\r\n");

	int iRetVal = EDT_DEV_LogAlldevices();

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_DEVICESLOG_STOP>\n");
	LOG(EDT_LINE_BREAK);

	return iRetVal;
}

//Device_LIST *deviceList, const wchar_t *deviceDescription, const wchar_t *deviceClass
int EDT_DEV_LogAlldevices()
{
	int iRetVal = EDT_OK;

	HDEVINFO lhDevInfo = INVALID_HANDLE_VALUE;
	lhDevInfo = SetupDiGetClassDevs(NULL,NULL,0,DIGCF_ALLCLASSES);// | DIGCF_PRESENT);
	if ( lhDevInfo == INVALID_HANDLE_VALUE )
	{
		LOG_LASTERROR(L"EDT_DEV_LogAlldevices failed");
		return EDT_ERR_INTERNAL;
	}

	DWORD ldwIndex = 0;
	SP_DEVINFO_DATA lDevInfoData;
	ZeroMemory(&lDevInfoData, sizeof(SP_DEVINFO_DATA));
	lDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	DWORD progressMax = 0;

	while(true) 
	{
		if(!SetupDiEnumDeviceInfo(lhDevInfo, ldwIndex++, &lDevInfoData))
		{
			if ( GetLastError() != ERROR_NO_MORE_ITEMS) 
			{
				LOG_LASTERROR(L"SetupDiEnumDeviceInfo failed");
				iRetVal = EDT_ERR_INTERNAL;
			}
			break;
		}
		else 
		{
			EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_DEVICEDESC,REG_SZ, L"Name");
			LogIncIndent();		
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,REG_SZ, L"id");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_CLASS,REG_SZ,L"ClassType");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_CLASSGUID,REG_SZ,L"ClassGUID");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_FRIENDLYNAME,REG_SZ,L"FriendlyName");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_ENUMERATOR_NAME,REG_SZ,L"Enumerator");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_DRIVER,REG_SZ,L"DriverInfo");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_HARDWAREID,REG_MULTI_SZ,L"HardWareID");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_MFG,REG_SZ,L"Mfg");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_LOCATION_INFORMATION,REG_SZ,L"Location");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_SERVICE,REG_SZ,L"Service");

			if(EDT_DEV_DeviceIsSmartcardRelated(lhDevInfo, &lDevInfoData)==TRUE)
			{
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_DEVTYPE,REG_DWORD,L"Device Type");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_ADDRESS,REG_SZ,L"Address");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_BUSNUMBER,REG_DWORD,L"Bus nr.");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_BUSTYPEGUID,REG_SZ,L"Bus Type's GUID");
				EDT_DEV_LogdeviceCapabilities(lhDevInfo, &lDevInfoData);							
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_CHARACTERISTICS,REG_DWORD,L"Device Characteristics");//see Wdm.h and Ntddk.h
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_CLASS,REG_SZ,L"Device Class");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_CLASSGUID,REG_SZ,L"Device Setup Class' GUID");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_COMPATIBLEIDS,REG_MULTI_SZ,L"list of compatible IDs");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_CONFIGFLAGS,REG_DWORD,L"configuration flags");//see Regstr.h
				//iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_DEVICE_POWER_DATA,CM_POWER_DATA,L"power management information");				
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_ENUMERATOR_NAME,REG_SZ,L"Device Enumerator");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_EXCLUSIVE,REG_DWORD,L"exclusive use is allowed (1:yes / 0:no)");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_INSTALL_STATE,REG_DWORD,L"Device installation state");
				//iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_LEGACYBUSTYPE,INTERFACE_TYPE,L"Device Type");//see Wdm.h and Ntddk.h
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_LOCATION_INFORMATION,REG_SZ,L"Device hardware location");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_LOCATION_PATHS,REG_MULTI_SZ,L"Device location in the device tree");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_LOWERFILTERS,REG_MULTI_SZ,L"Device's lower-filter drivers");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_REMOVAL_POLICY,REG_DWORD,L"Device's current removal policy");//see  Cfgmgr32.h
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_REMOVAL_POLICY_HW_DEFAULT,REG_DWORD,L"Device's hardware-specified default removal policy");//see Cfgmgr32.h
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_REMOVAL_POLICY_OVERRIDE,REG_DWORD,L"Device's override removal policy");//see Cfgmgr32.h
				//iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_SECURITY,SECURITY_DESCRIPTOR,L"Device Type");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_SECURITY_SDS,REG_SZ,L"Device's security descriptor");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_SERVICE,REG_SZ,L"Device's service name");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_UI_NUMBER,REG_DWORD,L"Device's UINumber");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_UI_NUMBER_DESC_FORMAT,REG_SZ,L"Device's UINumber value");
				iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_UPPERFILTERS,REG_MULTI_SZ,L"Device's upper filter drivers");
			}
			LogDecIndent();		
		}
	}

	if(!SetupDiDestroyDeviceInfoList(lhDevInfo))
	{
		LOG_LASTERROR(L"SetupDiDestroyDeviceInfoList failed");
	}

	return iRetVal;
}

int EDT_DEV_LogdeviceProperty(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pdevInfoData, DWORD devProperty, DWORD type, const wchar_t *devPropLabel, PUCHAR* ppBuf ) 
{
 	int iRetVal = EDT_OK;

	DWORD err;
	DWORD dwBytesNeeded = 0;
	DWORD dwBufSize = 0;
	PUCHAR pBuf = NULL;

	if(SetupDiGetDeviceRegistryProperty(hDevInfo, pdevInfoData, devProperty, NULL, NULL, 0, &dwBytesNeeded) == FALSE)
	{
		err=GetLastError();
		switch(err)
		{
		case ERROR_INVALID_DATA:
			LOG(L"--%s--not found\n",devPropLabel);	
			iRetVal = EDT_ERR_INTERNAL;
			break;
		case ERROR_INSUFFICIENT_BUFFER:	
			if ( dwBytesNeeded < 1 ) 
			{
				LOG(L"%s: ERROR_INSUFFICIENT_BUFFER",devPropLabel);
				iRetVal = EDT_ERR_INTERNAL;
			}
			break;
		case ERROR_NO_SUCH_DEVINST:
			LOG(L"--%s--entry not found (ERROR_NO_SUCH_DEVINST)\n",devPropLabel);
			iRetVal = EDT_ERR_INTERNAL;
			break;
		default:
			LOG_LASTERROR(L"SetupDiGetDeviceRegistryProperty failed to get size needed");
			iRetVal = EDT_ERR_INTERNAL;
		}
	}
	else if ( dwBytesNeeded < 1 ) 
	{
		LOG(L"%s:  dwBytesNeeded < 1",devPropLabel);
		iRetVal = EDT_ERR_INTERNAL;
	}

	if(iRetVal == EDT_OK) 
	{
  		dwBufSize = dwBytesNeeded; 
		pBuf = (PUCHAR) malloc(dwBufSize);
		if(pBuf == NULL) 
		{
			LOG_ERROR(L"Memory allocation failed");
			iRetVal = EDT_ERR_INTERNAL;
		}
		else 
		{
			if(!SetupDiGetDeviceRegistryProperty(hDevInfo, pdevInfoData, devProperty, NULL, pBuf, dwBufSize, &dwBufSize))
			{
				err = GetLastError();
				if ( err != ERROR_INVALID_DATA)
				{
					LOG_LASTERROR(L"SetupDiGetDeviceRegistryProperty failed");
					iRetVal = EDT_ERR_INTERNAL;
				}
				else
				{
					LOG(L"SetupDiGetDeviceRegistryProperty error (ERROR_INVALID_DATA) %d trying to get %s: \n",err,devPropLabel);
				}
			}
			else
			{
				wchar_t * multiStringPart = (wchar_t *)pBuf;
				switch(type)
				{
				case REG_SZ:
					LOG(L"%s: %s\n",devPropLabel,(wchar_t *)pBuf);
					break;
				case REG_MULTI_SZ:
					while( *multiStringPart != '\0')
					{					
						LOG(L"%s: %s\n",devPropLabel,multiStringPart);
						multiStringPart += (wcslen(multiStringPart)+1);
					}
					break;
				case REG_DWORD:
					LOG(L"%s: %ld\n",devPropLabel,*(DWORD*)pBuf);
					break;
				default:
					LOG(L"%s: \n",devPropLabel);
					LOG_ERROR(L"Unmanaged data type");
				}				
			}
			//If a pointer to the buffer is requested, don't free the buffer yet, it will be freed by the caller
			if( ppBuf == NULL)
			{
				free(pBuf);
			}
		}
	}
	if( ppBuf != NULL)
	{
		*ppBuf = pBuf;
	}
	return iRetVal;
}

BOOL EDT_DEV_DeviceIsSmartcardRelated(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pdevInfoData)
{
	BOOL bIsSmartcardRelated = FALSE;
	int iRetVal = EDT_OK;
	PUCHAR pBuf = NULL;
	wchar_t ucTempBuf[10];

	iRetVal = EDT_DEV_LogdeviceProperty(hDevInfo, pdevInfoData, SPDRP_CLASS,REG_SZ,L"ClassType",&pBuf );
	if(iRetVal == EDT_OK)
	{
		wmemcpy_s(ucTempBuf,9,(wchar_t *)pBuf,9);
		ucTempBuf[9]='\0';
		if(_wcsicmp(L"smartcard",ucTempBuf) == 0)
		{
			bIsSmartcardRelated = TRUE;
		}
	}

	if(pBuf != NULL)
	{
		free(pBuf);
	}
	return bIsSmartcardRelated;
}

int EDT_DEV_LogdeviceCapabilities(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pdevInfoData)
{
	BOOL bIsSmartcardRelated = FALSE;
	int iRetVal = EDT_OK;
	PUCHAR pBuf = NULL;
	DWORD dwCapabilities;

	iRetVal = EDT_DEV_LogdeviceProperty(hDevInfo, pdevInfoData, SPDRP_CAPABILITIES,REG_DWORD,L"Capabilities",&pBuf );
	if(iRetVal == EDT_OK)
	{
		LogIncIndent();
		dwCapabilities = *((DWORD*)pBuf);
		if(dwCapabilities & CM_DEVCAP_LOCKSUPPORTED)
			LOG(L"CM_DEVCAP_LOCKSUPPORTED\n");
		if(dwCapabilities & CM_DEVCAP_EJECTSUPPORTED)
			LOG(L"CM_DEVCAP_EJECTSUPPORTED\n");
		if(dwCapabilities & CM_DEVCAP_REMOVABLE)
			LOG(L"CM_DEVCAP_REMOVABLE\n");
		if(dwCapabilities & CM_DEVCAP_DOCKDEVICE)
			LOG(L"CM_DEVCAP_DOCKDEVICE\n");
		if(dwCapabilities & CM_DEVCAP_UNIQUEID)
			LOG(L"CM_DEVCAP_UNIQUEID\n");
		if(dwCapabilities & CM_DEVCAP_SILENTINSTALL)
			LOG(L"CM_DEVCAP_SILENTINSTALL\n");
		if(dwCapabilities & CM_DEVCAP_RAWDEVICEOK)
			LOG(L"CM_DEVCAP_RAWDEVICEOK\n");
		if(dwCapabilities & CM_DEVCAP_SURPRISEREMOVALOK)
			LOG(L"CM_DEVCAP_SURPRISEREMOVALOK\n");
		if(dwCapabilities & CM_DEVCAP_HARDWAREDISABLED)
			LOG(L"CM_DEVCAP_HARDWAREDISABLED\n");
		if(dwCapabilities & CM_DEVCAP_NONDYNAMIC)
			LOG(L"CM_DEVCAP_NONDYNAMIC\n");
		LogDecIndent();
	}

	if(pBuf != NULL)
	{
		free(pBuf);
	}
	return iRetVal;
}
