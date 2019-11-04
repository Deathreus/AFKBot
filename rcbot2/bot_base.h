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

//=============================================================================//
//
// HPB_bot2.h - bot header file (Copyright 2004, Jeffrey "botman" Broome)
//
//=============================================================================//

#ifndef __RCBOT2_H__
#define __RCBOT2_H__

#include <queue>
#include <stack>
#include <typeinfo>

#include <eiface.h>
#include <usercmd.h>
#include <fmtstr.h>

#include "bot_utility.h"
#include "bot_const.h"
#include "bot_ehandle.h"

#include <vstdlib/random.h>

#include <PlayerState.h>

#include "../extension.h"

extern bool BotFunc_BreakableIsEnemy(edict_t *pBreakable, edict_t *pEdict);

inline const char *GetTypeName(const std::type_info &type)
{
	const char *temp = type.name();
	while(*temp != ' ')
		temp++;
	return ++temp;
}

inline const int SlotOfEdict(edict_t *pEdict)
{
	return engine->IndexOfEdict(pEdict) - 1;
}

abstract_class IBotFunction
{
public:
	virtual void Execute(CBot *pBot) = 0;
};

class CBroadcastVoiceCommand : public IBotFunction
{
public:
	CBroadcastVoiceCommand(edict_t *pPlayer, byte voicecmd)
	{
		m_pPlayer = pPlayer;
		m_VoiceCmd = voicecmd;
	}

	void Execute(CBot *pBot);

private:
	edict_t *m_pPlayer;
	byte m_VoiceCmd;
};

typedef union
{
	struct
	{
		unsigned v1 : 2; // menu
		unsigned v2 : 3; // extra info
		unsigned unused : 3;
	}b1;

	byte voicecmd;
}voicecmd_u;

typedef union
{
	struct
	{
		unsigned said_in_position : 1;
		unsigned said_move_out : 1;
		unsigned said_area_clear : 1;
		unsigned unused : 5;
	}b1;

	byte dat;
}squad_u;

class CBotVisibles;
class CFindEnemyFunc;
class IBotNavigator;
class CBotSquad;
class CBotSchedules;
class CBotButtons;
class CFindEnemyFunc;
class CBotWeapons;
class CBotProfile;
class CWaypoint;
class CBotWeapon;
class CWeapon;
class INavMeshArea;

class FrameTimer
{
public:
	FrameTimer()
	{
		m_timestamp = -1.0f;
		m_duration = 0.0f;
	}

	void Reset()
	{
		m_timestamp = TIME_NOW + m_duration;
	}

	void Start(float duration)
	{
		m_timestamp = TIME_NOW + duration;
		m_duration = duration;
	}

	void Invalidate()
	{
		m_timestamp = -1.0f;
	}

	const bool HasStarted() const
	{
		return (m_timestamp > 0.0f);
	}

	const bool IsElapsed() const
	{
		return (TIME_NOW > m_timestamp);
	}

	FrameTimer& operator=(const float time)
	{
		this->Start(time);
		return *this;
	}

private:
	float m_duration;
	float m_timestamp;
};

class CBotLastSee
{
public:
	CBotLastSee()
	{
		Reset();
	}

	inline void Reset()
	{
		m_pLastSee = MyEHandle(); // edict
		m_fLastSeeTime = 0.0f; // time
	}

	CBotLastSee(edict_t *pEdict);

	void Update();

	inline bool Check(edict_t *pEdict)
	{
		return (pEdict == (m_pLastSee.Get()));
	}

	bool HasSeen(float fTime);

	Vector GetLocation();

private:
	MyEHandle m_pLastSee; // edict
	float m_fLastSeeTime; // time
	Vector m_vLastSeeLoc; // location
	Vector m_vLastSeeVel; // velocity
};

typedef struct
{
	short int m_iTeamMatesInRange;
	short int m_iEnemiesInRange;
	short int m_iEnemiesVisible;
	short int m_iTeamMatesVisible;
} bot_statistics_t;

class CBot
{
public:

	static const float m_fAttackLowestHoldTime;
	static const float m_fAttackHighestHoldTime;
	static const float m_fAttackLowestLetGoTime;
	static const float m_fAttackHighestLetGoTime;

	CBot();

	inline void ClearFailedWeaponSelect() { m_iPrevWeaponSelectFailed = 0; }
	inline void FailWeaponSelect() { m_iPrevWeaponSelectFailed++; }

	virtual short int MaxEntityIndex() { return MAX_PLAYERS; }

	virtual void OnInventoryApplication() { }

	int IsDesiredClass(int iclass)
	{
		return m_iDesiredClass == iclass;
	}

	virtual void HandleWeapons();

	inline Vector GetOrigin()
	{
		return m_pPI->GetAbsOrigin();
	}

	// return distance from this origin
	inline float DistanceFrom(Vector vOrigin)
	{
		return (vOrigin - m_pPI->GetAbsOrigin()).Length();
	}
	inline float DistanceFrom(edict_t *pEntity)
	{
		return (pEntity->GetCollideable()->GetCollisionOrigin() - GetOrigin()).Length();
	}

	inline float DistanceFrom2D(edict_t *pEntity)
	{
		return (pEntity->GetCollideable()->GetCollisionOrigin() - GetOrigin()).Length2D();
	}

	/*
	 * initialize all bot variables
	 * (this is called when bot is made for the first time)
	 */
	virtual void Init(bool bVarInit = false);
	// setup buttons and data structures
	virtual void Setup();

	/*
	 * fill out the bots usercmd buffer with desired values
	 * botman : see CBasePlayer::RunNullCommand() for example of PlayerRunCommand()...
	 */
	inline void RunPlayerMove();

	/*
	 * called when a bot dies
	 */
	virtual void Died(edict_t *pKiller, const char *pszWeapon);
	virtual void Killed(edict_t *pVictim, char *weapon);

	virtual int GetTeam();

	bool IsUnderWater();

	CBotWeapon *GetBestWeapon(edict_t *pEnemy, bool bAllowMelee = true, bool bAllowMeleeFallback = true, bool bMeleeOnly = false, bool bExplosivesOnly = false);

	virtual void ModThink() { }

	virtual bool IsEnemy(edict_t *pEdict, bool bCheckWeapons = true) { return false; }

	inline bool HasSomeConditions(int iConditions)
	{
		return (m_iConditions & iConditions) > 0;
	}

	virtual bool HandleAttack(CBotWeapon *pWeapon, edict_t *pEnemy);

	float DotProductFromOrigin(const Vector pOrigin);

	bool IsVisible(edict_t *pEdict);

	inline void SetEnemy(edict_t *pEnemy)
	{
		m_pEnemy = pEnemy;
	}

	inline int GetConditions()
	{
		return m_iConditions;
	}

	inline bool HasAllConditions(int iConditions)
	{
		return (m_iConditions & iConditions) == iConditions;
	}

	inline void UpdateCondition(int iCondition)
	{
		m_iConditions |= iCondition;
	}

	inline void RemoveCondition(int iCondition)
	{
		m_iConditions &= ~iCondition;
	}

	bool FInViewCone(edict_t *pEntity);

	bool FVisible(Vector vOrigin, edict_t *pDest = NULL);
	bool FVisible(edict_t *pEdict, bool bCheckHead = false);

	/*
	 * make bot start the gmae, e.g join a team first
	 */
	virtual bool StartGame();

	virtual bool CheckStuck();

	virtual void CurrentlyDead();

	/*
	 * initialize this bot as a new bot with the edict of pEdict
	 */
	bool CreateBotFromEdict(edict_t *pEdict, CBotProfile *pProfile);

	/*
	 * returns true if bot is used in game
	 */
	inline bool InUse()
	{
		return (m_bUsed && m_pEdict);
	}

	edict_t *GetEdict();

	void SetEdict(edict_t *pEdict);

	Vector GetEyePosition();

	void Think();

	virtual void FriendlyFire(edict_t *pEdict) { }

	virtual void FreeMapMemory();

	virtual void FreeAllMemory();

	///////////////////////////////
	inline bool MoveToIsValid() { return m_bMoveToIsValid; }

	inline bool LookAtIsValid() { return m_bLookAtIsValid; }

	inline Vector *GetMoveTo() { return &m_vMoveTo; }

	inline bool MoveFailed()
	{
		bool ret = m_bFailNextMove;

		m_bFailNextMove = false;

		return ret;
	}

	void SelectWeaponSlot(int iSlot);

	edict_t *GetAvoidEntity() { return m_pAvoidEntity; }

	void SetAvoidEntity(edict_t *pEntity) { m_pAvoidEntity = pEntity; }

	virtual void UpdateConditions();

	virtual bool CanAvoid(edict_t *pEntity);

	inline bool HasEnemy() { return m_pEnemy && HasSomeConditions(CONDITION_SEE_CUR_ENEMY); }
	inline edict_t *GetEnemy() { return m_pEnemy; }

	inline void SetMoveTo(Vector vNew)
	{
		if (m_iMoveLookPriority >= m_iMovePriority)
		{
			m_vMoveTo = vNew;
			m_bMoveToIsValid = true;
			m_iMovePriority = m_iMoveLookPriority;
		}
	}

	// this allows move speed to be changed in tasks
	inline void SetMoveSpeed(float fNewSpeed)
	{
		if (m_iMoveLookPriority >= m_iMoveSpeedPriority)
		{
			m_fIdealMoveSpeed = fNewSpeed;
			m_iMoveSpeedPriority = m_iMoveLookPriority;
		}
	}

	void FindEnemy(edict_t *pOldEnemy = NULL);
	virtual void EnemyFound(edict_t *pEnemy);

	virtual void CheckDependantEntities();

	inline IBotNavigator *GetNavigator() { return m_pNavigator; }

	inline void SetMoveLookPriority(int iPriority) { m_iMoveLookPriority = iPriority; }

	inline void StopMoving()
	{
		if (m_iMoveLookPriority >= m_iMovePriority)
		{
			m_bMoveToIsValid = false;
			m_iMovePriority = m_iMoveLookPriority;
			m_ftWaypointStuckTime.Invalidate();
			m_ftCheckStuckTime.Reset();
		}
	}

	inline void StopLooking()
	{
		if (m_iMoveLookPriority >= m_iLookPriority)
		{
			m_bLookAtIsValid = false;
			m_iLookPriority = m_iMoveLookPriority;
		}
	}

	inline void SetLookAtTask(eLookTask lookTask, float fTime = 0)
	{
		if ((m_iMoveLookPriority >= m_iLookPriority) && ((fTime > 0) || m_ftLookSetTime.IsElapsed()))
		{
			m_iLookPriority = m_iMoveLookPriority;
			m_iLookTask = lookTask;

			if (fTime > 0)
				m_ftLookSetTime.Start(fTime);
		}
	}

	virtual void EnemyLost(edict_t *pEnemy) { }

	void SetLastEnemy(edict_t *pEnemy);

	virtual void EnemyDown(edict_t *pEnemy)
	{
		if (pEnemy == m_pEnemy)
			UpdateCondition(CONDITION_ENEMY_DEAD);
		if (pEnemy == m_pLastEnemy)
		{
			m_pLastEnemy = MyEHandle(NULL);
		}
	}
	
	virtual bool IsCSS() { return false; }
	virtual bool IsHLDM() { return false; }
	virtual bool IsTF() { return false; }

	virtual void SpawnInit();

	QAngle GetEyeAngles();

	virtual bool IsAlive();

	bool OnLadder();

	inline bool CurrentEnemy(edict_t *pEntity) { return m_pEnemy == pEntity; }

	Vector *GetAimVector(edict_t *pEntity);

	virtual void ModAim(edict_t *pEntity, Vector &v_origin,
		Vector *v_desired_offset, Vector &v_size,
		float fDist, float fDist2D);

	inline Vector *GetGoalOrigin()
	{
		return &m_vGoal;
	}

	inline bool HasGoal()
	{
		return m_bHasGoal;
	}

	bool IsHoldingPrimaryAttack();

	void PrimaryAttack(bool bHold = false, float fTime = 0.0f);
	void SecondaryAttack(bool bHold = false);
	void Jump();
	void Duck(bool hold = false);
	void Use();
	void Reload();

	virtual bool SetVisible(edict_t *pEntity, bool bVisible);

	virtual bool Hurt(edict_t *pAttacker, int iHealthNow, bool bDontHide = false);
	virtual void Shot(edict_t *pEnemy);
	virtual void ShotMiss();

	int GetPlayerID(); // return player ID on server
	int GetHealth();

	float GetHealthPercent();

	inline CBotSchedules *GetSchedules() { return m_pSchedules; }

	virtual void ReachedCoverSpot(int flags);

	virtual bool WantToFollowEnemy();

	virtual void SeeFriendlyHurtEnemy(edict_t *pTeammate, edict_t *pEnemy, CWeapon *pWeapon) { }
	virtual void SeeEnemyHurtFriendly(edict_t *pTeammate, edict_t *pEnemy, CWeapon *pWeapon) { }
	virtual void SeeFriendlyDie(edict_t *pDied, edict_t *pKiller, CWeapon *pWeapon) { }
	virtual void SeeFriendlyKill(edict_t *pTeamMate, edict_t *pDied, CWeapon *pWeapon) { }

	inline void SelectWeapon(int iWeaponId) { m_iSelectWeapon = iWeaponId; }

	void SelectWeaponName(const char *szWeaponName);

	virtual CBotWeapon *GetCurrentWeapon();

	void Kill();

	bool IsUsingProfile(CBotProfile *pProfile);

	inline CBotProfile *GetProfile() { return m_pProfile; }

#if defined USE_NAVMESH
	virtual bool CanGotoWaypoint(Vector vPrevWaypoint, INavMeshArea *pWaypoint, INavMeshArea *pPrev = NULL);
#else
	virtual bool CanGotoWaypoint(Vector vPrevWaypoint, CWaypoint *pWaypoint, CWaypoint *pPrev = NULL);
#endif

	void TapButton(int iButton);

	inline int GetAmmo(int iIndex) { if (!m_iAmmo) return 0; else if (iIndex == -1) return 0;  return m_iAmmo[iIndex]; }

	inline void LookAtEdict(edict_t *pEdict) { m_pLookEdict = pEdict; }

	virtual bool Select_CWeapon(CWeapon *pWeapon);
	virtual bool SelectBotWeapon(CBotWeapon *pBotWeapon);

	void UpdatePosition();

	CBotWeapons *GetWeapons() { return m_pWeapons; }

	virtual float GetEnemyFactor(edict_t *pEnemy);

	virtual void CheckCanPickup(edict_t *pPickup);

#if defined USE_NAVMESH
	virtual void TouchedWpt(INavMeshArea *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1);
#else
	virtual void TouchedWpt(CWaypoint *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1);
#endif

	inline void SetAiming(Vector aiming) { m_vWaypointAim = aiming; }

	inline Vector *GetAiming() { return &m_vWaypointAim; }

	inline void SetLookVector(Vector vLook) { m_vLookVector = vLook; }

	inline Vector *GetLookVector() { return &m_vLookVector; }

	inline void ResetLookAroundTime() { m_ftLookAroundTime.Invalidate(); }

	Vector Snipe(Vector &vAiming);

	inline float GetSpeed() { return m_vVelocity.Length2D(); }

	void UpdateStatistics(); // updates number of teammates/enemies nearby/visible
	virtual void ListenForPlayers();
	// listens to this player
	void ListenToPlayer(edict_t *pListenTo, bool bIsEnemy = false, bool bIsAttacking = false);
	virtual bool WantToListenToPlayerAttack(edict_t *pPlayer, int iWeaponID = -1) { return true; }
	virtual bool WantToListenToPlayerFootsteps(edict_t *pPlayer) { return true; }

	virtual bool WantToInvestigateSound();
	inline void WantToInvestigateSound(bool bSet) { m_bWantToInvestigateSound = bSet; }
	inline bool WantToShoot(void) { return m_bOpenFire; }
	inline void WantToShoot(bool bSet) { m_bOpenFire = bSet; }
	inline void WantToListen(bool bSet) { m_bWantToListen = bSet; }
	bool WantToListen();
	inline void WantToChangeWeapon(bool bSet) { m_bWantToChangeWeapon = bSet; }

	int NearbyFriendlies(float fDistance);

	bool IsFacing(const Vector vOrigin);

	bool IsOnLift(void);

	virtual bool IsDOD() { return false; }

	virtual bool IsTF2() { return false; }

	// return an enemy sentry gun / special visible (e.g.) for quick checking - voffset is the 'head'
	virtual edict_t *GetVisibleSpecial();

	void UpdateDanger(float fBelief);

	inline void ReduceTouchDistance() { if (m_fWaypointTouchDistance > MIN_WPT_TOUCH_DIST) { m_fWaypointTouchDistance *= 0.9; } }

	inline void ResetTouchDistance(float fDist) { m_fWaypointTouchDistance = fDist; }

	inline float GetTouchDistance() { return m_fWaypointTouchDistance; }

	inline CUserCmd *GetUserCMD() { return &m_pCmd; }
	inline IPlayerInfo *GetPlayerInfo() { return m_pPI; }
	inline CPlayerState *GetPlayerState() { return m_pPS; }

	inline float GetForwardMove() { return m_fForwardSpeed; }
	inline float GetSideMove() { return m_fSideSpeed; }
	inline float GetUpMove() { return m_fUpSpeed; }

	inline int GetButtons() { return m_iButtons; }
	inline int GetImpulse() { return m_iImpulse; }
	inline int GetSelectWeapon() { return m_iSelectWeapon; }

	inline QAngle *GetViewAngles() { return &m_vViewAngles; }

	void ForceGotoWaypoint(int wpt);

	// bot is defending -- mod specific stuff
	virtual void Defending() { }

	virtual void HearVoiceCommand(edict_t *pPlayer, byte cmd) {};

	virtual void GrenadeThrown();

	virtual void VoiceCommand(int cmd) { };

	void AddVoiceCommand(int cmd);

	void LetGoOfButton(int button);

	virtual bool OverrideAmmoTypes() { return true; }

	virtual void DebugBot();

#if defined USE_NAVMESH
	virtual bool WalkingTowardsWaypoint(INavMeshArea *pWaypoint, bool *bOffsetApplied, Vector &vOffset);
#else
	virtual bool WalkingTowardsWaypoint(CWaypoint *pWaypoint, bool *bOffsetApplied, Vector &vOffset);
#endif

	void SetCoverFrom(edict_t *pCoverFrom) { m_pLastCoverFrom = MyEHandle(pCoverFrom); }

	virtual void AreaClear() { }

	inline void ResetAreaClear() { m_uSquadDetail.b1.said_area_clear = false; }

	void SquadInPosition();
	virtual void SayInPosition() { }
	virtual void SayMoveOut() { }

	bot_statistics_t *GetStats() { return &m_Stats; }

	virtual void HearPlayerAttack(edict_t *pAttacker, int iWeaponID);

	inline bool IsListeningToPlayer(edict_t *pPlayer)
	{
		return (m_PlayerListeningTo.Get() == pPlayer);
	}

	void UpdateUtilTime(int util);

	virtual bool GetIgnoreBox(Vector *vLoc, float *fSize)
	{
		return false;
	}

	bool RecentlyHurt(float fTime);

	eBotAction GetCurrentUtil(void) { return m_CurrentUtil; }

	bool RecentlySpawned(float fTime);

	FrameTimer m_ftWaypointStuckTime;

protected:

	inline void SetLookAt(Vector vNew)
	{
		m_vLookAt.Init(vNew.x, vNew.y, vNew.z);
		m_bLookAtIsValid = true;
	}

	inline void SetLookAt(Vector *vNew)
	{
		m_vLookAt.Init(vNew->x, vNew->y, vNew->z);
		m_bLookAtIsValid = true;
	}

	static void CheckEntity(edict_t **pEdict);
	
	void DoMove();

	void DoLook();

	virtual void GetLookAtVector();

	void DoButtons();

	void ChangeAngles(float fSpeed, float *fIdeal, float *fCurrent, float *fUpdate);

	// look for new tasks
	virtual void GetTasks(unsigned int iIgnore = 0);


	// really only need 249 bits (32 bytes) + WEAPON_SUBTYPE_BITS (whatever that is)
	static const int CMD_BUFFER_SIZE = 64;
	///////////////////////////////////
	// bots edict
	MyEHandle m_pEdict;
	// is bot used in the game?
	bool m_bUsed;
	// time the bot was made in the server
	float m_fTimeCreated;
	// next think time
	float m_fNextThink;

	char m_szBotName[64];

	float m_fSpawnTime;

	float m_fFov;

	bool m_bInitAlive;

	int m_iMovePriority;
	int m_iLookPriority;
	int m_iMoveSpeedPriority;
	int m_iMoveLookPriority;

	int m_iFlags;

	int *m_iAmmo;

	///////////////////////////////////
	// usercmds
	int m_iImpulse;
	int m_iButtons;
	float m_fForwardSpeed;
	float m_fSideSpeed;
	float m_fUpSpeed;
	Vector m_vVelocity;
	float m_fAimMoment;

	FrameTimer m_ftStrafeTime;

	float m_fLookAtTimeStart;
	float m_fLookAtTimeEnd;
	// Look task can't be changed if this is greater than Time()
	FrameTimer m_ftLookSetTime;
	FrameTimer m_ftLookAroundTime;
	MyEHandle m_pLookEdict;

	// Origin a second ago to check if stuck
	Vector m_vLastOrigin;
	// next update time (1 second update)
	FrameTimer m_ftUpdateOriginTime;
	float m_fStuckTime;
	FrameTimer m_ftCheckStuckTime;
	FrameTimer m_ftNextUpdateStuckConstants;
	bool m_bThinkStuck;
	Vector m_vStuckPos;
	//int m_iTimesStuck;

	FrameTimer m_ftUpdateDamageTime;
	// Damage bot accumulated over the last second or so
	int m_iAccumulatedDamage;
	int m_iPrevHealth;
	///////////////////////////////////
	int m_iConditions;
	// bot tasks etc -- complex actuators
	CBotSchedules *m_pSchedules;
	// buttons held -- simple actuators
	CBotButtons *m_pButtons;
	// Navigation used for this bot -- environment sensor 1
	IBotNavigator *m_pNavigator;
	// bot visible list -- environment sensor 2
	CBotVisibles *m_pVisibles;
	// visible functions -- sensory functions
	CFindEnemyFunc *m_pFindEnemyFunc;
	// weapons storage -- sensor
	CBotWeapons *m_pWeapons;
	////////////////////////////////////
	// engine exposed info
	IPlayerInfo *m_pPI;
	CPlayerState *m_pPS;
	CUserCmd m_pCmd;
	////////////////////////////////////
	MyEHandle m_pEnemy; // current enemy
	MyEHandle m_pOldEnemy;
	MyEHandle m_pAvoidEntity;
	Vector m_vLastSeeEnemy;
	Vector m_vLastSeeEnemyBlastWaypoint;
	MyEHandle m_pLastEnemy; // enemy we were fighting before we lost it
	float m_fLastSeeEnemy;
	FrameTimer m_ftLastUpdateLastSeeEnemy;
	float m_fLastSeeEnemyPlayer;
	bool m_bLookedForEnemyLast;
	//edict_t *m_pAvoidEntity; // avoid this guy
	Vector m_vHurtOrigin;
	Vector m_vLookVector;
	Vector m_vLookAroundOffset;
	MyEHandle m_pPickup;
	Vector m_vWaypointAim;

	Vector m_vMoveTo;
	Vector m_vLookAt;
	Vector m_vGoal; // goal vector
	bool m_bHasGoal;
	QAngle m_vViewAngles;
	FrameTimer m_ftNextUpdateAimVector;
	float m_fStartUpdateAimVector;
	Vector m_vAimVector;
	Vector m_vPrevAimVector;
	Vector m_vLastDiedOrigin;
	bool m_bPrevAimVectorValid;

	eLookTask m_iLookTask;

	bool m_bMoveToIsValid;
	bool m_bLookAtIsValid;

	bool m_bFailNextMove;

	float m_fIdealMoveSpeed;

	float m_fPercentMoved;

	float m_fLastWaypointVisible;

	int m_iSelectWeapon;

	int m_iDesiredTeam;
	int m_iDesiredClass;

	// bots profile data
	CBotProfile *m_pProfile;
	// bots preferred weapon
	CBotWeapon *m_pPrimaryWeapon;

	bool m_bOpenFire;
	bool m_bWantToChangeWeapon;

	unsigned int m_iPrevWeaponSelectFailed;

	FrameTimer m_ftUseRouteTime;

	bool m_bAvoidRight;
	FrameTimer m_ftAvoidSideSwitch;

	FrameTimer m_ftAvoidTime;

	FrameTimer m_ftHealClickTime;

	unsigned int m_iSpecialVisibleId;

	float m_fCurrentDanger;
	float m_fLastHurtTime;

	FrameTimer m_ftUtilTimes[BOT_UTIL_MAX];
	eBotAction m_CurrentUtil;

	std::queue<int> m_nextVoicecmd;
	FrameTimer m_ftNextVoiceCommand;
	FrameTimer m_ftLastVoiceCommand[MAX_VOICE_CMDS];

	float m_fTotalAimFactor;
	Vector m_vAimOffset;
	MyEHandle m_pLastCoverFrom;

	bot_statistics_t m_Stats; // this updates progressively
	FrameTimer m_ftStatsTime;
	short int m_iStatsIndex;

	CBotSquad *m_pSquad;
	float m_fSquadIdleTime;
	squad_u m_uSquadDetail;

	// if bot's profile sensitivity is too small it may not complete tasks
	// this is true during tasks that need high sensitivity e.g. rocket jumping
	bool m_bIncreaseSensitivity;

	bool m_bWantToInvestigateSound;
	bool m_bWantToListen;
	float m_fListenFactor; // the current weight of bots listening vector (higher = better)
	Vector m_vListenPosition; // listening player position, heard someone shoot
	bool m_bListenPositionValid;
	FrameTimer m_ftListenTime;
	MyEHandle m_PlayerListeningTo;
	FrameTimer m_ftWantToListenTime;

	float m_fWaypointTouchDistance;
};

class CBots
{
public:
	static void BotThink(bool simulating);

	static CBot *GetBotPointer(edict_t *pEdict);

	static void FreeMapMemory();

	static void FreeAllMemory();

	static CBot *FindBotByProfile(CBotProfile *pProfile);

	static void Init();

	static void RoundStart();

	//static void KickRandomBot();

	//static void KickRandomBotOnTeam(int team);

	static void MapInit();

	static void BotFunction(IBotFunction &function);

	static void RunPlayerMoveAll();

	static void MakeBot(edict_t *pPlayer);
	static void MakeNotBot(edict_t *pPlayer);

	static CBot *Get(int iIndex) { return m_Bots[iIndex]; }
	static CBot *Get(edict_t *pPlayer) { return m_Bots[SlotOfEdict(pPlayer)]; }

private:
	static CBot **m_Bots;
};


#endif // __RCBOT2_H__