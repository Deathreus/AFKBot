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
#ifndef __BOT_MODS_H__
#define __BOT_MODS_H__

#include "bot_const.h"
#include "bot_strings.h"
#include "bot_fortress.h"
//#include "bot_dod_bot.h"
#include "bot_waypoint.h"
#include "bot_tf2_points.h"

#define MAX_CAP_POINTS 32

//#define DOD_MAPTYPE_UNKNOWN 0 
//#define DOD_MAPTYPE_FLAG 1
//#define DOD_MAPTYPE_BOMB 2

#define BOT_ADD_METHOD_DEFAULT 0
#define BOT_ADD_METHOD_PUPPET 1
#define BOT_ADD_PUPPET_COMMAND "bot"

class CBotNeuralNet;

#include <vector>
using namespace std;


/*
		CSS
		TF2
		HL2DM
		HL1DM
		FF
		COOP
		ZOMBIE
		*/
typedef enum
{
	BOTTYPE_GENERIC = 0,
	BOTTYPE_CSS,
	BOTTYPE_TF2,
	BOTTYPE_HL2DM,
	BOTTYPE_HL1DM,
	BOTTYPE_FF,
	BOTTYPE_COOP,
	BOTTYPE_ZOMBIE,
	BOTTYPE_DOD,
	BOTTYPE_NS2,
	BOTTYPE_MAX
}eBotType;

class CBotMod
{
public:
	CBotMod()
	{
		m_szModFolder = NULL;
		m_szSteamFolder = NULL;
		m_szWeaponListName = NULL;
		m_iModId = MOD_UNSUPPORTED;
		m_iBotType = BOTTYPE_GENERIC;
		m_bPlayerHasSpawned = false;
		m_bBotCommand_ResetCheatFlag = false;
		m_bBotCommand_NeedCheatsHack = false;
	}

	virtual bool CheckWaypointForTeam(CWaypoint *pWpt, int iTeam)
	{
		return true; // okay -- no teams!!
	}
	// linux fix
	void Setup(const char *szModFolder, const char *szSteamFolder, eModId iModId, eBotType iBotType, const char *szWeaponListName);

	bool IsSteamFolder(char *szSteamFolder);

	bool IsModFolder(char *szModFolder);

	char *GetSteamFolder();

	char *GetModFolder();

	virtual const char *GetPlayerClass()
	{
		return "CBasePlayer";
	}

	eModId GetModId();

	virtual bool IsAreaOwnedByTeam(int iArea, int iTeam) { return (iArea == 0); }

	eBotType GetBotType() { return m_iBotType; }

	virtual void AddWaypointFlags(edict_t *pPlayer, edict_t *pEdict, int *iFlags, int *iArea, float *fMaxDistance){ return; }

	////////////////////////////////
	virtual void InitMod();

	virtual void MapInit();

	virtual bool PlayerSpawned(edict_t *pPlayer);

	virtual void ClientCommand(edict_t *pEntity, int argc, const char *pcmd, const char *arg1, const char *arg2) {};

	virtual void ModFrame() { };

	virtual void FreeMemory() {};

	virtual bool IsWaypointAreaValid(int iWptArea, int iWptFlags) { return true; }

	virtual void GetTeamOnlyWaypointFlags(int iTeam, int *iOn, int *iOff)
	{
		*iOn = 0;
		*iOff = 0;
	}

	inline bool NeedCheatsHack()
	{
		return m_bBotCommand_NeedCheatsHack;
	}

	inline bool NeedResetCheatFlag()
	{
		return m_bBotCommand_ResetCheatFlag;
	}
private:
	char *m_szModFolder;
	char *m_szSteamFolder;
	eModId m_iModId;
	eBotType m_iBotType;
protected:
	char *m_szWeaponListName;
	bool m_bPlayerHasSpawned;
	bool m_bBotCommand_ResetCheatFlag;
	bool m_bBotCommand_NeedCheatsHack;
};

///////////////////
/*
class CDODFlag
{
public:
CDODFlag()
{
m_pEdict = NULL;
m_iId = -1;
}

void setup (edict_t *pEdict, int id)
{
m_pEdict = pEdict;
m_iId = id;
}

inline bool isFlag ( edict_t *pEdict ) { return m_pEdict == pEdict; }

void update ();

private:
edict_t *m_pEdict;
int m_iId;
};

#define MAX_DOD_FLAGS 8

class CDODFlags
{
public:
CDODFlags()
{
init();
}

void init ()
{
m_iNumControlPoints = 0;
m_vCPPositions = NULL;

m_iAlliesReqCappers = NULL;
m_iAxisReqCappers = NULL;
m_iNumAllies = NULL;
m_iNumAxis = NULL;
m_iOwner = NULL;
m_bBombPlanted_Unreliable = NULL;
m_iBombsRequired = NULL;
m_iBombsRemaining = NULL;
m_bBombBeingDefused = NULL;
m_iNumAxisBombsOnMap = 0;
m_iNumAlliesBombsOnMap = 0;
memset(m_bBombPlanted,0,sizeof(bool)*MAX_DOD_FLAGS);
memset(m_pFlags,0,sizeof(edict_t*)*MAX_DOD_FLAGS);
memset(m_pBombs,0,sizeof(edict_t*)*MAX_DOD_FLAGS*2);

for ( short int i = 0; i < MAX_DOD_FLAGS; i ++ )
{
m_iWaypoint[i] = -1;
}
}

int getNumFlags () { return m_iNumControlPoints; }
int getNumFlagsOwned (int iTeam)
{
int count = 0;

for ( short int i = 0; i < m_iNumControlPoints; i ++ )
{
if ( m_iOwner[i] == iTeam )
count++;
}

return count;
}

int setup (edict_t *pResourceEntity);

bool getRandomEnemyControlledFlag ( CBot *pBot, Vector *position, int iTeam, int *id = NULL );
bool getRandomTeamControlledFlag ( CBot *pBot, Vector *position, int iTeam, int *id = NULL );

bool getRandomBombToDefuse ( Vector *position, int iTeam, edict_t **pBombTarget, int *id = NULL );
bool getRandomBombToPlant ( CBot *pBot, Vector *position, int iTeam, edict_t **pBombTarget, int *id = NULL );
bool getRandomBombToDefend ( CBot *pBot, Vector *position, int iTeam, edict_t **pBombTarget, int *id = NULL );

int findNearestObjective ( Vector vOrigin );

inline int getWaypointAtFlag ( int iFlagId )
{
return m_iWaypoint[iFlagId];
}

inline int getNumBombsToDefend ( int iTeam )
{
int count = 0;

for ( short int i = 0; i < m_iNumControlPoints; i ++ )
{
if ( canDefendBomb(iTeam,i) )
count++;
}

return count;
}

inline int getNumBombsToDefuse ( int iTeam )
{
int count = 0;

for ( short int i = 0; i < m_iNumControlPoints; i ++ )
{
if ( canDefuseBomb(iTeam,i) )
count++;
}

return count;
}

inline int getNumPlantableBombs (int iTeam)
{
int count = 0;

for ( short int i = 0; i < m_iNumControlPoints; i ++ )
{
if ( canPlantBomb(iTeam,i) )
count += getNumBombsRequired(i);
}

return count;
}

inline float isBombExplodeImminent ( int id )
{
return (engine->Time() - m_fBombPlantedTime[id]) > DOD_BOMB_EXPLODE_IMMINENT_TIME;
}

inline void setBombPlanted ( int id, bool val )
{
m_bBombPlanted[id] = val;

if ( val )
m_fBombPlantedTime[id] = engine->Time();
else
m_fBombPlantedTime[id] = 0;
}

inline int getNumBombsToPlant ( int iTeam)
{
int count = 0;

for ( short int i = 0; i < m_iNumControlPoints; i ++ )
{
if ( canPlantBomb(iTeam,i) )
count += getNumBombsRemaining(i);
}

return count;
}

inline bool ownsFlag ( edict_t *pFlag, int iTeam ) { return ownsFlag(getFlagID(pFlag),iTeam); }
inline bool ownsFlag ( int iFlag, int iTeam )
{
if ( iFlag == -1 )
return false;

return m_iOwner[iFlag] == iTeam;
}

inline int numFlagsOwned (int iTeam)
{
int count = 0;

for ( short int i = 0; i < m_iNumControlPoints; i ++ )
{
if ( m_iOwner[i] == iTeam )
count++;
}

return count;
}

inline int numCappersRequired ( edict_t *pFlag, int iTeam ) { return numCappersRequired(getFlagID(pFlag),iTeam); }
inline int numCappersRequired ( int iFlag, int iTeam )
{
if ( iFlag == -1 )
return 0;

return (iTeam == TEAM_ALLIES) ? (m_iAlliesReqCappers[iFlag]) : (m_iAxisReqCappers[iFlag]);
}

inline bool isBombPlanted ( int iId )
{
if ( iId == -1 )
return false;

return m_bBombPlanted[iId];
}

inline bool isBombPlanted ( edict_t *pBomb )
{
return isBombPlanted(getBombID(pBomb));
}

inline bool canDefendBomb ( int iTeam, int iId )
{
return ((m_pBombs[iId][0]!=NULL)&&(m_iOwner[iId]!=iTeam) && isBombPlanted(iId));
}

inline bool canDefuseBomb ( int iTeam, int iId )
{
return ((m_pBombs[iId][0]!=NULL)&&(m_iOwner[iId]==iTeam) && isBombPlanted(iId));
}

inline bool canPlantBomb ( int iTeam, int iId )
{
return ((m_pBombs[iId][0]!=NULL)&&(m_iOwner[iId]!=iTeam) && !isBombPlanted(iId));
}

bool isTeamMateDefusing ( edict_t *pIgnore, int iTeam, int id );
bool isTeamMatePlanting ( edict_t *pIgnore, int iTeam, int id );

bool isTeamMateDefusing ( edict_t *pIgnore, int iTeam, Vector vOrigin );
bool isTeamMatePlanting ( edict_t *pIgnore, int iTeam, Vector vOrigin );

inline int getNumBombsRequired ( int iId )
{
if ( iId == -1 )
return false;

return m_iBombsRequired[iId];
}

inline int getNumBombsRequired ( edict_t *pBomb )
{
return getNumBombsRequired(getBombID(pBomb));
}

inline int getNumBombsRemaining ( int iId )
{
if ( iId == -1 )
return false;

return m_iBombsRemaining[iId];
}

inline int getNumBombsRemaining ( edict_t *pBomb )
{
return getNumBombsRemaining(getBombID(pBomb));
}

inline bool isBombBeingDefused ( int iId )
{
if ( iId == -1 )
return false;

return m_bBombBeingDefused[iId];
}

inline bool isBombBeingDefused ( edict_t *pBomb )
{
return isBombBeingDefused(getBombID(pBomb));
}

inline int numEnemiesAtCap ( edict_t *pFlag, int iTeam ) { return numEnemiesAtCap(getFlagID(pFlag),iTeam); }

inline int numFriendliesAtCap ( edict_t *pFlag, int iTeam ) { return numFriendliesAtCap(getFlagID(pFlag),iTeam); }

inline int numFriendliesAtCap ( int iFlag, int iTeam )
{
if ( iFlag == -1 )
return 0;

return (iTeam == TEAM_ALLIES) ? (m_iNumAllies[iFlag]) : (m_iNumAxis[iFlag]);
}

inline int numEnemiesAtCap ( int iFlag, int iTeam )
{
if ( iFlag == -1 )
return 0;

return (iTeam == TEAM_ALLIES) ? (m_iNumAxis[iFlag]) : (m_iNumAllies[iFlag]);
}

inline edict_t *getFlagByID ( int id )
{
if ( (id >= 0) && (id < m_iNumControlPoints) )
return m_pFlags[id];

return NULL;
}

inline int getFlagID ( edict_t *pent )
{
for ( short int i = 0; i < m_iNumControlPoints; i ++ )
{
if ( m_pFlags[i] == pent )
return i;
}

return -1;
}

inline int getBombID ( edict_t *pent )
{
if ( pent == NULL )
return -1;

for ( short int i = 0; i < m_iNumControlPoints; i ++ )
{
if ( (m_pBombs[i][0] == pent) || (m_pBombs[i][1] == pent) )
return i;
}

return -1;
}

inline bool isFlag ( edict_t *pent )
{
return getFlagID(pent) != -1;
}

inline bool isBomb ( edict_t *pent )
{
return getBombID(pent) != -1;
}

inline int getNumBombsOnMap ( int iTeam )
{
if ( iTeam == TEAM_ALLIES )
return m_iNumAlliesBombsOnMap;
return m_iNumAxisBombsOnMap;
}

inline void reset ()
{
// time up
m_iNumControlPoints = 0;
}

private:
edict_t *m_pFlags[MAX_DOD_FLAGS];
edict_t *m_pBombs[MAX_DOD_FLAGS][2]; // maximum of 2 bombs per capture point
int m_iWaypoint[MAX_DOD_FLAGS];

int m_iNumControlPoints;
Vector *m_vCPPositions;

int *m_iAlliesReqCappers;
int *m_iAxisReqCappers;
int *m_iNumAllies;
int *m_iNumAxis;
int *m_iOwner;

// reply on this one
bool *m_bBombPlanted_Unreliable;
bool m_bBombPlanted[MAX_DOD_FLAGS];
float m_fBombPlantedTime[MAX_DOD_FLAGS];
int *m_iBombsRequired;
int *m_iBombsRemaining;
bool *m_bBombBeingDefused;
int m_iNumAlliesBombsOnMap;
int m_iNumAxisBombsOnMap;
};

class CDODMod : public CBotMod
{
public:
CDODMod()
{
setup("dod","day of defeat source",MOD_DOD,BOTTYPE_DOD,"DOD");

m_bBotCommand_NeedCheatsHack = false;
}

static void roundStart ();

bool checkWaypointForTeam(CWaypoint *pWpt, int iTeam);

static int numClassOnTeam( int iTeam, int iClass );

static int getScore(edict_t *pPlayer);

static int getHighestScore ();

void clientCommand ( edict_t *pEntity, int argc, const char *pcmd, const char *arg1, const char *arg2 );

static float getMapStartTime ();

inline static bool isBombMap () { return (m_iMapType & DOD_MAPTYPE_BOMB) == DOD_MAPTYPE_BOMB; }
inline static bool isFlagMap () { return (m_iMapType & DOD_MAPTYPE_FLAG) == DOD_MAPTYPE_FLAG; }
inline static bool mapHasBombs () { return (m_iMapType & DOD_MAPTYPE_BOMB) == DOD_MAPTYPE_BOMB; }

inline static bool isCommunalBombPoint () { return m_bCommunalBombPoint; }
inline static int getBombPointArea (int iTeam) { if ( iTeam == TEAM_ALLIES ) return m_iBombAreaAllies; return m_iBombAreaAxis; }

void addWaypointFlags (edict_t *pPlayer, edict_t *pEdict, int *iFlags, int *iArea, float *fMaxDistance );

static CDODFlags m_Flags;

static bool shouldAttack ( int iTeam ); // uses the neural net to return probability of attack

static edict_t *getBombTarget ( CWaypoint *pWpt );
static edict_t *getBreakable ( CWaypoint *pWpt );

void getTeamOnlyWaypointFlags ( int iTeam, int *iOn, int *iOff );

static bool isBreakableRegistered ( edict_t *pBreakable, int iTeam );

static inline CWaypoint *getBombWaypoint ( edict_t *pBomb )
{
for ( unsigned int i = 0; i < m_BombWaypoints.size(); i ++ )
{
if ( m_BombWaypoints[i].pEdict == pBomb )
return m_BombWaypoints[i].pWaypoint;
}

return NULL;
}

static inline bool isPathBomb ( edict_t *pBomb )
{
for ( unsigned int i = 0; i < m_BombWaypoints.size(); i ++ )
{
if ( m_BombWaypoints[i].pEdict == pBomb )
return true;
}

return false;
}

// for getting the ground of bomb to open waypoints
// the ground might change
static Vector getGround ( CWaypoint *pWaypoint );

//to do for snipers and machine gunners
/*static unsigned short int getNumberOfClassOnTeam ( int iClass );
static unsigned short int getNumberOfPlayersOnTeam ( int iClass );

protected:

void initMod ();

void mapInit ();

void modFrame ();

void freeMemory ();

static edict_t *m_pResourceEntity;
static edict_t *m_pPlayerResourceEntity;
static edict_t *m_pGameRules;
static float m_fMapStartTime;
static int m_iMapType;
static bool m_bCommunalBombPoint; // only one bomb suuply point for both teams
static int m_iBombAreaAllies;
static int m_iBombAreaAxis;

static vector<edict_wpt_pair_t> m_BombWaypoints;
static vector<edict_wpt_pair_t> m_BreakableWaypoints;

// enemy			// team
static float fAttackProbLookUp[MAX_DOD_FLAGS+1][MAX_DOD_FLAGS+1];
};

class CDODModDedicated : public CDODMod
{
public:
CDODModDedicated()
{
setup("dod", "source dedicated server", MOD_DOD, BOTTYPE_DOD, "DOD");
}
protected:

};

class CCounterStrikeSourceMod : public CBotMod
{
public:
CCounterStrikeSourceMod()
{
setup("cstrike", "counter-strike source", MOD_CSS, BOTTYPE_CSS, "CSS");
}

//void initMod ();

//void mapInit ();

//void entitySpawn ( edict_t *pEntity );
protected:
// storing mod specific info
vector<edict_t*> m_pHostages;
vector<edict_t*> m_pBombPoints;
vector<edict_t*> m_pRescuePoints;
};


class CCounterStrikeSourceModDedicated : public CCounterStrikeSourceMod
{
public:
CCounterStrikeSourceModDedicated()
{
setup("cstrike","source dedicated server",MOD_CSS,BOTTYPE_CSS,"CSS");
}

//void initMod ();

//void mapInit ();

//void entitySpawn ( edict_t *pEntity );
};

class CTimCoopMod : public CBotMod
{
public:
CTimCoopMod()
{
setup("SourceMods","timcoop",MOD_TIMCOOP,BOTTYPE_COOP,"HL2DM");
}

//void initMod ();

//void mapInit ();

//void entitySpawn ( edict_t *pEntity );
};

class CSvenCoop2Mod : public CBotMod
{
public:
CSvenCoop2Mod()
{
setup("SourceMods","svencoop2",MOD_SVENCOOP2,BOTTYPE_COOP,"SVENCOOP2");
}

//void initMod ();

//void mapInit ();

//void entitySpawn ( edict_t *pEntity );
};

class CFortressForeverMod : public CBotMod
{
public:
CFortressForeverMod()
{
setup("FortressForever", "SourceMods", MOD_FF, BOTTYPE_FF, "FF");
}
private:

};

class CFortressForeverModDedicated : public CBotMod
{
public:
CFortressForeverModDedicated()
{
setup("FortressForever","source dedicated server",MOD_FF,BOTTYPE_FF,"FF");
}
private:

};

class CHLDMSourceMod : public CBotMod
{
public:
CHLDMSourceMod()
{
setup("hl1mp","half-life deathmatch source",MOD_HL1DMSRC,BOTTYPE_HL1DM,"HLDMSRC");
}
};

class CSynergyMod : public CBotMod
{
public:
CSynergyMod()
{
setup("synergy","synergy",MOD_SYNERGY,BOTTYPE_COOP,"SYNERGY");
}

//void initMod ();

//void mapInit ();

//void entitySpawn ( edict_t *pEntity );
}; */

#define NEWENUM typedef enum {

typedef enum
{
	TF_MAP_DM = 0,
	TF_MAP_CTF,
	TF_MAP_CP,
	TF_MAP_TC,
	TF_MAP_CART,
	TF_MAP_CARTRACE,
	TF_MAP_ARENA,
	TF_MAP_KOTH,
	TF_MAP_SD, // special delivery : added 15 jul 12
	TF_MAP_TR,
	TF_MAP_MVM,
	TF_MAP_RD,
	TF_MAP_BUMPERCARS,
	TF_MAP_MAX
}eTFMapType;

// These must be MyEHandles because they may be destroyed at any time
typedef struct
{
	MyEHandle entrance;
	MyEHandle exit;
	MyEHandle sapper;
	float m_fLastTeleported;
	int m_iWaypoint;
	//	short builder;
}tf_tele_t;

typedef struct
{
	MyEHandle sentry;
	MyEHandle sapper;
	//	short builder;
}tf_sentry_t;

typedef struct
{
	MyEHandle disp;
	MyEHandle sapper;
	//	short builder;
}tf_disp_t;


class CTeamControlPointRound;
class CTeamControlPointMaster;
class CTeamControlPoint;
class CTeamRoundTimer;

class CAttribute
{
public:
	CAttribute(const char *name, float fval);

	void ApplyAttribute(edict_t *pEdict);

	const char *m_name;
	float m_fval;
};

class CTF2Loadout
{
public:
	CTF2Loadout(const char *pszClassname, int iIndex, int iQuality, int iMinLevel, int iMaxLevel);

	void FreeMemory();

	int m_iIndex;
	int m_iSlot;
	int m_iQuality;
	bool m_bCanBeUsedInMedieval;
	int m_iMinLevel;
	int m_iMaxLevel;
	const char *m_pszClassname;
};

class CTeamFortress2Mod : public CBotMod
{
public:
	CTeamFortress2Mod()
	{
		Setup("tf", "team fortress 2", MOD_TF2, BOTTYPE_TF2, "TF2");

		m_pResourceEntity = NULL;
		m_bBotCommand_NeedCheatsHack = true;
	}

	void MapInit();

	void ModFrame();

	bool IsAreaOwnedByTeam(int iArea, int iTeam);

	static void UpdatePointMaster();

	void ClientCommand(edict_t *pEntity, int argc, const char *pcmd, const char *arg1, const char *arg2);

	virtual const char *GetPlayerClass()
	{
		return "CTFPlayer";
	}

	void InitMod();

	static void RoundStart();

	void FreeMemory();

	static int GetTeam(edict_t *pEntity);

	static TFClass GetSpyDisguise(edict_t *pPlayer);

	static int GetSentryLevel(edict_t *pSentry);
	static int GetDispenserLevel(edict_t *pDispenser);

	static bool IsDispenser(edict_t *pEntity, int iTeam, bool checkcarrying = false);

	static bool IsPayloadBomb(edict_t *pEntity, int iTeam);

	static int GetTeleporterWaypoint(edict_t *pTele);

	bool IsWaypointAreaValid(int iWptArea, int iWptFlags);

	static bool IsSuddenDeath(void);

	static bool IsHealthKit(edict_t *pEntity);

	static bool IsAmmo(edict_t *pEntity);

	static int GetArea(); // get current area of map

	static void SetArea(int area) { m_iArea = area; }

	static bool IsSentry(edict_t *pEntity, int iTeam, bool checkcarrying = false);

	static bool IsTeleporter(edict_t *pEntity, int iTeam, bool checkcarrying = false);

	static void UpdateTeleportTime(edict_t *pOwner);
	static float GetTeleportTime(edict_t *pOwner);

	static bool IsTeleporterEntrance(edict_t *pEntity, int iTeam, bool checkcarrying = false);

	static bool IsTeleporterExit(edict_t *pEntity, int iTeam, bool checkcarrying = false);

	static inline bool IsMapType(eTFMapType iMapType) { return iMapType == m_MapType; }

	static bool IsFlag(edict_t *pEntity, int iTeam);

	static bool WithinEndOfRound(float fTime);

	static bool IsPipeBomb(edict_t *pEntity, int iTeam);

	static bool IsHurtfulPipeGrenade(edict_t *pEntity, edict_t *pPlayer, bool bCheckOwner = true);

	static bool IsRocket(edict_t *pEntity, int iTeam);

	static int GetEnemyTeam(int iTeam);

	static bool BuildingNearby(int iTeam, Vector vOrigin);

	// Naris @ AlliedModders .net

	static bool TF2_IsPlayerZoomed(edict_t *pPlayer);

	static bool TF2_IsPlayerSlowed(edict_t *pPlayer);

	static bool TF2_IsPlayerDisguised(edict_t *pPlayer);

	static bool TF2_IsPlayerCloaked(edict_t *pPlayer);

	static bool TF2_IsPlayerInvuln(edict_t *pPlayer);

	static bool TF2_IsPlayerKrits(edict_t *pPlayer);

	static bool TF2_IsPlayerOnFire(edict_t *pPlayer);

	static bool TF2_IsPlayerTaunting(edict_t *pPlayer);

	static float TF2_GetPlayerSpeed(edict_t *pPlayer, TFClass iClass);

	static void TeleporterBuilt(edict_t *pOwner, eEngiBuild type, edict_t *pBuilding);

	static edict_t *GetTeleporterExit(edict_t *pTele);

	static void SetPointOpenTime(int time);

	static void SetSetupTime(int time);

	static void ResetSetupTime();

	static bool IsArenaPointOpen();

	static bool HasRoundStarted();

	static int GetHighestScore();

	static edict_t *NearestDispenser(Vector vOrigin, int team);

	static void FlagPickedUp(int iTeam, edict_t *pPlayer);
	static void FlagReturned(int iTeam);

	static void SetAttackDefendMap(bool bSet) { m_bAttackDefendMap = bSet; }
	static bool IsAttackDefendMap() { return m_bAttackDefendMap; }

	void AddWaypointFlags(edict_t *pPlayer, edict_t *pEdict, int *iFlags, int *iArea, float *fMaxDistance);

	void GetTeamOnlyWaypointFlags(int iTeam, int *iOn, int *iOff);

	static bool GetFlagLocation(int iTeam, Vector *vec);

	static bool GetDroppedFlagLocation(int iTeam, Vector *vec)
	{
		if (iTeam == TF2_TEAM_BLUE)
		{
			*vec = m_vFlagLocationBlue;
			return m_bFlagLocationValidBlue;
		}
		else if (iTeam == TF2_TEAM_RED)
		{
			*vec = m_vFlagLocationRed;
			return m_bFlagLocationValidRed;
		}

		return false;
	}

	static void FlagDropped(int iTeam, Vector vLoc)
	{
		if (iTeam == TF2_TEAM_BLUE)
		{
			m_pFlagCarrierBlue = NULL;
			m_vFlagLocationBlue = vLoc;
			m_bFlagLocationValidBlue = true;
		}
		else if (iTeam == TF2_TEAM_RED)
		{
			m_pFlagCarrierRed = NULL;
			m_vFlagLocationRed = vLoc;
			m_bFlagLocationValidRed = true;
		}

		m_iFlagCarrierTeam = iTeam;
	}

	static void RoundStarted()
	{
		m_bHasRoundStarted = true;
		m_bRoundOver = false;
		m_iWinningTeam = 0;
	}

	static void RoundWon(int iWinningTeam)
	{
		m_bHasRoundStarted = false;
		m_bRoundOver = true;
		m_iWinningTeam = iWinningTeam;
	}

	static inline bool IsLosingTeam(int iTeam)
	{
		return !m_bHasRoundStarted && m_bRoundOver && m_iWinningTeam && (m_iWinningTeam != iTeam);
	}

	static void RoundReset();

	static inline bool IsFlagCarrier(edict_t *pPlayer)
	{
		return (m_pFlagCarrierBlue == pPlayer) || (m_pFlagCarrierRed == pPlayer);
	}

	static inline edict_t *GetFlagCarrier(int iTeam)
	{
		if (iTeam == TF2_TEAM_BLUE)
			return m_pFlagCarrierBlue;
		else if (iTeam == TF2_TEAM_RED)
			return m_pFlagCarrierRed;

		return NULL;
	}

	static bool IsFlagCarried(int iTeam)
	{
		if (iTeam == TF2_TEAM_BLUE)
			return (m_pFlagCarrierBlue != NULL);
		else if (iTeam == TF2_TEAM_RED)
			return (m_pFlagCarrierRed != NULL);

		return false;
	}

	static void SapperPlaced(edict_t *pOwner, eEngiBuild type, edict_t *pSapper);
	static void SapperDestroyed(edict_t *pOwner, eEngiBuild type, edict_t *pSapper);
	static void SentryBuilt(edict_t *pOwner, eEngiBuild type, edict_t *pBuilding);
	static void DispenserBuilt(edict_t *pOwner, eEngiBuild type, edict_t *pBuilding);

	static CWaypoint *GetBestWaypointMVM(CBot *pBot, int iFlags);

	static edict_t *GetMySentryGun(edict_t *pOwner)
	{
		int id = ENTINDEX(pOwner) - 1;

		if (id >= 0)
		{
			return m_SentryGuns[id].sentry.Get();
		}

		return NULL;
	}

	static edict_t *GetSentryOwner(edict_t *pSentry)
	{
		for (short int i = 1; i <= MAX_PLAYERS; i++)
		{
			if (m_SentryGuns[i].sentry.Get() == pSentry)
				return INDEXENT(i);
		}

		return NULL;
	}

	static bool IsMySentrySapped(edict_t *pOwner)
	{
		int id = ENTINDEX(pOwner) - 1;

		if (id >= 0)
		{
			return (m_SentryGuns[id].sentry.Get() != NULL) && (m_SentryGuns[id].sapper.Get() != NULL);
		}

		return false;
	}

	static edict_t *GetSentryGun(int id)
	{
		return m_SentryGuns[id].sentry.Get();
	}

	static edict_t *GetTeleEntrance(int id)
	{
		return m_Teleporters[id].entrance.Get();
	}

	static bool IsMyTeleporterSapped(edict_t *pOwner)
	{
		int id = ENTINDEX(pOwner) - 1;

		if (id >= 0)
		{
			return ((m_Teleporters[id].exit.Get() != NULL) || (m_Teleporters[id].entrance.Get() != NULL)) && (m_Teleporters[id].sapper.Get() != NULL);
		}

		return false;
	}

	static bool IsMyDispenserSapped(edict_t *pOwner)
	{
		int id = ENTINDEX(pOwner) - 1;

		if (id >= 0)
		{
			return (m_Dispensers[id].disp.Get() != NULL) && (m_Dispensers[id].sapper.Get() != NULL);
		}

		return false;
	}

	static bool IsSentrySapped(edict_t *pSentry)
	{
		unsigned int i;

		for (i = 0; i < MAX_PLAYERS; i++)
		{
			if (m_SentryGuns[i].sentry.Get() == pSentry)
				return m_SentryGuns[i].sapper.Get() != NULL;
		}

		return false;
	}

	static bool IsTeleporterSapped(edict_t *pTele)
	{
		unsigned int i;

		for (i = 0; i < MAX_PLAYERS; i++)
		{
			if ((m_Teleporters[i].entrance.Get() == pTele) || (m_Teleporters[i].exit.Get() == pTele))
				return m_Teleporters[i].sapper.Get() != NULL;
		}

		return false;
	}

	static bool IsDispenserSapped(edict_t *pDisp)
	{
		unsigned int i;

		for (i = 0; i < MAX_PLAYERS; i++)
		{
			if (m_Dispensers[i].disp.Get() == pDisp)
				return m_Dispensers[i].sapper.Get() != NULL;
		}

		return false;
	}

	static edict_t *FindResourceEntity();

	static void AddCapDefender(edict_t *pPlayer, int iCapIndex)
	{
		m_iCapDefenders[iCapIndex] |= (1 << (ENTINDEX(pPlayer) - 1));
	}

	static void RemoveCapDefender(edict_t *pPlayer, int iCapIndex)
	{
		m_iCapDefenders[iCapIndex] &= ~(1 << (ENTINDEX(pPlayer) - 1));
	}

	static void ResetDefenders()
	{
		memset(m_iCapDefenders, 0, sizeof(int)*MAX_CONTROL_POINTS);
	}

	static bool IsDefending(edict_t *pPlayer);//, int iCapIndex = -1 );

	static bool IsCapping(edict_t *pPlayer);//, int iCapIndex = -1 );

	static void AddCapper(int cp, int capper)
	{
		if (capper && (cp < MAX_CAP_POINTS))
			m_Cappers[cp] |= (1 << (capper - 1));
	}

	static void RemoveCappers(int cp)
	{
		m_Cappers[cp] = 0;
	}

	static void ResetCappers()
	{
		memset(m_Cappers, 0, sizeof(int)*MAX_CONTROL_POINTS);
	}

	static int NumPlayersOnTeam(int iTeam, bool bAliveOnly = false);
	static int NumClassOnTeam(int iTeam, int iClass);

	static int GetFlagCarrierTeam() { return m_iFlagCarrierTeam; }
	static bool CanTeamPickupFlag_SD(int iTeam, bool bGetUnknown);

	static edict_t *GetBuildingOwner(eEngiBuild object, short index);
	static edict_t *GetBuilding(eEngiBuild object, edict_t *pOwner);

	static bool IsBoss(edict_t *pEntity, float *fFactor = NULL);

	static void InitBoss(bool bSummoned) { m_bBossSummoned = bSummoned; m_pBoss = NULL; }

	static bool IsBossSummoned() { return m_bBossSummoned; }

	static bool IsSentryGun(edict_t *pEdict);

	static edict_t *GetMediGun(edict_t *pPlayer);

	static void FindMediGun(edict_t *pPlayer);

	bool CheckWaypointForTeam(CWaypoint *pWpt, int iTeam);

	static bool IsFlagAtDefaultState() { return bFlagStateDefault; }
	static void ResetFlagStateToDefault() { bFlagStateDefault = true; }
	static void SetDontClearPoints(bool bClear) { m_bDontClearPoints = bClear; }
	static bool DontClearPoints() { return m_bDontClearPoints; }
	static CTFObjectiveResource m_ObjectiveResource;

	static CTeamControlPointRound *GetCurrentRound() { return m_pCurrentRound; }

	static CTeamControlPointMaster *GetPointMaster() { return m_PointMaster; }

	static void UpdateRedPayloadBomb(edict_t *pent);
	static void UpdateBluePayloadBomb(edict_t *pent);

	static edict_t *GetPayloadBomb(int team);

	static void MVMAlarmSounded() { m_bMVMAlarmSounded = true; }
	static void MVMAlarmReset() { m_bMVMAlarmSounded = false; }
	static float GetMVMCapturePointRadius()
	{
		return m_fMVMCapturePointRadius;
	}
	static bool GetMVMCapturePoint(Vector *vec)
	{
		if (m_bMVMCapturePointValid)
		{
			*vec = m_vMVMCapturePoint;
			return true;
		}

		return (GetFlagLocation(TF2_TEAM_BLUE, vec));
	}

	static bool IsMedievalMode();

private:
	static float TF2_GetClassSpeed(int iClass);

	static CTeamControlPointMaster *m_PointMaster;
	static CTeamControlPointRound *m_pCurrentRound;
	static MyEHandle m_PointMasterResource;
	static CTeamRoundTimer m_Timer;

	static eTFMapType m_MapType;

	static MyEHandle m_pPayLoadBombRed;
	static MyEHandle m_pPayLoadBombBlue;

	static tf_tele_t m_Teleporters[MAX_PLAYERS];	// used to let bots know who made a teleport ans where it goes
	static tf_sentry_t m_SentryGuns[MAX_PLAYERS];	// used to let bots know if sentries have been sapped or not
	static tf_disp_t  m_Dispensers[MAX_PLAYERS];	// used to let bots know where friendly/enemy dispensers are

	static int m_iArea;

	static float m_fSetupTime;

	static float m_fRoundTime;

	static MyEHandle m_pFlagCarrierRed;
	static MyEHandle m_pFlagCarrierBlue;

	static float m_fPointTime;
	static float m_fArenaPointOpenTime;

	static MyEHandle m_pResourceEntity;
	static MyEHandle m_pGameRules;
	static bool m_bAttackDefendMap;

	static int m_Cappers[MAX_CONTROL_POINTS];
	static int m_iCapDefenders[MAX_CONTROL_POINTS];

	static bool m_bHasRoundStarted;

	static int m_iFlagCarrierTeam;
	static MyEHandle m_pBoss;
	static bool m_bBossSummoned;
	static bool bFlagStateDefault;

	static MyEHandle pMediGuns[MAX_PLAYERS];
	static bool m_bDontClearPoints;

	static bool m_bRoundOver;
	static int m_iWinningTeam;

	static Vector m_vFlagLocationBlue;
	static bool m_bFlagLocationValidBlue;
	static Vector m_vFlagLocationRed;
	static bool m_bFlagLocationValidRed;


	static bool m_bMVMFlagStartValid;
	static Vector m_vMVMFlagStart;
	static bool m_bMVMCapturePointValid;
	static Vector m_vMVMCapturePoint;
	static bool m_bMVMAlarmSounded;
	static float m_fMVMCapturePointRadius;
	static int m_iCapturePointWptID;
	static int m_iFlagPointWptID;

	static void SetupLoadOutWeapons(void);

	// slots X nine classes
	static vector<CTF2Loadout*> m_pLoadoutWeapons[TF2_SLOT_MAX][9];
	//static vector<CTF2Loadout*> m_pHats;
	//static CTF2Loadout *m_StockWeapons[3][9]; //stock weapons

};

class CTeamFortress2ModDedicated : public CTeamFortress2Mod
{
public:
	CTeamFortress2ModDedicated()
	{
#ifdef __linux__
		Setup("tf","orangebox",MOD_TF2,BOTTYPE_TF2,"TF2");    //bir3yk
#else
		Setup("tf", "source dedicated server", MOD_TF2, BOTTYPE_TF2, "TF2");
#endif
	}

private:

};
/*
class CHalfLifeDeathmatchMod : public CBotMod
{
public:
CHalfLifeDeathmatchMod()
{
setup("hl2mp", "half-life 2 deathmatch", MOD_HLDM2, BOTTYPE_HL2DM, "HL2DM");
}

void initMod ();

void mapInit ();

bool playerSpawned ( edict_t *pPlayer );

static inline edict_t *getButtonAtWaypoint ( CWaypoint *pWaypoint )
{
for ( unsigned int i = 0; i < m_LiftWaypoints.size(); i ++ )
{
if ( m_LiftWaypoints[i].pWaypoint == pWaypoint )
return m_LiftWaypoints[i].pEdict;
}

return NULL;
}

//void entitySpawn ( edict_t *pEntity );
private:
static vector<edict_wpt_pair_t> m_LiftWaypoints;
};

class CHalfLifeDeathmatchModDedicated : public CHalfLifeDeathmatchMod
{
public:
CHalfLifeDeathmatchModDedicated()
{
setup("hl2mp", "source dedicated server", MOD_HLDM2, BOTTYPE_HL2DM, "HL2DM");
}

//void initMod ();

//void mapInit ();

//void entitySpawn ( edict_t *pEntity );
protected:

};
/*
class CNaturalSelection2Mod : public CBotMod
{
public:
CNaturalSelection2Mod()
{
setup("ns2","natural selection 2",MOD_NS2,BOTTYPE_NS2);
}
// linux fix

virtual const char *getPlayerClass ()
{
return "CBaseNS2Player";
}

virtual bool isAreaOwnedByTeam (int iArea, int iTeam) { return (iArea == 0); }

virtual void addWaypointFlags (edict_t *pPlayer, edict_t *pEdict, int *iFlags, int *iArea, float *fMaxDistance ){ return; }

////////////////////////////////
virtual void initMod ();

virtual void mapInit ();

virtual bool playerSpawned ( edict_t *pPlayer );

virtual void clientCommand ( edict_t *pEntity, int argc,const char *pcmd, const char *arg1, const char *arg2 ) {};

virtual void modFrame () { };

virtual void freeMemory() {};

virtual bool isWaypointAreaValid ( int iWptArea, int iWptFlags ) { return true; }

virtual void getTeamOnlyWaypointFlags ( int iTeam, int *iOn, int *iOff )
{
*iOn = 0;
*iOff = 0;
}
};
*/

class CBotMods
{
public:

	static void ParseFile();

	static void CreateFile();

	static void ReadMods();

	static void FreeMemory();

	static CBotMod *GetMod(char *szModFolder, char *szSteamFolder);

private:
	static vector<CBotMod*> m_Mods;
};

#endif
