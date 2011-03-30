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
#include "hardware.h"
#include "util_registry.h"


int EDT_StartHWLog()
{
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_HWLOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"gathering hardware info...\r\n");

	std::wstring systemManufacturer;
	std::wstring SystemProductName;

	int iReturnCode = EDT_OK;
	int iFunctionCode = EDT_OK;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// hRootKey values are HKEY_CLASSES_ROOT   HKEY_CURRENT_USER    HKEY_LOCAL_MACHINE    HKEY_USERS 
	////////////////////////////////////////////////////////////////////////////////////////////////
	iFunctionCode = registryGetValue(HKEY_LOCAL_MACHINE,L"HARDWARE\\DESCRIPTION\\System\\BIOS",L"SystemProductName",&SystemProductName);
	if( (iReturnCode == EDT_OK) && (iFunctionCode!=EDT_OK) )
		iReturnCode = iFunctionCode;
	
	iFunctionCode = registryGetValue(HKEY_LOCAL_MACHINE,L"HARDWARE\\DESCRIPTION\\System\\BIOS",L"systemManufacturer",&systemManufacturer);
	if( (iReturnCode == EDT_OK) && (iFunctionCode!=EDT_OK) )
		iReturnCode = iFunctionCode;

	LOG(L"SystemProductName = %s\n",SystemProductName.c_str());
	LOG(L"systemManufacturer = %s\n",systemManufacturer.c_str());

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_HWLOG_STOP>\n\n");

	return iReturnCode;
}

