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
#ifndef __RCBOT_CLIENT_H__
#define __RCBOT_CLIENT_H__

#include <vector>
using namespace std;

#include "bot_const.h"

#define MAX_STORED_AUTOWAYPOINT 5

typedef enum eWptCopyType
{
	WPT_COPY_NONE = 0,
	WPT_COPY_COPY,
	WPT_COPY_CUT
}eWptCopyType_s;

struct edict_t;
/**** Autowaypoint stuff borrowed from RCBot1 *****/
// Store a vector as short integers and return as 
// normal floats for less space usage.
template <class T>
class CTypeVector
{
public:

	CTypeVector()
	{
		memset(this, 0, sizeof(CTypeVector<T>));
	}

	void SetVector(Vector vVec)
	{
		m_x = (T)vVec.x;
		m_y = (T)vVec.y;
		m_z = (T)vVec.z;

		m_bVectorSet = TRUE;
	}

	inline Vector GetVector(void) const
	{
		return Vector((float)m_x, (float)m_y, (float)m_z);
	}

	inline BOOL IsVectorSet()
	{
		return m_bVectorSet;
	}

	inline void UnSet()
	{
		m_bVectorSet = FALSE;
	}
protected:
	T m_x, m_y, m_z;

	BOOL m_bVectorSet;
};

class CAutoWaypointCheck : public CTypeVector<vec_t>
{
public:
	void SetPoint(Vector vec, int iFlags)
	{
		m_iFlags = iFlags;

		SetVector(vec);
	}

	int getFlags()
	{
		return m_iFlags;
	}

	inline void UnSetPoint()
	{
		m_bVectorSet = FALSE;
		m_iFlags = 0;
	}
private:
	int m_iFlags;
};

class CBot;
class CWaypoint;

class CClient
{
public:
	CClient()
	{
		m_szSteamID = NULL;
		m_pPI = NULL;
		m_pDebugBot = NULL;
		m_WaypointCopyType = WPT_COPY_NONE;
		m_iAutoEventWaypoint = 0;
		m_fAutoEventWaypointRadius = 0.0f;
		m_vAutoEventWaypointOrigin = Vector(0, 0, 0);
		m_bAutoEventWaypointAutoType = false;
		m_iAutoEventWaypointArea = 0;
		m_fSpeed = 0;
		m_fUpdatePos = 0;
		m_bTeleportVectorValid = false;
		m_vTeleportVector = Vector(0, 0, 0);
		m_fMonitorHighFiveTime = 0;
	}

	void MonitorHighFive()
	{
		m_fMonitorHighFiveTime = engine->Time() + 5.0f;
	}

	void Init();

	void SetTeleportVector();
	Vector *GetTeleportVector() { if (m_bTeleportVectorValid) return &m_vTeleportVector; return NULL; }
	// this player joins with pPlayer edict
	void ClientConnected(edict_t *pPlayer);
	// this player disconnects
	void ClientDisconnected();

	bool IsUsed();

	Vector GetOrigin();

	inline float GetSpeed() { return m_fSpeed; }

	inline Vector GetVelocity() { return m_vVelocity; }

	void SetEdict(edict_t *pPlayer);

	edict_t *GetPlayer() { return m_pPlayer; }

	inline bool IsPlayer(edict_t *pPlayer) { return m_pPlayer == pPlayer; }

	void TeleportTo(Vector vOrigin);

	inline const char *GetSteamID() { return m_szSteamID; }
	const char *GetName();

	void ClientActive();

	void Think();

	inline bool AutoWaypointOn() { return m_bAutoWaypoint; }
	void AutoEventWaypoint(int iType, float fRadius, bool bAtOtherOrigin = false, int iTeam = 0, Vector vOrigin = Vector(0, 0, 0), bool bIgnoreTeam = false, bool bAutoType = false);
private:
	edict_t *m_pPlayer;
	// steam id
	char *m_szSteamID;
	// player editing this waypoint
	int m_iCurrentWaypoint;

	int m_iPathFrom;
	int m_iPathTo;

	int m_iAccessLevel;

	Vector m_vLastPos;
	float m_fUpdatePos;

	int m_iWptArea;

	// auto path waypointing
	bool m_bAutoPaths;
	bool m_bPathWaypointOn;

	MyEHandle m_pDebugBot;

	IPlayerInfo *m_pPI;

	float m_fSpeed;
	Vector m_vVelocity;

	float m_fCopyWptRadius;
	int m_iCopyWptFlags;
	int m_iCopyWptArea;

	vector<int> m_WaypointCutPaths;
	eWptCopyType m_WaypointCopyType;

	bool m_bTeleportVectorValid;
	Vector m_vTeleportVector;

	bool m_bIsTeleporting;
	float m_fTeleportTime;

	float m_fMonitorHighFiveTime;

	/**** Autowaypoint stuff below borrowed and converted from RCBot1 ****/
	CAutoWaypointCheck m_vLastAutoWaypointCheckPos[MAX_STORED_AUTOWAYPOINT];

	bool m_bDebugAutoWaypoint;
	bool m_bAutoWaypoint;
	float m_fLastAutoWaypointCheckTime;
	Vector m_vLastAutoWaypointPlacePos;
	bool m_bSetUpAutoWaypoint;
	float m_fCanPlaceJump;
	int m_iLastButtons;

	int m_iLastJumpWaypointIndex;
	int m_iLastLadderWaypointIndex;
	int m_iLastMoveType;
	float m_fCanPlaceLadder;
	int m_iJoinLadderWaypointIndex;

	// new stuff
	int m_iAutoEventWaypoint;
	float m_fAutoEventWaypointRadius;
	Vector m_vAutoEventWaypointOrigin;
	int m_iAutoEventWaypointTeamOn; // waypoint flags to enable for team specific
	int m_iAutoEventWaypointTeamOff;  // waypoint flags to disable for team specific
	int m_iAutoEventWaypointTeam; // the player's team of the waypoint to add
	int m_iAutoEventWaypointArea;
	bool m_bAutoEventWaypointAutoType;
};

class CClients
{
public:
	// called when player joins
	static CClient *ClientConnected(edict_t *pPlayer);
	static void ClientDisconnected(edict_t *pPlayer);
	// player starts game
	static void ClientActive(edict_t *pPlayer);
	// get index in array
	static int SlotOfEdict(edict_t *pPlayer);

	static void Init(edict_t *pPlayer);

	static void ClientThink();

	static CClient *Get(int iIndex) { return &m_Clients[iIndex]; }
	static CClient *Get(edict_t *pPlayer) { return &m_Clients[SlotOfEdict(pPlayer)]; }

	static CClient *FindClientBySteamID(char *szSteamID);

	static void Initall() { for (int i = 1; i <= MAX_PLAYERS; i++) { m_Clients[i].Init(); } }
private:
	static CClient m_Clients[MAX_PLAYERS];
	static CClient *m_pListenServerClient;
	static bool m_bClientsDebugging;
};
#endif
