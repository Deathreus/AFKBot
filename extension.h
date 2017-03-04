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

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

#include "sdk/smsdk_ext.h"

#include <convar.h>

#include <iplayerinfo.h>
#include <IEngineSound.h>
#include <IEngineTrace.h>
#include <filesystem.h>
#include <IEffects.h>
#include <igameevents.h>
#include <engine/ivdebugoverlay.h>
#include <irecipientfilter.h>
#include <itoolentity.h>

#include <ISDKTools.h>

class CUserCmd;
class IMoveHelper;

class AFKBot : public SDKExtension,
	public IConCommandBaseAccessor,
	public IGameEventListener2,
	public IClientListener,
	public ICommandTargetProcessor
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

	/**
	 * @brief Called on server activation before plugins receive the OnServerLoad forward.
	 *
	 * @param pEdictList		Edicts list.
	 * @param edictCount		Number of edicts in the list.
	 * @param clientMax			Maximum number of clients allowed in the server.
	 */
	virtual void OnCoreMapStart(edict_t *pEdictList, int edictCount, int clientMax);
	virtual void OnCoreMapEnd();

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

public: // ISmmAPI
	bool LevelInit(char const *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background);
	void LevelShutdown();

	void GameFrame(bool simulating);

	void PlayerRunCmd(CUserCmd *ucmd, IMoveHelper *moveHelper);

	bool FireEvent(IGameEvent *pEvent, bool bDontBroadcast);

public: // IGameEventListener2
	void FireGameEvent(IGameEvent *pEvent);

public: //IConCommandBaseAccessor
	bool RegisterConCommandBase(ConCommandBase *pVar);

public: //IClientListner
	//void OnClientConnected(int iClient);
	void OnClientPutInServer(int iClient);
	void OnClientDisconnected(int iClient);
	//void OnServerActivated(int max_clients);

public:
	#if SOURCE_ENGINE >= SE_ORANGEBOX
		void OnClientCommand(edict_t *pEntity, const CCommand &args);
	#else
		void OnClientCommand(edict_t *pEntity);
	#endif
	#if SOURCE_ENGINE >= SE_ORANGEBOX
		void OnSayCommand(const CCommand &args);
	#else
		void OnSayCommand();
	#endif
#if SOURCE_ENGINE == SE_CSS || SOURCE_ENGINE == SE_CSGO
	void OnSendClientCommand(edict_t *pPlayer, const char *szFormat);
#endif

	void OnSetCommandClient(int client);

public: //ICommandTargetProcessor
	bool ProcessCommandTarget(cmd_target_info_t *info);


private:
	int m_iCommandClient;
};

inline const char *_strstr(const char *str, const char *substr)
{
#ifdef PLATFORM_WINDOWS
	return strstr(str, substr);
#elif defined PLATFORM_LINUX || defined PLATFORM_APPLE
	return (const char *)strstr(str, substr);
#endif
}

inline int strcont(const char *str, const char *substr)
{
	const char *pos = _strstr(str, substr);
	if (pos)
	{
		return (pos - str);
	}

	return -1;
}

extern ICvar *icvar;
extern IFileSystem *filesystem;

extern ConVar bot_enabled;

extern ISDKTools *g_pSDKTools;

extern IForward *forwardOnAFK;

PLUGIN_GLOBALVARS();

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
