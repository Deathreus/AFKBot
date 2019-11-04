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
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */
//============================================================================//
//
// HPB_bot2.cpp - bot source code file (Copyright 2004, Jeffrey "botman" Broome)
//
//============================================================================//

#include "mathlib.h"
#include "vector.h"
#include "bitbuf.h"
#include "in_buttons.h"
#include "tier0/threadtools.h" // for critical sections
#include "vstdlib/vstdlib.h"
#include "vstdlib/random.h" // for random functions
#include "iservernetworkable.h" // may come in handy
#include <inetchannel.h>

#include "bot_base.h"
#include "bot_schedule.h"
#include "bot_buttons.h"
#include "bot_navigator.h"
//#include "bot_css_bot.h"
//#include "bot_coop.h"
//#include "bot_zombie.h"
//#include "bot_dod_bot.h"
//#include "bot_hldm_bot.h"
//#include "bot_hl1dmsrc_bot.h"
#include "bot_fortress.h"
#include "bot_visibles.h"
//#include "bot_memory.h"
#include "bot_weapons.h"
#include "bot_profile.h"
#include "bot_getprop.h"

#if defined USE_NAVMESH
#include "bot_navmesh.h"
#else
#include "bot_waypoint.h"
#include "bot_waypoint_locations.h"
#endif

#ifndef __linux__
#include <ndebugoverlay.h>
#endif

#ifdef GetClassName
 #undef GetClassName
#endif


// instantiate bots -- make different for different mods
CBot **CBots::m_Bots = NULL;

const float CBot::m_fAttackLowestHoldTime = 0.1f;
const float CBot::m_fAttackHighestHoldTime = 0.6f;
const float CBot::m_fAttackLowestLetGoTime = 0.1f;
const float CBot::m_fAttackHighestLetGoTime = 0.5f;

extern const char *g_szUtils[BOT_UTIL_MAX + 1];

extern IPlayerInfoManager *playerinfomanager;
extern IServerPluginHelpers *helpers;
extern IServerGameClients* gameclients;
extern IServerGameEnts *gameents;

extern IForward *forwardOnAFK;

/*	to be used...
int CBots::m_iMaxBots = -1;
int CBots::m_iMinBots = -1;
*/

extern ConVar bot_debug_iglev;
extern ConVar bot_debug_dont_shoot;
extern ConVar bot_debug_notasks;
extern ConVar bot_skill_randomize;
extern ConVar bot_skill_min;
extern ConVar bot_skill_max;
extern ConVar bot_sensitivity_min;
extern ConVar bot_sensitivity_max;
extern ConVar bot_braveness_min;
extern ConVar bot_braveness_max;
extern ConVar bot_visrevs_min;
extern ConVar bot_visrevs_max;
extern ConVar bot_visrevs_client_min;
extern ConVar bot_visrevs_client_max;
extern ConVar bot_pathrevs_min;
extern ConVar bot_pathrevs_max;
extern ConVar bot_avoid_radius;
extern ConVar bot_avoid_strength;
extern ConVar bot_use_vc_commands;
extern ConVar bot_listen_dist;
extern ConVar bot_footstep_speed;
extern ConVar bot_stats_inrange_dist;
extern ConVar bot_anglespeed;
extern ConVar bot_supermode;
extern ConVar bot_stop;
extern ConVar bot_dont_move;
extern ConVar bot_move_forward;
extern ConVar bot_attack;
extern ConVar bot_printstatus;

const char *const g_szLookTaskToString[LOOK_MAX] =
{
	"LOOK_NONE",
	"LOOK_VECTOR",
	"LOOK_WAYPOINT",
	"LOOK_WAYPOINT_NEXT_ONLY",
	"LOOK_AROUND",
	"LOOK_ENEMY",
	"LOOK_LAST_ENEMY",
	"LOOK_HURT_ORIGIN",
	"LOOK_EDICT",
	"LOOK_GROUND",
	"LOOK_SNIPE",
	"LOOK_WAYPOINT_AIM",
	"LOOK_BUILD",
	"LOOK_NOISE",
};

const char *const g_szConditionsDebugStrings[CONDITION_MAX_BITS + 1] = {
	"CONDITION_ENEMY_OBSCURED",
	"CONDITION_NO_WEAPON",
	"CONDITION_OUT_OF_AMMO",
	"CONDITION_SEE_CUR_ENEMY",
	"CONDITION_ENEMY_DEAD",
	"CONDITION_SEE_WAYPOINT",
	"CONDITION_NEED_AMMO",
	"CONDITION_NEED_HEALTH",
	"CONDITION_SEE_LOOK_VECTOR",
	"CONDITION_POINT_CAPTURED",
	"CONDITION_PUSH",
	"CONDITION_LIFT",
	"CONDITION_SEE_HEAL",
	"CONDITION_SEE_LAST_ENEMY_POS",
	"CONDITION_CHANGED",
	"CONDITION_COVERT",
	"CONDITION_RUN",
	"CONDITION_GREN",
	"CONDITION_NEED_BOMB",
	"CONDITION_SEE_ENEMY_HEAD",
	"CONDITION_PRONE",
	"CONDITION_PARANOID",
	"CONDITION_DEFENSIVE",
	"CONDITION_BUILDING_SAPPED",
	"CONDITION_SEE_ENEMY_GROUND"
};

// Borrowed from RCBot1
bool BotFunc_BreakableIsEnemy(edict_t *pBreakable, edict_t *pEdict)
{
	int flags = GetEntData<int>(pBreakable, "m_fFlags");

	// i. explosives required to blow breakable
	// ii. OR is not a world brush (non breakable) and can be broken by shooting
	if (!(flags & FL_WORLDBRUSH))
	{
		Vector vSize = pBreakable->GetCollideable()->OBBMaxs() - pBreakable->GetCollideable()->OBBMins();
		Vector vMySize = pEdict->GetCollideable()->OBBMaxs() - pEdict->GetCollideable()->OBBMins();

		if ((vSize.x >= vMySize.x) ||
			(vSize.y >= vMySize.y) ||
			(vSize.z >= (vMySize.z / 2))) // this breakable could block my path
		{
			// 00000000001111111111222222222233333333334
			// 01234567890123456789012345678901234567890
			// models/props_c17/oildrum001_explosive.mdl
			const char *model = pBreakable->GetIServerEntity()->GetModelName().ToCStr();

			if ((model[13] == 'c') && (model[17] == 'o') && (model[20] == 'd') && (model[28] == 'e')) // explosive
				return false;
			// Only shoot breakables that are bigger than me (crouch size)
			// or that target something...
			return (GetEntSend<int>(pBreakable, "m_iHealth") < 1000); // breakable still visible (not broken yet)
		}
	}

	return false;
}

///////////////////////////////////////
// voice commands
////////////////////////////////////////////////
void CBroadcastVoiceCommand::Execute(CBot *pBot)
{
	if (!m_pPlayer)
		return;

	if (m_pPlayer == pBot->GetEdict())
		return;

	if (pBot->IsEnemy(m_pPlayer, false))
	{
		// listen to enemy voice commands if they are nearby
		if (pBot->WantToListen() && pBot->WantToListenToPlayerAttack(m_pPlayer) && (pBot->DistanceFrom(m_pPlayer) < 1024.0f))
			pBot->ListenToPlayer(m_pPlayer, true, false);
	}
	else
		pBot->HearVoiceCommand(m_pPlayer, m_VoiceCmd);
}
///////////////////////////////////////
void CBot::RunPlayerMove()
{
	int cmdnumbr = m_pCmd.command_number + 1;

	//////////////////////////////////
	Q_memset(&m_pCmd, 0, sizeof(CUserCmd));
	//////////////////////////////////

	if (bot_dont_move.GetBool())
	{
		m_pCmd.forwardmove = 0;
		m_pCmd.sidemove = 0;
	}
	else
	{
		m_pCmd.forwardmove = m_fForwardSpeed;
		m_pCmd.sidemove = m_fSideSpeed;
		m_pCmd.upmove = m_fUpSpeed;
	}

	m_pCmd.buttons = m_iButtons;
	m_pCmd.impulse = m_iImpulse;
	m_pCmd.viewangles = m_vViewAngles;
	m_pCmd.weaponselect = m_iSelectWeapon;
	m_pCmd.tick_count = gpGlobals->tickcount;
	m_pCmd.command_number = cmdnumbr;

	if (bot_attack.GetInt() == 1)
		m_pCmd.buttons = IN_ATTACK;

	m_iSelectWeapon = 0;
	m_iImpulse = 0;
}

bool CBot::StartGame()
{
	return true;
}

#if defined USE_NAVMESH
bool CBot::WalkingTowardsWaypoint(INavMeshArea *pWaypoint, bool *bOffsetApplied, Vector &vOffset)
{
	if((pWaypoint->GetAttributes() & NAV_MESH_CROUCH))
	{
		Duck(true);
	}

	if((pWaypoint->GetAttributes() & NAV_MESH_NO_HOSTAGES))
	{
		UpdateCondition(CONDITION_LIFT);
	}
	else
	{
		RemoveCondition(CONDITION_LIFT);
	}

	if(pWaypoint->GetAttributes() & NAV_MESH_PRECISE)
		return false;

	if(!*bOffsetApplied)
	{
		/*float fRadius = (pWaypoint->GetSWCornerZ() - pWaypoint->GetNECornerZ());

		if(fRadius > 0)
			vOffset = Vector(RandomFloat(-fRadius, fRadius), RandomFloat(-fRadius, fRadius), 0);
		else
			vOffset = Vector(0, 0, 0);

		*bOffsetApplied = true;*/

		return true;
	}

	return false;
}
#else
// returns true if offset has been applied when not before
bool CBot::WalkingTowardsWaypoint(CWaypoint *pWaypoint, bool *bOffsetApplied, Vector &vOffset)
{
	if (pWaypoint->HasFlag(CWaypointTypes::W_FL_CROUCH))
	{
		Duck(true);
	}

	if (pWaypoint->HasFlag(CWaypointTypes::W_FL_LIFT))
	{
		UpdateCondition(CONDITION_LIFT);
	}
	else
	{
		RemoveCondition(CONDITION_LIFT);
	}

	if (!*bOffsetApplied)
	{
		float fRadius = pWaypoint->GetRadius();

		if (fRadius > 0)
			vOffset = Vector(RandomFloat(-fRadius, fRadius), RandomFloat(-fRadius, fRadius), 0);
		else
			vOffset = Vector(0, 0, 0);

		*bOffsetApplied = true;

		return true;
	}

	return false;
}
#endif

void CBot::SetEdict(edict_t *pEdict)
{
	m_pEdict = pEdict;
	m_bUsed = true;

	if (m_pEdict)
	{
		m_pPI = playerinfomanager->GetPlayerInfo(m_pEdict);
		ke::SafeStrcpy(m_szBotName, sizeof(m_szBotName), m_pPI->GetName());
	}
	else
	{
		return;
	}

	SpawnInit();
}

bool CBot::IsUnderWater()
{
	return CClassInterface::GetWaterLevel(m_pEdict) > 1; //m_pController->IsEFlagSet(EFL_TOUCHING_FLUID);
}

// return false if there is a problem
bool CBot::CreateBotFromEdict(edict_t *pEdict, CBotProfile *pProfile)
{
	Init();
	SetEdict(pEdict);
	Setup();
	m_fTimeCreated = TIME_NOW;

	/////////////////////////////

	m_pProfile = pProfile;

	if (IsAlive() && !m_pPI->IsObserver())
	{
		m_iDesiredTeam = GetTeam();
		if(IsTF2())
			m_iDesiredClass = ((CBotTF2*)this)->GetClass();
	}
	else
	{
		m_iDesiredTeam = RandomInt(2, 3);
		m_iDesiredClass = RandomInt(1, 9);
	}

	return true;
}

bool CBot::CheckStuck()
{
	static float fTime;

	float fSpeed;
	float fIdealSpeed;

	if (!MoveToIsValid())
		return false;
	if (bot_dont_move.GetBool()) // bots not moving
		return false;
	if (HasEnemy())
		return false;

	fTime = TIME_NOW;

	if (m_fLastWaypointVisible == 0)
	{
		m_bFailNextMove = false;

		if (!HasSomeConditions(CONDITION_SEE_WAYPOINT))
			m_fLastWaypointVisible = fTime;
	}
	else
	{
		if (HasSomeConditions(CONDITION_SEE_WAYPOINT))
			m_fLastWaypointVisible = 0;
		else
		{
			if ((m_fLastWaypointVisible + 2.0) < fTime)
			{
				m_fLastWaypointVisible = 0;
				m_bFailNextMove = true;

				return true;
			}
		}
	}

	if (m_ftWaypointStuckTime.HasStarted() && m_ftWaypointStuckTime.IsElapsed())
	{
		m_bFailNextMove = true;
		m_ftWaypointStuckTime.Start(RandomFloat(15.0f, 20.0f));
	}

	if (!m_ftCheckStuckTime.IsElapsed())
		return m_bThinkStuck;

	if (HasSomeConditions(CONDITION_LIFT) || (OnLadder()))//fabs(m_vMoveTo.z - getOrigin().z) > 48 )
	{
		if (m_vVelocity.z != 0.0f)
			return false;
	}

	fSpeed = m_vVelocity.Length();
	fIdealSpeed = m_fIdealMoveSpeed;

	if (m_pButtons->HoldingButton(IN_DUCK))
		fIdealSpeed /= 2;

	if (fIdealSpeed == 0)
	{
		m_bThinkStuck = false; // not stuck
		m_fPercentMoved = 1.0f;
	}
	else
	{
		// alpha percentage check
		m_fPercentMoved = (m_fPercentMoved / 2) + ((fSpeed / fIdealSpeed) / 2);

		if (m_fPercentMoved < 0.1f)
		{
			m_bThinkStuck = true;
			m_fPercentMoved = 0.1f;

			m_pButtons->Jump();
			m_pButtons->Duck(0.25f, RandomFloat(0.2f, 0.4f));

			if (m_ftStrafeTime.IsElapsed())
			{
				ReduceTouchDistance();

				if (CBotGlobals::YawAngleFromEdict(m_pEdict, m_vMoveTo) > 0)
					m_fSideSpeed = m_fIdealMoveSpeed / 2;
				else
					m_fSideSpeed = -(m_fIdealMoveSpeed / 2);

			}

			m_ftCheckStuckTime.Reset();
		}
		else
			m_bThinkStuck = false;
	}

	return m_bThinkStuck;
}

bool CBot::IsVisible(edict_t *pEdict)
{
	return m_pVisibles->IsVisible(pEdict);
}

bool CBot::CanAvoid(edict_t *pEntity)
{
	float distance;
	Vector vAvoidOrigin;

	if (!CBotGlobals::EntityIsValid(pEntity))
		return false;
	if (m_pEdict == pEntity) // can't avoid self!!!
		return false;
	if (m_pLookEdict == pEntity)
		return false;
	if (m_pLastEnemy == pEntity)
		return false;

	vAvoidOrigin = CBotGlobals::EntityOrigin(pEntity);

	distance = DistanceFrom(vAvoidOrigin);

	if ((distance > 1) && (distance < bot_avoid_radius.GetFloat()) && (fabs(GetOrigin().z - vAvoidOrigin.z) < 32))
	{
		SolidType_t solid = pEntity->GetCollideable()->GetSolid();

		if ((solid == SOLID_BBOX) || (solid == SOLID_VPHYSICS))
		{
			return IsEnemy(pEntity, false);
		}
	}

	return false;
}

void CBot::ReachedCoverSpot(int flags)
{

}

// something now visiable or not visible anymore
bool CBot::SetVisible(edict_t *pEntity, bool bVisible)
{
	bool bValid = (pEntity->GetUnknown() != NULL);

	if (bValid && bVisible)
	{
		if (CanAvoid(pEntity))
		{
			if (!m_pAvoidEntity.IsValid() || (DistanceFrom(pEntity) < DistanceFrom(m_pAvoidEntity)))
				m_pAvoidEntity = pEntity;
		}
	}
	else
	{
		if (pEntity == m_pAvoidEntity)
			m_pAvoidEntity = MyEHandle();
		if (pEntity == m_pEnemy)
		{
			m_pLastEnemy = m_pEnemy;
		}
	}

	// return if entity is valid or not
	return bValid;
}

bool CBot::IsUsingProfile(CBotProfile *pProfile)
{
	return (m_pProfile == pProfile);
}

void CBot::CurrentlyDead()
{
	/*if ( m_bNeedToInit )
	{
	spawnInit();
	m_bNeedToInit = false;
	}*/

	//attack();

	// keep updating until alive
	m_fSpawnTime = TIME_NOW;

	return;
}

CBotWeapon *CBot::GetCurrentWeapon()
{
	return m_pWeapons->GetActiveWeapon(m_pPI->GetWeaponName());
}

SH_DECL_MANUALHOOK1_void(Weapon_Equip, 0, 0, 0, /*CBaseCombatWeapon*/CBaseEntity *)
void CBot::SelectWeaponSlot(int iSlot)
{
	static bool bSetup = false;
	static bool bWarned = false;
	if (!bSetup)
	{
		int iOffset = 0;
		if (!g_pGameConf->GetOffset("Weapon_Equip", &iOffset))
		{
			if (!bWarned)
			{
				smutils->LogError(myself, "Failed to get Weapon_Equip offset");
				bWarned = true;
			}

			return;
		}

		SH_MANUALHOOK_RECONFIGURE(Weapon_Equip, iOffset, 0, 0);
		bSetup = true;
	}

	CBaseEntity *pEntity = m_pEdict->GetNetworkable()->GetBaseEntity();
	if (pEntity)
	{
		edict_t *pWEdict = CBotGlobals::GetPlayerWeaponSlot(m_pEdict, iSlot);
		CBaseEntity *pWEntity = gameents->EdictToBaseEntity(pWEdict);
		if (pWEntity)
		{
			SH_MCALL(pEntity, Weapon_Equip)(pWEntity);
			return;
		}
	}

	/*char cmd[8];

	sprintf(cmd, "slot%d", iSlot);

	helpers->ClientCommand(m_pEdict, cmd);
	//m_iSelectWeapon = iSlot;*/
}

CBotWeapon *CBot::GetBestWeapon(edict_t *pEnemy, bool bAllowMelee, bool bAllowMeleeFallback, bool bMeleeOnly, bool bExplosivesOnly)
{
	return m_pWeapons->GetBestWeapon(pEnemy, bAllowMelee, bAllowMeleeFallback, bMeleeOnly, bExplosivesOnly);
}

bool CBot::IsHoldingPrimaryAttack()
{
	return m_pButtons->HoldingButton(IN_ATTACK);
}

void CBot::SquadInPosition()
{
	if (m_uSquadDetail.b1.said_in_position == false)
	{
		// say something here
		SayInPosition();
		m_uSquadDetail.b1.said_in_position = true;
	}
}

void CBot::Kill()
{
	helpers->ClientCommand(m_pEdict, "kill\n");
}

void CBot::Think()
{
	const float fTime = TIME_NOW;

	static const int ignoreLevel = bot_debug_iglev.GetInt();

	// important!!!
	//
	m_iLookPriority = 0;
	m_iMovePriority = 0;
	m_iMoveSpeedPriority = 0;

	if (ignoreLevel < 11)
	{
		// if bot is not in game, start it!!!
		if (!StartGame())
		{
			DoButtons();
			return; // don't do anything just now
		}

		DoButtons();
	}

	if (!IsAlive())
	{
		CurrentlyDead();
		return;
	}

	if (ignoreLevel < 10)
	{
		CheckDependantEntities();

		//m_bNeedToInit = true;

		DoMove();
		DoLook();
	}

	if (m_fNextThink > fTime)
		return;

	m_pButtons->LetGoAllButtons(false);

	m_fNextThink = fTime + 0.03f;

	if (m_pWeapons)
	{
		// update carried weapons
		if (m_pWeapons->Update(OverrideAmmoTypes()))
		{
			m_pPrimaryWeapon = m_pWeapons->GetPrimaryWeapon();
		}
	}

	if (ignoreLevel < 9)
	{
		m_pVisibles->UpdateVisibles();
	}

	if (ignoreLevel < 8)
	{
		if (CheckStuck())
		{
			// look in the direction I'm going to see what I'm stuck on
			SetLookAtTask(LOOK_WAYPOINT, RandomFloat(2.0f, 4.0f));
		}
	}
	
	m_bOpenFire = true;
	m_bWantToListen = true;
	m_bWantToChangeWeapon = true;

	if (!bot_debug_notasks.GetBool() && ignoreLevel < 7)
	{
		GetTasks();
	}

	if (ignoreLevel < 6)
	{
		WantToInvestigateSound(true);
		SetMoveLookPriority(MOVELOOK_TASK);
		m_pSchedules->Execute(this);
		SetMoveLookPriority(MOVELOOK_THINK);
	}

	// fix -- put listening AFTER task executed, as m_bWantToListen may update
	if (ignoreLevel < 5)
	{
		if (m_bWantToListen && !HasEnemy() && !HasSomeConditions(CONDITION_SEE_CUR_ENEMY) && m_ftWantToListenTime.IsElapsed())
		{
			SetMoveLookPriority(MOVELOOK_LISTEN);
			ListenForPlayers();
			SetMoveLookPriority(MOVELOOK_THINK);
		}
		else if (HasEnemy())
		{
			// got an enemy -- reset 
			m_PlayerListeningTo = MyEHandle();
			m_ftLookSetTime.Invalidate();
			m_ftListenTime = 0.0f;
			m_bListenPositionValid = false;
			m_ftWantToListenTime.Start(1.0f);

			// is player
			if (ENTINDEX(m_pEnemy) <= MAX_PLAYERS)
				m_fLastSeeEnemyPlayer = TIME_NOW;
		}
	}

	if (ignoreLevel < 4)
	{
		m_vGoal = m_pNavigator->GetGoalOrigin();

		if (m_pNavigator->HasNextPoint())
		{
			m_pNavigator->UpdatePosition();
		}
		else
		{
			m_ftWaypointStuckTime.Invalidate();
			StopMoving();
			SetLookAtTask(LOOK_AROUND);
		}
	}

	if (ignoreLevel < 3)
	{
		// update m_pEnemy with findEnemy()
		m_pOldEnemy = m_pEnemy;
		m_pEnemy = MyEHandle();

		if (m_pOldEnemy)
			FindEnemy(m_pOldEnemy); // any better enemies than this one?
		else
			FindEnemy();
	}

	UpdateConditions();

	if (m_ftUpdateOriginTime.IsElapsed())
	{
		Vector vOrigin = GetOrigin();

		m_vVelocity = m_vLastOrigin - vOrigin;
		m_vLastOrigin = vOrigin;
		m_ftUpdateOriginTime.Reset();
	}

	SetMoveLookPriority(MOVELOOK_MODTHINK);

	if (ignoreLevel < 2)
	{
		ModThink();
	}

	if (ignoreLevel < 1)
	{
		if (m_ftStatsTime.IsElapsed())
		{
			UpdateStatistics();
			m_ftStatsTime.Reset();
		}
	}

	SetMoveLookPriority(MOVELOOK_ATTACK);

	if (!bot_debug_dont_shoot.GetBool())
		HandleWeapons();

	// deal with voice commands bot wants to say,
	// incase that he wants to use it in between frames (e.g. during an event call)
	// deal with it here
	if (m_ftNextVoiceCommand.IsElapsed() && !m_nextVoicecmd.empty())
	{
		byte cmd = m_nextVoicecmd.front();

		m_ftNextVoiceCommand = RandomFloat(0.4f, 1.2f);

		if (m_ftLastVoiceCommand[cmd].IsElapsed())
		{
			VoiceCommand(cmd);
			m_ftLastVoiceCommand[cmd] = RandomFloat(6.0f, 10.0f);
		}

		m_nextVoicecmd.pop();
	}

	m_iPrevHealth = m_pPI->GetHealth();

	m_bInitAlive = false;

#if defined _DEBUG
	static FrameTimer ftDebugThrottle;
	if(ftDebugThrottle.IsElapsed() && bot_debug.GetBool())
	{
		DebugBot();
		ftDebugThrottle.Start(0.33f);
	}
#endif
}

void CBot::AddVoiceCommand(int cmd)
{
	if (bot_use_vc_commands.GetBool() && m_ftLastVoiceCommand[cmd].IsElapsed())
	{
		m_nextVoicecmd.push(cmd);
		m_ftNextVoiceCommand = RandomFloat(0.2f, 1.0f);
	}
}

void CBot::HandleWeapons()
{
	//
	// Handle attacking at this point
	//
	if (m_pEnemy && !HasSomeConditions(CONDITION_ENEMY_DEAD) &&
		HasSomeConditions(CONDITION_SEE_CUR_ENEMY) && WantToShoot() &&
		IsVisible(m_pEnemy) && IsEnemy(m_pEnemy))
	{
		CBotWeapon *pWeapon;

		pWeapon = GetBestWeapon(m_pEnemy);

		if (m_bWantToChangeWeapon && (pWeapon != NULL) && (pWeapon != GetCurrentWeapon()) && pWeapon->GetWeaponIndex())
		{
			//selectWeaponSlot(pWeapon->getWeaponInfo()->getSlot());
			SelectWeapon(pWeapon->GetWeaponIndex());
		}

		SetLookAtTask(LOOK_ENEMY);

		if (!HandleAttack(pWeapon, m_pEnemy))
		{
			m_pEnemy = MyEHandle();
			m_pOldEnemy = MyEHandle();
			WantToShoot(false);
		}
	}
}

CBot::CBot()
{
	Init(true);
}

void CBot::Init(bool bVarInit)
{
	//m_bNeedToInit = false; // doing this now
	m_fLastHurtTime = 0.0f;
	m_iAmmo = NULL;
	m_pButtons = NULL;
	m_pNavigator = NULL;
	m_pSchedules = NULL;
	m_pVisibles = NULL;
	m_pEdict = MyEHandle();
	m_pFindEnemyFunc = NULL;
	m_bUsed = false;
	m_pPI = NULL;

	m_pWeapons = NULL;
	m_fTimeCreated = 0;
	m_pProfile = NULL;
	m_fIdealMoveSpeed = 320;
	m_fFov = BOT_DEFAULT_FOV;
	m_bOpenFire = true;
	m_pSquad = NULL;

	//m_pCmd.command_number = 0;

	if (bVarInit)
		SpawnInit();
}

edict_t *CBot::GetEdict()
{
	return m_pEdict.Get_Old();
}

void CBot::UpdateConditions()
{
	if (m_pEnemy.IsValid())
	{
		if (!CBotGlobals::EntityIsAlive(m_pEnemy))
		{
			UpdateCondition(CONDITION_ENEMY_DEAD);
			m_pEnemy = MyEHandle();
		}
		else
		{
			RemoveCondition(CONDITION_ENEMY_DEAD);

			// clear enemy
			if (m_pVisibles->IsVisible(m_pEnemy))
			{
				UpdateCondition(CONDITION_SEE_CUR_ENEMY);
				RemoveCondition(CONDITION_ENEMY_OBSCURED);
			}
			else
			{
				if (!m_pLastEnemy || (m_pLastEnemy != m_pEnemy))
					EnemyLost(m_pEnemy);

				SetLastEnemy(m_pEnemy);

				RemoveCondition(CONDITION_SEE_CUR_ENEMY);
				RemoveCondition(CONDITION_SEE_ENEMY_HEAD);
				UpdateCondition(CONDITION_ENEMY_OBSCURED);
			}
		}
	}
	else
	{
		RemoveCondition(CONDITION_SEE_CUR_ENEMY);
		RemoveCondition(CONDITION_ENEMY_OBSCURED);
		RemoveCondition(CONDITION_ENEMY_DEAD);
		RemoveCondition(CONDITION_SEE_ENEMY_HEAD);
	}

	if (m_pLastEnemy)
	{
		if (m_fLastSeeEnemy > 0.0f)
		{
			if (m_ftLastUpdateLastSeeEnemy.IsElapsed())
			{
				m_ftLastUpdateLastSeeEnemy.Reset();

				if (FVisible(m_vLastSeeEnemyBlastWaypoint))
					UpdateCondition(CONDITION_SEE_LAST_ENEMY_POS);
				else
					RemoveCondition(CONDITION_SEE_LAST_ENEMY_POS);
			}
		}
		else
			RemoveCondition(CONDITION_SEE_LAST_ENEMY_POS);
	}

	if (FVisible(m_vLookVector))
	{
		UpdateCondition(CONDITION_SEE_LOOK_VECTOR);
	}
	else
	{
		RemoveCondition(CONDITION_SEE_LOOK_VECTOR);
	}
}

#if defined USE_NAVMESH
bool CBot::CanGotoWaypoint(Vector vPrevWaypoint, INavMeshArea *pWaypoint, INavMeshArea *pPrev)
{
	if(pWaypoint->IsBlocked())
		return false;

	if(pPrev)
	{
		if((pPrev->GetCenter().z - pWaypoint->GetCenter().z) > 200.0f)
		{
			if(GetHealthPercent() <= 0.1f)
				return false;
		}
	}

	return true;
}
#else
// Called when working out route
bool CBot::CanGotoWaypoint(Vector vPrevWaypoint, CWaypoint *pWaypoint, CWaypoint *pPrev)
{
	if (pWaypoint->HasFlag(CWaypointTypes::W_FL_UNREACHABLE))
		return false;

	if (!pWaypoint->ForTeam(GetTeam()))
		return false;

	if (pWaypoint->HasFlag(CWaypointTypes::W_FL_OPENS_LATER))
	{
		if (pPrev != NULL)
		{
			return pPrev->IsPathOpened(pWaypoint->GetOrigin());
		}
		else if ((vPrevWaypoint != pWaypoint->GetOrigin()) && !CBotGlobals::CheckOpensLater(vPrevWaypoint, pWaypoint->GetOrigin()))
			return false;
	}

	if (pWaypoint->HasFlag(CWaypointTypes::W_FL_FALL))
	{
		if (GetHealthPercent() <= 0.1f)
		{
			if ((vPrevWaypoint.z - pWaypoint->GetOrigin().z) > 200.0f)
				return false;
		}
	}

	return true;
}
#endif

void CBot::UpdatePosition()
{
	m_pNavigator->RollBackPosition();
}

bool CBot::HandleAttack(CBotWeapon *pWeapon, edict_t *pEnemy)
{
	if (pWeapon)
	{
		ClearFailedWeaponSelect();

		if (pWeapon->IsMelee())
			SetMoveTo(CBotGlobals::EntityOrigin(pEnemy));

		if (pWeapon->MustHoldAttack())
			PrimaryAttack(true, 0.65f);
		else
			PrimaryAttack();
	}
	else
		PrimaryAttack();

	return true;
}

int CBot::GetHealth()
{
	return m_pPI->GetHealth();
}

float CBot::GetHealthPercent()
{
	return ((float)m_pPI->GetHealth() / (float)m_pPI->GetMaxHealth());
}

bool CBot::IsOnLift()
{
	return ((m_vVelocity.z < -8.0f) || (m_vVelocity.z >= 8.0f));
}

edict_t *CBot::GetVisibleSpecial()
{
	return NULL;
}

bool CBot::WantToInvestigateSound()
{
	return ((m_fSpawnTime + 10.0f) < TIME_NOW) && !HasEnemy() && m_bWantToInvestigateSound;
}

bool CBot::RecentlyHurt(float fTime)
{
	return (m_fLastHurtTime > 0) && (m_fLastHurtTime > (TIME_NOW - fTime));
}

void CBot::SpawnInit()
{
	m_fLastHurtTime = 0.0f;
	m_bWantToInvestigateSound = true;
	m_fSpawnTime = TIME_NOW;
	m_bIncreaseSensitivity = false;
	m_fLastSeeEnemyPlayer = 0.0f;
	m_PlayerListeningTo = MyEHandle();
	m_pPrimaryWeapon = NULL;
	m_uSquadDetail.dat = 0;	
	m_iStatsIndex = 0;
	m_ftStatsTime = RandomFloat(0.5f, 1.0f);
	m_ftWantToListenTime = 0.0f;
	m_pLastCoverFrom = MyEHandle();
	m_vAimOffset = Vector(1.0f);
	m_fTotalAimFactor = 0.0f;
	m_fAimMoment = 0.0f;
	m_ftLastUpdateLastSeeEnemy = RandomFloat(0.5f, 1.0f);
	m_fPercentMoved = 1.0f;
	m_fCurrentDanger = 0.0f;
	m_iSpecialVisibleId = 0;
	m_ftUseRouteTime = 0.0f;
	m_bWantToListen = true;
	m_iPrevWeaponSelectFailed = 0;
	m_bOpenFire = true;
	m_ftListenTime = 0.0f;
	m_bListenPositionValid = false;
	m_bPrevAimVectorValid = false;
	m_fLastSeeEnemy = 0.0f;
	m_ftAvoidTime = 0.0f;
	m_vLookAroundOffset = Vector(0.0f);
	m_ftWaypointStuckTime = 0.0f;
	m_pPickup = MyEHandle();
	m_pAvoidEntity = MyEHandle();
	m_bThinkStuck = false;
	m_pLookEdict = MyEHandle();
	m_ftLookAroundTime = 0.0f;
	m_pAvoidEntity = MyEHandle();
	m_bLookedForEnemyLast = false;
	m_iPrevHealth = 0;
	m_vStuckPos = Vector(0.0f);
	m_ftUpdateDamageTime = RandomFloat(0.5f, 1.0f);
	m_iAccumulatedDamage = 0;
	m_ftCheckStuckTime = 4.0f;
	m_fStuckTime = 0.0f;
	m_vLastOrigin = Vector(0.0f);
	m_vVelocity = Vector(0.0f);
	m_ftUpdateOriginTime = RandomFloat(0.5f, 1.0f);
	m_ftNextUpdateAimVector = 0.0f;
	m_vAimVector = Vector(0.0f);
	m_ftLookSetTime = 0.0f;
	m_vHurtOrigin = Vector(0.0f);
	m_pOldEnemy = MyEHandle();
	m_pEnemy = MyEHandle();
	m_vLastSeeEnemy = Vector(0.0f);
	m_pLastEnemy = MyEHandle();
	m_fLastWaypointVisible = 0.0f;
	m_vGoal = Vector(0.0f);
	m_bHasGoal = false;
	m_fLookAtTimeStart = 0;
	m_fLookAtTimeEnd = 0;
	m_fNextThink = 0;
	m_iImpulse = 0;
	m_iButtons = 0;
	m_fForwardSpeed = 0.0f;
	m_fSideSpeed = 0.0f;
	m_fUpSpeed = 0.0f;
	m_iConditions = 0;
	m_ftStrafeTime = 0.0f;
	m_bInitAlive = true;
	m_vMoveTo = Vector(0.0f);
	m_bMoveToIsValid = false;
	m_vLookAt = Vector(0.0f);
	m_bLookAtIsValid = false;
	m_iSelectWeapon = 0;
	m_ftAvoidSideSwitch = 0.0f;
	m_bAvoidRight = (RandomInt(0, 1) == 0);
	m_iLookTask = LOOK_WAYPOINT;
	m_vViewAngles = QAngle(0, 0, 0);

	memset(&m_Stats, 0, sizeof(bot_statistics_t));

	ResetTouchDistance(48.0f);

	for (register short int i = 0; i < BOT_UTIL_MAX; i++)
		m_ftUtilTimes[i] = 0.0f;

	for (register short int i = 0; i < MAX_VOICE_CMDS; i++)
		m_ftLastVoiceCommand[i] = 0.0f;

	if (m_pSchedules != NULL)
		m_pSchedules->FreeMemory();

	if (m_pVisibles != NULL)
		m_pVisibles->Reset();

	if (m_pEdict && (m_iAmmo == NULL))
		m_iAmmo = CClassInterface::GetAmmoList(m_pEdict);
}

void CBot::SetLastEnemy(edict_t *pEnemy)
{
	if (pEnemy == NULL)
	{
		m_fLastSeeEnemy = 0.0f;
		m_pLastEnemy = MyEHandle();
		return;
	}

	m_fLastSeeEnemy = TIME_NOW;
	m_pLastEnemy = pEnemy;
	m_ftLastUpdateLastSeeEnemy.Invalidate();
	m_vLastSeeEnemy = CBotGlobals::EntityOrigin(m_pLastEnemy);
	m_vLastSeeEnemyBlastWaypoint = m_vLastSeeEnemy;

}

bool CBot::SelectBotWeapon(CBotWeapon *pBotWeapon)
{
	int id = pBotWeapon->GetWeaponIndex();

	if (id)
	{
		SelectWeapon(id);
		return true;
	}
	return false;
}

float CBot::GetEnemyFactor(edict_t *pEnemy)
{
	return DistanceFrom(pEnemy);
}

#if defined USE_NAVMESH
void CBot::TouchedWpt(INavMeshArea *pWaypoint, int iNextWaypoint, int iPrevWaypoint)
{
	ResetTouchDistance(48.0f);

	m_ftWaypointStuckTime.Start(RandomFloat(7.5f, 12.5f));

	if(pWaypoint->GetAttributes() & NAV_MESH_CROUCH)
		Duck();
	if(pWaypoint->GetAttributes() & NAV_MESH_RUN)
		UpdateCondition(CONDITION_RUN);

	UpdateDanger(m_pNavigator->GetBelief(pWaypoint->GetID()));
}
#else
void CBot::TouchedWpt(CWaypoint *pWaypoint, int iNextWaypoint, int iPrevWaypoint)
{
	ResetTouchDistance(48.0f);

	m_ftWaypointStuckTime.Start(RandomFloat(7.5f, 12.5f));

	if (pWaypoint->GetFlags() & CWaypointTypes::W_FL_JUMP)
		Jump();
	if (pWaypoint->GetFlags() & CWaypointTypes::W_FL_CROUCH)
		Duck();
	if (pWaypoint->GetFlags() & CWaypointTypes::W_FL_SPRINT)
		UpdateCondition(CONDITION_RUN);

	UpdateDanger(m_pNavigator->GetBelief(CWaypoints::GetWaypointIndex(pWaypoint)));
}
#endif

void CBot::UpdateDanger(float fBelief)
{
	m_fCurrentDanger = (m_fCurrentDanger * m_pProfile->m_fBraveness) + (fBelief * (1.0f - m_pProfile->m_fBraveness));
}
// setup buttons and data structures
void CBot::Setup()
{
	/////////////////////////////////
	m_pButtons = new CBotButtons();
	/////////////////////////////////
	m_pSchedules = new CBotSchedules();
	/////////////////////////////////
#if defined USE_NAVMESH
	m_pNavigator = new CNavMeshNavigator(this);
#else
	m_pNavigator = new CWaypointNavigator(this);
#endif
	/////////////////////////////////
	m_pVisibles = new CBotVisibles(this);
	/////////////////////////////////
	m_pFindEnemyFunc = new CFindEnemyFunc(this);
	/////////////////////////////////
	m_pWeapons = new CBotWeapons(this);
}

/*
* called when a bot dies
*/
void CBot::Died(edict_t *pKiller, const char *pszWeapon)
{
	SpawnInit();

	m_vLastDiedOrigin = GetOrigin();
}

/*
* called when a bot kills something
*/
void CBot::Killed(edict_t *pVictim, char *weapon)
{
	if (m_pLastEnemy == pVictim)
		m_pLastEnemy = MyEHandle();
}

// called when bot shoots a wall or similar object -i.e. not the enemy
void CBot::ShotMiss()
{

}
// shot an enemy (or teammate?)
void CBot::Shot(edict_t *pEnemy)
{

}
// got shot by someone
bool CBot::Hurt(edict_t *pAttacker, int iHealthNow, bool bDontHide)
{
	if (!pAttacker)
		return false;

	if (CClassInterface::GetTeam(pAttacker) == GetTeam())
		FriendlyFire(pAttacker);

	m_vHurtOrigin = CBotGlobals::EntityOrigin(pAttacker);

	if (!HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
	{
		m_ftLookSetTime.Invalidate();
		SetLookAtTask(LOOK_HURT_ORIGIN);
		m_ftLookSetTime.Start(RandomFloat(3.0, 8.0));
	}

	if (!m_ftUpdateDamageTime.IsElapsed())
	{
		m_ftUpdateDamageTime.Reset();
		m_fCurrentDanger += (((float)m_iAccumulatedDamage) / m_pPI->GetMaxHealth())*MAX_BELIEF;
		m_iAccumulatedDamage = 0;
	}

	m_fLastHurtTime = TIME_NOW;
	m_iAccumulatedDamage += (m_iPrevHealth - iHealthNow);
	m_iPrevHealth = iHealthNow;

	// TO DO: replace with perceptron method
	if (m_iAccumulatedDamage > (m_pPI->GetMaxHealth()*m_pProfile->m_fBraveness))
	{
		if (!bDontHide)
		{
			m_pSchedules->RemoveSchedule(SCHED_GOOD_HIDE_SPOT);
			m_pSchedules->AddFront(new CGotoHideSpotSched(this, m_vHurtOrigin));
		}

		m_iAccumulatedDamage = 0;
		m_ftUpdateDamageTime.Invalidate();

		return true;
	}

	return false;
}

void CBot::CheckEntity(edict_t **pEdict)
{
	if (pEdict && *pEdict && !CBotGlobals::EntityIsValid(*pEdict))
		*pEdict = NULL;
}

void CBot::CheckDependantEntities()
{
	return;
}

void CBot::FindEnemy(edict_t *pOldEnemy)
{
	m_pFindEnemyFunc->Init();

	if (pOldEnemy && IsEnemy(pOldEnemy, true))
		m_pFindEnemyFunc->SetOldEnemy(pOldEnemy);

	m_pVisibles->EachVisible(m_pFindEnemyFunc);

	m_pEnemy = m_pFindEnemyFunc->GetBestEnemy();

	if (m_pEnemy && (m_pEnemy != pOldEnemy))
	{
		EnemyFound(m_pEnemy);
	}
}

bool CBot::IsAlive()
{
	return !m_pPI->IsDead();
}

int CBot::GetTeam()
{
	return m_pPI->GetTeamIndex();
}

bool CBot::IsFacing(Vector vOrigin)
{
	return (DotProductFromOrigin(vOrigin) > DOT_15DEGREE);
}

void CBot::DebugBot()
{
#ifndef __linux__
	float currentY = 0.06;
	CFmtStr msg;
	extern ConVar bot_debug_show_route;
	extern ConVar bot_debug_show_waypoints;

	NDebugOverlay::ScreenText(0.2, currentY, msg.sprintf("Current Util: %s", (m_CurrentUtil >= 0) ? g_szUtils[m_CurrentUtil] : "none"), 80, 80, 200, 255, 0.4);
	currentY += 0.02;

	bool bHasSchedule = m_pSchedules->GetCurrentSchedule() != NULL;
	NDebugOverlay::ScreenText(0.2, currentY, msg.sprintf("Current Schedule: %s", bHasSchedule ? m_pSchedules->GetCurrentSchedule()->GetIDString() : "none"), 80, 80, 200, 255, 0.4);
	currentY += 0.02;

	bool bHastask = m_pSchedules->GetCurrentTask() != NULL;
	NDebugOverlay::ScreenText(0.2, currentY, msg.sprintf("Current Task: %s", bHastask ? m_pSchedules->GetCurrentTask()->DebugString() : "none"), 80, 80, 200, 255, 0.4);
	currentY += 0.02;

	NDebugOverlay::ScreenText(0.2, currentY, msg.sprintf("Look Task: %s", (m_iLookTask >= 0) ? g_szLookTaskToString[m_iLookTask] : "none"), 80, 80, 200, 255, 0.4);
	currentY += 0.02;

	NDebugOverlay::ScreenText(0.2, currentY, msg.sprintf("Current Waypoint: %d", m_pNavigator->HasNextPoint() ? m_pNavigator->GetCurrentWaypointID() : -1), 80, 80, 200, 255, 0.4);
	currentY += 0.02;

	NDebugOverlay::ScreenText(0.2, currentY, msg.sprintf("Current Goal: %d", m_pNavigator->HasNextPoint() ? m_pNavigator->GetCurrentGoalID() : -1), 80, 80, 200, 255, 0.4);
	currentY += 0.02;

	NDebugOverlay::ScreenText(0.2, currentY, msg.sprintf("Danger: %.2f pct", (m_fCurrentDanger / MAX_BELIEF) * 100), 255, 90, 70, 255, 0.4);
	currentY += 0.02;

	edict_t *pEnemy = m_pEnemy.Get();
	IPlayerInfo *pPI = NULL;
	if(CBotGlobals::IsPlayer(pEnemy))
		pPI = playerinfomanager->GetPlayerInfo(pEnemy);

	NDebugOverlay::ScreenText(0.2, currentY, msg.sprintf("Enemy: %s (name: '%s')", (pEnemy != NULL) ? pEnemy->GetClassName() : "none", (pPI != NULL) ? pPI->GetName() : "none"), 255, 90, 70, 255, 0.4);
	currentY += 0.02;

	NDebugOverlay::ScreenText(0.2, currentY, "---CONDITIONS---", 80, 80, 200, 255, 0.4);
	currentY += 0.02;

	for (int i = 0; i <= CONDITION_MAX_BITS; ++i)
	{
		if (m_iConditions & (1<<i))
		{
			NDebugOverlay::ScreenText(0.2, currentY, g_szConditionsDebugStrings[i], 160, 65, 160, 255, 0.4);
			currentY += 0.02;
		}
	}

	if(bot_debug_show_route.GetBool())
		GetNavigator()->DrawPath();

	if(bot_debug_show_waypoints.GetBool())
		CWaypoints::DrawWaypoints(m_pEdict);
#endif
}

int CBot::NearbyFriendlies(float fDistance)
{
	int8_t num = 0;
	edict_t *pEdict;

	for (short int i = 0; i <= MAX_PLAYERS; i++)
	{
		pEdict = INDEXENT(i);

		if (!CBotGlobals::EntityIsValid(pEdict))
			continue;

		if (DistanceFrom(pEdict) > fDistance)
			continue;

		if (!IsVisible(pEdict))
			continue;

		if (IsEnemy(pEdict))
			continue;

		num++;
	}

	return num;
}

void CBot::FreeMapMemory()
{
	// we can save things here
	// 
	/////////////////////////////////
	if (m_pButtons != NULL)
	{
		m_pButtons->FreeMemory();
		delete m_pButtons;
		m_pButtons = NULL;
	}
	/////////////////////////////////
	if (m_pSchedules != NULL)
	{
		m_pSchedules->FreeMemory();
		delete m_pSchedules;
		m_pSchedules = NULL;
	}
	/////////////////////////////////
	if (m_pNavigator != NULL)
	{
		m_pNavigator->BeliefSave(true);
		m_pNavigator->FreeMapMemory();
		delete m_pNavigator;
		m_pNavigator = NULL;
	}
	/////////////////////////////////
	if (m_pVisibles != NULL)
	{
		m_pVisibles->Reset();
		delete m_pVisibles;
		m_pVisibles = NULL;
	}
	/////////////////////////////////
	if (m_pFindEnemyFunc != NULL)
	{
		delete m_pFindEnemyFunc;
		m_pFindEnemyFunc = NULL;
	}
	/////////////////////////////////
	if (m_pWeapons != NULL)
	{
		delete m_pWeapons;
		m_pWeapons = NULL;
	}

	m_iAmmo = NULL;
	/////////////////////////////////
	Init();
}

void CBot::UpdateStatistics()
{
	bool bVisible;
	bool bIsEnemy;
	edict_t *pPlayer;
	IPlayerInfo *pPI;

	memset(&m_Stats, 0, sizeof(bot_statistics_t));

	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		pPlayer = INDEXENT(i);

		if (!pPlayer || !pPlayer->IsFree())
			return; // not valid

		if (pPlayer == m_pEdict)
			return; // don't listen to self

		pPI = playerinfomanager->GetPlayerInfo(pPlayer);

		if (!pPI || !pPI->IsConnected() || pPI->IsDead() || pPI->IsObserver())
			return;

		bVisible = IsVisible(pPlayer);
		bIsEnemy = IsEnemy(pPlayer, false);

		if (bIsEnemy)
		{
			if (bVisible)
				m_Stats.m_iEnemiesVisible++;

			if (DistanceFrom(pPlayer) < bot_stats_inrange_dist.GetFloat())
				m_Stats.m_iEnemiesInRange++;
		}
		else
		{
			// team-mate
			if (bVisible)
				m_Stats.m_iTeamMatesVisible++;

			if (DistanceFrom(pPlayer) < bot_stats_inrange_dist.GetFloat())
				m_Stats.m_iTeamMatesInRange++;
		}
	}
}

bool CBot::WantToListen()
{
	return (m_bWantToListen && m_ftWantToListenTime.IsElapsed() && ((m_fLastSeeEnemy + 2.5f) < TIME_NOW));
}

// Listen for players who are shooting
void CBot::ListenForPlayers()
{
	//m_fNextListenTime = TIME_NOW + RandomFloat(0.5f,2.0f);

	edict_t *pListenNearest = NULL;
	edict_t *pPlayer;
	IGamePlayer *pPl;
	IPlayerInfo *pPI;
	unsigned int buttons;
	float fFactor = 0;
	float fMaxFactor = 0;
	//float fMinDist = 1024.0f;
	float fDist;
	float fVelocity;
	Vector vVelocity;
	bool bIsNearestAttacking = false;

	if (m_bListenPositionValid && !m_ftListenTime.IsElapsed()) // already listening to something ?
	{
		SetLookAtTask(LOOK_NOISE);
		return;
	}

	m_bListenPositionValid = false;

	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		pPlayer = INDEXENT(i);
		if (pPlayer->IsFree())
			continue;

		if (pPlayer == m_pEdict)
			continue; // don't listen to self
		
		pPl = playerhelpers->GetGamePlayer(pPlayer);
		if (!pPl || !pPl->IsInGame())
			continue;

		pPI = pPl->GetPlayerInfo();
		if (!pPI || pPI->IsDead() || pPI->IsObserver())
			continue;

		if (!IsEnemy(pPlayer))
			continue; // ignore friendlies

		fDist = DistanceFrom(pPlayer);

		if (fDist > bot_listen_dist.GetFloat())
			continue;

		fFactor = 0.0f;

		buttons = pPI->GetLastUserCommand().buttons;

		if (buttons & IN_ATTACK)
		{
			if (WantToListenToPlayerAttack(pPlayer))
				fFactor += 1000.0f;
		}

		// can't see this player and I'm on my own
		if (WantToListenToPlayerFootsteps(pPlayer) && !IsVisible(pPlayer) && (m_Stats.m_iTeamMatesVisible == 0))
		{
			CClassInterface::GetVelocity(pPlayer, &vVelocity);
			fVelocity = vVelocity.Length();

			if (fVelocity > bot_footstep_speed.GetFloat())
				fFactor += fVelocity;
		}

		if (fFactor == 0.0f)
			continue;

		// add inverted distance to the factor (i.e. closer = better)
		fFactor += (bot_listen_dist.GetFloat() - fDist);

		if (fFactor > fMaxFactor)
		{
			fMaxFactor = fFactor;
			pListenNearest = pPlayer;
			bIsNearestAttacking = (buttons & IN_ATTACK) ? true : false;
		}
	}

	if (pListenNearest != NULL)
	{
		ListenToPlayer(pListenNearest, false, bIsNearestAttacking);
	}
}

void CBot::HearPlayerAttack(edict_t *pAttacker, int iWeaponID)
{
	if (!m_ftListenTime.IsElapsed()) // already listening to something ?
		return;
	
	ListenToPlayer(pAttacker, false, true);
}

void CBot::ListenToPlayer(edict_t *pPlayer, bool bIsEnemy, bool bIsAttacking)
{
	bool bIsVisible = IsVisible(pPlayer);

	if (CBotGlobals::IsPlayer(pPlayer))
	{
		IGamePlayer *pPl = playerhelpers->GetGamePlayer(pPlayer);
		if (!pPl || !pPl->IsInGame())
			return;

		IPlayerInfo *pPI = pPl->GetPlayerInfo();
		if (!pPI || pPI->IsDead())
			return;

		if (bIsEnemy == false)
			bIsEnemy = IsEnemy(pPlayer);

		// look at possible enemy
		if (!bIsVisible || bIsEnemy)
		{
			m_vListenPosition = pPI->GetAbsOrigin();
		}
		else if (bIsAttacking)
		{
			if (!bIsEnemy && WantToInvestigateSound())
			{
				QAngle angle = pPI->GetAbsAngles();
				Vector forward;

				AngleVectors(angle, &forward);

				// look where team mate is shooting
				m_vListenPosition = pPI->GetAbsOrigin() + (forward*1024.0f);

				// not investigating any noise right now -- depending on my braveness I will check it out
				if (!m_pSchedules->IsCurrentSchedule(SCHED_INVESTIGATE_NOISE) && (RandomFloat(0.0f, 0.75f) < m_pProfile->m_fBraveness))
				{
					Vector vAttackerOrigin = CBotGlobals::EntityOrigin(pPlayer);
					if (DistanceFrom(vAttackerOrigin) > 96.0f)
					{
						// check exactly where teammate is firing
						CBotGlobals::QuickTraceline(pPlayer, vAttackerOrigin, m_vListenPosition);
						trace_t *pTrace = CBotGlobals::GetTraceResult();

						// update the wall or player teammate is shooting
						m_vListenPosition = pTrace->endpos;

						CBotGlobals::QuickTraceline(m_pEdict, GetOrigin(), m_vListenPosition);
						pTrace = CBotGlobals::GetTraceResult();

						// can't see what my teammate is shooting -- go there
						if ((pTrace->fraction < 1.0f) && (pTrace->m_pEnt != m_pEdict->GetIServerEntity()->GetBaseEntity()))
						{
							m_pSchedules->RemoveSchedule(SCHED_INVESTIGATE_NOISE);

							Vector vecLOS;
							float flDot;

							vecLOS = GetOrigin() - vAttackerOrigin;
							vecLOS = vecLOS / vecLOS.Length();

							flDot = DotProduct(vecLOS, forward);

							if (flDot > 0.5f)
							{
								// Facing my direction
								m_pSchedules->AddFront(new CBotInvestigateNoiseSched(m_vListenPosition, m_vListenPosition));
							}
							else
							{
								// Not facing my direction
								m_pSchedules->AddFront(new CBotInvestigateNoiseSched(vAttackerOrigin, m_vListenPosition));
							}
						}
					}
				}
			}
		}
	}
	else
		m_vListenPosition = CBotGlobals::EntityOrigin(pPlayer);

	m_PlayerListeningTo = pPlayer;
	m_bListenPositionValid = true;
	m_ftListenTime = RandomFloat(1.0f, 2.0f);
	SetLookAtTask(LOOK_NOISE);
	m_ftLookSetTime.Start(2.0f);

	if (bIsVisible || !bIsEnemy)
	{// certain where noise is coming from -- don't listen elsewhere for another second
		m_ftWantToListenTime.Start(1.0f);
	}
	else
		m_ftWantToListenTime.Start(0.25f);

}

bool CBot::OnLadder()
{
	return CClassInterface::IsMoveType(m_pEdict, MOVETYPE_LADDER);
}

void CBot::FreeAllMemory()
{
	FreeMapMemory();
	return;
}

void CBot::ForceGotoWaypoint(int wpt)
{
	if (wpt != -1)
	{
		m_pSchedules->FreeMemory();
		m_pSchedules->Add(new CBotSchedule(new CFindPathTask(wpt)));
	}
}
// found a new enemy
void CBot::EnemyFound(edict_t *pEnemy)
{
	m_bLookedForEnemyLast = false;
}
// work move velocity
void CBot::DoMove()
{
	// Temporary measure to make bot follow me when i make listen serevr
	//setMoveTo(CBotGlobals::entityOrigin(INDEXENT(1)));

	// moving somewhere?
	if (MoveToIsValid())
	{
		Vector2D move;
		float flMove = 0.0;
		float flSide = 0.0;
		// fAngle is got from world realting to bots origin, not angles
		float fAngle;
		float radians;
		float fDist;

		if (m_pAvoidEntity && m_ftAvoidTime.IsElapsed())
		{
			if (CanAvoid(m_pAvoidEntity))
			{
				Vector m_vAvoidOrigin = CBotGlobals::EntityOrigin(m_pAvoidEntity);

				//m_vMoveTo = getOrigin() + ((m_vMoveTo-getOrigin())-((m_vAvoidOrigin-getOrigin())*bot_avoid_strength.GetFloat()));
				//float fAvoidDist = distanceFrom(m_pAvoidEntity);

				Vector vMove = m_vMoveTo - GetOrigin();
				Vector vLeft;

				if (vMove.Length2D() > bot_avoid_strength.GetFloat())
				{
					vLeft = vMove.Cross(Vector(0, 0, 1));
					vLeft = (vLeft / vLeft.Length());

					if (m_ftAvoidSideSwitch.IsElapsed())
					{
						m_ftAvoidSideSwitch = RandomFloat(2.0f, 3.0f);
						m_bAvoidRight = !m_bAvoidRight;
					}

					if (m_bAvoidRight)
						m_vMoveTo = GetOrigin() + ((vMove / vMove.Length())*bot_avoid_strength.GetFloat()) + (vLeft*bot_avoid_strength.GetFloat());
					else
						m_vMoveTo = GetOrigin() + ((vMove / vMove.Length())*bot_avoid_strength.GetFloat()) - (vLeft*bot_avoid_strength.GetFloat());
				}


			}
			else
				m_pAvoidEntity = MyEHandle();
		}

		fAngle = CBotGlobals::YawAngleFromEdict(m_pEdict, m_vMoveTo);
		fDist = (GetOrigin() - m_vMoveTo).Length2D();

		radians = DEG_TO_RAD(fAngle);
		//radians = fAngle * 3.141592f / 180.0f; // degrees to radians
		// fl Move is percentage (0 to 1) of forward speed,
		// flSide is percentage (0 to 1) of side speed.

		// quicker
		SinCos(radians, &move.y, &move.x);

		move = move / move.Length();

		flMove = move.x;
		flSide = move.y;

		m_fForwardSpeed = m_fIdealMoveSpeed * flMove;

		// dont want this to override strafe speed if we're trying 
		// to strafe to avoid a wall for instance.
		if (m_ftStrafeTime.IsElapsed())
		{
			// side speed 
			m_fSideSpeed = m_fIdealMoveSpeed * flSide;
		}

		if (HasSomeConditions(CONDITION_LIFT))//fabs(m_vMoveTo.z - getOrigin().z) > 48 )
		{
			if (fabs(m_vVelocity.z) > 16.0f)
			{
				m_fForwardSpeed = 0;
				m_fSideSpeed = 0;
			}
		}

		// moving less than 1.0 units/sec? just stop to 
		// save bot jerking around..
		if (fabs(m_fForwardSpeed) < 1.0)
			m_fForwardSpeed = 0.0;
		if (fabs(m_fSideSpeed) < 1.0)
			m_fSideSpeed = 0.0;

		if ((m_fForwardSpeed < 1.0f) && (m_fSideSpeed < 1.0f))
		{
			if (m_pButtons->HoldingButton(IN_SPEED) && m_pButtons->HoldingButton(IN_FORWARD))
			{
				m_pButtons->LetGo(IN_SPEED);
				m_pButtons->LetGo(IN_FORWARD);
			}
		}

		if ((!OnLadder() && !m_pNavigator->NextPointIsOnLadder()) && (fDist < 8.0f))
		{
			m_fForwardSpeed = 0.0f;
			m_fSideSpeed = 0.0f;
		}

		if (IsUnderWater() || OnLadder())
		{
			if (m_vMoveTo.z > (GetOrigin().z + 32.0))
				m_fUpSpeed = m_fIdealMoveSpeed;
			else if (m_vMoveTo.z < (GetOrigin().z - 32.0))
				m_fUpSpeed = -m_fIdealMoveSpeed;
		}
	}
	else
	{
		m_fForwardSpeed = 0;
		// bots side move speed
		m_fSideSpeed = 0;
		// bots upward move speed (e.g in water)
		m_fUpSpeed = 0;
	}
}

bool CBot::RecentlySpawned(float fTime)
{
	return ((m_fSpawnTime + fTime) > TIME_NOW);
}

SH_DECL_MANUALHOOK1(FInViewCone, 0, 0, 0, bool, const Vector *)
bool CBot::FInViewCone(edict_t *pDest)
{
	Vector origin = CBotGlobals::EntityOrigin(pDest);

	static bool bSetup = false;
	static bool bWarned = false;
	if (!bSetup)
	{
		int iOffset;
		if (!g_pGameConf->GetOffset("FInViewCone", &iOffset))
		{
			if (!bWarned)
			{
				smutils->LogError(myself, "Couldn't get FInViewCone offset, using fallback");
				bWarned = true;
			}
			return (((origin - GetEyePosition()).Length() > 1) && (DotProductFromOrigin(origin) > 0));
		}

		SH_MANUALHOOK_RECONFIGURE(FInViewCone, iOffset, 0, 0);
		bSetup = true;
	}

	CBaseEntity *pEntity = NULL;
	if (!(pEntity = m_pEdict->GetNetworkable()->GetBaseEntity()))
		return false;

	return SH_MCALL(pEntity, FInViewCone)(&origin);
}

SH_DECL_MANUALHOOK3(FVisible, 0, 0, 0, bool, const Vector *, int, CBaseEntity **)
bool CBot::FVisible(Vector vOrigin, edict_t *pDest)
{
	static bool bSetup = false;
	static bool bWarned = false;
	if (!bSetup)
	{
		int iOffset;
		if (!g_pGameConf->GetOffset("FVisible", &iOffset))
		{
			if (!bWarned)
			{
				smutils->LogError(myself, "Couldn't get FVisible offset, using fallback");
				bWarned = true;
			}
			return CBotGlobals::IsVisibleHitAllExceptPlayer(m_pEdict, GetEyePosition(), vOrigin, pDest);
		}

		SH_MANUALHOOK_RECONFIGURE(FVisible, iOffset, 0, 0);
		bSetup = true;
	}

	CBaseEntity *pEntity = NULL;
	if (!(pEntity = m_pEdict->GetNetworkable()->GetBaseEntity()))
		return false;

	CBaseEntity *pBlocker = NULL;

	bool ret = SH_MCALL(pEntity, FVisible)(&vOrigin, MASK_BLOCKLOS_AND_NPCS, &pBlocker);
	if (!ret && pBlocker)
	{
		if (gameents->BaseEntityToEdict(pBlocker) == pDest)
			return true;
	}

	return ret;
	//return CBotGlobals::IsVisibleHitAllExceptPlayer(m_pEdict, GetEyePosition(), vOrigin, pDest);
}

bool CBot::FVisible(edict_t *pEdict, bool bCheckHead)
{
	static Vector eye;

	// use special hit traceline for players so bots dont shoot through things 
	// For players -- do two tracelines -- one at the origin and one at the head (for headshots)
	if (bCheckHead || (pEdict == m_pEnemy) || CBotGlobals::IsPlayer(pEdict))
	{
		Vector vOrigin;
		Vector vHead;

		// use this method to get origin -- quicker 
		vOrigin = pEdict->GetCollideable()->GetCollisionOrigin();
		vHead = vOrigin + Vector(0.0f, 0.0f, pEdict->GetCollideable()->OBBMaxs().z);

		if (FVisible(vHead, pEdict))
		{
			if (m_pEnemy == pEdict)
			{
				UpdateCondition(CONDITION_SEE_ENEMY_HEAD);

				if (FVisible(vOrigin, pEdict))
					UpdateCondition(CONDITION_SEE_ENEMY_GROUND);
				else
					RemoveCondition(CONDITION_SEE_ENEMY_GROUND);
			}

			return true;
		}

		if (m_pEnemy == pEdict)
		{
			if (FVisible(vOrigin, pEdict))
			{
				UpdateCondition(CONDITION_SEE_ENEMY_GROUND);
				return true;
			}
			else
			{
				RemoveCondition(CONDITION_SEE_ENEMY_GROUND);
				return false;
			}
		}

		return FVisible(vOrigin, pEdict);
	}

	eye = GetEyePosition();

	// use typical traceline for non players
	return CBotGlobals::IsVisible(m_pEdict, eye, pEdict);
}

SH_DECL_MANUALHOOK0(EyeAngles, 0, 0, 0, const QAngle *)
QAngle CBot::GetEyeAngles()
{
	static bool bSetup = false;
	static bool bWarned = false;
	if (!bSetup)
	{
		int iOffset;
		if (!g_pGameConf->GetOffset("EyeAngles", &iOffset))
		{
			if (!bWarned)
			{
				smutils->LogError(myself, "Couldn't get EyeAngles offset, using fallback");
				bWarned = true;
			}

			return CBotGlobals::PlayerAngles(m_pEdict);
		}

		SH_MANUALHOOK_RECONFIGURE(EyeAngles, iOffset, 0, 0);
		bSetup = true;
	}

	CBaseEntity *pEntity = NULL;
	if ((pEntity = m_pEdict->GetNetworkable()->GetBaseEntity()))
	{
		QAngle ret; ret.Invalidate();
		ret = *SH_MCALL(pEntity, EyeAngles)();
		if (ret.Base() && ret.IsValid())
			return ret;
	}

	return CBotGlobals::PlayerAngles(m_pEdict);
}

SH_DECL_MANUALHOOK0(EyePosition, 0, 0, 0, Vector)
Vector CBot::GetEyePosition()
{
	static bool bSetup = false;
	static bool bWarned = false;
	if (!bSetup)
	{
		int iOffset;
		if (!g_pGameConf->GetOffset("EyePosition", &iOffset))
		{
			if (!bWarned)
			{
				smutils->LogError(myself, "Couldn't get EyePosition offset, using fallback");
				bWarned = true;
			}
			
			Vector vOrigin;
			gameclients->ClientEarPosition(m_pEdict, &vOrigin);
			return vOrigin;
		}

		SH_MANUALHOOK_RECONFIGURE(EyePosition, iOffset, 0, 0);
		bSetup = true;
	}

	CBaseEntity *pEntity = NULL;
	if ((pEntity = m_pEdict->GetNetworkable()->GetBaseEntity()))
	{
		Vector ret; ret.Invalidate();
		ret = SH_MCALL(pEntity, EyePosition)();
		if (ret.Base() && ret.IsValid())
			return ret;
	}

	Vector vOrigin;
	gameclients->ClientEarPosition(m_pEdict, &vOrigin);
	return vOrigin;
}

float CBot::DotProductFromOrigin(const Vector vOrigin)
{
	static Vector vecLOS;
	static float flDot;

	Vector vForward;
	QAngle eyes;

	eyes = GetEyeAngles();

	// in fov? Check angle to edict
	AngleVectors(eyes, &vForward);

	vecLOS = vOrigin - GetEyePosition();
	vecLOS = vecLOS / vecLOS.Length();

	flDot = DotProduct(vecLOS, vForward);

	return flDot;
}

void CBot::UpdateUtilTime(int util)
{
	m_ftUtilTimes[util] = RandomFloat(0.75f, 1.5f);
}

Vector *CBot::GetAimVector(edict_t *pEntity)
{
	static Vector v_desired_offset;
	static Vector v_origin;
	static float fSensitivity;
	static Vector v_size;
	static float fDist;
	static float fDist2D;

	if (!m_ftNextUpdateAimVector.IsElapsed())
	{
		return &m_vAimVector;
	}

	fDist = DistanceFrom(pEntity);
	fDist2D = DistanceFrom2D(pEntity);

	v_size = pEntity->GetCollideable()->OBBMaxs() - pEntity->GetCollideable()->OBBMins();
	v_size = v_size * 0.5f;

	fSensitivity = (float)m_pProfile->m_iSensitivity / 20;

	v_origin = CBotGlobals::EntityOrigin(pEntity);

	ModAim(pEntity, v_origin, &v_desired_offset, v_size, fDist, fDist2D);

	// post aim
	// update 
	if (bot_supermode.GetBool())
	{
		m_vAimOffset = v_desired_offset;
	}
	else
	{
		m_vAimOffset.x = ((1.0f - fSensitivity) * m_vAimOffset.x) + fSensitivity * v_desired_offset.x;
		m_vAimOffset.y = ((1.0f - fSensitivity) * m_vAimOffset.y) + fSensitivity * v_desired_offset.y;
		m_vAimOffset.z = ((1.0f - fSensitivity) * m_vAimOffset.z) + fSensitivity * v_desired_offset.z;

		// check for QNAN
		if ((m_vAimOffset.x != m_vAimOffset.x) ||
			(m_vAimOffset.y != m_vAimOffset.y) ||
			(m_vAimOffset.z != m_vAimOffset.z))
		{
			m_vAimOffset = Vector(1.0f, 1.0f, 1.0f);
		}
	}

	if (pEntity == CClassInterface::GetGroundEntity(m_pEdict))
		m_vAimOffset.z -= 32.0f;

	m_vAimVector = v_origin + m_vAimOffset;

	m_ftNextUpdateAimVector = (1.0f - m_pProfile->m_fAimSkill)*0.2f;

	return &m_vAimVector;
}

void CBot::ModAim(edict_t *pEntity, Vector &v_origin, Vector *v_desired_offset, Vector &v_size, float fDist, float fDist2D)
{
	static Vector vel;
	static Vector myvel;
	static Vector enemyvel;
	static float fDistFactor;
	static float fHeadOffset;

	int iPlayerFlags = CClassInterface::GetPlayerFlags(pEntity);

	fHeadOffset = 0;

	if (bot_supermode.GetBool())
	{
		v_desired_offset->x = 0;
		v_desired_offset->y = 0;
		v_desired_offset->z = v_size.z - 1;

		return;
	}

	CBotWeapon *pWp = GetCurrentWeapon();

	float fVelFactor = 0.003125f;

	if (pWp && pWp->IsMelee())
	{
		fDistFactor = 0;
		fVelFactor = 0;
	}
	else
	{
		if (fDist < 160)
			fVelFactor = 0.001f;

		fDistFactor = (1.0f - m_pProfile->m_fAimSkill) + (fDist*0.000125f)*(m_fFov / 90.0f);
	}
	// origin is always the bottom part of the entity
	// add body height
	fHeadOffset += v_size.z - 1;

	if (ENTINDEX(pEntity) <= MAX_PLAYERS) // add body height
	{
		// aim for head
		if (!(iPlayerFlags & FL_DUCKING) && HasSomeConditions(CONDITION_SEE_ENEMY_HEAD) && (m_fFov < BOT_DEFAULT_FOV))
			fHeadOffset += v_size.z - 1;
	}

	myvel = Vector(0, 0, 0);
	enemyvel = Vector(0, 0, 0);

	// change in velocity
	if (CClassInterface::GetVelocity(pEntity, &enemyvel) && CClassInterface::GetVelocity(m_pEdict, &myvel))
	{
		vel = (enemyvel - myvel); // relative velocity

		vel = vel * fVelFactor;
	}
	else
	{
		vel = Vector(0.5f, 0.5f, 0.5f);
	}

	// velocity
	v_desired_offset->x = RandomFloat(-vel.x, vel.x)*fDistFactor*v_size.x;
	v_desired_offset->y = RandomFloat(-vel.y, vel.y)*fDistFactor*v_size.y;
	v_desired_offset->z = RandomFloat(-vel.z, vel.z)*fDistFactor*v_size.z;

	// target
	v_desired_offset->z += (fHeadOffset * m_pProfile->m_fAimSkill) + (RandomFloat(0.0, 1.0f - m_pProfile->m_fAimSkill)*fHeadOffset);
}

void CBot::GrenadeThrown()
{

}

void CBot::CheckCanPickup(edict_t *pPickup)
{

}

Vector CBot::Snipe(Vector &vAiming)
{
	if (m_ftLookAroundTime.IsElapsed())
	{
		float fTime;
		Vector vOrigin = GetOrigin();

		m_vLookAroundOffset = Vector(RandomFloat(-512.0f, 512.0f), RandomFloat(-512.0f, 512.0f), RandomFloat(-64.0f, 64.0f));

		if (!m_ftLookAroundTime.HasStarted())
			fTime = 0.1f;
		else
			fTime = RandomFloat(3.0f, 7.0f);

		m_ftLookAroundTime.Start(fTime);
	}

	return vAiming + m_vLookAroundOffset;
}

void CBot::GetLookAtVector()
{
	static bool bDebug = false;
	static Vector vLook(0.0f);

	switch (m_iLookTask)
	{
	case LOOK_NONE:
		StopLooking();
		break;
	case LOOK_VECTOR:
		SetLookAt(m_vLookVector);
		break;
	case LOOK_EDICT:
		if (m_pLookEdict)
			SetLookAt(GetAimVector(m_pLookEdict));
		break;
	case LOOK_GROUND:
		SetLookAt(m_vMoveTo);
		break;
	case LOOK_ENEMY:
		if (m_pEnemy)
			SetLookAt(GetAimVector(m_pEnemy));
		else if (m_pLastEnemy)
			SetLookAt(m_vLastSeeEnemy);
		break;
	case LOOK_LAST_ENEMY:
		if (m_pLastEnemy)
			SetLookAt(m_vLastSeeEnemy);
		break;
	case LOOK_WAYPOINT_NEXT_ONLY:
		SetLookAt(m_vMoveTo);
		break;
	case LOOK_WAYPOINT:
		if (m_pNavigator->NextPointIsOnLadder())
		{
			QAngle angle;
			Vector vforward;

			vLook = m_pNavigator->GetNextPoint();

			angle = QAngle(0, m_pNavigator->GetNextYaw(), 0);

			AngleVectors(angle, &vforward);

			vforward = (vforward / vforward.Length()) * 64;

			vforward.z = 64.0f;

			SetLookAt(vLook + vforward);
		}
		else if (m_pNavigator->HasNextPoint() && m_pButtons->HoldingButton(IN_SPEED))
		{
			if (m_pNavigator->GetNextRoutePoint(&vLook))
			{
				SetLookAt(vLook);
			}
			else 
			{
				vLook = m_pNavigator->GetPreviousPoint();
				SetLookAt(vLook);
			}
		}
		else if (m_pLastEnemy.IsValid() && ((m_fLastSeeEnemy + 5.0f) > TIME_NOW))
			SetLookAt(m_vLastSeeEnemy);
		else if ((m_fCurrentDanger >= 20.0f) && m_pNavigator->GetDangerPoint(&vLook))
			SetLookAt(vLook);
		else if (m_pNavigator->GetNextRoutePoint(&vLook))
			SetLookAt(vLook);
		else
		{
			vLook = m_pNavigator->GetPreviousPoint();
			SetLookAt(vLook);
		}
		break;
	case LOOK_WAYPOINT_AIM:
		SetLookAt(m_vWaypointAim);
		break;
	case LOOK_BUILD:
		if (m_ftLookAroundTime.IsElapsed())
		{
			m_ftLookAroundTime.Start(RandomFloat(2.0f, 4.0f));

			m_vLookAroundOffset = Vector(RandomFloat(-64.0f, 64.0f), RandomFloat(-64.0f, 64.0f), RandomFloat(-32.0f, 32.0f));
		}

		SetLookAt(m_vWaypointAim + m_vLookAroundOffset);
		break;
	case LOOK_SNIPE:
		SetLookAt(Snipe(m_vWaypointAim));
		break;
	case LOOK_NOISE:
		if (m_pEnemy && HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
		{
			SetLookAtTask((LOOK_ENEMY));
			return;
		}
		else if (!m_bListenPositionValid || m_ftListenTime.IsElapsed()) // already listening to something ?
		{
			SetLookAtTask((LOOK_WAYPOINT));
			return;
		}

		SetLookAt(m_vListenPosition);
		break;
	case LOOK_AROUND:
		if (m_ftLookAroundTime.IsElapsed())
		{
			if ((m_fCurrentDanger < 10.0f) || ((m_pNavigator->NumPaths() == 0) || !m_pNavigator->RandomDangerPath(&m_vLookAroundOffset)))
			{
				// random
				m_vLookAroundOffset = GetEyePosition();
			}

			m_ftLookAroundTime.Start(RandomFloat(2.0f, 3.0f));
			m_vLookAroundOffset = m_vLookAroundOffset + Vector(RandomFloat(-1024.0f, 1024.0f), RandomFloat(-1024.0f, 1024.0f), RandomFloat(-128.0f, 128.0f));
		}

		SetLookAt(m_vLookAroundOffset);
		break;
	case LOOK_HURT_ORIGIN:
		SetLookAt(m_vHurtOrigin);
		break;
	default:
		break;
	}
}

int CBot::GetPlayerID()
{
	return m_pPI->GetUserID();
}

void CBot::ChangeAngles(float fSpeed, float *fIdeal, float *fCurrent, float *fUpdate)
{
	float current = *fCurrent;
	float ideal = *fIdeal;
	float diff;
	float delta;
	float alpha;
	float alphaspeed;

	if (bot_anglespeed.GetFloat() < 0.01f)
		bot_anglespeed.SetValue(0.16f);

	alphaspeed = (fSpeed / 20);

	alpha = alphaspeed * bot_anglespeed.GetFloat();

	diff = ideal - current;

	if (diff < -180.0f)
		diff += 360.0f;
	else if (diff > 180.0f)
		diff -= 360.0f;

	delta = (diff*alpha) + (m_fAimMoment*alphaspeed);

	//check for QNAN
	if (delta != delta)
		delta = 1.0f;

	m_fAimMoment = (m_fAimMoment * alphaspeed) + (delta * (1.0f - alphaspeed));

	//check for QNAN
	if (m_fAimMoment != m_fAimMoment)
		m_fAimMoment = 1.0f;

	current = current + delta;

	if (current > 180.0f)
		current -= 360.0f;
	else if (current < -180.0f)
		current += 360.0f;

	*fCurrent = current;

	if (*fCurrent > 180.0f)
		*fCurrent -= 360.0f;
	else if (*fCurrent < -180.0f)
		*fCurrent += 360.0f;
}

bool CBot::Select_CWeapon(CWeapon *pWeapon)
{
	CBotWeapon *pSelect = m_pWeapons->GetWeapon(pWeapon);

	if (pSelect)
	{
		int id = pSelect->GetWeaponIndex();

		if (id)
		{
			FailWeaponSelect();
			SelectWeapon(id);
			return true;
		}

	}

	return false;
}

void CBot::DoLook()
{
	// what do we want to look at
	GetLookAtVector();

	// looking at something?
	if (LookAtIsValid())
	{
		float fSensitivity;
		if (bot_supermode.GetBool() || m_bIncreaseSensitivity || OnLadder())
			fSensitivity = 15.0f;
		else
			fSensitivity = (float)m_pProfile->m_iSensitivity;

		QAngle requiredAngles;

		VectorAngles(m_vLookAt - GetEyePosition(), requiredAngles);
		CBotGlobals::NormalizeAngle(requiredAngles);

		if (m_iLookTask == LOOK_GROUND)
			requiredAngles.x = 89.0f;

		m_vViewAngles = GetEyeAngles();

		ChangeAngles(fSensitivity, &requiredAngles.x, &m_vViewAngles.x, NULL);
		ChangeAngles(fSensitivity, &requiredAngles.y, &m_vViewAngles.y, NULL);
		CBotGlobals::NormalizeAngle(m_vViewAngles);
	}

	m_bIncreaseSensitivity = false;
}

void CBot::DoButtons()
{
	m_iButtons = m_pButtons->GetBitMask();
}

void CBot::SecondaryAttack(bool bHold)
{
	float fLetGoTime = 0.15;
	float fHoldTime = 0.12;

	if (bHold)
	{
		fLetGoTime = 0.0;
		fHoldTime = 1.0;
	}

	// not currently in "letting go" stage?
	if (bHold || m_pButtons->CanPressButton(IN_ATTACK2))
	{
		m_pButtons->HoldButton
			(
			IN_ATTACK2,
			0/* reaction time? (time to press)*/,
			fHoldTime/* hold time*/,
			fLetGoTime/*let go time*/
			);
	}
}

void CBot::PrimaryAttack(bool bHold, float fTime)
{
	float fLetGoTime = 0.15f;
	float fHoldTime = 0.12f;

	if (bHold)
	{
		fLetGoTime = 0.0f;

		if (fTime)
			fHoldTime = fTime;
		else
			fHoldTime = 2.0f;
	}

	// not currently in "letting go" stage?
	if (bHold || m_pButtons->CanPressButton(IN_ATTACK))
	{
		m_pButtons->HoldButton
			(
			IN_ATTACK,
			0/* reaction time? (time to press)*/,
			fHoldTime/* hold time*/,
			fLetGoTime/*let go time*/
			);
	}
}

void CBot::TapButton(int iButton)
{
	m_pButtons->Tap(iButton);
}

void CBot::LetGoOfButton(int button)
{
	m_pButtons->LetGo(button);
}

void CBot::Reload()
{
	if (m_pButtons->CanPressButton(IN_RELOAD))
		m_pButtons->Tap(IN_RELOAD);
}

void CBot::Use()
{
	if (m_pButtons->CanPressButton(IN_USE))
		m_pButtons->Tap(IN_USE);
}

void CBot::Jump()
{
	if (m_pButtons->CanPressButton(IN_JUMP))
	{
		m_pButtons->HoldButton(IN_JUMP, 0/* time to press*/, 0.5/* hold time*/, 0.5/*let go time*/);
		// do the trademark jump & duck
		m_pButtons->HoldButton(IN_DUCK, 0.2/* time to press*/, 0.3/* hold time*/, 0.5/*let go time*/);
	}
}

void CBot::Duck(bool hold)
{
	if (hold || m_pButtons->CanPressButton(IN_DUCK))
		m_pButtons->HoldButton(IN_DUCK, 0.0/* time to press*/, 1.0/* hold time*/, 0.5/*let go time*/);
}

// TO DO: perceptron method
bool CBot::WantToFollowEnemy()
{
	return GetHealthPercent() > (1.0f - m_pProfile->m_fBraveness);
}
////////////////////////////
void CBot::GetTasks(unsigned int iIgnore)
{
	if (!m_bLookedForEnemyLast && m_pLastEnemy && CBotGlobals::EntityIsAlive(m_pLastEnemy))
	{
		if (WantToFollowEnemy())
		{
			Vector vVelocity;
			CClassInterface::GetVelocity(m_pLastEnemy, &vVelocity);
			CBotSchedule *pSchedule = new CBotSchedule();

			CFindPathTask *pFindPath = new CFindPathTask(m_vLastSeeEnemy);

			pSchedule->AddTask(pFindPath);
			pSchedule->AddTask(new CFindLastEnemy(m_vLastSeeEnemy, vVelocity));

			//////////////
			pFindPath->SetNoInterruptions();

			m_pSchedules->Add(pSchedule);

			m_bLookedForEnemyLast = true;
		}
	}

	if (!m_pSchedules->IsEmpty())
		return; // already got some tasks left

#if defined USE_NAVMESH
	// roam
	INavMeshArea *pWaypoint = g_pNavMesh->GetAreas()->Element(RandomInt(0, g_pNavMesh->GetAreas()->Count()-1));

	if(pWaypoint)
	{
		const Vector vMin = pWaypoint->GetExtentLow();
		const Vector vMax = pWaypoint->GetExtentHigh();

		Vector vPoint;
		vPoint.Init();
		vPoint.x = RandomFloat(vMin.x, vMax.x);
		vPoint.y = RandomFloat(vMin.y, vMax.y);
		vPoint.z = pWaypoint->GetZ(vPoint);

		m_pSchedules->Add(new CBotGotoOriginSched(vPoint));
	}
#else
	// roam
	CWaypoint *pWaypoint = CWaypoints::GetWaypoint(CWaypoints::RandomFlaggedWaypoint(GetTeam()));

	if (pWaypoint)
	{
		m_pSchedules->Add(new CBotGotoOriginSched(pWaypoint->GetOrigin()));
	}
#endif
}

//////////////////////

#if SOURCE_ENGINE >= SE_EYE
#define NETMSG_BITS 6
#else
#define NETMSG_BITS 5
#endif

#if SOURCE_ENGINE >= SE_LEFT4DEAD
#define NET_SETCONVAR	6
#else
#define NET_SETCONVAR	5
#endif

// If we turn off prediction, where the bot thinks other entities hitboxes are will be where they actually are.
// It also has a bonus to the server modifying the clients commands, weapons animate and play sound properly.
inline void SetPredicted(edict_t *pBot, bool bPredict)
{
	if(!pBot || pBot->IsFree())
		return;

	INetChannel *netchan = static_cast<INetChannel *>(engine->GetPlayerNetInfo(ENTINDEX(pBot)));
	if(netchan == NULL)
		return;

	ConVarRef cvar("sv_client_predict");
	const char *oldValue = cvar.GetString();

	char data[256];
	bf_write buffer(data, sizeof(data));
	buffer.WriteUBitLong(NET_SETCONVAR, NETMSG_BITS);
	buffer.WriteByte(1);
	buffer.WriteString("sv_client_predict");
	buffer.WriteString(bPredict ? oldValue : "0");

	netchan->SendData(buffer);

	//SetEntData<bool>(pBot, "m_bLagCompensation", bPredict);
	SetEntData<bool>(pBot, "m_bPredictWeapons", bPredict);
}

void CBots::BotFunction(IBotFunction &functor)
{
	for (unsigned int i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_Bots[i]->InUse() && m_Bots[i]->GetEdict())
			functor.Execute(m_Bots[i]);
	}
}

void CBots::Init()
{
	m_Bots = new CBot*[MAX_PLAYERS];

	for (short int i = 0; i < MAX_PLAYERS; i++)
	{
		switch (CBotGlobals::GetCurrentMod()->GetBotType())
		{
			/*case BOTTYPE_DOD:
				m_Bots[i] = new CDODBot();
				break;
			case BOTTYPE_CSS:
				m_Bots[i] = new CCSSBot();
				break;
			case BOTTYPE_HL2DM:
				m_Bots[i] = new CHLDMBot();
				break;
			case BOTTYPE_HL1DM:
				m_Bots[i] = new CHL1DMSrcBot();
				break;
			case BOTTYPE_COOP:
				m_Bots[i] = new CBotCoop();
				break;*/
			case BOTTYPE_TF2:
				m_Bots[i] = new CBotTF2();
				break;
			/*case BOTTYPE_FF:
				m_Bots[i] = new CBotFF();
				break;
			case BOTTYPE_ZOMBIE:
				m_Bots[i] = new CBotZombie();
				break;*/
			default:
				m_Bots[i] = new CBot();
				break;
		}
	}
}

CBot *CBots::FindBotByProfile(CBotProfile *pProfile)
{
	CBot *pBot = NULL;

	for (short int i = 0; i < MAX_PLAYERS; i++)
	{
		pBot = m_Bots[i];

		if (pBot->InUse())
		{
			if (pBot->IsUsingProfile(pProfile))
				return pBot;
		}
	}

	return NULL;
}

void CBots::RunPlayerMoveAll()
{
	static CBot *pBot;

	for (short int i = 0; i < MAX_PLAYERS; i++)
	{
		pBot = m_Bots[i];

		if (pBot->InUse())
		{
			pBot->RunPlayerMove();
		}
	}
}

void CBots::BotThink(bool simulating)
{
	if (/*!bot_stop.GetBool() && */simulating)
	{
		for (short int i = 0; i < MAX_PLAYERS; i++)
		{
			CBot *pBot = m_Bots[i];

			if (pBot->InUse())
			{
				pBot->SetMoveLookPriority(MOVELOOK_THINK);
				pBot->Think();
				pBot->SetMoveLookPriority(MOVELOOK_EVENT);
				pBot->RunPlayerMove();
			}
		}

		CBotGlobals::GetCurrentMod()->ModFrame();
	}
}

void CBots::MakeBot(edict_t *pEdict)
{
	if (forwardOnAFK != NULL)
	{
		if (forwardOnAFK->GetFunctionCount())
		{
			CBaseEntity *pEntity = pEdict->GetNetworkable()->GetBaseEntity();
			int client = gamehelpers->EntityToBCompatRef(pEntity);
			forwardOnAFK->PushCell(client);
			forwardOnAFK->PushCell(true);

			cell_t result = 0;
			forwardOnAFK->Execute(&result);

			if (result >= Pl_Handled)
			{
				return;
			}
		}
	}

	CBotProfile *pBotProfile = NULL;
	if (!bot_skill_randomize.GetBool())
	{
		pBotProfile = CBotProfiles::GetDefaultProfile();
	}
	else
	{
		int iVisionTicks = RandomInt(bot_visrevs_min.GetInt(), bot_visrevs_max.GetInt());
		int iPathTicks = RandomInt(bot_pathrevs_min.GetInt(), bot_pathrevs_max.GetInt());
		int iVisionTicksClients = RandomInt(bot_visrevs_client_min.GetInt(), bot_visrevs_client_max.GetInt());
		int iSensitivity = RandomInt(bot_sensitivity_min.GetInt(), bot_sensitivity_max.GetInt());
		float fBraveness = RandomFloat(bot_braveness_min.GetFloat(), bot_braveness_max.GetFloat());
		float fAimSkill = RandomFloat(bot_skill_min.GetFloat(), bot_skill_max.GetFloat());
		pBotProfile = new CBotProfile(iVisionTicks,
		                              iPathTicks,
			                          iVisionTicksClients,
			                          iSensitivity,
			                          fBraveness,
			                          fAimSkill);

		CBotProfiles::m_Profiles.push_back(pBotProfile);
	}

	m_Bots[SlotOfEdict(pEdict)]->CreateBotFromEdict(pEdict, pBotProfile);
	SetPredicted(pEdict, false);

	if (bot_printstatus.GetBool())
	{
		char message[255];
		smutils->Format(message, 255, "\x01\x07%06X[AFKBot]\x01 %s is now AFK", 0xD32CE6, playerhelpers->GetGamePlayer(pEdict)->GetName());
		CBotGlobals::PrintToChatAll(message);
	}
}

void CBots::MakeNotBot(edict_t *pEdict)
{
	if (forwardOnAFK != NULL)
	{
		if (forwardOnAFK->GetFunctionCount())
		{
			CBaseEntity *pEntity = pEdict->GetNetworkable()->GetBaseEntity();
			int client = gamehelpers->EntityToBCompatRef(pEntity);
			forwardOnAFK->PushCell(client);
			forwardOnAFK->PushCell(false);

			cell_t result = 0;
			forwardOnAFK->Execute(&result);

			if (result >= Pl_Handled)
			{
				return;
			}
		}
	}

	CBot *pBot = CBots::GetBotPointer(pEdict);
	if (pBot && pBot->InUse())
	{
		SetPredicted(pEdict, true);
		pBot->FreeAllMemory();

		if (bot_printstatus.GetBool())
		{
			char message[255];
			smutils->Format(message, 255, "\x01\x07%06X[AFKBot]\x01 %s is no longer AFK", 0xD32CE6, playerhelpers->GetGamePlayer(pEdict)->GetName());
			CBotGlobals::PrintToChatAll(message);
		}
	}
}

CBot *CBots::GetBotPointer(edict_t *pEdict)
{
	if (!pEdict)
		return NULL;

	int slot = SlotOfEdict(pEdict);
	if ((slot < 0) || (slot > MAX_PLAYERS))
		return NULL;

	CBot *pBot = m_Bots[slot];
	if (pBot->InUse())
		return pBot;

	return NULL;
}

void CBots::FreeMapMemory()
{
	if (m_Bots == NULL)
		return;

	//bots should have been freed when they disconnected
	// just incase do this 
	for (short int i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_Bots[i])
			m_Bots[i]->FreeMapMemory();
	}
}

void CBots::FreeAllMemory()
{
	if (m_Bots == NULL)
		return;

	for (short int i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_Bots[i] != NULL)
		{
			m_Bots[i]->FreeAllMemory();
			delete m_Bots[i];
			m_Bots[i] = NULL;
		}
	}

	delete[] m_Bots;
	m_Bots = NULL;
}

void CBots::RoundStart()
{
	for (short int i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_Bots[i]->InUse())
			m_Bots[i]->SpawnInit();
	}
}

void CBots::MapInit()
{
	
}

////////////////////////
CBotLastSee::CBotLastSee(edict_t *pEdict)
{
	m_pLastSee = MyEHandle(pEdict);
	Update();
}

void CBotLastSee::Update()
{
	if (!m_pLastSee.IsValid() || !CBotGlobals::EntityIsAlive(m_pLastSee))
	{
		m_fLastSeeTime = 0.0f;
		m_pLastSee = MyEHandle();
	}
	else
	{
		m_fLastSeeTime = TIME_NOW;
		m_vLastSeeLoc = CBotGlobals::EntityOrigin(m_pLastSee);
		CClassInterface::GetVelocity(m_pLastSee, &m_vLastSeeVel);
	}
}

bool CBotLastSee::HasSeen(float fTime)
{
	return (m_pLastSee.IsValid() && ((m_fLastSeeTime + fTime) > TIME_NOW));
}

Vector CBotLastSee::GetLocation()
{
	return (m_vLastSeeLoc + m_vLastSeeVel);
}
