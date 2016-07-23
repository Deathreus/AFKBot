/**
 * =============================================================================
 * TF2 Items Extension
 * Copyright (C) 2009-2010 AzuiSleet, Asher Baker (asherkin).  All rights reserved.
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
 */

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief AFKBot extension code header.
 */

#include "smsdk_ext.h"

#include <ISmmPlugin.h>
#include <sh_vector.h>
#include "rcbot2/engine_wrappers.h"
#include <shareddefs.h>

#include <convar.h>

#include <iplayerinfo.h>
#include <IEngineTrace.h>
#include <filesystem.h>
#include <IEffects.h>
#include <igameevents.h>
#include <engine/ivdebugoverlay.h>
#include <irecipientfilter.h>

#include <IPlayerHelpers.h>
#include <IGameHelpers.h>

#include "ISDKTools.h"

class CUserCmd;
class IMoveHelper;

#if defined WIN32 && !defined snprintf
#define snprintf _snprintf
#endif

/**
 * @brief Sample implementation of the SDK Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */
class AFKBot : public SDKExtension,
	           public IConCommandBaseAccessor,
			   public IMetamodListener
{
public:
	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed loading, false to fail.
	 */
	virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late);
	
	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	virtual void SDK_OnUnload();

	/**
	* @brief This is called once all known extensions have been loaded.
	* Note: It is is a good idea to add natives here, if any are provided.
	*/
	virtual void SDK_OnAllLoaded();

	/**
	 * @brief Called when the pause state is changed.
	 */
	//virtual void SDK_OnPauseChange(bool paused);

	/**
	 * @brief this is called when Core wants to know if your extension is working.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @return			True if working, false otherwise.
	 */
	virtual bool QueryRunning(char *error, size_t maxlen);
public:
#if defined SMEXT_CONF_METAMOD
	/**
	 * @brief Called when Metamod is attached, before the extension version is called.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @param late			Whether or not Metamod considers this a late load.
	 * @return				True to succeed, false to fail.
	 */
	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late);

	/**
	 * @brief Called when Metamod is detaching, after the extension version is called.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	virtual bool SDK_OnMetamodUnload(char *error, size_t maxlen);

	/**
	 * @brief Called when Metamod's pause state is changing.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param paused		Pause state being set.
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlen);
#endif
public: //IConCommandBaseAccessor
	bool RegisterConCommandBase(ConCommandBase *pCommand);

public:
	static CBaseEntity *TF2_GetPlayerWeaponSlot(edict_t *pPlayer, int iSlot);
	static void TF2_EquipWeapon(edict_t *pPlayer, CBaseEntity *pWeapon);

	static void HudTextMessage(edict_t *pEntity, const char *szMessage);
	static void BroadcastTextMessage(const char *szMessage);

public:
	bool LevelInit(char const *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background);
	void LevelShutdown();

	void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax);

	void GameFrame(bool simulating);

	void PlayerRunCmd(CUserCmd *ucmd, IMoveHelper *moveHelper);

	bool ClientConnect(edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen);
	void ClientPutInServer(edict_t *pEntity, char const *playername);
	void ClientDisconnect(edict_t *pEntity);
	void ClientSettingsChanged(edict_t *pEdict);
	void ClientActive(edict_t *pEntity, bool bLoadGame);

#if SOURCE_ENGINE >= SE_ORANGEBOX
	void ClientCommand(edict_t *pEntity, const CCommand &args);
#else
	void ClientCommand(edict_t *pEntity);
#endif

	void SetCommandClient(int index);

	bool FireGameEvent(IGameEvent *pEvent, bool bDontBroadcast);

public:
#if SOURCE_ENGINE == SE_DOTA
	void OnClientCommand(CEntityIndex index, const CCommand &args);
#elif SOURCE_ENGINE >= SE_ORANGEBOX
	void OnClientCommand(edict_t *pEntity, const CCommand &args);
#else
	void OnClientCommand(edict_t *pEntity);
#endif
#if SOURCE_ENGINE == SE_CSS || SOURCE_ENGINE == SE_CSGO
	void OnSendClientCommand(edict_t *pPlayer, const char *szFormat);
#endif
};

extern AFKBot g_AFKBot;

extern CGlobalVars *gpGlobals;

extern IGameEventManager2 *gameevents;
extern IServerPluginCallbacks *vsp_callback;
extern ICvar *icvar;
extern IFileSystem *filesystem;
extern IGameEventManager2 *gameeventmanager2;
extern IGameEventManager *gameeventmanager;
extern IPlayerInfoManager *playerinfomanager;
extern IServerPluginHelpers *helpers;
extern IServerGameClients* gameclients;
extern IEngineTrace *enginetrace;
extern IEffects *effects;
extern IServerGameEnts *servergameents;
extern IVDebugOverlay *debugoverlay;

extern sp_nativeinfo_t g_ExtensionNatives[];

extern ConVar bot_version;
extern ConVar bot_enabled;

static cell_t SetClientAFKBot(IPluginContext *pContext, const cell_t *params);
static cell_t IsClientAFKBot(IPluginContext *pContext, const cell_t *params);

PLUGIN_GLOBALVARS();

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
