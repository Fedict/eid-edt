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
#ifndef __EDT_UTIL_REGISTRY_H__
#define __EDT_UTIL_REGISTRY_H__

#include <string>

#define EDTREGFLAG_NONE			0x0
#define EDTREGFLAG_EIDMW_ONLY	0x1

////////////////////////////////////////////////////////////////////////////////////////////////
// hRootKey values are HKEY_CLASSES_ROOT   HKEY_CURRENT_USER    HKEY_LOCAL_MACHINE    HKEY_USERS 
////////////////////////////////////////////////////////////////////////////////////////////////
int EDT_UtilReg_LogKeyTree(HKEY hrootKey,const wchar_t* keyName, int flags,int recursion=0);
int EDT_UtilReg_LogValue(HKEY hRootKey, const wchar_t *wzKey, const wchar_t *wzName);
int EDT_UtilReg_LogPermissions(HKEY hRootKey, const wchar_t *wzKey);

#endif //__EDT_UTIL_REGISTRY_H__
