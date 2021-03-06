/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Sample Plugin
 * Written by AlliedModders LLC.
 * ======================================================
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software.
 *
 * This sample plugin is public domain.
 */

#ifndef _INCLUDE_SOURCE_ENGINE_WRAPPERS_
#define _INCLUDE_SOURCE_ENGINE_WRAPPERS_

#include <eiface.h>

extern IVEngineServer *engine;
extern CGlobalVars *gpGlobals;

#if SOURCE_ENGINE == SE_EPISODEONE && defined METAMOD_PLAPI_VERSION
#error "Metamod:Source 1.6 API is not supported on the old g_pEngine."
#endif

#define ENGINE_CALL(func) SH_CALL(engine, &IVEngineServer::func)

/**
 * Wrap some API calls for legacy MM:S.
 */
#if !defined METAMOD_PLAPI_VERSION
#define GetEngineFactory engineFactory
#define GetServerFactory serverFactory
#define MM_Format snprintf
#define	GetCGlobals	pGlobals
#else
#define MM_Format g_SMAPI->Format
#endif

#endif //_INCLUDE_SOURCE_ENGINE_WRAPPERS_

