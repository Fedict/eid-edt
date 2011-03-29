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
#ifndef __EDT_UTIL_SYSTEM_H__
#define __EDT_UTIL_SYSTEM_H__

#include <string>

typedef struct t_System_INFO {
	std::wstring OsType;

	int PlatformId;
    std::wstring MajorVersion;
    std::wstring MinorVersion;
    std::wstring BuildNumber;
	int ProductType;

    std::wstring ServicePack;
    std::wstring ProductName;
    std::wstring Description;
    std::wstring DefaultLanguage;
	std::wstring Architecture;
} System_INFO;

int systemGetInfo(System_INFO *info);
int systemReboot(void);

#endif //__EDT_UTIL_SYSTEM_H__
