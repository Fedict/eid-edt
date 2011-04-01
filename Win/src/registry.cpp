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
#include "registry.h"
#include "util_registry.h"


int EDT_StartRegistryLog()
{
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_REGISTRY_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"gathering registry info...\r\n");

	int iReturnCode = EDT_OK;

	registryLogPermissions(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Cryptography\\Calais\\Readers");


	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_REGISTRY_STOP>\n\n");

	return iReturnCode;
}

