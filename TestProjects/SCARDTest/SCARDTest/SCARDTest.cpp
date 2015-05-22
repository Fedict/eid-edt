// SCARDTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "winscard.h"

#include "WinBase.h"

#define LogTrace printf

DWORD SendMSCmd (SCARDHANDLE hCard);
DWORD BeidSelectCACApplets1(SCARDHANDLE  pCardData);
DWORD BeidSelectCACApplets2(SCARDHANDLE  pCardData);
DWORD BeidSelectBELPICApplets(SCARDHANDLE  pCardData);
DWORD BeidSelectApplet(SCARDHANDLE  hCard,BYTE P2,BYTE *bAID1, BYTE cbAID1, BYTE *bAID2, BYTE cbAID2 );

void testMinidriverStartSequence(SCARDHANDLE* phCard);
void testMacNewcardIssue(SCARDHANDLE* phCard);
void testCardConfusedIssue(SCARDHANDLE* phCard);
void testFullPathSelect(SCARDHANDLE* phCard);
void testGetATR(SCARDHANDLE* phCard);
void testGetATR_2(SCARDHANDLE* phCard);
void testGetATR_3(SCARDCONTEXT* phCard, WCHAR* readerName);
void testStatus(SCARDCONTEXT* phContext);
void testIDAIDSelect(SCARDHANDLE* phCard);
void testBELPICAIDSelect(SCARDHANDLE* phCard);
void testPPDU_GetFeatureList(SCARDHANDLE* phCard);
DWORD SendCommand(SCARDHANDLE hCard, unsigned char* orgCmd, unsigned int orgCmdLen,
	bool lcNeeded, unsigned char lc, unsigned char* Data,
	bool leNeeded, unsigned char le, unsigned char *recvbuf, unsigned long *recvlen);
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

int _tmain(int argc, _TCHAR* argv[])
{
	//WCHAR*						pmszReaders = NULL;
	WCHAR*							pBmszReaders;
	WCHAR*							pfirstReader;
	WCHAR*							pnexttReader;
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
	retval = SCardEstablishContext ( SCARD_SCOPE_USER,NULL,NULL,&hContext);
	if(retval == SCARD_S_SUCCESS)
	{
		retval = SCardListReaders(hContext,NULL,(LPWSTR)&pBmszReaders,&cchReaders);
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
					//BeidSelectCACApplets1(hCard);
					//SendMSCmd(hCard);
					//BeidSelectCACApplets2(hCard);
					BeidSelectBELPICApplets(hCard);
					GetCardData(hCard);
					//testMinidriverStartSequence(&hCard);
					//testMacNewcardIssue(&hCard);
					//testCardConfusedIssue(&hCard);
					//testIDAIDSelect(&hCard);
					//testBELPICAIDSelect(&hCard);
					//testGetATR(&hCard);
					//testGetATR_2(&hCard);
					//testFullPathSelect(&hCard);
					//testStatus(&hCard);
					//testPPDU_GetFeatureList(&hCard);

					SCardDisconnect(hCard,SCARD_RESET_CARD);
				}
				else
				{
					wprintf(L"SCardConnect to %s failed\n",pfirstReader);
				}

				dwTotalLen += wcslen(pnexttReader)+1;
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
	getchar();
	return 0;
}

void testMinidriverStartSequence(SCARDHANDLE* phCard)
{
	BeidSelectCACApplets1(*phCard);
	SendMSCmd(*phCard);
	BeidSelectCACApplets2(*phCard);
	GetCardData(*phCard);
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

	/*	BYTE pbSendBuffer[] = {0xFF ,0xC2 ,0x01 ,0x00 , 0x00};
	BYTE pbRecvBuffer[200];
	DWORD cbSendLength = sizeof(pbSendBuffer);
	DWORD cbRecvLength = sizeof(pbRecvBuffer);
	DWORD retval;*/
	//Sleep(50);
	//retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);

	//BYTE pbSendBuffer[14] = {0x00,0xA4,0x04,0x00,0x09,0xA0,0x00,0x00,0x03,0x97,0x42,0x54,0x46,0x59};
	//BYTE pbSendBuffer[16] = {0x00 ,0xA4 ,0x04 ,0x00 ,0x0B ,0xA0 ,0x00 ,0x00 ,0x03 ,0x97 ,0x43 ,0x49 ,0x44 ,0x5F ,0x01 ,0x00};
	BYTE pbSendBuffer[] = {0xFF,0xC2,0X01,0x06,0x20,
		0x1e,0x1e,0x89,0x47,0x04,0x0c,0x04,0x02,0x01,0x08,
		0x13,0x00,0x00,0x00,0x00,0xD0,0x00,0x00,0x00,
		0x00,0x20,0x00,0x01,0x08,0x20,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	BYTE pbRecvBuffer[4];
	DWORD cbSendLength = sizeof(pbSendBuffer);
	DWORD cbRecvLength = sizeof(pbRecvBuffer);
	DWORD retval;
	//BYTE pbSendBuffer2[14] = {0x00,0xA4,0x04,0x00,0x09,0xA0,0x00,0x00,0x03,0x08,0x00,0x00,0x10,0x00};
	BYTE pbSendBuffer2[5] =  {0x00,0xCA,0x7F,0x68,0x00};//
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
	BYTE pbSendBuffer5[] = {0xFF ,0xC2 ,0x01 ,0x00 , 0x00};//{0x80,0xE6,0x00,0x00};
	BYTE pbRecvBuffer5[4];
	DWORD cbSendLength5 = sizeof(pbSendBuffer5);
	DWORD cbRecvLength5 = sizeof(pbRecvBuffer5);
	DWORD retval5;

	int loop = 0;
	/*	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);
	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer2,cbSendLength2,NULL, pbRecvBuffer2,&cbRecvLength2);
	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer3,cbSendLength3,NULL, pbRecvBuffer3,&cbRecvLength3);
	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer4,cbSendLength4,NULL, pbRecvBuffer4,&cbRecvLength4);
	SCardTransmit(hCard,SCARD_PCI_T0,pbSendBuffer5,cbSendLength5,NULL, pbRecvBuffer5,&cbRecvLength5);*/
	while (loop < 10) 
	{
		loop++;
		//Sleep(50);
		retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);
		//BusySleep(1);
		//Sleep(250);
		retval2 = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer2,cbSendLength2,NULL, pbRecvBuffer2,&cbRecvLength2);
		//Sleep(50);
		//BusySleep(1);
		retval3 = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer3,cbSendLength3,NULL, pbRecvBuffer3,&cbRecvLength3);
		//BusySleep(1);
		//Sleep(50);
		retval4 = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer4,cbSendLength4,NULL, pbRecvBuffer4,&cbRecvLength4);
		//BusySleep(1);
		//Sleep(50);
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

DWORD GetCardData(SCARDHANDLE hCard)
{
	DWORD             dwReturn = 0;
	unsigned char     orgCmd[128];
	unsigned int      orgCmdLen = 0;
	unsigned char     recvbuf[256];
	unsigned long     recvlen = sizeof(recvbuf);
	BYTE SW1;
	BYTE SW2;

//	orgCmd [0] = 0x80;
//	orgCmd [1] = 0xE4;
//	orgCmd [2] = 0x00;
//	orgCmd [3] = 0x00;
//	orgCmdLen = 4;

	orgCmd [0] = 0x80;
	orgCmd [1] = 0xE4;
	orgCmd [2] = 0x00;
	orgCmd [3] = 0x00;
	orgCmd [4] = 0x10;
	orgCmdLen = 5;
	//dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, 0xFF , recvbuf, &recvlen);
	dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, FALSE, 0xFF , recvbuf, &recvlen);

	if(dwReturn == SCARD_S_SUCCESS ) 
	{
		if (recvlen >= 2)
		{
			SW1 = recvbuf[recvlen-2];
			SW2 = recvbuf[recvlen-1];

			if (SW1 == 0x6c)
			{
				//SW2 contains length of the carddata (1C + 2 = 1E)
				 recvlen = sizeof(recvbuf);
				dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, SW2, recvbuf, &recvlen);
				if (recvlen >= 2)
				{
					SW1 = recvbuf[recvlen-2];
					SW2 = recvbuf[recvlen-1];

				}
				else
				{
					LogTrace("GetCardData recvlen = %d\n", recvlen);
				}
			}
			if ( !((SW1 == 0x90) && (SW2 == 0x00)) && (SW1 != 0x61) )
			{
				LogTrace("GetCardData returned: SW1 SW2 = [0x%02X][0x%02X]\n", SW1, SW2);
			}
			else
			{
					int counter = 0;
					LogTrace("GetCardData recvlen = %d\n", recvlen);
					for(;counter < (recvlen-2);counter++)
					{
						if(((counter+1) % 16)==0)
						{
							LogTrace("\n");
						}
						LogTrace("0x%0.2x ", recvbuf[counter]);
					}
			}
		}
		else
		{
			LogTrace("GetCardData recvlen = %d\n", recvlen);
		}
	}
	return dwReturn;
}

DWORD SendMSCmd (SCARDHANDLE hCard)
{
	DWORD dwReturn = 0;
	unsigned char recvbuf[256];
	unsigned long recvlen = sizeof(recvbuf);

	unsigned char orgCmd[5] = {0x00,0xCA,0x7F,0x68,0x00};
	dwReturn = SendCommand(hCard, orgCmd, 5,
	false, 0, NULL,
	false, 0, 
	recvbuf, &recvlen);

	return dwReturn;
}

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
	unsigned char SW1, SW2;

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
		LogTrace("SendCommand: Command too long\n");
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
	if(*recvlen > 1)
	{
		SW1 = recvbuf[*recvlen-2];
		SW2 =  recvbuf[*recvlen-1];
		if ( !((SW1 == 0x90) && (SW2 == 0x00)) )
		{
			LogTrace("returned: SW1 SW2 = [0x%02X][0x%02X]\n", SW1, SW2);
		}
	}
	return dwReturn;
}

DWORD BeidSelectCACApplets1(SCARDHANDLE  hCard)
{
	BYTE bCAC_AID[12] = { 0x0B,0xA0, 0x00, 0x00, 0x03, 0x97, 0x43, 0x49, 0x44, 0x5f, 0x01, 0x00 };  
	BYTE cbCAC_AID = sizeof(bCAC_AID);

	return BeidSelectApplet(hCard, 0x00, bCAC_AID, cbCAC_AID, NULL, 0);
}

DWORD BeidSelectCACApplets2(SCARDHANDLE  hCard)
{
	BYTE bCAC_AID[10] = { 0x09,0xA0, 0x00, 0x00, 0x03, 0x08, 0x00, 0x00, 0x10, 0x00 };  
	BYTE cbCAC_AID = sizeof(bCAC_AID);
	BYTE bCAC2_AID[15] = { 0x09,0xA0, 0x00, 0x00, 0x03, 0x97, 0x42, 0x54, 0x46, 0x59 };
	BYTE cbCAC2_AID = sizeof(bCAC2_AID);

	return BeidSelectApplet(hCard, 0x00, bCAC_AID, cbCAC_AID, bCAC2_AID, cbCAC2_AID);
}

DWORD BeidSelectBELPICApplets(SCARDHANDLE  hCard)
{
	BYTE              bBELPIC_AID[12] = { 0xA0, 0x00, 0x00, 0x01, 0x77, 0x50, 0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };  
	BYTE              cbBELPIC_AID = sizeof(bBELPIC_AID);
	BYTE				 bAPPLET_AID[15] = { 0xA0, 0x00, 0x00, 0x00, 0x30, 0x29, 0x05, 0x70, 0x00, 0xAD, 0x13, 0x10, 0x01, 0x01, 0xFF };
	BYTE              cbAPPLET_AID = sizeof(bAPPLET_AID);

	return BeidSelectApplet(hCard, 0x0C, bBELPIC_AID, cbBELPIC_AID, bAPPLET_AID, cbAPPLET_AID);
}


DWORD BeidSelectApplet(SCARDHANDLE  hCard,BYTE P2, BYTE *bAID1, BYTE cbAID1, BYTE *bAID2, BYTE cbAID2 )
{
	DWORD             dwReturn = 0;

	SCARD_IO_REQUEST  ioSendPci = {1, sizeof(SCARD_IO_REQUEST)};
	SCARD_IO_REQUEST  ioRecvPci = {1, sizeof(SCARD_IO_REQUEST)};

	unsigned char     Cmd[128];
	unsigned int      uiCmdLg = 0;

	unsigned char     recvbuf[256];
	unsigned long     recvlen = sizeof(recvbuf);
	BYTE              SW1, SW2;


	int               i = 0;

	/***************/
	/* Select File */
	/***************/
	Cmd [0] = 0x00;
	Cmd [1] = 0xA4; /* SELECT COMMAND */
	Cmd [2] = 0x04;
	Cmd [3] = P2;//0x0C;
	Cmd [4] = cbAID1;
	memcpy(&Cmd[5], bAID1, cbAID1);

	uiCmdLg = 5 + cbAID1;

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
	if (( SW1 != 0x90 ) || ( SW2 != 0x00 ))
	{
		LogTrace("Select Failed: [0x%02X][0x%02X] \n", SW1, SW2);
		if(cbAID2 > 0)
		{
			Cmd [0] = 0x00;
			Cmd [1] = 0xA4; /* SELECT COMMAND */
			Cmd [2] = 0x04;
			Cmd [3] = 0x00;
			Cmd [4] = cbAID2;
			memcpy(&Cmd[5], bAID2, cbAID2);

			uiCmdLg = 5 + cbAID2;
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
	for(int i = 0; i < dwLen; i++)
	{
		printf(" 0x%02x",pbAtr[i]);
	}
	printf ("\n");

}

void testGetATR_2(SCARDHANDLE* phCard)
{
	WCHAR readerName[256];
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
	for(int i = 0; i < dwLen; i++)
	{
		printf(" 0x%02x",pbAtr[i]);
	}
	printf ("\n");

}

void testGetATR_3(SCARDCONTEXT* phContext, WCHAR* readerName)
{
	DWORD retval;
	ULONG ulTimeout = INFINITE;
	ULONG ulReaderCount = 1;
	SCARD_READERSTATEW txreaderStates[1];

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
	for(int i = 0; i < txreaderStates[0].cbAtr ; i++)
	{
		printf(" 0x%02x",txreaderStates[0].rgbAtr[i]);
	}
	printf ("\n");

}


void testStatus(SCARDCONTEXT* phContext)
{
	wchar_t szReaderName[1024];
	DWORD dwReaderLen = 1024;
	DWORD dwState;
	DWORD dwProtocol;
	BYTE bAttribute[32];
	DWORD dwAtrLen;

		BYTE pbRecvBuffer[200];
	DWORD dwRecvLength, dwReturn;


	dwReturn = SCardControl(*phContext, 
		SCARD_CTL_CODE(3400),
		NULL,
		0,
		pbRecvBuffer,
		sizeof(pbRecvBuffer),
		&dwRecvLength);

 SCardStatus( *phContext, 
  szReaderName, 
  &dwReaderLen, 
  &dwState, 
  &dwProtocol, 
  &bAttribute[0], 
  &dwAtrLen 
);
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

void testPPDU_GetFeatureList(SCARDHANDLE* phCard)
{

	//	static const unsigned char APPLET_AID[] = {0xA0,0x00,0x00,0x00,0x30,0x29,0x05,0x70,0x00,0xAD,0x13,0x10,0x01,0x01,0xFF};
	//static const unsigned char BELPIC_AID[] = {0xA0,0x00,0x00,0x01,0x77,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};
	//static const unsigned char ID_AID[] =     {0xA0,0x00,0x00,0x01,0x77,0x49,0x64,0x46,0x69,0x6C,0x65,0x73};

	BYTE pbSendBuffer[] = {0xFF ,0xC2 ,0x01 ,0x00 , 0x00};
	BYTE pbRecvBuffer[2];
	DWORD cbSendLength = sizeof(pbSendBuffer);
	DWORD cbRecvLength = sizeof(pbRecvBuffer);
	DWORD retval;
	//Sleep(50);
	retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbRecvBuffer,&cbRecvLength);

	printf ("trying to get PPDU GetFeatureList\n");
	if(retval == SCARD_E_INSUFFICIENT_BUFFER)
	{
		BYTE *pbBiggerRecvBuffer = (BYTE *)malloc(cbRecvLength);
		if (pbBiggerRecvBuffer != NULL)
		{
			retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBuffer,cbSendLength,NULL, pbBiggerRecvBuffer,&cbRecvLength);

			printf ("received: 0x%x 0x%x  len = %d - retval: 0x%08x\n",pbBiggerRecvBuffer[cbRecvLength-2],pbBiggerRecvBuffer[cbRecvLength-1],cbRecvLength, retval);
			if(retval == SCARD_S_SUCCESS)
			{
				BYTE bsupportedFeatureIndex = 0;
				for( bsupportedFeatureIndex = 0;bsupportedFeatureIndex < (cbRecvLength -2);bsupportedFeatureIndex++ )
				{
					//BYTE FEATURE_VERIFY_PIN_DIRECT 0x06
					if(pbBiggerRecvBuffer[bsupportedFeatureIndex] == 0x06)
					{
						BYTE xVerifyCmd[32];
						//see Pincmd1
						memset(&xVerifyCmd, 0, sizeof(xVerifyCmd));
						xVerifyCmd[0] = 30;//bTimerOut
						xVerifyCmd[1] = 30;//bTimerOut2
						xVerifyCmd[2] = 0x80 | 0x08 | 0x00 | 0x01;////BCD bmFormatString = ToFormatString(pin);
						xVerifyCmd[3] = 0x47;//.bmPINBlockString = ToPinBlockString(pin);
						xVerifyCmd[4] = 0x04;//BCD .bmPINLengthFormat = ToPinLengthFormat(pin);
						xVerifyCmd[5] = 12;//.wPINMaxExtraDigit[0] = 8;//GetMaxPinLen(pin);
						xVerifyCmd[6] = 4;//.wPINMaxExtraDigit[1] = 4;//(unsigned char) pin.ulMinLen;
						xVerifyCmd[7] = 0x02;//.bEntryValidationCondition = 0x02;
						xVerifyCmd[8] = 0x01;//.bNumberMessage = 0x01;
						xVerifyCmd[9] = 0x08;//0x04;// .wLangId[0] = (unsigned char) ((0x0409 & 0xff00) / 256);//PP_LANG_EN
						xVerifyCmd[10] = 0x13;//0x09;//.wLangId[1] = (unsigned char) (0x0409 & 0xff);
						xVerifyCmd[11] = 0;//.bMsgIndex = 0;
						xVerifyCmd[12] = 0;//bTeoPrologue
						xVerifyCmd[13] = 0;//bTeoPrologue
						xVerifyCmd[14] = 0;//bTeoPrologue
						xVerifyCmd[15] = 13;//ulDataLength
						xVerifyCmd[16] = 0;//ulDataLength
						xVerifyCmd[17] = 0;//ulDataLength
						xVerifyCmd[18] = 0;//ulDataLength
						xVerifyCmd[19] = 0x00; // CLA
						xVerifyCmd[20] = 0x20; // INS Verify
						xVerifyCmd[21] = 0x00; // P1
						xVerifyCmd[22] = 0x01; // P2
						xVerifyCmd[23] = 0x08; // Lc = 8 bytes in command data
						xVerifyCmd[24] = 0x20 ; // 
						xVerifyCmd[25] = 0xFF;//BELPIC_PAD_CHAR Pin[1]
						xVerifyCmd[26] = 0xFF;//BELPIC_PAD_CHAR Pin[1]
						xVerifyCmd[27] = 0xFF;//BELPIC_PAD_CHAR Pin[1]
						xVerifyCmd[28] = 0xFF;//BELPIC_PAD_CHAR Pin[1]
						xVerifyCmd[29] = 0xFF;//BELPIC_PAD_CHAR Pin[1]
						xVerifyCmd[30] = 0xFF;//BELPIC_PAD_CHAR Pin[1]
						xVerifyCmd[31] = 0xFF;//BELPIC_PAD_CHAR Pin[1]

						//ToUchar4(oAPDU.Size(), xVerifyCmd.ulDataLength);
						//memcpy(xVerifyCmd.abData, oAPDU.GetBytes(), oAPDU.Size());
						//ulVerifyCmdLen = sizeof(xVerifyCmd) - PP_APDU_MAX_LEN + oAPDU.Size();


						BYTE pbSendBufferVerifyPINDirect[256];// = {0xFF ,0xC2 ,0x01 ,0x06 , 0x00};
						pbSendBufferVerifyPINDirect[0] = 0xFF;
						pbSendBufferVerifyPINDirect[1] = 0xC2;
						pbSendBufferVerifyPINDirect[2] = 0x01;
						pbSendBufferVerifyPINDirect[3] = 0x06;
						pbSendBufferVerifyPINDirect[4] = sizeof(xVerifyCmd);
						BYTE bSendBufferVerifyPINDirectLength = 5;

						if(sizeof(xVerifyCmd) < (sizeof(pbSendBufferVerifyPINDirect)+5) )
						{
							memcpy(&pbSendBufferVerifyPINDirect[5],xVerifyCmd,sizeof(xVerifyCmd));
							bSendBufferVerifyPINDirectLength += sizeof(xVerifyCmd);
						}
						cbRecvLength = sizeof(pbBiggerRecvBuffer);
						
						retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBufferVerifyPINDirect,bSendBufferVerifyPINDirectLength,NULL, pbBiggerRecvBuffer,&cbRecvLength);
						if(retval == SCARD_E_INSUFFICIENT_BUFFER)
						{
							free (pbBiggerRecvBuffer);
							pbBiggerRecvBuffer = (BYTE *)malloc(cbRecvLength);
							retval = SCardTransmit(*phCard,SCARD_PCI_T0,pbSendBufferVerifyPINDirect,bSendBufferVerifyPINDirectLength,NULL, pbBiggerRecvBuffer,&cbRecvLength);
						}
						printf ("received: 0x%x 0x%x  len = %d - retval: 0x%08x\n",pbBiggerRecvBuffer[cbRecvLength-2],pbBiggerRecvBuffer[cbRecvLength-1],cbRecvLength, retval);
					}
				}
			}
			free (pbBiggerRecvBuffer);
		}

	}
	else
	{
		printf ("received: 0x%x 0x%x  len = %d - retval: 0x%08x\n",pbRecvBuffer[cbRecvLength-2],pbRecvBuffer[cbRecvLength-1],cbRecvLength, retval);
	}
	getchar();
}