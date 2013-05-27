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

DWORD BeidSelectApplet(SCARDHANDLE  pCardData);
void testMacNewcardIssue(SCARDHANDLE* phCard);
void testCardConfusedIssue(SCARDHANDLE* phCard);
void testFullPathSelect(SCARDHANDLE* phCard);
void testGetATR(SCARDHANDLE* phCard);
void testGetATR_2(SCARDHANDLE* phCard);
void testGetATR_3(SCARDCONTEXT* phCard, WCHAR* readerName);
void testIDAIDSelect(SCARDHANDLE* phCard);
void testBELPICAIDSelect(SCARDHANDLE* phCard);

DWORD GetCardData(SCARDHANDLE hCard);

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
		retval = SCardListReaders(hContext,NULL,(LPSTR)&pBmszReaders,&cchReaders);
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

				retval = SCardConnect(hContext,pnexttReader,
					SCARD_SHARE_SHARED,SCARD_PROTOCOL_T0,&hCard,&dwActiveProtocol);

				if(retval == SCARD_S_SUCCESS)
				{                
					//BeidSelectApplet(hCard);
					GetCardData(hCard);
					//testMacNewcardIssue(&hCard);
					//testCardConfusedIssue(&hCard);
					//testIDAIDSelect(&hCard);s
					//testBELPICAIDSelect(&hCard);
					//testGetATR(&hCard);
					//testGetATR_2(&hCard);
					//testFullPathSelect(&hCard);
					SCardDisconnect(hCard,SCARD_RESET_CARD);
				}
				else
				{
					printf("SCardConnect to %s failed\n",pfirstReader);
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
			printf("SCardListReaders failed\n");
		}
	}
	else
	{
		printf("SCardEstablishContext failed\n");
	}

	SCardReleaseContext(hContext);

	printf("retval = %d\n",retval);

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

DWORD BeidSelectApplet(SCARDHANDLE  hCard)
{
   DWORD             dwReturn = 0;

   SCARD_IO_REQUEST  ioSendPci = {1, sizeof(SCARD_IO_REQUEST)};
   SCARD_IO_REQUEST  ioRecvPci = {1, sizeof(SCARD_IO_REQUEST)};

   unsigned char     Cmd[128];
   unsigned int      uiCmdLg = 0;

   unsigned char     recvbuf[256];
   unsigned long     recvlen = sizeof(recvbuf);
   BYTE              SW1, SW2;
   BYTE              bBELPIC_AID[12] = { 0xA0, 0x00, 0x00, 0x01, 0x77, 0x50, 0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };  
   BYTE              cbBELPIC_AID = sizeof(bBELPIC_AID);
   BYTE				 bAPPLET_AID[15] = { 0xA0, 0x00, 0x00, 0x00, 0x30, 0x29, 0x05, 0x70, 0x00, 0xAD, 0x13, 0x10, 0x01, 0x01, 0xFF };
   BYTE              cbAPPLET_AID = sizeof(bAPPLET_AID);

   int               i = 0;
   
   /***************/
   /* Select File */
   /***************/
   Cmd [0] = 0x00;
   Cmd [1] = 0xA4; /* SELECT COMMAND */
   Cmd [2] = 0x04;
   Cmd [3] = 0x0C;
   Cmd [4] = cbBELPIC_AID;
   memcpy(&Cmd[5], bBELPIC_AID, cbBELPIC_AID);
      
   uiCmdLg = 5 + cbBELPIC_AID;

   dwReturn = SCardTransmit(hCard, 
                            &ioSendPci, 
                            Cmd, 
                            uiCmdLg, 
                            &ioRecvPci, 
                            recvbuf, 
                            &recvlen);
   if ( dwReturn != SCARD_S_SUCCESS )
   {
      LogTrace("1st SCardTransmit errorcode: [0x%02X]\n", dwReturn);
	  return(dwReturn);
   }
   SW1 = recvbuf[recvlen-2];
   SW2 = recvbuf[recvlen-1];
   if ( ( SW1 != 0x90 ) || ( SW2 != 0x00 ) )
   {
      LogTrace("Select Failed: [0x%02X][0x%02X] - we try to select the applet aid\n", SW1, SW2);

	  Cmd [0] = 0x00;
      Cmd [1] = 0xA4; /* SELECT COMMAND */
      Cmd [2] = 0x04;
      Cmd [3] = 0x00;
      Cmd [4] = cbAPPLET_AID;
      memcpy(&Cmd[5], bAPPLET_AID, cbAPPLET_AID);
      
      uiCmdLg = 5 + cbAPPLET_AID;
      recvlen = sizeof(recvbuf);
      dwReturn = SCardTransmit(hCard, 
                            &ioSendPci, 
                            Cmd, 
                            uiCmdLg, 
                            &ioRecvPci, 
                            recvbuf, 
                            &recvlen);
      if ( dwReturn != SCARD_S_SUCCESS )
      {
         LogTrace("SCardTransmit errorcode: [0x%02X]\n", dwReturn);
    	 return (1);
      }
	  SW1 = recvbuf[recvlen-2];
	  SW2 = recvbuf[recvlen-1];
	  if ( ( SW1 != 0x90 ) || ( SW2 != 0x00 ) )
	  {
		LogTrace("Select Failed: [0x%02X][0x%02X]\n", SW1, SW2);
		return (1);
	  }
   }
   
	LogTrace("BeidSelectApplet completed!\n");
   return (dwReturn);
}


void testFullPathSelect(SCARDHANDLE* phCard)
{
	BYTE				SW1 = 0x90;
	BYTE				SW2 = 0x00;

	BYTE pbSendBuffer[] = {0x00 ,0xA4 ,0x08 ,0x0C ,0x04 ,0xDF ,0x01 ,0x40 ,0x31};
	BYTE pbRecvBuffer[4];
	DWORD cbSendLength = sizeof(pbSendBuffer);
	DWORD cbRecvLength = sizeof(pbRecvBuffer);
	DWORD retval;

	retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);

	printf ("trying to select EF(ID) by its full path\n");
	printf ("received: 0x%x 0x%x  len = %d - retval: 0x%08x\n",pbRecvBuffer[0],pbRecvBuffer[1],cbRecvLength, retval);
	getchar();

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

void testIDAIDSelect(SCARDHANDLE* phCard)
{

//	static const unsigned char APPLET_AID[] = {0xA0,0x00,0x00,0x00,0x30,0x29,0x05,0x70,0x00,0xAD,0x13,0x10,0x01,0x01,0xFF};
//static const unsigned char BELPIC_AID[] = {0xA0,0x00,0x00,0x01,0x77,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};
//static const unsigned char ID_AID[] =     {0xA0,0x00,0x00,0x01,0x77,0x49,0x64,0x46,0x69,0x6C,0x65,0x73};

	BYTE pbSendBuffer[] = {0x00 ,0xA4 ,0x04 ,0x0C , 0x0C ,0xA0,0x00,0x00,0x01,0x77,0x49,0x64,0x46,0x69,0x6C,0x65,0x73};
	BYTE pbRecvBuffer[4];
	DWORD cbSendLength = sizeof(pbSendBuffer);
	DWORD cbRecvLength = sizeof(pbRecvBuffer);
	DWORD retval;

	retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);

	printf ("trying to select DF(ID) by its AID\n");
	printf ("received: 0x%x 0x%x  len = %d - retval: 0x%08x\n",pbRecvBuffer[0],pbRecvBuffer[1],cbRecvLength, retval);
	getchar();
}

void testBELPICAIDSelect(SCARDHANDLE* phCard)
{

//	static const unsigned char APPLET_AID[] = {0xA0,0x00,0x00,0x00,0x30,0x29,0x05,0x70,0x00,0xAD,0x13,0x10,0x01,0x01,0xFF};
//static const unsigned char BELPIC_AID[] = {0xA0,0x00,0x00,0x01,0x77,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};
//static const unsigned char ID_AID[] =     {0xA0,0x00,0x00,0x01,0x77,0x49,0x64,0x46,0x69,0x6C,0x65,0x73};

	BYTE pbSendBuffer[] = {0x00 ,0xA4 ,0x04 ,0x0C , 0x0C ,0xA0,0x00,0x00,0x01,0x77,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};
	BYTE pbRecvBuffer[4];
	DWORD cbSendLength = sizeof(pbSendBuffer);
	DWORD cbRecvLength = sizeof(pbRecvBuffer);
	DWORD retval;

	retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);

	printf ("trying to select DF(BELPIC) by its AID\n");
	printf ("received: 0x%x 0x%x  len = %d - retval: 0x%08x\n",pbRecvBuffer[0],pbRecvBuffer[1],cbRecvLength, retval);
	getchar();
}