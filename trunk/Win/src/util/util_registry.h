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
#ifndef __DIAGLIB_REGISTRY_H__
#define __DIAGLIB_REGISTRY_H__

#include <string>
////////////////////////////////////////////////////////////////////////////////////////////////
// hRootKey values are HKEY_CLASSES_ROOT   HKEY_CURRENT_USER    HKEY_LOCAL_MACHINE    HKEY_USERS 
////////////////////////////////////////////////////////////////////////////////////////////////
int registryGetValue(HKEY hRootKey, const wchar_t *wzKey, const wchar_t *wzName, std::wstring *ValueStr);

#endif //__DIAGLIB_REGISTRY_H__
