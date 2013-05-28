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
#include "sct_selectbyabsolutepath.h"

//function prototypes

//functions
DWORD SelectByAbsPath(SCARDHANDLE hCard)
{
	DWORD dwReturn = SCARD_S_SUCCESS;
	DWORD dwFunctionReturn = SCARD_S_SUCCESS;

	BYTE EFDIR[] = {0x2F,0x00};
	BYTE EFODF[] = {0xDF, 0x00, 0x50, 0x31};
	BYTE EFTokenInfo[] = {0xDF, 0x00, 0x50, 0x32};
	BYTE EFAODF[] = {0xDF, 0x00, 0x50, 0x34};
	BYTE EFPrKDF[] = {0xDF, 0x00, 0x50, 0x35};
	BYTE EFCDF[] = {0xDF, 0x00, 0x50, 0x37};
	BYTE EFCertAuth[] = {0xDF, 0x00, 0x50, 0x38};
	BYTE EFCertNonrep[] = {0xDF, 0x00, 0x50, 0x39};
	BYTE EFCertCA[] = {0xDF, 0x00, 0x50, 0x3A};
	BYTE EFCertRoot[] = {0xDF, 0x00, 0x50, 0x3B};
	BYTE EFCertRN[] = {0xDF, 0x00, 0x50, 0x3C};

	BYTE EFIDRN[] = {0xDF, 0x01, 0x40, 0x31};
	BYTE EFSGNRN[] = {0xDF, 0x01, 0x40, 0x32};
	BYTE EFIDADDR[] = {0xDF, 0x01, 0x40, 0x33};
	BYTE EFSGNADDR[] = {0xDF, 0x01, 0x40, 0x34};
	BYTE EFIDPhoto[] = {0xDF, 0x01, 0x40, 0x35};
	BYTE EFPUK[] = {0xDF, 0x01, 0x40, 0x38};
	BYTE EFPref[] = {0xDF, 0x01, 0x40, 0x39};

	Log("\nSelectAbsPath EFDIR\n");
	dwReturn = SelectAbsPath(hCard, EFDIR, sizeof(EFDIR));

	Log("\nSelectAbsPath EFODF\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFODF, sizeof(EFODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFODF\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFTokenInfo, sizeof(EFODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFTokenInfo\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFTokenInfo, sizeof(EFTokenInfo));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFAODF\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFAODF, sizeof(EFAODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFPrKDF\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFPrKDF, sizeof(EFPrKDF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFCDF\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFCDF, sizeof(EFCDF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFCertAuth\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFCertAuth, sizeof(EFCertAuth));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFCertNonrep\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFCertNonrep, sizeof(EFCertNonrep));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFCertCA\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFCertCA, sizeof(EFCertCA));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFCertRoot\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFCertRoot, sizeof(EFCertRoot));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFCertRN\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFCertRN, sizeof(EFCertRN));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFODF\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFODF, sizeof(EFODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;


	Log("\nSelectAbsPath EFIDRN\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFIDRN, sizeof(EFIDRN));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFSGNRN\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFSGNRN, sizeof(EFSGNRN));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFIDADDR\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFIDADDR, sizeof(EFIDADDR));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFSGNADDR\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFSGNADDR, sizeof(EFSGNADDR));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFIDPhoto\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFIDPhoto, sizeof(EFIDPhoto));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFPUK\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFPUK, sizeof(EFPUK));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectAbsPath EFPref\n");
	dwFunctionReturn = SelectAbsPath(hCard, EFPref, sizeof(EFPref));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	return dwReturn;
}

DWORD SelectAbsPath(SCARDHANDLE hCard, BYTE *absPath, BYTE absPathLen)
{
	DWORD             dwReturn = SCARD_S_SUCCESS;
	unsigned char     orgCmd[128];
	unsigned int      orgCmdLen = 0;
	unsigned char     recvbuf[256];
	unsigned long     recvlen = sizeof(recvbuf);
	unsigned long     recvbuflen = sizeof(recvbuf);

	orgCmd [0] = 0x00; //CLA
	orgCmd [1] = 0xA4; //INS :SELECT COMMAND
	orgCmd [2] = 0x08; //absolute path
	orgCmd [3] = 0x0C; //No FCI to be returned
	orgCmd [4] = absPathLen; //LC
	memcpy(&orgCmd[5], absPath, absPathLen); //DATA

	orgCmdLen = 5 + absPathLen;

	dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, FALSE, 0 , recvbuf, &recvlen);

	if(dwReturn == SCARD_S_SUCCESS ) 
	{
		HandleResponse(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, FALSE, 0 , recvbuf, &recvlen, recvbuflen, "SelectAbsPath");
	}
	return dwReturn;
}
