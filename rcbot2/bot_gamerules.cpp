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

void *g_pGameRules;

#define FIND_PROP_SEND(type, type_name) \
	char error[256]; \
	sm_sendprop_info_t info; \
	SendProp *pProp; \
	if (!gamehelpers->FindSendPropInfo("CTFGameRulesProxy", prop, &info)) \
	{ \
		snprintf(error, sizeof(error), "Property \"%s\" not found on the gamerules proxy", prop); \
		return -1; \
	} \
	\
	offset = info.actual_offset; \
	pProp = info.prop; \
	bit_count = pProp->m_nBits; \
	\
	switch (pProp->GetType()) \
	{ \
	case type: \
		{ \
			if (element > 0) \
			{ \
				snprintf(error, sizeof(error), "SendProp %s is not an array. Element %d is invalid.", \
					prop, \
					element); \
				return -1; \
			} \
			break; \
		} \
	case DPT_DataTable: \
		{ \
			FIND_PROP_SEND_IN_SENDTABLE(info, pProp, element, type, type_name); \
			\
			offset += pProp->GetOffset(); \
			bit_count = pProp->m_nBits; \
			break; \
		} \
	default: \
		{ \
			snprintf(error, sizeof(error), "SendProp %s type is not " type_name " (%d != %d)", \
				prop, \
				pProp->GetType(), \
				type); \
			return -1; \
		} \
	} \

#define FIND_PROP_SEND_IN_SENDTABLE(info, pProp, element, type, type_name) \
	char error[256]; \
	SendTable *pTable = pProp->GetDataTable(); \
	if (!pTable) \
	{ \
		snprintf(error, sizeof(error), "Error looking up DataTable for prop %s", prop); \
		return -1; \
	} \
	\
	int elementCount = pTable->GetNumProps(); \
	if (element >= elementCount) \
	{ \
		snprintf(error, sizeof(error), "Element %d is out of bounds (Prop %s has %d elements).", \
			element, \
			prop, \
			elementCount); \
		return -1; \
	} \
	\
	pProp = pTable->GetProp(element); \
	if (pProp->GetType() != type) \
	{ \
		snprintf(error, sizeof(error), "SendProp %s type is not " type_name " ([%d,%d] != %d)", \
			prop, \
			pProp->GetType(), \
			pProp->m_nBits, \
			type); \
		return -1; \
	}

bool CGameRulesObject::GetGameRules(char *error, size_t maxlen)
{
	if (!(g_pGameRules = g_pSDKTools->GetGameRules()))
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
	bool is_unsigned;

	void *pGameRules = GetGameRules();
	if (!pGameRules)
		return -1;

	int elementCount = 1;

	FIND_PROP_SEND(DPT_Int, "integer");
	is_unsigned = ((pProp->GetFlags() & SPROP_UNSIGNED) == SPROP_UNSIGNED);

#if SOURCE_ENGINE == SE_CSS || SOURCE_ENGINE == SE_HL2DM || SOURCE_ENGINE == SE_DODS || SOURCE_ENGINE == SE_TF2 \
	|| SOURCE_ENGINE == SE_SDK2013 || SOURCE_ENGINE == SE_BMS
	if (pProp->GetFlags() & SPROP_VARINT)
	{
		bit_count = sizeof(int) * 8;
	}
#endif
	if (bit_count < 1)
	{
		bit_count = size * 8;
	}

	if (bit_count >= 17)
	{
		return *(int32_t *)((intptr_t)pGameRules + offset);
	}
	else if (bit_count >= 9)
	{
		if (is_unsigned)
		{
			return *(uint16_t *)((intptr_t)pGameRules + offset);
		}
		else
		{
			return *(int16_t *)((intptr_t)pGameRules + offset);
		}
	}
	else if (bit_count >= 2)
	{
		if (is_unsigned)
		{
			return *(uint8_t *)((intptr_t)pGameRules + offset);
		}
		else
		{
			return *(int8_t *)((intptr_t)pGameRules + offset);
		}
	}
	else
	{
		return *(bool *)((intptr_t)pGameRules + offset) ? 1 : 0;
	}

	return -1;
}
