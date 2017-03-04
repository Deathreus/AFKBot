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
#ifndef __BOT_FORTRESS_H__
#define __BOT_FORTRESS_H__

#include "bot_base.h"
#include "bot_utility.h"

//#include <stack>
//using namespace std;

#define TF2_ROCKETSPEED   1100
#define TF2_GRENADESPEED  1216 // TF2 wiki
#define TF2_MAX_SENTRYGUN_RANGE 1024
#define TF2_STICKYGRENADE_MAX_DISTANCE 1600

class CBotWeapon;
class CWaypoint;
class CBotUtility;

#define TF2_SLOT_PRMRY 0 // primary
#define TF2_SLOT_SCNDR 1 // secondary
#define TF2_SLOT_MELEE 2
#define TF2_SLOT_PDA 3
#define TF2_SLOT_PDA2 4
#define TF2_SLOT_HAT 5
#define TF2_SLOT_MISC 6
#define TF2_SLOT_ACTION 7
#define TF2_SLOT_MAX 8

#define TF2_TEAM_BLUE 3
#define TF2_TEAM_RED 2

#define RESIST_BULLETS 0
#define RESIST_EXPLO 1
#define RESIST_FIRE 2

#define TF2_SENTRY_LEVEL1_HEALTH 150
#define TF2_SENTRY_LEVEL2_HEALTH 180
#define TF2_SENTRY_LEVEL3_HEALTH 216

#define TF2_DISPENSER_LEVEL1_HEALTH 150
#define TF2_DISPENSER_LEVEL2_HEALTH 180
#define TF2_DISPENSER_LEVEL3_HEALTH 216

#define TF2_PLAYER_SLOWED       (1 << 0)
#define TF2_PLAYER_ZOOMED       (1 << 1)
#define TF2_PLAYER_DISGUISING   (1 << 2)
#define TF2_PLAYER_DISGUISED	(1 << 3)
#define TF2_PLAYER_CLOAKED      (1 << 4)
#define TF2_PLAYER_INVULN       (1 << 5)
#define TF2_PLAYER_TELEGLOW     (1 << 6)
#define TF2_PLAYER_TAUNTING	    (1 << 7)
#define TF2_PLAYER_TELEPORTING	(1 << 10)
#define TF2_PLAYER_KRITS		(1 << 11)
#define TF2_PLAYER_BONKED		(1 << 14)
#define TF2_PLAYER_HEALING	    (1 << 21)    
#define TF2_PLAYER_ONFIRE	    (1 << 22)

//#define TF2_SPY_FOV_KNIFEATTACK 90.0f

typedef enum
{
	TF_VC_MEDIC = 0,
	TF_VC_INCOMING = 1,
	TF_VC_HELP = 2,
	TF_VC_THANKS = 4,
	TF_VC_SPY = 5,
	TF_VC_BATTLECRY = 6,
	TF_VC_GOGOGO = 8,
	TF_VC_SENTRYAHEAD = 9,
	TF_VC_CHEERS = 10,
	TF_VC_MOVEUP = 12,
	TF_VC_TELEPORTERHERE = 13,
	TF_VC_JEERS = 14,
	TF_VC_GOLEFT = 16,
	TF_VC_DISPENSERHERE = 17,
	TF_VC_POSITIVE = 18,
	TF_VC_GORIGHT = 20,
	TF_VC_SENTRYHERE = 21,
	TF_VC_NEGATIVE = 22,
	TF_VC_YES = 24,
	TF_VC_ACTIVATEUBER = 25,
	TF_VC_NICESHOT = 26,
	TF_VC_NO = 28,
	TF_VC_UBERREADY = 29,
	TF_VC_GOODJOB = 30,
	TF_VC_INVALID = 31
}eTFVoiceCMD;


typedef enum
{
	TF_TRAP_TYPE_NONE,
	TF_TRAP_TYPE_WPT,
	TF_TRAP_TYPE_POINT,
	TF_TRAP_TYPE_FLAG,
	TF_TRAP_TYPE_PL,
	TF_TRAP_TYPE_ENEMY
}eDemoTrapType;

typedef enum
{
	TF_CLASS_CIVILIAN = 0,
	TF_CLASS_SCOUT,
	TF_CLASS_SNIPER,
	TF_CLASS_SOLDIER,
	TF_CLASS_DEMOMAN,
	TF_CLASS_MEDIC,
	TF_CLASS_HWGUY,
	TF_CLASS_PYRO,
	TF_CLASS_SPY,
	TF_CLASS_ENGINEER,
	TF_CLASS_MAX
}TFClass;

typedef enum
{
	TF_TEAM_UNASSIGNED = 0,
	TF_TEAM_SPEC = 1,
	TF_TEAM_BLUE = 2,
	TF_TEAM_RED = 3
}TFTeam;

typedef enum
{
	ENGI_DISP = 0,
	ENGI_TELE,
	ENGI_SENTRY,
	ENGI_SAPPER,
	ENGI_EXIT,
	ENGI_ENTRANCE,
}eEngiBuild;

typedef enum
{
	ENGI_BUILD,
	ENGI_DESTROY
}eEngiCmd;

class CBotTF2FunctionEnemyAtIntel : public IBotFunction
{
public:
	CBotTF2FunctionEnemyAtIntel(int iTeam, Vector vPos, int type, edict_t *pPlayer = NULL, int capindex = -1){ m_iTeam = iTeam; m_vPos = vPos; m_iType = type; m_pPlayer = pPlayer; m_iCapIndex = capindex; }

	void Execute(CBot *pBot);
private:
	int m_iTeam;
	Vector m_vPos;
	int m_iType;
	edict_t *m_pPlayer;
	int m_iCapIndex;
};

class CBroadcastSpySap : public IBotFunction
{
public:
	CBroadcastSpySap(edict_t *pSpy) { m_pSpy = pSpy; }

	void Execute(CBot *pBot);
private:
	edict_t *m_pSpy;
};

class CBroadcastOvertime : public IBotFunction
{
public:
	CBroadcastOvertime() {};

	void Execute(CBot *pBot);
};

class CBroadcastFlagReturned : public IBotFunction
{
public:
	CBroadcastFlagReturned(int iTeam) { m_iTeam = iTeam; }

	void Execute(CBot *pBot);
private:
	int m_iTeam;
};

class CBroadcastFlagDropped : public IBotFunction
{
public:
	CBroadcastFlagDropped(int iTeam, Vector origin) { m_iTeam = iTeam; m_vOrigin = origin; }

	void Execute(CBot *pBot);
private:
	Vector m_vOrigin;
	int m_iTeam;
};

class CBroadcastFlagCaptured : public IBotFunction
{
public:
	CBroadcastFlagCaptured(int iTeam) { m_iTeam = iTeam; }

	void Execute(CBot *pBot);
private:
	int m_iTeam;
};

class CBroadcastRoundStart : public IBotFunction
{
public:
	CBroadcastRoundStart(bool bFullReset) { m_bFullReset = bFullReset; }

	void Execute(CBot *pBot);
private:
	bool m_bFullReset;
};

class CBroadcastCapturedPoint : public IBotFunction
{
public:
	CBroadcastCapturedPoint(int iPoint, int iTeam, const char *szName);

	void Execute(CBot *pBot);
private:
	int m_iPoint;
	int m_iTeam;
	const char *m_szName;
};

class CBasePlayer;

#define EVENT_FLAG_PICKUP 0
#define EVENT_CAPPOINT    1

class CBotFortress : public CBot
{
public:

	CBotFortress();

	//virtual bool wantToZoom () { return m_bWantToZoom; }

	//virtual void wantToZoom ( bool bSet ) { m_bWantToZoom = bSet; }

	virtual void EnemyLost(edict_t *pEnemy);

	virtual void UpdateConditions();

	virtual void Shot(edict_t *pEnemy);

	virtual int EngiBuildObject(int *iState, eEngiBuild iObject, float *fTime, int *iTries);

	virtual float GetEnemyFactor(edict_t *pEnemy) { return CBot::GetEnemyFactor(pEnemy); }

	virtual void CheckDependantEntities();

	int GetMetal();

	//virtual Vector getAimVector ( edict_t *pEntity ) { return CBot::getAimVector(pEntity); }

	virtual void ModAim(edict_t *pEntity, Vector &v_origin, Vector *v_desired_offset, Vector &v_size, float fDist, float fDist2D)
	{
		CBot::ModAim(pEntity, v_origin, v_desired_offset, v_size, fDist, fDist2D);
	}

	virtual void TouchedWpt(CWaypoint *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1) { CBot::TouchedWpt(pWaypoint); }

	inline edict_t *GetHealingEntity() { return m_pHeal; }

	inline void ClearHealingEntity() { m_pHeal = NULL; }

	virtual unsigned int MaxEntityIndex() { return gpGlobals->maxEntities; }

	virtual void Init(bool bVarInit = false);

	virtual void FoundSpy(edict_t *pEdict, TFClass iDisguise);

	virtual void GetTasks(unsigned int iIgnore = 0) { CBot::GetTasks(iIgnore); }

	virtual void Died(edict_t *pKiller, const char *pszWeapon);

	virtual void Killed(edict_t *pVictim, char *weapon);

	virtual void ModThink();

	bool IsBuilding(edict_t *pBuilding);

	float GetHealFactor(edict_t *pPlayer);

	virtual bool WantToFollowEnemy();

	virtual void CheckBuildingsValid(bool bForce = false) {};

	virtual void CheckHealingValid();

	// linux fix 2
	virtual edict_t *FindEngineerBuiltObject(eEngiBuild iBuilding, int index) { return NULL; }

	virtual void EngineerBuild(eEngiBuild iBuilding, eEngiCmd iEngiCmd) {};

	virtual void SpyDisguise(int iTeam, int iClass) {};

	virtual bool LookAfterBuildings(float *fTime) { return false; }

	inline void NextLookAfterSentryTime(float fTime) { m_fLookAfterSentryTime = fTime; }

	inline edict_t *GetSentry() { return m_pSentryGun; }

	virtual bool HasEngineerBuilt(eEngiBuild iBuilding) { return false; }

	virtual void EngiBuildSuccess(eEngiBuild iObject, int index) {};

	virtual bool HealPlayer(edict_t *pPlayer, edict_t *pPrevPlayer) { return false; }
	virtual bool UpgradeBuilding(edict_t *pBuilding, bool removesapper = false) { return false; }

	virtual bool IsCloaked() { return false; }
	virtual bool IsDisguised() { return false; }

	virtual CBotWeapon *GetCurrentWeapon()
	{
		return CBot::GetCurrentWeapon();
	}

	virtual bool HandleAttack(CBotWeapon *pWeapon, edict_t *pEnemy) { return CBot::HandleAttack(pWeapon, pEnemy); }

	void ResetAttackingEnemy() { m_pAttackingEnemy = NULL; }

	virtual bool SetVisible(edict_t *pEntity, bool bVisible);

	virtual void SetClass(TFClass _class);

	inline edict_t *SeeFlag(bool reset = false) { if (reset) { m_pFlag = NULL; } return m_pFlag; }

	virtual bool CanAvoid(edict_t *pEntity);

	virtual bool IsEnemy(edict_t *pEdict, bool bCheckWeapons = true);

	virtual bool StartGame();

	virtual void SpawnInit();

	bool IsTF() { return true; }

	virtual bool IsTF2() { return false; }

	virtual bool Hurt(edict_t *pAttacker, int iHealthNow, bool bDontHide = false)
	{
		return CBot::Hurt(pAttacker, iHealthNow, bDontHide);
	}

	void ChooseClass();

	virtual TFClass GetClass() { return TF_CLASS_CIVILIAN; }

	virtual void UpdateClass() { };

	virtual void CurrentlyDead();

	void PickedUpFlag();

	inline bool HasFlag() { return m_bHasFlag; }

	inline void DroppedFlag() { m_bHasFlag = false; }

	void MedicCalled(edict_t *pPlayer);

	bool IsAlive();

	void EnemyDown(edict_t *pEnemy)
	{
		CBot::EnemyDown(pEnemy);

		if (pEnemy == m_pPrevSpy)
		{
			m_pPrevSpy = NULL;
			m_fSeeSpyTime = 0.0f;
		}
	}

	bool IsTeleporterUseful(edict_t *pTele);

	bool WaitForFlag(Vector *vOrigin, float *fWait, bool bFindFlag);

	void FlagDropped(Vector vOrigin);
	void TeamFlagDropped(Vector vOrigin);
	void TeamFlagPickup();

	virtual bool WantToListenToPlayer(edict_t *pPlayer, int iWeaponID = -1) { return true; }
	virtual bool WantToListenToPlayerFootsteps(edict_t *pPlayer) { return true; }
	virtual bool WantToInvestigateSound() { return true; }

	inline void FlagReset() { m_fLastKnownFlagTime = 0.0f; }
	inline void TeamFlagReset() { m_fLastKnownTeamFlagTime = 0.0f; }

	virtual bool CanGotoWaypoint(Vector vPrevWaypoint, CWaypoint *pWaypoint, CWaypoint *pPrev = NULL)
	{
		return CBot::CanGotoWaypoint(vPrevWaypoint, pWaypoint, pPrev);
	}

	virtual void Setup();

	virtual bool NeedHealth();

	virtual bool NeedAmmo();

	void WaitBackstab();

	void WantToDisguise(bool bSet);

	virtual bool Select_CWeapon(CWeapon *pWeapon) { return CBot::Select_CWeapon(pWeapon); }
	virtual bool SelectBotWeapon(CBotWeapon *pBotWeapon) { return CBot::SelectBotWeapon(pBotWeapon); }

	virtual bool GetIgnoreBox(Vector *vLoc, float *fSize);

	// found a new enemy
	virtual void EnemyFound(edict_t *pEnemy){ CBot::EnemyFound(pEnemy); }

	bool WantToNest();

	bool OverrideAmmoTypes() { return false; }

	bool WantToCloak();

	bool WantToUnCloak();

	bool SomeoneCalledMedic();

	void WaitCloak();

	void DetectedAsSpy(edict_t *pDetector, bool bDisguiseComprimised);

	// return an enemy sentry gun / special visible (e.g.) for quick checking
	virtual edict_t *GetVisibleSpecial();

	inline bool IsBeingHealed() { return m_bIsBeingHealed; }

	virtual void HandleWeapons() { CBot::HandleWeapons(); }

	virtual void SeeFriendlyDie(edict_t *pDied, edict_t *pKiller, CWeapon *pWeapon) { CBot::SeeFriendlyDie(pDied, pKiller, pWeapon); }
	virtual void SeeFriendlyKill(edict_t *pTeamMate, edict_t *pDied, CWeapon *pWeapon) { CBot::SeeFriendlyKill(pTeamMate, pDied, pWeapon); }

	virtual void CoiceCommand(int cmd) { };

	virtual void SeeFriendlyHurtEnemy(edict_t *pTeammate, edict_t *pEnemy, CWeapon *pWeapon);

	bool IncomingRocket(float fRange);

	virtual void HearPlayerAttack(edict_t *pAttacker, int iWeaponID) { CBot::HearPlayerAttack(pAttacker, iWeaponID); }
protected:
	virtual void SelectTeam();

	virtual void SelectClass();

	virtual void CallMedic();

	static bool IsClassOnTeam(int iClass, int iTeam);

	int GetSpyDisguiseClass(int iTeam);

	virtual bool ThinkSpyIsEnemy(edict_t *pEdict, TFClass iDisguise);

	virtual bool CheckStuck(void) { return CBot::CheckStuck(); }

	float m_fCallMedic;
	float m_fTauntTime;
	float m_fTaunting;
	float m_fDefendTime;

	float m_fHealFactor;

	MyEHandle m_pHeal;
	MyEHandle m_pLastHeal;
	MyEHandle m_pSentryGun;
	MyEHandle m_pDispenser;
	MyEHandle m_pTeleEntrance;
	MyEHandle m_pTeleExit;

	MyEHandle m_pAmmo;
	MyEHandle m_pHealthkit;

	MyEHandle m_pNearestDisp;
	MyEHandle m_pNearestEnemySentry;
	MyEHandle m_pNearestAllySentry;
	MyEHandle m_pNearestEnemyTeleporter;
	MyEHandle m_pNearestEnemyDisp;
	MyEHandle m_pNearestTeleEntrance;
	MyEHandle m_pNearestPipeGren;
	MyEHandle m_pAttackingEnemy;

	MyEHandle m_pFlag;
	MyEHandle m_pPrevSpy;

	float m_fFrenzyTime;
	float m_fSpyCloakTime;
	float m_fSpyUncloakTime;
	float m_fSeeSpyTime;
	float m_fLastSeeSpyTime;
	float m_fSpyDisguiseTime;
	float m_fLastSaySpy;
	float m_fPickupTime;
	float m_fLookAfterSentryTime;

	TFClass m_iPrevSpyDisguise;

	Vector m_vLastSeeSpy;

	// valid flag point area
	Vector m_vLastKnownFlagPoint;
	Vector m_vLastKnownTeamFlagPoint;

	Vector m_vTeleportEntrance;
	bool m_bEntranceVectorValid;
	Vector m_vSentryGun;
	bool m_bSentryGunVectorValid;
	Vector m_vDispenser;
	bool m_bDispenserVectorValid;
	Vector m_vTeleportExit;
	bool m_bTeleportExitVectorValid;

	// 1 minute wait
	float m_fLastKnownFlagTime;
	float m_fLastKnownTeamFlagTime;

	float m_fBackstabTime;

	TFClass m_iClass;

	float m_fUpdateClass;
	float m_fUseTeleporterTime;

	bool m_bHasFlag;
	float m_fSnipeAttackTime;

	// time left before the bot decides if it wants to change class
	float m_fChangeClassTime;
	// bot should check if he can change class now
	bool m_bCheckClass;
	MyEHandle m_pLastCalledMedic;
	CBotLastSee m_pLastSeeMedic;
	/*MyEHandle m_pLastSeeMedic;
	Vector m_vLastSeeMedic;
	float m_fLastSeeMedicTime;*/
	float m_fLastCalledMedicTime;
	bool m_bIsBeingHealed;
	float m_fMedicUpdatePosTime;
	Vector m_vMedicPosition;

	bool m_bCanBeUbered;
	float m_fCheckHealTime;

	float m_fClassDisguiseFitness[10]; // classes disguised as fitness
	float m_fClassDisguiseTime[10];
	float m_fDisguiseTime;
	unsigned short m_iDisguiseClass;
	float m_fSentryPlaceTime;
	unsigned int m_iSentryKills;
	float m_fTeleporterEntPlacedTime;
	float m_fTeleporterExtPlacedTime;
	unsigned m_iTeleportedPlayers;

	// list of spies who I saw were attacked by my team-mates recently
	// for use with spy checking
	float m_fSpyList[MAX_PLAYERS];

	int m_iTeam;

	float m_fWaitTurnSentry;			// amount of time to wait before engineer turns their sentry before building

	// currently unused
	float m_fCallMedicTime[MAX_PLAYERS]; // for every player ID is kept the last time they called medic

	int m_iLastFailSentryWpt;
	int m_iLastFailTeleExitWpt;

	MyEHandle m_pHealer;

	float m_fHealingMoveTime;

	MyEHandle m_pLastEnemySentry;
	MyEHandle m_NearestEnemyRocket;
	MyEHandle m_NearestEnemyGrenade;

	float m_fLastSentryEnemyTime;
	//bool m_bWantToZoom;
};
//
//
//
//
class CTF2Loadout;

class CTF2LoadoutAdded
{
public:
	CTF2LoadoutAdded(CBaseEntity *pEnt, CTF2Loadout *pLoadout)
	{
		m_pEnt = pEnt;
		m_loadout = pLoadout;
	}

	CBaseEntity *m_pEnt;
	CTF2Loadout *m_loadout;

};

class CBotTF2 : public CBotFortress
{
public:

	// 
	CBotTF2();

	virtual CBotWeapon *getCurrentWeapon();

	void MannVsMachineWaveComplete();
	void MannVsMachineAlarmTriggered(Vector vLoc);

	bool SentryRecentlyHadEnemy();

	void HighFivePlayer(edict_t *pPlayer, float fYaw);

	virtual bool Hurt(edict_t *pAttacker, int iHealthNow, bool bDontHide = false);

	void UpdateAttackDefendPoints();

	void UpdateAttackPoints();
	void UpdateDefendPoints();

	// found a new enemy
	void EnemyFound(edict_t *pEnemy);

	void EnemyAtIntel(Vector vPos, int type = EVENT_FLAG_PICKUP, int iArea = -1);
	//

	bool IsTF2() { return true; }

	void CheckDependantEntities();

	virtual bool WantToListenToPlayerAttack(edict_t *pPlayer, int iWeaponID = -1);
	virtual bool WantToListenToPlayerFootsteps(edict_t *pPlayer);

	bool WantToInvestigateSound();

	void GetDefendArea(vector<int> *m_iAreas);

	void GetAttackArea(vector <int> *m_iAreas);

	int GetCurrentAttackArea() { return m_iCurrentAttackArea; }
	int GetCurrentDefendArea() { return m_iCurrentDefendArea; }

	void PointsUpdated();

	eBotFuncState RocketJump(int *iState, float *fTime);

	virtual bool WantToFollowEnemy();

	void ResetCloakTime() { m_fSpyCloakTime = 0.0f; }

	float GetEnemyFactor(edict_t *pEnemy);

	void FoundSpy(edict_t *pEdict, TFClass iDisguise);

	void TouchedWpt(CWaypoint *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1);

	bool HandleAttack(CBotWeapon *pWeapon, edict_t *pEnemy);

	void EngiBuildSuccess(eEngiBuild iObject, int index);

	bool LookAfterBuildings(float *fTime);

	void SpawnInit();

	bool SetVisible(edict_t *pEntity, bool bVisible);

	//Vector getAimVector ( edict_t *pEntity );
	virtual void ModAim(edict_t *pEntity, Vector &v_origin,
		Vector *v_desired_offset, Vector &v_size,
		float fDist, float fDist2D);

	void ModThink();

	bool IsCloaked();

	bool ExecuteAction(CBotUtility *util);//eBotAction id, CWaypoint *pWaypointResupply, CWaypoint *pWaypointHealth, CWaypoint *pWaypointAmmo );

	void SetClass(TFClass _class);

	bool IsDisguised();

	void CheckBuildingsValid(bool bForce = false);

	edict_t *FindEngineerBuiltObject(eEngiBuild iBuilding, int index);

	bool IsEnemy(edict_t *pEdict, bool bCheckWeapons = true);

	bool IsTF() { return true; }

	void Taunt(bool bOverride = false);

	void CallMedic();

	void RoundReset(bool bFullReset);

	void PointCaptured(int iPoint, int iTeam, const char *szPointName);

	void EngineerBuild(eEngiBuild iBuilding, eEngiCmd iEngiCmd);

	void SpyDisguise(int iTeam, int iClass);

	bool HasEngineerBuilt(eEngiBuild iBuilding);

	void GetTasks(unsigned int iIgnore = 0);

	void Died(edict_t *pKiller, const char *pszWeapon);

	void Killed(edict_t *pVictim, char *weapon);

	void CapturedFlag();

	void PointCaptured();

	void WaitRemoveSap();

	void RoundWon(int iTeam, bool bFullRound);

	void ChangeClass();

	virtual bool NeedAmmo();

	void BuildingDestroyed(int iType, edict_t *pAttacker, edict_t *pEdict);

	TFClass GetClass();

	void UpdateClass();

	bool HealPlayer();

	bool UpgradeBuilding(edict_t *pBuilding, bool removesapper = false);

	void Setup();

	void BuildingSapped(eEngiBuild building, edict_t *pSapper, edict_t *pSpy);

	void SapperDestroyed(edict_t *pSapper);

	bool CanGotoWaypoint(Vector vPrevWaypoint, CWaypoint *pWaypoint, CWaypoint *pPrev = NULL);

	bool DeployStickies(eDemoTrapType type, Vector vStand, Vector vLocation, Vector vSpread, Vector *vPoint, int *iState, int *iStickyNum, bool *bFail, float *fTime, int wptindex);

	void DetonateStickies(bool isJumping = false);

	void SetStickyTrapType(Vector vLocation, eDemoTrapType iTrapType) { m_vStickyLocation = vLocation; m_iTrapType = iTrapType; }

	bool CanDeployStickies();

	bool ThinkSpyIsEnemy(edict_t *pEdict, TFClass iDisguise);

	void SeeFriendlyDie(edict_t *pDied, edict_t *pKiller, CWeapon *pWeapon);
	void SeeFriendlyKill(edict_t *pTeamMate, edict_t *pDied, CWeapon *pWeapon);

	void VoiceCommand(int cmd);

	void HandleWeapons(void);

	virtual bool Select_CWeapon(CWeapon *pWeapon);
	virtual bool SelectBotWeapon(CBotWeapon *pBotWeapon);

	void CheckStuckonSpy(void);

	bool CheckStuck(void);

	void Init(bool bVarInit = false);

	bool CheckAttackPoint(void);

	bool CanAvoid(edict_t *pEntity);

	void HearVoiceCommand(edict_t *pPlayer, byte cmd);

	void CheckBeingHealed();

	void SpyCloak();

	void SpyUnCloak();

	void HealedPlayer(edict_t *pPlayer, float fAmount);

	void TeleportedPlayer(void);

	void UpgradeWeapon(int iSlot);

	inline bool IsCarrying() { return m_bIsCarryingObj; }

	void UpdateCarrying();

	inline void ResetCarryTime() { m_fCarryTime = engine->Time(); }

	void MvM_Upgrade();

	bool DidReadyUp() { return !!m_bMvMReady; }
	void ReadyUp(bool bReady);

	//void addLoadoutWeapon ( CTF2Loadout *weap );
private:
	// time for next jump
	float m_fDoubleJumpTime;
	// time bot has taken to sap something
	float m_fSpySapTime;
	// 
	int m_iCurrentDefendArea;
	int m_iCurrentAttackArea;
	//
	//bool m_bBlockPushing;
	//float m_fBlockPushTime;
	//
	MyEHandle m_pDefendPayloadBomb;
	MyEHandle m_pPushPayloadBomb;
	MyEHandle m_pRedPayloadBomb;
	MyEHandle m_pBluePayloadBomb;
	//
	// if demoman has already deployed stickies this is true
	// once the demoman explodes them then this becomes false
	// and it can deploy stickies again
	//bool m_bDeployedStickies;
	eDemoTrapType m_iTrapType;
	int m_iTrapCPIndex;
	Vector m_vStickyLocation;
	float m_fRemoveSapTime;
	float m_fRevMiniGunTime;
	float m_fNextRevMiniGunTime;

	float m_fRevMiniGunBelief;
	float m_fCloakBelief;

	//
	MyEHandle m_pCloakedSpy;

	float m_fAttackPointTime; // used in cart maps

	float m_prevSentryHealth;
	float m_prevDispHealth;
	float m_prevTeleExtHealth;
	float m_prevTeleEntHealth;

	float m_fDispenserHealAmount;
	float m_fDispenserPlaceTime;

	int m_iSentryArea;
	int m_iDispenserArea;
	int m_iTeleEntranceArea;
	int m_iTeleExitArea;

	eTFVoiceCMD m_nextVoicecmd;

	bool m_bIsCarryingTeleExit;
	bool m_bIsCarryingSentry;
	bool m_bIsCarryingDisp;
	bool m_bIsCarryingTeleEnt;
	bool m_bIsCarryingObj;

	float m_fCarryTime;

	float m_fCheckNextCarrying;

	//stack<CTF2LoadoutAdded*> m_toApply;

	float m_fUseBuffItemTime;

	CTF2Loadout *m_pMelee;
	CTF2Loadout *m_pPrimary;
	CTF2Loadout *m_pSecondary;

	int m_iDesiredResistType;
	
	bool m_bMvMReady;
};

class CBotFF : public CBotFortress
{
public:

	CBotFF() { CBotFortress(); }

	void ModThink();

	bool IsEnemy(edict_t *pEdict, bool bCheckWeapons = true);

	bool IsTF() { return true; }

};

#endif
