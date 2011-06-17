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
#include "thirdParty.h"
#include "util_process.h"


////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_StartThirdPartyLog()
{
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_THIRDPARTYLOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"searching for related processes...\r\n");

	processPresenceMap mapProcessPresence; 
	mapProcessPresence.insert(processPresenceItem(L"accoca.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"pthosttr.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"accrdsub.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"acevents.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"asghost.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"ac_sharedstore.exe",false));
	mapProcessPresence.insert(processPresenceItem(L"beid35xsign.exe",false));

	int iReturnCode = EDT_process_logList(&mapProcessPresence);

	if (iReturnCode==EDT_OK)
	{
		LOG(L"HP Protect Tools Processes:\n");
		LogIncIndent();
		EDT_process_logProcessPresenceMap(&mapProcessPresence);
		LogDecIndent();
	}
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_THIRDPARTYLOG_STOP>\n");
	LOG(EDT_LINE_BREAK);

	return iReturnCode;
}
