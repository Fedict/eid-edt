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

DWORD SendCommand(SCARDHANDLE hCard, unsigned char* orgCmd, unsigned int orgCmdLen,
	bool lcNeeded, unsigned char lc, unsigned char* Data,
	bool leNeeded, unsigned char le, 
	unsigned char *recvbuf, unsigned long *recvlen)
{
	DWORD             dwReturn = 0;
	SCARD_IO_REQUEST  ioSendPci = {1, sizeof(SCARD_IO_REQUEST)};
	SCARD_IO_REQUEST  ioRecvPci = {1, sizeof(SCARD_IO_REQUEST)};

	unsigned char     Cmd[256];
	unsigned int	totalCmdLen = orgCmdLen;


	if(lcNeeded == TRUE)
	{
		totalCmdLen += lc;
		totalCmdLen++; //1 for lc;
	}
	if(leNeeded == TRUE)
	{
		totalCmdLen++;
	}
	if (totalCmdLen > 256)
	{
		Log("Error: SendCommand: Command too long\n");
		return (1);
	}

	memcpy(Cmd,orgCmd,orgCmdLen);

	if(lcNeeded == TRUE)
	{
		Cmd[orgCmdLen] = lc;
		memcpy(Cmd+orgCmdLen+1,Data,lc);
	}
	if(leNeeded)
	{
		Cmd[totalCmdLen-1] = le;
	}

	LogApduSend(Cmd,totalCmdLen);

	dwReturn = SCardTransmit(hCard, 
		&ioSendPci, 
		Cmd, 
		totalCmdLen, 
		&ioRecvPci, 
		recvbuf, 
		recvlen);
	if ( dwReturn != SCARD_S_SUCCESS )
	{
		LogTrace("SendCommand errorcode: [0x%02X]\n", dwReturn);
	}
	else
	{
		LogApduResp(recvbuf,*recvlen);
	}
	return dwReturn;
}