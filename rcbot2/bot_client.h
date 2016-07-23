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

class CBot;
class CWaypoint;
class CBotMenu;

/**** Autowaypoint stuff borrowed from RCBot1 *****/
// Store a vector as short integers and return as 
// normal floats for less space usage.
template <class T>
class CTypeVector
{
public:

	CTypeVector()
	{
		memset(this,0,sizeof(CTypeVector<T>));
	}

	void SetVector ( Vector vVec ) 
	{ 
		m_x = (T)vVec.x;
		m_y = (T)vVec.y;
		m_z = (T)vVec.z;

		m_bVectorSet = TRUE;
	}

	inline Vector GetVector ( void ) const
	{
		return Vector((float)m_x,(float)m_y,(float)m_z);
	}

	inline BOOL IsVectorSet ()
	{
		return m_bVectorSet;
	}

	inline void UnSet ()
	{
		m_bVectorSet = FALSE;
	}
protected:
	T m_x,m_y,m_z;

	BOOL m_bVectorSet;
};

class CAutoWaypointCheck : public CTypeVector<vec_t>
{
public:
	void SetPoint ( Vector vec, int iFlags )
	{
		m_iFlags = iFlags;

		SetVector(vec);
	}

	int GetFlags ()
	{
		return m_iFlags;
	}

	inline void UnSetPoint ()
	{
		m_bVectorSet = FALSE;
		m_iFlags = 0;
	}
private:
	int m_iFlags;
};

class CToolTip
{
public:
	CToolTip ( const char *pszMessage, const char *pszSound = NULL )
	{
		m_pszMessage = pszMessage;
		m_pszSound = pszSound;
	}

	void Send(edict_t *pPlayer);
private:
	const char *m_pszMessage;
	const char *m_pszSound;
};

class CClient
{
public:
	CClient ()
	{
		m_szSteamID = NULL;
		m_pPlayerInfo = NULL;
		m_pDebugBot = NULL;
		m_WaypointCopyType = WPT_COPY_NONE;
		m_pMenu = NULL;
		m_iMenuCommand = -1;
		m_fNextUpdateMenuTime = 0.0f;
		m_iWaypointShowFlags = 0;
		m_iWaypointDrawType = 3;
		m_szSoundToPlay[0] = 0;
		m_iAutoEventWaypoint = 0;
		m_fAutoEventWaypointRadius = 0.0f;
		m_vAutoEventWaypointOrigin = Vector(0,0,0);
		m_bAutoEventWaypointAutoType = false;
		m_iAutoEventWaypointArea = 0;
		m_fNextBotServerMessage = 0;
		m_bSentWelcomeMessage = false;
		m_fSpeed = 0;
		m_fUpdatePos = 0;
		m_bTeleportVectorValid = false;
		m_vTeleportVector = Vector(0,0,0);
		m_fMonitorHighFiveTime = 0;
	}

	void MonitorHighFive ()
	{
		m_fMonitorHighFiveTime = engine->Time() + 5.0f;
	}

	void Init ();

	void SetupMenuCommands ();
	void ResetMenuCommands ();

	void SetTeleportVector ();
	Vector *GetTeleportVector () { if ( m_bTeleportVectorValid ) return &m_vTeleportVector; return NULL; }

	inline bool IsUsingMenu () { return (m_pMenu != NULL); }
	inline void SetCurrentMenu ( CBotMenu *pMenu ) 
	{ 
		m_pMenu = pMenu; 

		if ( pMenu == NULL )
			ResetMenuCommands();
		else
			SetupMenuCommands();
	}
	inline CBotMenu *GetCurrentMenu () { return m_pMenu; }
	inline void SetMenuCommand ( int iCommand ) { m_iMenuCommand = iCommand; }
	inline int GetLastMenuCommand () { return m_iMenuCommand; }
	bool NeedToRenderMenu ();
	void UpdateRenderMenuTime ();

	int AccessLevel ();
	// this player joins with pPlayer edict
	void ClientConnected ( edict_t *pPlayer );
	// this player disconnects
	void ClientDisconnected ();	

	inline void ShowMenu () { m_bShowMenu = true; };

	bool IsUsed ();

	Vector GetOrigin ();

	inline float GetSpeed () { return m_fSpeed; }
	inline Vector GetVelocity () { return m_vVelocity; }

	void SetWaypointCut ( CWaypoint *pWaypoint );
	void SetWaypointCopy (CWaypoint *pWaypoint); 
	void SetEdict ( edict_t *pPlayer );

	edict_t *GetPlayer () { return m_pPlayer; }

	inline bool IsPlayer ( edict_t *pPlayer ) { return m_pPlayer == pPlayer; }

	inline bool IsWaypointOn () { return m_bWaypointOn; }
	inline void SetWaypointOn ( bool bOn ) { m_bWaypointOn = bOn; }
	inline void SetWaypoint ( int iWpt ) { m_iCurrentWaypoint = iWpt; }
	inline int CurrentWaypoint () { return m_iCurrentWaypoint; }

	inline void SetAccessLevel ( int iLev ) { m_iAccessLevel = iLev; }

	inline bool IsAutoPathOn () { return m_bAutoPaths; }
	inline void SetAutoPath ( bool bOn ) { m_bAutoPaths = bOn; }
	inline bool IsPathWaypointOn () { return m_bPathWaypointOn; }
	inline void SetPathWaypoint ( bool bOn ) { m_bPathWaypointOn = bOn; }

	inline int GetWptArea () { return m_iWptArea; }
	inline void SetWptArea ( int area ) { m_iWptArea = area; }	

	inline void SetPathFrom ( int iWpt ) { m_iPathFrom = iWpt; }
	inline void SetPathTo ( int iWpt ) { m_iPathTo = iWpt; }

	inline int GetPathFrom () { return m_iPathFrom; }
	inline int GetPathTo () { return m_iPathTo; }

	void TeleportTo ( Vector vOrigin );

	inline const char *GetSteamID () { return m_szSteamID; }
	const char *GetName ();

	void UpdateCurrentWaypoint ();

	void ClientActive ();

	void SetDebug ( int iLevel, bool bSet ) { if ( bSet ) { m_iDebugLevels |= (1<<iLevel); } else { m_iDebugLevels &= ~(1<<iLevel); } }
	bool IsDebugOn ( int iLevel ) { return (m_iDebugLevels & (1<<iLevel))>0; }
	void ClearDebug ( ) { m_iDebugLevels = 0; }
	bool IsDebugging () { return (m_iDebugLevels != 0); }

	inline void SetDebugBot ( edict_t *pBot ) { m_pDebugBot = pBot; }	
	inline bool IsDebuggingBot ( edict_t *pBot ) { return m_pDebugBot == pBot; }
	inline edict_t *GetDebugBot () { return m_pDebugBot; }

	void Think ();

	inline void SetDrawType ( unsigned short int iType ) { m_iWaypointDrawType = iType; }
	inline unsigned short int GetDrawType () { return m_iWaypointDrawType; }

	inline float GetWptCopyRadius() { return m_fCopyWptRadius; }
	inline int GetWptCopyFlags () { return m_iCopyWptFlags; }
	inline int GetWptCopyArea () { return m_iCopyWptArea; }

	inline eWptCopyType GetWptCopyType () { return m_WaypointCopyType; }

	inline bool IsShowingWaypoint ( int iFlags ) { return (m_iWaypointShowFlags & iFlags) > 0; }
	inline void ShowWaypoints ( int iFlags ) { m_iWaypointShowFlags |= iFlags; }
	inline void DontShowWaypoints ( int iFlags ) { m_iWaypointShowFlags &= ~iFlags; }
	inline bool IsShowingAllWaypoints () { return m_iWaypointShowFlags == 0; }
	inline int GetShowWaypointFlags () { return m_iWaypointShowFlags; }
	void PlaySound ( const char *pszSound );
	inline void SetAutoWaypointMode ( bool mode, bool debug ) 
	{ 
		m_bAutoWaypoint = mode; 
		m_bDebugAutoWaypoint = debug; 
	}
	inline bool AutoWaypointOn () { return m_bAutoWaypoint; }
	void AutoEventWaypoint ( int iType, float fRadius, bool bAtOtherOrigin = false, int iTeam = 0, Vector vOrigin = Vector(0,0,0), bool bIgnoreTeam = false, bool bAutoType = false );
	void GiveMessage(char*msg, float fTime=0.1f);
private:
	edict_t *m_pPlayer;
	// steam id
	char *m_szSteamID;
	// is drawing waypoints ON for this player
	bool m_bWaypointOn;
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
	unsigned short int m_iWaypointDrawType;

	unsigned int m_iDebugLevels;

	IPlayerInfo *m_pPlayerInfo;

	MyEHandle m_pDebugBot;

	float m_fSpeed;
	Vector m_vVelocity;

	bool m_bShowMenu;

	float m_fCopyWptRadius;
	int m_iCopyWptFlags;
	int m_iCopyWptArea;

	vector<int> m_WaypointCutPaths;
	eWptCopyType m_WaypointCopyType;
	// TODO: tooltips queue
	// vector<CToolTip*> tooltips

	float m_fNextPrintDebugInfo;

	// menu stuff
	CBotMenu *m_pMenu;
	int m_iPrevMenu;
	int m_iMenuCommand;

	float m_fNextUpdateMenuTime;

	int m_iWaypointShowFlags; // 0 = showall (default)

	char m_szSoundToPlay[128];

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
	float m_fNextBotServerMessage;
	queue<CToolTip*> m_NextTooltip;
	bool m_bSentWelcomeMessage;

	bool m_bTeleportVectorValid;
	Vector m_vTeleportVector;

	bool m_bIsTeleporting;
	float m_fTeleportTime;

	float m_fMonitorHighFiveTime;
};

class CClients
{
public:
	// called when player joins
	static CClient *ClientConnected ( edict_t *pPlayer );
	static void ClientDisconnected ( edict_t *pPlayer );
	// player starts game
	static void ClientActive ( edict_t *pPlayer );
	// get index in array
	static int SlotOfEdict ( edict_t *pPlayer );
	static void Init ( edict_t *pPlayer );
	static CClient *Get ( int iIndex ) { return &m_Clients[iIndex]; }
	static CClient *Get ( edict_t *pPlayer ) { return &m_Clients[SlotOfEdict(pPlayer)]; }
	static void SetListenServerClient ( CClient *pClient ) { m_pListenServerClient = pClient; }
	static bool IsListenServerClient ( CClient *pClient ) { return m_pListenServerClient == pClient; }
	static bool NoListenServerClient () { return m_pListenServerClient == NULL; }
	static void ClientThink ();
	static bool ClientsDebugging ( int iLev = 0 );
	static void ClientDebugMsg ( int iLev, const char *szMsg, CBot *pBot = NULL );
	static void ClientDebugMsg(CBot *pBot, int iLev, const char *fmt, ... );
	static CClient *FindClientBySteamID ( char *szSteamID );
	static edict_t *GetListenServerClient() { if ( m_pListenServerClient ) return m_pListenServerClient->GetPlayer(); else return NULL; }

	static void Initall () { for ( int i = 1; i <= MAX_PLAYERS; i ++ ) { m_Clients[i].Init(); } }
	static void GiveMessage (char *msg, float fTime = 0.1, edict_t *pPlayer = NULL );// NULL to everyone
private:
	static CClient m_Clients[MAX_PLAYERS];
	static CClient *m_pListenServerClient;
	static bool m_bClientsDebugging;
};
#endif
