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

#include "sct_log.h"
#include "sct_general.h"

DWORD GetCardData(SCARDHANDLE hCard)
{
	DWORD             dwReturn = 0;
	unsigned char     orgCmd[128];
   unsigned int      orgCmdLen = 0;
   	unsigned char     recvbuf[256];
	unsigned long     recvlen = sizeof(recvbuf);
   BYTE SW1;
   BYTE SW2;

   orgCmd [0] = 0x80;
   orgCmd [1] = 0xE4;
   orgCmd [2] = 0x00;
   orgCmd [3] = 0x00;
   orgCmdLen = 4;

   dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, 0xFF , recvbuf, &recvlen);

   if(dwReturn == SCARD_S_SUCCESS ) 
   {
	   if (recvlen >= 2)
	   {
		   SW1 = recvbuf[recvlen-2];
		   SW2 = recvbuf[recvlen-1];

			 Log("GetCardData returned: SW1 SW2 = [0x%02X][0x%02X]\n", SW1, SW2);
		   if (SW1 == 0x6c)
		   {
			   dwReturn = SendCommand(hCard,orgCmd,orgCmdLen,FALSE, 0, NULL, TRUE, SW2, recvbuf, &recvlen);
			   if (recvlen >= 2)
			   {
				   SW1 = recvbuf[recvlen-2];
				   SW2 = recvbuf[recvlen-1];
			   }
			   else
			   {
				   Log("Error: GetCardData recvlen = %d\n", recvlen);
			   }
		   }
		   if ( !((SW1 == 0x90) && (SW2 == 0x00)) )
		   {
			   Log("Error: GetCardData returned: SW1 SW2 = [0x%02X][0x%02X]\n", SW1, SW2);
		   }
	   }
	   else
	   {
			Log("Error: GetCardData recvlen = %d\n", recvlen);
	   }
   }
   return dwReturn;
}

