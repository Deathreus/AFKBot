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
#include <IBinTools.h>

class CUserCmd;
class IMoveHelper;

class AFKBot : 
	public SDKExtension,
	public IConCommandBaseAccessor,
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
		 * @brief Notifies the extension that an external interface it uses is being removed.
		 *
		 * @param pInterface		Pointer to interface being dropped.  This
		 * 							pointer may be opaque, and it should not
		 *							be queried using SMInterface functions unless
		 *							it can be verified to match an existing
		 */
	virtual void NotifyInterfaceDrop(SMInterface *pInterface);

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

public: // ISmmAPI
	void GameFrame(bool);

public: //IConCommandBaseAccessor
	bool RegisterConCommandBase(ConCommandBase *);

public: //IClientListner
	void OnClientPutInServer(int iClient);
	void OnClientDisconnecting(int iClient);

public: //ICommandTargetProcessor
	bool ProcessCommandTarget(cmd_target_info_t *info);

public:
	static void DebugMessage(const char *, ...);
	static void VerboseDebugMessage(const char *, ...);
	static void BeginLogging(bool);
	static void EndLogging(void);

private:
	static FILE* m_pLogFile;
};

extern ICvar *icvar;
extern IFileSystem *filesystem;

extern ConVar bot_enabled;
extern ConVar bot_debug;

extern IGameConfig *g_pGameConf;
extern ISDKTools *g_pSDKTools;
extern IBinTools *g_pBinTools;

extern IForward *forwardOnAFK;

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_