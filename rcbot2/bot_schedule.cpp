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

#include "bot_base.h"
#include "bot_schedule.h"
#include "bot_task.h"
#include "bot_weapons.h"
#include "bot_globals.h"
#include "bot_getprop.h"

#if defined USE_NAVMESH
#include "bot_navmesh.h"
#else
#include "bot_waypoint_locations.h"
#include "bot_waypoint_visibility.h"
#endif

#ifndef __linux__
#include <ndebugoverlay.h>
#endif

////////////////////////////////////
// these must match the SCHED IDs
const char *const g_szSchedules[SCHED_MAX] =
{
	"SCHED_NONE",
	"SCHED_ATTACK",
	"SCHED_RUN_FOR_COVER",
	"SCHED_GOTO_ORIGIN",
	"SCHED_GOOD_HIDE_SPOT",
	"SCHED_TF2_GET_FLAG",
	"SCHED_TF2_GET_HEALTH",
	"SCHED_TF_BUILD",
	"SCHED_HEAL",
	"SCHED_GET_METAL",
	"SCHED_SNIPE",
	"SCHED_UPGRADE",
	"SCHED_USE_TELE",
	"SCHED_SPY_SAP_BUILDING",
	"SCHED_USE_DISPENSER",
	"SCHED_PICKUP",
	"SCHED_TF2_GET_AMMO",
	"SCHED_TF2_FIND_FLAG",
	"SCHED_LOOKAFTERSENTRY",
	"SCHED_DEFEND",
	"SCHED_ATTACKPOINT",
	"SCHED_DEFENDPOINT",
	"SCHED_TF2_PUSH_PAYLOADBOMB",
	"SCHED_TF2_DEFEND_PAYLOADBOMB",
	"SCHED_TF2_DEMO_PIPETRAP",
	"SCHED_TF2_DEMO_PIPESENTRY",
	"SCHED_BACKSTAB",
	"SCHED_REMOVESAPPER",
	"SCHED_GOTONEST",
	"SCHED_MESSAROUND",
	"SCHED_TF2_ENGI_MOVE_BUILDING",
	"SCHED_FOLLOW_LAST_ENEMY",
	"SCHED_SHOOT_LAST_ENEMY_POS",
	"SCHED_GRAVGUN_PICKUP",
	"SCHED_HELP_PLAYER",
	"SCHED_BOMB",
	"SCHED_TF_SPYCHECK",
	"SCHED_FOLLOW",
	"SCHED_DOD_DROPAMMO",
	"SCHED_INVESTIGATE_NOISE",
	"SCHED_CROUCH_AND_HIDE",
	"SCHED_DEPLOY_MACHINE_GUN",
	"SCHED_ATTACK_SENTRY_GUN",
	"SCHED_RETURN_TO_INTEL",
	"SCHED_INVESTIGATE_HIDE",
	"SCHED_TAUNT",
	"SCHED_UPGRADE_WEAPON"
};

CBotTF2DemoPipeEnemySched::CBotTF2DemoPipeEnemySched(CBotWeapon *pLauncher, Vector vStand, edict_t *pEnemy)
{
	//AddTask(new CFindPathTask(vStand));
	//AddTask(new CBotTF2DemomanPipeEnemy(pLauncher,pEnemy));
}

void CBotTF2DemoPipeEnemySched::Init()
{
	SetID(SCHED_TF2_DEMO_PIPEENEMY);
}

CBotTF2DemoPipeTrapSched::CBotTF2DemoPipeTrapSched(eDemoTrapType type, Vector vStand, Vector vLoc, Vector vSpread, bool bAutoDetonate, int wptarea)
{
	AddTask(new CFindPathTask(vStand));
	AddTask(new CBotTF2DemomanPipeTrap(type, vStand, vLoc, vSpread, bAutoDetonate, wptarea));
}

void CBotTF2DemoPipeTrapSched::Init()
{
	SetID(SCHED_TF2_DEMO_PIPETRAP);
}

CBotTF2UpgradeWeaponSched::CBotTF2UpgradeWeaponSched(Vector vStationLoc, TFClass iClass)
{
	PassVector(&vStationLoc);
	AddTask(new CFindPathTask(vStationLoc)); // 1, path to nearest waypoint
	AddTask(new CMoveToTask(vStationLoc)); // 2, proceed from there to the station
	AddTask(new CBotTF2UpgradeWeapon(iClass)); // 3, perform upgrade
	AddTask(new CBotDefendTask(vStationLoc, 2.0f)); // 4, wait a moment, simulate buy process
}

void CBotTF2UpgradeWeaponSched::Init()
{
	SetID(SCHED_UPGRADE_WEAPON);
}

CBotTF2HealSched::CBotTF2HealSched(edict_t *pHeal)
{
	CFindPathTask *findpath = new CFindPathTask(pHeal);
	findpath->SetCompleteInterrupt(CONDITION_SEE_HEAL);
	AddTask(findpath);
	AddTask(new CBotTF2MedicHeal());
}

void CBotTF2HealSched::Init()
{
	SetID(SCHED_HEAL);
}

#if defined USE_NAVMESH
CBotTFEngiBuild::CBotTFEngiBuild(CBot *pBot, eObjectType iObject, Vector vSpot)
{
	CFindPathTask *pathtask = new CFindPathTask(vSpot);

	AddTask(pathtask); // first

	pathtask->SetInterruptFunction(new CBotTF2EngineerInterrupt(pBot));

	AddTask(new CBotTFEngiBuildTask(iObject, vSpot)); // second
}
#else
CBotTFEngiBuild::CBotTFEngiBuild(CBot *pBot, eObjectType iObject, CWaypoint *pWaypoint)
{
	CFindPathTask *pathtask = new CFindPathTask(CWaypoints::GetWaypointIndex(pWaypoint));

	AddTask(pathtask); // first

	pathtask->SetInterruptFunction(new CBotTF2EngineerInterrupt(pBot));

	AddTask(new CBotTFEngiBuildTask(iObject, pWaypoint)); // second
}
#endif

void CBotTFEngiBuild::Init()
{
	SetID(SCHED_TF_BUILD);
}

CBotGetMetalSched::CBotGetMetalSched(Vector vOrigin)
{

	CFindPathTask *task1 = new CFindPathTask(vOrigin);
	CBotTF2WaitAmmoTask *task2 = new CBotTF2WaitAmmoTask(vOrigin);

	task1->SetCompleteInterrupt(0, CONDITION_NEED_AMMO);
	task2->SetCompleteInterrupt(0, CONDITION_NEED_AMMO);

	AddTask(task1); // first
	AddTask(task2);
}

void CBotGetMetalSched::Init()
{
	SetID(SCHED_GET_METAL);
}

CBotEngiMoveBuilding::CBotEngiMoveBuilding(edict_t *pBotEdict, edict_t *pBuilding, eObjectType iObject, Vector vNewLocation, bool bCarrying)
{
	// not carrying
	if (!bCarrying)
	{
		AddTask(new CFindPathTask(pBuilding));
		AddTask(new CBotTaskEngiPickupBuilding(pBuilding));
	}
	// otherwise already carrying

	AddTask(new CFindPathTask(vNewLocation));
	AddTask(new CBotTaskEngiPlaceBuilding(iObject, vNewLocation));
}

void CBotEngiMoveBuilding::Init()
{
	SetID(SCHED_TF2_ENGI_MOVE_BUILDING);
}

CBotTF2PushPayloadBombSched::CBotTF2PushPayloadBombSched(edict_t *ePayloadBomb)
{
	AddTask(new CFindPathTask(ePayloadBomb)); // first
	AddTask(new CBotTF2PushPayloadBombTask(ePayloadBomb)); // second
}

void CBotTF2PushPayloadBombSched::Init()
{
	SetID(SCHED_TF2_PUSH_PAYLOADBOMB);
}

CBotTF2DefendPayloadBombSched::CBotTF2DefendPayloadBombSched(edict_t *ePayloadBomb)
{
	AddTask(new CFindPathTask(CBotGlobals::EntityOrigin(ePayloadBomb))); // first
	AddTask(new CBotTF2DefendPayloadBombTask(ePayloadBomb)); // second
}

void CBotTF2DefendPayloadBombSched::Init()
{
	SetID(SCHED_TF2_DEFEND_PAYLOADBOMB);
}

CBotTFEngiUpgrade::CBotTFEngiUpgrade(CBot *pBot, edict_t *pBuilding)
{
	CFindPathTask *pathtask = new CFindPathTask(pBuilding);

	AddTask(pathtask);

	pathtask->CompleteInRangeFromEdict();
	pathtask->FailIfTaskEdictDead();
	pathtask->SetRange(150.0f);

	if (!CTeamFortress2Mod::IsSentryGun(pBuilding))
	{
		pathtask->SetInterruptFunction(new CBotTF2EngineerInterrupt(pBot));

		CBotTF2UpgradeBuilding *upgbuilding = new CBotTF2UpgradeBuilding(pBuilding);
		AddTask(upgbuilding);
		upgbuilding->SetInterruptFunction(new CBotTF2EngineerInterrupt(pBot));

	}
	else
	{
		AddTask(new CBotTF2UpgradeBuilding(pBuilding));
	}
}

void CBotTFEngiUpgrade::Init()
{
	SetID(SCHED_UPGRADE);
}

CBotBackstabSched::CBotBackstabSched(edict_t *pEnemy)
{
	Vector vrear;
	Vector vangles;

	AngleVectors(CBotGlobals::EntityEyeAngles(pEnemy), &vangles);
	vrear = CBotGlobals::EntityOrigin(pEnemy) - (vangles * 45) + Vector(0, 0, 32);

	AddTask(new CFindPathTask(vrear));
	AddTask(new CBotBackstab(pEnemy));
}

void CBotBackstabSched::Init()
{
	SetID(SCHED_BACKSTAB);
}

CBotTF2SnipeCrossBowSched::CBotTF2SnipeCrossBowSched(Vector vOrigin, int iWpt)
{
	CBotTask *pFindPath = new CFindPathTask(iWpt);
	CBotTask *pSnipeTask = new CBotTF2SnipeCrossBow(vOrigin, iWpt);

	AddTask(pFindPath); // first
	AddTask(pSnipeTask); // second

	pFindPath->SetFailInterrupt(CONDITION_PARANOID);
	pSnipeTask->SetFailInterrupt(CONDITION_PARANOID);

}

void CBotTF2SnipeCrossBowSched::Init()
{
	SetID(SCHED_SNIPE);
}

CBotTF2SnipeSched::CBotTF2SnipeSched(Vector vOrigin, int iWpt)
{
	CBotTask *pFindPath = new CFindPathTask(iWpt);
	CBotTask *pSnipeTask = new CBotTF2Snipe(vOrigin, iWpt);

	AddTask(pFindPath); // first
	AddTask(pSnipeTask); // second

	pFindPath->SetFailInterrupt(CONDITION_PARANOID);
	pSnipeTask->SetFailInterrupt(CONDITION_PARANOID);

}

void CBotTF2SnipeSched::Init()
{
	SetID(SCHED_SNIPE);
}

CBotTFEngiLookAfterSentry::CBotTFEngiLookAfterSentry(edict_t *pSentry)
{
	AddTask(new CFindPathTask(pSentry)); // first
	AddTask(new CBotTF2EngiLookAfter(pSentry)); // second
}

void CBotTFEngiLookAfterSentry::Init()
{
	SetID(SCHED_LOOKAFTERSENTRY);
}

CBotTF2GetHealthSched::CBotTF2GetHealthSched(Vector vOrigin)
{
	CFindPathTask *task1 = new CFindPathTask(vOrigin);
	CBotTF2WaitHealthTask *task2 = new CBotTF2WaitHealthTask(vOrigin);

	// if bot doesn't have need ammo flag anymore ....
	// fail so that the bot doesn't move onto the next task
	task1->SetCompleteInterrupt(0, CONDITION_NEED_HEALTH);
	task2->SetCompleteInterrupt(0, CONDITION_NEED_HEALTH);

	AddTask(task1); // first
	AddTask(task2); // second
}

void CBotTF2GetHealthSched::Init()
{
	SetID(SCHED_TF2_GET_HEALTH);
}

CBotTF2GetAmmoSched::CBotTF2GetAmmoSched(Vector vOrigin)
{
	CFindPathTask *task1 = new CFindPathTask(vOrigin);
	CBotTF2WaitAmmoTask *task2 = new CBotTF2WaitAmmoTask(vOrigin);

	// if bot doesn't have need ammo flag anymore ....
	// fail so that the bot doesn't move onto the next task
	task1->SetCompleteInterrupt(0, CONDITION_NEED_AMMO);
	task2->SetCompleteInterrupt(0, CONDITION_NEED_AMMO);

	AddTask(task1); // first
	AddTask(task2); // second
}

void CBotTF2GetAmmoSched::Init()
{
	SetID(SCHED_TF2_GET_AMMO);
}

CBotTF2GetFlagSched::CBotTF2GetFlagSched(Vector vOrigin, bool bUseRoute, Vector vRoute)
{
	if (bUseRoute)
		AddTask(new CFindPathTask(vRoute));

	AddTask(new CFindPathTask(vOrigin)); // first
	AddTask(new CBotTF2WaitFlagTask(vOrigin)); // second
}

void CBotTF2GetFlagSched::Init()
{
	SetID(SCHED_TF2_GET_FLAG);
}

CBotUseTeleSched::CBotUseTeleSched(edict_t *pTele)
{
	// find path
	AddTask(new CFindPathTask(pTele)); // first
	AddTask(new CBotTFUseTeleporter(pTele)); // second
}
void CBotUseTeleSched::Init()
{
	SetID(SCHED_USE_TELE);
}

CBotUseDispSched::CBotUseDispSched(CBot *pBot, edict_t *pDisp)//, bool bNest )
{
	CFindPathTask *pathtask = new CFindPathTask(pDisp);
	CBotTF2WaitHealthTask *gethealth = new CBotTF2WaitHealthTask(CBotGlobals::EntityOrigin(pDisp));
	AddTask(pathtask);
	pathtask->SetInterruptFunction(new CBotTF2EngineerInterrupt(pBot));

	AddTask(gethealth); // second
	gethealth->SetInterruptFunction(new CBotTF2EngineerInterrupt(pBot));

	//if ( bNest )
	//	AddTask(new CBotNest()); // third
}

void CBotUseDispSched::Init()
{
	SetID(SCHED_USE_DISPENSER);
}

void CBotSpySapBuildingSched::Init()
{
	SetID(SCHED_SPY_SAP_BUILDING);
}

CBotSpySapBuildingSched::CBotSpySapBuildingSched(edict_t *pBuilding, eObjectType id)
{
	CFindPathTask *findpath = new CFindPathTask(pBuilding);

	AddTask(findpath); // first
	AddTask(new CBotTF2SpySap(pBuilding, id)); // second

#if defined USE_NAVMESH
	INavMeshArea *area = g_pNavMesh->GetArea(CBotGlobals::EntityOrigin(pBuilding));
	if(area) findpath->SetDangerPoint(area->GetID());
#else
	findpath->SetDangerPoint(CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(pBuilding), 200.0f, -1));
#endif
}

CBotTauntSchedule::CBotTauntSchedule(edict_t *pPlayer, float fYaw)
{
	QAngle angles = QAngle(0, fYaw, 0);
	Vector forward;
	Vector vOrigin;
	Vector vGoto;
	const float fTauntDist = 40.0f;

	m_pPlayer = pPlayer;
	m_fYaw = 180 - fYaw;

	AngleVectors(angles, &forward);

	forward = forward / forward.Length();
	vOrigin = CBotGlobals::EntityOrigin(pPlayer);

	vGoto = vOrigin + (forward*fTauntDist);

	CBotGlobals::FixFloatAngle(&m_fYaw);

	AddTask(new CFindPathTask(vOrigin));
	AddTask(new CMoveToTask(vOrigin));
	AddTask(new CBotTF2TauntTask(vOrigin, vGoto, fTauntDist));
}

void CBotTauntSchedule::Init()
{
	SetID(SCHED_TAUNT);
}

CBotTF2FindFlagSched::CBotTF2FindFlagSched(Vector vOrigin)
{
	AddTask(new CFindPathTask(vOrigin)); // first
	AddTask(new CBotTF2WaitFlagTask(vOrigin, true)); // second
}

void CBotTF2FindFlagSched::Init()
{
	SetID(SCHED_TF2_FIND_FLAG);
}

CBotPickupSched::CBotPickupSched(edict_t *pEdict)
{
	AddTask(new CFindPathTask(pEdict));
	AddTask(new CMoveToTask(pEdict));
}

void CBotPickupSched::Init()
{
	SetID(SCHED_PICKUP);
}

CBotPickupSchedUse::CBotPickupSchedUse(edict_t *pEdict)
{
	AddTask(new CFindPathTask(pEdict));
	AddTask(new CMoveToTask(pEdict));
	//AddTask(new CBotHL2DMUseButton(pEdict));
}

void CBotPickupSchedUse::Init()
{
	SetID(SCHED_PICKUP);

}

CBotInvestigateNoiseSched::CBotInvestigateNoiseSched(Vector vSource, Vector vAttackPoint)
{
	AddTask(new CFindPathTask(vSource, LOOK_NOISE));
	AddTask(new CBotInvestigateTask(vSource, 200.0f, vAttackPoint, true, 3.0f));
}

void CBotInvestigateNoiseSched::Init()
{
	SetID(SCHED_INVESTIGATE_NOISE);
}

CBotGotoOriginSched::CBotGotoOriginSched(Vector vOrigin)
{
	AddTask(new CFindPathTask(vOrigin)); // first
	AddTask(new CMoveToTask(vOrigin)); // second
}

CBotGotoOriginSched::CBotGotoOriginSched(edict_t *pEdict)
{
	AddTask(new CFindPathTask(pEdict));
	AddTask(new CMoveToTask(pEdict));
}

void CBotGotoOriginSched::Init()
{
	SetID(SCHED_GOTO_ORIGIN);
}

CBotDefendSched::CBotDefendSched(Vector vOrigin, float fMaxTime)
{
	AddTask(new CFindPathTask(vOrigin));
	AddTask(new CBotDefendTask(vOrigin, fMaxTime));
}

#if !defined USE_NAVMESH
CBotDefendSched::CBotDefendSched(int iWaypointID, float fMaxTime)
{
	CWaypoint *pWaypoint;

	pWaypoint = CWaypoints::GetWaypoint(iWaypointID);

	AddTask(new CFindPathTask(iWaypointID));
	AddTask(new CBotDefendTask(pWaypoint->GetOrigin(), fMaxTime, 8, false, Vector(0, 0, 0), LOOK_SNIPE, pWaypoint->GetFlags()));
}
#endif

void CBotDefendSched::Init()
{
	SetID(SCHED_DEFEND);
}

CBotRemoveSapperSched::CBotRemoveSapperSched(edict_t *pBuilding, eObjectType id)
{
	CFindPathTask *pathtask = new CFindPathTask(pBuilding);
	AddTask(pathtask);
	pathtask->CompleteInRangeFromEdict();
	pathtask->SetRange(150.0f);
	AddTask(new CBotRemoveSapper(pBuilding, id));
}

void CBotRemoveSapperSched::Init()
{
	SetID(SCHED_REMOVESAPPER);
}

CGotoHideSpotSched::CGotoHideSpotSched(CBot *pBot, edict_t *pEdict, bool bIsGrenade)
{
	// run at flank while shooting	
	CFindPathTask *pHideGoalPoint = new CFindPathTask(pEdict);

	pBot->SetCoverFrom(pEdict);
	AddTask(new CFindGoodHideSpot(pEdict));
	AddTask(pHideGoalPoint);
	//if ( bIsGrenade )
	//AddTask(new CDODWaitForGrenadeTask(pEdict));

	// don't need to hide if the player we're hiding from died while we're running away
	pHideGoalPoint->FailIfTaskEdictDead();
	pHideGoalPoint->SetLookTask(LOOK_WAYPOINT);
	// no interrupts, should be a quick waypoint path anyway
	pHideGoalPoint->SetNoInterruptions();
	// get vector from good hide spot task
	pHideGoalPoint->GetPassedVector();
	pHideGoalPoint->DontGoToEdict();
	if (bIsGrenade)
	{
		pHideGoalPoint->SetRange(BLAST_RADIUS + 100.0f);
		pHideGoalPoint->CompleteOutOfRangeFromEdict();
	}
}

CGotoHideSpotSched::CGotoHideSpotSched(CBot *pBot, Vector vOrigin, IBotTaskInterrupt *interrupt)
{
	// run at flank while shooting	
	CFindPathTask *pHideGoalPoint = new CFindPathTask();

	pBot->SetCoverFrom(NULL);
	AddTask(new CFindGoodHideSpot(vOrigin));
	AddTask(pHideGoalPoint);

	// no interrupts, should be a quick waypoint path anyway
	pHideGoalPoint->SetNoInterruptions();
	pHideGoalPoint->SetInterruptFunction(interrupt);
	// get vector from good hide spot task
	pHideGoalPoint->GetPassedVector();
}

void CGotoHideSpotSched::Init()
{
	SetID(SCHED_GOOD_HIDE_SPOT);
}

CGotoNestSched::CGotoNestSched(int iWaypoint)
{
	//AddTask(new CFindGoodHideSpot(1));
	//AddTask(new CNestTask());
}
void CGotoNestSched::Init()
{
	SetID(SCHED_GOTONEST);
}

CCrouchHideSched::CCrouchHideSched(edict_t *pCoverFrom)
{
	AddTask(new CCrouchHideTask(pCoverFrom));
}

void CCrouchHideSched::Init()
{
	SetID(SCHED_CROUCH_AND_HIDE);
}

CBotTF2AttackSentryGun::CBotTF2AttackSentryGun(edict_t *pSentry, CBotWeapon *pWeapon)
{
	CFindPathTask *path = new CFindPathTask(pSentry);

	AddTask(path);
	AddTask(new CBotTF2AttackSentryGunTask(pSentry, pWeapon));

	path->CompleteInRangeFromEdict();
	path->CompleteIfSeeTaskEdict();

	path->SetRange(pWeapon->PrimaryMaxRange() - 100);
}

CBotAttackSched::CBotAttackSched(edict_t *pEdict)
{
	AddTask(new CAttackEntityTask(pEdict));
	//AddTask(new CFindGoodHideSpot(pEdict));
}

void CBotAttackSched::Init()
{
	SetID(SCHED_ATTACK);
}

#if defined USE_NAVMESH
CBotAttackPointSched::CBotAttackPointSched(Vector vPoint, int iRadius, int iArea, bool bHasRoute, Vector vRoute, bool bNest, edict_t *pLastEnemySentry)
{
	// First find random route 
	if(bHasRoute)
	{
		CFindPathTask *toRoute = new CFindPathTask(vRoute);
		AddTask(toRoute); // first

		if(bNest)
			AddTask(new CBotNest());
	}

	CFindPathTask *toPoint = new CFindPathTask(vPoint);
	AddTask(toPoint); // second / first
	AddTask(new CBotTF2AttackPoint(iArea, vPoint, iRadius)); // third / second 
}
#else
CBotAttackPointSched::CBotAttackPointSched(Vector vPoint, int iRadius, int iArea, bool bHasRoute, Vector vRoute, bool bNest, edict_t *pLastEnemySentry)
{
	int iDangerWpt = -1;

	if (pLastEnemySentry != NULL)
		iDangerWpt = CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(pLastEnemySentry), 200.0f, -1, true, true);

	// First find random route 
	if (bHasRoute)
	{
		CFindPathTask *toRoute = new CFindPathTask(vRoute);
		AddTask(toRoute); // first
		toRoute->SetDangerPoint(iDangerWpt);

		if (bNest)
			AddTask(new CBotNest());
	}

	CFindPathTask *toPoint = new CFindPathTask(vPoint);
	AddTask(toPoint); // second / first
	toPoint->SetDangerPoint(iDangerWpt);
	AddTask(new CBotTF2AttackPoint(iArea, vPoint, iRadius)); // third / second 
}
#endif

void CBotAttackPointSched::Init()
{
	SetID(SCHED_ATTACKPOINT);
}

CBotTF2MessAroundSched::CBotTF2MessAroundSched(edict_t *pFriendly, int iMaxVoiceCmd)
{
	AddTask(new CMessAround(pFriendly, iMaxVoiceCmd));
}

void CBotTF2MessAroundSched::Init()
{
	SetID(SCHED_MESSAROUND);
}

CBotFollowLastEnemy::CBotFollowLastEnemy(CBot *pBot, edict_t *pEnemy, Vector vLastSee)
{
	Vector vVelocity = CBotGlobals::GetVelocity(pEnemy);

	CFindPathTask *pFindPath = new CFindPathTask(vLastSee, LOOK_LAST_ENEMY);
	pFindPath->SetCompleteInterrupt(CONDITION_SEE_CUR_ENEMY);
	AddTask(pFindPath);
	pFindPath->SetNoInterruptions();

	AddTask(new CFindLastEnemy(vLastSee, vVelocity));
}

void CBotFollowLastEnemy::Init()
{
	SetID(SCHED_FOLLOW_LAST_ENEMY);
}

CBotTF2ShootLastEnemyPos::CBotTF2ShootLastEnemyPos(Vector vLastSeeEnemyPos, Vector vVelocity, edict_t *pLastEnemy)
{
	AddTask(new CBotTF2ShootLastEnemyPosition(vLastSeeEnemyPos, pLastEnemy, vVelocity));
}

void CBotTF2ShootLastEnemyPos::Init()
{
	SetID(SCHED_SHOOT_LAST_ENEMY_POS);
}

CDeployMachineGunSched::CDeployMachineGunSched(CBotWeapon *pWeapon, CWaypoint *pWaypoint, Vector vEnemy)
{
	//AddTask(new CFindPathTask(CWaypoints::GetWaypointIndex(pWaypoint), LOOK_LAST_ENEMY));
	//AddTask(new CBotDODSnipe(pWeapon,pWaypoint->getOrigin(),pWaypoint->getAimYaw(),true,vEnemy.z,pWaypoint->getFlags()));
}

CBotDefendPointSched::CBotDefendPointSched(Vector vPoint, int iRadius, int iArea)
{
	AddTask(new CFindPathTask(vPoint)); // first
	AddTask(new CBotTF2DefendPoint(iArea, vPoint, iRadius)); // second
}

void CBotDefendPointSched::Init()
{
	SetID(SCHED_DEFENDPOINT);
}

void CBotSchedule::Execute(CBot *pBot)
{
	// current task
	static CBotTask *pTask;
	static eTaskState iState;

	if (m_Tasks.IsEmpty())
	{
		m_bFailed = true;
		return;
	}

	pTask = m_Tasks.GetFrontInfo();

	if (pTask == NULL)
	{
		m_bFailed = true;
		return;
	}

	iState = pTask->IsInterrupted(pBot);

	if (iState == STATE_FAIL)
		pTask->Fail();
	else if (iState == STATE_COMPLETE)
		pTask->Complete();
	else // still running
	{
		// timed out ??
		if (pTask->TimedOut())
			pTask->Fail(); // fail
		else
		{
			pTask->Execute(pBot, this); // run
		}
	}

	if (pTask->HasFailed())
	{
		m_bFailed = true;
	}
	else if (pTask->IsComplete())
	{
		RemoveTop();
	}
}

void CBotSchedule::AddTask(CBotTask *pTask)
{
	// Initialize
	pTask->Init();
	// add
	m_Tasks.Add(pTask);
}

void CBotSchedule::FreeMemory()
{
	if(HasFailed())
	{
		CFmtStr fmt;
		NDebugOverlay::ScreenText(0.5, 0.5, fmt.sprintf("Schedule %s(%d) has failed!", GetIDString(), m_iSchedId), 255, 34, 34, 192, 3.0f);
	}

	m_Tasks.Destroy();
}

void CBotSchedule::RemoveTop()
{
	CBotTask *pTask = m_Tasks.GetFrontInfo();

	m_Tasks.RemoveFront();

	delete pTask;
}

const char *CBotSchedule::GetIDString()
{
	return g_szSchedules[m_iSchedId];
}

CBotTask *CBotSchedule::CurrentTask()
{
	if(m_Tasks.IsEmpty())
		return NULL;

	return m_Tasks.GetFrontInfo();
}

///////////////////////////////////////////
CBotSchedule::CBotSchedule()
{
	_Init();
}

void CBotSchedule::_Init()
{
	m_bFailed = false;
	m_bitsPass = 0;
	m_iSchedId = SCHED_NONE;

	// pass information
	iPass = 0;
	fPass = 0;
	vPass = 0;
	pPass = 0;

	Init();
}

void CBotSchedule::PassInt(int i)
{
	iPass = i;
	m_bitsPass |= BITS_SCHED_PASS_INT;
}
void CBotSchedule::PassFloat(float f)
{
	fPass = f;
	m_bitsPass |= BITS_SCHED_PASS_FLOAT;
}
void CBotSchedule::PassVector(Vector *v)
{
	vPass = v;
	m_bitsPass |= BITS_SCHED_PASS_VECTOR;
}
void CBotSchedule::PassEdict(edict_t *p)
{
	pPass = p;
	m_bitsPass |= BITS_SCHED_PASS_EDICT;
}

bool CBotSchedules::HasSchedule(eBotSchedule iSchedule)
{
	dataQueue<CBotSchedule*> tempQueue = m_Schedules;

	while(!tempQueue.IsEmpty())
	{
		CBotSchedule *sched = tempQueue.ChooseFrom();

		if(sched->IsID(iSchedule))
		{
			tempQueue.Init();
			return true;
		}
	}

	return false;
}

bool CBotSchedules::IsCurrentSchedule(eBotSchedule iSchedule)
{
	if(m_Schedules.IsEmpty())
		return false;

	return m_Schedules.GetFrontInfo()->IsID(iSchedule);
}

void CBotSchedules::RemoveSchedule(eBotSchedule iSchedule)
{
	dataQueue<CBotSchedule*> tempQueue = m_Schedules;

	CBotSchedule *toRemove = NULL;

	while(!tempQueue.IsEmpty())
	{
		CBotSchedule *sched = tempQueue.ChooseFrom();

		if(sched->IsID(iSchedule))
		{
			toRemove = sched;
			tempQueue.Init();
			break;
		}
	}

	if(toRemove)
		m_Schedules.Remove(toRemove);

	return;
}

void CBotSchedules::Execute(CBot *pBot)
{
	if(IsEmpty())
		return;

	CBotSchedule *pSched = m_Schedules.GetFrontInfo();

	pSched->Execute(pBot);

	if(pSched->IsComplete() || pSched->HasFailed())
		RemoveTop();
}

void CBotSchedules::RemoveTop()
{
	CBotSchedule *pSched = m_Schedules.GetFrontInfo();

	m_Schedules.RemoveFront();

	pSched->FreeMemory();

	delete pSched;
}

CBotTask *CBotSchedules::GetCurrentTask()
{
	CBotSchedule *sched = GetCurrentSchedule();

	if(sched != NULL)
	{
		return sched->CurrentTask();
	}

	return NULL;
}

CBotSchedule *CBotSchedules::GetCurrentSchedule()
{
	if(IsEmpty())
		return NULL;

	return m_Schedules.GetFrontInfo();
}
