/**
* vim: set ts=4 :
* =============================================================================
* SourceMod
* Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
* =============================================================================
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
* As a special exception, AlliedModders LLC gives you permission to link the
* code of this program (as well as its derivative works) to "Half-Life 2," the
* "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
* by the Valve Corporation.  You must obey the GNU General Public License in
* all respects for all other code used.  Additionally, AlliedModders LLC grants
* this exception to all derivative works.  AlliedModders LLC defines further
* exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
* or <http://www.sourcemod.net/license.php>.
*
* Version: $Id$
*/

#include <server_class.h>
#include "bot_gamerules.h"
#include "..\extension.h"

#ifdef GetClassName
#undef GetClassName
#endif

#ifdef GetProp
#undef GetProp
#endif

void *m_pGameRules;

bool CGameRulesObject::GetGameRules(char *error, size_t maxlen)
{
	m_pGameRules = g_pSDKTools->GetGameRules();
	if (m_pGameRules == NULL)
	{
		snprintf(error, maxlen, "Could not fetch the gamerules entity!");
		return false;
	}
	return true;
}

cell_t CGameRulesObject::GameRules_GetProp(const char *prop, int size, int element)
{
	int offset;
	int bit_count;

	if (!m_pGameRules)
		return 0;

	sm_sendprop_info_t info;
	SendProp *pProp;
	if (!gamehelpers->FindSendPropInfo("CTFGameRulesProxy", prop, &info))
	{
		smutils->LogError(myself, "Property \"%s\" not found on the gamerules proxy", prop);
		return 0;
	}

	offset = info.actual_offset;
	pProp = info.prop;
	bit_count = pProp->m_nBits;

	switch (pProp->GetType())
	{
		case DPT_Int:
		{
			if (element > 0)
			{
				return 0;
			}
			break;
		}
		case DPT_DataTable:
		{
			SendTable *pTable = pProp->GetDataTable();
			if (!pTable)
			{
				return 0;
			}

			int elementCount = pTable->GetNumProps();
			if (element >= elementCount)
			{
				return 0;
			}

			pProp = pTable->GetProp(element);
			if (pProp->GetType() != DPT_Int)
			{
				return 0;
			}

			offset += pProp->GetOffset();
			bit_count = pProp->m_nBits;
			break;
		}
		default:
		{
			return 0;
		}
	}
	bool is_unsigned = ((pProp->GetFlags() & SPROP_UNSIGNED) == SPROP_UNSIGNED);

	if (pProp->GetFlags() & SPROP_VARINT)
	{
		bit_count = sizeof(int) * 8;
	}

	if (bit_count < 1)
	{
		bit_count = size * 8;
	}

	if (bit_count >= 17)
	{
		return *(int32_t *)((intptr_t)m_pGameRules + offset);
	}
	else if (bit_count >= 9)
	{
		if (is_unsigned)
		{
			return *(uint16_t *)((intptr_t)m_pGameRules + offset);
		}
		else
		{
			return *(int16_t *)((intptr_t)m_pGameRules + offset);
		}
	}
	else if (bit_count >= 2)
	{
		if (is_unsigned)
		{
			return *(uint8_t *)((intptr_t)m_pGameRules + offset);
		}
		else
		{
			return *(int8_t *)((intptr_t)m_pGameRules + offset);
		}
	}
	else
	{
		return *(bool *)((intptr_t)m_pGameRules + offset) ? 1 : 0;
	}

	return 0;
}
