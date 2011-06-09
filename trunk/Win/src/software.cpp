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
//#include <windows.h>
#include "EDT.h"

#include "software.h"
#include "log.h"
#include "util_registry.h"

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


int EDT_StartSoftwareLog()
{
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_SOFTWARELOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"searching for installed middleware...\r\n");

	int iReturnCode = EDT_OK;
	int iFunctionCode = EDT_OK;

	iFunctionCode = EDT_UtilReg_LogKeyTree(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",EDTREGFLAG_NONE);
	if( (iReturnCode == EDT_OK) && (iFunctionCode!=EDT_OK) )
		iReturnCode = iFunctionCode;
	
	LOG(EDT_HALF_LINE_BREAK);

	iFunctionCode = EDT_UtilReg_LogKeyTree(HKEY_CURRENT_USER,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",EDTREGFLAG_NONE);
	if( (iReturnCode == EDT_OK) && (iFunctionCode!=EDT_OK) )
		iReturnCode = iFunctionCode;

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_SOFTWARELOG_STOP>\n");
	LOG(EDT_LINE_BREAK);

	return iReturnCode;

}
