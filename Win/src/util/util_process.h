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
#ifndef __EDT_UTIL_PROCESS_H__
#define __EDT_UTIL_PROCESS_H__

#include <map>

typedef std::map<const wchar_t*,bool> processPresenceMap;
typedef std::pair<const wchar_t*,bool> processPresenceItem;


//if a map is used, it should be constructed of process names and a boolean
//the function will set the boolean to true is the process name was found, false otherwise
//when no map is used, this function logs all processes (and their modules) running on the system
int EDT_process_logList(processPresenceMap *pProcessPresenceMap = NULL);
void EDT_process_logProcessPresenceMap(processPresenceMap *pmapProcessPresence);

#endif //__EDT_UTIL_PROCESS_H__
