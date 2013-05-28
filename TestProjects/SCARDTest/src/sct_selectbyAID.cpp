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

//function prototypes
DWORD SelectAID(SCARDHANDLE hCard, BYTE *AID, BYTE AIDLen);
DWORD SelectAIDApplet(SCARDHANDLE hCard);
DWORD SelectAIDBelpic(SCARDHANDLE hCard);
DWORD SelectAIDId(SCARDHANDLE hCard);

//functions
DWORD SelectByAID(SCARDHANDLE hCard)
{
	DWORD dwReturn = SCARD_S_SUCCESS;
	DWORD dwFunctionReturn = SCARD_S_SUCCESS;

	Log("\SelectAIDApplet\n");
	dwReturn = SelectAIDApplet(hCard);

	Log("\SelectAIDBelpic\n");
	dwFunctionReturn = SelectAIDBelpic(hCard);

//  only on old beidcards
//	Log("\SelectAIDId\n");
//	dwFunctionReturn = SelectAIDId(hCard);

	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	return dwReturn;
}

DWORD SelectAIDApplet(SCARDHANDLE hCard)
{
	BYTE appletAID[15] = { 0xA0, 0x00, 0x00, 0x00, 0x30, 0x29, 0x05, 0x70, 0x00, 0xAD, 0x13, 0x10, 0x01, 0x01, 0xFF };
	BYTE appletAIDLen = sizeof(appletAID);

	return SelectAID(hCard, appletAID, appletAIDLen);
}

DWORD SelectAIDBelpic(SCARDHANDLE hCard)
{
	BYTE belpicAID[12] = { 0xA0, 0x00, 0x00, 0x01, 0x77, 0x50, 0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };  
  BYTE belpicAIDLen = sizeof(belpicAID);

	return SelectAID(hCard, belpicAID, belpicAIDLen);
}

//ID AID is obsolete
DWORD SelectAIDId(SCARDHANDLE hCard)
{
	BYTE IdAID[] = {0xA0,0x00,0x00,0x01,0x77,0x49,0x64,0x46,0x69,0x6C,0x65,0x73};
  BYTE IdAIDLen = sizeof(IdAID);

	return SelectAID(hCard, IdAID, IdAIDLen);
}

DWORD SelectAID(SCARDHANDLE hCard, BYTE *AID, BYTE AIDLen)
{
	DWORD             dwReturn = SCARD_S_SUCCESS;
	unsigned char     orgCmd[128];
	unsigned int      orgCmdLen = 0;
	unsigned char     recvbuf[256];
	unsigned long     recvlen = sizeof(recvbuf);
	unsigned long     recvbuflen = sizeof(recvbuf);

	orgCmd [0] = 0x00; //CLA
	orgCmd [1] = 0xA4; //INS :SELECT COMMAND
	orgCmd [2] = 0x04; //AID
	orgCmd [3] = 0x0C; //No FCI to be returned
	orgCmd [4] = AIDLen; //LC
	memcpy(&orgCmd[5], AID, AIDLen); //DATA

	orgCmdLen = 5 + AIDLen;

	dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, FALSE, 0 , recvbuf, &recvlen);

	if(dwReturn == SCARD_S_SUCCESS ) 
	{
		HandleResponse(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, FALSE, 0 , recvbuf, &recvlen, recvbuflen, "SelectAID");
	}
	return dwReturn;
}
