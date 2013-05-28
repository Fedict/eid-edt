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

void testMacNewcardIssue(SCARDHANDLE* phCard);
void testCardConfusedIssue(SCARDHANDLE* phCard);
void testGetATR(SCARDHANDLE* phCard);
void testGetATR_2(SCARDHANDLE* phCard);
void testGetATR_3(SCARDCONTEXT* phCard, WCHAR* readerName);

void BusySleep(int msecs)
{
	LARGE_INTEGER lPerformanceCount;
	LARGE_INTEGER lPerformanceFrequency;
	LONGLONG lTimeToWait;
	QueryPerformanceCounter(&lPerformanceCount);
	QueryPerformanceFrequency(&lPerformanceFrequency);

	lTimeToWait = lPerformanceCount.LowPart + (msecs * lPerformanceFrequency.LowPart/1000);
	QueryPerformanceCounter(&lPerformanceCount);
	do 
	{
		QueryPerformanceCounter(&lPerformanceCount);

	} while (lPerformanceCount.LowPart < lTimeToWait);
	return;
}

int main(int argc, CHAR* argv[])
{
	//WCHAR*						pmszReaders = NULL;
	CHAR*							pBmszReaders;
	CHAR*							pfirstReader;
	CHAR*							pnexttReader;
	LONG							retval = 0;
	SCARDCONTEXT  		hContext;
	DWORD							cchReaders = SCARD_AUTOALLOCATE;
	DWORD							dwTotalLen = 0;
	SCARDHANDLE						hCard;
	DWORD							dwActiveProtocol;
//	BYTE              SW1, SW2;
	LPBYTE							pbAttr = NULL;
	DWORD							cByte = SCARD_AUTOALLOCATE;
//	DWORD							i;
	DWORD							iSleep = 250;

	LogInitialize();

	retval = SCardEstablishContext ( SCARD_SCOPE_USER,NULL,NULL,&hContext);
	if(retval == SCARD_S_SUCCESS)
	{
		retval = SCardListReadersA(hContext,NULL,(LPSTR)&pBmszReaders,&cchReaders);
		if(retval == SCARD_S_SUCCESS)
		{
			pfirstReader = pBmszReaders;
			pnexttReader = pfirstReader;

			while(cchReaders > dwTotalLen+1)
			{
				//testGetATR_3(&hContext, pnexttReader);
	/*			do
				{
					retval = SCardConnect(hContext,pnexttReader,
					SCARD_SHARE_EXCLUSIVE,SCARD_PROTOCOL_T0,&hCard,&dwActiveProtocol);
				}
				while (retval != SCARD_S_SUCCESS);
				wprintf(L"YAY, exclusive access, sleeping now for 10 minutes\n");
				Sleep(600000);
					SCardReleaseContext(hContext);
wprintf(L"YAY, released access, waiting for keystroke\n");
	getchar();*/

				retval = SCardConnectA(hContext,pnexttReader,
					SCARD_SHARE_SHARED,SCARD_PROTOCOL_T0,&hCard,&dwActiveProtocol);

				if(retval == SCARD_S_SUCCESS)
				{                
					GetCardData(hCard);
					SelectByAID(hCard);
					SelectByAbsPath(hCard);
					SelectByFileId(hCard);
					ReadBinaries(hCard);
					//testMacNewcardIssue(&hCard);
					//testCardConfusedIssue(&hCard);
					//testGetATR(&hCard);
					//testGetATR_2(&hCard);
					SCardDisconnect(hCard,SCARD_RESET_CARD);
				}
				else
				{
					Log("SCardConnect to %s failed\n",pfirstReader);
				}

				dwTotalLen += strlen(pnexttReader)+1;
				if(dwTotalLen < cchReaders)
					pnexttReader = pfirstReader + dwTotalLen;		
			}

			/*SCARD_LEAVE_CARD Do not do anything special.
			SCARD_RESET_CARD Reset the card.
			SCARD_UNPOWER_CARD Power down the card.
			SCARD_EJECT_CARD Eject the card.*/

			SCardFreeMemory(hContext, pBmszReaders);
		}
		else
		{
			Log("SCardListReaders failed\n");
		}
	}
	else
	{
		Log("SCardEstablishContext failed\n");
	}

	SCardReleaseContext(hContext);

	Log("retval = %d\n",retval);

	LogFinalize();
	getchar();
	return 0;
}

void testMacNewcardIssue(SCARDHANDLE* phCard)
{
	BYTE pbSendBuffer[7] = {0x00 ,0xA4 ,0x02 ,0x0C ,0x02 ,0x50 ,0x38};
	BYTE pbRecvBuffer[4];
	BYTE pbSendBuffer2[5] = {0x00 ,0xB0 ,0x00 ,0x00 ,0xF8};
	BYTE pbRecvBuffer2[248];//248=F8
	//00 A4 08 0C 04 DF 00 50 32
	DWORD cbSendLength = sizeof(pbSendBuffer);
	DWORD cbRecvLength = sizeof(pbRecvBuffer);
	DWORD cbSendLength2 = sizeof(pbSendBuffer2);
	DWORD cbRecvLength2 = sizeof(pbRecvBuffer2);
	DWORD retval;
	BYTE				SW1 = 0x90;
	BYTE				SW2 = 0x00;
	//select authentication cert
	retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);
	SW1 = pbRecvBuffer2[cbRecvLength-2];
	SW2 = pbRecvBuffer2[cbRecvLength-1];
	if( (SW1 != 0x90) || (SW2 != 0x00) || retval != SCARD_S_SUCCESS )
		printf ("select authentication cert SW1 = 0x%x SW1 = 0x%x retval = 0x%x\n",SW1,SW2,retval);
	//read the authentication cert
	
	retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer2,cbSendLength2,NULL, pbRecvBuffer2,&cbRecvLength2);
	SW1 = pbRecvBuffer2[cbRecvLength2-2];
	SW2 = pbRecvBuffer2[cbRecvLength2-1];
	if( (SW1 != 0x90) || (SW2 != 0x00) || retval != SCARD_S_SUCCESS )
		printf ("read authentication cert SW1 = 0x%x SW1 = 0x%x retval = 0x%x\n",SW1,SW2,retval);

	while ( (SW1 == 0x90) && (SW2 == 0x00) )
	{
		SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer2,cbSendLength2,NULL, pbRecvBuffer2,&cbRecvLength2);
		SW1 = pbRecvBuffer2[cbRecvLength2-2];
		SW2 = pbRecvBuffer2[cbRecvLength2-1];
		printf ("received: 0x%x 0x%x  len = %d - retval: 0x%x\n",pbRecvBuffer2[0],pbRecvBuffer2[1],cbRecvLength2, retval);
	}
	if(SW1 == 0x6c)
	{
		cbRecvLength2 = SW2;
		SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer2,cbSendLength2,NULL, pbRecvBuffer2,&cbRecvLength2);
		SW1 = pbRecvBuffer2[cbRecvLength2-2];
		SW2 = pbRecvBuffer2[cbRecvLength2-1];
		printf ("final received: 0x%x 0x%x  len = %d - retval: 0x%x\n",pbRecvBuffer2[0],pbRecvBuffer2[1],cbRecvLength2, retval);
	}

	//retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);
	//printf ("received: 0x%x 0x%x  len = %d - retval: 0x%x\n",pbRecvBuffer2[0],pbRecvBuffer2[1],cbRecvLength2, retval);
	printf ("testMacNewcardIssue completed \n");
}

void testCardConfusedIssue(SCARDHANDLE* phCard)
{

	BYTE				SW1 = 0x90;
	BYTE				SW2 = 0x00;


	//BYTE pbSendBuffer[14] = {0x00,0xA4,0x04,0x00,0x09,0xA0,0x00,0x00,0x03,0x97,0x42,0x54,0x46,0x59};
	BYTE pbSendBuffer[16] = {0x00 ,0xA4 ,0x04 ,0x00 ,0x0B ,0xA0 ,0x00 ,0x00 ,0x03 ,0x97 ,0x43 ,0x49 ,0x44 ,0x5F ,0x01 ,0x00};
	BYTE pbRecvBuffer[4];
	DWORD cbSendLength = sizeof(pbSendBuffer);
	DWORD cbRecvLength = sizeof(pbRecvBuffer);
	DWORD retval;
	//BYTE pbSendBuffer2[14] = {0x00,0xA4,0x04,0x00,0x09,0xA0,0x00,0x00,0x03,0x08,0x00,0x00,0x10,0x00};
	BYTE pbSendBuffer2[5] = {0x00,0xCA,0x7F,0x68,0x00};
	//				BYTE pbSendBuffer2[5] = {0x80,0xE4,0x00,0x00,0x10};

	BYTE pbRecvBuffer2[4];
	DWORD cbSendLength2 = sizeof(pbSendBuffer2);
	DWORD cbRecvLength2 = sizeof(pbRecvBuffer2);
	DWORD retval2;
	//BYTE pbSendBuffer3[16] = {0x00,0xA4,0x04,0x00,0x0B,0xA0,0x00,0x00,0x03,0x97,0x43,0x49,0x44,0x5F,0x01,0x00};
	BYTE pbSendBuffer3[14] = {0x00,0xA4,0x04,0x00,0x09,0xA0,0x00,0x00,0x03,0x08,0x00,0x00,0x10,0x00};
	BYTE pbRecvBuffer3[4];
	DWORD cbSendLength3 = sizeof(pbSendBuffer3);
	DWORD cbRecvLength3 = sizeof(pbRecvBuffer3);
	DWORD retval3;
	BYTE pbSendBuffer4[14] = {0x00,0xA4,0x04,0x00,0x09,0xA0,0x00,0x00,0x03,0x97,0x42,0x54,0x46,0x59};
	BYTE pbRecvBuffer4[4];
	DWORD cbSendLength4 = sizeof(pbSendBuffer4);
	DWORD cbRecvLength4 = sizeof(pbRecvBuffer4);
	DWORD retval4;
	//	BYTE pbSendBuffer5[14] = {0x00,0xA4,0x04,0x00,0x09,0xA0,0x00,0x00,0x03,0x08,0x00,0x00,0x10,0x00};
	BYTE pbSendBuffer5[4] = {0x80,0xE6,0x00,0x00};
	BYTE pbRecvBuffer5[4];
	DWORD cbSendLength5 = sizeof(pbSendBuffer5);
	DWORD cbRecvLength5 = sizeof(pbRecvBuffer5);
	DWORD retval5;

	/*	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);
	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer2,cbSendLength2,NULL, pbRecvBuffer2,&cbRecvLength2);
	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer3,cbSendLength3,NULL, pbRecvBuffer3,&cbRecvLength3);
	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer4,cbSendLength4,NULL, pbRecvBuffer4,&cbRecvLength4);
	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer5,cbSendLength5,NULL, pbRecvBuffer5,&cbRecvLength5);*/
	while (1) 
	{
		Sleep(500);
		retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);
		//BusySleep(1);
		retval2 = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer2,cbSendLength2,NULL, pbRecvBuffer2,&cbRecvLength2);
		//BusySleep(1);
		retval3 = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer3,cbSendLength3,NULL, pbRecvBuffer3,&cbRecvLength3);
		//BusySleep(1);
		retval4 = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer4,cbSendLength4,NULL, pbRecvBuffer4,&cbRecvLength4);
		//BusySleep(1);
		retval5 = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer5,cbSendLength5,NULL, pbRecvBuffer5,&cbRecvLength5);
		printf ("received: 0x%x 0x%x  len = %d - retval: 0x%x\n",pbRecvBuffer[0],pbRecvBuffer[1],cbRecvLength, retval);
		printf ("received: 0x%x 0x%x  len = %d - retval: 0x%x\n",pbRecvBuffer2[0],pbRecvBuffer2[1],cbRecvLength2, retval2);
		printf ("received: 0x%x 0x%x  len = %d - retval: 0x%x\n",pbRecvBuffer3[0],pbRecvBuffer3[1],cbRecvLength3, retval3);
		printf ("received: 0x%x 0x%x  len = %d - retval: 0x%x\n",pbRecvBuffer4[0],pbRecvBuffer4[1],cbRecvLength4, retval4);

		printf ("received: 0x%x 0x%x  len = %d - retval: 0x%x\n",pbRecvBuffer5[0],pbRecvBuffer5[1],cbRecvLength5, retval5);

		/*	Sleep(iSleep);
		BeidSelectApplet( hCard);
		Sleep(iSleep);
		SW1 = pbRecvBuffer5[cbRecvLength5-2];
		SW2 = pbRecvBuffer5[cbRecvLength5-1];
		if ( ( SW1 != 0x90 ) || ( SW2 != 0x00 ) )
		{
		BeidSelectApplet( hCard);
		}

		getchar();*/
	}

	retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);
}

void testGetATR(SCARDHANDLE* phCard)
{
	BYTE pbAtr[36];
	DWORD dwLen = 36;
	DWORD retval;

	retval = SCardGetAttrib(*phCard, SCARD_ATTR_ATR_STRING, pbAtr, &dwLen);

	printf ("trying to get the card's ATR by calling SCardGetAttrib\n");
	printf ("returned: ATR length = %d retval: 0x%08x\n",dwLen, retval);
	printf ("ATR = ");
	for(DWORD i = 0; i < dwLen; i++)
	{
		printf(" 0x%02x",pbAtr[i]);
	}
	printf ("\n");

}

void testGetATR_2(SCARDHANDLE* phCard)
{
	CHAR readerName[256];
	DWORD readerNameLen = 256;
	BYTE  pbAtr[32];
	DWORD dwLen = 32;
	DWORD dwState;
	DWORD dwProtocol;
	DWORD retval;

	retval = SCardStatus(*phCard, readerName, &readerNameLen, &dwState,
		&dwProtocol, pbAtr,	&dwLen); 

	printf ("trying to get the card's ATR by calling SCardStatus\n");
	printf ("returned: ATR length = %d retval: 0x%08x\n",dwLen, retval);
	printf ("ATR = ");
	for(DWORD i = 0; i < dwLen; i++)
	{
		printf(" 0x%02x",pbAtr[i]);
	}
	printf ("\n");

}

void testGetATR_3(SCARDCONTEXT* phContext, CHAR* readerName)
{
	DWORD retval;
	ULONG ulTimeout = INFINITE;
	ULONG ulReaderCount = 1;
	SCARD_READERSTATE txreaderStates[1];

	memset(txreaderStates,0, sizeof(txreaderStates));
	txreaderStates[0].szReader = readerName;

	//get initial values
	retval = SCardGetStatusChange(*phContext, 0, txreaderStates ,ulReaderCount); 
	//wait for changes
	txreaderStates[0].dwCurrentState = txreaderStates[0].dwEventState;
	retval = SCardGetStatusChange(*phContext, ulTimeout, txreaderStates ,ulReaderCount); 

	printf ("trying to get the card's ATR by calling SCardGetStatusChange\n");
	printf ("returned: ATR length = %d retval: 0x%08x\n",txreaderStates[0].cbAtr, retval);
	printf ("ATR = ");
	for(DWORD i = 0; i < txreaderStates[0].cbAtr ; i++)
	{
		printf(" 0x%02x",txreaderStates[0].rgbAtr[i]);
	}
	printf ("\n");

}
