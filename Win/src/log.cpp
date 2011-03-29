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

#define LOGFILE L"EDTlog.txt"

static FILE *g_pfile=NULL;
static bool g_logAvailable=true;
static HWND g_htextWnd=NULL;
static int g_indent=0;

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void getLocalTime(std::wstring &timestamp);
int logWrite(bool time,bool indent, const wchar_t *format, va_list argList);

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
int logInitialize(HWND htextWnd)
{
	int iReturnCode = EDT_OK;
	errno_t err = 0;
	const DWORD bufferSize = 512;
	TCHAR folderpath[bufferSize];

	if(g_pfile)  return EDT_OK;

	g_htextWnd = htextWnd;
	//create file path
	if(!SHGetSpecialFolderPath(NULL,folderpath,CSIDL_MYDOCUMENTS,FALSE))
	{
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"My Documents folder not found");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	else
	{
		if(0 != wcscat_s(folderpath,bufferSize,L"\\")) //Ending by slash
		{			
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"Folderpath is too long. folderpath = ");
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)folderpath);
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\n");
			iReturnCode = EDT_ERR_INTERNAL;
		}
		if(0 != wcscat_s(folderpath,bufferSize,LOGFILE))
		{			
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"Folderpath too long. folderpath = ");
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)folderpath);
			SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"\r\n");
			iReturnCode = EDT_ERR_INTERNAL;
		}
	}
	if(iReturnCode != EDT_OK)
	{
		return iReturnCode;
	}
	
	//Create the file (Erase previous file)
	for(int i=0;i<LOG_OPEN_ATTEMPT_COUNT;i++)
	{
		err = _wfopen_s(&g_pfile, folderpath, L"w");
		
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
	
	WCHAR TempBuffer[100];
	wsprintf(TempBuffer,L"%d.%d\r\n",EDT_VERSION_MAJOR,EDT_VERSION_MINOR);
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"EDT TOOL VERSION: ");
	SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)TempBuffer);
	if(g_pfile)
	{
		fwprintf_s(g_pfile,L"EDT TOOL VERSION: %d.%d\n",EDT_VERSION_MAJOR,EDT_VERSION_MINOR);
	}
	else
	{
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"Could not open the logfile at ");
		SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)folderpath);
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

int LOG_SCREEN(const wchar_t *line)
{
	SendMessage(g_htextWnd, EM_REPLACESEL,0,  (LPARAM)line);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void LOG_INC_INDENT()
{
	g_indent++; 
}
////////////////////////////////////////////////////////////////////////////////////////////////
void LOG_DEC_INDENT()
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
