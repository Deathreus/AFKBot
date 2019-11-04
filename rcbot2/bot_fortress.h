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

#define TF2_ROCKETSPEED   1100
#define TF2_GRENADESPEED  1216
#define TF2_SENTRYGUN_RANGE 1024
#define TF2_STICKYGRENADE_MAX_DISTANCE 1600

#define TF2_SLOT_PRMRY  0
#define TF2_SLOT_SCNDR  1
#define TF2_SLOT_MELEE  2
#define TF2_SLOT_PDA    3
#define TF2_SLOT_PDA2   4
#define TF2_SLOT_HAT    5
#define TF2_SLOT_MISC   6
#define TF2_SLOT_ACTION 7
#define TF2_SLOT_MAX    8

#define TF2_TEAM_BLUE 3
#define TF2_TEAM_RED  2

#define RESIST_BULLET 0
#define RESIST_EXPLO  1
#define RESIST_FIRE   2

#define TF2_SENTRY_LEVEL1_HEALTH 150
#define TF2_SENTRY_LEVEL2_HEALTH 180
#define TF2_SENTRY_LEVEL3_HEALTH 216

#define TF2_DISPENSER_LEVEL1_HEALTH 150
#define TF2_DISPENSER_LEVEL2_HEALTH 180
#define TF2_DISPENSER_LEVEL3_HEALTH 216

#define TF2_PLAYER_SLOWED       (1 << 0)
#define TF2_PLAYER_DISGUISING   (1 << 2)
#define TF2_PLAYER_DISGUISED	(1 << 3)
#define TF2_PLAYER_CLOAKED      (1 << 4)
#define TF2_PLAYER_INVULN       (1 << 5)
#define TF2_PLAYER_TAUNTING	    (1 << 7)
#define TF2_PLAYER_KRITS		(1 << 11)
#define TF2_PLAYER_BONKED		(1 << 14)
#define TF2_PLAYER_ONFIRE	    (1 << 22)

#define FLAGEVENT_PICKUP    0
#define FLAGEVENT_CAPPED    1
#define FLAGEVENT_DEFENDED  2
#define FLAGEVENT_DROPPED   3
#define FLAGEVENT_RETURNED  4

typedef enum
{
	TFCond_Slowed = 0,
	TFCond_Zoomed,
	TFCond_Disguising,
	TFCond_Disguised,
	TFCond_Cloaked,
	TFCond_Ubercharged,
	TFCond_TeleportedGlow,
	TFCond_Taunting,
	TFCond_UberchargeFading,
	TFCond_Unknown1,
	TFCond_CloakFlicker = 9,
	TFCond_Teleporting,
	TFCond_Kritzkrieged,
	TFCond_Unknown2,
	TFCond_TmpDamageBonus = 12,
	TFCond_DeadRingered,
	TFCond_Bonked,
	TFCond_Dazed,
	TFCond_Buffed,
	TFCond_Charging,
	TFCond_DemoBuff,
	TFCond_CritCola,
	TFCond_InHealRadius,
	TFCond_Healing,
	TFCond_OnFire,
	TFCond_Overhealed,
	TFCond_Jarated,
	TFCond_Bleeding,
	TFCond_DefenseBuffed,
	TFCond_Milked,
	TFCond_MegaHeal,
	TFCond_RegenBuffed,
	TFCond_MarkedForDeath,
	TFCond_NoHealingDamageBuff,
	TFCond_SpeedBuffAlly,
	TFCond_HalloweenCritCandy,
	TFCond_CritCanteen,
	TFCond_CritDemoCharge,
	TFCond_CritHype,
	TFCond_CritOnFirstBlood,
	TFCond_CritOnWin,
	TFCond_CritOnFlagCapture,
	TFCond_CritOnKill,
	TFCond_RestrictToMelee,
	TFCond_DefenseBuffNoCritBlock,
	TFCond_Reprogrammed,
	TFCond_CritMmmph,
	TFCond_DefenseBuffMmmph,
	TFCond_FocusBuff,
	TFCond_DisguiseRemoved,
	TFCond_MarkedForDeathSilent,
	TFCond_DisguisedAsDispenser,
	TFCond_Sapped,
	TFCond_UberchargedHidden,
	TFCond_UberchargedCanteen,
	TFCond_HalloweenBombHead,
	TFCond_HalloweenThriller,
	TFCond_RadiusHealOnDamage,
	TFCond_CritOnDamage,
	TFCond_UberchargedOnTakeDamage,
	TFCond_UberBulletResist,
	TFCond_UberBlastResist,
	TFCond_UberFireResist,
	TFCond_SmallBulletResist,
	TFCond_SmallBlastResist,
	TFCond_SmallFireResist,
	TFCond_Stealthed,
	TFCond_MedigunDebuff,
	TFCond_StealthedUserBuffFade,
	TFCond_BulletImmune,
	TFCond_BlastImmune,
	TFCond_FireImmune,
	TFCond_PreventDeath,
	TFCond_MVMBotRadiowave,
	TFCond_HalloweenSpeedBoost,
	TFCond_HalloweenQuickHeal,
	TFCond_HalloweenGiant,
	TFCond_HalloweenTiny,
	TFCond_HalloweenInHell,
	TFCond_HalloweenGhostMode,
	TFCond_MiniCritOnKill,
	TFCond_DodgeChance,
	TFCond_ObscuredSmoke = 79,
	TFCond_Parachute,
	TFCond_BlastJumping,
	TFCond_HalloweenKart,
	TFCond_HalloweenKartDash,
	TFCond_BalloonHead,
	TFCond_MeleeOnly,
	TFCond_SwimmingCurse,
	TFCond_HalloweenKartNoTurn,
	TFCond_FreezeInput = 87,
	TFCond_HalloweenKartCage,
	TFCond_HasRune,
	TFCond_RuneStrength,
	TFCond_RuneHaste,
	TFCond_RuneRegen,
	TFCond_RuneResist,
	TFCond_RuneVampire,
	TFCond_RuneWarlock,
	TFCond_RunePrecision,
	TFCond_RuneAgility,
	TFCond_GrapplingHook,
	TFCond_GrapplingHookSafeFall,
	TFCond_GrapplingHookLatched,
	TFCond_GrapplingHookBleeding,
	TFCond_AfterburnImmune,
	TFCond_RuneKnockout,
	TFCond_RuneImbalance,
	TFCond_CritRuneTemp,
	TFCond_PasstimeInterception,
	TFCond_SwimmingNoEffects,
	TFCond_EyeaductUnderworld,
	TFCond_KingRune,
	TFCond_PlagueRune,
	TFCond_SupernovaRune,
	TFCond_Plague,
	TFCond_KingAura,
	TFCond_SpawnOutline,
	TFCond_KnockedIntoAir,
	TFCond_CompetitiveWinner,
	TFCond_CompetitiveLoser,
	TFCond_NoTaunting_DEPRECATED,
	TFCond_HealingDebuff = 118,
	TFCond_PasstimePenaltyDebuff,
	TFCond_GrappledToPlayer,
	TFCond_GrappledByPlayer,
	TFCond_ParachuteDeployed,
	TFCond_Gas,
	TFCond_BurningPyro,
	TFCond_RocketPack,
	TFCond_LostFooting,
	TFCond_AirCurrent,
}TFCond;

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
	TF_TRAP_TYPE_NONE = -1,
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
DEFINE_ENUM_INCREMENT_OPERATORS(TFClass)

typedef enum
{
	TF_TEAM_UNASSIGNED = 0,
	TF_TEAM_SPEC,
	TF_TEAM_RED,
	TF_TEAM_BLUE,
	TF_TEAM_GREEN,
	TF_TEAM_YELLOW,
}TFTeam;

typedef enum
{
	OBJ_DISP = 0,
	OBJ_TELE,
	OBJ_SENTRY,
	OBJ_SAPPER,
	OBJ_ENTRANCE,
	OBJ_EXIT,
}eObjectType;

typedef enum
{
	ENGI_BUILD,
	ENGI_DESTROY
}eEngiCmd;

class CBotTF2FunctionEnemyAtIntel : public IBotFunction
{
public:
	CBotTF2FunctionEnemyAtIntel(int iTeam, Vector vPos, int type, edict_t *pPlayer = NULL, int capindex = -1)
		: m_iTeam(iTeam), m_vPos(vPos), m_iType(type), m_pPlayer(pPlayer), m_iCapIndex(capindex) {}

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
	CBroadcastSpySap(edict_t *pSpy) : m_pSpy(pSpy) {}

	void Execute(CBot *pBot);
private:
	edict_t *m_pSpy;
};

class CBroadcastOvertime : public IBotFunction
{
public:
	CBroadcastOvertime() {}

	void Execute(CBot *pBot);
};

class CBroadcastFlagReturned : public IBotFunction
{
public:
	CBroadcastFlagReturned(int iTeam) : m_iTeam(iTeam) {}

	void Execute(CBot *pBot);
private:
	int m_iTeam;
};

class CBroadcastFlagDropped : public IBotFunction
{
public:
	CBroadcastFlagDropped(int iTeam, Vector origin)
		: m_iTeam(iTeam), m_vOrigin(origin) {}

	void Execute(CBot *pBot);
private:
	Vector m_vOrigin;
	int m_iTeam;
};

class CBroadcastFlagCaptured : public IBotFunction
{
public:
	CBroadcastFlagCaptured(int iTeam) : m_iTeam(iTeam) {}

	void Execute(CBot *pBot);
private:
	int m_iTeam;
};

class CBroadcastRoundStart : public IBotFunction
{
public:
	CBroadcastRoundStart(bool bFullReset) : m_bFullReset(bFullReset) {}

	void Execute(CBot *pBot);
private:
	bool m_bFullReset;
};

class CBroadcastCapturedPoint : public IBotFunction
{
public:
	CBroadcastCapturedPoint(int iPoint, int iTeam, const char *szName)
		: m_iPoint(iPoint), m_iTeam(iTeam), m_szName(szName) {}

	void Execute(CBot *pBot);
private:
	int m_iPoint;
	int m_iTeam;
	const char *m_szName;
};

class CWeapon;
class CBotWeapon;
class CWaypoint;
class CBotUtility;

class CBotFortress : public CBot
{
public:
	CBotFortress();

	virtual void EnemyLost(edict_t *pEnemy);

	virtual void UpdateConditions();

	virtual void Shot(edict_t *pEnemy);

	virtual int EngiBuildObject(int *iState, eObjectType iObject, float *fTime, int *iTries);

	virtual float GetEnemyFactor(edict_t *pEnemy) { return CBot::GetEnemyFactor(pEnemy); }

	virtual void CheckDependantEntities();

	int GetMetal();

	virtual void ModAim(edict_t *pEntity, Vector &v_origin, Vector *v_desired_offset, Vector &v_size, float fDist, float fDist2D) { CBot::ModAim(pEntity, v_origin, v_desired_offset, v_size, fDist, fDist2D); }

#if defined USE_NAVMESH
	virtual void TouchedWpt(INavMeshArea *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1) { CBot::TouchedWpt(pWaypoint, iNextWaypoint, iPrevWaypoint); }
#else
	virtual void TouchedWpt(CWaypoint *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1) { CBot::TouchedWpt(pWaypoint, iNextWaypoint, iPrevWaypoint); }
#endif

	inline edict_t *GetHealingEntity() { return m_pHeal; }

	inline void ClearHealingEntity() { m_pHeal = MyEHandle(); }

	virtual short int MaxEntityIndex() { return gpGlobals->maxEntities; }

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

	virtual edict_t *FindEngineerBuiltObject(eObjectType iBuilding, int index) { return NULL; }

	virtual void EngineerBuild(eObjectType iBuilding, eEngiCmd iEngiCmd) {};

	virtual void SpyDisguise(int iTeam, int iClass) {};

	virtual bool LookAfterBuildings(float *fTime) { return false; }

	inline void NextLookAfterSentryTime(float fTime) { m_ftLookAfterSentryTime.Start(fTime); }

	inline edict_t *GetSentry() { return m_pSentryGun; }

	virtual bool HasEngineerBuilt(eObjectType iBuilding) { return false; }

	virtual void EngiBuildSuccess(eObjectType iObject, int index) {};

	virtual bool HealPlayer(edict_t *pPlayer, edict_t *pPrevPlayer) { return false; }
	virtual bool UpgradeBuilding(edict_t *pBuilding, bool removesapper = false) { return false; }

	virtual bool IsCloaked() { return false; }
	virtual bool IsDisguised() { return false; }

	virtual CBotWeapon *GetCurrentWeapon() { return CBot::GetCurrentWeapon(); }

	virtual bool HandleAttack(CBotWeapon *pWeapon, edict_t *pEnemy) { return CBot::HandleAttack(pWeapon, pEnemy); }

	void ResetAttackingEnemy() { m_pAttackingEnemy = MyEHandle(); }

	virtual bool SetVisible(edict_t *pEntity, bool bVisible);

	virtual void SetClass(TFClass _class);

	inline edict_t *SeeFlag(bool reset = false) { if (reset) { m_pFlag = MyEHandle(); } return m_pFlag; }

	virtual bool CanAvoid(edict_t *pEntity);

	virtual bool IsEnemy(edict_t *pEdict, bool bCheckWeapons = true);

	virtual bool StartGame();

	virtual void SpawnInit();

	bool IsTF() { return true; }

	virtual bool IsTF2() { return false; }

	virtual bool Hurt(edict_t *pAttacker, int iHealthNow, bool bDontHide = false) { return CBot::Hurt(pAttacker, iHealthNow, bDontHide); }

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
			m_pPrevSpy = MyEHandle();
			m_ftSeeSpyTime.Invalidate();
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

#if defined USE_NAVMESH
	virtual bool CanGotoWaypoint(Vector vPrevWaypoint, INavMeshArea *pWaypoint, INavMeshArea *pPrev = NULL) { return CBot::CanGotoWaypoint(vPrevWaypoint, pWaypoint, pPrev); }
#else
	virtual bool CanGotoWaypoint(Vector vPrevWaypoint, CWaypoint *pWaypoint, CWaypoint *pPrev = NULL) { return CBot::CanGotoWaypoint(vPrevWaypoint, pWaypoint, pPrev); }
#endif

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

	virtual void VoiceCommand(int cmd) { };

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

	float m_fLastSeeSpyTime;
	FrameTimer m_ftLastSaySpy;

	// medic voice command throttle
	FrameTimer m_ftCallMedic;
	FrameTimer m_ftTauntTime;
	FrameTimer m_ftTaunting;

	// how long should I hang around?
	FrameTimer m_ftDefendTime;

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

	// panic at the disco
	FrameTimer m_ftFrenzyTime;
	FrameTimer m_ftSpyCloakTime;
	FrameTimer m_ftSpyUncloakTime;
	// time until we aren't paranoid
	FrameTimer m_ftSeeSpyTime;
	FrameTimer m_ftSpyDisguiseTime;
	FrameTimer m_ftPickupTime;
	FrameTimer m_ftLookAfterSentryTime;
	// wait for the back
	FrameTimer m_ftBackstabTime;
	FrameTimer m_ftUpdateClass;
	FrameTimer m_ftUseTeleporterTime;
	// time left before the bot decides if it wants to change class
	FrameTimer m_ftChangeClassTime;
	FrameTimer m_ftSnipeAttackTime;

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

	TFClass m_iClass;

	bool m_bHasFlag;

	// bot should check if he can change class now
	bool m_bCheckClass;

	MyEHandle m_pLastCalledMedic;
	CBotLastSee m_pLastSeeMedic;
	/*MyEHandle m_pLastSeeMedic;
	Vector m_vLastSeeMedic;
	float m_fLastSeeMedicTime;*/
	float m_fLastCalledMedicTime;
	bool m_bIsBeingHealed;
	FrameTimer m_ftMedicUpdatePosTime;
	Vector m_vMedicPosition;
	bool m_bCanBeUbered;
	FrameTimer m_ftCheckHealTime;

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

	TFTeam m_iTeam;

	// amount of time to wait before engineer turns their sentry before building
	float m_fWaitTurnSentry;

	// currently unused
	float m_fCallMedicTime[MAX_PLAYERS]; // for every player ID is kept the last time they called medic

	int m_iLastFailSentryWpt;
	int m_iLastFailTeleExitWpt;

	MyEHandle m_pHealer;

	float m_fHealingMoveTime;

	MyEHandle m_pLastEnemySentry;
	MyEHandle m_pNearestEnemyRocket;
	MyEHandle m_NearestEnemyGrenade;

	float m_fLastSentryEnemyTime;
	//bool m_bWantToZoom;
};

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
	CBotTF2();

	virtual CBotWeapon *GetCurrentWeapon();

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

	void EnemyAtIntel(Vector vPos, int type = FLAGEVENT_PICKUP, int iArea = -1);

	bool IsTF2() { return true; }

	void CheckDependantEntities();

	virtual bool WantToListenToPlayerAttack(edict_t *pPlayer, int iWeaponID = -1);
	virtual bool WantToListenToPlayerFootsteps(edict_t *pPlayer);

	bool WantToInvestigateSound();

	void GetDefendArea(std::vector<int> *m_iAreas);

	void GetAttackArea(std::vector <int> *m_iAreas);

	int GetCurrentAttackArea() { return m_iCurrentAttackArea; }
	int GetCurrentDefendArea() { return m_iCurrentDefendArea; }

	void PointsUpdated();

	eBotFuncState RocketJump(int *iState, float *fTime);

	virtual bool WantToFollowEnemy();

	void ResetCloakTime() { m_ftSpyCloakTime.Invalidate(); }

	float GetEnemyFactor(edict_t *pEnemy);

	void FoundSpy(edict_t *pEdict, TFClass iDisguise);

#if defined USE_NAVMESH
	void TouchedWpt(INavMeshArea *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1);
#else
	void TouchedWpt(CWaypoint *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1);
#endif

	bool HandleAttack(CBotWeapon *pWeapon, edict_t *pEnemy);

	void EngiBuildSuccess(eObjectType iObject, int index);

	bool LookAfterBuildings(float *fTime);

	void SpawnInit();

	bool SetVisible(edict_t *pEntity, bool bVisible);

	virtual void ModAim(edict_t *pEntity, Vector &v_origin, Vector *v_desired_offset, Vector &v_size, float fDist, float fDist2D);

	void ModThink();

	bool IsCloaked();

	bool ExecuteAction(CBotUtility *util);

	void SetClass(TFClass _class);

	bool IsDisguised();

	void CheckBuildingsValid(bool bForce = false);

	edict_t *FindEngineerBuiltObject(eObjectType iBuilding, int index);

	bool IsEnemy(edict_t *pEdict, bool bCheckWeapons = true);

	bool IsTF() { return true; }

	void Taunt(bool bOverride = false);

	void CallMedic();

	void RoundReset(bool bFullReset);

	void PointCaptured(int iPoint, int iTeam, const char *szPointName);

	void EngineerBuild(eObjectType iBuilding, eEngiCmd iEngiCmd);

	void SpyDisguise(int iTeam, int iClass);

	bool HasEngineerBuilt(eObjectType iBuilding);

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

	void BuildingSapped(eObjectType building, edict_t *pSapper, edict_t *pSpy);

	void SapperDestroyed(edict_t *pSapper);

#if defined USE_NAVMESH
	bool CanGotoWaypoint(Vector vPrevWaypoint, INavMeshArea *pWaypoint, INavMeshArea *pPrev = NULL);
#else
	bool CanGotoWaypoint(Vector vPrevWaypoint, CWaypoint *pWaypoint, CWaypoint *pPrev = NULL);
#endif

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

	inline bool IsCarrying() { return m_bIsCarryingObj; }

	void UpdateCarrying();

	inline void ResetCarryTime() { m_fCarryTime = 0.0f; }

	KeyValues *SelectNextUpgrade(bool bReset = false);
	void UpgradeWeapon();
	bool CanUpgradeWeapon();

	bool DidReadyUp() { return !!m_bMvMReady; }
	void ReadyUp(bool bReady);

private:
	
	FrameTimer m_ftDoubleJumpTime;
	
	FrameTimer m_ftSpySapTime;
	FrameTimer m_ftRemoveSapTime;
	 
	int m_iCurrentDefendArea;
	int m_iCurrentAttackArea;
	
	//bool m_bBlockPushing;
	//float m_fBlockPushTime;
	
	MyEHandle m_pDefendPayloadBomb;
	MyEHandle m_pPushPayloadBomb;
	MyEHandle m_pRedPayloadBomb;
	MyEHandle m_pBluePayloadBomb;
	
	//bool m_bDeployedStickies;
	eDemoTrapType m_iTrapType;
	int m_iTrapCPIndex;
	Vector m_vStickyLocation;
	
	float m_fRevMiniGunTime;
	float m_fNextRevMiniGunTime;

	float m_fRevMiniGunBelief;
	float m_fCloakBelief;

	MyEHandle m_pCloakedSpy;

	FrameTimer m_ftAttackPointTime;

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
	FrameTimer m_ftCheckNextCarrying;

	FrameTimer m_ftUseBuffItemTime;

	CTF2Loadout *m_pMelee;
	CTF2Loadout *m_pPrimary;
	CTF2Loadout *m_pSecondary;

	int m_iDesiredResistType;
	
	bool m_bMvMReady;

	int m_iMvMCurrUpgrade;
	int m_iMvMNextUpgrade;
	
	int m_nMvMTotalUpgrades;
};

class CBotFF : public CBotFortress
{
public:
	CBotFF() : CBotFortress() { }

	void ModThink();

	bool IsEnemy(edict_t *pEdict, bool bCheckWeapons = true);

	bool IsTF() { return true; }
};

#endif