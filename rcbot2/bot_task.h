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
#ifndef __RCBOT_TASK_H__
#define __RCBOT_TASK_H__

#include "bot_base.h"
#include "bot_const.h"
#include "bot_fortress.h"

class CBotSquad;
class CBotSchedule;
class CWaypointVisibilityTable;

abstract_class IBotTaskInterrupt
{
public:
	virtual bool IsInterrupted(CBot *pBot, bool *bFailed, bool *bCompleted) = 0;
};

class CBotTF2EngineerInterrupt : public IBotTaskInterrupt
{
public:
	CBotTF2EngineerInterrupt(CBot *pBot);

	bool IsInterrupted(CBot *pBot, bool *bFailed, bool *bCompleted);
private:
	float m_fPrevSentryHealth;
	MyEHandle m_pSentryGun;
};

class CBotTF2CoverInterrupt : public IBotTaskInterrupt
{
public:
	bool IsInterrupted(CBot *pBot, bool *bFailed, bool *bCompleted);
};

class CBotTF2HurtInterrupt : public IBotTaskInterrupt
{
public:
	CBotTF2HurtInterrupt(CBot *pBot);

	bool IsInterrupted(CBot *pBot, bool *bFailed, bool *bCompleted);
private:
	float m_iHealth;
};

class CBotTask
{
public:
	CBotTask();
	~CBotTask()
	{
		if (m_pInterruptFunc != NULL)
		{
			delete m_pInterruptFunc;
			m_pInterruptFunc = NULL;
		}
	}

	virtual void Init() { } // optional
	virtual void Execute(CBot *pBot, CBotSchedule *pSchedule) = 0; // required
	bool HasFailed();
	bool IsComplete();
	bool TimedOut();
	void SetFailInterrupt(int iInterruptHave, int iInterruptDontHave = 0);
	void SetCompleteInterrupt(int iInterruptHave, int iInterruptDontHave = 0);
	void SetInterruptFunction(IBotTaskInterrupt *func) { m_pInterruptFunc = func; }
	virtual eTaskState IsInterrupted(CBot *pBot);
	void Fail();
	void Complete();
	inline bool HasFlag(int iFlag) { return (m_iFlags & iFlag) == iFlag; }
	inline void SetFlag(int iFlag) { m_iFlags |= iFlag; }
	virtual void ClearFailInterrupts() { m_iFailInterruptConditionsHave = m_iFailInterruptConditionsDontHave = 0; }
	virtual const char *DebugString(void);

protected:
	IBotTaskInterrupt *m_pInterruptFunc;
	// flags
	int m_iFlags;
	// conditions that may happen to fail task
	int m_iFailInterruptConditionsHave;
	int m_iCompleteInterruptConditionsHave;
	int m_iFailInterruptConditionsDontHave;
	int m_iCompleteInterruptConditionsDontHave;
	// current state
	eTaskState m_iState;
	// time out
	float m_fTimeOut;
};

class CFindPathTask : public CBotTask
{
public:
	CFindPathTask()
	{
		m_pEdict = MyEHandle();
		m_LookTask = LOOK_WAYPOINT;
		m_iWaypointId = -1;
		m_flags.m_data = 0;
		m_fRange = 0;
		m_iDangerPoint = -1;
		m_bGetPassedIntAsWaypointId = false;
	}

	CFindPathTask(Vector vOrigin, eLookTask looktask = LOOK_WAYPOINT)
	{
		m_vVector = vOrigin;
		m_pEdict = MyEHandle(); // no edict
		m_LookTask = looktask;
		m_iWaypointId = -1;
		m_flags.m_data = 0;
		m_fRange = 0;
		m_iDangerPoint = -1;
		m_bGetPassedIntAsWaypointId = false;
	}

	// having this constructor saves us trying to find the goal waypoint again if we
	// already know it
	CFindPathTask(int iWaypointId, eLookTask looktask = LOOK_WAYPOINT);

	CFindPathTask(edict_t *pEdict);

	void SetRange(float fRange) { m_fRange = fRange; }

	void SetEdict(edict_t *pEdict) { m_pEdict = pEdict; }

	void SetDangerPoint(int iWpt) { m_iDangerPoint = iWpt; }

	void GetPassedVector() { m_flags.bits.m_bGetPassedVector = true; }

	void GetPassedIntAsWaypointId() { m_bGetPassedIntAsWaypointId = true; }

	void DontGoToEdict() { m_flags.bits.m_bDontGoToEdict = true; }

	void SetNoInterruptions() { ClearFailInterrupts(); m_flags.bits.m_bNoInterruptions = true; }

	void Execute(CBot *pBot, CBotSchedule *pSchedule);

	void CompleteOutOfRangeFromEdict() { m_flags.bits.m_bCompleteOutOfRangeEdict = true; }

	void CompleteInRangeFromEdict() { m_flags.bits.m_bCompleteInRangeOfEdict = true; }

	void CompleteIfSeeTaskEdict() { m_flags.bits.m_bCompleteSeeTaskEdict = true; }

	void FailIfTaskEdictDead() { m_flags.bits.m_bFailTaskEdictDied = true; }

	void Init();

	void SetLookTask(eLookTask task) { m_LookTask = task; }

private:
	Vector m_vVector;
	MyEHandle m_pEdict;
	eLookTask m_LookTask;
	int m_iInt;
	int m_iWaypointId;
	int m_iDangerPoint;
	float m_fRange;

	union
	{
		byte m_data;

		struct
		{
			bool m_bNoInterruptions : 1;	// quick path finding - non concurrent
			bool m_bGetPassedVector : 1;  // receive vector from previous task
			bool m_bDontLookAtWaypoints : 1;
			bool m_bCompleteSeeTaskEdict : 1; // complete when see the edict
			bool m_bFailTaskEdictDied : 1; // fail if the edict no longer exists or dead
			bool m_bDontGoToEdict : 1; // don't complete if nearby edict
			bool m_bCompleteOutOfRangeEdict : 1; // complete if outside of m_fRange from edict (grenades)
			bool m_bCompleteInRangeOfEdict : 1;
		}bits;
	}m_flags;

	bool m_bGetPassedIntAsWaypointId;
	//bool m_bWaitUntilReached;
};

class CBotTF2AttackSentryGunTask : public CBotTask
{
public:
	CBotTF2AttackSentryGunTask(edict_t *pSentryGun, CBotWeapon *pWeapon)
	{
		m_pSentryGun = pSentryGun;
		m_pWeapon = pWeapon;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);

	void Init()
	{
		m_fTime = 0.0f;
	}
private:
	MyEHandle m_pSentryGun;
	CBotWeapon *m_pWeapon;
	int m_iStartingWaypoint;
	int m_iSentryWaypoint;
	Vector m_vStart;
	Vector m_vHide;
	float m_fDist;
	float m_fTime;
};

//defensive technique
class CBotTF2Spam : public CBotTask
{
public:
	CBotTF2Spam(Vector vStart, Vector vTarget, CBotWeapon *pWeapon);

	CBotTF2Spam(CBot *pBot, Vector vStart, int iYaw, CBotWeapon *pWeapon);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);

	Vector GetTarget() { return m_vTarget; }

	float GetDistance();
private:
	Vector m_vTarget;
	Vector m_vStart;
	CBotWeapon *m_pWeapon;
	float m_fTime;

	float m_fNextAttack;
};

#define TASK_TF2_DEMO_STATE_LAY_BOMB 0
#define TASK_TF2_DEMO_STATE_RUN_UP   1

class CBotTF2DemomanPipeJump : public CBotTask
{
public:
	CBotTF2DemomanPipeJump(CBot *pBot, Vector vWaypointGround, Vector vWaypointNext, CBotWeapon *pWeapon);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vStart;
	Vector m_vPipe;
	Vector m_vEnd;
	MyEHandle m_pPipeBomb;
	bool m_bFired;
	float m_fTime;
	int m_iState;
	int m_iStartingAmmo;
	CBotWeapon *m_pWeapon;
};

// automatically detonate pipes from a standing location, make sure
// the bot is not standing in a location visible to the enemy
// in vStand
class CBotTF2DemomanPipeEnemy : public CBotTask
{
public:
	CBotTF2DemomanPipeEnemy(CBotWeapon *pPipeLauncher, Vector vEnemy, edict_t *pEnemy)
	{
		m_vEnemy = vEnemy;
		m_pEnemy = pEnemy;
		m_fTime = 0.0f;
		m_vAim = vEnemy;
		m_pPipeLauncher = pPipeLauncher;
		m_fHoldAttackTime = 0.0f;
		m_fHeldAttackTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vStand;
	Vector m_vEnemy;
	MyEHandle m_pEnemy;
	Vector m_vAim;
	float m_fTime;
	float m_fHoldAttackTime;
	float m_fHeldAttackTime;
	CBotWeapon *m_pPipeLauncher;
};

class CBotTF2DemomanPipeTrap : public CBotTask
{
public:
	// Set up a pipe trap or fire pipe bombs -- 
	// if autodetonate, detonate them when I've shot them rather than wait for an enemy
	// such as when attacking a sentry
	CBotTF2DemomanPipeTrap(eDemoTrapType type, Vector vStand, Vector vLoc, Vector vSpread, bool bAutoDetonate = false, int wptarea = -1)
	{
		m_vPoint = vLoc;
		m_vLocation = vLoc;
		m_vSpread = vSpread;
		m_iState = 0;
		m_iStickies = 6;
		m_iTrapType = type;
		m_vStand = vStand;
		m_fTime = 0.0f;
		m_bAutoDetonate = bAutoDetonate;
		m_iWptArea = wptarea;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vPoint;
	Vector m_vStand;
	Vector m_vLocation;
	Vector m_vSpread;
	float m_fTime;
	eDemoTrapType m_iTrapType;
	int m_iState;
	int m_iStickies;
	bool m_bAutoDetonate;
	int m_iWptArea;

};

class CBotTF2FindPipeWaypoint : public CBotTask
{
public:
	CBotTF2FindPipeWaypoint(Vector vOrigin, Vector vTarget);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	int m_iters;
	unsigned short int m_i;
	unsigned short int m_j;
	Vector m_vOrigin;
	Vector m_vTarget;
	short int m_iTargetWaypoint;
	float m_fNearesti;
	float m_fNearestj;
	short int m_iNearesti;
	short int m_iNearestj;

	CWaypointVisibilityTable *m_pTable;
	CWaypoint *m_pTarget;
	std::vector<int> m_WaypointsI;
	std::vector<int> m_WaypointsJ;
};
/*
class CBotGravGunPickup : public CBotTask
{
public:
	CBotGravGunPickup(edict_t *pWeapon, edict_t *pProp)
	{
		m_Weapon = pWeapon;
		m_Prop = pProp;
		m_fTime = 0;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_Weapon;
	MyEHandle m_Prop;
	float m_fTime;
	float m_fSecAttTime;
};

#define CHARGER_HEALTH 0
#define CHARGER_ARMOR 1

class CBotHL2DMUseCharger : public CBotTask
{
public:
	CBotHL2DMUseCharger(edict_t *pCharger, int type)
	{
		m_pCharger = pCharger;
		m_fTime = 0;
		m_iType = type;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pCharger;
	float m_fTime;
	int m_iType;
};

class CBotHL2DMUseButton : public CBotTask
{
public:
	CBotHL2DMUseButton(edict_t *pButton)
	{
		m_pButton = pButton;
		m_fTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pButton;
	float m_fTime;
};
*/
class CBotTF2MedicHeal : public CBotTask
{
public:
	CBotTF2MedicHeal()
	{
		m_pHeal = MyEHandle();
		m_bHealerJumped = false;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pHeal;
	Vector m_vJump;
	bool m_bHealerJumped;

};

class CBotRemoveSapper : public CBotTask
{
public:
	CBotRemoveSapper(edict_t *pBuilding, eObjectType id)
	{
		m_fTime = 0.0f;
		m_pBuilding = pBuilding;
		m_id = id;
		m_fHealTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
	float m_fHealTime;
	MyEHandle m_pBuilding;
	eObjectType m_id;
};

class CBotUseLunchBoxDrink : public CBotTask
{
public:
	CBotUseLunchBoxDrink()
	{
		m_fTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
};

class CBotUseBuffItem : public CBotTask
{
public:
	CBotUseBuffItem()
	{
		m_fTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
};

class CBotBackstab : public CBotTask
{
public:
	CBotBackstab(edict_t *pEnemy)
	{
		m_fTime = 0.0f;
		m_pEnemy = pEnemy;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
	MyEHandle m_pEnemy;
};

class CBotJoinSquad : public CBotTask
{
public:
	CBotJoinSquad(edict_t *pPlayerToJoin)
	{
		m_pPlayer = pPlayerToJoin;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	edict_t *m_pPlayer;
};

class CBotFollowSquadLeader : public CBotTask
{
public:
	CBotFollowSquadLeader(CBotSquad *pSquad)
	{
		m_fLeaderSpeed = 0.0f;
		m_pSquad = pSquad;
		m_fVisibleTime = 0.0f;
		m_fUpdateMovePosTime = 0.0f;
		m_vPos = Vector(0, 0, 0);
		m_vForward = Vector(0, 0, 0);
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	CBotSquad *m_pSquad;
	float m_fVisibleTime;
	float m_fUpdateMovePosTime;
	float m_fLeaderSpeed;
	Vector m_vPos;
	Vector m_vForward;
};


class CBotNest : public CBotTask
{
public:
	CBotNest()
	{
		m_fTime = 0.0f;
		m_pEnemy = MyEHandle();
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
	MyEHandle m_pEnemy;
};

class CBotDefendTask : public CBotTask
{
public:
	CBotDefendTask(Vector vOrigin, float fMaxTime = 0, int iInterrupt = CONDITION_SEE_CUR_ENEMY, bool bDefendOrigin = false, Vector vDefendOrigin = Vector(0.0f), eLookTask looktask = LOOK_SNIPE, int iWaypointType = 0)
	{
		m_fMaxTime = fMaxTime;
		m_vOrigin = vOrigin;
		m_fTime = 0;
		SetCompleteInterrupt(iInterrupt);
		m_bDefendOrigin = bDefendOrigin;
		m_vDefendOrigin = vDefendOrigin;
		m_LookTask = looktask;
		m_iWaypointType = iWaypointType;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
	float m_fMaxTime;
	Vector m_vOrigin;
	bool m_bDefendOrigin;
	Vector m_vDefendOrigin;
	eLookTask m_LookTask;
	int m_iWaypointType;
};

class CBotInvestigateTask : public CBotTask
{
public:
	CBotInvestigateTask(Vector vOrigin, float fRadius, Vector vPOV, bool bHasPOV, float fMaxTime = 0, int iInterrupt = CONDITION_SEE_CUR_ENEMY)
	{
		m_fMaxTime = fMaxTime;
		m_vOrigin = vOrigin;
		m_fRadius = fRadius;
		m_fTime = 0;
		SetCompleteInterrupt(iInterrupt);
		m_iState = 0;
		m_vPOV = vPOV;
		m_bHasPOV = bHasPOV;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	int m_iState;
	float m_fTime;
	float m_fMaxTime;
	Vector m_vOrigin;
	float m_fRadius;
	int m_iCurPath;
	bool m_bHasPOV;
	Vector m_vPOV;
	std::vector<Vector> m_InvPoints; // investigation points (waypoint paths)
};

class CBotTF2EngiLookAfter : public CBotTask
{
public:
	CBotTF2EngiLookAfter(edict_t *pSentry)
	{
		m_pSentry = pSentry;
		m_fTime = 0;
		m_fHitSentry = 0;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
	float m_fHitSentry;
	MyEHandle m_pSentry;
};

class CBotTF2SnipeCrossBow : public CBotTask
{
public:
	CBotTF2SnipeCrossBow(Vector vOrigin, int iWpt);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime; // time of task
	Vector m_vAim; // base origin to aim at
	Vector m_vOrigin; // origin of snipe waypoint
	Vector m_vEnemy; // origin of last enemy
	int m_iArea; // area of snipe waypoint
	int m_iHideWaypoint; // waypoint id of place to hide
	int m_iSnipeWaypoint; // waypoint id of place to snipe
	Vector m_vHideOrigin; // origin of hiding place
	float m_fHideTime; // if above engine time, hide
	int m_iPrevClip; // used to check i actually fired a bullet or not
	float m_fEnemyTime; // last time i saw an enemy
	float m_fAimTime; // last time i got ready to aim - gives bots time to aim before shooting
	float m_fCheckTime; // time to check if there is a wall in front of me while sniping
	float m_fOriginDistance;
};

class CBotTF2Snipe : public CBotTask
{
public:
	CBotTF2Snipe(Vector vOrigin, int iWpt);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime; // time of task
	Vector m_vAim; // base origin to aim at
	Vector m_vOrigin; // origin of snipe waypoint
	Vector m_vEnemy; // origin of last enemy
	int m_iArea; // area of snipe waypoint
	int m_iHideWaypoint; // waypoint id of place to hide
	int m_iSnipeWaypoint; // waypoint id of place to snipe
	Vector m_vHideOrigin; // origin of hiding place
	float m_fHideTime; // if above engine time, hide
	int m_iPrevClip; // used to check i actually fired a bullet or not
	float m_fEnemyTime; // last time i saw an enemy
	float m_fAimTime; // last time i got ready to aim - gives bots time to aim before shooting
	float m_fCheckTime; // time to check if there is a wall in front of me while sniping
	float m_fOriginDistance;
};

/*class CBotDODSnipe : public CBotTask
{
public:
	CBotDODSnipe( CBotWeapon *pWeaponToUse, Vector vOrigin, float fYaw, bool bUseZ = false, float z = 0, int iWaypointType = 0 );

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
	float m_fEnemyTime;
	float m_fScopeTime;
	Vector m_vAim;
	Vector m_vOrigin;
	CBotWeapon *m_pWeaponToUse;
	Vector m_vLastEnemy;
	bool m_bUseZ;
	float m_z; // z = ground level
	int m_iWaypointType;
	float m_fTimeout;
};

class CBotHL2DMSnipe : public CBotTask
{
public:
	CBotHL2DMSnipe( CBotWeapon *pWeaponToUse, Vector vOrigin, float fYaw, bool bUseZ = false, float z = 0, int iWaypointType = 0 );

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
	float m_fEnemyTime;
	float m_fScopeTime;
	Vector m_vAim;
	Vector m_vOrigin;
	CBotWeapon *m_pWeaponToUse;
	bool m_bUseZ;
	float m_z; // z = ground level
	int m_iWaypointType;
};*/

class CBotTF2SpyDisguise : public CBotTask
{
public:
	CBotTF2SpyDisguise();

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
};

class CBotTFEngiBuildTask : public CBotTask
{
public:
#if defined USE_NAVMESH
	CBotTFEngiBuildTask(eObjectType iObject, Vector vSpot);
#else
	CBotTFEngiBuildTask(eObjectType iObject, CWaypoint *pWaypoint);
#endif

	void Execute(CBot *pBot, CBotSchedule *pSchedule);

	void OneTryOnly() { m_iTries = 2; }
private:
	Vector m_vOrigin;
	eObjectType m_iObject;
	int m_iState;
	float m_fTime;
	int m_iTries;
	float m_fNextUpdateAngle;
	Vector m_vAimingVector;
	int m_iArea;
	Vector m_vBaseOrigin;
	float m_fRadius;
};
/*
class CDODDropAmmoTask : public CBotTask
{
public:
	CDODDropAmmoTask(edict_t *pPlayer)
	{
		m_fTime = 0.0f;
		m_pPlayer = pPlayer;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pPlayer;
	float m_fTime;
};

class CDODWaitForGrenadeTask : public CBotTask
{
public:
	CDODWaitForGrenadeTask(edict_t *pGrenade)
	{
		m_pGrenade = pGrenade;
		m_fTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pGrenade;
	float m_fTime;
};

class CDODWaitForBombTask : public CBotTask
{
public:
	CDODWaitForBombTask(edict_t *pBombTarget, CWaypoint *pBlocking)
	{
		m_pBombTarget = pBombTarget;
		m_fTime = 0.0f;
		m_pBlocking = pBlocking;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pBombTarget;
	float m_fTime;
	CWaypoint *m_pRunTo;
	CWaypoint *m_pBlocking;
};

class CBotDODBomb : public CBotTask
{
public:
	CBotDODBomb(int iBombType, int iBombID, edict_t *m_pBombTarget, Vector vPosition, int iPrevOwner);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vOrigin;
	float m_fTime;
	int m_iBombID;
	int m_iPrevTeam; // prev owner
	edict_t *m_pBombTarget;
	int m_iType;
};

class CBotDODAttackPoint : public CBotTask
{
public:
	CBotDODAttackPoint(int iFlagID, Vector vOrigin, float fRadius);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vOrigin;
	Vector m_vMoveTo;
	float m_fAttackTime;
	float m_fTime;
	int m_iFlagID;
	float m_fRadius;
	bool m_bProne;
};
*/
class CBotTF2AttackPoint : public CBotTask
{
public:
	CBotTF2AttackPoint(int iArea, Vector vOrigin, int iRadius)
	{
		m_vOrigin = vOrigin;
		m_fAttackTime = 0.0f;
		m_fTime = 0.0f;
		m_iArea = iArea;
		m_iRadius = iRadius;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vOrigin;
	Vector m_vMoveTo;
	float m_fAttackTime;
	float m_fTime;
	int m_iArea;
	int m_iRadius;
};

class CBotTF2ShootLastEnemyPosition : public CBotTask
{
public:
	CBotTF2ShootLastEnemyPosition(Vector vPosition, edict_t *pEnemy, Vector velocity);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pEnemy;
	Vector m_vPosition;
	float m_fTime;
};

class CBotTF2DefendPoint : public CBotTask
{
public:
	CBotTF2DefendPoint(int iArea, Vector vOrigin, int iRadius)
	{
		m_vOrigin = vOrigin;
		m_fDefendTime = 0.0f;
		m_fTime = 0.0f;
		m_iArea = iArea;
		m_iRadius = iRadius;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vOrigin;
	Vector m_vMoveTo;
	float m_fDefendTime;
	float m_fTime;
	int m_iArea;
	int m_iRadius;
};

class CBotInvestigateHidePoint : public CBotTask
{
	// investigate a  possible enemy hiding point

public:
	CBotInvestigateHidePoint(int iWaypointIndexToInvestigate, int iOriginalWaypointIndex);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vOrigin;
	Vector m_vMoveTo;
	std::vector<Vector> m_CheckPoints;
	unsigned int m_iCurrentCheckPoint;
	float m_fInvestigateTime;
	float m_fTime;
	int m_iState;
};

class CBotTF2PushPayloadBombTask : public CBotTask
{
public:
	CBotTF2PushPayloadBombTask(edict_t *pPayloadBomb)
	{
		m_pPayloadBomb = pPayloadBomb;
		m_fPushTime = 0.0f;
		m_fTime = 0.0f;
		m_vRandomOffset = Vector(0.0f);
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pPayloadBomb;
	Vector m_vMoveTo;
	Vector m_vRandomOffset;
	float m_fPushTime;
	float m_fTime;
	Vector m_vOrigin;
};

class CBotTF2DefendPayloadBombTask : public CBotTask
{
public:
	CBotTF2DefendPayloadBombTask(edict_t *pPayloadBomb)
	{
		m_pPayloadBomb = pPayloadBomb;
		m_fDefendTime = 0.0f;
		m_fTime = 0.0f;
		m_vRandomOffset = Vector(0.0f);
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pPayloadBomb;
	Vector m_vMoveTo;
	Vector m_vRandomOffset;
	float m_fDefendTime;
	float m_fTime;
	Vector m_vOrigin;
};

class CBotTF2UpgradeBuilding : public CBotTask
{
public:
	CBotTF2UpgradeBuilding(edict_t *pBuilding)
	{
		m_pBuilding = pBuilding;
		m_fTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pBuilding;
	float m_fTime;
};

class CBotTF2UpgradeWeapon : public CBotTask
{
public:
	CBotTF2UpgradeWeapon(TFClass iClass)
	{
		m_iClass = iClass;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	TFClass m_iClass;
};

class CBotTF2WaitAmmoTask : public CBotTask
{
public:
	CBotTF2WaitAmmoTask(Vector vOrigin)
	{
		m_vOrigin = vOrigin;
		m_fWaitTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vOrigin;
	float m_fWaitTime;
};

class CBotTF2WaitHealthTask : public CBotTask
{
public:
	CBotTF2WaitHealthTask(Vector vOrigin)
	{
		m_vOrigin = vOrigin;
		m_fWaitTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vOrigin;
	float m_fWaitTime;
};

class CBotTFDoubleJump : public CBotTask
{
public:
	CBotTFDoubleJump()
	{
		m_fTime = 0.0f;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
};

class CBotTFRocketJump : public CBotTask
{
public:
	CBotTFRocketJump()
	{
		m_fTime = 0.0f;
		m_fJumpTime = 0.0f;
		m_iState = 0;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
	float m_fJumpTime;
	int m_iState;
};

class CBotTF2SpySap : public CBotTask
{
public:
	CBotTF2SpySap(edict_t *pBuilding, eObjectType id)
	{
		m_pBuilding = pBuilding;
		m_fTime = 0.0f;
		m_id = id;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pBuilding;
	float m_fTime;
	eObjectType m_id;
};

class CBotTFUseTeleporter : public CBotTask
{
public:
	CBotTFUseTeleporter(edict_t *pTele)
	{
		m_pTele = pTele;
		m_fTime = 0.0;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pTele;
	float m_fTime;
	Vector m_vLastOrigin;
};

class CBotTaskEngiPickupBuilding : public CBotTask
{
public:
	CBotTaskEngiPickupBuilding(edict_t *pBuilding)
	{
		m_fTime = 0.0f;
		m_pBuilding = pBuilding;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pBuilding;
	float m_fTime;
};

class CBotTaskEngiPlaceBuilding : public CBotTask
{
public:
	CBotTaskEngiPlaceBuilding(eObjectType iObject, Vector vOrigin)
	{
		m_vOrigin = vOrigin;
		m_fTime = 0.0f;
		m_iState = 1; // BEGIN HERE , otherwIse bot will try to destroy the Building
		m_iObject = iObject;
		m_iTries = 0;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vOrigin;
	float m_fTime;
	eObjectType m_iObject;
	int m_iState;
	int m_iTries;
};

class CBotTF2WaitFlagTask : public CBotTask
{
public:
	CBotTF2WaitFlagTask(Vector vOrigin, bool bFind = false)
	{
		m_vOrigin = vOrigin;
		m_fWaitTime = 0.0f;
		m_bFind = bFind;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vOrigin;
	float m_fWaitTime;
	bool m_bFind;
};
/*
class CThrowGrenadeTask : public CBotTask
{
public:
	CThrowGrenadeTask(CBotWeapon *pWeapon, int ammo, Vector vLoc);

	void Init();

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vLoc;
	CBotWeapon *m_pWeapon;
	float m_fTime;
	float m_fHoldAttackTime;
	int m_iAmmo;
};
*/
class CAttackEntityTask : public CBotTask
{
public:
	CAttackEntityTask(edict_t *pEdict)
	{
		m_pEdict = pEdict;
	}

	void Init();

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pEdict;
};
/*
class CAutoBuy : public CBotTask
{
public:
	void Init();

	void Execute(CBot *pBot, CBotSchedule *pSchedule);

private:
	float m_fTime;
	bool m_bTimeset;
};
*/
class CBotTF2TauntTask : public CBotTask
{
public:
	CBotTF2TauntTask(Vector vPlayer, Vector vOrigin, float fDist)
	{
		m_vPlayer = vPlayer;
		m_vOrigin = vOrigin;
		m_fDist = fDist;
	}

	void Init();

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vPlayer;
	Vector m_vOrigin;
	float m_fDist;
	float m_fTime;
};

class CMoveToTask : public CBotTask
{
public:
	CMoveToTask(Vector vOrigin);

	CMoveToTask(edict_t *pEdict);

	void Init();

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fPrevDist;
	Vector m_vVector;
	MyEHandle m_pEdict;
};

class CMessAround : public CBotTask
{
public:
	CMessAround(edict_t *pFriendly, int iMaxVoiceCmd)
	{
		m_fTime = 0.0f;
		m_pFriendly = pFriendly;
		m_iType = RandomInt(0, 3);
		m_iMaxVoiceCmd = iMaxVoiceCmd;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	float m_fTime;
	MyEHandle m_pFriendly;
	int m_iMaxVoiceCmd;
	int m_iType; // 0 = attack friendly , 1 = taunt, 2 = random voice command
};

class CFindLastEnemy : public CBotTask
{
public:
	CFindLastEnemy(Vector vLast, Vector vVelocity)
	{
		SetCompleteInterrupt(CONDITION_SEE_CUR_ENEMY);
		m_vLast = vLast + (vVelocity * 10);
		m_fTime = 0;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vLast;
	float m_fTime;
};

class CFindGoodHideSpot : public CBotTask
{
public:
	CFindGoodHideSpot(edict_t *pEntity);

	CFindGoodHideSpot(Vector vec);

	void Init();

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vHideFrom;
};

class CHideTask : public CBotTask
{
public:
	CHideTask(Vector vHideFrom)
	{
		m_vHideFrom = vHideFrom;
	}

	void Init();

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	Vector m_vHideFrom;
	float m_fHideTime;
};

class CFollowTask : public CBotTask
{
public:
	CFollowTask(edict_t *pFollow);

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pFollow;
	float m_fFollowTime;
	Vector m_vLastSeeVector;
	Vector m_vLastSeeVelocity;
};

class CCrouchHideTask : public CBotTask
{
public:
	CCrouchHideTask(edict_t *pHideFrom);

	void Init();

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pHideFrom;
	float m_fHideTime;
	float m_fChangeTime;
	bool m_bCrouching;
	Vector m_vLastSeeVector;
};

class CSpyCheckAir : public CBotTask
{
public:
	CSpyCheckAir()
	{
		m_fTime = 0.0f;
		m_pUnseenBefore = MyEHandle();
		m_bHitPlayer = false;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
private:
	MyEHandle m_pUnseenBefore;
	int seenlist;
	float m_fNextCheckUnseen;
	float m_fTime;
	bool m_bHitPlayer;
};

class CTaskVoiceCommand : public CBotTask
{
public:
	CTaskVoiceCommand(int iVoiceCmd)
	{
		m_iVoiceCmd = iVoiceCmd;
	}

	void Execute(CBot *pBot, CBotSchedule *pSchedule)
	{
		pBot->AddVoiceCommand(m_iVoiceCmd);

		Complete();
	}

private:
	int m_iVoiceCmd;
};

class CPrimaryAttack : public CBotTask
{
public:

	void Execute(CBot *pBot, CBotSchedule *pSchedule);
};
/*
class CAttackTask : public CBotTask
{
public:
	CAttackTask ( Vector vToAttack )
	{
		SetVector(vToAttack);
	}

	CAttackTask ( edict_t *pToAttack )
	{
		SetEdict(pToAttack);
	}

	void Init ()
	{
		SetInterrupt(CONDITION_OUT_OF_AMMO|CONDITION_NO_WEAPON);
	}

	virtual void Execute ( CBot *pBot )
	{

	}
};

class CFindRunPath : public CBotTask
{
public:
	CFindRunPath(Vector pGoto)
	{
		SetVector(pGoto);
	}

	CFindRunPath(edict_t *pGoto)
	{
		SetEdict(pGoto);
		//SetVector(pGoto->v.origin); ??
	}

	void Init()
	{
		SetInterrupt(0);
	}

	virtual void Execute(CBot *pBot);
};
*/
#endif