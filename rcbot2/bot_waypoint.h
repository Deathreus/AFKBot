/*
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
#ifndef __RCBOT_WAYPOINT_H__
#define __RCBOT_WAYPOINT_H__

#include <stdio.h>

#include "bot_base.h"
#include "bot_genclass.h"

class CWaypointVisibilityTable;
class CWaypoint;
class CClient;

class CWaypointAuthorInfo
{
public:
	char szAuthor[32];
	char szModifiedBy[32];
};

class CWaypointHeader
{
public:
	char szFileType[16];
	char szMapName[64];
	int iVersion;
	int iNumWaypoints;
	int iFlags;
};

typedef struct
{
	MyEHandle pEdict; // MyEHandle fixes problems with reused edict slots
	CWaypoint *pWaypoint;
	Vector v_ground;
}edict_wpt_pair_t;

class CWaypointType
{
public:

	CWaypointType(int iBit, const char *szName, const char *szDescription, Color vColour, int iModBits = BITS_MOD_ALL, int iImportance = 0);

	inline const char *GetName() { return m_szName; }
	inline const char *GetDescription() { return m_szDescription; }

	inline bool IsBitsInFlags(int iFlags) { return (iFlags & m_iBit) == m_iBit; }
	inline int GetBits() { return m_iBit; }
	inline void SetMods(int iMods){ m_iMods = iMods; }// input bitmask of mods (32 max)
	inline bool ForMod(int iMod) { return ((1 << iMod)&m_iMods) == (1 << iMod); }
	inline const Color& GetColour() { return m_vColour; }
	inline int GetImportance() { return m_iImportance; }

	bool operator < (CWaypointType *other)
	{
		return m_iImportance < other->GetImportance();
	}

	//virtual void giveTypeToWaypoint ( CWaypoint *pWaypoint );
	//virtual void removeTypeFromWaypoint ( CWaypoint *pWaypoint );

private:
	int m_iMods; // mods allowed
	int m_iBit; // bits used
	char *m_szName; // e.g. "jump"/"ladder"
	char *m_szDescription; // e.g. "will jump here"/"will climb here"
	Color m_vColour;
	int m_iImportance;
};

// This is wrong to what the operator means, but I just wanted a inline way of mixing two colors
inline Color& operator|=(Color &lhs, const Color &rhs)
{
	float fr = (lhs.r()-rhs.r())*0.5;
	float fg = (lhs.g()-rhs.g())*0.5;
	float fb = (lhs.b()-rhs.b())*0.5;

	if(fr < 0)
		fr = 0;
	if(fg < 0)
		fg = 0;
	if(fb < 0)
		fb = 0;

	lhs.SetColor(
		(unsigned char)((rhs.r()*0.5)+fr),
		(unsigned char)((rhs.g()*0.5)+fg),
		(unsigned char)((rhs.b()*0.5)+fb)
	);

	return lhs;
}

class CWaypointTypes
{
public:

	static const int W_FL_NONE = 0;
	static const int W_FL_JUMP = 1;
	static const int W_FL_CROUCH = 2;
	static const int W_FL_UNREACHABLE = 4;
	static const int W_FL_LADDER = 8;
	static const int W_FL_FLAG = 16;
	static const int W_FL_CAPPOINT = 32;
	static const int W_FL_NOBLU = 64;
	static const int W_FL_NOAXIS = 64;
	static const int W_FL_NORED = 128;
	static const int W_FL_NOALLIES = 128;
	static const int W_FL_HEALTH = 256;
	static const int W_FL_OPENS_LATER = 512;
	static const int W_FL_ROCKET_JUMP = 1024;
	static const int W_FL_BOMB_TO_OPEN = 1024; // DOD:S
	static const int W_FL_SNIPER = 2048;
	static const int W_FL_AMMO = 4096;
	static const int W_FL_RESUPPLY = 8192;
	static const int W_FL_BOMBS_HERE = 8192;
	static const int W_FL_SENTRY = 16384;
	static const int W_FL_MACHINEGUN = 16384;
	static const int W_FL_DOUBLEJUMP = 32768;
	static const int W_FL_PRONE = 32768;
	static const int W_FL_TELE_ENTRANCE = 65536;
	static const int W_FL_TELE_EXIT = 131072;
	static const int W_FL_DEFEND = 262144;
	static const int W_FL_AREAONLY = 524288;
	static const int W_FL_ROUTE = 1048576;
	static const int W_FL_WAIT_GROUND = 2097152;
	static const int W_FL_NO_FLAG = 4194304;
	static const int W_FL_COVER_RELOAD = 4194304; // DOD:S only
	static const int W_FL_LIFT = 8388608;
	static const int W_FL_FLAGONLY = 16777216;
	static const int W_FL_FALL = 33554432;
	static const int W_FL_BREAKABLE = 67108864;
	static const int W_FL_SPRINT = 134217728;
	static const int W_FL_TELEPORT_CHEAT = 268435456; // teleports bots to the next waypoint (cheat)
	static const int W_FL_OWNER_ONLY = 536870912; // Only owners of this area can use the waypoint
	//static const int W_FL_ATTACKPOINT = 1073741824; // Tactical waypoint -- each squad will go to different attack points and signal others to go

	static void Setup();

	static void AddType(CWaypointType *type);

	static void PrintInfo(CWaypoint *pWpt, edict_t *pPrintTo, float duration = 6.0f);

	static CWaypointType *GetType(const char *szType);

	static void FreeMemory();

	static CWaypointType *GetTypeByIndex(unsigned int iIndex);

	static unsigned int GetNumTypes();

	static CWaypointType *GetTypeByFlags(int iFlags);

	static void ShowTypesOnConsole(edict_t *pPrintTo);

	static Color GetColour(unsigned int iFlags);

private:
	static std::vector<CWaypointType*> m_Types;
};

typedef struct
{
	float fNextCheck;
	Vector vOrigin;
	bool bVisibleLastCheck;
}wpt_opens_later_t;

class CWaypoint //: public INavigatorNode
{
public:
	static const int WAYPOINT_HEIGHT = 72;
	static const int WAYPOINT_WIDTH = 8;
	static const int PATHWAYPOINT_WIDTH = 4;

	CWaypoint()
	{
		m_thePaths.Init();
		Init();
	}

	CWaypoint(Vector vOrigin, int iFlags = 0, int iYaw = 0, int fRadius = 0)
	{
		m_thePaths.Clear();
		Init();
		m_iFlags = iFlags;
		m_vOrigin = vOrigin;
		m_bUsed = true;
		SetAim(iYaw);
		m_fNextCheckGroundTime = 0;
		m_bHasGround = false;
		m_fRadius = fRadius;
		m_OpensLaterInfo.Clear();
		m_bIsReachable = true;
		m_fCheckReachableTime = 0;
	}

	bool CheckGround();

	inline void SetAim(int iYaw)
	{
		m_iAimYaw = iYaw;
	}

	inline float GetAimYaw()
	{
		return (float)m_iAimYaw;
	}

	inline const Vector GetOrigin()
	{
		return m_vOrigin;
	}

	void Init();

	inline void AddFlag(int iFlag)
	{
		m_iFlags |= iFlag;
	}

	inline void RemoveFlag(int iFlag)
	{
		m_iFlags &= ~iFlag;
	}

	// removes all waypoint flags
	inline void RemoveFlags()
	{
		m_iFlags = 0;
	}

	inline bool HasFlag(int iFlag)
	{
		return (m_iFlags & iFlag) == iFlag;
	}

	inline bool HasSomeFlags(int iFlag)
	{
		return (m_iFlags & iFlag) > 0;
	}

	inline void Move(Vector origin)
	{
		// move to new origin
		m_vOrigin = origin;
	}

	void CheckAreas(edict_t *pActivator);

	// show info to player
	void Info(edict_t *pEdict);

	bool AddPathTo(int iWaypointIndex);
	void RemovePathTo(int iWaypointIndex);

	void AddPathFrom(int iWaypointIndex);
	void RemovePathFrom(int iWaypointIndex);

	bool CheckReachable();

	bool IsPathOpened(Vector vPath);

	inline bool IsUsed()
	{
		return m_bUsed;
	}

	bool Touched(Vector vOrigin, Vector vOffset, float fTouchDist, bool onground = true);

	inline void FreeMapMemory()
	{
		m_thePaths.Clear();
	}

	inline int GetArea() { return m_iArea; }
	inline void SetArea(int area) { m_iArea = area; }

	inline void SetUsed(bool bUsed){ m_bUsed = bUsed; }

	inline void ClearPaths();

	inline float DistanceFrom(CWaypoint *other)
	{
		return DistanceFrom(other->GetOrigin());
	}

	float DistanceFrom(Vector vOrigin);

	int NumPaths();

	int NumPathsToThisWaypoint();
	int GetPathToThisWaypoint(int i);

	int GetPath(int i);

	void Load(FILE *bfp, int iVersion);

	void Save(FILE *bfp);

	inline int GetFlags(){ return m_iFlags; }

	bool ForTeam(int iTeam);

	inline float GetRadius() { return m_fRadius; }

	inline void SetRadius(float fRad) { m_fRadius = fRad; }

	Vector ApplyRadius();

	bool IsAiming(void);

	void Draw(edict_t *pPlayer);

private:
	Vector m_vOrigin;
	// aim of vector (used with certain waypoint types)
	int m_iAimYaw;
	int m_iFlags;
	int m_iArea;
	float m_fRadius;
	// not deleted
	bool m_bUsed;
	// paths to other waypoints
	dataUnconstArray<int> m_thePaths;
	// for W_FL_WAIT_GROUND waypoints
	float m_fNextCheckGroundTime;
	bool m_bHasGround;
	// Update m_iNumPathsTo (For display)
	bool m_bIsReachable;
	float m_fCheckReachableTime;
	dataUnconstArray<int> m_PathsTo; // paths to this waypoint from other waypoints

	dataUnconstArray<wpt_opens_later_t> m_OpensLaterInfo;
};

class CWaypoints
{
public:
	static const int MAX_WAYPOINTS = 1024;
	static const int WAYPOINT_VERSION = 4; // waypoint version 4 add author information

	static const int W_FILE_FL_VISIBILITY = 1;

	static void Init(const char *pszAuthor = NULL, const char *pszModifiedBy = NULL);

	static inline int GetWaypointIndex(CWaypoint *pWpt)
	{
		if (pWpt == NULL)
			return -1;

		return ((int)pWpt - (int)m_theWaypoints) / sizeof(CWaypoint);
	}

	static void AutoFix(bool bAutoFixNonArea);

	static void CheckAreas(edict_t *pActivator);

	static void ShiftVisibleAreas(edict_t *pPlayer, int from, int to);

	static void DrawWaypoints(edict_t *pPlayer);

	static int AddWaypoint(edict_t *pPlayer, const char *type1, const char *type2, const char *type3, const char *type4, bool bUseTemplate = false);
	static int AddWaypoint(edict_t *pPlayer, Vector& vOrigin, int iFlags = CWaypointTypes::W_FL_NONE, bool bAutoPath = false, int iYaw = 0, int iArea = 0, float fRadius = 0);

	static void RemoveWaypoint(int iIndex);

	static int NumWaypoints();

	static CWaypoint *NearestPipeWaypoint(Vector vTarget, Vector vOrigin, int *iAiming);

	static int FreeWaypointIndex();

	static void DeletePathsTo(int iWpt);
	static void DeletePathsFrom(int iWpt);

	static void ShiftAreas(int val);

	static inline CWaypoint *GetWaypoint(int iIndex)
	{
		if (!ValidWaypointIndex(iIndex))
			return NULL;

		return &m_theWaypoints[iIndex];
	}

	static CWaypoint *GetNextCoverPoint(CBot *pBot, CWaypoint *pCurrent, CWaypoint *pBlocking);

	// save waypoints
	static bool Save(bool bVisiblityMade, edict_t *pPlayer = NULL, const char *pszAuthor = NULL, const char *pszModifier = NULL);
	// load waypoints
	static bool Load(const char *szMapName = NULL);

	static inline bool ValidWaypointIndex(int iIndex)
	{
		return ((iIndex >= 0) && (iIndex < m_iNumWaypoints));
	}

	static void DeleteWaypoint(int iIndex);

	static void FreeMemory();

	static int GetClosestFlagged(int iFlags, Vector &vOrigin, int iTeam, float *fReturnDist = NULL, unsigned char *failedwpts = NULL);

	static int NearestWaypointGoal(int iFlags, Vector &origin, float fDist, int iTeam = 0);
	static CWaypoint *RandomRouteWaypoint(CBot *pBot, Vector vOrigin, Vector vGoal, int iTeam, int iArea);
	static CWaypoint *RandomWaypointGoal(int iFlags, int iTeam = 0, int iArea = 0, bool bForceArea = false, CBot *pBot = NULL, bool bHighDanger = false, int iSearchFlags = 0, int iIgnore = -1);
	static CWaypoint *RandomWaypointGoalBetweenArea(int iFlags, int iTeam, int iArea, bool bForceArea, CBot *pBot, bool bHighDanger, Vector *org1, Vector *org2, bool bIgnoreBelief = false, int iWpt1 = -1, int iWpt2 = -1);
	static CWaypoint *RandomWaypointGoalNearestArea(int iFlags, int iTeam, int iArea, bool bForceArea, CBot *pBot, bool bHighDanger, Vector *origin, int iIgnore = -1, bool bIgnoreBelief = false, int iWpt1 = -1);
	static int RandomFlaggedWaypoint(int iTeam = 0);

	static CWaypointVisibilityTable *GetVisiblity() { return m_pVisibilityTable; }
	static void SetupVisibility();
	static CWaypoint *GetPinchPointFromWaypoint(Vector vPlayerOrigin, Vector vPinchOrigin);
	static CWaypoint *GetNestWaypoint(int iTeam, int iArea, bool bForceArea = false, CBot *pBot = NULL);

	static void UpdateWaypointPairs(std::vector<edict_wpt_pair_t> *pPairs, int iWptFlag, const char *szClassname);
	static bool HasAuthor() { return (m_szAuthor[0] != 0); }
	static const char *GetAuthor() { return m_szAuthor; }
	static bool IsModified() { return (m_szModifiedBy[0] != 0); }
	static const char *GetModifier() { return m_szModifiedBy; }
	static const char *GetWelcomeMessage() { return m_szWelcomeMessage; }

	static bool WantToGenerate() { return m_bWantToGenerate; }
	static void WantToGenerate(bool bSet) { m_bWantToGenerate = bSet; }
	static void PrepareGeneration();
	static void ProcessGeneration();
	static void PostProcessGeneration();

private:
	static CWaypoint m_theWaypoints[MAX_WAYPOINTS];
	static int m_iNumWaypoints;
	static float m_fNextDrawWaypoints;
	static int m_iWaypointTexture;
	static CWaypointVisibilityTable *m_pVisibilityTable;
	static char m_szAuthor[32];
	static char m_szModifiedBy[32];
	static char m_szWelcomeMessage[128];
	static bool m_bWantToGenerate;

	typedef struct
	{
		bool bNeedToPostProcess;
		bool bDoneGenerating;
		bool bFirstStageDone;
		bool bSecondStageDone;
		int iCurrentIndex;
		int iMaxIndex;
	}GenData_t;
	static GenData_t m_GenData;
};

#endif