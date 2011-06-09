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

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_EIDMWLOG_STOP>\n");
	LOG(EDT_LINE_BREAK);

	return iReturnCode;
}

/*
eidmw related services, processes and files
TODO: search for these too
//Stop and remove the services
if(RETURN_OK!= (nRetCode = StopAndRemoveService(L"BELGIUM_ID_CARD_SERVICE",	g_lTimeout)))	goto end;
if(RETURN_OK!= (nRetCode = StopAndRemoveService(L"eID Privacy Service",		g_lTimeout)))	goto end;
if(RETURN_OK!= (nRetCode = StopAndRemoveService(L"beidPrivacyFilter",		g_lTimeout)))	goto end;
if(RETURN_OK!= (nRetCode = StopAndRemoveService(L"eID CRL Service",			g_lTimeout)))	goto end;

//Kill the known process
if(RETURN_OK!= (nRetCode = KillProcess(L"beidgui.exe")))		goto end;
if(RETURN_OK!= (nRetCode = KillProcess(L"beid35gui.exe")))		goto end;
if(RETURN_OK!= (nRetCode = KillProcess(L"beidsystemtray.exe"))) goto end;
if(RETURN_OK!= (nRetCode = KillProcess(L"xsign.exe")))			goto end;
if(RETURN_OK!= (nRetCode = KillProcess(L"beid35xsign.exe")))	goto end;

//Check if the library are used (if g_bForceRemove kill the using process)
if(RETURN_OK!= (nRetCode = LibraryUsage(L"beidmdrv.dll",					g_bForceRemove)))	goto end;	//minidriver
if(RETURN_OK!= (nRetCode = LibraryUsage(L"beid35common.dll",				g_bForceRemove)))	goto end;	//3.5
if(RETURN_OK!= (nRetCode = LibraryUsage(L"beidcommon.dll",					g_bForceRemove)))	goto end;	//3.0
if(RETURN_OK!= (nRetCode = LibraryUsage(L"beidwinscard.dll",				g_bForceRemove)))	goto end;	//2.5, 2.6
if(RETURN_OK!= (nRetCode = LibraryUsage(L"beidcsp.dll",						g_bForceRemove)))	goto end;	//2.5, 2.6
if(RETURN_OK!= (nRetCode = LibraryUsage(L"BELGIUM IDENTITY CARD CSP.DLL",	g_bForceRemove)))	goto end;	//2.3, 2.4
if(RETURN_OK!= (nRetCode = LibraryUsage(L"BELPIC.DLL",						g_bForceRemove)))	goto end;	//2.3, 2.4




//Uninstall
if(RETURN_OK!= (nRetCode = Uninstall(L"2.3",    L"{44CFED0B-BF92-455B-94D3-FA967A81712E}",INSTALLTYPE_MSI,		0,			g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
if(RETURN_OK!= (nRetCode = Uninstall(L"2.4",    L"{BA42ACEA-3782-4CAD-AA10-EBC2FA14BB7E}",INSTALLTYPE_IS_MSI,	IDR_ISS_24,	g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
if(RETURN_OK!= (nRetCode = Uninstall(L"2.5",    L"{85D158F2-648C-486A-9ECC-C5D4F4ACC965}",INSTALLTYPE_IS_MSI,	IDR_ISS_25,	g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
if(RETURN_OK!= (nRetCode = Uninstall(L"2.6",    L"{EA248851-A7D5-4906-8C46-A3CA267F6A24}",INSTALLTYPE_IS_MSI,	IDR_ISS_26,	g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
if(RETURN_OK!= (nRetCode = Uninstall(L"3.0",    L"{82493A8F-7125-4EAD-8B6D-E9EA889ECD6A}",INSTALLTYPE_IS,		IDR_ISS_30,	g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
if(RETURN_OK!= (nRetCode = Uninstall(L"3.5",    L"{40420E84-2E4C-46B2-942C-F1249E40FDCB}",INSTALLTYPE_IS,		IDR_ISS_35,	g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
if(RETURN_OK!= (nRetCode = Uninstall(L"3.5 Pro",L"{4C2FBD23-962C-450A-A578-7556BC79B8B2}",INSTALLTYPE_IS,		IDR_ISS_35P,g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
//if(RETURN_OK!= (nRetCode = Uninstall(L"3.5",	L"{824563DE-75AD-4166-9DC0-B6482F2DED5A}",INSTALLTYPE_MSI,		0,			g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
if(RETURN_OK!= (nRetCode = Uninstall(L"3.5 Pro",L"{FBB5D096-1158-4e5e-8EA3-C73B3F30780A}",INSTALLTYPE_MSI,		0,			g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;

if(RETURN_OK!= (nRetCode = SearchAndUninstall(L"3.5",    L"{824563DE-75AD-4166-9DC0-B6482F2?????}",	g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
if(RETURN_OK!= (nRetCode = SearchAndUninstall(L"3.5 Pro",L"{FBB5D096-1158-4e5e-8EA3-C73B3F3?????}",	g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;
if(RETURN_OK!= (nRetCode = SearchAndUninstall(L"minidriver",L"{842C2A79-289B-4cfa-9158-349B73F?????}", g_lTimeout,g_csKeepGuid,&g_bRebootNeeded)))	goto end;

if(g_bForceRemove && wcscmp(g_csKeepGuid,L"")==0)
{
//Delete the remaining files 2.3
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"Belgium Identity Card CSP.dll")))		goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"Belgium Identity Card PKCS11.dll")))	goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"Belpic PCSC Service.exe")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"belpic.dll")))						goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"eidlib.dll")))						goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"EIDLibCtrl.dll")))					goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"eidlibj.dll")))						goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"eidlibj.dll.manifest")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"eid_libeay32.dll")))					goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"eid_ssleay32.dll")))					goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"pinpad_emulator.dll")))				goto end;
//if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"winscarp.dll")))						goto end;

//Delete the remaining files 2.4
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"BelgianEID.cfg")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"belpicgui.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"shbelpicgui.exe")))			goto end;

//Delete the remaining files 2.5/2.6
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidcsp.conf")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidcsp.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidgui.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidlib.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidlibaxctrl.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidlibeay32.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidlibjni.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidlibjni.dll.manifest")))	goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidlibopensc.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidpkcs11.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidservicecrl.exe")))		goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidservicepcsc.exe")))		goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidssleay32.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidwinscard.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"shbeidgui.exe")))				goto end;

if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"beidgui.exe")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"beidsystemtray.exe")))		goto end;

//Delete the remaining files 3.0
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidapplayer.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidcardlayer.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidcommon.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidCSPlib.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidDlgsWin32.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidlibJava_Wrapper.dll")))	goto end;

//Delete the remaining files 3.5
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beid35applayer.dll")))		goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beid35cardlayer.dll")))		goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beid35common.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beid35DlgsWin32.dll")))		goto end;

if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"siscardplugins\\siscardplugin1_BE_EID_35__ACS_ACR38U__.dll"))) goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"siscardplugins\\siscardplugin1_BE_EID_35__ACS ACR38U__.dll"))) goto end;

if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"beid35gui.exe")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"beid35libCpp.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"eid.ico")))					goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"beidoutlooksnc.exe")))		goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"beid-pkcs11-register.html")))	goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"beid-pkcs11-unregister.html"))) goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"eidmw_en.qm")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"eidmw_nl.qm")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"eidmw_fr.qm")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"eidmw_de.qm")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"License_en.rtf")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"License_nl.rtf")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"License_fr.rtf")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"License_de.rtf")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"THIRDPARTY-LICENSES.txt")))	goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"QtCore4.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"QtGui4.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"imageformats\\qjpeg4.dll")))	goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"beidlib.jar")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"eidlib.jar")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"BEID_old.html")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"XAdESLib.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"beidlibC.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"x509ac.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"XalanC_1_10.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"XalanMessages_1_10.dll")))	goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"xercesc_2_7.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"xsec_1_4_0.dll")))			goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_APP,		L"libeay32.dll")))				goto end;

//Delete the remaining files minidriver
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_SYSTEM32,	L"beidmdrv.dll")))				goto end;
if(RETURN_OK!= (nRetCode = DeleteFile(FOLDER_WOWSYS64,	L"beidmdrv.dll")))				goto end;
}*/
