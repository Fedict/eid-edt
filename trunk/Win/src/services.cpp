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
#include "EDT.h"
#include "log.h"
#include "Winsvc.h"

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int serviceGetInfoFromManagerHandle(SC_HANDLE schManager, LPCTSTR lpServiceName);
const wchar_t *serviceGetStatusLabel(int Status);
int serviceGetStatusInfoFromServiceHandle(SC_HANDLE schService);
int serviceGetConfigInfoFromServiceHandle(SC_HANDLE schService);

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_StartServicesLog ()
{
	int iReturnCode = EDT_OK;

	SC_HANDLE      schManager=0;
	SC_HANDLE      schService=0;

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_SERVICESLOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"logging services' info...\r\n");

	if( (schManager = OpenSCManager( NULL, SERVICES_ACTIVE_DATABASE,GENERIC_READ )) == NULL ) 
	{
		LOG_LASTERROR(L"OpenSCManager failed");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	else
	{
		DWORD dwBytesNeeded = 0;
		DWORD dwBufSize = 0;
		DWORD dwNumberOfService = 0;
		PUCHAR pBuf = NULL;
		ENUM_SERVICE_STATUS_PROCESS *pStatus = NULL;

		//First call to get the size of the buffer
		if(EnumServicesStatusEx( schManager,						// __in        SC_HANDLE hSCManager,
							  SC_ENUM_PROCESS_INFO,				// __in        SC_ENUM_TYPE InfoLevel,
							  SERVICE_DRIVER | SERVICE_WIN32,	// __in        DWORD dwServiceType,
							  SERVICE_STATE_ALL,				// __in        DWORD dwServiceState,
							  NULL,							    //__out_opt    LPBYTE lpServices,
							  0,								//__in         DWORD cbBufSize,
							  &dwBytesNeeded,					//__out        LPDWORD pcbBytesNeeded,
							  &dwNumberOfService,				//__out        LPDWORD lpServicesReturned,
							  NULL,								//__inout_opt  LPDWORD lpResumeHandle,
							  NULL								//__in_opt     LPCTSTR pszGroupName
							  ) != 0)
		{
			LOG_LASTERROR(L"EnumServicesStatusEx failed to get size needed");
			iReturnCode = EDT_ERR_INTERNAL;
		}
		else
		{
			if( ERROR_MORE_DATA != GetLastError() || dwBytesNeeded < 1)
			{
				LOG_LASTERROR(L"EnumServicesStatusEx failed to get size needed");
				iReturnCode = EDT_ERR_INTERNAL;
			}
		}

		if(iReturnCode == EDT_OK) 
		{
			//Allocate the buffer
			dwBufSize = dwBytesNeeded + 0x10; 
			if(NULL == (pBuf  = (PUCHAR) malloc(dwBufSize)))
			{
				LOG_ERROR(L"Memory allocation failed");
				iReturnCode = EDT_ERR_INTERNAL;
			}
			else if(EnumServicesStatusEx( schManager,				// __in        SC_HANDLE hSCManager,
				SC_ENUM_PROCESS_INFO,				// __in        SC_ENUM_TYPE InfoLevel,
				SERVICE_DRIVER | SERVICE_WIN32,	// __in        DWORD dwServiceType,
				SERVICE_STATE_ALL,				// __in        DWORD dwServiceState,
				pBuf,							    //__out_opt    LPBYTE lpServices,
				dwBufSize,						//__in         DWORD cbBufSize,
				&dwBytesNeeded,					//__out        LPDWORD pcbBytesNeeded,
				&dwNumberOfService,				//__out        LPDWORD lpServicesReturned,
				NULL,								//__inout_opt  LPDWORD lpResumeHandle,
				NULL								//__in_opt     LPCTSTR pszGroupName
				) == 0)
			{
				LOG_LASTERROR(L"EnumServicesStatusEx failed");
				iReturnCode = EDT_ERR_INTERNAL;
			}
			else
			{
				pStatus = (LPENUM_SERVICE_STATUS_PROCESS)pBuf; 
				for (ULONG i=0;i<dwNumberOfService;i++) 
				{  
					LOG(L"ServiceName = %s\n",pStatus[i].lpServiceName);
					LogIncIndent();

					if(pStatus[i].lpDisplayName != NULL)
					{
						LOG(L"Service Displayname = %s\n",pStatus[i].lpDisplayName);
					}
					else
					{
						LOG(L"No Service Displayname\n");
					}

					serviceGetInfoFromManagerHandle(schManager, pStatus[i].lpServiceName);
					LogDecIndent();
				}
			}
			if(pBuf) free(pBuf);
		}
	}
	if(schManager) CloseServiceHandle(schManager);

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_PROCESSLOG_STOP>\n");
	LOG(EDT_LINE_BREAK);

	return iReturnCode;
} 
////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

const wchar_t *serviceGetStatusLabel(int Status)
{
	switch(Status)
	{
	case SERVICE_CONTINUE_PENDING:				//5
		return L"SERVICE_CONTINUE_PENDING";
	case SERVICE_PAUSE_PENDING:					//6
		return L"SERVICE_PAUSE_PENDING";
	case SERVICE_PAUSED:						//7
		return L"SERVICE_PAUSED";
	case SERVICE_RUNNING:						//4
		return L"SERVICE_RUNNING";
	case SERVICE_START_PENDING:					//2
		return L"SERVICE_START_PENDING";
	case SERVICE_STOP_PENDING:					//3
		return L"SERVICE_STOP_PENDING";
	case SERVICE_STOPPED:						//1
		return L"SERVICE_STOPPED";
	}

	return L"UNKNOWN_STATUS";
}

////////////////////////////////////////////////////////////////////////////////////////////////
int serviceGetInfoFromManagerHandle(SC_HANDLE schManager, LPCTSTR lpServiceName)
{
	int iReturnCode = EDT_OK;

	if(schManager==0 )
	{
		LOG_ERROR(L"serviceGetInfoFromManagerHandle failed, schManager==0 \n")
		LOG_EXIT_ERROR(L"EDT_ERR_BAD_PARAM");
		return EDT_ERR_BAD_PARAM;
	}

	SC_HANDLE schService=0;

	if( (schService = OpenService( schManager, lpServiceName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG )) == NULL ) 
	{
		switch( GetLastError() ) 
		{
		case ERROR_ACCESS_DENIED:
		case ERROR_INVALID_HANDLE:
			LOG_LASTERROR(L"OpenService failed");
			iReturnCode = EDT_ERR_INTERNAL;
			break;
		default:
			LOG(L"NOT FOUND\n");
			break;
		}
	}
	else 
	{
		iReturnCode = serviceGetStatusInfoFromServiceHandle(schService);
		if(iReturnCode == EDT_OK)
			iReturnCode = serviceGetConfigInfoFromServiceHandle(schService);
		else
			serviceGetConfigInfoFromServiceHandle(schService);
	}

    if(schService) CloseServiceHandle(schService); 

	return iReturnCode;
} 

////////////////////////////////////////////////////////////////////////////////////////////////
int serviceGetStatusInfoFromServiceHandle(SC_HANDLE schService)
{
	int iReturnCode = EDT_OK;

	if(schService==0)
	{
		return EDT_ERR_BAD_PARAM;
	}

	DWORD dwBytesNeeded = 0;
	DWORD dwBufSize = 0;
	PUCHAR pBuf = NULL;

	//QueryServiceStatusEx
	SERVICE_STATUS_PROCESS *pStatus = NULL;
	//First call to get the size of the buffer
	if( QueryServiceStatusEx(schService,				//__in       SC_HANDLE hService,
							      SC_STATUS_PROCESS_INFO,	//__in       SC_STATUS_TYPE InfoLevel,
							      NULL,						//__out_opt  LPBYTE lpBuffer,
							      0,						//__in       DWORD cbBufSize,
							      &dwBytesNeeded			//__out      LPDWORD pcbBytesNeeded
							      ) != 0 )
	{
		LOG_LASTERROR(L"QueryServiceStatusEx failed to get size needed");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	else
	{
		if( ERROR_INSUFFICIENT_BUFFER != GetLastError() || dwBytesNeeded < 1)
		{
			LOG_LASTERROR(L"QueryServiceStatusEx failed to get size needed");
			iReturnCode = EDT_ERR_INTERNAL;
		}
	}

	if(iReturnCode == EDT_OK) 
	{
		//Allocate the buffer
		dwBufSize = dwBytesNeeded + 0x10; 
		if((pBuf  = (PUCHAR) malloc(dwBufSize)) == NULL)
		{
			LOG_ERROR(L"Memory allocation failed");
			iReturnCode = EDT_ERR_INTERNAL;
		}
		else if( QueryServiceStatusEx(	schService,				//__in       SC_HANDLE hService,
											SC_STATUS_PROCESS_INFO,	//__in       SC_STATUS_TYPE InfoLevel,
											pBuf,					//__out_opt  LPBYTE lpBuffer,
											dwBufSize,				//__in       DWORD cbBufSize,
											&dwBytesNeeded			//__out      LPDWORD pcbBytesNeeded
											) == 0)
		{
			switch( GetLastError() ) 
			{
			case NO_ERROR:
			case ERROR_SERVICE_NOT_ACTIVE:
			case ERROR_SHUTDOWN_IN_PROGRESS:
				break;
			default:
				LOG_LASTERROR(L"QueryServiceStatusEx failed");
				iReturnCode = EDT_ERR_INTERNAL;
				break;
			}
		}
		else
		{
			pStatus = (SERVICE_STATUS_PROCESS*)pBuf; 
			LOG(L"status = %s\n",serviceGetStatusLabel(pStatus->dwCurrentState));
			LOG(L"processId = %d\n",pStatus->dwProcessId);
		}
		if(pBuf) free(pBuf);
	}

	return iReturnCode;
}

int serviceGetConfigInfoFromServiceHandle(SC_HANDLE schService)
{
	int iReturnCode = EDT_OK;

	if(schService==0)
	{
		return EDT_ERR_BAD_PARAM;
	}

	DWORD dwBytesNeeded = 0;
	DWORD dwBufSize = 0;
	LPQUERY_SERVICE_CONFIG pBuf = NULL;

	//QueryServiceStatusEx
	//First call to get the size of the buffer
	if( QueryServiceConfig(schService,NULL,0,&dwBytesNeeded) != 0 )
	{
		LOG_LASTERROR(L"QueryServiceConfig failed to get size needed");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	else
	{
		if( ERROR_INSUFFICIENT_BUFFER != GetLastError() || dwBytesNeeded < 1)
		{
			LOG_LASTERROR(L"QueryServiceConfig failed to get size needed");
			iReturnCode = EDT_ERR_INTERNAL;
		}
	}

	if(iReturnCode == EDT_OK) 
	{
		//Allocate the buffer
		dwBufSize = dwBytesNeeded + 0x10; 
		if((pBuf  = (LPQUERY_SERVICE_CONFIG) malloc(dwBufSize)) == NULL)
		{
			LOG_ERROR(L"Memory allocation failed");
			iReturnCode = EDT_ERR_INTERNAL;
		}
		else 
		{
			if(QueryServiceConfig(schService, pBuf,dwBufSize,&dwBytesNeeded) == 0)
			{
				switch( GetLastError() ) 
				{
				case NO_ERROR:
				case ERROR_SERVICE_NOT_ACTIVE:
				case ERROR_SHUTDOWN_IN_PROGRESS:
					break;
				default:
					LOG_LASTERROR(L"QueryServiceConfig failed");
					iReturnCode = EDT_ERR_INTERNAL;
					break;
				}
			}
			else
			{
				LOG(L"Log On As %s\n",pBuf->lpServiceStartName);
			}
			if(pBuf) free(pBuf);
		}
	}

	return iReturnCode;
}