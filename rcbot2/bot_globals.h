/*
 *    part of https://rcbot2.svn.sourceforge.net/svnroot/rcbot2
 *
 *    This file is part of RCBot.
 *
 *    RCBot by Paul Murphy adapted from Botman's HPB Bot 2 template.
 *
 *    RCBot is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    RCBot is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RCBot; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game engine ("HL
 *    engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#ifndef __BOT_GLOBALS_H__
#define __BOT_GLOBALS_H__

#include "bot_mods.h"
#include "bot_const.h"
#include "bot_commands.h"

#include <basetypes.h>

#ifdef _WIN32
#include <ctype.h>
#endif

#define DebugMsg(str, ...) AFKBot::DebugMessage(str, __VA_ARGS__)

inline bool FStrEq(const char *sz1, const char *sz2)
{
	return (sz1 == sz2 || strcasecmp(sz1, sz2) == 0);
}

inline bool FNullEnt(const edict_t* pent)
{
	return (pent == NULL || ENTINDEX(pent) == 0);
}

template<typename T>
inline T Min(const T& a, const T& b)
{
	if(a < b)
		return a;

	return b;
}

template<typename T>
inline T Max(const T& a, const T& b)
{
	if(a > b)
		return a;

	return b;
}

template<typename T>
inline T Clamp(const T& val, const T& min, const T& max)
{
	if(val < min)
		return min;
	else if(val > max)
		return max;
	
	return val;
}

class CBotGlobals
{
public:
	static bool InitModFolder();

	static bool GameStart();

	static QAngle EntityEyeAngles(edict_t *pEntity);

	static QAngle PlayerAngles(edict_t *pPlayer);

	static void FreeMemory();

	static inline bool IsPlayer(edict_t *pEdict)
	{
		return (ENTINDEX(pEdict) > 0 && ENTINDEX(pEdict) < MAX_PLAYERS);
		/*IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(pEdict);

		return (pPlayer != NULL && pPlayer->IsInGame());*/
	}

	static bool WalkableFromTo(edict_t *pPlayer, Vector v_src, Vector v_dest);

	static float YawAngleFromEdict(edict_t *pEntity, Vector vOrigin);
	//static float getAvoidAngle(edict_t *pEdict,Vector origin);

	// make folders for a file if they don't exist
	static bool MakeFolders(char *szFile);
	// just open file but also make folders if possible
	static FILE *OpenFile(char *szFile, char *szMode);
	// get the proper location
	static void BuildFileName(char *szOutput, const char *szFile, const char *szFolder = NULL, const char *szExtension = NULL, bool bModDependent = false);
	// add a directory delimiter to the string like '/' (linux) or '\\' (windows)
	static void AddDirectoryDelimiter(char *szString);
	// print a message to client pEntity with bot formatting
	static void BotMessage(edict_t *pEntity, int iErr, char *fmt, ...);
	static void PrintToChat(int client, const char* fmt, ...);
	static void PrintToChatAll(const char* fmt, ...);
	static void PrintHintText(int client, const char* fmt, ...);
	static void PrintHintTextAll(const char* fmt, ...);

	static void FixFloatAngle(float *fAngle);

	static void NormalizeAngle(QAngle &qAngle);

	static float DotProductFromOrigin(edict_t *pEnemy, Vector pOrigin);
	static float DotProductFromOrigin(Vector vPlayer, Vector vFacing, QAngle eyes);

	static int NumPlayersOnTeam(int iTeam, bool bAliveOnly);
	static inline void SetMapName(string_t iMapName) { m_iMapName = iMapName; }
	static const char *GetMapName() { return STRING(m_iMapName); }

	static bool IsMapRunning() { return m_bMapRunning; }
	static void SetMapRunning(bool bSet) { m_bMapRunning = bSet; }

	static bool IsNetworkable(edict_t *pEntity);

	static inline bool EntityIsValid(edict_t *pEntity)
	{
		return pEntity && !pEntity->IsFree() && pEntity->GetNetworkable() && pEntity->GetIServerEntity();
		//return ( !FNullEnt(pEntity) && !pEntity->IsFree() && pEntity->GetNetworkable() && pEntity->GetIServerEntity() && pEntity->GetCollideable() );
	}

	static void ServerSay(char *fmt, ...);

	static bool IsAlivePlayer(edict_t *pEntity);

	static bool SetWaypointDisplayType(int iType);

	static void FixFloatDegrees360(float *pFloat);

	static edict_t *FindPlayerByTruncName(const char *name);

	// linux fix
	inline static CBotMod *GetCurrentMod()
	{
		return m_pCurrentMod;
	}

	////////////////////////////////////////////////////////////////////////
	// useful functions
	static bool BoundingBoxTouch2d(
		const Vector2D &a1, const Vector2D &a2,
		const Vector2D &bmins, const Vector2D &bmaxs);

	static bool OnOppositeSides2d(
		const Vector2D &amins, const Vector2D &amaxs,
		const Vector2D &bmins, const Vector2D &bmaxs);

	static bool LinesTouching2d(
		const Vector2D &amins, const Vector2D &amaxs,
		const Vector2D &bmins, const Vector2D &bmaxs);

	static bool BoundingBoxTouch3d(
		const Vector &a1, const Vector &a2,
		const Vector &bmins, const Vector &bmaxs);

	static bool OnOppositeSides3d(
		const Vector &amins, const Vector &amaxs,
		const Vector &bmins, const Vector &bmaxs);

	static bool LinesTouching3d(
		const Vector &amins, const Vector &amaxs,
		const Vector &bmins, const Vector &bmaxs);

	static float GrenadeWillLand(Vector vOrigin, Vector vEnemy, float fProjSpeed = 400.0f, float fGrenadePrimeTime = 5.0f, float *fAngle = NULL);
	////////////////////////////////////////////////////////////////////////

	/*static Vector forwardVec ();
	static Vector rightVec ();
	static Vector upVec ();*/
	////////
	static trace_t *GetTraceResult() { return &m_TraceResult; }
	static bool IsVisibleHitAllExceptPlayer(edict_t *pPlayer, Vector vSrc, Vector vDest, edict_t *pDest = NULL);
	static bool IsVisible(edict_t *pPlayer, Vector vSrc, Vector vDest);
	static bool IsVisible(edict_t *pPlayer, Vector vSrc, edict_t *pDest);
	static bool IsShotVisible(edict_t *pPlayer, Vector vSrc, Vector vDest, edict_t *pDest);
	static bool IsVisible(Vector vSrc, Vector vDest);
	static void TraceLine(Vector vSrc, Vector vDest, unsigned int mask, ITraceFilter *pFilter);
	static float QuickTraceline(edict_t *pIgnore, Vector vSrc, Vector vDest); // return fFraction
	static bool TraceVisible(edict_t *pEnt);
	////////
	static inline Vector EntityOrigin(edict_t *pEntity)
	{
		return pEntity->GetIServerEntity()->GetCollideable()->GetCollisionOrigin();
	}
	static int GetTeam(edict_t *pEntity);
	static bool EntityIsAlive(edict_t *pEntity);
	static int CountTeamMatesNearOrigin(Vector vOrigin, float fRange, int iTeam, edict_t *pIgnore = NULL);
	static int NumClients();
	static int NumBots();
	static void LevelInit();

	static inline void SetEventVersion(int iVersion){ m_iEventVersion = iVersion; }

	static inline bool IsEventVersion(int iVersion){ return (m_iEventVersion == iVersion); }

	static inline bool GetTeamplayOn(){ return m_bTeamplay; }

	static inline void SetTeamplay(bool bOn){ m_bTeamplay = bOn; }

	static inline bool IsMod(eModId iMod) { return m_iCurrentMod == iMod; }

	static inline const char *GameFolder(){ return m_szGameFolder; }

	static inline const char *ModFolder(){ return m_szModFolder; }

	static edict_t *PlayerByUserId(int iUserId);

	static edict_t *GetPlayerWeaponSlot(edict_t *pPlayer, int iSlot);

	static bool IsCurrentMod(eModId modid);

	static bool CheckOpensLater(Vector vSrc, Vector vDest);

	inline static bool SetupMapTime() { return m_fMapStartTime == 0; }

	static bool IsBreakableOpen(edict_t *pBreakable);

	static Vector GetVelocity(edict_t *pPlayer);

	static CBotCommandContainer *m_pCommands;

	static void ReadRCBotFolder();

private:
	static eModId m_iCurrentMod;
	static CBotMod *m_pCurrentMod;
	static char *m_szModFolder;
	static char *m_szGameFolder;
	static string_t m_iMapName;
	static int m_iDebugLevels;
	static bool m_bMapRunning;
	static trace_t m_TraceResult;
	static int m_iEventVersion;
	static int m_iWaypointDisplayType;
	static bool m_bTeamplay;
	static float m_fMapStartTime;
	static char *m_szRCBotFolder;
};

#endif