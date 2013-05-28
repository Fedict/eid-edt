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
DWORD ReadBinary(SCARDHANDLE hCard, BYTE *absPath, BYTE absPathLen);

//functions
DWORD ReadBinaries(SCARDHANDLE hCard)
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

	Log("\nReadBinary EFDIR\n");
	dwReturn = ReadBinary(hCard, EFDIR, sizeof(EFDIR));

	Log("\nReadBinary EFODF\n");
	dwFunctionReturn = ReadBinary(hCard, EFODF, sizeof(EFODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFODF\n");
	dwFunctionReturn = ReadBinary(hCard, EFTokenInfo, sizeof(EFODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFTokenInfo\n");
	dwFunctionReturn = ReadBinary(hCard, EFTokenInfo, sizeof(EFTokenInfo));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFAODF\n");
	dwFunctionReturn = ReadBinary(hCard, EFAODF, sizeof(EFAODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFPrKDF\n");
	dwFunctionReturn = ReadBinary(hCard, EFPrKDF, sizeof(EFPrKDF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFCDF\n");
	dwFunctionReturn = ReadBinary(hCard, EFCDF, sizeof(EFCDF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFCertAuth\n");
	dwFunctionReturn = ReadBinary(hCard, EFCertAuth, sizeof(EFCertAuth));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFCertNonrep\n");
	dwFunctionReturn = ReadBinary(hCard, EFCertNonrep, sizeof(EFCertNonrep));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFCertCA\n");
	dwFunctionReturn = ReadBinary(hCard, EFCertCA, sizeof(EFCertCA));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFCertRoot\n");
	dwFunctionReturn = ReadBinary(hCard, EFCertRoot, sizeof(EFCertRoot));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFCertRN\n");
	dwFunctionReturn = ReadBinary(hCard, EFCertRN, sizeof(EFCertRN));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFODF\n");
	dwFunctionReturn = ReadBinary(hCard, EFODF, sizeof(EFODF));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;


	Log("\nReadBinary EFIDRN\n");
	dwFunctionReturn = ReadBinary(hCard, EFIDRN, sizeof(EFIDRN));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFSGNRN\n");
	dwFunctionReturn = ReadBinary(hCard, EFSGNRN, sizeof(EFSGNRN));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFIDADDR\n");
	dwFunctionReturn = ReadBinary(hCard, EFIDADDR, sizeof(EFIDADDR));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFSGNADDR\n");
	dwFunctionReturn = ReadBinary(hCard, EFSGNADDR, sizeof(EFSGNADDR));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFIDPhoto\n");
	dwFunctionReturn = ReadBinary(hCard, EFIDPhoto, sizeof(EFIDPhoto));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFPUK\n");
	dwFunctionReturn = ReadBinary(hCard, EFPUK, sizeof(EFPUK));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	Log("\nReadBinary EFPref\n");
	dwFunctionReturn = ReadBinary(hCard, EFPref, sizeof(EFPref));
	if(dwReturn == SCARD_S_SUCCESS)
		dwReturn = dwFunctionReturn;

	return dwReturn;
}

DWORD ReadBinary(SCARDHANDLE hCard, BYTE *absPath, BYTE absPathLen)
{
	DWORD dwReturn = SCARD_S_SUCCESS;

	dwReturn = SelectAbsPath(hCard, absPath, absPathLen);
	if( dwReturn == SCARD_S_SUCCESS )
	{
		DWORD             dwReturn = SCARD_S_SUCCESS;
		unsigned char     orgCmd[128];
		unsigned int      orgCmdLen = 0;
		unsigned char     recvbuf[4096];
		unsigned long     recvlen = sizeof(recvbuf);
		unsigned long     recvbuflen = sizeof(recvbuf);
		unsigned long			readlen = 0;
		bool							keepreading = TRUE;

		orgCmd[0] = 0x00; //CLA
		orgCmd[1] = 0xB0; //INS :READ COMMAND
		orgCmd[2] = 0x00; //high byte of offset
		orgCmd[3] = 0x00; //low byte of offset

		orgCmdLen = 4;

		dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, 0x00, recvbuf, &recvlen);

		if(dwReturn == SCARD_S_SUCCESS ) 
		{
			if(recvlen >= 2)
			{
				if((recvbuf[recvlen-2] == 0x90 )&&(recvbuf[recvlen-1] == 0x00))
				{
					while (keepreading == TRUE)
					{
						readlen += recvlen-2;
						orgCmd[2] = (BYTE)(readlen/256); //high byte of offset
						orgCmd[3] = (BYTE)(readlen - (orgCmd[2] * 256)); //low byte of offset

						dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, 0x00, recvbuf, &recvlen);
						if(dwReturn != SCARD_S_SUCCESS)
						{
							keepreading = FALSE;
						}
						else 
						{
							if(recvlen >= 2)
							{
								if((recvbuf[recvlen-2] != 0x90 )||(recvbuf[recvlen-1] != 0x00))
								{
									keepreading = FALSE;
								}
							}
							else
							{
								keepreading = FALSE;
							}
						}
					}
				}
			}
			HandleResponse(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, FALSE, 0, recvbuf, &recvlen, recvbuflen, "ReadBinary");
		}
	}
	return dwReturn;
}
