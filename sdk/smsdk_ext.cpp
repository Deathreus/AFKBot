/**
 * vim: set ts=4 sw=4 tw=99 noet:
 * =============================================================================
 * SourceMod Base Extension Code
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

#include <stdio.h>
#include <stdlib.h>
#include "smsdk_ext.h"

/**
 * @file smsdk_ext.cpp
 * @brief Contains wrappers for making Extensions easier to write.
 */

IExtension *myself = NULL;				/**< Ourself */
IShareSys *sharesys = NULL;				/**< Share system */
ISourceMod *smutils = NULL;				/**< SourceMod helpers */

IForwardManager *forwards = NULL;		/**< Forward system */
IPlayerManager *playerhelpers = NULL;	/**< Player helpers */
IGameConfigManager *gameconfs = NULL;	/**< Game config manager */
IGameHelpers *gamehelpers = NULL;
ITimerSystem *timersys = NULL;
IAdminSystem *adminsys = NULL;
ITextParsers *textparsers = NULL;
IUserMessages *usermsgs = NULL;

/** Exports the main interface */
PLATFORM_EXTERN_C IExtensionInterface *GetSMExtAPI()
{
	return g_pExtensionIface;
}

SDKExtension::SDKExtension()
{
	m_SourceMMLoaded = false;
	m_WeAreUnloaded = false;
	m_WeGotPauseChange = false;
}

bool SDKExtension::OnExtensionLoad(IExtension *me, IShareSys *sys, char *error, size_t maxlength, bool late)
{
	sharesys = sys;
	myself = me;

	m_WeAreUnloaded = true;

	if (!m_SourceMMLoaded)
	{
		if (error)
		{
			ke::SafeStrcpy(error, maxlength, "Metamod attach failed");
		}
		return false;
	}

	SM_GET_IFACE(SOURCEMOD, smutils)
	SM_GET_IFACE(FORWARDMANAGER, forwards)
	SM_GET_IFACE(PLAYERMANAGER, playerhelpers)
	SM_GET_IFACE(GAMECONFIG, gameconfs)
	SM_GET_IFACE(GAMEHELPERS, gamehelpers)
	SM_GET_IFACE(TIMERSYS, timersys)
	SM_GET_IFACE(ADMINSYS, adminsys)
	SM_GET_IFACE(TEXTPARSERS, textparsers)
	SM_GET_IFACE(USERMSGS, usermsgs)

	if (SDK_OnLoad(error, maxlength, late))
	{
		m_WeAreUnloaded = true;
		return true;
	}

	return false;
}

bool SDKExtension::IsMetamodExtension()
{
	return true;
}

void SDKExtension::OnExtensionPauseChange(bool state)
{
	m_WeGotPauseChange = true;
	SDK_OnPauseChange(state);
}

void SDKExtension::OnExtensionsAllLoaded()
{
	SDK_OnAllLoaded();
}

void SDKExtension::OnExtensionUnload()
{
	m_WeAreUnloaded = true;
	SDK_OnUnload();
}

void SDKExtension::OnDependenciesDropped()
{
	SDK_OnDependenciesDropped();
}

const char *SDKExtension::GetExtensionAuthor()
{
	return SMEXT_CONF_AUTHOR;
}

const char *SDKExtension::GetExtensionDateString()
{
	return SMEXT_CONF_DATESTRING;
}

const char *SDKExtension::GetExtensionDescription()
{
	return SMEXT_CONF_DESCRIPTION;
}

const char *SDKExtension::GetExtensionVerString()
{
	return SMEXT_CONF_VERSION;
}

const char *SDKExtension::GetExtensionName()
{
	return SMEXT_CONF_NAME;
}

const char *SDKExtension::GetExtensionTag()
{
	return SMEXT_CONF_LOGTAG;
}

const char *SDKExtension::GetExtensionURL()
{
	return SMEXT_CONF_URL;
}

bool SDKExtension::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	return true;
}

void SDKExtension::SDK_OnUnload()
{
}

void SDKExtension::SDK_OnPauseChange(bool paused)
{
}

void SDKExtension::SDK_OnAllLoaded()
{
}

void SDKExtension::SDK_OnDependenciesDropped()
{
}


PluginId g_PLID = 0;						/**< Metamod plugin ID */
ISmmPlugin *g_PLAPI = NULL;					/**< Metamod plugin API */
SourceHook::ISourceHook *g_SHPtr = NULL;	/**< SourceHook pointer */
ISmmAPI *g_SMAPI = NULL;					/**< SourceMM API pointer */

IVEngineServer *engine = NULL;				/**< IVEngineServer pointer */
IServerGameDLL *gamedll = NULL;				/**< IServerGameDLL pointer */
CGlobalVars *gpGlobals = NULL;				/**< CGlobalVars pointer */

/** Exposes the extension to Metamod */
SMM_API void *PL_EXPOSURE(const char *name, int *code)
{
#if defined METAMOD_PLAPI_VERSION
	if (name && !strcmp(name, METAMOD_PLAPI_NAME))
#else
	if (name && !strcmp(name, PLAPI_NAME))
#endif
	{
		if (code)
		{
			*code = META_IFACE_OK;
		}
		return static_cast<void *>(g_pExtensionIface);
	}

	if (code)
	{
		*code = META_IFACE_FAILED;
	}

	return NULL;
}

bool SDKExtension::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS()

#if !defined METAMOD_PLAPI_VERSION
	GET_V_IFACE_ANY(serverFactory, gamedll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_CURRENT(engineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
#else
	GET_V_IFACE_ANY(GetServerFactory, gamedll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
#if SOURCE_ENGINE == SE_CSS || SOURCE_ENGINE == SE_DODS || SOURCE_ENGINE == SE_HL2DM || SOURCE_ENGINE == SE_SDK2013
	// Shim to avoid hooking shims
	engine = (IVEngineServer *) ismm->GetEngineFactory()("VEngineServer023", nullptr);
	if (!engine)
	{
		engine = (IVEngineServer *) ismm->GetEngineFactory()("VEngineServer022", nullptr);
		if (!engine)
		{
			engine = (IVEngineServer *) ismm->GetEngineFactory()("VEngineServer021", nullptr);
			if (!engine)
			{
				if (error && maxlen)
				{
					ke::SafeStrcpy(error, maxlen, "Could not find interface: VEngineServer023 or VEngineServer022");
				}
				return false;
			}
		}
	}
#else
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
#endif // TF2 / CSS / DODS / HL2DM / SDK2013
#endif // !METAMOD_PLAPI_VERSION

	gpGlobals = ismm->GetCGlobals();

	m_SourceMMLoaded = true;

	return SDK_OnMetamodLoad(ismm, error, maxlen, late);
}

bool SDKExtension::Unload(char *error, size_t maxlen)
{
	if (!m_WeAreUnloaded)
	{
		if (error)
		{
			ke::SafeStrcpy(error, maxlen, "This extension must be unloaded by SourceMod.");
		}
		return false;
	}

	return SDK_OnMetamodUnload(error, maxlen);
}

bool SDKExtension::Pause(char *error, size_t maxlen)
{
	if (!m_WeGotPauseChange)
	{
		if (error)
		{
			ke::SafeStrcpy(error, maxlen, "This extension must be paused by SourceMod.");
		}
		return false;
	}

	m_WeGotPauseChange = false;

	return SDK_OnMetamodPauseChange(true, error, maxlen);
}

bool SDKExtension::Unpause(char *error, size_t maxlen)
{
	if (!m_WeGotPauseChange)
	{
		if (error)
		{
			ke::SafeStrcpy(error, maxlen, "This extension must be unpaused by SourceMod.");
		}
		return false;
	}

	m_WeGotPauseChange = false;

	return SDK_OnMetamodPauseChange(false, error, maxlen);
}

const char *SDKExtension::GetAuthor()
{
	return GetExtensionAuthor();
}

const char *SDKExtension::GetDate()
{
	return GetExtensionDateString();
}

const char *SDKExtension::GetDescription()
{
	return GetExtensionDescription();
}

const char *SDKExtension::GetLicense()
{
	return SMEXT_CONF_LICENSE;
}

const char *SDKExtension::GetLogTag()
{
	return GetExtensionTag();
}

const char *SDKExtension::GetName()
{
	return GetExtensionName();
}

const char *SDKExtension::GetURL()
{
	return GetExtensionURL();
}

const char *SDKExtension::GetVersion()
{
	return GetExtensionVerString();
}

bool SDKExtension::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late)
{
	return true;
}

bool SDKExtension::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
	return true;
}

bool SDKExtension::SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlength)
{
	return true;
}

/* Overload a few things to prevent libstdc++ linking */
#if defined __linux__ || defined __APPLE__
extern "C" void __cxa_pure_virtual(void)
{
}

void *operator new(size_t size)
{
	return malloc(size);
}

void *operator new[](size_t size) 
{
	return malloc(size);
}

void operator delete(void *ptr) 
{
	free(ptr);
}

void operator delete[](void * ptr)
{
	free(ptr);
}
#endif