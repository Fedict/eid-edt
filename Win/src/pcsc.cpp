/* ****************************************************************************

 * EDT Project.
 * Copyright (C) 2010-2011 FedICT.
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

#include <edt.h>
#include "pcsc.h"
#include "log.h"
#include "util_pcsc.h"
//#include "progress.h"



////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_StartPCSCLog()
{
	int iReturnCode = EDT_OK;
	//int iFunctionCode = EDT_OK;
	bool pcscAvailable = false;
	LOG(L"<EDT_PCSCLOG_START>\n");
	LOG_SCREEN(L"testing pcsc...\r\n");

	//load the pcsc lib
	LOG(L"pcsctest1: PCSC load library\n");
	iReturnCode = EDT_pcscLoad();
	if(iReturnCode == EDT_OK){
		LOG(L"pcsctest1: PCSC load library succeeded\n------\n");
		//try to establish and release a connection
		LOG(L"pcsctest2: EDT_pcscEstablishContext\n");
		iReturnCode = EDT_pcscEstablishContext();
		if(iReturnCode == EDT_OK){
			LOG(L"pcsctest2: EDT_pcscEstablishContext succeeded\n------\n");
			//try to get the list of connected readers
			LOG(L"pcsctest3: EDT_pcscLogReaderList\n");
			iReturnCode = EDT_pcscLogReaderList();
			if(iReturnCode == EDT_OK){
				LOG(L"pcsctest3: EDT_pcscLogReaderList succeeded\n------\n");
				//try to get the list of connected cards
				LOG(L"pcsctest4: EDT_pcscLogCardList\n");
				iReturnCode = EDT_pcscLogCardList();				
				if(iReturnCode == EDT_OK){
					LOG(L"pcsctest4: EDT_pcscLogCardList succeeded\n------\n");


				}
				else{
					LOG_ERROR(L"pcsctest4: EDT_pcscLogCardList failed\n");
				}
			}
			else{
				LOG_ERROR(L"pcsctest3: EDT_pcscLogReaderList failed\n");
			}
		}
		else{
			LOG_ERROR(L"pcsctest2: EDT_pcscEstablishContext failed\n");
		}
	}
	else
	{
		LOG_ERROR(L"pcsctest1: PCSC load library failed\n");
	}

	LOG(L"<EDT_PCSCLOG_STOP>\n\n");
	return iReturnCode;
}
