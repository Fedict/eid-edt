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

#include "certstore.h"
#include "log.h"

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS DECLARATION ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_StartCertStoreLog()
{
	int iReturnCode = EDT_OK;

	//LOG(EDT_LINE_BREAK);
	//LOG(L"<EDT_CERTSTORELOG_START>\n");
	//LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"logging certificate store info...\r\n");

	//_wsystem(L"certutil -silent -store -user MY >> F:\\Users\\Frederik\Documents\\EDTlog.txt");
	iReturnCode = LOG_CMD(L"certutil -silent -store -user MY");

	//LOG(EDT_LINE_BREAK);
	//LOG(L"<EDT_CERTSTORELOG_STOP>\n");

	return iReturnCode;
}
