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
#include "qcommon/q_shared.h"
#include "rd-common/tr_types.h"

#ifdef _CGAME

#include "cgame/cg_public.h"
extern cgameExport_t* GetModuleAPI( int apiVersion, cgameImport_t *import );

#elif defined(_GAME)

#include "game/g_public.h"
extern gameExport_t* GetModuleAPI( int apiVersion, gameImport_t *import );

#elif defined(_UI)

#include "ui/ui_public.h"
extern uiExport_t* GetModuleAPI( int apiVersion, uiImport_t *import );

#else

#error No DLL macro is defined!

#endif


dll_tExportSymbol DLL_ExportSymbols[] =
{
	{(void *)GetModuleAPI, "GetModuleAPI"},
	{0,                    0             }
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
