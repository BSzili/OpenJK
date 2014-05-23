/*
 * Copyright (C) 2014 Szilárd Biró
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <dll.h>
#include "../game/common_headers.h"

extern "C" intptr_t vmMain( intptr_t command, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6, intptr_t arg7  );
extern "C" void dllEntry( intptr_t ( *syscallptr)( intptr_t arg, ... ) );
extern "C" game_export_t *GetGameAPI( game_import_t *import );

dll_tExportSymbol DLL_ExportSymbols[] =
{
	{(void *)dllEntry,   "dllEntry"  },
	{(void *)vmMain,     "vmMain"    },
	{(void *)GetGameAPI, "GetGameAPI"},
	{0,                  0           }
};

dll_tImportSymbol DLL_ImportSymbols[] =
{
	{0, 0, 0, 0}
};

int DLL_Init(void)
{
	return 1;
}

void DLL_DeInit(void)
{
}
