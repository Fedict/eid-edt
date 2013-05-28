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
DWORD SelectFileID(SCARDHANDLE hCard, BYTE *absPath, BYTE absPathLen);
DWORD SelectFileIDFollowPath(SCARDHANDLE hCard,BYTE *FolderID, BYTE FolderIDLen, BYTE *FileID, BYTE FileIDLen);

//functions
DWORD SelectByFileId(SCARDHANDLE hCard)
{
	DWORD dwReturn = SCARD_S_SUCCESS;
	DWORD dwFunctionReturn = SCARD_S_SUCCESS;

	BYTE MF[] = {0x3F,0x00};

	BYTE EFDIR[] = {0x2F,0x00};

	BYTE DFBELPIC[] = {0xDF,0x00};
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

	BYTE DFID[] = {0xDF,0x00};
	BYTE EFIDRN[] = {0xDF, 0x01, 0x40, 0x31};
	BYTE EFSGNRN[] = {0xDF, 0x01, 0x40, 0x32};
	BYTE EFIDADDR[] = {0xDF, 0x01, 0x40, 0x33};
	BYTE EFSGNADDR[] = {0xDF, 0x01, 0x40, 0x34};
	BYTE EFIDPhoto[] = {0xDF, 0x01, 0x40, 0x35};
	BYTE EFPUK[] = {0xDF, 0x01, 0x40, 0x38};
	BYTE EFPref[] = {0xDF, 0x01, 0x40, 0x39};

	Log("\nSelectFileIDFollowPath EFDIR\n");
	dwReturn = SelectFileID(hCard, MF, sizeof(MF));
	dwFunctionReturn = SelectFileID(hCard, EFDIR, sizeof(EFDIR));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFODF\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFODF, sizeof(EFODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFODF\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFTokenInfo, sizeof(EFODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFTokenInfo\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFTokenInfo, sizeof(EFTokenInfo));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFAODF\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFAODF, sizeof(EFAODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFPrKDF\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFPrKDF, sizeof(EFPrKDF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFCDF\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFCDF, sizeof(EFCDF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFCertAuth\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFCertAuth, sizeof(EFCertAuth));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFCertNonrep\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFCertNonrep, sizeof(EFCertNonrep));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFCertCA\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFCertCA, sizeof(EFCertCA));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFCertRoot\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFCertRoot, sizeof(EFCertRoot));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFCertRN\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFCertRN, sizeof(EFCertRN));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFODF\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFBELPIC, sizeof(DFBELPIC), EFODF, sizeof(EFODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;


	Log("\nSelectFileIDFollowPath EFIDRN\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFID, sizeof (DFID),  EFIDRN, sizeof(EFIDRN));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFSGNRN\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFID, sizeof (DFID),  EFSGNRN, sizeof(EFSGNRN));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFIDADDR\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFID, sizeof (DFID),  EFIDADDR, sizeof(EFIDADDR));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFSGNADDR\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFID, sizeof (DFID),  EFSGNADDR, sizeof(EFSGNADDR));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFIDPhoto\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFID, sizeof (DFID),  EFIDPhoto, sizeof(EFIDPhoto));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFPUK\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFID, sizeof (DFID),  EFPUK, sizeof(EFPUK));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nSelectFileIDFollowPath EFPref\n");
	dwFunctionReturn = SelectFileIDFollowPath(hCard, DFID, sizeof (DFID),  EFPref, sizeof(EFPref));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	return dwReturn;
}

DWORD SelectFileIDFollowPath(SCARDHANDLE hCard,BYTE *FolderID, BYTE FolderIDLen, BYTE *FileID, BYTE FileIDLen)
{
	DWORD dwReturn = SCARD_S_SUCCESS;
	BYTE MF[] = {0x3F,0x00};
	BYTE MFLen = sizeof(MF);

	dwReturn = SelectFileID(hCard, MF, MFLen);
	if(dwReturn == SCARD_S_SUCCESS)
	{
		dwReturn = SelectFileID(hCard, FolderID, FolderIDLen);
		if(dwReturn == SCARD_S_SUCCESS)
			dwReturn = SelectFileID(hCard, FileID, FileIDLen);
	}
	return dwReturn;
}

DWORD SelectFileID(SCARDHANDLE hCard, BYTE *FileID, BYTE FileIDLen)
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
	orgCmd [4] = FileIDLen; //LC
	memcpy(&orgCmd[5], FileID, FileIDLen); //DATA

	orgCmdLen = 5 + FileIDLen;

	dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, FALSE, 0 , recvbuf, &recvlen);

	if(dwReturn == SCARD_S_SUCCESS ) 
	{
		HandleResponse(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, FALSE, 0 , recvbuf, &recvlen, recvbuflen, "SelectFileID");
	}
	return dwReturn;
}
