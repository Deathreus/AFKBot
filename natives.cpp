/*
 * ================================================================================
 * AFK Bot Extension
 * Copyright (C) 2016 Chris Moore (Deathreus).  All rights reserved.
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

#include "rcbot2/bot_base.h"
#include "rcbot2/bot_profile.h"

extern IPlayerInfoManager *playerinfomanager;
extern ConVar bot_enabled;
extern ConVar bot_skill_randomize;


 /**
  * Sets the AFK status of the client index.
  *
  * @param client   The client index.
  * @param enabled  True to enable, false to disable
  * @error          The client index is invalid.
  * @noreturn
  */
cell_t Native_SetAFKBot(IPluginContext *pContext, const cell_t *params)
{
	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int userid = playerhelpers->GetClientOfUserId(params[1]);
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(userid > 0 ? userid : params[1]);

	bool bEnable = params[2] == 1;

	if (pPL && pPL->IsInGame() && !pPL->IsFakeClient())
	{
		if (bEnable) CBots::MakeBot(pPL->GetEdict());
		else CBots::MakeNotBot(pPL->GetEdict());
	}
	else
	{
		pContext->ReportErrorNumber(SP_ERROR_INDEX);
		return 0;
	}

	return 1;
}

/**
 * Gets the current AFK status of the client index.
 *
 * @param client  The client index of the player being checked.
 * @return        The clients AFK status.
 * @error         The client index is invalid.
 */
cell_t Native_IsAFKBot(IPluginContext *pContext, const cell_t *params)
{
	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int userid = playerhelpers->GetClientOfUserId(params[1]);
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(userid > 0 ? userid : params[1]);

	if (pPL && pPL->IsInGame() && !pPL->IsFakeClient())
	{
		CBot *pBot = CBots::GetBotPointer(pPL->GetEdict());
		if (pBot && pBot->InUse())
			return 1;
	}
	else
	{
		pContext->ReportErrorNumber(SP_ERROR_INDEX);
	}

	return 0;
}

/**
 * Sets the minimum and maximum aim skill for random skill selection.
 *
 * @param float min   Minimum aiming skill, clamped to 0.0
 * @param float max	  Maximum aiming skill, clamped to 1.0
 * @error             The value is below 0.0 or above 1.0, or skill randomizing is turned off.
 * @noreturn
 */
cell_t Native_SetMinMaxSkill(IPluginContext *pContext, const cell_t *params)
{
	extern ConVar bot_skill_min;
	extern ConVar bot_skill_max;

	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	float fMin = sp_ctof(params[1]);
	if (fMin < 0.0 || fMin > 1.0)
	{
		pContext->ReportError("Minimum skill setting out of bounds, clamping.\nMinimum is 0.0, maximum is 1.0, value is %.1f", fMin);
		fMin = clamp(fMin, 0.0, 1.0);
	}

	float fMax = sp_ctof(params[2]);
	if (fMax < 0.0 || fMax > 1.0)
	{
		pContext->ReportError("Maximum skill setting out of bounds, clamping.\nMinimum is 0.0, maximum is 1.0, value is %.1f", fMax);
		fMax = clamp(fMax, 0.0, 1.0);
	}

	if (bot_skill_randomize.GetBool())
	{
		bot_skill_min.SetValue(fMin);
		bot_skill_max.SetValue(fMax);
	}
	else
	{
		pContext->ReportError("Skill randomization is turned off, ignoring setting.");
		return 0;
	}

	return 1;
}

/**
 * Sets skill level of the bot for the client index.
 *
 * @param int client    Client index
 * @param float skill   Skill level
 * @error               The client index is invalid, or the value is below 0.0 or above 1.0.
 * @noreturn
 */
cell_t Native_SetSkill(IPluginContext *pContext, const cell_t *params)
{
	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int userid = playerhelpers->GetClientOfUserId(params[1]);
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(userid > 0 ? userid : params[1]);

	float fSkill = sp_ctof(params[2]);
	if (fSkill < 0.0 || fSkill > 1.0)
	{
		pContext->ReportError("Skill setting out of bounds, clamping.\nMinimum is 0.0, maximum is 1.0, value is %.1f", fSkill);
		fSkill = clamp(fSkill, 0.0, 1.0);
	}

	if (pPL && pPL->IsConnected() && !pPL->IsFakeClient())
	{
		CBot *pBot = CBots::GetBotPointer(pPL->GetEdict());
		if (!pBot || !pBot->InUse())
		{
			pContext->ReportError("Client %s(%d) is not afk.", pPL->GetName(), params[1]);
			return 0;
		}

		CBotProfile *pBotProfile = pBot->GetProfile();
		pBotProfile->m_fAimSkill = fSkill;
	}
	else
	{
		pContext->ReportError("Invalid client %s(%d)!\nHe is either not connected, a fake client, or in spectator.", pPL->GetName(), params[1]);
		return 0;
	}

	return 1;
}

/**
 * Sets the minimum and maximum braveness factor (How often or how little
 * the bot will try to accomplish something it deems too dangerous) for random skill selection.
 *
 * @param float min    Minimum braveness factor, clamped to 0.0
 * @param float max    Maximum braveness factor, clamped to 1.0
 * @error              The value is below 0.0 or above 1.0
 * @noreturn
 */
cell_t Native_SetMinMaxBraveness(IPluginContext *pContext, const cell_t *params)
{
	extern ConVar bot_braveness_min;
	extern ConVar bot_braveness_max;

	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	float fMin = sp_ctof(params[1]);
	if (fMin < 0.0 || fMin > 1.0)
	{
		pContext->ReportError("Minimum braveness setting out of bounds, clamping\nMinimum is 0.0, maximum is 1.0, value is %.1f", fMin);
		fMin = clamp(fMin, 0.0, 1.0);
	}

	float fMax = sp_ctof(params[2]);
	if (fMax < 0.0 || fMax > 1.0)
	{
		pContext->ReportError("Maximum braveness setting out of bounds, clamping\nMinimum is 0.0, maximum is 1.0, value is %.1f", fMax);
		fMax = clamp(fMax, 0.0, 1.0);
	}

	if (bot_skill_randomize.GetBool())
	{
		bot_braveness_min.SetValue(fMin);
		bot_braveness_max.SetValue(fMax);
	}
	else
	{
		pContext->ReportError("Skill randomization is turned off, ignoring setting.");
		return 0;
	}

	return 1;
}

/**
 * Sets braveness factor for the client index.
 *
 * @param int client        Client index
 * @param float braveness   Braveness factor
 * @error                   The client index is invalid, or the value is below 0.0 or above 1.0.
 * @noreturn
 */
cell_t Native_SetBraveness(IPluginContext *pContext, const cell_t *params)
{
	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int userid = playerhelpers->GetClientOfUserId(params[1]);
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(userid > 0 ? userid : params[1]);

	float fBraveness = sp_ctof(params[2]);
	if (fBraveness < 0.0 || fBraveness > 1.0)
	{
		pContext->ReportError("Braveness setting out of bounds, clamping\nMinimum is 0.0, maximum is 1.0, value is %.1f", fBraveness);
		fBraveness = clamp(fBraveness, 0.0, 1.0);
	}

	if (pPL && pPL->IsConnected() && !pPL->IsFakeClient())
	{
		CBot *pBot = CBots::GetBotPointer(pPL->GetEdict());
		if (!pBot || !pBot->InUse())
		{
			pContext->ReportError("Client %s(%d) is not afk.", pPL->GetName(), params[1]);
			return 0;
		}

		CBotProfile *pBotProfile = pBot->GetProfile();
		pBotProfile->m_fBraveness = fBraveness;
	}
	else
	{
		pContext->ReportErrorNumber(SP_ERROR_INDEX);
		return 0;
	}

	return 1;
}

/**
 * Sets the minimum and maximum turning sensitivty(speed) for random skill selection.
 *
 * The higher this number is the faster he will turn his viewpoint, however,
 * too fast and it will get bouncy(Which is to say, it will bounce back and forth over the
 * target while it tries to resolve the aim vector via smoothing).
 *
 * @param int min    Minimum sensitivty, clamped to 0
 * @param int max    Maximum sensitivty
 * @error            The value is below 0.
 * @noreturn
 */
cell_t Native_SetMinMaxSensitivity(IPluginContext *pContext, const cell_t *params)
{
	extern ConVar bot_sensitivity_min;
	extern ConVar bot_sensitivity_max;

	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int iMin = params[1];
	if (iMin < 0)
	{
		pContext->ReportError("Minimum sensitivity setting out of bounds, clamping\nMinimum is 0, value is %i", params[1]);
		iMin = 0;
	}

	int iMax = params[2];
	if (iMax < 0)
	{
		pContext->ReportError("Maximum sensitivity setting out of bounds, clamping\nMinimum is 0, value is %i", params[2]);
		iMax = 0;
	}

	if (bot_skill_randomize.GetBool())
	{
		bot_sensitivity_min.SetValue(iMin);
		bot_sensitivity_max.SetValue(iMax);
	}
	else
	{
		pContext->ReportError("Skill randomization is turned off, ignoring setting.");
		return 0;
	}

	return 1;
}

/**
 * Sets the turning sensitivty(speed) for the client index.
 *
 * @param int client		Client index
 * @param int sensitivty	Turn speed
 * @error					The client index is invalid, or the value is below 0.
 * @noreturn
 */
cell_t Native_SetSensitivity(IPluginContext *pContext, const cell_t *params)
{
	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int userid = playerhelpers->GetClientOfUserId(params[1]);
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(userid > 0 ? userid : params[1]);

	int iSensitivity = params[2];
	if (iSensitivity < 0)
	{
		pContext->ReportError("Sensitivity setting out of bounds, clamping.\nMinimum is 0, value is %i", params[2]);
		iSensitivity = 0;
	}

	if (pPL && pPL->IsConnected() && !pPL->IsFakeClient())
	{
		CBot *pBot = CBots::GetBotPointer(pPL->GetEdict());
		if (!pBot || !pBot->InUse())
		{
			pContext->ReportError("Client %s(%d) is not afk.", pPL->GetName(), params[1]);
			return 0;
		}

		CBotProfile *pBotProfile = pBot->GetProfile();
		pBotProfile->m_iSensitivity = iSensitivity;
	}
	else
	{
		pContext->ReportErrorNumber(SP_ERROR_INDEX);
		return 0;
	}

	return 1;
}

/**
 * Sets the minimum and maximum revisions the bot tries
 * to do per vision check for random skill selection.
 *
 * This is a bit ambiguous, this is mostly running checks on what the bot can immediately
 * see around him, looking for buildings and NPC's.
 *
 * @param int min   Minimum revisions, clamped to 1
 * @param int max   Maximum revisions, for performance reasons, clamped to 24
 * @error           The value is below 1 or above 24.
 * @noreturn
 */
cell_t Native_SetMinMaxVisRevs(IPluginContext *pContext, const cell_t *params)
{
	extern ConVar bot_visrevs_min;
	extern ConVar bot_visrevs_max;

	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int iMin = params[1];
	if (iMin < 1 || iMin > 24)
	{
		pContext->ReportError("Minimum vis rev setting out of bounds, clamping.\nMinimum is 1, maximum is 24, value is %i", params[1]);
		iMin = clamp(params[1], 1, 24);
	}

	int iMax = params[2];
	if (iMax < 1 || iMax > 24)
	{
		pContext->ReportError("Maximum vis rev setting out of bounds, clamping.\nMinimum is 1, maximum is 24, value is %i", params[2]);
		iMax = clamp(params[2], 1, 24);
	}

	if (bot_skill_randomize.GetBool())
	{
		bot_visrevs_min.SetValue(iMin);
		bot_visrevs_max.SetValue(iMax);
	}
	else
	{
		pContext->ReportError("Skill randomization is turned off, ignoring setting.");
		return 0;
	}

	return 1;
}

/**
 * Sets number of revisions the bot tries
 * to do per vision check for the client index.
 *
 * @param int client     Client index
 * @param int visrevs    Vision revisions
 * @error                The client index is invalid, or the value is below 1 or above 24.
 * @noreturn
 */
cell_t Native_SetVisRevs(IPluginContext *pContext, const cell_t *params)
{
	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int userid = playerhelpers->GetClientOfUserId(params[1]);
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(userid > 0 ? userid : params[1]);

	int iVisRevs = params[2];
	if (iVisRevs < 1 || iVisRevs > 24)
	{
		pContext->ReportError("Visrevs setting out of bounds, clamping.\nMinimum is 1, maximum is 16, value is %i", params[2]);
		iVisRevs = clamp(params[2], 1, 24);
	}

	if (pPL && pPL->IsConnected() && !pPL->IsFakeClient())
	{
		CBot *pBot = CBots::GetBotPointer(pPL->GetEdict());
		if (!pBot || !pBot->InUse())
		{
			pContext->ReportError("Client %s(%d) is not afk.", pPL->GetName(), params[1]);
			return 0;
		}

		CBotProfile *pBotProfile = pBot->GetProfile();
		pBotProfile->m_iVisionTicks = iVisRevs;
	}
	else
	{
		pContext->ReportErrorNumber(SP_ERROR_INDEX);
		return 0;
	}

	return 1;
}

/**
 * Sets the minimum and maximum revisions the bot
 * tries to do per path check for random skill selection
 *
 * @param int min   Minimum revisions, clamped to 1
 * @param int max   Maximum revisions, for performance reasons, clamped to 256
 * @error           The value is below 1 or above 256.
 * @noreturn
 */
cell_t Native_SetMinMaxPathRevs(IPluginContext *pContext, const cell_t *params)
{
	extern ConVar bot_pathrevs_min;
	extern ConVar bot_pathrevs_max;

	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int iMin = params[1];
	if (iMin < 1 || iMin > 256)
	{
		pContext->ReportError("Minimum path rev setting out of bounds, clamping.\nMinimum is 1, maximum is 256, value is %i", params[1]);
		iMin = clamp(params[1], 1, 256);
	}

	int iMax = params[2];
	if (iMax < 1 || iMax > 256)
	{
		pContext->ReportError("Maximum path rev setting out of bounds, clamping.\nMinimum is 1, maximum is 256, value is %i", params[2]);
		iMax = clamp(params[2], 1, 256);
	}

	if (bot_skill_randomize.GetBool())
	{
		bot_pathrevs_min.SetValue(params[1]);
		bot_pathrevs_max.SetValue(params[2]);
	}
	else
	{
		pContext->ReportError("Skill randomization is turned off, ignoring setting.");
		return 0;
	}

	return 1;
}

/**
 * Sets the number of revisions the bot
 * tries to do per path check for the client index.
 *
 * @param int client     Client index
 * @param int pathrevs   Path revisions
 * @error                The client index is invalid, or the value is below 1 or above 256.
 * @noreturn
 */
cell_t Native_SetPathRevs(IPluginContext *pContext, const cell_t *params)
{
	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int userid = playerhelpers->GetClientOfUserId(params[1]);
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(userid > 0 ? userid : params[1]);

	int iPathRevs = params[2];
	if (iPathRevs < 1 || iPathRevs > 256)
	{
		pContext->ReportError("Pathrevs setting out of bounds, clamping.\nMinimum is 1, maximum is 256, value is %i", params[2]);
		iPathRevs = clamp(params[2], 1, 256);
	}

	if (pPL && pPL->IsConnected() && !pPL->IsFakeClient())
	{
		CBot *pBot = CBots::GetBotPointer(pPL->GetEdict());
		if (!pBot || !pBot->InUse())
		{
			pContext->ReportError("Client %s(%d) is not afk.", pPL->GetName(), params[1]);
		}

		CBotProfile *pBotProfile = pBot->GetProfile();
		pBotProfile->m_iPathTicks = iPathRevs;
	}
	else
	{
		pContext->ReportErrorNumber(SP_ERROR_INDEX);
		return 0;
	}

	return 1;
}

/**
 * Sets the minimum and maximum revisions the bot tries
 * to do per vision check for clients for random skill selection.
 *
 * THIS one is what's looking for players, but it puts a higher emphasis on bots.
 *
 * @param int min   Minimum revisions, clamped to 1
 * @param int max   Maximum revisions. for performance reasons, clamped to 16
 * @error           The value is below 1 or above 16.
 * @noreturn
 */
cell_t Native_SetMinMaxClientVisRevs(IPluginContext *pContext, const cell_t *params)
{
	extern ConVar bot_visrevs_client_min;
	extern ConVar bot_visrevs_client_max;

	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int iMin = params[1];
	if (iMin < 1 || iMin > 16)
	{
		pContext->ReportError("Minimum client vis rev setting out of bounds, clamping.\nMinimum is 1, maximum is 16, value is %i", params[1]);
		iMin = clamp(params[1], 1, 16);
	}

	int iMax = params[2];
	if (iMax < 1 || iMax > 16)
	{
		pContext->ReportError("Maximum client vis rev setting out of bounds, clamping.\nMinimum is 1, maximum is 16, value is %i", params[2]);
		iMax = clamp(params[2], 1, 16);
	}

	if (bot_skill_randomize.GetBool())
	{
		bot_visrevs_client_min.SetValue(iMin);
		bot_visrevs_client_max.SetValue(iMax);
	}
	else
	{
		pContext->ReportError("Skill randomization is turned off, ignoring setting.");
		return 0;
	}

	return 1;
}

/**
 * Sets the number of revisions the bot tries
 * to do per vision check for clients for the client index.
 *
 * @param int client		Client index
 * @param int revisions		Vision revisions
 * @error					The client index is invalid, or the value is below 1 or above 16.
 * @noreturn
 */
cell_t Native_SetClientVisRevs(IPluginContext *pContext, const cell_t *params)
{
	if (!bot_enabled.GetBool())
	{
		pContext->ReportError("The bot extension is disabled!");
		return 0;
	}

	int userid = playerhelpers->GetClientOfUserId(params[1]);
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(userid > 0 ? userid : params[1]);

	int iVisRevs = params[2];
	if (iVisRevs < 1 || iVisRevs > 16)
	{
		pContext->ReportError("Clientrevs setting out of bounds, clamping.\nMinimum is 1, maximum is 16, value is %i", params[2]);
		iVisRevs = clamp(params[2], 1, 16);
	}

	if (pPL && pPL->IsConnected() && !pPL->IsFakeClient())
	{
		CBot *pBot = CBots::GetBotPointer(pPL->GetEdict());
		if (!pBot || !pBot->InUse())
		{
			pContext->ReportError("Client %s(%d) is not afk.", pPL->GetName(), params[1]);
			return 0;
		}

		CBotProfile *pBotProfile = pBot->GetProfile();
		pBotProfile->m_iVisionTicksClients = iVisRevs;
	}
	else
	{
		pContext->ReportErrorNumber(SP_ERROR_INDEX);
		return 0;
	}

	return 1;
}

sp_nativeinfo_t g_ExtensionNatives[] = {
	{ "SetClientAFKBot",				Native_SetAFKBot },
	{ "IsClientAFKBot",					Native_IsAFKBot },
	{ "SetAFKBotMinMaxSkill",			Native_SetMinMaxSkill },
	{ "SetClientBotSkill",				Native_SetSkill },
	{ "SetAFKBotMinMaxSensitivity",		Native_SetMinMaxSensitivity },
	{ "SetClientBotSensitivity",		Native_SetSensitivity },
	{ "SetAFKBotMinMaxBraveness",		Native_SetMinMaxBraveness },
	{ "SetClientBotBraveness",			Native_SetBraveness },
	{ "SetAFKBotMinMaxVisRevs",			Native_SetMinMaxVisRevs },
	{ "SetClientBotVisRevs",			Native_SetVisRevs },
	{ "SetAFKBotMinMaxPathRevs",		Native_SetMinMaxPathRevs },
	{ "SetClientBotPathRevs",			Native_SetPathRevs },
	{ "SetAFKBotMinMaxClientVisRevs",	Native_SetMinMaxClientVisRevs },
	{ "SetClientBotClientVisRevs",		Native_SetClientVisRevs },
	{ NULL, 							NULL }
};
