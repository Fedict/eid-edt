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
#include "eidmw.h"
#include "util_registry.h"
#include <shlobj.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include "util_process.h"

#define TIME_BUF_SIZE 26

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_EIDMW_FindFile(const wchar_t *fileName, const wchar_t *folderName );
int EDT_EIDMW_LogEidmwFiles(void);


////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_StartEidmwLog()
{
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_EIDMWLOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"searching for installed middleware...\r\n");

	int iReturnCode = EDT_OK;
	int iFunctionCode = EDT_OK;

	iFunctionCode = EDT_UtilReg_LogKeyTree(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",EDTREGFLAG_EIDMW_ONLY);
	if( (iReturnCode == EDT_OK) && (iFunctionCode!=EDT_OK) )
		iReturnCode = iFunctionCode;
	
	LOG(EDT_HALF_LINE_BREAK);

	iFunctionCode = EDT_UtilReg_LogKeyTree(HKEY_CURRENT_USER,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",EDTREGFLAG_EIDMW_ONLY);
	if( (iReturnCode == EDT_OK) && (iFunctionCode!=EDT_OK) )
		iReturnCode = iFunctionCode;

	LOG(EDT_HALF_LINE_BREAK);

	iFunctionCode = EDT_EIDMW_LogEidmwFiles();
	if( (iReturnCode == EDT_OK) && (iFunctionCode!=EDT_OK) )
		iReturnCode = iFunctionCode;

	LOG(EDT_HALF_LINE_BREAK);

	processPresenceMap mapProcessPresence; 
	mapProcessPresence.insert(processPresenceItem(L"beidgui.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"beid35gui.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"beidsystemtray.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"xsign.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"beid35xsign.exe",false));

	iFunctionCode = EDT_process_logList(&mapProcessPresence);
	if( (iReturnCode == EDT_OK) && (iFunctionCode!=EDT_OK) )
		iReturnCode = iFunctionCode;

	if (iFunctionCode==EDT_OK)
	{
		LOG(L"Eidmw Processes:\n");
		LogIncIndent();
		EDT_process_logProcessPresenceMap(&mapProcessPresence);
		LogDecIndent();
	}
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_EIDMWLOG_STOP>\n");
	LOG(EDT_LINE_BREAK);

	return iReturnCode;
}
//eidmw related files

int EDT_EIDMW_FindFile(const wchar_t *fileName, const wchar_t *folderName )
{
	DWORD filePathNameLen = wcslen(fileName) + wcslen(folderName) + 2;//extra bytes: for '\' and for ending NULL
	wchar_t* filePathName;
	struct _wfinddata_t c_file;
	long hFile;
	int iReturnCode = EDT_OK;
	wchar_t timeBuf[TIME_BUF_SIZE];

	filePathName = (wchar_t*)malloc(filePathNameLen*sizeof(wchar_t));
	if(filePathName == NULL)
	{
		LOG_ERROR(L"EDT_EIDMW_FindFile malloc failed\n");
		return EDT_ERR_INTERNAL;
	}
	if(wcsncpy_s(filePathName,filePathNameLen,folderName,wcslen(folderName))!=0)
	{
		LOG_ERROR(L"EDT_EIDMW_FindFile %s wcsncpy_s failed\n");
		LOG(L"file to find was %s\n",fileName);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	else if(wcscat_s(filePathName,filePathNameLen,L"\\") != 0) //Ending by slash
	{		
		LOG_ERROR(L"EDT_EIDMW_FindFile %s wcscat_s failed\n");
		LOG(L"file to find was %s\n",fileName);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	else if(wcscat_s(filePathName,filePathNameLen,fileName) != 0)
	{		
		LOG_ERROR(L"EDT_EIDMW_FindFile %s wcscat_s 2e failed\n");
		LOG(L"file to find was %s\n",fileName);
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(iReturnCode == EDT_OK)
	{
		hFile = _wfindfirst( filePathName, &c_file );
		if( hFile == -1L )
		{
			LOG( L"didn't find %s\n",filePathName );
		}
		else
		{
			LOG( L"Found: %s\n",filePathName );
			LogIncIndent();
			LOG( L"file length: %d\n",c_file.size );
			
			if (_wctime_s(timeBuf,TIME_BUF_SIZE, &(c_file.time_create) ) == 0 )
				LOG( L"create time: %s",timeBuf);
			if (_wctime_s(timeBuf,TIME_BUF_SIZE, &(c_file.time_access) ) == 0 )
				LOG( L"access time: %s",timeBuf);
			if (_wctime_s(timeBuf,TIME_BUF_SIZE, &(c_file.time_write) ) == 0 )
				LOG( L"write time: %s",timeBuf);
			LOG( L"file attributes:\n");
			if(c_file.attrib & _A_ARCH)
				LOG( L"_A_ARCH : Archive (file is changed or cleared by the BACKUP command)\n" );
			if(c_file.attrib & _A_HIDDEN)
				LOG( L"_A_HIDDEN : Hidden file\n" );
			if(c_file.attrib & _A_NORMAL)
				LOG( L"_A_NORMAL : Normal file\n" );
			if(c_file.attrib & _A_RDONLY)
				LOG( L"_A_RDONLY : read only file\n" );
			if(c_file.attrib & _A_SUBDIR)
				LOG( L"_A_SUBDIR : Subdirectory\n" );
			if(c_file.attrib & _A_SYSTEM)
				LOG( L"_A_SYSTEM : System file\n" );
			_findclose( hFile );
			LogDecIndent();
		}
	}
	free(filePathName);
	return iReturnCode;
}


int EDT_EIDMW_LogEidmwFiles(void)
{
	int iReturnCode=EDT_OK;
	wchar_t theSystemFolder[MAX_PATH];
	wchar_t theApplicationsFolder[MAX_PATH];

	if(!SHGetSpecialFolderPath(NULL,theSystemFolder,CSIDL_SYSTEM,FALSE))
	{
		LOG_ERROR(L"System folder not found");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(!SHGetSpecialFolderPath(NULL,theApplicationsFolder,CSIDL_PROGRAM_FILES,FALSE))
	{
		LOG_ERROR(L"Applications folder not found");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(iReturnCode == EDT_OK)
	{
		LOG(L"Eidmw Files\n");
		LogIncIndent();
		EDT_EIDMW_FindFile(L"Belgium Identity Card CSP.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"Belgium Identity Card PKCS11.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"Belpic PCSC Service.exe", theSystemFolder );
		EDT_EIDMW_FindFile(L"belpic.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"eidlib.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"EIDLibCtrl.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"eidlibj.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"eidlibj.dll.manifest", theSystemFolder );
		EDT_EIDMW_FindFile(L"eid_libeay32.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"eid_ssleay32.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"pinpad_emulator.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"winscarp.dll", theSystemFolder );
		LogDecIndent();

		LOG(L"Eidmw 2.4 Files\n");
		LogIncIndent();
		EDT_EIDMW_FindFile(L"BelgianEID.cfg", theSystemFolder );
		EDT_EIDMW_FindFile(L"belpicgui.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"shbelpicgui.exe", theSystemFolder );
		LogDecIndent();

		LOG(L"Eidmw 2.5/2.6 Files\n");
		LogIncIndent();
		EDT_EIDMW_FindFile(L"beidcsp.conf", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidcsp.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidgui.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidlib.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidlibaxctrl.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidlibeay32.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidlibjni.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidlibjni.dll.manifest", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidlibopensc.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidpkcs11.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidservicecrl.exe", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidservicepcsc.exe", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidssleay32.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidwinscard.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"shbeidgui.exe", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidgui.exe", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidsystemtray.exe", theSystemFolder );
		LogDecIndent();

		LOG(L"Eidmw 3.0 Files\n");
		LogIncIndent();
		EDT_EIDMW_FindFile(L"beidapplayer.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidcardlayer.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidcommon.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidCSPlib.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidDlgsWin32.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidlibJava_Wrapper.dll", theSystemFolder );
		LogDecIndent();

		LOG(L"Eidmw 3.5/4.0 Files\n");
		LogIncIndent();
		EDT_EIDMW_FindFile(L"beid35applayer.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beid35cardlayer.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beid35common.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beid35DlgsWin32.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"siscardplugins\\siscardplugin1_BE_EID_35__ACS_ACR38U__.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"siscardplugins\\siscardplugin1_BE_EID_35__ACS ACR38U__.dll", theSystemFolder );
		LogDecIndent();

		LOG(L"Eidmw 4.0 Files\n");
		LogIncIndent();
		EDT_EIDMW_FindFile(L"Belgium Identity Card\\EidViewer\\eID Viewer.exe", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"Belgium Identity Card\\beidoffice2010_XAdES_XL.exe", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"Belgium Identity Card\\beidoutlooksnc.exe", theApplicationsFolder );

		EDT_EIDMW_FindFile(L"beid_ff_pkcs11.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"beidpkcs11.dll", theSystemFolder );
		EDT_EIDMW_FindFile(L"\\beidpp\\xireid.dll", theSystemFolder );
		LogDecIndent();

		LOG(L"Eidmw minidriver Files\n");
		LogIncIndent();
		EDT_EIDMW_FindFile(L"beidmdrv.dll", theSystemFolder );
		LogDecIndent();
	}
	if(!SHGetSpecialFolderPath(NULL,theApplicationsFolder,CSIDL_PROGRAM_FILES,FALSE))
	{
		LOG_ERROR(L"System folder not found");
		iReturnCode = EDT_ERR_INTERNAL;
	}
	if(iReturnCode == EDT_OK)
	{
		LOG(L"Eidmw 2.3 Files\n");
		LogIncIndent();
		EDT_EIDMW_FindFile(L"beid35gui.exe", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"beid35libCpp.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"eid.ico", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"beidoutlooksnc.exe", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"beid-pkcs11-register.html", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"beid-pkcs11-unregister.html", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"eidmw_en.qm", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"eidmw_nl.qm", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"eidmw_fr.qm", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"eidmw_de.qm", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"License_en.rtf", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"License_nl.rtf", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"License_fr.rtf", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"License_de.rtf", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"THIRDPARTY-LICENSES.txt", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"QtCore4.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"QtGui4.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"imageformats\\qjpeg4.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"beidlib.jar", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"eidlib.jar", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"BEID_old.html", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"XAdESLib.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"beidlibC.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"x509ac.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"XalanC_1_10.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"XalanMessages_1_10.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"xercesc_2_7.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"xsec_1_4_0.dll", theApplicationsFolder );
		EDT_EIDMW_FindFile(L"libeay32.dll", theApplicationsFolder );
		LogDecIndent();
	}
	return iReturnCode;
}

/*
eidmw related services
TODO: search for these too (already part of other loggings, but might be nice for the overview to print them here again)
//Stop and remove the services
if(RETURN_OK!= (nRetCode = StopAndRemoveService(L"BELGIUM_ID_CARD_SERVICE",	g_lTimeout)))	goto end;
if(RETURN_OK!= (nRetCode = StopAndRemoveService(L"eID Privacy Service",		g_lTimeout)))	goto end;
if(RETURN_OK!= (nRetCode = StopAndRemoveService(L"beidPrivacyFilter",		g_lTimeout)))	goto end;
if(RETURN_OK!= (nRetCode = StopAndRemoveService(L"eID CRL Service",			g_lTimeout)))	goto end;

}*/
