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
#include "log.h"
#ifdef WIN32
#define CK_PTR *
//allign at 1 byte
#pragma pack(push, cryptoki, 1)
#include <win32.h>
#include <pkcs11.h>
#pragma pack(pop, cryptoki)
//back to default allignment

#define dlopen(lib,h) LoadLibrary(lib)
#define dlsym(h, function) GetProcAddress(h, function)
#define dlclose(h) FreeLibrary(h)
#define PKCS11_LIB L"beidpkcs11.dll"
#define RTLD_LAZY	1
#define RTLD_NOW	2
#define RTLD_GLOBAL 4

#else
#include <opensc/pkcs11.h>
#include <dlfcn.h>
#include <unistd.h>
//#define PKCS11_LIB "/usr/local/lib/libbeidpkcs11.so" 
#define PKCS11_LIB "/Library/Frameworks/BeIdPKCS11.framework/Versions/Current/lib/libbeidpkcs11.dylib"
#endif
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PRIVATE FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_pkcs11FindCards();
void PrintSlotInfo(CK_SLOT_INFO *slotInfo);
void PrintTokenInfo(CK_TOKEN_INFO *tokenInfo);

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_StartPKCS11Log()
{
	int iReturnCode = EDT_OK;
	//int iFunctionCode = EDT_OK;
	bool pkcs11Available = false;
	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_PKCS11LOG_START>\n");
	LOG(EDT_LINE_BREAK);
	LOG_SCREEN(L"testing pksc11...\r\n");

	//load the pkcs11 lib
	LOG(L"PKCS11 load library test\n");
	iReturnCode = EDT_pkcs11FindCards();

	LOG(EDT_LINE_BREAK);
	LOG(L"<EDT_PKCS11LOG_STOP>\n");
	return iReturnCode;
}


int EDT_pkcs11FindCards()
{
#ifdef WIN32
	HMODULE					hpkcs11 = NULL;
#else
	void*					hpkcs11 = NULL;
#endif
	CK_C_GetFunctionList 	pC_GetFunctionList;
	CK_RV 					rv;
	CK_FUNCTION_LIST_PTR 	functions;
	CK_INFO 				info;
	CK_SESSION_HANDLE 		session_handle;
	CK_ULONG				slot_count;
	CK_SLOT_ID_PTR 			slotIds;
	CK_ULONG				slotIdx;

	LOG(L"PKCS11 test\n");

	LOG(L"Opening %s\n",PKCS11_LIB);
	hpkcs11 = dlopen(PKCS11_LIB, RTLD_LAZY); // RTLD_NOW is slower
	if(hpkcs11==NULL)
	{
		LOG_LASTERROR(L"LoadLibrary failed");
		return EDT_ERR_LIBRARY_NOT_FOUND; 
	}

	// get function pointer to C_GetFunctionList
	LOG(L"Getting Function Pointer To C_GetFunctionList\n");
	pC_GetFunctionList = (CK_C_GetFunctionList) dlsym(hpkcs11, "C_GetFunctionList");
	if (pC_GetFunctionList == NULL)
	{
		LOG_LASTERROR(L"GetProcAddress on C_GetFunctionList failed");
		dlclose(hpkcs11);
		LOG_EXIT_ERROR(L"loadPKCS11Lib failed\n");
		return EDT_ERR_NOT_AVAILABLE;
	}

	// invoke C_GetFunctionList
	rv = (*pC_GetFunctionList) (&functions);
	LOG(L"C_GetFunctionList returned 0x%x\n",rv);
	if (rv != CKR_OK)
	{
		LOG_LASTERROR(L"C_GetFunctionList");
		LOG_EXIT_ERROR(L"C_GetFunctionList failed\n");
		return EDT_ERR_INTERNAL;   
	}

	// C_Initialize
	rv = (*functions->C_Initialize) (NULL);
	LOG(L"C_Initialize returned 0x%x\n",rv);
	if (CKR_OK != rv)
	{
		LOG_LASTERROR(L"C_Initialize error\n");
		return EDT_ERR_INTERNAL;    
	}

	// C_GetInfo
	LOG(L"Invoke C_GetInfo\n");
	rv = (*functions->C_GetInfo) (&info);
	if (CKR_OK != rv)
	{
		LOG_LASTERROR(L"C_GetInfo error\n");
		goto finalize;
	}

	LOG(L"library version: %d.%d\n", info.libraryVersion.major, info.libraryVersion.minor);
	LOG(L"PKCS#11 version: %d.%d\n", info.cryptokiVersion.major, info.cryptokiVersion.minor);

	// C_GetSlotList
	rv = (*functions->C_GetSlotList) (0, 0, &slot_count);
	LOG(L"C_GetSlotInfo returned 0x%x\n",rv);
	if (CKR_OK != rv) 
	{
		LOG_LASTERROR(L"C_GetSlotList error\n");
		goto finalize;
	}

	LOG(L"slot count: %d\n", slot_count);
	slotIds = (CK_SLOT_ID_PTR)malloc(slot_count * sizeof(CK_SLOT_INFO));
	rv = (*functions->C_GetSlotList) (CK_FALSE, slotIds, &slot_count);
	LOG(L"C_GetSlotInfo returned 0x%x\n",rv);
	if (CKR_OK != rv) 
	{
		LOG_LASTERROR(L"C_GetSlotList (2) error\n");
		goto finalize;
	}

	for (slotIdx = 0; slotIdx < slot_count; slotIdx++)
	{
		CK_SLOT_INFO 	slotInfo;		
		CK_SLOT_ID 		slotId = slotIds[slotIdx];

		rv = (*functions->C_GetSlotInfo) (slotId, &slotInfo);
		LOG(L"C_GetSlotInfo returned 0x%x\n",rv);
		if (CKR_OK != rv)
		{
			LOG_LASTERROR(L"C_GetSlotInfo error\n");
			goto finalize;
		}

		LOG(L"slot Id: %d\n", slotId);
		LogIncIndent();
		PrintSlotInfo(&slotInfo);

		CK_TOKEN_INFO tokenInfo;

		// Get token information for slot
		rv = (*functions->C_GetTokenInfo)(slotId, &tokenInfo);
		LOG(L"C_GetTokenInfo returned 0x%x\n",rv);
		if ( rv == CKR_TOKEN_NOT_PRESENT)
		{
			LOG_ERRORCODE(L"No Card Found \n",rv);
		}
		else if ( rv != CKR_OK)
		{
			LOG_ERRORCODE(L"C_GetTokenInfo error \n",rv);
		}
		else
		{
			PrintTokenInfo(&tokenInfo);
			CK_ULONG certificate = CKO_CERTIFICATE;
			CK_ATTRIBUTE attributes[1] = {CKA_CLASS,&certificate,sizeof(CK_ULONG)};
			CK_VOID_PTR pLabel = malloc (256);
			CK_ULONG ullabelLen = 255;//last one is for the string termination
			CK_VOID_PTR pValue = malloc (31256);
			CK_ULONG ulvalueLen = 31255;//last one is for the string termination
			CK_OBJECT_CLASS classvalue;
			CK_ULONG ulclassvalueLen = sizeof(CK_ULONG);//last one is for the string termination
			CK_ATTRIBUTE attr_templ[] = {{CKA_LABEL,pLabel,ullabelLen},
			{CKA_CLASS,&classvalue,ulclassvalueLen},
			{CKA_VALUE,pValue,ulvalueLen}};
			CK_OBJECT_HANDLE hObject;
			CK_ULONG ulObjectCount;

			if( (pLabel != NULL) && (pValue != NULL) )
			{				
				rv = (*functions->C_OpenSession)(slotId, CKF_SERIAL_SESSION, NULL_PTR, NULL_PTR, &session_handle);
				LOG(L"C_OpenSession returned 0x%x\n",rv);	
				if (rv == CKR_OK)
				{
					rv = (*functions->C_FindObjectsInit)(session_handle, attributes, 1); 
					LOG(L"C_FindObjectsInit returned 0x%x\n",rv);	
					if (rv == CKR_OK)
					{
						rv = (*functions->C_FindObjects)(session_handle, &hObject,1,&ulObjectCount); 
						LOG(L"C_FindObjects returned 0x%x\n",rv);	
						if (rv == CKR_OK)
						{
							LOG(L"found %d objects\n",ulObjectCount);
							while (ulObjectCount > 0)
							{								
								LOG(L"found object, CK_OBJECT_HANDLE = %d\n",hObject);
								LogIncIndent();
								rv = (*functions->C_GetAttributeValue)(session_handle,hObject,attr_templ,3);
								LOG(L"C_GetAttributeValue returned 0x%x\n",rv);	
								if(rv == CKR_ATTRIBUTE_TYPE_INVALID)//object doesn't have one of the specified attributes
								{
									LOG_ERROR(L"C_GetAttributeValue wrong attribute specified\n");
								}
								else if (rv == CKR_OK)
								{
									LOG(L"key CKA_LABEL: ");
									LOG_BYTE_ARRAY_AS_STRING((CK_BYTE_PTR)(attr_templ[0].pValue), attr_templ[0].ulValueLen);
									LOG(L"key CKA_CLASS = %d\n", classvalue);
									if(classvalue == CKO_CERTIFICATE)
									{
										//LOG(L"key CKA_VALUE: ");
										//LOG_BYTE_ARRAY_AS_STRING((CK_BYTE_PTR)(attr_templ[2].pValue), attr_templ[2].ulValueLen);
										LOG(L"key CKA_VALUE length = %d\n",attr_templ[2].ulValueLen);
									}
									//reset allocated lengths
									attr_templ[0].ulValueLen = ullabelLen;
									attr_templ[1].ulValueLen = ulclassvalueLen;
									attr_templ[2].ulValueLen = ulvalueLen;
								}
								rv = (*functions->C_FindObjects)(session_handle, &hObject,1,&ulObjectCount);
								LOG(L"C_FindObjects returned 0x%x\n",rv);		
								LogDecIndent();
							}
						}
						rv = (*functions->C_FindObjectsFinal)(session_handle); 
						LOG(L"C_FindObjectsFinal returned 0x%x\n",rv);		
					}
					rv = (*functions->C_CloseSession) (session_handle);
					LOG(L"C_CloseSession returned 0x%x\n",rv);
				}
			}
			else //malloc failed
			{
				LOG_ERROR(L"malloc failed");
			}
		}
		LogDecIndent();
	}

	// C_Finalize
finalize:
	rv = (*functions->C_Finalize)(NULL_PTR);
	LOG(L"C_Finalize returned 0x%x\n",rv);
	if (CKR_OK != rv)
	{ 
		LOG_LASTERROR(L"C_Finalize error\n");
		return EDT_ERR_INTERNAL;
	}

	dlclose(hpkcs11);
	return EDT_OK;
}


void PrintSlotInfo(CK_SLOT_INFO *slotInfo)
{
	CK_UTF8CHAR printbuffer[65];

	strncpy_s((char*)printbuffer,65,(const char*)(slotInfo->slotDescription),64);
	LOG(L"slotDescription: %hs\n",printbuffer);
	strncpy_s((char*)printbuffer,65,(const char*)(slotInfo->manufacturerID),32);
	LOG(L"manufacturerID: %hs\n",printbuffer);

	LOG(L"flags 0x%x\n",slotInfo->flags);
	LOG(L"hardwareVersion: %d.%d \n",slotInfo->hardwareVersion.major,slotInfo->hardwareVersion.minor);
	LOG(L"firmwareVersion: %d.%d \n",slotInfo->firmwareVersion.major,slotInfo->firmwareVersion.minor);
}

void PrintTokenInfo(CK_TOKEN_INFO *tokenInfo)
{
	CK_UTF8CHAR printbuffer[33];

	strncpy_s((char*)printbuffer,33,(const char*)(tokenInfo->label),32);
	LOG(L"label: %hs\n",printbuffer);
	strncpy_s((char*)printbuffer,33,(const char*)(tokenInfo->manufacturerID),32);
	LOG(L"manufacturerID: %hs\n",printbuffer);
	strncpy_s((char*)printbuffer,33,(const char*)(tokenInfo->model),16);
	LOG(L"model: %hs\n",printbuffer);
	strncpy_s((char*)printbuffer,33,(const char*)(tokenInfo->serialNumber),16);
	LOG(L"serialNumber: %hs\n",printbuffer);
	LOG(L"flags: 0x%x\n",tokenInfo->flags);

	/* ulMaxSessionCount, ulSessionCount, ulMaxRwSessionCount,
	* ulRwSessionCount, ulMaxPinLen, and ulMinPinLen have all been
	* changed from CK_USHORT to CK_ULONG for v2.0 */
	LOG(L"ulMaxSessionCount: %u\n",tokenInfo->ulMaxSessionCount);
	LOG(L"ulSessionCount: %u\n",tokenInfo->ulSessionCount);
	LOG(L"ulMaxRwSessionCount: %u\n",tokenInfo->ulMaxRwSessionCount);
	LOG(L"ulRwSessionCount: %u\n",tokenInfo->ulRwSessionCount);
	LOG(L"ulMaxPinLen: %u\n",tokenInfo->ulMaxPinLen);
	LOG(L"ulMinPinLen: %u\n",tokenInfo->ulMinPinLen);
	LOG(L"ulTotalPublicMemory: %u\n",tokenInfo->ulTotalPublicMemory);
	LOG(L"ulFreePublicMemory: %u\n",tokenInfo->ulFreePublicMemory);
	LOG(L"ulTotalPrivateMemory: %u\n",tokenInfo->ulTotalPrivateMemory);
	LOG(L"ulFreePrivateMemory: %u\n",tokenInfo->ulFreePrivateMemory);

	/* hardwareVersion, firmwareVersion, and time are new for
	* v2.0 */
	LOG(L"hardwareVersion: %d.%d \n",tokenInfo->hardwareVersion.major,tokenInfo->hardwareVersion.minor);
	LOG(L"firmwareVersion: %d.%d \n",tokenInfo->firmwareVersion.major,tokenInfo->firmwareVersion.minor);

	strncpy_s((char*)printbuffer,33,(const char*)(tokenInfo->utcTime),16);
	LOG(L"utcTime: %hs\n",printbuffer);
}
