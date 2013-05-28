/* ****************************************************************************

 * SCTest Project.
 * Copyright (C) 2013 FedICT.
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
#pragma once

#include <stdio.h>
#include <winscard.h>

#define SCT_VERSION_MAJOR 0
#define SCT_VERSION_MINOR 1

DWORD GetCardData(SCARDHANDLE hCard);
DWORD SelectByAID(SCARDHANDLE hCard);
DWORD SelectByAbsPath(SCARDHANDLE hCard);
DWORD SelectByFileId(SCARDHANDLE hCard);
DWORD ReadBinaries(SCARDHANDLE hCard);