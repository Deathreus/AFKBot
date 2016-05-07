/*
 * ================================================================================
 * AFK Bot Extension
 * Copyright (C) 2015 Chris Moore (Deathreus).  All rights reserved.
 * ================================================================================
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
 */

/*
 *	Attributions & Thanks:
 *	=====================
 *	Cheeseh	-	Built the bot code that all of this runs on
 *	pimpinjuice -	Made the NavMesh parsing code to convert the bot from waypointing
 */

#define NO_FORCE_QUALITY

#include "extension.h"

AFKBot g_AFKBot;

SMEXT_LINK(&g_AFKBot);

SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, edict_t *, char const *);

ICvar *icvar = NULL;
IServerGameClients *gameclients = NULL;
IServerGameEnts *gameents = NULL;

ConVar AFKBotVersion("afkbot_version", SMEXT_CONF_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY, "AFK Bot Version");
ConVar HookTFBot("afkbot_bothook", "0", FCVAR_NONE, "Hook intelligent TF2 bots.");

IGameConfig *g_pGameConf = NULL;

int GiveNamedItem_player_Hook = 0;
int GiveNamedItem_bot_Hook = 0;
int GiveNamedItem_player_Hook_Post = 0;
int GiveNamedItem_bot_Hook_Post = 0;
int ClientPutInServer_Hook = 0;

IForward *g_pForwardOnAFK = NULL;
IForward *g_pForwardOnExitAFK = NULL;

sp_nativeinfo_t g_ExtensionNatives[] =
{
	{ "AFKBot_SetAFK",	AFKBot_SetAFK },
	{ "AFKBot_GetAFK",	AFKBot_GetAFK },
	{ NULL,						 NULL }
};

void Hook_ClientPutInServer(edict_t *pEntity, char const *playername)
{
	if(pEntity->m_pNetworkable)
	{
		CBaseEntity *baseentity = pEntity->m_pNetworkable->GetBaseEntity();
		if(!baseentity)
		{
			return;
		}

		CBasePlayer *player = (CBasePlayer *)baseentity;

		if (HookTFBot.GetBool() && strcmp(pEntity->GetClassName(), "tf_bot") == 0)
		{
			if(GiveNamedItem_bot_Hook == 0)
			{
				GiveNamedItem_bot_Hook = SH_ADD_MANUALVPHOOK(MHook_GiveNamedItem, player, SH_STATIC(Hook_GiveNamedItem), false);
			}

			if(GiveNamedItem_bot_Hook_Post == 0)
			{
				GiveNamedItem_bot_Hook_Post = SH_ADD_MANUALVPHOOK(MHook_GiveNamedItem, player, SH_STATIC(Hook_GiveNamedItem_Post), true);
			}
		} else {
			if(GiveNamedItem_player_Hook == 0)
			{
				GiveNamedItem_player_Hook = SH_ADD_MANUALVPHOOK(MHook_GiveNamedItem, player, SH_STATIC(Hook_GiveNamedItem), false);
			}

			if(GiveNamedItem_player_Hook_Post == 0)
			{
				GiveNamedItem_player_Hook_Post = SH_ADD_MANUALVPHOOK(MHook_GiveNamedItem, player, SH_STATIC(Hook_GiveNamedItem_Post), true);
			}

			if (!HookTFBot.GetBool() && ClientPutInServer_Hook != 0) {
				SH_REMOVE_HOOK_ID(ClientPutInServer_Hook);
				ClientPutInServer_Hook = 0;
			}
		}

		if (ClientPutInServer_Hook != 0) {
			SH_REMOVE_HOOK_ID(ClientPutInServer_Hook);
			ClientPutInServer_Hook = 0;
		}
	}
}

bool AFKBot::SDK_OnLoad(char *error, size_t maxlen, bool late) {

	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("tf2.afk", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			snprintf(error, maxlen, "Could not read tf2.afk.txt: %s\n", conf_error);
		}
		return false;
	}

	int iOffset;
	if (!g_pGameConf->GetOffset("GiveNamedItem", &iOffset))
	{
		snprintf(error, maxlen, "Could not find offset for GiveNamedItem");
		return false;
	} else {
		SH_MANUALHOOK_RECONFIGURE(MHook_GiveNamedItem, iOffset, 0, 0);
		g_pSM->LogMessage(myself, "\"GiveNamedItem\" offset = %d", iOffset);
	}

	// If it's a late load, there might be the chance there are players already on the server. Just
	// check for this and try to hook them instead of waiting for the next player. -- Damizean
	if (late)
	{
		int iMaxClients = playerhelpers->GetMaxClients();
		for (int iClient = 1; iClient <= iMaxClients; iClient++)
		{
			IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(iClient);
			if (pPlayer == NULL || !pPlayer->IsConnected() || !pPlayer->IsInGame())
			{
				continue;
			}

			// Retrieve the edict
			edict_t *pEdict = pPlayer->GetEdict();
			if (pEdict == NULL)
			{
				continue;
			}

			// Retrieve base player
			CBasePlayer *pBasePlayer = (CBasePlayer *)pEdict->m_pNetworkable->GetBaseEntity();
			if (pBasePlayer == NULL)
			{
				continue;
			}

			// Done, hook the BasePlayer
			GiveNamedItem_player_Hook = SH_ADD_MANUALVPHOOK(MHook_GiveNamedItem, pBasePlayer, SH_STATIC(Hook_GiveNamedItem), false);
			
			if (GiveNamedItem_player_Hook != 0)
			{
				break;
			}
		}
	}

	if (GiveNamedItem_player_Hook == 0)
	{
		ClientPutInServer_Hook = SH_ADD_HOOK_STATICFUNC(IServerGameClients, ClientPutInServer, gameclients, Hook_ClientPutInServer, true);
	}

	// Register natives for Pawn
	sharesys->AddNatives(myself, g_ExtensionNatives);
	sharesys->RegisterLibrary(myself, "AFKBot");

	// Create handles
	

	// Create forwards
	g_pForwardOnAFK = g_pForwards->CreateForward("AFKBot_OnAFK", ET_Hook, 2, NULL, Param_Cell, Param_Cell);
	g_pForwardOnExitAFK = g_pForwards->CreateForward("AFKBot_OnExitAFK", ET_Ignore, 2, NULL, Param_Cell, Param_Cell);

	return true;
}

bool AFKBot::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{

	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, gameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

	g_pCVar = icvar;

	ConVar_Register(0, this);

	return true;
}

void AFKBot::SDK_OnUnload()
{
	gameconfs->CloseGameConfigFile(g_pGameConf);

	g_pHandleSys->RemoveType(g_ScriptedItemOverrideHandleType, myself->GetIdentity());

	g_pForwards->ReleaseForward(g_pForwardGiveItem);
	g_pForwards->ReleaseForward(g_pForwardGiveItem_Post);
}

bool AFKBot::SDK_OnMetamodUnload(char *error, size_t maxlen)
{
	if (ClientPutInServer_Hook != 0)
	{
		SH_REMOVE_HOOK_ID(ClientPutInServer_Hook);
		ClientPutInServer_Hook = 0;
	}

	if (GiveNamedItem_player_Hook != 0)
	{
		SH_REMOVE_HOOK_ID(GiveNamedItem_player_Hook);
		GiveNamedItem_player_Hook = 0;
	}

	if (GiveNamedItem_player_Hook_Post != 0)
	{
		SH_REMOVE_HOOK_ID(GiveNamedItem_player_Hook_Post);
		GiveNamedItem_player_Hook_Post = 0;
	}

	return true;
}

bool AFKBot::RegisterConCommandBase(ConCommandBase *pCommand)
{
	META_REGCVAR(pCommand);
	return true;
}

static cell_t TF2Items_GiveNamedItem(IPluginContext *pContext, const cell_t *params)
{
	CBaseEntity *pEntity;
	if ((pEntity = GetCBaseEntityFromIndex(params[1], true)) == NULL)
	{
		return pContext->ThrowNativeError("Client index %d is not valid", params[1]);
	}
	
	TScriptedItemOverride *pScriptedItemOverride = GetScriptedItemOverrideFromHandle(params[2], pContext);
	if (pScriptedItemOverride == NULL)
	{
		return -1;
	}
	
	// Create new script created item object and prepare it.
	CEconItemView hScriptCreatedItem;
	memset(&hScriptCreatedItem, 0, sizeof(CEconItemView));

	// initialize the vtable pointers
	hScriptCreatedItem.m_pVTable = g_pVTable;
	hScriptCreatedItem.m_AttributeList.m_pVTable = g_pVTable_Attributes;
	hScriptCreatedItem.m_NetworkedDynamicAttributesForDemos.m_pVTable = g_pVTable_Attributes;

	char *strWeaponClassname = pScriptedItemOverride->m_strWeaponClassname;
	hScriptCreatedItem.m_iItemDefinitionIndex = pScriptedItemOverride->m_iItemDefinitionIndex;
	hScriptCreatedItem.m_iEntityLevel = pScriptedItemOverride->m_iEntityLevel;
	hScriptCreatedItem.m_iEntityQuality = pScriptedItemOverride->m_iEntityQuality;
	hScriptCreatedItem.m_AttributeList.m_Attributes.CopyArray(pScriptedItemOverride->m_Attributes, pScriptedItemOverride->m_iCount);
	hScriptCreatedItem.m_bInitialized = true;
	
	if (!(pScriptedItemOverride->m_bFlags & PRESERVE_ATTRIBUTES))
	{
		hScriptCreatedItem.m_bDoNotIterateStaticAttributes = true;
	}

#ifndef NO_FORCE_QUALITY
	if (hScriptCreatedItem.m_iEntityQuality == 0 && hScriptCreatedItem.m_iAttributesCount > 0)
	{
		hScriptCreatedItem.m_iEntityQuality = 6;
	}
#endif

	// Call the function.
	CBaseEntity *tempItem = NULL;
	tempItem = SH_MCALL(pEntity, MHook_GiveNamedItem)(strWeaponClassname, 0, &hScriptCreatedItem, ((pScriptedItemOverride->m_bFlags & FORCE_GENERATION) == FORCE_GENERATION));

	if (tempItem == NULL)
	{
		g_pSM->LogError(myself, "---------------------------------------");
		g_pSM->LogError(myself, ">>> szClassname = %s", strWeaponClassname);
		g_pSM->LogError(myself, ">>> iItemDefinitionIndex = %u", hScriptCreatedItem.m_iItemDefinitionIndex);
		g_pSM->LogError(myself, ">>> iEntityQuality = %u", hScriptCreatedItem.m_iEntityQuality);
		g_pSM->LogError(myself, ">>> iEntityLevel = %u", hScriptCreatedItem.m_iEntityLevel);
		g_pSM->LogError(myself, "---------------------------------------");

		for (int i = 0; i < ((hScriptCreatedItem.m_AttributeList.m_Attributes.Count() > 16) ? 0 : hScriptCreatedItem.m_AttributeList.m_Attributes.Count()); i++)
		{
			g_pSM->LogError(myself, ">>> iAttributeDefinitionIndex = %u", hScriptCreatedItem.m_AttributeList.m_Attributes.Element(i).m_iAttributeDefinitionIndex);
			g_pSM->LogError(myself, ">>> flValue = %f", hScriptCreatedItem.m_AttributeList.m_Attributes.Element(i).m_flValue);
			g_pSM->LogError(myself, "---------------------------------------");
		}

		return pContext->ThrowNativeError("Item is NULL. File a bug report if you are sure you set all the data correctly. (Try the FORCE_GENERATION flag.)");
	}

	int entIndex = gamehelpers->EntityToBCompatRef(tempItem);

	// Need to manually fire the forward.
	g_pForwardGiveItem_Post->PushCell(params[1]);
	g_pForwardGiveItem_Post->PushString(strWeaponClassname);
	g_pForwardGiveItem_Post->PushCell(hScriptCreatedItem.m_iItemDefinitionIndex);
	g_pForwardGiveItem_Post->PushCell(hScriptCreatedItem.m_iEntityLevel);
	g_pForwardGiveItem_Post->PushCell(hScriptCreatedItem.m_iEntityQuality);
	g_pForwardGiveItem_Post->PushCell(entIndex);
	g_pForwardGiveItem_Post->Execute(NULL);

	return entIndex;
}

static cell_t AFKBot_SetAFK(IPluginContext *pContext, const cell_t *params)
{
	TScriptedItemOverride *pScriptedItemOverride = GetScriptedItemOverrideFromHandle(params[1], pContext);

	if (pScriptedItemOverride != NULL)
	{
		pScriptedItemOverride->m_iItemDefinitionIndex = params[2];
	}

	return 1;
}

static cell_t AFKBot_GetAFK(IPluginContext *pContext, const cell_t *params)
{
	TScriptedItemOverride *pScriptedItemOverride = GetScriptedItemOverrideFromHandle(params[1], pContext);

	if (pScriptedItemOverride != NULL)
	{
		return pScriptedItemOverride->m_iItemDefinitionIndex;
	}

	return -1;
}

CBaseEntity *GetCBaseEntityFromIndex(int p_iEntity, bool p_bOnlyPlayers)
{
	edict_t *edtEdict = engine->PEntityOfEntIndex(p_iEntity);
	if (!edtEdict || edtEdict->IsFree())
	{
		return NULL;
	}
	
	if (p_iEntity > 0 && p_iEntity <= playerhelpers->GetMaxClients())
	{
		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(edtEdict);

		if (!pPlayer || !pPlayer->IsConnected())
		{
			return NULL;
		}
	} else if (p_bOnlyPlayers) {
		return NULL;
	}

	IServerUnknown *pUnk = edtEdict->GetUnknown();
	if (pUnk == NULL)
	{
		return NULL;
	}

	return pUnk->GetBaseEntity();
}

int GetIndexFromCBaseEntity(CBaseEntity *p_hEntity)
{
	if (p_hEntity == NULL)
	{
		return -1;
	}

	edict_t *edtEdict = gameents->BaseEntityToEdict(p_hEntity);
	if (!edtEdict || edtEdict->IsFree())
	{
		return -1;
	}

	return gamehelpers->IndexOfEdict(edtEdict);
}
