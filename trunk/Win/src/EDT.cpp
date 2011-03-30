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
#include "log.h"
#include "hardware.h"
#include "system.h"
#include "pcsc.h"

// Forward declarations of functions included in this code module:
DWORD WINAPI EDTThreadFunction( LPVOID lpParam )
{
	HWND htextWnd = (HWND)lpParam ;
	//http://msdn.microsoft.com/en-us/library/aa929135.aspx
	// SetWindowText(htextWnd,L"Extra text\n");
	//SendMessage(htextWnd, EM_REPLACESEL,0,  (LPARAM)L"Extra text\r\n"); // for Win32 windows

	logInitialize(htextWnd);

	EDT_StartHWLog();
	EDT_StartSystemLog();
	EDT_StartPCSCLog();

	logFinalize(htextWnd);

	return 0;
}