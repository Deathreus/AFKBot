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
#include "../extension.h"

#ifdef GetProp
 #undef GetProp
#endif

void *CGameRulesObject::g_pGameRules = NULL;

const char *g_szGameRulesProxy = NULL;

bool CGameRulesObject::GetGameRules(char *error, size_t maxlen)
{
	if(!(g_pGameRules=g_pSDKTools->GetGameRules()))
	{
		ke::SafeStrcpy(error, maxlen, "Could not fetch the gamerules pointer!");
		return false;
	}

	if(!(g_szGameRulesProxy=g_pGameConf->GetKeyValue("GameRulesProxy")))
	{
		ke::SafeStrcpy(error, maxlen, "Could not find GameRulesProxy value in afk.games.txt!");
		return false;
	}

	return true;
}

void *CGameRulesObject::GetGameRules()
{
	if(g_pSDKTools)
	{
		if(g_pGameRules == NULL)
			g_pGameRules = g_pSDKTools->GetGameRules();

		return g_pGameRules;
	}

	return NULL;
}

int32_t CGameRulesObject::GetProperty(const char *prop, int size, int element)
{
	if(!GetGameRules() || !g_szGameRulesProxy || !*g_szGameRulesProxy)
	{
		smutils->LogError(myself, "GameRules lookup failed.");
		return 0;
	}

	sm_sendprop_info_t info;
	if(!gamehelpers->FindSendPropInfo(g_szGameRulesProxy, prop, &info))
	{
		smutils->LogError(myself, "Property \"%s\" not found on the gamerules proxy.", prop);
		return 0;
	}

	int offset = info.actual_offset;
	SendProp *pProp = info.prop;
	int bit_count = pProp->m_nBits;

	switch(pProp->GetType())
	{
		case DPT_Int:
		{
			if(element > 0)
			{
				smutils->LogError(myself, "Property \"%s\" is not an array.", prop);
				return 0;
			}
			break;
		}
		case DPT_DataTable:
		{
			SendTable *pTable = pProp->GetDataTable();
			if(!pTable)
			{
				smutils->LogError(myself, "Error looking up DataTable for prop %s", prop);
				return 0;
			}

			int elementCount = pTable->GetNumProps();
			if(element >= elementCount)
			{
				smutils->LogError(myself, "Element %d is outside of array bounds", element);
				return 0;
			}

			pProp = pTable->GetProp(element);
			if(pProp->GetType() != DPT_Int)
			{
				return 0;
			}

			offset += pProp->GetOffset();
			bit_count = pProp->m_nBits;
			break;
		}
		default:
			return 0;
	}

	bool is_unsigned = ((pProp->GetFlags() & SPROP_UNSIGNED) == SPROP_UNSIGNED);

	if(pProp->GetFlags() & SPROP_VARINT)
	{
		bit_count = sizeof(int) * 8;
	}

	if(bit_count < 1)
	{
		bit_count = size * 8;
	}

	if(bit_count >= 17)
	{
		return *(int32_t *)((intptr_t)g_pGameRules + offset);
	}
	else if(bit_count >= 9)
	{
		if(is_unsigned)
		{
			return *(uint16_t *)((intptr_t)g_pGameRules + offset);
		}
		else
		{
			return *(int16_t *)((intptr_t)g_pGameRules + offset);
		}
	}
	else if(bit_count >= 2)
	{
		if(is_unsigned)
		{
			return *(uint8_t *)((intptr_t)g_pGameRules + offset);
		}
		else
		{
			return *(int8_t *)((intptr_t)g_pGameRules + offset);
		}
	}
	else
	{
		return *(bool *)((intptr_t)g_pGameRules + offset) ? 1 : 0;
	}

	return 0;
}
