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

#include "util_system.h"
#include "log.h"

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_StartSystemLog()
{
	int iReturnCode = EDT_OK;
	System_INFO info;

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_SYSTEMLOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"gathering system info...\r\n");

	if(EDT_OK == (iReturnCode = systemGetInfo(&info)))
	{
		LOG(L"          OsType = %ls\n", info.OsType.c_str());
		LOG(L"      PlatformId = %ld\n", info.PlatformId);
		LOG(L"    MajorVersion = %ls\n", info.MajorVersion.c_str());
		LOG(L"    MinorVersion = %ls\n", info.MinorVersion.c_str());
		LOG(L"     BuildNumber = %ls\n", info.BuildNumber.c_str());
		LOG(L"     ProductType = %ld\n", info.ProductType);
		LOG(L"     ServicePack = %ls\n", info.ServicePack.c_str());
		LOG(L"     ProductName = %ls\n", info.ProductName.c_str());
		LOG(L"     Description = %ls\n", info.Description.c_str());
		LOG(L" DefaultLanguage = %ls\n", info.DefaultLanguage.c_str());
		LOG(L"    Architecture = %ls\n", info.Architecture.c_str());
	}
	else
	{
		LOG(L"systemGetInfo failed, returnvalue = %d\n",iReturnCode);
	}

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_SYSTEMLOG_STOP>\n\n");
	return iReturnCode;
}
////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
