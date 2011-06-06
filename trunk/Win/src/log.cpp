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

#include <string.h>
#include <iostream>
#include <time.h>
#include <errno.h>


//#include "error.h"
#include "log.h"
#include <shlobj.h>
#include <stdio.h>

//#define LOGFILE64 L"EDTlog64.txt"
//#ifdef WIN64
//#define LOGFILE LOGFILE64
//#else
#define LOGFILE L"EDTlog.txt"
//#endif

#define EDT_FOLDERPATH_LEN 512
static FILE *g_pfile=NULL;
static bool g_logAvailable=true;
static HWND g_htextWnd=NULL;
static int g_indent=0;
static TCHAR g_folderpath[EDT_FOLDERPATH_LEN];

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void getLocalTime(std::wstring &timestamp);
int logWrite(bool time,bool indent, const wchar_t *format, va_list argList);
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
int logInitialize(HWND htextWnd)
{
	int iReturnCode = EDT_OK;
	errno_t err = 0;
	const DWORD bufferSize = EDT_FOLDERPATH_LEN;
	BOOL bIsWow64 = FALSE;
	//TCHAR g_folderpath[bufferSize];

	if(g_pfile)  return EDT_OK;

	g_htextWnd = htextWnd;
	//create file path
	if(!SHGetSpecialFolderPath(NULL,g_folderpath,CSIDL_MYDOCUMENTS,FALSE))
	{
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"My Documents folder not found");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	else
	{
		if(0 != wcscat_s(g_folderpath,bufferSize,L"\\")) //Ending by slash
		{			
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"g_folderpath is too long. g_folderpath = ");
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)g_folderpath);
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\n");
			iReturnCode = EDT_ERR_INTERNAL;
		}
		if(0 != wcscat_s(g_folderpath,bufferSize,LOGFILE))
		{			
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"g_folderpath too long. g_folderpath = ");
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)g_folderpath);
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\n");
			iReturnCode = EDT_ERR_INTERNAL;
		}
	}
	if(iReturnCode != EDT_OK)
	{
		return iReturnCode;
	}

	//if we are a 32 bit application, running on a 64 bit OS,
	//we append our log to the one of our 64 bit counterpart
#ifndef WIN64
	LPFN_ISWOW64PROCESS fnIsWow64Process;
    

	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            LOG_LASTERROR(L"fnIsWow64Process failed");
			bIsWow64 = TRUE;//Could not determine architecture, but append, just in case
        }
    }
#endif

	
	for(int i=0;i<LOG_OPEN_ATTEMPT_COUNT;i++)
	{
		if(bIsWow64 == TRUE)
		{
			//Create the file (or append previous file)
			err = _wfopen_s(&g_pfile, g_folderpath, L"a+, ccs=UTF-8");//UNICODE
		}
		else
		{
			//Create the file (or erase previous file)
			err = _wfopen_s(&g_pfile, g_folderpath, L"w, ccs=UTF-8");//UNICODE
		}
		//g_pfile = _wfsopen(g_folderpath, L"w+",_SH_DENYWR);

		if (g_pfile)
		{
			break;
		}
		else if( err == ENOENT ) //Folder does not exist
		{
			g_logAvailable = false;
			return EDT_ERR_NOT_AVAILABLE;
		}
		else
		{
			Sleep(50);
		}
	}
	
#ifdef WIN64
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"64 bit EDT TOOL VERSION: ");
#else
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"32 bit EDT TOOL VERSION: ");
#endif
	WCHAR TempBuffer[100];
	wsprintf(TempBuffer,L"%d.%d\r\n",EDT_VERSION_MAJOR,EDT_VERSION_MINOR);
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)TempBuffer);

	if(g_pfile)
	{
		fwprintf_s(g_pfile,L"EDT TOOL VERSION: %d.%d\n",EDT_VERSION_MAJOR,EDT_VERSION_MINOR);
	}
	else
	{
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"Could not open the logfile at ");
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)g_folderpath);
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\n");
		return EDT_ERR_FILE_CREATE_FAILED;
	}

	return iReturnCode;
}
////////////////////////////////////////////////////////////////////////////////////////////////
int logFinalize(HWND htextWnd)
{
	if(g_pfile)
	{
		fclose(g_pfile);
		g_pfile=NULL;
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"logfile closed \r\n");
	}
	else
	{
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"no log file specified, couldn't close logfile\r\n");
	}
	return EDT_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////
int logFileToScreen(HWND htextWnd)
{
	if(htextWnd == NULL)
	{
		return EDT_ERR_BAD_PARAM;
	}
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"printing logfile to screen..\r\n");
	if(g_pfile == NULL)
	{
		for(int i=0;i<LOG_OPEN_ATTEMPT_COUNT;i++)
		{
			_wfopen_s(&g_pfile, g_folderpath, L"rb, ccs=UTF-8");
			if (g_pfile)
			{
				SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"logfile opened..\r\n");
				break;
			}
			else
			{
				if (i == (LOG_OPEN_ATTEMPT_COUNT -1))
				{
					return EDT_ERR_FILE_NOT_FOUND;
				}
				Sleep(500);
			}
		}
	}
	if ( fseek(g_pfile, 0L,SEEK_SET) != 0 )
	{
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"couldn't rewind logfile\r\n");
		return EDT_ERR_INTERNAL;
	};

	wchar_t buffer[100001];
	//size_t lineLen = 0;
	size_t numRead = 0;
	
	//while (fgetws( buffer, 1022, g_pfile) != NULL)
	while( (numRead = fread( buffer, sizeof( wchar_t ), 100000, g_pfile)) > 0  )
	{
		//lineLen = wcslen(buffer);//fgetws sets wchar #1022 to null, so lineLen < 1022
		//buffer[numread] = '\r';
		//buffer[numread+1] = '\n';
		buffer[numRead] = '\0';
		SendMessageA(htextWnd, EM_REPLACESEL,0,  (LPARAM)buffer);
		//SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\n");
	}
	if (feof(g_pfile) == 0 )
	{
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"an error occured trying to read from the logfile\r\n");
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"errno =");
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)errno);
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\nplease copy the logfile from ");
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)g_folderpath);
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\n");
	}

	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"logfile successfully printed from ");
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)g_folderpath);
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\n");

	return EDT_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void logShowLogFile(void)
{
	wchar_t thecmd[1024+EDT_FOLDERPATH_LEN];

	wcscpy_s(thecmd,L"explorer /select,");
	wcscat_s(thecmd,g_folderpath);
	_wsystem(thecmd);
}
////////////////////////////////////////////////////////////////////////////////////////////////
int logFilePathToScreen(HWND htextWnd)
{
	if(htextWnd == NULL)
	{
		return EDT_ERR_BAD_PARAM;
	}
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\nLogFile stored at ");
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)g_folderpath);
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\n");
	
	return EDT_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void LOG_APDU(BYTE *apdu, DWORD apdulen)
{
	LOG(L"APDU: ");
	LOG_BYTE_ARRAY(apdu, apdulen);
}

void LOG_BYTE_ARRAY(BYTE *byteArray, DWORD arrayLen)
{
	DWORD counter = 0;
	while (counter < arrayLen)
	{
		LOG_BASIC(L"0x%.2x ", *byteArray);
		counter++;
		byteArray++;
	}
	LOG_BASIC(L"\n");
}

void LOG_BYTE_ARRAY_AS_STRING(BYTE *byteArray, DWORD arrayLen)
{
	DWORD counter = 0;
	while (counter < arrayLen)
	{
		LOG_BASIC(L"%c", *byteArray);
		counter++;
		byteArray++;
	}
	LOG_BASIC(L"\n");
}

int LOG_SCREEN(const wchar_t *line)
{
	SendMessage(g_htextWnd, EM_REPLACESEL,0,  (LPARAM)line);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void LogIncIndent()
{
	g_indent++; 
}
////////////////////////////////////////////////////////////////////////////////////////////////
void LogDecIndent()
{
	g_indent--; 
}

////////////////////////////////////////////////////////////////////////////////////////////////
// The default log : no time displayed and indent used
////////////////////////////////////////////////////////////////////////////////////////////////
int LOG(const wchar_t *format, ...)
{
	int iReturnCode = EDT_OK;
	
	if(!g_logAvailable)
		return EDT_OK;

	va_list args;
	va_start(args, format);
	iReturnCode = logWrite(false,true,format, args);
	va_end(args);

	return iReturnCode;
}
////////////////////////////////////////////////////////////////////////////////////////////////
// The basic log : no time displayed and no indent used
////////////////////////////////////////////////////////////////////////////////////////////////
int LOG_BASIC(const wchar_t *format, ...)
{
	int iReturnCode = EDT_OK;
	
	if(!g_logAvailable)
		return EDT_OK;

	va_list args;
	va_start(args, format);
	iReturnCode = logWrite(false,false,format, args);
	va_end(args);

	return iReturnCode;
}
////////////////////////////////////////////////////////////////////////////////////////////////
// The time log : time displayed and indent used
////////////////////////////////////////////////////////////////////////////////////////////////
int LOG_TIME(const wchar_t *format, ...)
{
	int iReturnCode = EDT_OK;
	
	if(!g_logAvailable)
		return EDT_OK;

	va_list args;
	va_start(args, format);
	iReturnCode = logWrite(true,true,format, args);
	va_end(args);

	return iReturnCode;
}
////////////////////////////////////////////////////////////////////////////////////////////////
// LOG_CMD : log the output of a system command
///////////////////////////////////////////////////////////////////////////////////////////
int LOG_CMD(const wchar_t *cmd)
{
	wchar_t thecmd[1024+EDT_FOLDERPATH_LEN];

	//fflush(g_pfile);
	if (g_pfile != NULL)
	{
		return -1;
	}
	if (wcslen(cmd) > 1020 )
	{
		return -1;
	}
	wcscpy_s(thecmd,cmd);
	wcscat_s(thecmd,L" >> ");
	wcscat_s(thecmd,g_folderpath);
	_wsystem(thecmd);

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void getLocalTime(std::wstring &timestamp)
{
	time_t rawtime;
	struct tm timeinfo;
	wchar_t buffer [20];

	time ( &rawtime );
#ifdef WIN32
	localtime_s( &timeinfo, &rawtime );
#else
	timeinfo = *(localtime(&rawtime));
#endif

	wcsftime (buffer,20,L"%Y-%m-%d %H:%M:%S",&timeinfo);

	timestamp.assign(buffer);
}
////////////////////////////////////////////////////////////////////////////////////////////////
int logWrite(bool time, bool indent, const wchar_t *format, va_list argList)
{
	int iReturnCode = EDT_OK;
	
	if(!g_logAvailable)
		return EDT_OK;
	
	if(format==NULL)
		return EDT_ERR_BAD_PARAM;
		   
	if(!g_pfile) 
		return EDT_ERR_LOGFILE_NOT_CREATED;
	
	if( indent && (g_indent > 0) )
	{
		for(int i = 0; i < g_indent;i++)
		{
			fwprintf_s(g_pfile,L"  ");
		}
	}

	if(time)
	{
		std::wstring timestamp;
		getLocalTime(timestamp);
		fwprintf_s(g_pfile,L"%ls - ",timestamp.c_str());
	}
	vfwprintf_s(g_pfile, format, argList);
	
	return EDT_OK;
}
