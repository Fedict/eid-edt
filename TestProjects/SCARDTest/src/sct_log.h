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

#pragma once

#include "sct_main.h"

#define LogTrace printf

#define LOG_OPEN_ATTEMPT_COUNT 5


//some macro's for easiness of log use
#define LOG_ENTER() {LogIncIndent();Log(L"Enter \'%hs\'\n",__FUNCTION__);}
#define LOG_EXIT(code) {Log(L"Exit \'%hs\', ReturnValue = %d\n",__FUNCTION__,code);LogDecIndent();}


void LogInitialize(void);
void LogFinalize(void);

inline void Log(const char *format, ...);
//void LOG_BASIC(const char *format, ...);
void LogTime(const char *format, ...);
void LogByteArray(BYTE *byteArray, DWORD arrayLen);
void LogApduSend(BYTE *apdu, DWORD apdulen);//same as LOG_APDU, but prints "SEND" in front
void LogApduResp(BYTE *apdu, DWORD apdulen);//same as LOG_APDU, but prints "RESP" in front

void LogIncIndent();
void LogDecIndent();

