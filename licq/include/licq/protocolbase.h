/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LICQ_PROTOCOLBASE_H
#define LICQ_PROTOCOLBASE_H

#ifdef __cplusplus
extern "C" {
#endif

const char* LProto_Name();

const char* LProto_Version();

const char* LProto_PPID();

const char* LProto_ConfigFile();

bool LProto_Init();

unsigned long LProto_SendFuncs();

int LProto_Main();

#ifdef __cplusplus
}
#endif

#endif
