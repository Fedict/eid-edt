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

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_DEV_LogAlldevices();
int EDT_DEV_LogdeviceProperty(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pdevInfoData, DWORD devProperty,DWORD type, const wchar_t *devPropLabel );


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
	lhDevInfo = SetupDiGetClassDevs(NULL,NULL,0,DIGCF_ALLCLASSES | DIGCF_PRESENT);
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
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,REG_SZ, L"id");
			LogIncIndent();		
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_DEVICEDESC,REG_SZ, L"Name");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_CLASS,REG_SZ,L"ClassType");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_CLASSGUID,REG_SZ,L"ClassGUID");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_FRIENDLYNAME,REG_SZ,L"FriendlyName");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_ENUMERATOR_NAME,REG_SZ,L"Enumerator");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_DRIVER,REG_SZ,L"DriverInfo");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_HARDWAREID,REG_MULTI_SZ,L"HardWareID");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_MFG,REG_SZ,L"Mfg");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_LOCATION_INFORMATION,REG_SZ,L"Location");
			iRetVal = EDT_DEV_LogdeviceProperty(lhDevInfo, &lDevInfoData, SPDRP_SERVICE,REG_SZ,L"Service");
			LogDecIndent();		
		}
	}

	if(!SetupDiDestroyDeviceInfoList(lhDevInfo))
	{
		LOG_LASTERROR(L"SetupDiDestroyDeviceInfoList failed");
	}

	return iRetVal;
}

int EDT_DEV_LogdeviceProperty(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pdevInfoData, DWORD devProperty, DWORD type, const wchar_t *devPropLabel ) 
{
 	int iRetVal = EDT_OK;

	DWORD err;
	DWORD dwBytesNeeded = 0;
	DWORD dwBufSize = 0;
	PUCHAR pBuf = NULL;

	if(SetupDiGetDeviceRegistryProperty(hDevInfo, pdevInfoData, devProperty, NULL, NULL, 0, &dwBytesNeeded) == FALSE)
	{
		err = GetLastError();
		switch(err)
		{
		case ERROR_INVALID_DATA:
			LOG(L"--%s--not found\n",devPropLabel);
			iRetVal = EDT_ERR_INTERNAL;
			break;
		case ERROR_INSUFFICIENT_BUFFER:	
			if ( dwBytesNeeded < 1 ) 
			{
				LOG(L"%s: ",devPropLabel);
				iRetVal = EDT_ERR_INTERNAL;
			}
			break;
		default:
			LOG_LASTERROR(L"SetupDiGetDeviceRegistryProperty failed to get size needed");
			iRetVal = EDT_ERR_INTERNAL;
		}
	}
	else if ( dwBytesNeeded < 1 ) 
	{
		LOG(L"%s: ",devPropLabel);
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
					LOG(L"SetupDiGetDeviceRegistryProperty error %d trying to get %s: \n",err,devPropLabel);
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
						LOG(L"%s: %ls\n",devPropLabel,multiStringPart);
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
			free(pBuf);
		}
	}

	return iRetVal;
}