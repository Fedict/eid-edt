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
#ifdef __APPLE__
#include "Mac/mac_helper.h"
#endif

#include "util_process.h"
#include "process.h"
#include "log.h"

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_StartprocessLog()
{
	int iReturnCode = EDT_OK;

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_PROCESSLOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"logging running process' info...\r\n");

	iReturnCode = EDT_process_logList();

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_PROCESSLOG_STOP>\n");
	LOG(EDT_LINE_BREAK);

	return iReturnCode;
}
