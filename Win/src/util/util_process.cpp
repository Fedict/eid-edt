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
#include "edt.h"//#include <windows.h>
#include "log.h"
#include <aclapi.h>
#include "Tlhelp32.h" 
#include <psapi.h>
#include "util_process.h"

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_LogModules(DWORD dwPID);

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_process_logList(processPresenceMap *pmapProcessPresence)
{
	int iReturnCode = EDT_OK;

	LOG_ENTER();

	HANDLE hndl;
	if(INVALID_HANDLE_VALUE == ( hndl=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0)))
	{
		LOG_EXIT_ERROR(L"CreateToolhelp32Snapshot failed");
		return EDT_ERR_INTERNAL;
	}

	PROCESSENTRY32  procEntry={0};
	procEntry.dwSize = sizeof( PROCESSENTRY32 );

	if(!Process32First(hndl,&procEntry))
	{
		LOG_LASTERROR(L"Process32First failed");
		if(!CloseHandle( hndl ))
		{
			LOG_LASTERROR(L"CloseHandle failed");
		}
		LOG_EXIT(EDT_ERR_INTERNAL);
		return EDT_ERR_INTERNAL;
	}

	if(pmapProcessPresence != NULL)
	{
		for(processPresenceMap::iterator i=pmapProcessPresence->begin();i!=pmapProcessPresence->end();i++)
			i->second = false;
	}

	do 
	{
		if(procEntry.th32ProcessID != 0)
		{
			if(pmapProcessPresence != NULL)
			{
				for(processPresenceMap::iterator i=pmapProcessPresence->begin();i!=pmapProcessPresence->end();i++)
				{
					if( _wcsnicmp(i->first,procEntry.szExeFile, wcslen(i->first)) == 0)
					{
						i->second = true;
					}
				}
			}
			else
			{
				LOG(L"ProcessID = %d\n",procEntry.th32ProcessID);
				LOG(L"Process name = %s\n",procEntry.szExeFile);
				EDT_LogModules(procEntry.th32ProcessID);
			}
		}
	} while(Process32Next(hndl,&procEntry));


	if(!CloseHandle( hndl ))
	{
		LOG_LASTERROR(L"CloseHandle failed");
	}
	LOG_EXIT(iReturnCode);
	return iReturnCode;
} 
void EDT_process_logProcessPresenceMap(processPresenceMap *pmapProcessPresence)
{
	for(processPresenceMap::iterator i=pmapProcessPresence->begin();i!=pmapProcessPresence->end();i++)
	{		
		if(i->second == true)
		{
			LOG(L"%s found\n",i->first);
		}
		else
		{
			LOG(L"--%s-- not found\n",i->first);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_LogModules(DWORD dwPID)
{
	int iReturnCode = EDT_OK;

	LOG_ENTER();
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

	if(INVALID_HANDLE_VALUE == ( hModuleSnap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID)))
	{
		LOG_LASTERROR(L"CreateToolhelp32Snapshot failed");
		LOG_EXIT(EDT_ERR_INTERNAL);
		return EDT_ERR_INTERNAL;
	}

	me32.dwSize = sizeof(MODULEENTRY32);

    if (!Module32First (hModuleSnap, &me32))
    {
 		LOG_LASTERROR(L"Module32First failed");
		if(!CloseHandle( hModuleSnap ))
		{
			LOG_LASTERROR(L"CloseHandle failed");
		}
		LOG_EXIT(EDT_ERR_INTERNAL);
		return EDT_ERR_INTERNAL;
    }

    do
    {
		LOG(L"Pid %d Module %s%s\n", dwPID,me32.szExePath,me32.szModule);
    } while (Module32Next(hModuleSnap, &me32));

	if(!CloseHandle( hModuleSnap ))
	{
		LOG_LASTERROR(L"CloseHandle failed");
	}
	LOG_EXIT(iReturnCode);
    return iReturnCode;
}
