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

#include "sct_general.h"

//function declarations
DWORD GetCardDataOneStep(SCARDHANDLE hCard);
DWORD GetCardDataTwoStep(SCARDHANDLE hCard);

//functions
DWORD GetCardData(SCARDHANDLE hCard)
{
	DWORD dwReturn = SCARD_S_SUCCESS;

	Log("\nGetCardDataOneStep\n");
	dwReturn = GetCardDataOneStep(hCard);

	Log("\nGetCardDataTwoStep\n");
	if(dwReturn == SCARD_S_SUCCESS)
	{
		dwReturn = GetCardDataTwoStep(hCard);
	}
	else
	{
		GetCardDataTwoStep(hCard);
	}
	return dwReturn;
}

DWORD GetCardDataOneStep(SCARDHANDLE hCard)
{
	DWORD             dwReturn = SCARD_S_SUCCESS;
	unsigned char     orgCmd[128];
	unsigned int      orgCmdLen = 0;
	unsigned char     recvbuf[256];
	unsigned long     recvlen = sizeof(recvbuf);

	orgCmd [0] = 0x80;
	orgCmd [1] = 0xE4;
	orgCmd [2] = 0x00;
	orgCmd [3] = 0x00;
	orgCmdLen = 4;

	dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, 0x1C , recvbuf, &recvlen);

	if(dwReturn == SCARD_S_SUCCESS ) 
	{
		HandleResponse(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, 0xFF , recvbuf, &recvlen, 256, "GetCardData");
	}
	return dwReturn;
}

DWORD GetCardDataTwoStep(SCARDHANDLE hCard)
{
	DWORD             dwReturn = SCARD_S_SUCCESS;
	unsigned char     orgCmd[128];
	unsigned int      orgCmdLen = 0;
	unsigned char     recvbuf[256];
	unsigned long     recvlen = sizeof(recvbuf);

	orgCmd [0] = 0x80;
	orgCmd [1] = 0xE4;
	orgCmd [2] = 0x00;
	orgCmd [3] = 0x00;
	orgCmdLen = 4;

	dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, 0xFF , recvbuf, &recvlen);

	if(dwReturn == SCARD_S_SUCCESS ) 
	{
		HandleResponse(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, 0xFF , recvbuf, &recvlen, 256, "GetCardData");
	}
	return dwReturn;
}

