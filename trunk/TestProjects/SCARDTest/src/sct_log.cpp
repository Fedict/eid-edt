/* ****************************************************************************

 * SCTest Project.
 * Copyright (C) 2013 FedICT.
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
#include <string.h>
#include <iostream>
#include <time.h>
#include <errno.h>

#include "sct_log.h"
#include <shlobj.h>
#include <stdio.h>

#define LOGFILE "sctest_log.txt"


#define EDT_FOLDERPATH_LEN 512
static FILE *g_pfile=NULL;
static bool g_logAvailable=true;
static int g_indent=0;
static char g_folderpath[EDT_FOLDERPATH_LEN];

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void GetLocalTime(std::string &timestamp);
void LogWrite(bool time,bool indent, const char *format, va_list argList);

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
void LogInitialize()
{
	errno_t err = 0;
	const DWORD bufferSize = EDT_FOLDERPATH_LEN;
	BOOL bIsWow64 = FALSE;

	if(g_pfile != NULL)  {
			return;
	}

	//create file path
	if(!SHGetSpecialFolderPath(NULL,g_folderpath,CSIDL_MYDOCUMENTS,FALSE))
	{
		LogTrace("My Documents folder not found");
		return;
	}
	else
	{
		if(0 != strcat_s(g_folderpath,bufferSize,"\\")) //Ending by slash
		{			
			LogTrace("g_folderpath is too long.");
			return;
		}
		if(0 != strcat_s(g_folderpath,bufferSize,LOGFILE))
		{			
			LogTrace("g_folderpath is too long");
			return;
		}
	}

	
	for(int i=0;i<LOG_OPEN_ATTEMPT_COUNT;i++)
	{
		//Create the file (or erase previous file)
		err = fopen_s(&g_pfile, g_folderpath, "w");//, ccs=UNICODE");//UTF-8");//UNICODE
		//g_pfile = _wfsopen(g_folderpath, L"w+",_SH_DENYWR);
		if (g_pfile)
		{
			break;
		}
		else if( err == ENOENT ) //Folder does not exist
		{
			g_logAvailable = false;
			return;
		}
		else
		{
			Sleep(50);
		}
	}
	
	if(g_pfile)
	{
		fprintf_s(g_pfile,"SCARDTEST VERSION: %d.%d\n",SCT_VERSION_MAJOR,SCT_VERSION_MINOR);
	}
	else
	{
		LogTrace("Could not open the logfile at %s \r\n",g_folderpath);
		return;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
void LogFinalize()
{
	if(g_pfile)
	{
		fclose(g_pfile);
		g_pfile=NULL;
		LogTrace("logfile closed \r\n");
		LogTrace("LogFile stored at %s \r\n",g_folderpath);
	}
	else
	{
		LogTrace("no log file specified, couldn't close logfile\r\n");
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void LogApduSend(BYTE *apdu, DWORD apdulen)
{
	Log("SEND APDU: ");
	LogByteArray(apdu, apdulen);
}

void LogApduResp(BYTE *apdu, DWORD apdulen)
{
	Log("RESP APDU: ");
	LogByteArray(apdu, apdulen);
}

void LogByteArray(BYTE *byteArray, DWORD arrayLen)
{
	DWORD counter = 0;
	while (counter < arrayLen)
	{
		Log("0x%.2x ", *byteArray);
		counter++;
		byteArray++;
	}
	Log("\n");
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
// The default log : no time displayed and no indent used
////////////////////////////////////////////////////////////////////////////////////////////////
void Log(const char *format, ...)
{
	if(!g_logAvailable)
		return;

	va_list args;
	va_start(args, format);
	LogWrite(false,false,format, args);
	va_end(args);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// The time log : time displayed and indent used
////////////////////////////////////////////////////////////////////////////////////////////////
void LogTime(const char *format, ...)
{
	if(!g_logAvailable)
		return;

	va_list args;
	va_start(args, format);
	LogWrite(true,true,format, args);
	va_end(args);
}

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void getLocalTime(std::string &timestamp)
{
	time_t rawtime;
	struct tm timeinfo;
	char buffer [20];

	time ( &rawtime );
#ifdef WIN32
	localtime_s( &timeinfo, &rawtime );
#else
	timeinfo = *(localtime(&rawtime));
#endif

	strftime (buffer,20,"%Y-%m-%d %H:%M:%S",&timeinfo);

	timestamp.assign(buffer);
}
////////////////////////////////////////////////////////////////////////////////////////////////
void LogWrite(bool time, bool indent, const char *format, va_list argList)
{	
	if(!g_logAvailable)
		return;
	
	if(format==NULL)
		return;
		   
	if(!g_pfile) 
		return;
	
	if( indent && (g_indent > 0) )
	{
		for(int i = 0; i < g_indent;i++)
		{
			fprintf_s(g_pfile,"  ");
			LogTrace("  ");
		}
	}

	if(time)
	{
		std::string timestamp;
		getLocalTime(timestamp);
		fprintf_s(g_pfile,"%ls - ",timestamp.c_str());
		LogTrace("%ls - ",timestamp.c_str());
	}
	vfprintf_s(g_pfile, format, argList);
	vprintf(format, argList);
}
