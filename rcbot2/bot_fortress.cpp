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

#include "ndebugoverlay.h"

#include "bot_fortress.h"
#include "bot_buttons.h"
#include "bot_globals.h"
#include "bot_profile.h"
#include "bot_schedule.h"
#include "bot_task.h"
#include "bot_waypoint.h"
#include "bot_navigator.h"
#include "bot_mods.h"
#include "bot_visibles.h"
#include "bot_weapons.h"
#include "bot_waypoint_locations.h"
#include "in_buttons.h"
#include "bot_utility.h"
#include "bot_configfile.h"
#include "bot_getprop.h"
//#include "bot_mtrand.h"
#include "bot_wpt_dist.h"
#include "bot_squads.h"
//#include "bot_hooks.h"

#include <IEngineTrace.h>

extern ConVar bot_beliefmulti;
extern ConVar bot_spyknifefov;
extern ConVar bot_use_vc_commands;
extern ConVar bot_max_cc_time;
extern ConVar bot_min_cc_time;
extern ConVar bot_change_class;
extern ConVar bot_demo_jump;
extern ConVar bot_melee_only;
extern ConVar bot_tf2_protect_cap_time;
extern ConVar bot_tf2_protect_cap_percent;
extern ConVar bot_tf2_spy_kill_on_cap_dist;
extern ConVar bot_speed_boost;
extern ConVar bot_projectile_tweak;

#ifdef GetClassName
#undef GetClassName
#endif

#define TF2_SPY_CLOAK_BELIEF 40
#define TF2_HWGUY_REV_BELIEF 60
//extern float g_fBotUtilityPerturb [TF_CLASS_MAX][BOT_UTIL_MAX];

// Payload stuff by   The_Shadow

extern IServerPluginHelpers* helpers;
extern IPlayerInfoManager* playerinfomanager;

void CBroadcastOvertime::Execute(CBot*pBot)
{
	pBot->UpdateCondition(CONDITION_CHANGED);
	pBot->UpdateCondition(CONDITION_PUSH);
}
void CBroadcastCapturedPoint::Execute(CBot *pBot)
{
	((CBotTF2*)pBot)->PointCaptured(m_iPoint, m_iTeam, m_szName);
}

CBroadcastCapturedPoint::CBroadcastCapturedPoint(int iPoint, int iTeam, const char *szName)
{
	m_iPoint = iPoint;
	m_iTeam = iTeam;
	m_szName = CStrings::GetString(szName);
}

void CBroadcastSpySap::Execute(CBot *pBot)
{
	if (CTeamFortress2Mod::GetTeam(m_pSpy) != pBot->GetTeam())
	{
		if (pBot->IsVisible(m_pSpy))
			((CBotTF2*)pBot)->FoundSpy(m_pSpy, CTeamFortress2Mod::GetSpyDisguise(m_pSpy));
	}
}
// special delivery
void CBroadcastFlagReturned::Execute(CBot*pBot)
{
	//if (pBot->GetTeam() == m_iTeam)
	//	((CBotTF2*)pBot)->FlagReturned_SD(m_vOrigin);
	//else
	//	((CBotTF2*)pBot)->TeamFlagDropped(m_vOrigin);	
}

void CBroadcastFlagDropped::Execute(CBot *pBot)
{
	if (pBot->GetTeam() == m_iTeam)
		((CBotTF2*)pBot)->FlagDropped(m_vOrigin);
	else
		((CBotTF2*)pBot)->TeamFlagDropped(m_vOrigin);
}
// flag picked up
void CBotTF2FunctionEnemyAtIntel::Execute(CBot *pBot)
{
	if (m_iType == EVENT_CAPPOINT)
	{
		if (CTeamFortress2Mod::m_ObjectiveResource.GetOwningTeam(m_iCapIndex) != pBot->GetTeam())
			return;
	}

	pBot->UpdateCondition(CONDITION_PUSH);

	if (pBot->GetTeam() != m_iTeam)
	{
		((CBotTF2*)pBot)->EnemyAtIntel(m_vPos, m_iType, m_iCapIndex);
	}
	else
		((CBotTF2*)pBot)->TeamFlagPickup();
}

void CBotTF2::HearVoiceCommand(edict_t *pPlayer, byte cmd)
{
	switch (cmd)
	{
	case TF_VC_SPY:
		// someone shouted spy, HACK the bot to think they saw a spy here too
		// for spy checking purposes
		if (IsVisible(pPlayer))
		{
			m_vLastSeeSpy = CBotGlobals::EntityOrigin(pPlayer);
			m_fSeeSpyTime = engine->Time() + RandomFloat(3.0f, 6.0f);
			//m_pPrevSpy = pPlayer; // HACK
		}
		break;
		// somebody shouted "MEDIC!"
	case TF_VC_MEDIC:
		MedicCalled(pPlayer);
		break;
	case TF_VC_SENTRYHERE: // hear 'put sentry here' 
		// if I'm carrying a sentry just drop it here
		if (GetClass() == TF_CLASS_ENGINEER)
		{
			if (m_bIsCarryingObj && m_bIsCarryingSentry)
			{
				if (IsVisible(pPlayer) && (DistanceFrom(pPlayer) < 512))
				{
					if (RandomInt(0, 100) > 75)
						AddVoiceCommand(TF_VC_YES);

					PrimaryAttack();

					m_pSchedules->RemoveSchedule(SCHED_TF2_ENGI_MOVE_BUILDING);
				}
				else if (RandomInt(0, 100) > 75)
					AddVoiceCommand(TF_VC_NO);
			}
		}
		break;
	case TF_VC_HELP:
		// add utility can find player
		if (IsVisible(pPlayer))
		{
			if (!m_pSchedules->IsCurrentSchedule(SCHED_GOTO_ORIGIN))
			{
				m_pSchedules->RemoveSchedule(SCHED_GOTO_ORIGIN);

				m_pSchedules->AddFront(new CBotGotoOriginSched(pPlayer));
			}
		}
		break;
	case TF_VC_GOGOGO:
		// if bot is nesting, or waiting for something, it will go
		if (DistanceFrom(pPlayer) > 512)
			return;

		UpdateCondition(CONDITION_PUSH);

		if (RandomFloat(0, 1.0) > 0.75f)
			m_nextVoicecmd = TF_VC_YES;

		// don't break // flow down to uber if medic
	case TF_VC_ACTIVATEUBER:
		if (CTeamFortress2Mod::HasRoundStarted() && (GetClass() == TF_CLASS_MEDIC))
		{
			if (m_pHeal == pPlayer)
			{
				if (!CTeamFortress2Mod::IsFlagCarrier(pPlayer))
					SecondaryAttack();
				else if (RandomFloat(0, 1.0) > 0.5f)
					m_nextVoicecmd = TF_VC_NO;
			}
		}
		break;
	case TF_VC_MOVEUP:

		if (DistanceFrom(pPlayer) > 1000)
			return;

		UpdateCondition(CONDITION_PUSH);

		if (RandomFloat(0, 1.0) > 0.75f)
			m_nextVoicecmd = TF_VC_YES;

		break;
	default:
		break;
	}
}

void CBroadcastFlagCaptured::Execute(CBot *pBot)
{
	if (pBot->GetTeam() == m_iTeam)
		((CBotTF2*)pBot)->FlagReset();
	else
		((CBotTF2*)pBot)->TeamFlagReset();
}

void CBroadcastRoundStart::Execute(CBot *pBot)
{
	((CBotTF2*)pBot)->RoundReset(m_bFullReset);
}

CBotFortress::CBotFortress()
{
	CBot();

	m_iLastFailSentryWpt = -1;
	m_iLastFailTeleExitWpt = -1;

	// remember prev spy disguised in game while playing
	m_iPrevSpyDisguise = (TFClass)0;

	m_fSentryPlaceTime = 0;
	m_iSentryKills = 0;
	m_fSnipeAttackTime = 0;
	m_pAmmo = NULL;
	m_pHealthkit = NULL;
	m_pFlag = NULL;
	m_pHeal = NULL;
	m_fCallMedic = 0;
	m_fTauntTime = 0;
	m_fLastKnownFlagTime = 0.0f;
	m_bHasFlag = false;
	m_pSentryGun = NULL;
	m_pDispenser = NULL;
	m_pTeleExit = NULL;
	m_pTeleEntrance = NULL;
	m_pNearestDisp = NULL;
	m_pNearestEnemySentry = NULL;
	m_pNearestEnemyTeleporter = NULL;
	m_pNearestEnemyDisp = NULL;
	m_pNearestPipeGren = NULL;
	m_pPrevSpy = NULL;
	m_fSeeSpyTime = 0.0f;
	m_bEntranceVectorValid = false;
	m_pLastCalledMedic = NULL;
	m_fLastCalledMedicTime = 0.0f;
	m_bIsBeingHealed = false;
	m_bCanBeUbered = false;
}

void CBotFortress::CheckDependantEntities()
{
	CBot::CheckDependantEntities();
}

void CBotFortress::Init(bool bVarInit)
{
	CBot::Init(bVarInit);

	m_bCheckClass = false;
	m_bHasFlag = false;
	m_iClass = TF_CLASS_MAX; // important

}

void CBotFortress::Setup()
{
	CBot::Setup();
}

bool CBotFortress::SomeoneCalledMedic()
{
	return (GetClass() == TF_CLASS_MEDIC) &&
		(m_pLastCalledMedic.Get() != NULL) &&
		((m_fLastCalledMedicTime + 30.0f) > engine->Time());
}

bool CBotTF2::SentryRecentlyHadEnemy()
{
	return (m_fLastSentryEnemyTime + 15.0f) > engine->Time();
}

bool CBotFortress::StartGame()
{
	int team = m_pPI->GetTeamIndex();

	m_iClass = (TFClass)CClassInterface::GetTF2Class(m_pEdict);

	if ((team != TF2_TEAM_BLUE) && (team != TF2_TEAM_RED))
	{
		SelectTeam();
	}

	if (m_iDesiredClass == -1) // invalid class
	{
		ChooseClass();
	}
	else if ((m_iDesiredClass > 0 && (m_iClass != m_iDesiredClass)) || (m_iClass == TF_CLASS_MAX))
	{
		// can't change class in MVM during round!
		if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM) && CTeamFortress2Mod::HasRoundStarted())
			return true;

		SelectClass();
	}
	else
		return true;

	return false;
}

void CBotFortress::PickedUpFlag()
{
	m_bHasFlag = true;
	// clear tasks
	m_pSchedules->FreeMemory();
}

void CBotFortress::CheckHealingValid()
{
	if (m_pHeal)
	{
		if (!CBotGlobals::EntityIsValid(m_pHeal) || !CBotGlobals::EntityIsAlive(m_pHeal))
		{
			m_pHeal = NULL;
			RemoveCondition(CONDITION_SEE_HEAL);
		}
		else if (!IsVisible(m_pHeal))
		{
			m_pHeal = NULL;
			RemoveCondition(CONDITION_SEE_HEAL);
		}
		else if (GetHealFactor(m_pHeal) == 0.0f)
		{
			m_pHeal = NULL;
			RemoveCondition(CONDITION_SEE_HEAL);
		}
	}
	else
		RemoveCondition(CONDITION_SEE_HEAL);
}

float CBotFortress::GetHealFactor(edict_t *pPlayer)
{
	// Factors are 
	// 1. health
	// 2. max health
	// 3. ubercharge
	// 4. player class
	// 5. etc
	float fFactor = 0.0f;
	float fLastCalledMedic = 0.0f;
	bool bHeavyClass = false;
	edict_t *pMedigun = CTeamFortress2Mod::GetMediGun(m_pEdict);
	float fHealthPercent;
	Vector vVel = Vector(0, 0, 0);
	int iHighestScore = CTeamFortress2Mod::GetHighestScore();

	CClassInterface::GetVelocity(pPlayer, &vVel);

	IPlayerInfo *p = playerhelpers->GetGamePlayer(pPlayer)->GetPlayerInfo();
	TFClass iClass = (TFClass)CClassInterface::GetTF2Class(pPlayer);

	if (!CBotGlobals::EntityIsAlive(pPlayer) || !p || p->IsDead() || p->IsObserver() || !p->IsConnected())
		return 0.0f;

	if (CClassInterface::GetTF2NumHealers(pPlayer) > 1)
		return 0.0f;

	fHealthPercent = (p->GetHealth() / p->GetMaxHealth());

	switch (iClass)
	{
	case TF_CLASS_MEDIC:

		if (fHealthPercent >= 1.0f)
			return 0.0f;

		fFactor = 0.1f;

		break;
	case TF_CLASS_DEMOMAN:
	case TF_CLASS_HWGUY:
	case TF_CLASS_SOLDIER:
	case TF_CLASS_PYRO:
	{
		bHeavyClass = true;

		fFactor += 1.0f;

		if (pMedigun)
		{
			// overheal HWGUY/SOLDIER/DEMOMAN
			fFactor += (float)(CClassInterface::GetUberChargeLevel(pMedigun)) / 100;

			if (CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict)) // uber deployed
				fFactor += (1.0f - ((float)(CClassInterface::GetUberChargeLevel(pMedigun)) / 100));
		}
	}
	// drop down
	case TF_CLASS_SPY:
		if (iClass == TF_CLASS_SPY)
		{
			int iClass, iTeam, iIndex, iHealth;

			if (CClassInterface::GetTF2SpyDisguised(pPlayer, &iClass, &iTeam, &iIndex, &iHealth))
			{
				if (iTeam != m_iTeam)
					return 0.0f;
			}

			if (CTeamFortress2Mod::TF2_IsPlayerCloaked(pPlayer))
				return 0.0f;
		}
	default:

		if (!bHeavyClass && pMedigun) // add more factor bassed on uber charge level - bot can gain more uber charge
		{
			fFactor += (0.1f - ((float)(CClassInterface::GetUberChargeLevel(pMedigun)) / 1000));

			if ((m_StatsCanUse.stats.m_iTeamMatesVisible == 1) && (fHealthPercent >= 1.0f))
				return 0.0f; // find another guy
		}

		if (bHeavyClass)
		{
			IPlayerInfo *p;

			p = playerinfomanager->GetPlayerInfo(pPlayer);

			if (p)
			{
				if (p->GetLastUserCommand().buttons & IN_ATTACK)
					fFactor += 1.0f;
			}
		}

		fFactor += 1.0f - fHealthPercent;

		if (CTeamFortress2Mod::TF2_IsPlayerOnFire(pPlayer))
			fFactor += 2.0f;

		// higher movement better
		fFactor += (vVel.Length() / 1000);

		// favour big guys
		fFactor += ((float)p->GetMaxHealth()) / 200;

		// favour those with bigger scores
		if (iHighestScore == 0)
			iHighestScore = 1;

		fFactor += (((float)CClassInterface::GetTF2Score(pPlayer)) / iHighestScore) / 2;

		if ((fLastCalledMedic = m_fCallMedicTime[ENTINDEX(pPlayer) - 1]) > 0)
			fFactor += MAX(0.0f, 1.0f - ((engine->Time() - fLastCalledMedic) / 5));
		if (((m_fLastCalledMedicTime + 5.0f) > engine->Time()) && (m_pLastCalledMedic == pPlayer))
			fFactor += 0.5f;

		// More priority to flag carriers and cappers
		if (CTeamFortress2Mod::IsFlagCarrier(pPlayer) || CTeamFortress2Mod::IsCapping(pPlayer))
			fFactor *= 1.5f;
	}

	return fFactor;
}


/////////////////////////////////////////////////////////////////////
//
// When a new Entity becomes visible or Invisible this is called
// 
// bVisible = true when pEntity is Visible
// bVisible = false when pEntity becomes inVisible
bool CBotFortress::SetVisible(edict_t *pEntity, bool bVisible)
{
	bool bValid = CBot::SetVisible(pEntity, bVisible);

	// check for people to heal
	if (m_iClass == TF_CLASS_MEDIC)
	{
		if (bValid && bVisible)
		{
			if (CBotGlobals::IsPlayer(pEntity)) // player
			{
				CBotWeapon *pMedigun = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_MEDIGUN));
				bool bIsSpy = CClassInterface::GetTF2Class(pEntity) == TF_CLASS_SPY;
				int iDisguise = 0;

				if (bIsSpy)
				{
					CClassInterface::GetTF2SpyDisguised(pEntity, &iDisguise, NULL, NULL, NULL);
				}

				if (pMedigun && pMedigun->HasWeapon() &&
					(  // Heal my team member or a spy if I think he is on my team
					(CBotGlobals::GetTeam(pEntity) == GetTeam()) ||
					((bIsSpy && !ThinkSpyIsEnemy(pEntity, (TFClass)iDisguise)))
					))
				{
					Vector vPlayer = CBotGlobals::EntityOrigin(pEntity);

					if (DistanceFrom(vPlayer) <= CWaypointLocations::REACHABLE_RANGE)
					{
						float fFactor;

						if ((fFactor = GetHealFactor(pEntity)) > 0)
						{
							if (m_pHeal.Get() != NULL)
							{
								if (m_pHeal != pEntity)
								{
									if (fFactor > m_fHealFactor)
									{
										m_pHeal = pEntity;
										m_fHealFactor = fFactor;
										UpdateCondition(CONDITION_SEE_HEAL);
									}
								}
								else
								{
									// not healing -- what am I doing?
									if (!m_pSchedules->HasSchedule(SCHED_HEAL))
									{
										// not healing -- what am I doing?
										m_pSchedules->FreeMemory();
										m_pSchedules->AddFront(new CBotTF2HealSched(m_pHeal));
									}
								}
							}
							else
							{
								m_fHealFactor = fFactor;
								m_pHeal = pEntity;
								UpdateCondition(CONDITION_SEE_HEAL);

								if (!m_pSchedules->HasSchedule(SCHED_HEAL))
								{
									// not healing -- what am I doing?
									m_pSchedules->FreeMemory();
									m_pSchedules->AddFront(new CBotTF2HealSched(m_pHeal));
								}
							}
						}
					}
				}
			}
		}
		else if (m_pHeal == pEntity)
		{
			m_pHeal = NULL;
			RemoveCondition(CONDITION_SEE_HEAL);
		}
	}
	//else if ( m_iClass == TF_CLASS_SPY ) // Fix
	//{
	// Look for nearest sentry to sap!!!
	if (bValid && bVisible)
	{
		if (CTeamFortress2Mod::IsSentry(pEntity, CTeamFortress2Mod::GetEnemyTeam(GetTeam())))
		{
			if ((m_iClass != TF_CLASS_ENGINEER) || !CClassInterface::IsObjectCarried(pEntity))
			{
				if (!m_pNearestEnemySentry || ((pEntity != m_pNearestEnemySentry) && (DistanceFrom(pEntity) < DistanceFrom(m_pNearestEnemySentry))))
				{
					m_pNearestEnemySentry = pEntity;
				}
			}
		}
		else if (CTeamFortress2Mod::IsTeleporter(pEntity, CTeamFortress2Mod::GetEnemyTeam(GetTeam())))
		{
			if (!m_pNearestEnemyTeleporter || ((pEntity != m_pNearestEnemyTeleporter) && (DistanceFrom(pEntity) < DistanceFrom(m_pNearestEnemyTeleporter))))
			{
				m_pNearestEnemyTeleporter = pEntity;
			}
		}
		else if (CTeamFortress2Mod::IsDispenser(pEntity, CTeamFortress2Mod::GetEnemyTeam(GetTeam())))
		{
			if (!m_pNearestEnemyDisp || ((pEntity != m_pNearestEnemyDisp) && (DistanceFrom(pEntity) < DistanceFrom(m_pNearestEnemyDisp))))
			{
				m_pNearestEnemyDisp = pEntity;
			}
		}
		else if (CTeamFortress2Mod::IsHurtfulPipeGrenade(pEntity, m_pEdict))
		{
			if (!m_pNearestPipeGren || ((pEntity != m_pNearestPipeGren) && (DistanceFrom(pEntity) < DistanceFrom(m_pNearestPipeGren))))
			{
				m_pNearestPipeGren = pEntity;
			}
		}
	}
	else if (pEntity == m_pNearestEnemySentry)
	{
		m_pNearestEnemySentry = NULL;
	}
	else if (pEntity == m_pNearestEnemyTeleporter)
	{
		m_pNearestEnemyTeleporter = NULL;
	}
	else if (pEntity == m_pNearestEnemyDisp)
	{
		m_pNearestEnemyDisp = NULL;
	}
	else if (pEntity == m_pNearestPipeGren)
	{
		m_pNearestPipeGren = NULL;
	}

	//}

	// Check for nearest Dispenser for health/ammo & flag
	if (bValid && bVisible && !(CClassInterface::GetEffects(pEntity) & EF_NODRAW)) // EF_NODRAW == invisible
	{
		if ((m_pFlag != pEntity) && CTeamFortress2Mod::IsFlag(pEntity, GetTeam()))
			m_pFlag = pEntity;
		else if ((m_pNearestAllySentry != pEntity) && CTeamFortress2Mod::IsSentry(pEntity, GetTeam()))
		{
			if (!m_pNearestAllySentry || (DistanceFrom(pEntity) < DistanceFrom(m_pNearestAllySentry)))
				m_pNearestAllySentry = pEntity;
		}
		else if ((m_pNearestDisp != pEntity) && CTeamFortress2Mod::IsDispenser(pEntity, GetTeam()))
		{
			if (!m_pNearestDisp || (DistanceFrom(pEntity) < DistanceFrom(m_pNearestDisp)))
				m_pNearestDisp = pEntity;
		}
		else if ((pEntity != m_pNearestTeleEntrance) && CTeamFortress2Mod::IsTeleporterEntrance(pEntity, GetTeam()))
		{
			if (!m_pNearestTeleEntrance || (DistanceFrom(pEntity) < DistanceFrom(m_pNearestTeleEntrance)))
				m_pNearestTeleEntrance = pEntity;
		}
		else if ((pEntity != m_pAmmo) && CTeamFortress2Mod::IsAmmo(pEntity))
		{
			static float fDistance;

			fDistance = DistanceFrom(pEntity);

			if (fDistance <= 200)
			{
				if (!m_pAmmo || (fDistance < DistanceFrom(m_pAmmo)))
					m_pAmmo = pEntity;
			}

		}
		else if ((pEntity != m_pHealthkit) && CTeamFortress2Mod::IsHealthKit(pEntity))
		{
			static float fDistance;

			fDistance = DistanceFrom(pEntity);

			if (fDistance <= 200)
			{
				if (!m_pHealthkit || (fDistance < DistanceFrom(m_pHealthkit)))
					m_pHealthkit = pEntity;
			}
		}
	}
	else
	{
		if (pEntity == m_pFlag.Get_Old())
			m_pFlag = NULL;
		else if (pEntity == m_pNearestDisp.Get_Old())
			m_pNearestDisp = NULL;
		else if (pEntity == m_pAmmo.Get_Old())
			m_pAmmo = NULL;
		else if (pEntity == m_pHealthkit.Get_Old())
			m_pHealthkit = NULL;
		else if (pEntity == m_pHeal.Get_Old())
			m_pHeal = NULL;
		else if (pEntity == m_pNearestPipeGren.Get_Old())
			m_pNearestPipeGren = NULL;
	}

	return bValid;
}

void CBotFortress::MedicCalled(edict_t *pPlayer)
{
	bool bGoto = true;

	if (pPlayer == m_pEdict)
		return; // can't heal self!
	if (m_iClass != TF_CLASS_MEDIC)
		return; // nothing to do
	if (DistanceFrom(pPlayer) > 1024) // a bit far away
		return; // ignore
	if ((CBotGlobals::GetTeam(pPlayer) == GetTeam()) || (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_SPY) && ThinkSpyIsEnemy(pPlayer, CTeamFortress2Mod::GetSpyDisguise(pPlayer)))
	{

		m_pLastCalledMedic = pPlayer;
		m_fLastCalledMedicTime = engine->Time();
		m_fCallMedicTime[ENTINDEX(pPlayer) - 1] = m_fLastCalledMedicTime;

		if (m_pHeal == pPlayer)
			return; // already healing


		if (m_pHeal)
		{
			if (CClassInterface::GetPlayerHealth(pPlayer) >= CClassInterface::GetPlayerHealth(m_pHeal))
				bGoto = false;
		}

		if (bGoto)
		{
			m_pHeal = pPlayer;
		}

		m_pLastHeal = m_pHeal;

	}
}

void CBotFortress::WaitBackstab()
{
	m_fBackstabTime = engine->Time() + RandomFloat(5.0f, 10.0f);
	m_pLastEnemy = NULL;
}

bool CBotFortress::IsAlive()
{
	return !m_pPI->IsDead() && !m_pPI->IsObserver();
}

// hurt enemy player
void CBotFortress::SeeFriendlyHurtEnemy(edict_t *pTeammate, edict_t *pEnemy, CWeapon *pWeapon)
{
	if (CBotGlobals::IsPlayer(pEnemy) && (CClassInterface::GetTF2Class(pEnemy) == TF_CLASS_SPY))
	{
		m_fSpyList[ENTINDEX(pEnemy) - 1] = engine->Time();
	}
}

void CBotFortress::Shot(edict_t *pEnemy)
{
	SeeFriendlyHurtEnemy(m_pEdict, pEnemy, NULL);
}

void CBotFortress::Killed(edict_t *pVictim, char *weapon)
{
	CBot::Killed(pVictim, weapon);

	return;
}

void CBotFortress::Died(edict_t *pKiller, const char *pszWeapon)
{
	CBot::Died(pKiller, pszWeapon);

	if (CBotGlobals::IsPlayer(pKiller) && (CClassInterface::GetTF2Class(pKiller) == TF_CLASS_SPY))
		FoundSpy(pKiller, (TFClass)0);

	DroppedFlag();

	if (RandomInt(0, 1))
		m_pButtons->Attack();

	m_bCheckClass = true;
}

void CBotTF2::BuildingDestroyed(int iType, edict_t *pAttacker, edict_t *pEdict)
{
	eEngiBuild type = (eEngiBuild)iType;

	switch (type)
	{
	case ENGI_DISP:
		m_pDispenser = NULL;
		m_bDispenserVectorValid = false;
		m_iDispenserArea = 0;

		break;
	case ENGI_SENTRY:
		m_pSentryGun = NULL;
		m_bSentryGunVectorValid = false;
		m_iSentryArea = 0;

		break;
	case ENGI_ENTRANCE:
		m_pTeleEntrance = NULL;
		m_bEntranceVectorValid = false;
		m_iTeleEntranceArea = 0;

		break;
	case ENGI_EXIT:
		m_pTeleExit = NULL;
		m_bTeleportExitVectorValid = false;
		m_iTeleExitArea = 0;
		break;
	}

	m_pSchedules->FreeMemory();

	if (pEdict && CBotGlobals::EntityIsValid(pEdict) && pAttacker && CBotGlobals::EntityIsValid(pAttacker))
	{
		Vector vSentry = CBotGlobals::EntityOrigin(pEdict);
		Vector vAttacker = CBotGlobals::EntityOrigin(pAttacker);

		m_pNavigator->Belief(vSentry, vAttacker, bot_beliefmulti.GetFloat(), (vAttacker - vSentry).Length(), BELIEF_DANGER);
	}
}

void CBotFortress::WantToDisguise(bool bSet)
{
	extern ConVar bot_tf2_debug_spies_cloakdisguise;

	if (bot_tf2_debug_spies_cloakdisguise.GetBool())
	{
		if (bSet)
			m_fSpyDisguiseTime = 0.0f;
		else
			m_fSpyDisguiseTime = engine->Time() + 2.0f;
	}
	else
		m_fSpyDisguiseTime = engine->Time() + 10.0f;

}

void CBotFortress::DetectedAsSpy(edict_t *pDetector, bool bDisguiseComprimised)
{
	if (bDisguiseComprimised)
	{
		float fTime = engine->Time() - m_fDisguiseTime;
		float fTotal = 0;

		if ((m_fDisguiseTime < 1) || (fTime < 3.0f))
			return;

		if (m_fClassDisguiseTime[m_iDisguiseClass] == 0)
			m_fClassDisguiseTime[m_iDisguiseClass] = fTime;
		else
			m_fClassDisguiseTime[m_iDisguiseClass] = (m_fClassDisguiseTime[m_iDisguiseClass] * 0.5f) + (fTime * 0.5f);

		for (unsigned short int i = 0; i < 10; i++)
		{
			fTotal += m_fClassDisguiseTime[i];
		}

		for (unsigned short int i = 0; i < 10; i++)
		{
			if (m_fClassDisguiseTime[i] > 0)
				m_fClassDisguiseFitness[i] = (m_fClassDisguiseTime[i] / fTotal);
		}

		m_fDisguiseTime = 0.0f;
	}
	else // go for cover
	{
		m_pAvoidEntity = pDetector;

		if (!m_pSchedules->HasSchedule(SCHED_GOOD_HIDE_SPOT))
		{
			m_pSchedules->FreeMemory();
			m_pSchedules->AddFront(new CGotoHideSpotSched(this, m_pAvoidEntity));
		}
	}
}

void CBotFortress::SpawnInit()
{
	CBot::SpawnInit();

	m_fLastSentryEnemyTime = 0.0f;

	m_fHealFactor = 0.0f;

	m_pHealthkit = MyEHandle(NULL);
	m_pFlag = MyEHandle(NULL);
	m_pNearestDisp = MyEHandle(NULL);
	m_pAmmo = MyEHandle(NULL);
	m_pHeal = MyEHandle(NULL);
	m_pNearestPipeGren = MyEHandle(NULL);

	//m_bWantToZoom = false;

	memset(m_fCallMedicTime, 0, sizeof(float)*MAX_PLAYERS);
	m_fWaitTurnSentry = 0.0f;


	m_pLastSeeMedic.Reset();

	memset(m_fSpyList, 0, sizeof(float)*MAX_PLAYERS);

	m_fTaunting = 0.0f; // bots not moving FIX

	m_fMedicUpdatePosTime = 0.0f;

	m_pLastHeal = NULL;

	m_fDisguiseTime = 0.0f;

	m_pNearestEnemyTeleporter = NULL;
	m_pNearestTeleEntrance = NULL;
	m_fBackstabTime = 0.0f;
	m_fPickupTime = 0.0f;
	m_fDefendTime = 0.0f;
	m_fLookAfterSentryTime = 0.0f;

	m_fSnipeAttackTime = 0.0f;
	m_fSpyCloakTime = 0.0f; //engine->Time();// + randomFloat(5.0f,10.0f);
	m_fSpyUncloakTime = 0.0f;

	m_fLastSaySpy = 0.0f;
	m_fSpyDisguiseTime = 0.0f;
	m_pHeal = NULL;
	m_pNearestDisp = NULL;
	m_pNearestEnemySentry = NULL;
	m_pNearestAllySentry = NULL;
	m_bHasFlag = false;
	//m_pPrevSpy = NULL;
	//m_fSeeSpyTime = 0.0f;

	m_bSentryGunVectorValid = false;
	m_bDispenserVectorValid = false;
	m_bTeleportExitVectorValid = false;
}

bool CBotFortress::IsBuilding(edict_t *pBuilding)
{
	return (pBuilding == m_pSentryGun.Get()) || (pBuilding == m_pDispenser.Get());
}

// return 0 : fail
// return 1 : built ok
// return 2 : next state
// return 3 : another try -- restart
int CBotFortress::EngiBuildObject(int *iState, eEngiBuild iObject, float *fTime, int *iTries)
{
	// can't build while standing on my building!
	if (IsBuilding(CClassInterface::GetGroundEntity(m_pEdict)))
	{
		return 0;
	}

	if (m_fWaitTurnSentry > engine->Time())
		return 2;

	switch (*iState)
	{
	case 0:
	{
		// initialise
		if (HasEngineerBuilt(iObject))
		{
			EngineerBuild(iObject, ENGI_DESTROY);
		}

		*iState = 1;
	}
	break;
	case 1:
	{
		CTraceFilterWorldAndPropsOnly filter;
		QAngle eyes = CBotGlobals::PlayerAngles(m_pEdict);
		QAngle turn;
		Vector forward;
		Vector building;
		Vector vchosen;
		Vector v_right, v_up, v_left;
		Vector v_src = GetEyePosition();
		// find best place to turn it to
		trace_t *tr = CBotGlobals::GetTraceResult();
		int iNextState = 2;

		//		CBotWeapon *pWeapon = getCurrentWeapon();

		float bestfraction = 0.0f;

		m_fWaitTurnSentry = 0.0f;
		// unselect current weapon
		SelectWeapon(0);

		EngineerBuild(iObject, ENGI_BUILD);
		/*
		if (pWeapon->getID() != TF2_WEAPON_BUILDER)
		{
		if (*iTries > 9)
		return 0; // fail

		*iTries = *iTries + 1;

		return 1; // continue;
		}*/

		*iTries = 0;
		iNextState = 8;
		eyes.x = 0; // nullify pitch / we want yaw only
		AngleVectors(eyes, &forward, &v_right, &v_up);
		building = v_src + (forward * 100);
		//////////////////////////////////////////

		// forward
		CBotGlobals::TraceLine(building, building + forward*4096.0, MASK_SOLID_BRUSHONLY, &filter);

		iNextState = 8;
		bestfraction = tr->fraction;

		////////////////////////////////////////

		v_left = -v_right;

		// left
		CBotGlobals::TraceLine(building, building - v_right*4096.0, MASK_SOLID_BRUSHONLY, &filter);

		if (tr->fraction > bestfraction)
		{
			iNextState = 6;
			bestfraction = tr->fraction;
			vchosen = building + v_right*4096.0;
		}
		////////////////////////////////////////
		// back
		CBotGlobals::TraceLine(building, building - forward*4096.0, MASK_SOLID_BRUSHONLY, &filter);

		if (tr->fraction > bestfraction)
		{
			iNextState = 4;
			bestfraction = tr->fraction;
			vchosen = building - forward*4096.0;
		}
		///////////////////////////////////
		// right
		CBotGlobals::TraceLine(building, building + v_right*4096.0, MASK_SOLID_BRUSHONLY, &filter);

		if (tr->fraction > bestfraction)
		{
			iNextState = 2;
			bestfraction = tr->fraction;
			vchosen = building + v_right*4096.0;
		}
		////////////////////////////////////
		*iState = iNextState;
	}
	case 2:
	{
		// let go
		m_pButtons->LetGo(IN_ATTACK2);
		*iState = *iState + 1;
	}
	break;
	case 3:
	{
		TapButton(IN_ATTACK2);
		*iState = *iState + 1;
		m_fWaitTurnSentry = engine->Time() + 0.33f;
		*fTime = *fTime + 0.33f;
	}
	break;
	case 4:
	{
		// let go
		m_pButtons->LetGo(IN_ATTACK2);
		*iState = *iState + 1;
	}
	break;
	case 5:
	{
		TapButton(IN_ATTACK2);
		*iState = *iState + 1;
		m_fWaitTurnSentry = engine->Time() + 0.33f;
		*fTime = *fTime + 0.33f;
	}
	break;
	case 6:

	{
		// let go 
		m_pButtons->LetGo(IN_ATTACK2);
		*iState = *iState + 1;
	}
	break;
	case 7:
	{
		TapButton(IN_ATTACK2);
		*iState = *iState + 1;
		m_fWaitTurnSentry = engine->Time() + 0.33f;
		*fTime = *fTime + 0.33f;
	}
	break;
	case 8:
	{
		// let go (wait)
		m_pButtons->LetGo(IN_ATTACK2);
		*iState = *iState + 1;
	}
	break;
	case 9:
	{
		TapButton(IN_ATTACK);

		*fTime = engine->Time() + RandomFloat(0.5f, 1.0f);
		*iState = *iState + 1;
	}
	break;
	case 10:
	{
		// Check if sentry built OK
		// Wait for Built object message

		m_pButtons->Tap(IN_ATTACK);
		Duck(true);// crouch too

		if (*fTime < engine->Time())
		{
			//hasbuiltobject
			if (HasEngineerBuilt(iObject))
			{
				*iState = *iState + 1;
				// OK, set up whacking time!
				*fTime = engine->Time() + RandomFloat(5.0f, 10.0f);

				if (iObject == ENGI_SENTRY)
				{
					m_bSentryGunVectorValid = false;
					m_fLastSentryEnemyTime = 0.0f;
				}
				else
				{
					if (iObject == ENGI_DISP)
						m_bDispenserVectorValid = false;
					else if (iObject == ENGI_EXIT)
						m_bTeleportExitVectorValid = false;

					RemoveCondition(CONDITION_COVERT);
					return 1;
				}
			}
			else if (*iTries > 3)
			{
				if (iObject == ENGI_SENTRY)
					m_bSentryGunVectorValid = false;
				else if (iObject == ENGI_DISP)
					m_bDispenserVectorValid = false;
				else if (iObject == ENGI_EXIT)
					m_bTeleportExitVectorValid = false;

				return 0;
			}
			else
			{
				//*fTime = engine->Time() + randomFloat(0.5,1.0);
				*iTries = *iTries + 1;
				*iState = 1;

				return 3;
			}
		}
	}
	break;
	case 11:
	{
		// whack it for a while
		if (*fTime < engine->Time())
		{
			RemoveCondition(CONDITION_COVERT);
			return 1;
		}
		else
		{
			TapButton(IN_ATTACK);
			Duck(true);// crouch too
		}

		// someone blew my sentry before I built it!
		if (!HasEngineerBuilt(iObject))
			return 1;
	}
	break;
	}

	return 2;
}

void CBotFortress::SetClass(TFClass _class)
{
	m_iClass = _class;
}

bool CBotFortress::ThinkSpyIsEnemy(edict_t *pEdict, TFClass iDisguise)
{
	return ((m_fSeeSpyTime > engine->Time()) &&  // if bot is in spy check mode
		(m_pPrevSpy == pEdict) &&				 // and its the last spy we saw
		// and its the same disguise or we last saw the spy just a couple of seconds ago
		((m_iPrevSpyDisguise == iDisguise) || ((engine->Time() - m_fLastSeeSpyTime) < 3.0f)));
}

bool CBotTF2::ThinkSpyIsEnemy(edict_t *pEdict, TFClass iDisguise)
{
	return CBotFortress::ThinkSpyIsEnemy(pEdict, iDisguise) ||
		(m_pCloakedSpy && (m_pCloakedSpy == pEdict) && !CTeamFortress2Mod::TF2_IsPlayerCloaked(m_pCloakedSpy)); // maybe i put him on fire
}

bool CBotFortress::IsEnemy(edict_t *pEdict, bool bCheckWeapons)
{
	if (pEdict == m_pEdict)
		return false;

	if (!ENTINDEX(pEdict) || (ENTINDEX(pEdict) > MAX_PLAYERS))
		return false;

	if (CBotGlobals::GetTeam(pEdict) == GetTeam())
		return false;

	return true;
}

bool CBotFortress::NeedAmmo()
{
	return false;
}

bool CBotFortress::NeedHealth()
{
	return (GetHealthPercent() < 0.7) || CTeamFortress2Mod::TF2_IsPlayerOnFire(m_pEdict);
}

bool CBotTF2::NeedAmmo()
{
	if (GetClass() == TF_CLASS_ENGINEER)
	{
		CBotWeapon *pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_WRENCH));

		if (pWeapon)
		{
			int iMetal = pWeapon->GetAmmo(this);

			if ((m_pSentryGun.Get() == NULL) || (CClassInterface::GetTF2UpgradeLevel(m_pSentryGun) < 3))
				return (iMetal < 200); // need 200 to upgrade sentry
			else
				return iMetal < 125; // need 125 for other stuff (e.g. teleporters)
		}
	}
	else if (GetClass() == TF_CLASS_SOLDIER)
	{
		CBotWeapon *pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_ROCKETLAUNCHER));

		if (pWeapon)
		{
			return (pWeapon->GetAmmo(this) < 1);
		}
	}
	else if (GetClass() == TF_CLASS_DEMOMAN)
	{
		CBotWeapon *pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_GRENADELAUNCHER));

		if (pWeapon)
		{
			return (pWeapon->GetAmmo(this) < 1);
		}
	}
	else if (GetClass() == TF_CLASS_HWGUY)
	{
		CBotWeapon *pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_MINIGUN));

		if (pWeapon)
		{
			return (pWeapon->GetAmmo(this) < 1);
		}
	}
	else if (GetClass() == TF_CLASS_PYRO)
	{
		CBotWeapon *pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_FLAMETHROWER));

		if (pWeapon)
		{
			return (pWeapon->GetAmmo(this) < 1);
		}
	}

	return false;
}

void CBotFortress::CurrentlyDead()
{
	CBot::CurrentlyDead();

	m_fUpdateClass = engine->Time() + 0.1f;
}

void CBotFortress::ModThink()
{
	// get class
	m_iClass = (TFClass)CClassInterface::GetTF2Class(m_pEdict);
	m_iTeam = GetTeam();
	//updateClass();

	if (NeedHealth())
		UpdateCondition(CONDITION_NEED_HEALTH);
	else
		RemoveCondition(CONDITION_NEED_HEALTH);

	if (NeedAmmo())
		UpdateCondition(CONDITION_NEED_AMMO);
	else
		RemoveCondition(CONDITION_NEED_AMMO);

	//if ( !hasSomeConditions(CONDITION_PUSH) )
	///{
	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) || CTeamFortress2Mod::TF2_IsPlayerKrits(m_pEdict))
		UpdateCondition(CONDITION_PUSH);
	//}

	if (m_fCallMedic < engine->Time())
	{
		if (GetHealthPercent() < 0.5)
		{
			m_fCallMedic = engine->Time() + RandomFloat(10.0f, 30.0f);

			CallMedic();
		}
	}

	if ((m_fUseTeleporterTime < engine->Time()) && !HasFlag() && m_pNearestTeleEntrance)
	{
		if (IsTeleporterUseful(m_pNearestTeleEntrance))
		{
			if (!m_pSchedules->IsCurrentSchedule(SCHED_USE_TELE))
			{
				m_pSchedules->FreeMemory();
				m_pSchedules->AddFront(new CBotUseTeleSched(m_pNearestTeleEntrance));

				m_fUseTeleporterTime = engine->Time() + RandomFloat(25.0f, 35.0f);
				return;
			}
		}
	}


	// Check redundant tasks
	if (!HasSomeConditions(CONDITION_NEED_AMMO) && m_pSchedules->IsCurrentSchedule(SCHED_TF2_GET_AMMO))
	{
		m_pSchedules->RemoveSchedule(SCHED_TF2_GET_AMMO);
	}

	if (!HasSomeConditions(CONDITION_NEED_HEALTH) && m_pSchedules->IsCurrentSchedule(SCHED_TF2_GET_HEALTH))
	{
		m_pSchedules->RemoveSchedule(SCHED_TF2_GET_HEALTH);
	}

	CheckHealingValid();

	if (m_bInitAlive)
	{
		Vector vOrigin = GetOrigin();
		CWaypoint *pWpt = CWaypoints::GetWaypoint(CWaypoints::NearestWaypointGoal(CWaypointTypes::W_FL_TELE_ENTRANCE, vOrigin, 4096, m_iTeam));

		if (pWpt)
		{
			// Get the nearest waypoint outside spawn (flagged as a teleporter entrance)
			// useful for Engineers and medics who want to camp for players
			m_vTeleportEntrance = pWpt->GetOrigin();
			m_bEntranceVectorValid = true;
		}
	}

	if (m_fLastSeeEnemy && ((m_fLastSeeEnemy + 5.0) < engine->Time()))
	{
		m_fLastSeeEnemy = 0;
		m_pButtons->Tap(IN_RELOAD);
	}

	if (!CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) && (m_pNearestPipeGren.Get() != NULL))
	{
		if (!m_pSchedules->HasSchedule(SCHED_GOOD_HIDE_SPOT) && (DistanceFrom(m_pNearestPipeGren) < BLAST_RADIUS))
		{
			CGotoHideSpotSched *pSchedule = new CGotoHideSpotSched(this, m_pNearestPipeGren.Get(), true);

			m_pSchedules->AddFront(pSchedule);
		}
	}

	/*if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
	{
		if (CClassInterface::TF2_MVMMinPlayersToReady())
		{
			int readycount = 0;
			for (int i = 1; i <= MAX_PLAYERS; i++)
			{
				edict_t *pClient = INDEXENT(i);
				if (!pClient->IsFree())
				{
					IPlayerInfo *pl = playerhelpers->GetGamePlayer(pClient)->GetPlayerInfo();
					if (pl && pl->IsConnected() && !pl->IsFakeClient())
					{
						if (CClassInterface::TF2_MVMIsPlayerReady(i))
							readycount++;
					}
				}

				CBot *pBot = CBots::Get(pClient);
				if (pBot->InUse())
				{
					if (readycount > 0)
						helpers->ClientCommand(pClient, "tournament_player_readystate 1");
					else
						helpers->ClientCommand(pClient, "tournament_player_readystate 0");
				}
			}
		}
	}*/
}


bool CBotFortress::IsTeleporterUseful(edict_t *pTele)
{
	edict_t *pExit = CTeamFortress2Mod::GetTeleporterExit(pTele);

	if (pExit)
	{
		if (!CTeamFortress2Mod::IsTeleporterSapped(pTele) && !CTeamFortress2Mod::IsTeleporterSapped(pExit) && !CClassInterface::IsObjectBeingBuilt(pExit) && !CClassInterface::IsObjectBeingBuilt(pTele) && !CClassInterface::IsObjectCarried(pExit) && !CClassInterface::IsObjectCarried(pTele))
		{
			float fEntranceDist = DistanceFrom(pTele);
			Vector vExit = CBotGlobals::EntityOrigin(pExit);
			Vector vEntrance = CBotGlobals::EntityOrigin(pTele);

			int countplayers = CBotGlobals::CountTeamMatesNearOrigin(vEntrance, 80.0f, m_iTeam, m_pEdict);

			int iCurWpt = (m_pNavigator->GetCurrentWaypointID() == -1) ? CWaypointLocations::NearestWaypoint(GetOrigin(), 200.0f, -1, true) : m_pNavigator->GetCurrentWaypointID();
			int iTeleWpt = CTeamFortress2Mod::GetTeleporterWaypoint(pExit);

			int iGoalId = m_pNavigator->GetCurrentGoalID();

			float fGoalDistance = ((iCurWpt == -1) || (iGoalId == -1)) ? ((m_vGoal - vEntrance).Length() + fEntranceDist) : CWaypointDistances::GetDistance(iCurWpt, iGoalId);
			float fTeleDistance = ((iTeleWpt == -1) || (iGoalId == -1)) ? ((m_vGoal - vExit).Length() + fEntranceDist) : CWaypointDistances::GetDistance(iTeleWpt, iGoalId);

			// still need to run to entrance then from exit
			if ((fEntranceDist + fTeleDistance) < fGoalDistance)// ((m_vGoal - CBotGlobals::entityOrigin(pExit)).Length()+distanceFrom(pTele)) < fGoalDistance )
			{
				// now check wait time based on how many players are waiting for the teleporter
				float fMaxSpeed = CClassInterface::GetMaxSpeed(m_pEdict);
				float fDuration = CClassInterface::GetTF2TeleRechargeDuration(pTele);

				edict_t *pOwner = CTeamFortress2Mod::GetBuildingOwner(ENGI_TELE, ENTINDEX(pTele));
				float fRunTime;
				float fWaitTime;
				float fTime = engine->Time();

				float fTeleRechargeTime = CTeamFortress2Mod::GetTeleportTime(pOwner);

				fWaitTime = (fEntranceDist / fMaxSpeed);

				// Teleporter never been used before and is ready to teleport
				if (fTeleRechargeTime > 0)
				{
					fWaitTime = (fEntranceDist / fMaxSpeed) + MAX(0, (fTeleRechargeTime - fTime) + fDuration);

					if (countplayers > 0)
						fWaitTime += fDuration * countplayers;
				}

				// see if i can run it myself in this time
				fRunTime = fGoalDistance / fMaxSpeed;

				return (fWaitTime < fRunTime);
			}
		}
	}

	return false;
}

void CBotFortress::ChooseClass()
{
	float fClassFitness[10];
	float fTotalFitness = 0;
	float fRandom;

	int iNumMedics = 0;
	int i = 0;
	int iTeam = GetTeam();
	int iClass;
	edict_t *pPlayer;

	for (i = 1; i < 10; i++)
		fClassFitness[i] = 1.0f;

	if ((m_iClass >= 0) && (m_iClass < 10))
		fClassFitness[m_iClass] = 0.1f;

	for (i = 1; i <= MAX_PLAYERS; i++)
	{
		pPlayer = INDEXENT(i);

		if (CBotGlobals::EntityIsValid(pPlayer) && (CTeamFortress2Mod::GetTeam(pPlayer) == iTeam))
		{
			iClass = CClassInterface::GetTF2Class(pPlayer);

			if (iClass == TF_CLASS_MEDIC)
				iNumMedics++;

			if ((iClass >= 0) && (iClass < 10))
				fClassFitness[iClass] *= 0.6f;
		}
	}

	if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
	{
		fClassFitness[TF_CLASS_ENGINEER] *= 1.5;
		fClassFitness[TF_CLASS_SPY] *= 0.6;
		fClassFitness[TF_CLASS_SCOUT] *= 0.6;
		fClassFitness[TF_CLASS_HWGUY] *= 1.5;
		fClassFitness[TF_CLASS_MEDIC] *= 1.1;
		fClassFitness[TF_CLASS_SOLDIER] *= 1.5;
		fClassFitness[TF_CLASS_DEMOMAN] *= 1.4;
		fClassFitness[TF_CLASS_PYRO] *= 1.6;
		// attacking team?
	}
	else if (CTeamFortress2Mod::IsAttackDefendMap())
	{
		if (GetTeam() == TF2_TEAM_BLUE)
		{
			fClassFitness[TF_CLASS_ENGINEER] *= 0.75;
			fClassFitness[TF_CLASS_SPY] *= 1.25;
			fClassFitness[TF_CLASS_SCOUT] *= 1.05;
		}
		else
		{
			fClassFitness[TF_CLASS_ENGINEER] *= 2.0;
			fClassFitness[TF_CLASS_SCOUT] *= 0.5;
			fClassFitness[TF_CLASS_HWGUY] *= 1.5;
			fClassFitness[TF_CLASS_MEDIC] *= 1.1;
		}
	}
	else if (CTeamFortress2Mod::IsMapType(TF_MAP_CP))
		fClassFitness[TF_CLASS_SCOUT] *= 1.2f;

	if (m_pLastEnemySentry.Get() != NULL)
	{
		fClassFitness[TF_CLASS_SPY] *= 1.25;
		fClassFitness[TF_CLASS_DEMOMAN] *= 1.3;
	}

	if (iNumMedics == 0)
		fClassFitness[TF_CLASS_MEDIC] *= 2.0f;


	for (int i = 1; i < 10; i++)
		fTotalFitness += fClassFitness[i];

	fRandom = RandomFloat(0, fTotalFitness);

	fTotalFitness = 0;

	m_iDesiredClass = 0;

	for (int i = 1; i < 10; i++)
	{
		fTotalFitness += fClassFitness[i];

		if (fRandom <= fTotalFitness)
		{
			m_iDesiredClass = i;
			break;
		}
	}
}

void CBotFortress::SelectTeam()
{
	char buffer[32];

	int team;
	if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
	{
		team = 1;
	}
	else
	{
		int numRed = CTeamFortress2Mod::NumPlayersOnTeam(TF2_TEAM_RED);
		int numBlu = CTeamFortress2Mod::NumPlayersOnTeam(TF2_TEAM_BLUE);
		if (numRed < numBlu)
			team = 1;
		else if (numRed > numBlu)
			team = 2;
		else
			team = RandomInt(1, 2);
	}

	sprintf(buffer, "jointeam %d", team);

	helpers->ClientCommand(m_pEdict, buffer);
}

void CBotFortress::SelectClass()
{
	char buffer[32];
	TFClass _class;

	if (m_iDesiredClass == 0)
		_class = (TFClass)RandomInt(1, 9);
	else
		_class = (TFClass)m_iDesiredClass;

	/*if (CBotGlobals::NumClients() > 3)
	{
		if (_class != TF_CLASS_MEDIC)
		{
			if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
			{
				if (CTeamFortress2Mod::NumClassOnTeam(TF2_TEAM_RED, (int)TF_CLASS_MEDIC) < 1)
					_class = TF_CLASS_MEDIC;
			}
			else
			{
				if (CTeamFortress2Mod::NumClassOnTeam(CClassInterface::GetTeam(m_pEdict), (int)TF_CLASS_MEDIC) < 2)
					_class = TF_CLASS_MEDIC;
			}
		}
		else if (_class != TF_CLASS_ENGINEER)
		{
			if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
			{
				if (CTeamFortress2Mod::NumClassOnTeam(TF2_TEAM_RED, (int)TF_CLASS_ENGINEER) < 1)
					_class = TF_CLASS_ENGINEER;
			}
			else if (CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CP))
			{
				if (CClassInterface::GetTeam(m_pEdict) == TF2_TEAM_RED)
					if (CTeamFortress2Mod::NumClassOnTeam(TF2_TEAM_RED, (int)TF_CLASS_ENGINEER) < 3)
						_class = TF_CLASS_ENGINEER;
			}
			else
			{
				if (CTeamFortress2Mod::NumClassOnTeam(CClassInterface::GetTeam(m_pEdict), (int)TF_CLASS_ENGINEER) < RandomInt(1, 2))
					_class = TF_CLASS_ENGINEER;
			}
		}
		else if (_class != TF_CLASS_SCOUT)
		{
			if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
			{
				if (CTeamFortress2Mod::NumClassOnTeam(TF2_TEAM_RED, (int)TF_CLASS_SCOUT) < 1)
					_class = TF_CLASS_SCOUT;
			}
			else if (CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CP))
			{
				if (CClassInterface::GetTeam(m_pEdict) == TF2_TEAM_BLUE)
					if (CTeamFortress2Mod::NumClassOnTeam(TF2_TEAM_BLUE, (int)TF_CLASS_SCOUT) < 2)
						_class = TF_CLASS_SCOUT;
			}
			else
			{
				if (CTeamFortress2Mod::NumClassOnTeam(CClassInterface::GetTeam(m_pEdict), (int)TF_CLASS_SCOUT) < RandomInt(1, 2))
					_class = TF_CLASS_SCOUT;
			}
		}
	}*/

	m_iClass = _class;

	if (_class == TF_CLASS_SCOUT)
	{
		sprintf(buffer, "joinclass scout");
	}
	else if (_class == TF_CLASS_ENGINEER)
	{
		sprintf(buffer, "joinclass engineer");
	}
	else if (_class == TF_CLASS_DEMOMAN)
	{
		sprintf(buffer, "joinclass demoman");
	}
	else if (_class == TF_CLASS_SOLDIER)
	{
		sprintf(buffer, "joinclass soldier");
	}
	else if (_class == TF_CLASS_HWGUY)
	{
		sprintf(buffer, "joinclass heavyweapons");
	}
	else if (_class == TF_CLASS_MEDIC)
	{
		sprintf(buffer, "joinclass medic");
	}
	else if (_class == TF_CLASS_SPY)
	{
		sprintf(buffer, "joinclass spy");
	}
	else if (_class == TF_CLASS_PYRO)
	{
		sprintf(buffer, "joinclass pyro");
	}
	else
	{
		sprintf(buffer, "joinclass sniper");
	}
	helpers->ClientCommand(m_pEdict, buffer);

	m_fChangeClassTime = engine->Time() + RandomFloat(bot_min_cc_time.GetFloat(), bot_max_cc_time.GetFloat());
}

bool CBotFortress::WaitForFlag(Vector *vOrigin, float *fWait, bool bFindFlag)
{
	// job calls!
	if (SomeoneCalledMedic())
		return false;

	if (SeeFlag(false) != NULL)
	{
		edict_t *m_pFlag = SeeFlag(false);

		if (CBotGlobals::EntityIsValid(m_pFlag))
		{
			LookAtEdict(m_pFlag);
			SetLookAtTask(LOOK_EDICT);
			*vOrigin = CBotGlobals::EntityOrigin(m_pFlag);
			*fWait = engine->Time() + 5.0f;
		}
		else
			SeeFlag(true);
	}
	else
		SetLookAtTask(LOOK_AROUND);

	if (DistanceFrom(*vOrigin) > 48)
		SetMoveTo(*vOrigin);
	else
	{
		if (!bFindFlag && ((GetClass() == TF_CLASS_SPY) && IsDisguised()))
		{
			if (!CTeamFortress2Mod::IsFlagCarried(m_iTeam))
				PrimaryAttack();
		}

		StopMoving();
	}

	return true;
}

void CBotFortress::FoundSpy(edict_t *pEdict, TFClass iDisguise)
{
	m_pPrevSpy = pEdict;
	m_fSeeSpyTime = engine->Time() + RandomFloat(9.0f, 18.0f);
	m_vLastSeeSpy = CBotGlobals::EntityOrigin(pEdict);
	m_fLastSeeSpyTime = engine->Time();
	if (iDisguise && (m_iPrevSpyDisguise != iDisguise))
		m_iPrevSpyDisguise = iDisguise;

	//m_fFirstSeeSpy = engine->Time(); // to do, add delayed action
};

// got shot by someone
bool CBotTF2::Hurt(edict_t *pAttacker, int iHealthNow, bool bDontHide)
{
	extern ConVar bot_spy_runaway_health;

	if (!pAttacker)
		return false;

	if ((m_iClass != TF_CLASS_MEDIC) || (!m_pHeal))
	{
		if (CBot::Hurt(pAttacker, iHealthNow, true))
		{
			if (m_bIsBeingHealed || m_bCanBeUbered)
			{
				// don't hide if I am being healed or can be ubered

				if (m_bCanBeUbered && !HasFlag()) // hack
					m_nextVoicecmd = TF_VC_ACTIVATEUBER;
			}
			else if (!bDontHide)
			{
				if (WantToNest())
				{
					CBotSchedule *pSchedule = new CBotSchedule();

					pSchedule->SetID(SCHED_GOOD_HIDE_SPOT);

					// run at flank while shooting	
					CFindPathTask *pHideGoalPoint = new CFindPathTask();
					Vector vOrigin = CBotGlobals::EntityOrigin(pAttacker);


					pSchedule->AddTask(new CFindGoodHideSpot(vOrigin));
					pSchedule->AddTask(pHideGoalPoint);
					pSchedule->AddTask(new CBotNest());

					// no interrupts, should be a quick waypoint path anyway
					pHideGoalPoint->SetNoInterruptions();
					// get vector from good hide spot task
					pHideGoalPoint->GetPassedVector();
					pHideGoalPoint->SetInterruptFunction(new CBotTF2CoverInterrupt());


					m_pSchedules->RemoveSchedule(SCHED_GOOD_HIDE_SPOT);
					m_pSchedules->AddFront(pSchedule);
				}
				else
				{
					m_pSchedules->RemoveSchedule(SCHED_GOOD_HIDE_SPOT);
					m_pSchedules->AddFront(new CGotoHideSpotSched(this, m_vHurtOrigin, new CBotTF2CoverInterrupt()));
				}

				if (CBotGlobals::IsPlayer(pAttacker) && (m_iClass == TF_CLASS_SPY) && (iHealthNow < bot_spy_runaway_health.GetInt()))
				{
					// cloak and run
					if (!IsCloaked())
					{
						SpyCloak();
						// hide and find health
						//updateCondition(CONDITION_CHANGED);				
						WantToShoot(false);
						m_fFrenzyTime = 0.0f;
						if (HasEnemy())
							SetLastEnemy(m_pEnemy);
						m_pEnemy = NULL; // reset enemy
					}
				}
			}

			return true;
		}
	}

	if (pAttacker)
	{
		if ((m_iClass == TF_CLASS_SPY) && !IsCloaked() && !CTeamFortress2Mod::IsSentry(pAttacker, CTeamFortress2Mod::GetEnemyTeam(m_iTeam)))
		{

			// TO DO : make sure I'm not just caught in crossfire
			// search for other team members
			if (!m_StatsCanUse.stats.m_iTeamMatesVisible || !m_StatsCanUse.stats.m_iTeamMatesInRange)
				m_fFrenzyTime = engine->Time() + RandomFloat(2.0f, 6.0f);

			if (IsDisguised())
				DetectedAsSpy(pAttacker, true);

			if (CBotGlobals::IsPlayer(pAttacker) && (iHealthNow<bot_spy_runaway_health.GetInt()) && (CClassInterface::GetTF2SpyCloakMeter(m_pEdict) > 0.3f))
			{
				// cloak and run
				SpyCloak();
				// hide and find health
				m_pSchedules->RemoveSchedule(SCHED_GOOD_HIDE_SPOT);
				m_pSchedules->AddFront(new CGotoHideSpotSched(this, m_vHurtOrigin, new CBotTF2CoverInterrupt()));
				WantToShoot(false);
				m_fFrenzyTime = 0.0f;

				if (HasEnemy())
					SetLastEnemy(m_pEnemy);

				m_pEnemy = NULL; // reset enemy

				return true;
			}
		}
	}

	return false;
}


/////////////////////////////////////////////////////////////////////////////
// TEAM FORTRESS 2

void CBotTF2::SpawnInit()
{
	CBotFortress::SpawnInit();

	m_iDesiredResistType = 0;
	m_pMelee = NULL;
	m_pPrimary = NULL;
	m_pSecondary = NULL;
	m_fUseBuffItemTime = 0.0f;
	//m_bHatEquipped = false;
	m_iTrapCPIndex = -1;
	m_pHealer = NULL;
	m_fCallMedic = engine->Time() + 10.0f;
	m_fCarryTime = 0.0f;

	m_bIsCarryingTeleExit = false;
	m_bIsCarryingSentry = false;
	m_bIsCarryingDisp = false;
	m_bIsCarryingTeleEnt = false;
	m_bIsCarryingObj = false;

	m_nextVoicecmd = TF_VC_INVALID;
	m_fAttackPointTime = 0.0f;
	m_fNextRevMiniGunTime = 0.0f;
	m_fRevMiniGunTime = 0.0f;

	m_pCloakedSpy = NULL;

	m_fRemoveSapTime = 0.0f;

	// stickies destroyed now
	m_iTrapType = TF_TRAP_TYPE_NONE;

	//m_fBlockPushTime = 0.0f;

	m_iTeam = GetTeam();
	// update current areas
	UpdateAttackDefendPoints();
	//CPoints::GetAreas(m_iTeam,&m_iCurrentDefendArea,&m_iCurrentAttackArea);

	m_fDoubleJumpTime = 0.0f;
	m_fFrenzyTime = 0.0f;
	m_fUseTeleporterTime = 0.0f;
	m_fSpySapTime = 0.0f;

	//m_pPushPayloadBomb = NULL;
	//m_pDefendPayloadBomb = NULL;

	m_iPrevWeaponSelectFailed = 0;

	m_fCheckNextCarrying = 0.0;
}

// return true if we don't want to hang around on the point
bool CBotTF2::CheckAttackPoint()
{
	if (CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE))
	{
		m_fAttackPointTime = engine->Time() + RandomFloat(5.0f, 15.0f);
		return true;
	}

	return false;
}

void CBotTF2::SetClass(TFClass _class)
{
	m_iClass = _class;
}

void CBotTF2::HighFivePlayer(edict_t *pPlayer, float fYaw)
{
	if (!m_pSchedules->IsCurrentSchedule(SCHED_TAUNT))
		m_pSchedules->AddFront(new CBotTauntSchedule(pPlayer, fYaw));
}

// bOverride will be true in messaround mode
void CBotTF2::Taunt(bool bOverride)
{
	extern ConVar bot_taunt;
	// haven't taunted for a while, no emeny, not ubered, OK! Taunt!
	if (bOverride || (!m_bHasFlag && bot_taunt.GetBool() && !CTeamFortress2Mod::TF2_IsPlayerOnFire(m_pEdict) && !m_pEnemy && (m_fTauntTime < engine->Time()) && (!CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict))))
	{
		helpers->ClientCommand(m_pEdict, "taunt");
		m_fTauntTime = engine->Time() + RandomFloat(40.0, 100.0); // Don't taunt for another minute or two
		m_fTaunting = engine->Time() + 5.0;
	}
}

void CBotTF2::HealedPlayer(edict_t *pPlayer, float fAmount)
{
	if (m_iClass == TF_CLASS_ENGINEER) // my dispenser was used
	{
		m_fDispenserHealAmount += fAmount;
	}
}
// useful for quick check of mod entities (especially ones which I need to find quickly
// e.g. sentry gun -- if I dont see it quickly it might kill me
edict_t *CBotFortress::GetVisibleSpecial()
{
	static edict_t *pPlayer;
	static edict_t *pReturn;

	// this is a special visible which will return something important 
	// that should be visible quickly, e.g. an enemy sentry gun
	// or teleporter entrance for bots to make decisions quickly
	if ((signed int)m_iSpecialVisibleId >= MAX_PLAYERS)
		m_iSpecialVisibleId = 0;

	pPlayer = INDEXENT(m_iSpecialVisibleId + 1);
	pReturn = NULL;

	if (pPlayer && (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_ENGINEER))
	{
		edict_t *pSentry = CTeamFortress2Mod::GetSentryGun(m_iSpecialVisibleId);

		// more interested in enemy sentries to sap and shoot!
		pReturn = pSentry;

		if (CClassInterface::GetTeam(pPlayer) == m_iTeam)
		{
			// more interested in teleporters on my team
			edict_t *pTele = CTeamFortress2Mod::GetTeleEntrance(m_iSpecialVisibleId);

			if (pTele)
				pReturn = pTele;

			if (GetClass() == TF_CLASS_ENGINEER)
			{
				// be a bit more random with engi's as they both might be important
				// to repair other team members sentries!
				if (!pTele || RandomInt(0, 1))
					pReturn = pSentry;
			}

		}
	}

	m_iSpecialVisibleId++;

	return pReturn;
}
/*
lambda-
NEW COMMAND SYNTAX:
- "build 2 0" - Build sentry gun
- "build 0 0" - Build dispenser
- "build 1 0" - Build teleporter entrance
- "build 1 1" - Build teleporter exit
*/
void CBotTF2::EngineerBuild(eEngiBuild iBuilding, eEngiCmd iEngiCmd)
{
	//char buffer[16];
	//char cmd[256];

	if (iEngiCmd == ENGI_BUILD)
	{
		//strcpy(buffer,"build");

		switch (iBuilding)
		{
		case ENGI_DISP:
			//m_pDispenser = NULL;
			helpers->ClientCommand(m_pEdict, "build 0 0");
			break;
		case ENGI_SENTRY:
			//m_pSentryGun = NULL;
			helpers->ClientCommand(m_pEdict, "build 2 0");
			break;
		case ENGI_ENTRANCE:
			//m_pTeleEntrance = NULL;
			helpers->ClientCommand(m_pEdict, "build 1 0");
			break;
		case ENGI_EXIT:
			//m_pTeleExit = NULL;
			helpers->ClientCommand(m_pEdict, "build 1 1");
			break;
		default:
			return;
			break;
		}
	}
	else
	{
		//strcpy(buffer,"destroy");

		switch (iBuilding)
		{
		case ENGI_DISP:
			m_pDispenser = NULL;
			m_bDispenserVectorValid = false;
			m_iDispenserArea = 0;
			helpers->ClientCommand(m_pEdict, "destroy 0 0");
			break;
		case ENGI_SENTRY:
			m_pSentryGun = NULL;
			m_bSentryGunVectorValid = false;
			m_iSentryArea = 0;
			helpers->ClientCommand(m_pEdict, "destroy 2 0");
			break;
		case ENGI_ENTRANCE:
			m_pTeleEntrance = NULL;
			m_bEntranceVectorValid = false;
			m_iTeleEntranceArea = 0;
			helpers->ClientCommand(m_pEdict, "destroy 1 0");
			break;
		case ENGI_EXIT:
			m_pTeleExit = NULL;
			m_bTeleportExitVectorValid = false;
			m_iTeleExitArea = 0;
			helpers->ClientCommand(m_pEdict, "destroy 1 1");
			break;
		default:
			return;
			break;
		}
	}

	//sprintf(cmd,"%s %d 0",buffer,iBuilding); // added extra value to end

	//helpers->ClientCommand(m_pEdict,cmd);
}

void CBotTF2::UpdateCarrying()
{
	if ((m_bIsCarryingObj = CClassInterface::IsCarryingObj(m_pEdict)) == true)
	{
		edict_t *pCarriedObj = CClassInterface::GetCarriedObj(m_pEdict);

		if (pCarriedObj)
		{
			m_bIsCarryingTeleExit = CTeamFortress2Mod::IsTeleporterExit(pCarriedObj, m_iTeam, true);
			m_bIsCarryingSentry = CTeamFortress2Mod::IsSentry(pCarriedObj, m_iTeam, true);
			m_bIsCarryingDisp = CTeamFortress2Mod::IsDispenser(pCarriedObj, m_iTeam, true);
			m_bIsCarryingTeleEnt = CTeamFortress2Mod::IsTeleporterEntrance(pCarriedObj, m_iTeam, true);
		}
	}
	else
	{
		m_bIsCarryingTeleExit = NULL;
		m_bIsCarryingSentry = NULL;
		m_bIsCarryingDisp = NULL;
		m_bIsCarryingTeleEnt = NULL;
	}
}

void CBotTF2::CheckBuildingsValid(bool bForce) // force check carrying
{
	if (m_pSentryGun)
	{
		if (!CBotGlobals::EntityIsValid(m_pSentryGun) || !CBotGlobals::EntityIsAlive(m_pSentryGun) || !CTeamFortress2Mod::IsSentry(m_pSentryGun, m_iTeam))
		{
			m_pSentryGun = NULL;
			m_prevSentryHealth = 0;
			m_iSentryArea = 0;
		}
		else if (CClassInterface::GetSentryEnemy(m_pSentryGun) != NULL)
			m_fLastSentryEnemyTime = engine->Time();
	}

	if (m_pDispenser)
	{
		if (!CBotGlobals::EntityIsValid(m_pDispenser) || !CBotGlobals::EntityIsAlive(m_pDispenser) || !CTeamFortress2Mod::IsDispenser(m_pDispenser, m_iTeam))
		{
			m_pDispenser = NULL;
			m_prevDispHealth = 0;
			m_iDispenserArea = 0;
		}
	}

	if (m_pTeleEntrance)
	{
		if (!CBotGlobals::EntityIsValid(m_pTeleEntrance) || !CBotGlobals::EntityIsAlive(m_pTeleEntrance) || !CTeamFortress2Mod::IsTeleporterEntrance(m_pTeleEntrance, m_iTeam))
		{
			m_pTeleEntrance = NULL;
			m_prevTeleEntHealth = 0;
			m_iTeleEntranceArea = 0;
		}
	}

	if (m_pTeleExit)
	{
		if (!CBotGlobals::EntityIsValid(m_pTeleExit) || !CBotGlobals::EntityIsAlive(m_pTeleExit) || !CTeamFortress2Mod::IsTeleporterExit(m_pTeleExit, m_iTeam))
		{
			m_pTeleExit = NULL;
			m_prevTeleExtHealth = 0;
			m_iTeleExitArea = 0;
		}
	}
}

// Find the EDICT_T of the building that the engineer just built...
edict_t *CBotTF2::FindEngineerBuiltObject(eEngiBuild iBuilding, int index)
{
	int team = GetTeam();

	edict_t *pBest = INDEXENT(index);

	if (pBest)
	{
		if (iBuilding == ENGI_TELE)
		{
			if (CTeamFortress2Mod::IsTeleporterEntrance(pBest, team))
				iBuilding = ENGI_ENTRANCE;
			else if (CTeamFortress2Mod::IsTeleporterExit(pBest, team))
				iBuilding = ENGI_EXIT;
		}

		switch (iBuilding)
		{
		case ENGI_DISP:
			m_pDispenser = pBest;
			break;
		case ENGI_ENTRANCE:
			m_pTeleEntrance = pBest;
			//m_vTeleportEntrance = CBotGlobals::EntityOrigin(pBest);
			//m_bEntranceVectorValid = true;
			break;
		case ENGI_EXIT:
			m_pTeleExit = pBest;
			break;
		case ENGI_SENTRY:
			m_pSentryGun = pBest;
			break;
		default:
			return NULL;
		}
	}

	return pBest;
}

void CBotTF2::Died(edict_t *pKiller, const char *pszWeapon)
{
	CBotFortress::Died(pKiller, pszWeapon);

	if (pKiller)
	{
		if (CBotGlobals::EntityIsValid(pKiller))
		{
			m_pNavigator->Belief(CBotGlobals::EntityOrigin(pKiller), GetEyePosition(), bot_beliefmulti.GetFloat(), DistanceFrom(pKiller), BELIEF_DANGER);

			if (!strncmp(pszWeapon, "obj_sentrygun", 13) || !strncmp(pszWeapon, "obj_minisentry", 14))
				m_pLastEnemySentry = CTeamFortress2Mod::GetMySentryGun(pKiller);
		}
	}
}

void CBotTF2::Killed(edict_t *pVictim, char *weapon)
{
	CBotFortress::Killed(pVictim, weapon);

	if ((m_pSentryGun.Get() != NULL) && (m_iClass == TF_CLASS_ENGINEER) && weapon && *weapon && (strncmp(weapon, "obj_sentry", 10) == 0))
	{
		m_iSentryKills++;

		if (pVictim && CBotGlobals::EntityIsValid(pVictim))
		{
			Vector vSentry = CBotGlobals::EntityOrigin(m_pSentryGun);
			Vector vVictim = CBotGlobals::EntityOrigin(pVictim);

			m_pNavigator->Belief(vVictim, vSentry, bot_beliefmulti.GetFloat(), (vSentry - vVictim).Length(), BELIEF_SAFETY);
		}
	}
	else if (pVictim && CBotGlobals::EntityIsValid(pVictim))
		m_pNavigator->Belief(CBotGlobals::EntityOrigin(pVictim), GetEyePosition(), bot_beliefmulti.GetFloat(), DistanceFrom(pVictim), BELIEF_SAFETY);

	if (CBotGlobals::IsPlayer(pVictim) && (CClassInterface::GetTF2Class(pVictim) == TF_CLASS_SPY))
	{
		if (m_pPrevSpy == pVictim)
		{
			m_pPrevSpy = NULL;
			m_fLastSeeSpyTime = 0.0f;
		}

		RemoveCondition(CONDITION_PARANOID);
	}

	Taunt();
}

void CBotTF2::CapturedFlag()
{
	Taunt();
}

void CBotTF2::SpyDisguise(int iTeam, int iClass)
{
	//char cmd[256];

	if (iTeam == 3)
		m_iImpulse = 230 + iClass;
	else if (iTeam == 2)
		m_iImpulse = 220 + iClass;

	m_fDisguiseTime = engine->Time();
	m_iDisguiseClass = iClass;
	m_fFrenzyTime = 0.0f; // reset frenzy time

	//moooo

	//sprintf(cmd,"disguise %d %d",iClass,iTeam);

	//helpers->ClientCommand(m_pEdict,cmd);
}
// Test
bool CBotTF2::IsCloaked()
{
	return CTeamFortress2Mod::TF2_IsPlayerCloaked(m_pEdict);
}
// Test
bool CBotTF2::IsDisguised()
{
	int _class, _team, _index, _health;

	if (CClassInterface::GetTF2SpyDisguised(m_pEdict, &_class, &_team, &_index, &_health))

	{
		if (_class > 0)
			return true;
	}

	return false;

}

void CBotTF2::UpdateClass()
{
	if (m_fUpdateClass && (m_fUpdateClass < engine->Time()))
	{


		m_fUpdateClass = 0;
	}
}

TFClass CBotTF2::GetClass()
{
	return m_iClass;
}

void CBotTF2::Setup()
{
	CBotFortress::Setup();
}

void CBotTF2::SeeFriendlyKill(edict_t *pTeamMate, edict_t *pDied, CWeapon *pWeapon)
{
	if (CBotGlobals::IsPlayer(pDied) && CClassInterface::GetTF2Class(pDied) == TF_CLASS_SPY)
	{
		if (m_pPrevSpy == pDied)
		{
			m_pPrevSpy = NULL;
			m_fLastSeeSpyTime = 0.0f;
		}

		RemoveCondition(CONDITION_PARANOID);
	}
}


void CBotTF2::SeeFriendlyDie(edict_t *pDied, edict_t *pKiller, CWeapon *pWeapon)
{
	if (pKiller && !m_pEnemy && !HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
	{
		//if ( pWeapon )
		//{
		//	DOD_Class pclass = (DOD_Class)CClassInterface::getPlayerClassDOD(pKiller);

		if (pWeapon && (pWeapon->GetID() == TF2_WEAPON_SENTRYGUN))
		{
			AddVoiceCommand(TF_VC_SENTRYAHEAD);
			UpdateCondition(CONDITION_COVERT);
			m_fCurrentDanger += 100.0f;
			m_pLastEnemySentry = CTeamFortress2Mod::GetMySentryGun(pKiller);
			m_vLastDiedOrigin = CBotGlobals::EntityOrigin(pDied);
			m_pLastEnemySentry = pKiller;

			if ((m_iClass == TF_CLASS_DEMOMAN) || (m_iClass == TF_CLASS_SPY))
			{
				// perhaps pipe it!
				UpdateCondition(CONDITION_CHANGED);
			}
		}
		else
		{
			AddVoiceCommand(TF_VC_INCOMING);
			UpdateCondition(CONDITION_COVERT);
			m_fCurrentDanger += 50.0f;
		}

		//}

		// encourage bots to snoop out enemy or throw grenades
		m_fLastSeeEnemy = engine->Time();
		m_pLastEnemy = pKiller;
		m_fLastUpdateLastSeeEnemy = 0;
		m_vLastSeeEnemy = CBotGlobals::EntityOrigin(m_pLastEnemy);
		m_vLastSeeEnemyBlastWaypoint = m_vLastSeeEnemy;

		CWaypoint *pWpt = CWaypoints::GetWaypoint(CWaypointLocations::NearestBlastWaypoint(m_vLastSeeEnemy, GetOrigin(), 4096.0, -1, true, true, false, false, 0, false));

		if (pWpt)
			m_vLastSeeEnemyBlastWaypoint = pWpt->GetOrigin();

		UpdateCondition(CONDITION_CHANGED);
	}
}


void CBotTF2::EngiBuildSuccess(eEngiBuild iBuilding, int index)
{
	edict_t *pEntity = FindEngineerBuiltObject(iBuilding, index);

	if (iBuilding == ENGI_SENTRY)
	{
		m_fSentryPlaceTime = engine->Time();
		m_iSentryKills = 0;
	}
	else if (iBuilding == ENGI_DISP)
	{
		m_fDispenserPlaceTime = engine->Time();
		m_fDispenserHealAmount = 0.0f;
	}
	else if (iBuilding == ENGI_TELE)
	{
		if (CTeamFortress2Mod::IsTeleporterEntrance(pEntity, m_iTeam))
		{
			m_fTeleporterEntPlacedTime = engine->Time();

			//if ( m_pTeleExit.get() != NULL ) // already has exit built
			m_iTeleportedPlayers = 0;
		}
		else if (CTeamFortress2Mod::IsTeleporterExit(pEntity, m_iTeam))
		{
			m_fTeleporterExtPlacedTime = engine->Time();

			if (m_pTeleEntrance.Get() == NULL) // doesn't have entrance built
				m_iTeleportedPlayers = 0;
		}
	}
}

bool CBotTF2::HasEngineerBuilt(eEngiBuild iBuilding)
{
	switch (iBuilding)
	{
	case ENGI_SENTRY:
		return m_pSentryGun != NULL; // TODO
		break;
	case ENGI_DISP:
		return m_pDispenser != NULL; // TODO
		break;
	case ENGI_ENTRANCE:
		return m_pTeleEntrance != NULL; // TODO
		break;
	case ENGI_EXIT:
		return m_pTeleExit != NULL; // TODO
		break;
	}

	return false;
}

// ENEMY Flag dropped
void CBotFortress::FlagDropped(Vector vOrigin)
{
	m_vLastKnownFlagPoint = vOrigin;
	m_fLastKnownFlagTime = engine->Time() + 60.0f;

	if (m_pSchedules->HasSchedule(SCHED_RETURN_TO_INTEL))
		m_pSchedules->RemoveSchedule(SCHED_RETURN_TO_INTEL);
}

void CBotFortress::TeamFlagDropped(Vector vOrigin)
{
	m_vLastKnownTeamFlagPoint = vOrigin;

	if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
		m_fLastKnownTeamFlagTime = engine->Time() + 1800.0f; // its going to stay there
	else
		m_fLastKnownTeamFlagTime = engine->Time() + 60.0f;

	// FIX
	if (m_pSchedules->HasSchedule(SCHED_TF2_GET_FLAG))
		m_pSchedules->RemoveSchedule(SCHED_TF2_GET_FLAG);
}

void CBotFortress::CallMedic()
{
	helpers->ClientCommand(m_pEdict, "saveme");
}

bool CBotTF2::CanGotoWaypoint(Vector vPrevWaypoint, CWaypoint *pWaypoint, CWaypoint *pPrev)
{
	static edict_t *pSentry;

	if (CBot::CanGotoWaypoint(vPrevWaypoint, pWaypoint, pPrev))
	{
		if (pWaypoint->HasFlag(CWaypointTypes::W_FL_OWNER_ONLY))
		{
			int area = pWaypoint->GetArea();
			int capindex = CTeamFortress2Mod::m_ObjectiveResource.m_WaypointAreaToIndexTranslation[area];

			if (area && (CTeamFortress2Mod::m_ObjectiveResource.GetOwningTeam(capindex) != m_iTeam))
				return false;
		}

		if (pWaypoint->HasFlag(CWaypointTypes::W_FL_AREAONLY))
		{
			if (!CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(pWaypoint->GetArea(), pWaypoint->GetFlags()))
				return false;
		}

		if (pWaypoint->HasFlag(CWaypointTypes::W_FL_ROCKET_JUMP))
		{
			CBotWeapons *pWeapons = GetWeapons();
			CBotWeapon *pWeapon;

			// only roccket jump if more than 50% health
			if (GetHealthPercent() > 0.5)
			{
				// only soldiers or demomen can use these
				if (GetClass() == TF_CLASS_SOLDIER)
				{
					pWeapon = pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_ROCKETLAUNCHER));

					if (pWeapon)
						return (pWeapon->GetAmmo(this) > 0);
				}
				else if ((GetClass() == TF_CLASS_DEMOMAN) && bot_demo_jump.GetBool())
				{
					pWeapon = pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_PIPEBOMBS));

					if (pWeapon)
						return (pWeapon->GetClip1(this) > 0);
				}
			}

			return false;
		}

		if (pWaypoint->HasFlag(CWaypointTypes::W_FL_DOUBLEJUMP))
		{
			return (GetClass() == TF_CLASS_SCOUT);
		}

		if (pWaypoint->HasFlag(CWaypointTypes::W_FL_WAIT_GROUND))
		{
			return pWaypoint->CheckGround();
		}

		if (pWaypoint->HasFlag(CWaypointTypes::W_FL_NO_FLAG))
		{
			return (!HasFlag());
		}

		if (pWaypoint->HasFlag(CWaypointTypes::W_FL_FLAGONLY))
		{
			return HasFlag();
		}

		pSentry = NULL;

		if (m_iClass == TF_CLASS_ENGINEER)
			pSentry = m_pSentryGun.Get();
		else if (m_iClass == TF_CLASS_SPY)
			pSentry = m_pNearestEnemySentry.Get();

		if (pSentry != NULL)
		{
			Vector vWaypoint = pWaypoint->GetOrigin();
			Vector vWptMin = vWaypoint.Min(vPrevWaypoint) - Vector(32, 32, 32);
			Vector vWptMax = vWaypoint.Max(vPrevWaypoint) + Vector(32, 32, 32);

			Vector vSentry = CBotGlobals::EntityOrigin(pSentry);
			Vector vMax = vSentry + pSentry->GetCollideable()->OBBMaxs();
			Vector vMin = vSentry + pSentry->GetCollideable()->OBBMins();

			if (vSentry.WithinAABox(vWptMin, vWptMax))
			{
				Vector vSentryComp = vSentry - vPrevWaypoint;

				float fSentryDist = vSentryComp.Length();
				//float fWaypointDist = pWaypoint->distanceFrom(vPrevWaypoint);

				//if ( fSentryDist < fWaypointDist )
				//{
				Vector vComp = pWaypoint->GetOrigin() - vPrevWaypoint;

				vComp = vPrevWaypoint + ((vComp / vComp.Length()) * fSentryDist);

				// Path goes through sentry -- can't walk through here
				if (vComp.WithinAABox(vMin, vMax))
					return false;
				//}
			}

			// check if waypoint goes through sentry (can't walk through)

		}

		if (CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE))
		{
			if (m_pRedPayloadBomb.Get() != NULL)
			{
				edict_t *pSentry = m_pRedPayloadBomb.Get();
				// check path doesn't go through pay load bomb

				Vector vWaypoint = pWaypoint->GetOrigin();
				Vector vWptMin = vWaypoint.Min(vPrevWaypoint) - Vector(32, 32, 32);
				Vector vWptMax = vWaypoint.Max(vPrevWaypoint) + Vector(32, 32, 32);

				Vector vSentry = CBotGlobals::EntityOrigin(pSentry);
				Vector vMax = vSentry + Vector(32, 32, 32);
				Vector vMin = vSentry - Vector(32, 32, 32);

				if (vSentry.WithinAABox(vWptMin, vWptMax))
				{
					Vector vSentryComp = vSentry - vPrevWaypoint;

					float fSentryDist = vSentryComp.Length();
					//float fWaypointDist = pWaypoint->distanceFrom(vPrevWaypoint);

					//if ( fSentryDist < fWaypointDist )
					//{
					Vector vComp = pWaypoint->GetOrigin() - vPrevWaypoint;

					vComp = vPrevWaypoint + ((vComp / vComp.Length()) * fSentryDist);

					// Path goes through sentry -- can't walk through here
					if (vComp.WithinAABox(vMin, vMax))
						return false;
					//}
				}
			}

			if (m_pBluePayloadBomb.Get() != NULL)
			{
				edict_t *pSentry = m_pBluePayloadBomb.Get();
				Vector vWaypoint = pWaypoint->GetOrigin();
				Vector vWptMin = vWaypoint.Min(vPrevWaypoint) - Vector(32, 32, 32);
				Vector vWptMax = vWaypoint.Max(vPrevWaypoint) + Vector(32, 32, 32);

				Vector vSentry = CBotGlobals::EntityOrigin(pSentry);
				Vector vMax = vSentry + Vector(32, 32, 32);
				Vector vMin = vSentry - Vector(32, 32, 32);

				if (vSentry.WithinAABox(vWptMin, vWptMax))
				{
					Vector vSentryComp = vSentry - vPrevWaypoint;

					float fSentryDist = vSentryComp.Length();
					//float fWaypointDist = pWaypoint->distanceFrom(vPrevWaypoint);

					//if ( fSentryDist < fWaypointDist )
					//{
					Vector vComp = pWaypoint->GetOrigin() - vPrevWaypoint;

					vComp = vPrevWaypoint + ((vComp / vComp.Length()) * fSentryDist);

					// Path goes through sentry -- can't walk through here
					if (vComp.WithinAABox(vMin, vMax))
						return false;
					//}
				}
			}
		}

		return true;
	}
	else if (pWaypoint->HasFlag(CWaypointTypes::W_FL_FALL))
	{
		return (GetClass() == TF_CLASS_SCOUT);
	}

	return false;
}

void CBotTF2::CallMedic()
{
	AddVoiceCommand(TF_VC_MEDIC);
}

void CBotFortress::WaitCloak()
{
	m_fSpyCloakTime = engine->Time() + RandomFloat(2.0f, 6.0f);
}

bool CBotFortress::WantToCloak()
{
	extern ConVar bot_tf2_debug_spies_cloakdisguise;

	if (bot_tf2_debug_spies_cloakdisguise.GetBool())
	{
		if ((m_fFrenzyTime < engine->Time()) && (!m_pEnemy || !HasSomeConditions(CONDITION_SEE_CUR_ENEMY)))
		{
			return ((!m_bStatsCanUse || (m_StatsCanUse.stats.m_iEnemiesVisible>0)) && (CClassInterface::GetTF2SpyCloakMeter(m_pEdict) > 90.0f) && (m_fCurrentDanger > TF2_SPY_CLOAK_BELIEF));
		}
	}

	return false;
}

bool CBotFortress::WantToUnCloak()
{
	if (WantToShoot() && m_pEnemy && HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
	{
		// hopefully the enemy can't see me
		if (CBotGlobals::IsAlivePlayer(m_pEnemy) && (fabs(CBotGlobals::YawAngleFromEdict(m_pEnemy, GetOrigin())) > bot_spyknifefov.GetFloat()))
			return true;
		else if (!m_pEnemy || !HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
			return (m_fCurrentDanger < 1.0f);
	}

	return (m_bStatsCanUse && (m_StatsCanUse.stats.m_iEnemiesVisible == 0));
}


void CBotTF2::SpyUnCloak()
{
	if (CTeamFortress2Mod::TF2_IsPlayerCloaked(m_pEdict) && (m_fSpyCloakTime < engine->Time()))
	{
		SecondaryAttack();

		m_fSpyCloakTime = engine->Time() + RandomFloat(2.0f, 4.0f);
		//m_fSpyCloakTime = m_fSpyUncloakTime;
	}
}

void CBotTF2::SpyCloak()
{
	if (!CTeamFortress2Mod::TF2_IsPlayerCloaked(m_pEdict) && (m_fSpyCloakTime < engine->Time()))
	{
		m_fSpyCloakTime = engine->Time() + RandomFloat(2.0f, 4.0f);
		//m_fSpyUncloakTime = m_fSpyCloakTime;

		SecondaryAttack();
	}
}

void CBotFortress::UpdateConditions()
{
	CBot::UpdateConditions();

	if (CTeamFortress2Mod::HasRoundStarted() && CTeamFortress2Mod::WithinEndOfRound(29.0f))
		UpdateCondition(CONDITION_PUSH);

	if (m_iClass == TF_CLASS_ENGINEER)
	{
		if (CTeamFortress2Mod::IsMySentrySapped(GetEdict()) || CTeamFortress2Mod::IsMyTeleporterSapped(GetEdict()) || CTeamFortress2Mod::IsMyDispenserSapped(GetEdict()))
		{
			UpdateCondition(CONDITION_BUILDING_SAPPED);
			UpdateCondition(CONDITION_PARANOID);

			/*if ( (m_fLastSeeSpyTime + 10.0f) > engine->Time() )
			{
			m_vLastSeeSpy = m_vSentryGun;
			m_fLastSeeSpyTime = engine->Time();
			}*/
		}
		else
			RemoveCondition(CONDITION_BUILDING_SAPPED);
	}
}


void CBotTF2::ModThink()
{
	static bool bNeedHealth;
	static bool bNeedAmmo;
	static bool bIsCloaked;

	if (CTeamFortress2Mod::IsLosingTeam(m_iTeam))
		WantToShoot(false);
	//if ( m_pWeapons ) // done in bot.cpp 
	//	m_pWeapons->Update(false); // don't override ammo types from engine

	bNeedHealth = HasSomeConditions(CONDITION_NEED_HEALTH) && !m_bIsBeingHealed;
	bNeedAmmo = HasSomeConditions(CONDITION_NEED_AMMO);

	// mod specific think code here
	CBotFortress::ModThink();

	CheckBeingHealed();

	if (WantToListen())
	{
		if ((m_pNearestAllySentry.Get() != NULL) && (CClassInterface::GetSentryEnemy(m_pNearestAllySentry) != NULL))
		{
			m_PlayerListeningTo = m_pNearestAllySentry;
			m_bListenPositionValid = true;
			m_fListenTime = engine->Time() + RandomFloat(1.0f, 2.0f);
			SetLookAtTask(LOOK_NOISE);
			m_fLookSetTime = m_fListenTime;
			m_vListenPosition = CBotGlobals::EntityOrigin(m_pNearestAllySentry.Get());
		}
	}

	if (CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE))
	{
		if (GetTeam() == TF2_TEAM_BLUE)
		{
			m_pDefendPayloadBomb = m_pRedPayloadBomb;
			m_pPushPayloadBomb = m_pBluePayloadBomb;
		}
		else
		{
			m_pDefendPayloadBomb = m_pBluePayloadBomb;
			m_pPushPayloadBomb = m_pRedPayloadBomb;
		}
	}
	else if (CTeamFortress2Mod::IsMapType(TF_MAP_CART))
	{
		if (GetTeam() == TF2_TEAM_BLUE)
		{
			m_pPushPayloadBomb = m_pBluePayloadBomb;
			m_pDefendPayloadBomb = NULL;
		}
		else
		{
			m_pPushPayloadBomb = NULL;
			m_pDefendPayloadBomb = m_pBluePayloadBomb;
		}
	}

	// when respawned -- check if I should change class
	if (m_bCheckClass && !m_pPI->IsDead())
	{
		m_bCheckClass = false;

		if (bot_change_class.GetBool() && (m_fChangeClassTime < engine->Time()) && (!CTeamFortress2Mod::IsMapType(TF_MAP_MVM) || !CTeamFortress2Mod::HasRoundStarted()))
		{
			// get score for this class
			float scoreValue = CClassInterface::GetTF2Score(m_pEdict);

			if (m_iClass == TF_CLASS_ENGINEER)
			{
				if (m_pSentryGun.Get())
				{
					scoreValue *= 2.0f * CTeamFortress2Mod::GetSentryLevel(m_pSentryGun);
					scoreValue *= ((m_fLastSentryEnemyTime + 15.0f) < engine->Time()) ? 2.0f : 1.0f;
				}
				if (m_pTeleEntrance.Get() && m_pTeleExit.Get())
					scoreValue *= CTeamFortress2Mod::IsAttackDefendMap() ? 2.0f : 1.5f;
				if (m_pDispenser.Get())
					scoreValue *= 1.25f;
				// less chance of changing class if bot has these up
				m_fChangeClassTime = engine->Time() + RandomFloat(bot_min_cc_time.GetFloat() / 2, bot_max_cc_time.GetFloat() / 2);
			}

			// if I think I could do better
			if (RandomFloat(0.0f, 1.0f) > (scoreValue / CTeamFortress2Mod::GetHighestScore()))
			{
				ChooseClass(); // edits m_iDesiredClass

				// change class
				SelectClass();
			}
		}
	}

	m_fIdealMoveSpeed = CTeamFortress2Mod::TF2_GetPlayerSpeed(m_pEdict, m_iClass)*bot_speed_boost.GetFloat();


	/* spy check code */
	if (((m_iClass != TF_CLASS_SPY) || (!IsDisguised())) && ((m_pEnemy.Get() == NULL) || !HasSomeConditions(CONDITION_SEE_CUR_ENEMY)) && (m_pPrevSpy.Get() != NULL) && (m_fSeeSpyTime > engine->Time()) &&
		!m_bIsCarryingObj && CBotGlobals::IsAlivePlayer(m_pPrevSpy) && !CTeamFortress2Mod::TF2_IsPlayerInvuln(GetEdict()))
	{
		if ((m_iClass != TF_CLASS_ENGINEER) || !HasSomeConditions(CONDITION_BUILDING_SAPPED))
		{
			// check for spies within radius of bot / use aim skill as a skill factor
			float fPossibleDistance = (engine->Time() - m_fLastSeeSpyTime) *
				(m_pProfile->m_fAimSkill * 310.0f) * (m_fCurrentDanger / MAX_BELIEF);

			// increase distance for pyro, he can use flamethrower !
			if (m_iClass == TF_CLASS_PYRO)
				fPossibleDistance += 200.0f;

			if ((m_vLastSeeSpy - GetOrigin()).Length() < fPossibleDistance)
			{
				UpdateCondition(CONDITION_PARANOID);

				if (m_pNavigator->HasNextPoint() && !m_pSchedules->IsCurrentSchedule(SCHED_TF_SPYCHECK))
				{
					CBotSchedule *newSchedule = new CBotSchedule(new CSpyCheckAir());

					newSchedule->SetID(SCHED_TF_SPYCHECK);

					m_pSchedules->AddFront(newSchedule);
				}
			}
			else
				RemoveCondition(CONDITION_PARANOID);
		}
	}
	else
		RemoveCondition(CONDITION_PARANOID);

	if (HasFlag())
		RemoveCondition(CONDITION_COVERT);

	switch (m_iClass)
	{
	case TF_CLASS_SCOUT:
		if (m_pWeapons->HasWeapon(TF2_WEAPON_LUNCHBOX_DRINK))
		{
			int pcond = CClassInterface::GetTF2Conditions(m_pEdict);

			if ((pcond & TF2_PLAYER_BONKED) == TF2_PLAYER_BONKED)
			{
				WantToShoot(false);
				// clear enemy
				m_pEnemy = NULL;
				m_pOldEnemy = NULL;
			}
			else if (!HasEnemy() && !HasFlag() && (CClassInterface::TF2_GetEnergyDrinkMeter(m_pEdict) > 99.99f))
			{
				if (m_fCurrentDanger > 49.0f)
				{
					if (m_fUseBuffItemTime < engine->Time())
					{
						m_fUseBuffItemTime = engine->Time() + 20.0f;
						m_pSchedules->AddFront(new CBotSchedule(new CBotUseLunchBoxDrink()));
					}
				}
			}
		}

		break;
	case TF_CLASS_SNIPER:
		if (CTeamFortress2Mod::TF2_IsPlayerZoomed(m_pEdict))
		{
			m_fFov = 20.0f; // Jagger

			//if ( !wantToZoom() )

			if (MoveToIsValid() && !HasEnemy())
				SecondaryAttack();
		}
		else
			m_fFov = BOT_DEFAULT_FOV;

		break;
	case TF_CLASS_SOLDIER:
		if (m_pWeapons->HasWeapon(TF2_WEAPON_BUFF_ITEM))
		{
			if (CClassInterface::GetRageMeter(m_pEdict) > 99.99f)
			{
				if (m_fCurrentDanger > 99.0f)
				{
					if (m_fUseBuffItemTime < engine->Time())
					{
						m_fUseBuffItemTime = engine->Time() + 30.0f;
						m_pSchedules->AddFront(new CBotSchedule(new CBotUseBuffItem()));
					}
				}
			}
		}
		break;
	case TF_CLASS_DEMOMAN:
		if (m_iTrapType != TF_TRAP_TYPE_NONE)
		{
			if (m_pEnemy)
			{
				if ((CBotGlobals::EntityOrigin(m_pEnemy) - m_vStickyLocation).Length() < BLAST_RADIUS)
					DetonateStickies();
			}
		}
		break;
	case TF_CLASS_MEDIC:
		if (!HasFlag() && m_pHeal && CBotGlobals::EntityIsAlive(m_pHeal))
		{
			if (!m_pSchedules->HasSchedule(SCHED_HEAL))
			{
				m_pSchedules->FreeMemory();
				m_pSchedules->Add(new CBotTF2HealSched(m_pHeal));
			}

			WantToShoot(false);
		}
		break;
	case TF_CLASS_HWGUY:
	{
		bool bRevMiniGun;

		bRevMiniGun = false;

		// hwguys dont rev minigun if they have the flag
		if (WantToShoot() && !m_bHasFlag)
		{
			CBotWeapon *pWeapon = GetCurrentWeapon();

			if (pWeapon && (pWeapon->GetID() == TF2_WEAPON_MINIGUN))
			{
				if (!CTeamFortress2Mod::TF2_IsPlayerOnFire(m_pEdict) &&
					!CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict))
				{
					if (m_fCurrentDanger >= TF2_HWGUY_REV_BELIEF)
					{
						if (pWeapon->GetAmmo(this) > 100)
						{
							bRevMiniGun = true;
						}
					}
				}
			}
		}

		// Rev the minigun
		if (bRevMiniGun)
		{
			// record time when bot started revving up
			if (m_fRevMiniGunTime == 0)
			{
				float fMinTime = (m_fCurrentDanger / 200) * 10;

				m_fRevMiniGunTime = engine->Time();
				m_fNextRevMiniGunTime = RandomFloat(fMinTime, fMinTime + 5.0f);
			}

			// rev for 10 seconds
			if ((m_fRevMiniGunTime + m_fNextRevMiniGunTime) > engine->Time())
			{
				SecondaryAttack(true);
				//m_fIdealMoveSpeed = 30.0f; Improve Max Speed here

				if (m_fCurrentDanger < 1)
				{
					m_fRevMiniGunTime = 0.0f;
					m_fNextRevMiniGunTime = 0.0f;
				}
			}
			else if ((m_fRevMiniGunTime + (2.0f*m_fNextRevMiniGunTime)) < engine->Time())
			{
				m_fRevMiniGunTime = 0.0;
			}
		}

		if (m_pButtons->HoldingButton(IN_ATTACK) || m_pButtons->HoldingButton(IN_ATTACK2))
		{
			if (m_pButtons->HoldingButton(IN_JUMP))
				m_pButtons->LetGo(IN_JUMP);
		}
	}

	break;
	case TF_CLASS_ENGINEER:

		CheckBuildingsValid(false);

		if (!m_pSchedules->HasSchedule(SCHED_REMOVESAPPER))
		{
			if ((m_fRemoveSapTime < engine->Time()) && m_pNearestAllySentry && CBotGlobals::EntityIsValid(m_pNearestAllySentry) && CTeamFortress2Mod::IsSentrySapped(m_pNearestAllySentry))
			{
				m_pSchedules->FreeMemory();
				m_pSchedules->Add(new CBotRemoveSapperSched(m_pNearestAllySentry, ENGI_SENTRY));
				UpdateCondition(CONDITION_PARANOID);
			}
			else if ((m_fRemoveSapTime < engine->Time()) && m_pSentryGun && CBotGlobals::EntityIsValid(m_pSentryGun) && CTeamFortress2Mod::IsSentrySapped(m_pSentryGun))
			{
				if (DistanceFrom(m_pSentryGun) < 1024.0f) // only go back if I can remove the sapper
				{
					m_pSchedules->FreeMemory();
					m_pSchedules->Add(new CBotRemoveSapperSched(m_pSentryGun, ENGI_SENTRY));
					UpdateCondition(CONDITION_PARANOID);
				}
			}
		}
		break;
	case TF_CLASS_SPY:
		if (!HasFlag())
		{
			extern ConVar bot_tf2_debug_spies_cloakdisguise;

			if (bot_tf2_debug_spies_cloakdisguise.GetBool() && (m_fSpyDisguiseTime < engine->Time()))
			{
				// if previously detected or isn't disguised
				if ((m_fDisguiseTime == 0.0f) || !IsDisguised())
				{
					int iTeam = CTeamFortress2Mod::GetEnemyTeam(GetTeam());

					SpyDisguise(iTeam, GetSpyDisguiseClass(iTeam));
				}

				m_fSpyDisguiseTime = engine->Time() + 5.0f;
			}

			bIsCloaked = CTeamFortress2Mod::TF2_IsPlayerCloaked(m_pEdict);

			if (bIsCloaked && WantToUnCloak())
			{
				SpyUnCloak();
			}
			else if (!bIsCloaked && WantToCloak())
			{
				SpyCloak();
			}
			else if (bIsCloaked || IsDisguised() && !HasEnemy())
			{
				UpdateCondition(CONDITION_COVERT);
			}
			/*else if ( bIsCloaked && m_pEnemy && )
			{
			if ( CClassInterface::GetTF2SpyCloakMeter(m_pEdict) <
			}*/


			if (m_pNearestEnemySentry && (m_fSpySapTime < engine->Time()) && !CTeamFortress2Mod::IsSentrySapped(m_pNearestEnemySentry) && !m_pSchedules->HasSchedule(SCHED_SPY_SAP_BUILDING))
			{
				m_fSpySapTime = engine->Time() + RandomFloat(1.0f, 4.0f);
				m_pSchedules->FreeMemory();
				m_pSchedules->Add(new CBotSpySapBuildingSched(m_pNearestEnemySentry, ENGI_SENTRY));
			}
			else if (m_pNearestEnemyTeleporter && (m_fSpySapTime < engine->Time()) && !CTeamFortress2Mod::IsTeleporterSapped(m_pNearestEnemyTeleporter) && !m_pSchedules->HasSchedule(SCHED_SPY_SAP_BUILDING))
			{
				m_fSpySapTime = engine->Time() + RandomFloat(1.0f, 4.0f);
				m_pSchedules->FreeMemory();
				m_pSchedules->Add(new CBotSpySapBuildingSched(m_pNearestEnemyTeleporter, ENGI_TELE));
			}
		}
		break;
	default:
		break;
	}

	// look for tasks / more important tasks here

	if (!HasSomeConditions(CONDITION_SEE_CUR_ENEMY) && !m_bLookedForEnemyLast && m_pLastEnemy && CBotGlobals::EntityIsValid(m_pLastEnemy) && CBotGlobals::EntityIsAlive(m_pLastEnemy))
	{
		if (WantToFollowEnemy())
		{
			m_pSchedules->FreeMemory();
			m_pSchedules->AddFront(new CBotFollowLastEnemy(this, m_pLastEnemy, m_vLastSeeEnemy));
			m_bLookedForEnemyLast = true;
		}
	}

	if (m_fTaunting > engine->Time())
	{
		m_pButtons->LetGoAllButtons(true);
		SetMoveLookPriority(MOVELOOK_OVERRIDE);
		StopMoving();
		SetMoveLookPriority(MOVELOOK_MODTHINK);
	}
	else if (m_fDoubleJumpTime && (m_fDoubleJumpTime < engine->Time()))
	{
		TapButton(IN_JUMP);
		m_fDoubleJumpTime = 0;
	}

	if (m_pSchedules->IsCurrentSchedule(SCHED_GOTO_ORIGIN) && (m_fPickupTime < engine->Time()) && (bNeedHealth || bNeedAmmo) && (!m_pEnemy && !HasSomeConditions(CONDITION_SEE_CUR_ENEMY)))
	{
		if ((m_fPickupTime < engine->Time()) && m_pNearestDisp && !m_pSchedules->IsCurrentSchedule(SCHED_USE_DISPENSER))
		{
			if (fabs(CBotGlobals::EntityOrigin(m_pNearestDisp).z - GetOrigin().z) < BOT_JUMP_HEIGHT)
			{
				m_pSchedules->RemoveSchedule(SCHED_USE_DISPENSER);
				m_pSchedules->AddFront(new CBotUseDispSched(this, m_pNearestDisp));

				m_fPickupTime = engine->Time() + RandomFloat(6.0f, 20.0f);
				return;
			}
		}
		else if ((m_fPickupTime < engine->Time()) && bNeedHealth && m_pHealthkit && !m_pSchedules->IsCurrentSchedule(SCHED_TF2_GET_HEALTH))
		{
			if (fabs(CBotGlobals::EntityOrigin(m_pHealthkit).z - GetOrigin().z) < BOT_JUMP_HEIGHT)
			{
				m_pSchedules->RemoveSchedule(SCHED_TF2_GET_HEALTH);
				m_pSchedules->AddFront(new CBotTF2GetHealthSched(CBotGlobals::EntityOrigin(m_pHealthkit)));

				m_fPickupTime = engine->Time() + RandomFloat(5.0f, 10.0f);

				return;
			}

		}
		else if ((m_fPickupTime < engine->Time()) && bNeedAmmo && m_pAmmo && !m_pSchedules->IsCurrentSchedule(SCHED_PICKUP))
		{
			if (fabs(CBotGlobals::EntityOrigin(m_pAmmo).z - GetOrigin().z) < BOT_JUMP_HEIGHT)
			{
				m_pSchedules->RemoveSchedule(SCHED_TF2_GET_AMMO);
				m_pSchedules->AddFront(new CBotTF2GetAmmoSched(CBotGlobals::EntityOrigin(m_pAmmo)));

				m_fPickupTime = engine->Time() + RandomFloat(5.0f, 10.0f);

				return;
			}
		}
	}

	SetMoveLookPriority(MOVELOOK_MODTHINK);

}

void CBotTF2::HandleWeapons()
{
	if (m_iClass == TF_CLASS_ENGINEER)
	{
		edict_t *pSentry = m_pSentryGun.Get();

		if (m_bIsCarryingObj)
		{
			// don't shoot while carrying object unless after 5 seconds of carrying
			if ((GetHealthPercent() > 0.9f) || ((m_fCarryTime + 3.0f) > engine->Time()))
				return;
		}
		else if ((pSentry != NULL) && !HasSomeConditions(CONDITION_PARANOID))
		{
			if (IsVisible(pSentry) && (CClassInterface::GetSentryEnemy(pSentry) != NULL))
			{
				if (DistanceFrom(pSentry) < 100)
					return; // don't shoot -- i probably want to upgrade sentry
			}
		}

	}

	//
	// Handle attacking at this point
	//
	if (m_pEnemy && !HasSomeConditions(CONDITION_ENEMY_DEAD) &&
		HasSomeConditions(CONDITION_SEE_CUR_ENEMY) && WantToShoot() &&
		IsVisible(m_pEnemy) && IsEnemy(m_pEnemy))
	{
		CBotWeapon *pWeapon;

		pWeapon = m_pWeapons->GetBestWeapon(m_pEnemy, !HasFlag(), !HasFlag(), bot_melee_only.GetBool(), false, CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict));

		SetLookAtTask(LOOK_ENEMY);

		m_pAttackingEnemy = NULL;

		if (m_bWantToChangeWeapon && (pWeapon != NULL) && (pWeapon != GetCurrentWeapon()) && pWeapon->GetWeaponIndex())
		{
			//SelectWeaponSlot(pWeapon->GetWeaponInfo()->GetSlot());
			Select_CWeapon(pWeapon->GetWeaponInfo());
			//SelectWeapon(pWeapon->GetWeaponIndex());
		}
		else
		{
			if (!HandleAttack(pWeapon, m_pEnemy))
			{
				m_pEnemy = NULL;
				m_pOldEnemy = NULL;
				WantToShoot(false);
			}
		}
	}
}

void CBotTF2::EnemyFound(edict_t *pEnemy)
{
	CBotFortress::EnemyFound(pEnemy);
	m_fRevMiniGunTime = 0.0f;

	if (m_pNearestEnemySentry == pEnemy)
	{
		CBotWeapon *pWeapon = m_pWeapons->GetPrimaryWeapon();

		if ((pWeapon != NULL) && (m_iClass != TF_CLASS_SPY) && !pWeapon->OutOfAmmo(this) && pWeapon->PrimaryGreaterThanRange(TF2_MAX_SENTRYGUN_RANGE + 32.0f))
		{
			UpdateCondition(CONDITION_CHANGED);
		}
	}
}

bool CBotFortress::CanAvoid(edict_t *pEntity)
{
	return CBot::CanAvoid(pEntity);
}

bool CBotTF2::CanAvoid(edict_t *pEntity)
{
	extern ConVar bot_avoid_radius;

	float distance;
	Vector vAvoidOrigin;
	int index;

	if (!CBotGlobals::EntityIsValid(pEntity))
		return false;
	if (pEntity == m_pLookEdict)
		return false;
	if (m_pEdict == pEntity) // can't avoid self!!!!
		return false;
	if (pEntity == m_pLastEnemy)
		return false;
	if (pEntity == m_pTeleEntrance)
		return false;
	if (pEntity == m_pNearestTeleEntrance)
		return false;
	if (pEntity == m_pNearestDisp)
		return false;
	if (pEntity == m_pHealthkit)
		return false;
	if (pEntity == m_pAmmo)
		return false;
	if ((pEntity == m_pSentryGun) && (CClassInterface::IsObjectCarried(pEntity)))
		return false;
	if ((pEntity == m_pDispenser) && (CClassInterface::IsObjectCarried(pEntity)))
		return false;
	if ((pEntity == m_pTeleExit) && (CClassInterface::IsObjectCarried(pEntity)))
		return false;

	edict_t *groundEntity = CClassInterface::GetGroundEntity(m_pEdict);

	// must stand on worldspawn
	if (groundEntity && (ENTINDEX(groundEntity) > 0) && (pEntity == groundEntity))
	{
		if (pEntity == m_pSentryGun)
			return true;
		if (pEntity == m_pDispenser)
			return true;
	}

	index = ENTINDEX(pEntity);

	if (!index)
		return false;

	vAvoidOrigin = CBotGlobals::EntityOrigin(pEntity);

	if (vAvoidOrigin == m_vMoveTo)
		return false;

	distance = DistanceFrom(vAvoidOrigin);

	if ((distance > 1) && (distance < bot_avoid_radius.GetFloat()) && (vAvoidOrigin.z >= GetOrigin().z) && (fabs(GetOrigin().z - vAvoidOrigin.z) < 64))
	{
		if ((m_pAttackingEnemy.Get() != NULL) && (m_pAttackingEnemy.Get() == pEntity))
			return false; // I need to melee this guy probably
		else if (IsEnemy(pEntity, false))
			return true;
		else if ((m_iClass == TF_CLASS_ENGINEER) && ((pEntity == m_pSentryGun.Get()) || (pEntity == m_pDispenser.Get())))
			return true;
	}

	return false;
}

bool CBotTF2::WantToInvestigateSound()
{
	if (!CBot::WantToInvestigateSound())
		return false;
	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict))
		return false;

	return (m_fLastSeeEnemy + 8.0f < engine->Time()) && !m_bHasFlag && ((m_iClass != TF_CLASS_ENGINEER) ||
		(!this->m_bIsCarryingObj && (m_pSentryGun.Get() != NULL) && ((CTeamFortress2Mod::GetSentryLevel(m_pSentryGun)>2) && (CClassInterface::GetSentryHealth(m_pSentryGun) > 90))));
}

bool CBotTF2::WantToListenToPlayerFootsteps(edict_t *pPlayer)
{
	switch (CClassInterface::GetTF2Class(pPlayer))
	{
	case TF_CLASS_MEDIC:
	{
		if (m_pHealer.Get() == pPlayer)
			return false;
	}
	break;
	case TF_CLASS_SPY:
	{
		// don't listen to cloaked spies
		if (CTeamFortress2Mod::TF2_IsPlayerCloaked(pPlayer))
			return false;
	}
	break;

	default:
		break;
	}

	return true;
}

bool CBotTF2::WantToListenToPlayerAttack(edict_t *pPlayer, int iWeaponID)
{
	static edict_t *pWeapon;
	static const char *szWeaponClassname;

	pWeapon = CClassInterface::GetCurrentWeapon(pPlayer);

	if (!pWeapon)
		return true;

	szWeaponClassname = pWeapon->GetClassName();

	switch (CClassInterface::GetTF2Class(pPlayer))
	{
		case TF_CLASS_MEDIC:
		{
			// don't listen to mediguns
			if (!strcmp("medigun", &szWeaponClassname[10]))
				return false;
			break;
		}
	
		case TF_CLASS_ENGINEER:
		{
			// don't listen to engis upgrading stuff
			if (!strcmp("wrench", &szWeaponClassname[10]))
				return false;
			else if (!strcmp("builder", &szWeaponClassname[10]))
				return false;
			break;
		}
	
		case TF_CLASS_SPY:
		{
			// don't listen to cloaked spies
			if (CTeamFortress2Mod::TF2_IsPlayerCloaked(pPlayer))
				return false;
			if (!strcmp("knife", &szWeaponClassname[10]))
			{
				// only hear spy knives if they know there are spies around
				// in 15% of cases
				return HasSomeConditions(CONDITION_PARANOID) && (RandomFloat(0.0f, 1.0f) <= 0.15f);
			}
			break;
		}

		default:
			break;
	}

	return true;
}

void CBotTF2::CheckStuckonSpy(void)
{
	edict_t *pPlayer;
	edict_t *pStuck = NULL;

	int i = 0;
	int iTeam = GetTeam();

	float fDistance;
	float fMaxDistance = 80;

	for (i = 1; i < MAX_PLAYERS; i++)
	{
		pPlayer = INDEXENT(i);

		if (pPlayer == m_pEdict)
			continue;

		if (CBotGlobals::EntityIsValid(pPlayer) && CBotGlobals::EntityIsAlive(pPlayer) && (CTeamFortress2Mod::GetTeam(pPlayer) != iTeam))
		{
			if ((fDistance = DistanceFrom(pPlayer)) < fMaxDistance) // touching distance
			{
				if (IsVisible(pPlayer))
				{
					fMaxDistance = fDistance;
					pStuck = pPlayer;
				}
			}
		}
	}

	if (pStuck)
	{
		if (CClassInterface::GetTF2Class(pStuck) == TF_CLASS_SPY)
		{
			FoundSpy(pStuck, CTeamFortress2Mod::GetSpyDisguise(pStuck));
		}

		if ((m_iClass == TF_CLASS_SPY) && IsDisguised())
		{
			// Doh! found me!
			if (RandomFloat(0.0f, 100.0f) < GetHealthPercent())
				m_fFrenzyTime = engine->Time() + RandomFloat(0.0f, GetHealthPercent());

			DetectedAsSpy(pStuck, false);
			return;
		}
	}
}

bool CBotFortress::IsClassOnTeam(int iClass, int iTeam)
{
	int i = 0;
	edict_t *pPlayer;

	for (i = 1; i < MAX_PLAYERS; i++)
	{
		pPlayer = INDEXENT(i);

		if (CBotGlobals::EntityIsValid(pPlayer) && (CTeamFortress2Mod::GetTeam(pPlayer) == iTeam))
		{
			if (CClassInterface::GetTF2Class(pPlayer) == iClass)
				return true;
		}
	}

	return false;
}

bool CBotTF2::WantToFollowEnemy()
{
	edict_t *pEnemy = m_pLastEnemy.Get();

	if (CTeamFortress2Mod::IsLosingTeam(CTeamFortress2Mod::GetEnemyTeam(m_iTeam)))
		return true;
	else if ((m_iClass == TF_CLASS_SCOUT) && ((CClassInterface::GetTF2Conditions(m_pEdict)&TF2_PLAYER_BONKED) == TF2_PLAYER_BONKED))
		return false; // currently can't shoot 
	else if (!WantToInvestigateSound()) // maybe capturing point right now
		return false;
	else if ((pEnemy != NULL) && CBotGlobals::IsPlayer(pEnemy) && CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) && (m_iClass != TF_CLASS_MEDIC))
		return true; // I am ubered  GO!!!
	else if ((pEnemy != NULL) && CBotGlobals::IsPlayer(pEnemy) && CTeamFortress2Mod::TF2_IsPlayerInvuln(pEnemy))
		return false; // Enemy is UBERED  -- don't follow
	else if ((m_iCurrentDefendArea != 0) && (pEnemy != NULL) && CTeamFortress2Mod::IsMapType(TF_MAP_CP) && (CTeamFortress2Mod::m_ObjectiveResource.GetNumControlPoints() > 0))
	{
		Vector vDefend = CTeamFortress2Mod::m_ObjectiveResource.GetCPPosition(CTeamFortress2Mod::m_ObjectiveResource.m_WaypointAreaToIndexTranslation[m_iCurrentDefendArea]);

		Vector vEnemyOrigin = CBotGlobals::EntityOrigin(pEnemy);
		Vector vOrigin = GetOrigin();

		// He's trying to cap the point? Maybe! Go after him!
		if (((vDefend - vEnemyOrigin).Length() + 80.0f) < (vDefend - vOrigin).Length())
		{
			UpdateCondition(CONDITION_DEFENSIVE);
			return true;
		}
	}
	else if ((m_fLastKnownTeamFlagTime > 0) && (pEnemy != NULL) && (CTeamFortress2Mod::IsMapType(TF_MAP_CTF) || CTeamFortress2Mod::IsMapType(TF_MAP_MVM)))
	{
		Vector vDefend = m_vLastKnownTeamFlagPoint;

		Vector vEnemyOrigin = CBotGlobals::EntityOrigin(pEnemy);
		Vector vOrigin = GetOrigin();

		// He's trying to get the flag? Maybe! Go after him!
		if (((vDefend - vEnemyOrigin).Length() + 80.0f) < (vDefend - vOrigin).Length())
		{
			UpdateCondition(CONDITION_DEFENSIVE);
			return true;
		}
	}

	return CBotFortress::WantToFollowEnemy();
}

bool CBotFortress::WantToFollowEnemy()
{
	if (HasSomeConditions(CONDITION_NEED_HEALTH))
		return false;
	if (HasSomeConditions(CONDITION_NEED_AMMO))
		return false;
	if (!CTeamFortress2Mod::HasRoundStarted())
		return false;
	if (!m_pLastEnemy)
		return false;
	if (HasFlag())
		return false;
	if (m_iClass == TF_CLASS_SCOUT)
		return false;
	else if ((m_iClass == TF_CLASS_MEDIC) && m_pHeal)
		return false;
	else if ((m_iClass == TF_CLASS_SPY) && IsDisguised()) // sneak around the enemy
		return true;
	else if ((m_iClass == TF_CLASS_SNIPER) && (DistanceFrom(m_pLastEnemy) > CWaypointLocations::REACHABLE_RANGE))
		return false; // don't bother!
	else if (CBotGlobals::IsPlayer(m_pLastEnemy) && (CClassInterface::GetTF2Class(m_pLastEnemy) == TF_CLASS_SPY) && (ThinkSpyIsEnemy(m_pLastEnemy, CTeamFortress2Mod::GetSpyDisguise(m_pLastEnemy))))
		return true; // always find spies!
	else if (CTeamFortress2Mod::IsFlagCarrier(m_pLastEnemy))
		return true; // follow flag carriers to the death
	else if (m_iClass == TF_CLASS_ENGINEER)
		return false; // have work to do

	return CBot::WantToFollowEnemy();
}

void CBotTF2::VoiceCommand(int cmd)
{
	char scmd[64];
	u_VOICECMD vcmd;

	vcmd.voicecmd = cmd;

	sprintf(scmd, "voicemenu %d %d", vcmd.b1.v1, vcmd.b1.v2);

	helpers->ClientCommand(m_pEdict, scmd);
}

bool CBotTF2::CheckStuck(void)
{
	if (!CTeamFortress2Mod::IsAttackDefendMap() || (CTeamFortress2Mod::HasRoundStarted() || (GetTeam() == TF2_TEAM_RED)))
	{
		if (CBot::CheckStuck())
		{
			CheckStuckonSpy();

			return true;
		}
	}

	return false;
}

void CBotTF2::FoundSpy(edict_t *pEdict, TFClass iDisguise)
{
	CBotFortress::FoundSpy(pEdict, iDisguise);

	if (m_fLastSaySpy < engine->Time())
	{
		AddVoiceCommand(TF_VC_SPY);

		m_fLastSaySpy = engine->Time() + RandomFloat(10.0f, 40.0f);
	}
}

int CBotFortress::GetSpyDisguiseClass(int iTeam)
{
	int i = 0;
	edict_t *pPlayer;
	dataUnconstArray<int> m_classes;
	int _class;
	float fTotal;
	float fRand;

	for (i = 1; i < MAX_PLAYERS; i++)
	{
		pPlayer = INDEXENT(i);

		if (CBotGlobals::EntityIsValid(pPlayer) && (CTeamFortress2Mod::GetTeam(pPlayer) == iTeam))
		{
			_class = CClassInterface::GetTF2Class(pPlayer);

			if (_class)
				m_classes.Add(_class);
		}
	}


	if (m_classes.IsEmpty())
		return RandomInt(1, 9);

	fTotal = 0;

	for (int i = 0; i < m_classes.Size(); i++)
	{
		fTotal += m_fClassDisguiseFitness[m_classes.ReturnValueFromIndex(i)];
	}

	if (fTotal > 0)
	{

		fRand = RandomFloat(0.0, fTotal);

		fTotal = 0;

		for (int i = 0; i < m_classes.Size(); i++)
		{
			fTotal += m_fClassDisguiseFitness[m_classes.ReturnValueFromIndex(i)];

			if (fRand <= fTotal)
				return m_classes.ReturnValueFromIndex(i);
		}

	}

	return m_classes.Random();
}

bool CBotFortress::IncomingRocket(float fRange)
{
	edict_t *pRocket = m_NearestEnemyRocket;

	if (pRocket != NULL)
	{
		Vector vel;
		Vector vorg = CBotGlobals::EntityOrigin(pRocket);
		Vector vcomp;
		float fDist = DistanceFrom(pRocket);

		if (fDist < fRange)
		{
			CClassInterface::GetVelocity(pRocket, &vel);

			vel = vel / vel.Length();
			vcomp = vorg + vel*fDist;

			return (DistanceFrom(vcomp) < BLAST_RADIUS);
		}
	}

	pRocket = m_pNearestPipeGren;

	if (pRocket)
	{
		Vector vel;
		Vector vorg = CBotGlobals::EntityOrigin(pRocket);
		Vector vcomp;
		float fDist = DistanceFrom(pRocket);

		if (fDist < fRange)
		{
			CClassInterface::GetVelocity(pRocket, &vel);

			if (vel.Length() > 0)
			{
				vel = vel / vel.Length();
				vcomp = vorg + vel*fDist;
			}
			else
				vcomp = vorg;

			return (DistanceFrom(vcomp) < BLAST_RADIUS);
		}
	}

	return false;
}

void CBotFortress::EnemyLost(edict_t *pEnemy)
{
	if (CBotGlobals::IsPlayer(pEnemy) && (CClassInterface::GetTF2Class(pEnemy) == TF_CLASS_SPY))
	{
		if (CBotGlobals::IsAlivePlayer(pEnemy))
		{
			UpdateCondition(CONDITION_CHANGED);
		}
	}

	//CBot::enemyLost(pEnemy);
}

bool CBotTF2::SetVisible(edict_t *pEntity, bool bVisible)
{
	bool bValid = CBotFortress::SetVisible(pEntity, bVisible);

	if (bValid)
	{
		if ((m_pRedPayloadBomb.Get() == NULL) && CTeamFortress2Mod::IsPayloadBomb(pEntity, TF2_TEAM_RED))
		{
			m_pRedPayloadBomb = pEntity;
			CTeamFortress2Mod::UpdateRedPayloadBomb(pEntity);
			//if ( CTeamFortress2Mod::se
		}
		else if ((m_pBluePayloadBomb.Get() == NULL) && CTeamFortress2Mod::IsPayloadBomb(pEntity, TF2_TEAM_BLUE))
		{
			m_pBluePayloadBomb = pEntity;
			CTeamFortress2Mod::UpdateBluePayloadBomb(pEntity);
		}
	}

	if (bValid && bVisible)
	{
		edict_t *pTest;

		if (((pTest = m_NearestEnemyRocket.Get()) != pEntity) && CTeamFortress2Mod::IsRocket(pEntity, CTeamFortress2Mod::GetEnemyTeam(m_iTeam)))
		{
			if ((pTest == NULL) || (DistanceFrom(pEntity) < DistanceFrom(pTest)))
				m_NearestEnemyRocket = pEntity;
		}
	}
	else
	{
		if (pEntity == m_NearestEnemyRocket.Get_Old())
			m_NearestEnemyRocket = NULL;
	}

	if ((ENTINDEX(pEntity) <= MAX_PLAYERS) && (ENTINDEX(pEntity) > 0))
	{
		if (bVisible)
		{
			TFClass iPlayerclass = (TFClass)CClassInterface::GetTF2Class(pEntity);

			if (iPlayerclass == TF_CLASS_SPY)
			{
				// check if disguise is not spy on my team
				int iClass, iTeam, iIndex, iHealth;

				CClassInterface::GetTF2SpyDisguised(pEntity, &iClass, &iTeam, &iIndex, &iHealth);

				if (CTeamFortress2Mod::TF2_IsPlayerCloaked(pEntity))
				{
					if (iClass != TF_CLASS_SPY) // spies cloaking is normal / non spies cloaking is not!
					{
						if (!m_pCloakedSpy || ((m_pCloakedSpy != pEntity) && (DistanceFrom(pEntity) < DistanceFrom(m_pCloakedSpy))))
							m_pCloakedSpy = pEntity;
					}
				}
				else if ((iClass == TF_CLASS_MEDIC) && (iTeam == m_iTeam))
				{
					if (!m_pLastSeeMedic.Check(pEntity) && CBotGlobals::EntityIsAlive(pEntity))
					{
						// i think this spy can cure me!
						if ((m_pLastSeeMedic.Check(NULL) || (DistanceFrom(pEntity) < DistanceFrom(m_pLastSeeMedic.GetLocation()))) &&
							!ThinkSpyIsEnemy(pEntity, (TFClass)iClass))
						{
							m_pLastSeeMedic = CBotLastSee(pEntity);
						}
					}
					else
						m_pLastSeeMedic.Update();

				}
			}
			else if (iPlayerclass == TF_CLASS_MEDIC)
			{
				if (m_pLastSeeMedic.Check(pEntity))
				{
					m_pLastSeeMedic.Update();
				}
				else
				{
					if (CBotGlobals::EntityIsAlive(pEntity) && ((m_pLastSeeMedic.Check(NULL)) || (DistanceFrom(pEntity) < DistanceFrom(m_pLastSeeMedic.GetLocation()))))
					{
						m_pLastSeeMedic = CBotLastSee(pEntity);
					}

				}
			}
		}
		else
		{
			if (m_pCloakedSpy.Get_Old() == pEntity)
				m_pCloakedSpy = NULL;
		}
	}

	return bValid;

}

void CBotTF2::CheckBeingHealed()
{
	static edict_t *pPlayer;
	static edict_t *pWeapon;
	static IPlayerInfo *pPI;
	static IGamePlayer *pPl;
	static const char *szWeaponName;

	if (m_fCheckHealTime > engine->Time())
		return;

	m_fCheckHealTime = engine->Time() + 1.0f;

	m_bIsBeingHealed = false;
	m_bCanBeUbered = false;

	for (short int i = 1; i <= MAX_PLAYERS; i++)
	{
		pPlayer = INDEXENT(i);

		if (pPlayer == m_pEdict)
			continue;

		pPl = playerhelpers->GetGamePlayer(pPlayer);
		if (!pPl || !pPl->IsInGame() || !pPl->IsConnected())
			continue;

		pPI = pPl->GetPlayerInfo();
		if (pPlayer && !pPlayer->IsFree() && pPI && !pPI->IsDead() && pPI->GetTeamIndex() == GetTeam())
		{
			szWeaponName = pPI->GetWeaponName();
			if (szWeaponName && !strcmp(szWeaponName, "tf_weapon_medigun"))
			{
				pWeapon = CTeamFortress2Mod::GetMediGun(pPlayer);

				if (!pWeapon)
					continue;

				if (CClassInterface::GetMedigunHealing(pWeapon) && (CClassInterface::IsMedigunTargetting(pWeapon, m_pEdict)))
				{
					if (CClassInterface::GetUberChargeLevel(pWeapon) > 99)
						m_bCanBeUbered = true;

					m_bIsBeingHealed = true;
					m_pHealer = pPlayer;
				}
			}
		}
	}
}

// Preconditions :  Current weapon is Medigun
//					pPlayer is not NULL
//
bool CBotTF2::HealPlayer()
{
	static CBotWeapon *pWeap;
	static IPlayerInfo *pPl;
	static edict_t *pWeapon;
	static Vector vOrigin;
	static Vector vForward;
	static QAngle eyes;
	static float fSpeed;
	static CClient *pClient;

	if (!m_pHeal)
		return false;

	if (GetHealFactor(m_pHeal) == 0.0f)
		return false;

	vOrigin = CBotGlobals::EntityOrigin(m_pHeal);
	pWeap = GetCurrentWeapon();

	//if ( (distanceFrom(vOrigin) > 250) && !isVisible(m_pHeal) ) 
	//	return false;
	pPl = playerhelpers->GetGamePlayer(m_pHeal)->GetPlayerInfo();

	if (!pPl || pPl->IsDead() || !pPl->IsConnected() || pPl->IsObserver())
		return false;

	if (m_fMedicUpdatePosTime < engine->Time())
	{
		float fRand;


		fRand = RandomFloat(1.0f, 2.0f);

		Vector vVelocity;
		CClassInterface::GetVelocity(m_pHeal, &vVelocity);
		fSpeed = vVelocity.Length();

		m_fMedicUpdatePosTime = engine->Time() + (fRand * (1.0f - (fSpeed / 320)));

		if (pPl && (pPl->GetLastUserCommand().buttons & IN_ATTACK))
		{
			// keep out of cross fire
			eyes = CBotGlobals::PlayerAngles(m_pHeal);
			AngleVectors(eyes, &vForward);
			vForward = vForward / vForward.Length();
			vOrigin = vOrigin - (vForward * 150);
			m_fHealingMoveTime = engine->Time();
		}
		else if (fSpeed > 100.0f)
			m_fHealingMoveTime = engine->Time();

		if (m_fHealingMoveTime == 0.0f)
			m_fHealingMoveTime = engine->Time();

		m_vMedicPosition = vOrigin;


		{
			if (m_pNearestPipeGren.Get() || m_NearestEnemyRocket.Get())
			{
				m_iDesiredResistType = RESIST_EXPLO;
			}
			else if (CTeamFortress2Mod::TF2_IsPlayerOnFire(m_pHeal) || CTeamFortress2Mod::TF2_IsPlayerOnFire(m_pEdict))
			{
				m_iDesiredResistType = RESIST_FIRE;
			}
			else if (RandomInt(0, 1) == 1)
			{
				m_iDesiredResistType = RESIST_BULLETS;
			}
		}
	}

	/*if ( CTeamFortress2Mod::hasRoundStarted() && (m_fHealingMoveTime + 8.0f < engine->Time()) )
	{
	m_pLastHeal
	return false;
	}*/
	edict_t *pMedigun;

	if ((pWeap->GetID() == TF2_WEAPON_MEDIGUN) && ((pMedigun = pWeap->GetWeaponEntity()) != NULL))
	{
		if (CClassInterface::GetChargeResistType(pMedigun) != m_iDesiredResistType)
		{
			if (RandomInt(0, 1) == 1)
				m_pButtons->Tap(IN_RELOAD);
		}
	}

	if (DistanceFrom(m_vMedicPosition) < 100)
		StopMoving();
	else
		SetMoveTo(m_vMedicPosition);

	pWeapon = INDEXENT(pWeap->GetWeaponIndex());

	if (pWeapon == NULL)
		return false;

	m_bIncreaseSensitivity = true;

	//!!!CRASH!!!
	/*if ( !CClassInterface::GetMedigunHealing(pWeapon) )
	{
	if ( (m_fHealClickTime < engine->Time()) && (DotProductFromOrigin(vOrigin) > 0.98f) )
	PrimaryAttack();
	//else
	//m_pButtons->LetGo(IN_ATTACK);
	}
	else
	{*/
	edict_t *pent;

	edict_t *pPlayer = NULL;

	// Find the player I'm currently healing
	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		pent = INDEXENT(i);

		if (pent && CBotGlobals::EntityIsValid(pent))
		{
			if (CClassInterface::IsMedigunTargetting(pWeapon, pent))
			{
				pPlayer = pent;
				break;
			}
		}
	}

	// found it
	if (pPlayer)
	{
		// is the person I want to heal different from the player I am healing now?
		if (m_pHeal != pPlayer)
		{
			// yes -- press fire to disconnect from player
			if (m_fHealClickTime < engine->Time())
			{
				extern ConVar bot_tf2_medic_letgotime;
				m_fHealClickTime = engine->Time() + bot_tf2_medic_letgotime.GetFloat();
			}

			//m_pButtons->letGo(IN_ATTACK);
		}
		else if (m_fHealClickTime < engine->Time())
			PrimaryAttack();
	}
	else if ((m_fHealClickTime < engine->Time()) && (DotProductFromOrigin(vOrigin) > 0.98f))
		PrimaryAttack(); // bug fix for now
	//}
	//else
	//	m_pHeal = CClassInterface::getMedigunTarget(INDEXENT(pWeap->getWeaponIndex()));
	LookAtEdict(m_pHeal);
	SetLookAtTask(LOOK_EDICT);

	m_pLastHeal = m_pHeal;

	// Simple UBER check : healing player not ubered already
	if (!CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pHeal) && !CTeamFortress2Mod::IsFlagCarrier(m_pHeal) &&
		(m_pEnemy && IsVisible(m_pEnemy)) || (((((float)m_pPI->GetHealth()) / m_pPI->GetMaxHealth()) < 0.33) || (GetHealthPercent() < 0.33)))
	{
		if (CTeamFortress2Mod::HasRoundStarted())
		{
			// uber if ready / and round has started
			m_pButtons->Tap(IN_ATTACK2);
		}
	}

	return true;
}
// The lower the better
float CBotTF2::GetEnemyFactor(edict_t *pEnemy)
{
	float fPreFactor = 0;

	// Player
	if (CBotGlobals::IsPlayer(pEnemy))
	{
		const char *szModel = pEnemy->GetIServerEntity()->GetModelName().ToCStr();
		IPlayerInfo *p = playerhelpers->GetGamePlayer(pEnemy)->GetPlayerInfo();

		if (CTeamFortress2Mod::IsFlagCarrier(pEnemy))
		{
			// this enemy is carrying the flag, attack!
			// shoot flag carrier even if 1000 units away from nearest enemy
			fPreFactor = -1000.0f;
		}
		else if (!CTeamFortress2Mod::IsMapType(TF_MAP_CTF) && (CTeamFortress2Mod::IsCapping(pEnemy) || CTeamFortress2Mod::IsDefending(pEnemy)))
		{
			// this enemy is capping the point, attack!
			fPreFactor = -400.0f;
		}
		else if (CTeamFortress2Mod::TF2_IsPlayerInvuln(pEnemy))
		{
			// dont shoot ubered player unlesss he's the only thing around for 2000 units
			fPreFactor = 2000.0f;
		}
		else if ((m_iClass == TF_CLASS_SPY) && IsDisguised() && (p != NULL) && (CBotGlobals::DotProductFromOrigin(CBotGlobals::EntityOrigin(pEnemy), GetOrigin(), p->GetLastUserCommand().viewangles) > 0.1f))
		{
			// I'm disguised as a spy but this guy can see me , better lay off the attack unless theres someone else around
			fPreFactor = 1000.0f;
		}
		// models/bots/demo/bot_sentry_buster.mdl
		// 0000000000111111111122222222223333333
		// 0123456789012345678901234567890123456
		else if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM) && (szModel[7] == 'b') && (szModel[12] == 'd') && (szModel[17] == 'b') && (szModel[21] == 's') && (szModel[28] == 'b'))
		{
			// sentry buster
			fPreFactor = -500.0f;
		}
		else
		{
			int iClass = CClassInterface::GetTF2Class(pEnemy);

			if (iClass == TF_CLASS_MEDIC)
			{
				// shoot medic even if 250 units further from nearest enemy (approx. healing range)
				fPreFactor = -260.0f;
			}
			else if (iClass == TF_CLASS_SPY)
			{
				// shoot spy even if a little further from nearest enemy
				fPreFactor = -400.0f;
			}
			else if (iClass == TF_CLASS_SNIPER)
			{
				// I'm a spy, and I can see a sniper
				//	if ( m_iClass == TF_CLASS_SPY )
				//		fPreFactor = -600;
				//	else
				if (m_iClass == TF_CLASS_SNIPER)
					fPreFactor = -2048.0f;
				else
				{
					// Behind the sniper ATTACK
					if (CBotGlobals::DotProductFromOrigin(pEnemy, GetOrigin()) > 0.98) //10 deg
						fPreFactor = -1024.0f;
					else // Sniper can see me
						fPreFactor = -300.0f;
				}
			}
			else if (iClass == TF_CLASS_ENGINEER)
			{
				// I'm a spy and I'm attacking an engineer!
				if (m_iClass == TF_CLASS_SPY)
				{
					edict_t *pSentry = NULL;

					fPreFactor = -400.0f;

					if ((pSentry = m_pNearestEnemySentry.Get()) != NULL)
					{
						if ((CTeamFortress2Mod::GetSentryOwner(pSentry) == pEnemy) && CTeamFortress2Mod::IsSentrySapped(pSentry) && (DistanceFrom(pSentry) < TF2_MAX_SENTRYGUN_RANGE))
						{
							// this guy is the owner of a disabled sentry gun -- take him out!
							fPreFactor = -1024.0f;
						}
					}
				}
			}
		}
	}
	else
	{
		float fBossFactor = -1024.0f;

		if (CTeamFortress2Mod::IsSentry(pEnemy, CTeamFortress2Mod::GetEnemyTeam(GetTeam())))
		{
			edict_t *pOwner = CTeamFortress2Mod::GetBuildingOwner(ENGI_SENTRY, ENTINDEX(pEnemy));

			if (pOwner && IsVisible(pOwner))
			{
				// owner probably repairing it -- maybe make use of others around me
				fPreFactor = -512.0f;
			}
			else if (CClassInterface::GetSentryEnemy(pEnemy) != m_pEdict)
				fPreFactor = -1124.0f;
			else
				fPreFactor = -768.0f;
		}
		else if (CTeamFortress2Mod::IsBoss(pEnemy, &fBossFactor))
		{
			extern ConVar bot_bossattackfactor;

			fPreFactor = fBossFactor * bot_bossattackfactor.GetFloat();
		}
		else if (CTeamFortress2Mod::IsPipeBomb(pEnemy, CTeamFortress2Mod::GetEnemyTeam(m_iTeam)))
		{
			fPreFactor = 320.0f;
		}
	}

	fPreFactor += DistanceFrom(pEnemy);

	return fPreFactor;
}


bool CBotFortress::WantToNest()
{
	return (!HasFlag() && ((GetClass() != TF_CLASS_ENGINEER) && (m_pSentryGun.Get() != NULL)) && ((GetClass() != TF_CLASS_MEDIC) || !m_pHeal) && (GetHealthPercent() < 0.95) && (NearbyFriendlies(256.0f) < 2));
}

void CBotTF2::TeleportedPlayer(void)
{
	m_iTeleportedPlayers++;
}

void CBotTF2::GetTasks(unsigned int iIgnore)
{
	static TFClass iClass;
	static int iMetal;
	static bool bNeedAmmo;
	static bool bNeedHealth;
	static CBotUtilities utils;
	static CBotWeapon *pWeapon;
	static CWaypoint *pWaypointResupply;
	static CWaypoint *pWaypointAmmo;
	static CWaypoint *pWaypointHealth;
	static CBotUtility *next;
	static float fResupplyDist;
	static float fHealthDist;
	static float fAmmoDist;
	static bool bHasFlag;
	static float fGetFlagUtility;
	static float fDefendFlagUtility;
	static int iTeam;
	static float fMetalPercent;
	static Vector vOrigin;
	static unsigned char *failedlist;

	extern ConVar bot_move_sentry_kpm;
	extern ConVar bot_move_sentry_time;
	extern ConVar bot_move_disp_time;
	extern ConVar bot_move_disp_healamount;
	extern ConVar bot_move_tele_time;
	extern ConVar bot_move_tele_tpm;
	extern ConVar bot_move_obj;

	static bool bMoveObjs;

	static bool bSentryHasEnemy;
	static int iSentryLevel;
	static int iDispenserLevel;
	static int iAllySentryLevel;
	static int iAllyDispLevel;

	static float fEntranceDist;
	static float fExitDist;
	static float fUseDispFactor;

	static float fAllyDispenserHealthPercent;
	static float fAllySentryHealthPercent;

	static float fSentryHealthPercent;
	static float fDispenserHealthPercent;
	static float fTeleporterEntranceHealthPercent;
	static float fTeleporterExitHealthPercent;

	static float fSentryPlaceTime;
	static float fDispenserPlaceTime;
	static float fTeleporterEntPlaceTime;
	static float fTeleporterExtPlaceTime;

	static edict_t *pMedigun;
	static float fSentryUtil;
	static int iMetalInDisp;


	static int numplayersonteam;
	static int numplayersonteam_alive;

	static bool bCheckCurrent;

	extern ConVar bot_messaround;
	extern ConVar bot_defrate;

	static CBotWeapon *pBWMediGun = NULL;
	//static float fResupplyUtil = 0.5;
	//static float fHealthUtil = 0.5;
	//static float fAmmoUtil = 0.5;

	// if in setup time this will tell bot not to shoot yet
	WantToShoot(CTeamFortress2Mod::HasRoundStarted());
	WantToListen(CTeamFortress2Mod::HasRoundStarted());

	if (!HasSomeConditions(CONDITION_CHANGED) && !m_pSchedules->IsEmpty())
		return;

	RemoveCondition(CONDITION_CHANGED);

	if (CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) && HasEnemy())
	{
		// keep attacking enemy -- no change to task
		return;
	}

	bCheckCurrent = true; // important for checking the current schedule if not empty
	iMetal = 0;
	pBWMediGun = NULL;
	vOrigin = GetOrigin();
	bNeedAmmo = false;
	bNeedHealth = false;
	fResupplyDist = 1;
	fHealthDist = 1;
	fAmmoDist = 1;
	bHasFlag = false;
	fGetFlagUtility = 0.5;
	fDefendFlagUtility = 0.5;
	iTeam = m_iTeam;
	bHasFlag = HasFlag();
	failedlist = NULL;

	numplayersonteam = CBotGlobals::NumPlayersOnTeam(iTeam, false);
	numplayersonteam_alive = CBotGlobals::NumPlayersOnTeam(iTeam, true);

	// No Enemy now
	if ((m_iClass == TF_CLASS_SNIPER) && !HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
		// un zoom
	{
		if (CTeamFortress2Mod::TF2_IsPlayerZoomed(m_pEdict))
			SecondaryAttack();
	}

	iClass = GetClass();

	bNeedAmmo = HasSomeConditions(CONDITION_NEED_AMMO);

	// don't need health if being healed or ubered!
	bNeedHealth = HasSomeConditions(CONDITION_NEED_HEALTH)
		&& !m_bIsBeingHealed && !CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict);

	if (m_pHealthkit)
	{
		if (!CBotGlobals::EntityIsValid(m_pHealthkit))
			m_pHealthkit = NULL;
	}

	if (m_pAmmo)
	{
		if (!CBotGlobals::EntityIsValid(m_pAmmo))
			m_pAmmo = NULL;
	}

	pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_WRENCH));
	if (pWeapon != NULL)
	{
		iMetal = pWeapon->GetAmmo(this);
		fMetalPercent = (float)iMetal / 200;
	}

	if (bNeedAmmo || bNeedHealth)
	{
		dataUnconstArray<int> *failed;
		Vector vOrigin = GetOrigin();

		m_pNavigator->GetFailedGoals(&failed);

		failedlist = CWaypointLocations::ResetFailedWaypoints(failed);

		fResupplyDist = 1;
		fHealthDist = 1;
		fAmmoDist = 1;

		pWaypointResupply = CWaypoints::GetWaypoint(CWaypoints::GetClosestFlagged(CWaypointTypes::W_FL_RESUPPLY, vOrigin, iTeam, &fResupplyDist, failedlist));

		if (bNeedAmmo)
			pWaypointAmmo = CWaypoints::GetWaypoint(CWaypoints::GetClosestFlagged(CWaypointTypes::W_FL_AMMO, vOrigin, iTeam, &fAmmoDist, failedlist));
		if (bNeedHealth)
			pWaypointHealth = CWaypoints::GetWaypoint(CWaypoints::GetClosestFlagged(CWaypointTypes::W_FL_HEALTH, vOrigin, iTeam, &fHealthDist, failedlist));
	}

	if (iClass == TF_CLASS_ENGINEER)
	{
		CheckBuildingsValid(true);
		UpdateCarrying();
	}

	ADD_UTILITY(BOT_UTIL_CAPTURE_FLAG, (CTeamFortress2Mod::IsMapType(TF_MAP_CTF) || CTeamFortress2Mod::IsMapType(TF_MAP_SD)) && bHasFlag, 0.95f);

	if (iClass == TF_CLASS_ENGINEER)
	{
		bool bCanBuild = m_pWeapons->HasWeapon(TF2_WEAPON_BUILDER);

		bMoveObjs = bot_move_obj.GetBool();

		iSentryLevel = 0;
		iDispenserLevel = 0;
		iAllySentryLevel = 0;
		iAllyDispLevel = 0;

		fEntranceDist = 99999.0f;
		fExitDist = 99999.0f;
		fUseDispFactor = 0.0f;

		fAllyDispenserHealthPercent = 1.0f;
		fAllySentryHealthPercent = 1.0f;

		fSentryHealthPercent = 1.0f;
		fDispenserHealthPercent = 1.0f;
		fTeleporterEntranceHealthPercent = 1.0f;
		fTeleporterExitHealthPercent = 1.0f;

		fSentryPlaceTime = (engine->Time() - m_fSentryPlaceTime);
		fDispenserPlaceTime = (engine->Time() - m_fDispenserPlaceTime);
		fTeleporterEntPlaceTime = (engine->Time() - m_fTeleporterEntPlacedTime);
		fTeleporterExtPlaceTime = (engine->Time() - m_fTeleporterExtPlacedTime);


		if (m_pTeleExit.Get())
		{
			fExitDist = DistanceFrom(m_pTeleExit);

			fTeleporterExitHealthPercent = CClassInterface::GetTeleporterHealth(m_pTeleExit) / 180;

			ADD_UTILITY(BOT_UTIL_ENGI_MOVE_EXIT, (CTeamFortress2Mod::HasRoundStarted() || CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) && (!m_bIsCarryingObj || m_bIsCarryingTeleExit) && bMoveObjs && m_pTeleEntrance && m_pTeleExit &&
				m_fTeleporterExtPlacedTime && (fTeleporterExtPlaceTime > bot_move_tele_time.GetFloat()) &&
				(((60.0f * m_iTeleportedPlayers) / fTeleporterExtPlaceTime) < bot_move_tele_tpm.GetFloat()), (fTeleporterExitHealthPercent*GetHealthPercent()*fMetalPercent) + ((int)m_bIsCarryingTeleExit));

		}

		if (m_pTeleEntrance.Get())
		{
			fEntranceDist = DistanceFrom(m_pTeleEntrance);

			fTeleporterEntranceHealthPercent = CClassInterface::GetTeleporterHealth(m_pTeleEntrance) / 180;

			ADD_UTILITY(BOT_UTIL_ENGI_MOVE_ENTRANCE, (!m_bIsCarryingObj || m_bIsCarryingTeleEnt) && bMoveObjs && m_bEntranceVectorValid && m_pTeleEntrance && m_pTeleExit &&
				m_fTeleporterEntPlacedTime && (fTeleporterEntPlaceTime > bot_move_tele_time.GetFloat()) &&
				(((60.0f * m_iTeleportedPlayers) / fTeleporterEntPlaceTime) < bot_move_tele_tpm.GetFloat()), (fTeleporterEntranceHealthPercent*GetHealthPercent()*fMetalPercent) + ((int)m_bIsCarryingTeleEnt));

		}

		if (m_pSentryGun.Get())
		{
			bSentryHasEnemy = (CClassInterface::GetSentryEnemy(m_pSentryGun) != NULL);
			iSentryLevel = CClassInterface::GetTF2UpgradeLevel(m_pSentryGun);//CTeamFortress2Mod::GetSentryLevel(m_pSentryGun);
			fSentryHealthPercent = ((float)CClassInterface::GetSentryHealth(m_pSentryGun)) / CClassInterface::GetTF2GetBuildingMaxHealth(m_pSentryGun);
			// move sentry
			ADD_UTILITY(BOT_UTIL_ENGI_MOVE_SENTRY, (CTeamFortress2Mod::HasRoundStarted() || CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) && (!m_bIsCarryingObj || m_bIsCarryingSentry) &&
				bMoveObjs && (m_fSentryPlaceTime > 0.0f) && !bHasFlag && m_pSentryGun && (CClassInterface::GetSentryEnemy(m_pSentryGun) == NULL) && ((m_fLastSentryEnemyTime + 15.0f) < engine->Time()) &&
				(!CTeamFortress2Mod::IsMapType(TF_MAP_CP) || CTeamFortress2Mod::m_ObjectiveResource.TestProbWptArea(m_iSentryArea, m_iTeam)) &&
				(fSentryPlaceTime > bot_move_sentry_time.GetFloat()) && (((60.0f*m_iSentryKills) / fSentryPlaceTime) < bot_move_sentry_kpm.GetFloat()),
				(fMetalPercent*GetHealthPercent()*fSentryHealthPercent) + ((int)m_bIsCarryingSentry));

		}

		if (m_pDispenser.Get())
		{
			iMetalInDisp = CClassInterface::GetTF2DispMetal(m_pDispenser);
			iDispenserLevel = CClassInterface::GetTF2UpgradeLevel(m_pDispenser); // CTeamFortress2Mod::GetDispenserLevel(m_pDispenser);
			fDispenserHealthPercent = ((float)CClassInterface::GetDispenserHealth(m_pDispenser)) / CClassInterface::GetTF2GetBuildingMaxHealth(m_pDispenser);

			fUseDispFactor = (((float)iMetalInDisp) / 400) * (1.0f - fMetalPercent) * ((float)iDispenserLevel / 3) * (1000.0f / DistanceFrom(m_pDispenser));

			// move disp
			ADD_UTILITY(BOT_UTIL_ENGI_MOVE_DISP, (CTeamFortress2Mod::HasRoundStarted() || CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) && (!m_bIsCarryingObj || m_bIsCarryingDisp) && bMoveObjs && (m_fDispenserPlaceTime > 0.0f) && !bHasFlag && m_pDispenser && (fDispenserPlaceTime > bot_move_disp_time.GetFloat()) && (((60.0f*m_fDispenserHealAmount) / fDispenserPlaceTime) < bot_move_disp_healamount.GetFloat()), ((((float)iMetalInDisp) / 400)*fMetalPercent*GetHealthPercent()*fDispenserHealthPercent) + ((int)m_bIsCarryingDisp));

		}

		if (m_pNearestDisp && (m_pNearestDisp.Get() != m_pDispenser.Get()))
		{
			iMetalInDisp = CClassInterface::GetTF2DispMetal(m_pNearestDisp);
			iAllyDispLevel = CClassInterface::GetTF2UpgradeLevel(m_pNearestDisp); // CTeamFortress2Mod::getDispenserLevel(m_pDispenser);
			fAllyDispenserHealthPercent = ((float)CClassInterface::GetDispenserHealth(m_pNearestDisp)) / CClassInterface::GetTF2GetBuildingMaxHealth(m_pNearestDisp);

			fUseDispFactor = (((float)iMetalInDisp) / 400) * (1.0f - fMetalPercent) * ((float)iAllyDispLevel / 3) * (1000.0f / DistanceFrom(m_pNearestDisp));

			ADD_UTILITY(BOT_UTIL_GOTODISP, m_pNearestDisp && !CClassInterface::IsObjectBeingBuilt(m_pNearestDisp) && (bNeedAmmo || bNeedHealth), fUseDispFactor + ((!CTeamFortress2Mod::HasRoundStarted() && CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) ? 0.5f : 0.0f));
			ADD_UTILITY(BOT_UTIL_REMOVE_TMDISP_SAPPER, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && m_pNearestDisp && CTeamFortress2Mod::IsDispenserSapped(m_pNearestDisp), 1.1f);
			ADD_UTILITY(BOT_UTIL_UPGTMDISP, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && (m_pNearestDisp != NULL) && (m_pNearestDisp != m_pDispenser) && (iMetal >= (200 - CClassInterface::GetTF2SentryUpgradeMetal(m_pNearestDisp))) && ((iAllyDispLevel < 3) || (fAllyDispenserHealthPercent < 1.0f)), 0.7 + ((1.0f - fAllyDispenserHealthPercent)*0.3));
		}

		if (m_pNearestAllySentry && (m_pNearestAllySentry.Get() != m_pSentryGun.Get()) && !CClassInterface::GetTF2BuildingIsMini(m_pNearestAllySentry))
		{
			iAllySentryLevel = CClassInterface::GetTF2UpgradeLevel(m_pNearestAllySentry);
			fAllySentryHealthPercent = CClassInterface::GetSentryHealth(m_pNearestAllySentry);
			fAllySentryHealthPercent = fAllySentryHealthPercent / CClassInterface::GetTF2GetBuildingMaxHealth(m_pNearestAllySentry);

			ADD_UTILITY(BOT_UTIL_REMOVE_TMSENTRY_SAPPER, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && m_pNearestAllySentry && CTeamFortress2Mod::IsSentrySapped(m_pNearestAllySentry), 1.1f);
			ADD_UTILITY(BOT_UTIL_UPGTMSENTRY, (fAllySentryHealthPercent > 0.0f) && !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && !bHasFlag && m_pNearestAllySentry && (m_pNearestAllySentry != m_pSentryGun) && (iMetal >= (200 - CClassInterface::GetTF2SentryUpgradeMetal(m_pNearestAllySentry))) && ((iAllySentryLevel < 3) || (fAllySentryHealthPercent < 1.0f)), 0.8 + ((1.0f - fAllySentryHealthPercent)*0.2));
		}

		fSentryUtil = 0.8 + (((float)((int)bNeedAmmo))*0.1) + (((float)(int)bNeedHealth)*0.1);

		ADD_UTILITY(BOT_UTIL_PLACE_BUILDING, m_bIsCarryingObj, 1.0f); // something went wrong moving this- I still have it!!!

		// destroy and build anew
		ADD_UTILITY(BOT_UTIL_ENGI_DESTROY_SENTRY, !m_bIsCarryingObj && (iMetal >= 130) && (m_pSentryGun.Get() != NULL) && !CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(m_iSentryArea), fSentryUtil);
		ADD_UTILITY(BOT_UTIL_ENGI_DESTROY_DISP, !m_bIsCarryingObj && (iMetal >= 125) && (m_pDispenser.Get() != NULL) && !CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(m_iDispenserArea), RandomFloat(0.7, 0.9));
		ADD_UTILITY(BOT_UTIL_ENGI_DESTROY_ENTRANCE, !m_bIsCarryingObj && (iMetal >= 125) && (m_pTeleEntrance.Get() != NULL) && !CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(m_iTeleEntranceArea), RandomFloat(0.7, 0.9));
		//ADD_UTILITY(BOT_UTIL_ENGI_DESTROY_EXIT, (iMetal>=125) && (m_pTeleExit.Get()!=NULL) && !CPoints::IsValidArea(m_iTeleExitArea), RandomFloat(0.7,0.9));

		if (bCanBuild)
		{
			ADD_UTILITY(BOT_UTIL_BUILDSENTRY, !m_bIsCarryingObj && !bHasFlag && !m_pSentryGun && (iMetal >= 130), 0.9);
			ADD_UTILITY(BOT_UTIL_BUILDDISP, !m_bIsCarryingObj && !bHasFlag && m_pSentryGun && (CClassInterface::GetSentryHealth(m_pSentryGun) > 125) && !m_pDispenser && (iMetal >= 100), fSentryUtil);

			if (CTeamFortress2Mod::IsAttackDefendMap() && (iTeam == TF2_TEAM_BLUE))
			{
				ADD_UTILITY(BOT_UTIL_BUILDTELEXT, (fSentryHealthPercent > 0.99f) && !m_bIsCarryingObj && !bHasFlag && !m_pTeleExit && (iMetal >= 125), RandomFloat(0.7f, 0.9f));
				ADD_UTILITY(BOT_UTIL_BUILDTELENT, !bSentryHasEnemy && (fSentryHealthPercent > 0.99f) && !m_bIsCarryingObj && !bHasFlag && m_bEntranceVectorValid && !m_pTeleEntrance && (iMetal >= 125), 0.7f);
			}
			else
			{
				ADD_UTILITY(BOT_UTIL_BUILDTELENT, (fSentryHealthPercent > 0.99f) && !m_bIsCarryingObj && !bHasFlag && ((m_pSentryGun.Get() && (iSentryLevel > 1)) || (m_pSentryGun.Get() == NULL)) && m_bEntranceVectorValid && !m_pTeleEntrance && (iMetal >= 125), 0.7f);
				ADD_UTILITY(BOT_UTIL_BUILDTELEXT, !bSentryHasEnemy && (fSentryHealthPercent > 0.99f) && !m_bIsCarryingObj && !bHasFlag && m_pSentryGun && (iSentryLevel > 1) && !m_pTeleExit && (iMetal >= 125), RandomFloat(0.7, 0.9));
			}


			if ((m_fSpawnTime + 5.0f) > engine->Time())
			{

				dataUnconstArray<int> *failed;
				Vector vOrigin = GetOrigin();

				m_pNavigator->GetFailedGoals(&failed);

				failedlist = CWaypointLocations::ResetFailedWaypoints(failed);

				pWaypointResupply = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(vOrigin, 1024.0f, -1, false, false, true, NULL, false, GetTeam(), true, false, Vector(0, 0, 0), CWaypointTypes::W_FL_RESUPPLY));//CWaypoints::GetWaypoint(CWaypoints::GetClosestFlagged(CWaypointTypes::W_FL_RESUPPLY, vOrigin, iTeam, &fResupplyDist, failedlist));

				ADD_UTILITY_DATA(BOT_UTIL_BUILDTELENT_SPAWN, !m_bIsCarryingObj && (pWaypointResupply != NULL) && !bHasFlag && !m_pTeleEntrance && (iMetal >= 125), 0.95f, CWaypoints::GetWaypointIndex(pWaypointResupply));
			}
		}
		// to do -- split into two
		ADD_UTILITY(BOT_UTIL_UPGSENTRY, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && !bHasFlag && (m_pSentryGun.Get() != NULL) && !CClassInterface::GetTF2BuildingIsMini(m_pSentryGun) && (((iSentryLevel < 3) && (iMetal >= (200 - CClassInterface::GetTF2SentryUpgradeMetal(m_pSentryGun)))) || ((fSentryHealthPercent < 1.0f) && (iMetal>75)) || (CClassInterface::GetSentryEnemy(m_pSentryGun) != NULL)), 0.8 + ((1.0f - fSentryHealthPercent)*0.2));

		ADD_UTILITY(BOT_UTIL_GETAMMODISP, !m_bIsCarryingObj && m_pDispenser && !CClassInterface::IsObjectBeingBuilt(m_pDispenser) && IsVisible(m_pDispenser) && (iMetal < 200), fUseDispFactor);

		ADD_UTILITY(BOT_UTIL_UPGTELENT, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && m_pTeleEntrance != NULL && !CClassInterface::IsObjectBeingBuilt(m_pTeleEntrance) && (iMetal >= (200 - CClassInterface::GetTF2SentryUpgradeMetal(m_pTeleEntrance))) && (fTeleporterEntranceHealthPercent < 1.0f), ((fEntranceDist < fExitDist)) * 0.51 + (0.5 - (fTeleporterEntranceHealthPercent*0.5)));
		ADD_UTILITY(BOT_UTIL_UPGTELEXT, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && m_pTeleExit != NULL && !CClassInterface::IsObjectBeingBuilt(m_pTeleExit) && (iMetal >= (200 - CClassInterface::GetTF2SentryUpgradeMetal(m_pTeleExit))) && (fTeleporterExitHealthPercent < 1.0f), ((fExitDist < fEntranceDist) * 0.51) + ((0.5 - fTeleporterExitHealthPercent)*0.5));
		ADD_UTILITY(BOT_UTIL_UPGDISP, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && m_pDispenser != NULL && !CClassInterface::IsObjectBeingBuilt(m_pDispenser) && (iMetal >= (200 - CClassInterface::GetTF2SentryUpgradeMetal(m_pDispenser))) && ((iDispenserLevel < 3) || (fDispenserHealthPercent < 1.0f)), 0.7 + ((1.0f - fDispenserHealthPercent)*0.3));

		// remove sappers
		ADD_UTILITY(BOT_UTIL_REMOVE_SENTRY_SAPPER, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && !bHasFlag && (m_pSentryGun != NULL) && CTeamFortress2Mod::IsMySentrySapped(m_pEdict), 1000.0f);
		ADD_UTILITY(BOT_UTIL_REMOVE_DISP_SAPPER, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && !bHasFlag && (m_pDispenser != NULL) && CTeamFortress2Mod::IsMyDispenserSapped(m_pEdict), 1000.0f);

		ADD_UTILITY_DATA(BOT_UTIL_GOTORESUPPLY_FOR_AMMO, !CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) && !m_bIsCarryingObj && !bHasFlag && pWaypointResupply && bNeedAmmo && !m_pAmmo, 1000.0f / fResupplyDist, CWaypoints::GetWaypointIndex(pWaypointResupply));
		ADD_UTILITY_DATA(BOT_UTIL_FIND_NEAREST_AMMO, !m_bIsCarryingObj && !bHasFlag&&bNeedAmmo && !m_pAmmo&&pWaypointAmmo, (400.0f / fAmmoDist) + ((!CTeamFortress2Mod::HasRoundStarted() && CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) ? 0.5f : 0.0f), CWaypoints::GetWaypointIndex(pWaypointAmmo)); // only if close

		ADD_UTILITY(BOT_UTIL_ENGI_LOOK_AFTER_SENTRY, !m_bIsCarryingObj && (m_pSentryGun.Get() != NULL) && (iSentryLevel>2) && (m_fLookAfterSentryTime < engine->Time()), fGetFlagUtility + 0.01f);

		// remove sappers

		ADD_UTILITY(BOT_UTIL_REMOVE_TMTELE_SAPPER, !m_bIsCarryingObj && (m_fRemoveSapTime < engine->Time()) && m_pNearestTeleEntrance && CTeamFortress2Mod::IsTeleporterSapped(m_pNearestTeleEntrance), 1.1f);



		// booooo
	}
	else
	{
		pMedigun = CTeamFortress2Mod::GetMediGun(m_pEdict);

		if (pMedigun != NULL)
			pBWMediGun = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_MEDIGUN));

		if (!m_pNearestDisp)
		{
			m_pNearestDisp = CTeamFortress2Mod::NearestDispenser(GetOrigin(), iTeam);
		}

		if ((m_iClass != TF_CLASS_SPY) && (m_iClass != TF_CLASS_MEDIC) && (m_iClass != TF_CLASS_SNIPER))
		{
			//squads // follow leader // follow_leader
			ADD_UTILITY(BOT_UTIL_FOLLOW_SQUAD_LEADER, !HasSomeConditions(CONDITION_DEFENSIVE) && HasSomeConditions(CONDITION_SQUAD_LEADER_INRANGE) && InSquad() && (m_pSquad->GetLeader() != m_pEdict) && HasSomeConditions(CONDITION_SEE_SQUAD_LEADER), HasSomeConditions(CONDITION_SQUAD_IDLE) ? 0.1f : 2.0f);
			ADD_UTILITY(BOT_UTIL_FIND_SQUAD_LEADER, !HasSomeConditions(CONDITION_DEFENSIVE) && InSquad() && (m_pSquad->GetLeader() != m_pEdict) && (!HasSomeConditions(CONDITION_SEE_SQUAD_LEADER) || !HasSomeConditions(CONDITION_SQUAD_LEADER_INRANGE)), HasSomeConditions(CONDITION_SQUAD_IDLE) ? 0.1f : 2.0f);
		}

		ADD_UTILITY_DATA(BOT_UTIL_GOTORESUPPLY_FOR_AMMO, !CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) && !m_bIsCarryingObj && !bHasFlag && pWaypointResupply && bNeedAmmo && !m_pAmmo, 1000.0f / fResupplyDist, CWaypoints::GetWaypointIndex(pWaypointResupply));
		ADD_UTILITY_DATA(BOT_UTIL_FIND_NEAREST_AMMO, !bHasFlag && bNeedAmmo && !m_pAmmo && pWaypointAmmo, (400.0f / fAmmoDist) + ((!CTeamFortress2Mod::HasRoundStarted() && CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) ? 0.5f : 0.0f), CWaypoints::GetWaypointIndex(pWaypointAmmo));

		if (m_pNearestDisp)
			ADD_UTILITY(BOT_UTIL_GOTODISP, m_pNearestDisp && !CClassInterface::IsObjectBeingBuilt(m_pNearestDisp) && !CTeamFortress2Mod::IsDispenserSapped(m_pNearestDisp) && (bNeedAmmo || bNeedHealth), (1000.0f / DistanceFrom(m_pNearestDisp)) + ((!CTeamFortress2Mod::HasRoundStarted() && CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) ? 0.5f : 0.0f));
	}

	fGetFlagUtility = 0.2 + RandomFloat(0.0f, 0.2f);

	if (m_iClass == TF_CLASS_SCOUT)
		fGetFlagUtility = 0.6f;
	else if (m_iClass == TF_CLASS_SPY)
		fGetFlagUtility = 0.6f;
	else if (m_iClass == TF_CLASS_MEDIC)
	{
		if (CTeamFortress2Mod::HasRoundStarted())
			fGetFlagUtility = 0.85f - (((float)numplayersonteam) / (MAX_PLAYERS / 2));
		else
			fGetFlagUtility = 0.1f; // not my priority
	}
	else if (m_iClass == TF_CLASS_ENGINEER)
	{
		fGetFlagUtility = 0.1f; // not my priority

		if (m_bIsCarryingObj)
			fGetFlagUtility = 0.0f;
	}

	fDefendFlagUtility = bot_defrate.GetFloat() / 2;

	if ((m_iClass == TF_CLASS_HWGUY) || (m_iClass == TF_CLASS_DEMOMAN) || (m_iClass == TF_CLASS_SOLDIER) || (m_iClass == TF_CLASS_PYRO))
		fDefendFlagUtility = bot_defrate.GetFloat() - RandomFloat(0.0f, fDefendFlagUtility);
	else if (m_iClass == TF_CLASS_MEDIC)
		fDefendFlagUtility = fGetFlagUtility;
	else if (m_iClass == TF_CLASS_SPY)
		fDefendFlagUtility = 0.0f;

	if (HasSomeConditions(CONDITION_PUSH) || CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict))
	{
		fGetFlagUtility *= 2;
		fDefendFlagUtility *= 2;
	}

	// recently saw an enemy go near the point
	if (HasSomeConditions(CONDITION_DEFENSIVE) && (m_pLastEnemy.Get() != NULL) && CBotGlobals::EntityIsAlive(m_pLastEnemy.Get()) && (m_fLastSeeEnemy > 0))
		fDefendFlagUtility = fGetFlagUtility + 0.1f;

	if (m_iClass == TF_CLASS_ENGINEER)
	{
		if (m_bIsCarryingObj)
			fDefendFlagUtility = 0.0f;
	}

	ADD_UTILITY_DATA(BOT_UTIL_GOTORESUPPLY_FOR_HEALTH, !CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) && !m_bIsCarryingObj && !bHasFlag && pWaypointResupply && bNeedHealth && !m_pHealthkit, 1000.0f / fResupplyDist, CWaypoints::GetWaypointIndex(pWaypointResupply));

	ADD_UTILITY(BOT_UTIL_GETAMMOKIT, bNeedAmmo && m_pAmmo, 1.0 + ((!CTeamFortress2Mod::HasRoundStarted() && CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) ? 0.5f : 0.0f));
	ADD_UTILITY(BOT_UTIL_GETHEALTHKIT, bNeedHealth && m_pHealthkit, 1.0 + ((!CTeamFortress2Mod::HasRoundStarted() && CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) ? 0.5f : 0.0f));

	ADD_UTILITY(BOT_UTIL_GETFLAG, (CTeamFortress2Mod::IsMapType(TF_MAP_CTF) || (CTeamFortress2Mod::IsMapType(TF_MAP_SD) && CTeamFortress2Mod::CanTeamPickupFlag_SD(iTeam, false))) && !bHasFlag, fGetFlagUtility);
	ADD_UTILITY(BOT_UTIL_GETFLAG_LASTKNOWN, (CTeamFortress2Mod::IsMapType(TF_MAP_CTF) || CTeamFortress2Mod::IsMapType(TF_MAP_MVM) || (CTeamFortress2Mod::IsMapType(TF_MAP_SD) && CTeamFortress2Mod::CanTeamPickupFlag_SD(iTeam, true))) && !bHasFlag && (m_fLastKnownFlagTime && (m_fLastKnownFlagTime > engine->Time())), fGetFlagUtility + 0.1);

	ADD_UTILITY(BOT_UTIL_DEFEND_FLAG, CTeamFortress2Mod::IsMapType(TF_MAP_MVM) || (CTeamFortress2Mod::IsMapType(TF_MAP_CTF) && !bHasFlag), fDefendFlagUtility + 0.1);
	ADD_UTILITY(BOT_UTIL_DEFEND_FLAG_LASTKNOWN, !bHasFlag &&
		(CTeamFortress2Mod::IsMapType(TF_MAP_CTF) || CTeamFortress2Mod::IsMapType(TF_MAP_MVM) ||
		(CTeamFortress2Mod::IsMapType(TF_MAP_SD) &&
		(CTeamFortress2Mod::GetFlagCarrierTeam() == CTeamFortress2Mod::GetEnemyTeam(iTeam)))) &&
		(m_fLastKnownTeamFlagTime && (m_fLastKnownTeamFlagTime > engine->Time())),
		fDefendFlagUtility + (RandomFloat(0.0, 0.2) - 0.1));
	ADD_UTILITY(BOT_UTIL_SNIPE, (iClass == TF_CLASS_SNIPER) && m_pWeapons->HasWeapon(TF2_WEAPON_SNIPERRIFLE) && !m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_SNIPERRIFLE))->OutOfAmmo(this) && !HasSomeConditions(CONDITION_PARANOID) && !bHasFlag && (GetHealthPercent() > 0.2f), 0.95);
	ADD_UTILITY(BOT_UTIL_SNIPE_CROSSBOW, (iClass == TF_CLASS_SNIPER) && m_pWeapons->HasWeapon(TF2_WEAPON_BOW) && !m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_BOW))->OutOfAmmo(this) && !HasSomeConditions(CONDITION_PARANOID) && !bHasFlag && (GetHealthPercent() > 0.2f), 0.95);

	ADD_UTILITY(BOT_UTIL_ROAM, true, 0.0001f);
	ADD_UTILITY_DATA(BOT_UTIL_FIND_NEAREST_HEALTH, !bHasFlag && bNeedHealth && !m_pHealthkit && pWaypointHealth, (1000.0f / fHealthDist) + ((!CTeamFortress2Mod::HasRoundStarted() && CTeamFortress2Mod::IsMapType(TF_MAP_MVM)) ? 0.5f : 0.0f), CWaypoints::GetWaypointIndex(pWaypointHealth));

	ADD_UTILITY(BOT_UTIL_FIND_MEDIC_FOR_HEALTH, (m_iClass != TF_CLASS_MEDIC) && !bHasFlag && bNeedHealth && m_pLastSeeMedic.HasSeen(10.0f), 1.0f);

	if ((m_pNearestEnemySentry.Get() != NULL) && !CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict))
	{
		CBotWeapon *pWeapon = m_pWeapons->GetPrimaryWeapon();

		ADD_UTILITY_DATA(BOT_UTIL_ATTACK_SENTRY, (DistanceFrom(m_pNearestEnemySentry) >= CWaypointLocations::REACHABLE_RANGE) && (m_iClass != TF_CLASS_SPY) && pWeapon && !pWeapon->OutOfAmmo(this) && pWeapon->PrimaryGreaterThanRange(TF2_MAX_SENTRYGUN_RANGE + 32.0f), 0.7f, ENTINDEX(m_pNearestEnemySentry.Get()));
	}
	// only attack if attack area is > 0
	ADD_UTILITY(BOT_UTIL_ATTACK_POINT, !CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) && (m_fAttackPointTime < engine->Time()) &&
		((m_iClass != TF_CLASS_SPY) || !IsDisguised()) && (m_iCurrentAttackArea>0) &&
		(CTeamFortress2Mod::IsMapType(TF_MAP_SD) || CTeamFortress2Mod::IsMapType(TF_MAP_CART) ||
		CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE) ||
		(CTeamFortress2Mod::IsMapType(TF_MAP_ARENA) && CTeamFortress2Mod::IsArenaPointOpen()) ||
		(CTeamFortress2Mod::IsMapType(TF_MAP_KOTH) && CTeamFortress2Mod::IsArenaPointOpen()) ||
		CTeamFortress2Mod::IsMapType(TF_MAP_CP) || CTeamFortress2Mod::IsMapType(TF_MAP_TC)), fGetFlagUtility);

	// only defend if defend area is > 0
	// (!CTeamFortress2Mod::isAttackDefendMap()||(m_iTeam==TF2_TEAM_RED))
	ADD_UTILITY(BOT_UTIL_DEFEND_POINT, (m_iCurrentDefendArea > 0) &&
		(CTeamFortress2Mod::IsMapType(TF_MAP_MVM) || CTeamFortress2Mod::IsMapType(TF_MAP_SD) || CTeamFortress2Mod::IsMapType(TF_MAP_CART) ||
		CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE) || CTeamFortress2Mod::IsMapType(TF_MAP_ARENA) ||
		CTeamFortress2Mod::IsMapType(TF_MAP_KOTH) || CTeamFortress2Mod::IsMapType(TF_MAP_CP) ||
		CTeamFortress2Mod::IsMapType(TF_MAP_TC)) && m_iClass != TF_CLASS_SCOUT, fDefendFlagUtility);

	ADD_UTILITY(BOT_UTIL_MEDIC_HEAL, (m_iClass == TF_CLASS_MEDIC) && (pMedigun != NULL) && pBWMediGun && pBWMediGun->HasWeapon() && !HasFlag() && m_pHeal &&
		CBotGlobals::EntityIsAlive(m_pHeal) && (GetHealFactor(m_pHeal) > 0), 0.98f);

	ADD_UTILITY(BOT_UTIL_MEDIC_HEAL_LAST, (m_iClass == TF_CLASS_MEDIC) && (pMedigun != NULL) && pBWMediGun && pBWMediGun->HasWeapon() && !HasFlag() && m_pLastHeal &&
		CBotGlobals::EntityIsAlive(m_pLastHeal) && (GetHealFactor(m_pLastHeal) > 0), 0.99f);

	ADD_UTILITY(BOT_UTIL_HIDE_FROM_ENEMY, !CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) && (m_pEnemy.Get() != NULL) && HasSomeConditions(CONDITION_SEE_CUR_ENEMY) &&
		!HasFlag() && !CTeamFortress2Mod::IsFlagCarrier(m_pEnemy) && (((m_iClass == TF_CLASS_SPY) && (!IsDisguised() && !IsCloaked())) || (((m_iClass == TF_CLASS_MEDIC) && (pMedigun != NULL) && !m_pHeal) ||
		CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEnemy))), 1.0f);

	ADD_UTILITY(BOT_UTIL_HIDE_FROM_ENEMY, (m_pEnemy.Get() != NULL) && HasSomeConditions(CONDITION_SEE_CUR_ENEMY) && (CTeamFortress2Mod::IsLosingTeam(m_iTeam) || ((m_iClass == TF_CLASS_ENGINEER) && m_bIsCarryingObj)), 1.0f);

	ADD_UTILITY(BOT_UTIL_MEDIC_FINDPLAYER, (m_iClass == TF_CLASS_MEDIC) &&
		!m_pHeal && m_pLastCalledMedic && (pMedigun != NULL) && pBWMediGun && pBWMediGun->HasWeapon() && ((m_fLastCalledMedicTime + 30.0f) > engine->Time()) &&
		((numplayersonteam > 1) &&
		(numplayersonteam > CTeamFortress2Mod::NumClassOnTeam(iTeam, GetClass()))), 0.95f);

	ADD_UTILITY(BOT_UTIL_MEDIC_FINDPLAYER_AT_SPAWN, (m_iClass == TF_CLASS_MEDIC) &&
		!m_pHeal && !m_pLastCalledMedic && (pMedigun != NULL) && pBWMediGun && pBWMediGun->HasWeapon() && m_bEntranceVectorValid && (numplayersonteam > 1) &&
		((!CTeamFortress2Mod::IsAttackDefendMap() && !CTeamFortress2Mod::HasRoundStarted()) || (numplayersonteam_alive < numplayersonteam)), 0.94f);

	if ((m_iClass == TF_CLASS_DEMOMAN) && !HasEnemy() && !CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict))
	{
		CBotWeapon *pPipe = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_PIPEBOMBS));
		CBotWeapon *pGren = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_GRENADELAUNCHER));

		if (pPipe && pPipe->HasWeapon() && !pPipe->OutOfAmmo(this) && (m_iTrapType != TF_TRAP_TYPE_ENEMY))
		{
			ADD_UTILITY_WEAPON(BOT_UTIL_PIPE_LAST_ENEMY, (m_pLastEnemy != NULL) && (DistanceFrom(m_pLastEnemy) > (BLAST_RADIUS)), 0.8f, pPipe);
			ADD_UTILITY_WEAPON(BOT_UTIL_PIPE_NEAREST_SENTRY, (m_pNearestEnemySentry != NULL) && (DistanceFrom(m_pNearestEnemySentry) > (BLAST_RADIUS)), 0.81f, pPipe);
			ADD_UTILITY_WEAPON(BOT_UTIL_PIPE_LAST_ENEMY_SENTRY, (m_pLastEnemySentry != NULL) && (DistanceFrom(m_pLastEnemySentry) > (BLAST_RADIUS)), 0.82f, pPipe);
		}
		else if (pGren && pGren->HasWeapon() && !pGren->OutOfAmmo(this))
		{
			ADD_UTILITY_WEAPON(BOT_UTIL_SPAM_LAST_ENEMY, (m_pLastEnemy != NULL) && (DistanceFrom(m_pLastEnemy) > (BLAST_RADIUS)), 0.78f, pGren);
			ADD_UTILITY_WEAPON(BOT_UTIL_SPAM_NEAREST_SENTRY, (m_pNearestEnemySentry != NULL) && (DistanceFrom(m_pNearestEnemySentry) > (BLAST_RADIUS)), 0.79f, pGren);
			ADD_UTILITY_WEAPON(BOT_UTIL_SPAM_LAST_ENEMY_SENTRY, (m_pLastEnemySentry != NULL) && (DistanceFrom(m_pLastEnemySentry) > (BLAST_RADIUS)), 0.80f, pGren);
		}
	}

	if (m_iClass == TF_CLASS_SPY)
	{
		ADD_UTILITY(BOT_UTIL_BACKSTAB, !HasFlag() && (!m_pNearestEnemySentry || (CTeamFortress2Mod::IsSentrySapped(m_pNearestEnemySentry))) && (m_fBackstabTime < engine->Time()) && (m_iClass == TF_CLASS_SPY) &&
			((m_pEnemy&& CBotGlobals::IsAlivePlayer(m_pEnemy)) ||
			(m_pLastEnemy&& CBotGlobals::IsAlivePlayer(m_pLastEnemy))),
			fGetFlagUtility + (GetHealthPercent() / 10));

		ADD_UTILITY(BOT_UTIL_SAP_ENEMY_SENTRY,
			m_pEnemy && CTeamFortress2Mod::IsSentry(m_pEnemy, CTeamFortress2Mod::GetEnemyTeam(iTeam)) && !CTeamFortress2Mod::IsSentrySapped(m_pEnemy),
			fGetFlagUtility + (GetHealthPercent() / 5));

		ADD_UTILITY(BOT_UTIL_SAP_NEAREST_SENTRY, m_pNearestEnemySentry &&
			!CTeamFortress2Mod::IsSentrySapped(m_pNearestEnemySentry),
			fGetFlagUtility + (GetHealthPercent() / 5));

		ADD_UTILITY(BOT_UTIL_SAP_LASTENEMY_SENTRY,
			m_pLastEnemy && CTeamFortress2Mod::IsSentry(m_pLastEnemy, CTeamFortress2Mod::GetEnemyTeam(iTeam)) && !CTeamFortress2Mod::IsSentrySapped(m_pLastEnemy), fGetFlagUtility + (GetHealthPercent() / 5));

		ADD_UTILITY(BOT_UTIL_SAP_LASTENEMY_SENTRY,
			m_pLastEnemySentry.Get() != NULL, fGetFlagUtility + (GetHealthPercent() / 5));
		////////////////
		// sap tele
		ADD_UTILITY(BOT_UTIL_SAP_ENEMY_TELE,
			m_pEnemy && CTeamFortress2Mod::IsTeleporter(m_pEnemy, CTeamFortress2Mod::GetEnemyTeam(iTeam)) && !CTeamFortress2Mod::IsTeleporterSapped(m_pEnemy),
			fGetFlagUtility + (GetHealthPercent() / 6));

		ADD_UTILITY(BOT_UTIL_SAP_NEAREST_TELE, m_pNearestEnemyTeleporter &&
			!CTeamFortress2Mod::IsTeleporterSapped(m_pNearestEnemyTeleporter),
			fGetFlagUtility + (GetHealthPercent() / 6));

		ADD_UTILITY(BOT_UTIL_SAP_LASTENEMY_TELE,
			m_pLastEnemy && CTeamFortress2Mod::IsTeleporter(m_pLastEnemy, CTeamFortress2Mod::GetEnemyTeam(iTeam)) && !CTeamFortress2Mod::IsTeleporterSapped(m_pLastEnemy), fGetFlagUtility + (GetHealthPercent() / 6));
		////////////////
		// sap dispenser
		ADD_UTILITY(BOT_UTIL_SAP_ENEMY_DISP,
			m_pEnemy && CTeamFortress2Mod::IsDispenser(m_pEnemy, CTeamFortress2Mod::GetEnemyTeam(iTeam)) && !CTeamFortress2Mod::IsDispenserSapped(m_pEnemy),
			fGetFlagUtility + (GetHealthPercent() / 7));

		ADD_UTILITY(BOT_UTIL_SAP_NEAREST_DISP, m_pNearestEnemyDisp &&
			!CTeamFortress2Mod::IsDispenserSapped(m_pNearestEnemyDisp),
			fGetFlagUtility + (GetHealthPercent() / 7));

		ADD_UTILITY(BOT_UTIL_SAP_LASTENEMY_DISP,
			m_pLastEnemy && CTeamFortress2Mod::IsDispenser(m_pLastEnemy, CTeamFortress2Mod::GetEnemyTeam(iTeam)) && !CTeamFortress2Mod::IsDispenserSapped(m_pLastEnemy), fGetFlagUtility + (GetHealthPercent() / 7));
	}

	//fGetFlagUtility = 0.2+randomFloat(0.0f,0.2f);

	if (CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE))
	{
		if (iTeam == TF2_TEAM_BLUE)
		{
			ADD_UTILITY(BOT_UTIL_DEFEND_PAYLOAD_BOMB, ((m_iClass != TF_CLASS_SPY) || !IsDisguised()) && (m_pDefendPayloadBomb != NULL), fDefendFlagUtility + RandomFloat(-0.1, 0.2));
			ADD_UTILITY(BOT_UTIL_PUSH_PAYLOAD_BOMB, ((m_iClass != TF_CLASS_SPY) || !IsDisguised()) && (m_pPushPayloadBomb != NULL), fGetFlagUtility + RandomFloat(-0.1, 0.2));
		}
		else
		{
			ADD_UTILITY(BOT_UTIL_DEFEND_PAYLOAD_BOMB, ((m_iClass != TF_CLASS_SPY) || !IsDisguised()) && (m_pDefendPayloadBomb != NULL), fDefendFlagUtility + RandomFloat(-0.1, 0.2));
			ADD_UTILITY(BOT_UTIL_PUSH_PAYLOAD_BOMB, ((m_iClass != TF_CLASS_SPY) || !IsDisguised()) && (m_pPushPayloadBomb != NULL), fGetFlagUtility + RandomFloat(-0.1, 0.2));
		}
	}
	else if (CTeamFortress2Mod::IsMapType(TF_MAP_CART))
	{
		if (iTeam == TF2_TEAM_BLUE)
		{
			ADD_UTILITY(BOT_UTIL_PUSH_PAYLOAD_BOMB, ((m_iClass != TF_CLASS_SPY) || !IsDisguised()) && (m_pPushPayloadBomb != NULL),
				fGetFlagUtility + (HasSomeConditions(CONDITION_PUSH) ? 0.25f : RandomFloat(-0.1f, 0.2f)));
			// Goto Payload bomb
		}
		else
		{
			// Defend Payload bomb
			ADD_UTILITY(BOT_UTIL_DEFEND_PAYLOAD_BOMB,
				((m_iClass != TF_CLASS_SPY) || !IsDisguised()) && (m_pDefendPayloadBomb != NULL), fDefendFlagUtility +
				(HasSomeConditions(CONDITION_PUSH) ? 0.25f : RandomFloat(-0.1f, 0.2f)));
		}
	}

	if ((m_iClass == TF_CLASS_DEMOMAN) && (m_iTrapType == TF_TRAP_TYPE_NONE))
	{
		CBotWeapon *pPipe = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_PIPEBOMBS));

		if (pPipe && pPipe->HasWeapon() && !pPipe->OutOfAmmo(this))
		{
			ADD_UTILITY(BOT_UTIL_DEMO_STICKYTRAP_LASTENEMY, m_pLastEnemy &&
				(m_iTrapType == TF_TRAP_TYPE_NONE) && CanDeployStickies(),
				RandomFloat(min(fDefendFlagUtility, fGetFlagUtility), max(fDefendFlagUtility, fGetFlagUtility)));

			ADD_UTILITY(BOT_UTIL_DEMO_STICKYTRAP_FLAG,
				CTeamFortress2Mod::IsMapType(TF_MAP_CTF) && !bHasFlag &&
				(!m_fLastKnownTeamFlagTime || (m_fLastKnownTeamFlagTime < engine->Time())) &&
				CanDeployStickies(),
				fDefendFlagUtility + 0.3f);

			ADD_UTILITY(BOT_UTIL_DEMO_STICKYTRAP_FLAG_LASTKNOWN,
				(CTeamFortress2Mod::IsMapType(TF_MAP_MVM) || CTeamFortress2Mod::IsMapType(TF_MAP_CTF) || (CTeamFortress2Mod::IsMapType(TF_MAP_SD) &&
				(CTeamFortress2Mod::GetFlagCarrierTeam() == CTeamFortress2Mod::GetEnemyTeam(iTeam)))) && !bHasFlag &&
				(m_fLastKnownTeamFlagTime && (m_fLastKnownTeamFlagTime > engine->Time())) &&
				CanDeployStickies(), fDefendFlagUtility + 0.4f);

			ADD_UTILITY(BOT_UTIL_DEMO_STICKYTRAP_POINT, (iTeam == TF2_TEAM_RED) && (m_iCurrentDefendArea > 0) &&
				(CTeamFortress2Mod::IsMapType(TF_MAP_MVM) || CTeamFortress2Mod::IsMapType(TF_MAP_SD) || CTeamFortress2Mod::IsMapType(TF_MAP_CART) ||
				CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE) || CTeamFortress2Mod::IsMapType(TF_MAP_ARENA) ||
				CTeamFortress2Mod::IsMapType(TF_MAP_KOTH) || CTeamFortress2Mod::IsMapType(TF_MAP_CP) ||
				CTeamFortress2Mod::IsMapType(TF_MAP_TC)) && CanDeployStickies(),
				fDefendFlagUtility + 0.4f);

			ADD_UTILITY(BOT_UTIL_DEMO_STICKYTRAP_PL,
				(CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE)) &&
				(m_pDefendPayloadBomb != NULL) && CanDeployStickies(),
				fDefendFlagUtility + 0.4f);
		}
	}

	//if ( !CTeamFortress2Mod::hasRoundStarted() && (iTeam == TF_TEAM_BLUE) )
	//{
	if (bot_messaround.GetBool())
	{
		float fMessUtil = fGetFlagUtility + 0.2f;

		if (GetClass() == TF_CLASS_MEDIC)
			fMessUtil -= RandomFloat(0.0f, 0.3f);

		ADD_UTILITY(BOT_UTIL_MESSAROUND, (GetHealthPercent() > 0.75f) && ((iTeam == TF2_TEAM_BLUE) || (!CTeamFortress2Mod::IsAttackDefendMap())) && !CTeamFortress2Mod::HasRoundStarted(), fMessUtil);
	}
	//}

	/////////////////////////////////////////////////////////
	// Work out utilities
	//////////////////////////////////////////////////////////
	utils.Execute();

	while ((next = utils.NextBest()) != NULL)
	{
		if (!m_pSchedules->IsEmpty() && bCheckCurrent)
		{
			if (m_CurrentUtil != next->GetId())
				m_pSchedules->FreeMemory();
			else
				break;
		}

		bCheckCurrent = false;

		if (ExecuteAction(next))//>getId(),pWaypointResupply,pWaypointHealth,pWaypointAmmo) )
		{
			m_CurrentUtil = next->GetId();
			// avoid trying to do same thing again and again if it fails
			m_fUtilTimes[m_CurrentUtil] = engine->Time() + 0.5f;

			utils.FreeMemory();
			return;
		}
	}

	utils.FreeMemory();
}


bool CBotTF2::CanDeployStickies()
{
	if (m_pEnemy.Get() != NULL)
	{
		if (CBotGlobals::IsAlivePlayer(m_pEnemy))
		{
			if (IsVisible(m_pEnemy))
				return false;
		}
	}

	// enough ammo???
	CBotWeapon *pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_PIPEBOMBS));

	if (pWeapon)
	{
		return (pWeapon->GetAmmo(this) >= 6);
	}

	return false;
}

#define STICKY_INIT			0
#define STICKY_SELECTWEAP	1
#define STICKY_RELOAD		2
#define STICKY_FACEVECTOR   3
#define IN_RANGE(x,low,high) ((x>low)&&(x<high))

// returns true when finished
bool CBotTF2::DeployStickies(eDemoTrapType type, Vector vStand, Vector vLocation, Vector vSpread, Vector *vPoint, int *iState, int *iStickyNum, bool *bFail, float *fTime, int wptindex)
{
	CBotWeapon *pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_PIPEBOMBS));
	int iPipesLeft = 0;
	WantToListen(false);
	m_bWantToInvestigateSound = false;

	if (pWeapon)
	{
		iPipesLeft = pWeapon->GetAmmo(this);
	}

	if (*iState == STICKY_INIT)
	{
		if (iPipesLeft < 6)
			*iStickyNum = iPipesLeft;
		else
			*iStickyNum = 6;

		*iState = 1;
	}

	if (GetCurrentWeapon() != pWeapon)
		SelectBotWeapon(pWeapon);
	else
	{
		if (*iState == 1)
		{
			*vPoint = vLocation + Vector(RandomFloat(-vSpread.x, vSpread.x), RandomFloat(-vSpread.y, vSpread.y), 0);
			*iState = 2;
		}

		if (DistanceFrom(vStand) > 70)
			SetMoveTo(vStand);
		else
			StopMoving();

		if (*iState == 2)
		{
			SetLookVector(*vPoint);
			SetLookAtTask(LOOK_VECTOR);

			if ((*fTime < engine->Time()) && (CBotGlobals::YawAngleFromEdict(m_pEdict, *vPoint) < 20))
			{
				PrimaryAttack();
				*fTime = engine->Time() + RandomFloat(1.0f, 1.5f);
				*iState = 1;
				*iStickyNum = *iStickyNum - 1;
			}
		}

		if ((*iStickyNum == 0) || (iPipesLeft == 0))
		{
			m_iTrapType = type;
			if (IN_RANGE(wptindex, 1, MAX_CONTROL_POINTS + 1))
				m_iTrapCPIndex = CTeamFortress2Mod::m_ObjectiveResource.m_WaypointAreaToIndexTranslation[wptindex];
			else
				m_iTrapCPIndex = -1;
			m_vStickyLocation = vLocation;
		}
	}

	return m_iTrapType != TF_TRAP_TYPE_NONE;
}

void CBotTF2::DetonateStickies(bool isJumping)
{
	// don't try to blow myself up unless i'm jumping
	if (isJumping || (DistanceFrom(m_vStickyLocation) > (BLAST_RADIUS / 2)))
	{
		SecondaryAttack();
		m_iTrapType = TF_TRAP_TYPE_NONE;
		m_iTrapCPIndex = -1;
	}
}

bool CBotTF2::LookAfterBuildings(float *fTime)
{
	CBotWeapon *pWeapon = GetCurrentWeapon();

	WantToListen(false);

	SetLookAtTask(LOOK_AROUND);

	if (!pWeapon)
		return false;
	else if (pWeapon->GetID() != TF2_WEAPON_WRENCH)
	{
		if (!Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_WRENCH)))
			return false;
	}

	if (m_pSentryGun)
	{
		if (m_prevSentryHealth > CClassInterface::GetSentryHealth(m_pSentryGun))
			return true;

		m_prevSentryHealth = CClassInterface::GetSentryHealth(m_pSentryGun);

		if (DistanceFrom(m_pSentryGun) > 100)
			SetMoveTo(CBotGlobals::EntityOrigin(m_pSentryGun));
		else
		{
			StopMoving();

			Duck(true); // crouch too

		}

		LookAtEdict(m_pSentryGun);
		SetLookAtTask(LOOK_EDICT); // LOOK_EDICT fix engineers not looking at their sentry

		if (*fTime < engine->Time())
		{
			m_pButtons->Tap(IN_ATTACK);
			*fTime = engine->Time() + RandomFloat(10.0f, 20.0f);
		}

	}
	/*
	if (m_pDispenser)
	{
	if (m_prevDispHealth > CClassInterface::GetDispenserHealth(m_pDispenser))
	return true;

	m_prevDispHealth = CClassInterface::GetDispenserHealth(m_pDispenser);
	}

	if (m_pTeleExit)
	{
	if (m_prevTeleExtHealth > CClassInterface::GetTeleporterHealth(m_pTeleExit))
	return true;

	m_prevTeleExtHealth = CClassInterface::GetTeleporterHealth(m_pTeleExit);
	}

	if (m_pTeleEntrance)
	{
	if (m_prevTeleEntHealth > CClassInterface::GetTeleporterHealth(m_pTeleEntrance))
	return true;

	m_prevTeleEntHealth = CClassInterface::GetTeleporterHealth(m_pTeleEntrance);
	}*/

	return false;
}

bool CBotTF2::Select_CWeapon(CWeapon *pWeapon)
{
	char cmd[128];
	CBotWeapon *pBotWeapon;

	pBotWeapon = m_pWeapons->GetWeapon(pWeapon);

	if (pBotWeapon && !pBotWeapon->HasWeapon())
		return false;
	if (pBotWeapon && !pBotWeapon->IsMelee() && pBotWeapon->CanAttack() && pBotWeapon->OutOfAmmo(this))
		return false;

	sprintf(cmd, "use %s", pWeapon->GetWeaponName());

	helpers->ClientCommand(m_pEdict, cmd);

	return true;
}

bool CBotTF2::SelectBotWeapon(CBotWeapon *pBotWeapon)
{
	CWeapon *pSelect = pBotWeapon->GetWeaponInfo();

	if (pSelect)
	{
		//int id = pSelect->getWeaponIndex();
		char cmd[128];

		sprintf(cmd, "use %s", pSelect->GetWeaponName());

		helpers->ClientCommand(m_pEdict, cmd);

		return true;
	}
	else
		FailWeaponSelect();

	return false;
}

//
// Execute a given Action
//
bool CBotTF2::ExecuteAction(CBotUtility *util)//eBotAction id, CWaypoint *pWaypointResupply, CWaypoint *pWaypointHealth, CWaypoint *pWaypointAmmo )
{
	static CWaypoint *pWaypoint;
	extern ConVar bot_move_dist;
	int id;

	id = util->GetId();
	pWaypoint = NULL;

	switch (id)
	{
	case BOT_UTIL_DEFEND_PAYLOAD_BOMB:
	{
		RemoveCondition(CONDITION_DEFENSIVE);

		if (m_pDefendPayloadBomb)
		{
			pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT, 0, m_iCurrentDefendArea, true, this);

			if (pWaypoint)
			{
				Vector org1 = pWaypoint->GetOrigin();
				Vector org2 = CBotGlobals::EntityOrigin(m_pDefendPayloadBomb);

				pWaypoint = CWaypoints::RandomWaypointGoalBetweenArea(CWaypointTypes::W_FL_DEFEND, m_iTeam, m_iCurrentDefendArea, true, this, true, &org1, &org2);

				if (pWaypoint)
				{
					m_pSchedules->Add(new CBotDefendSched(pWaypoint->GetOrigin()));
					RemoveCondition(CONDITION_PUSH);
					return true;
				}
			}
		}

		pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_DEFEND, GetTeam(), m_iCurrentDefendArea, true, this, true);

		if (pWaypoint)
		{
			m_pSchedules->Add(new CBotDefendSched(pWaypoint->GetOrigin()));
			RemoveCondition(CONDITION_PUSH);
			return true;
		}

		if (m_pDefendPayloadBomb.Get() != NULL)
		{
			m_pSchedules->Add(new CBotTF2DefendPayloadBombSched(m_pDefendPayloadBomb));
			RemoveCondition(CONDITION_PUSH);
			return true;
		}
	}
	break;
	case BOT_UTIL_PUSH_PAYLOAD_BOMB:
	{
		if (m_pPushPayloadBomb.Get() != NULL)
		{
			m_pSchedules->Add(new CBotTF2PushPayloadBombSched(m_pPushPayloadBomb));
			RemoveCondition(CONDITION_PUSH);
			return true;
		}
	}
	break;
	case BOT_UTIL_ENGI_LOOK_AFTER_SENTRY:
	{
		if (m_pSentryGun.Get() != NULL)
		{
			m_pSchedules->Add(new CBotTFEngiLookAfterSentry(m_pSentryGun));
			return true;
		}
	}
	break;
	case BOT_UTIL_DEFEND_FLAG:
		// use last known flag position
	{
		CWaypoint *pWaypoint = NULL;

		if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
		{

			pWaypoint = CTeamFortress2Mod::GetBestWaypointMVM(this, CWaypointTypes::W_FL_DEFEND);
			/*Vector vFlagLocation;

			if ( CTeamFortress2Mod::GetFlagLocation(TF2_TEAM_BLUE,&vFlagLocation) )
			{
			pWaypoint = CWaypoints::RandomWaypointGoalNearestArea(CWaypointTypes::W_FL_DEFEND,m_iTeam,0,false,this,true,&vFlagLocation);
			}*/
		}

		if (pWaypoint == NULL)
			pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_DEFEND, GetTeam(), 0, false, this, true);

		//if ( pWaypoint && randomInt(0,1) )
		//	pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),pWaypoint->getOrigin());

		if (pWaypoint)
		{
			SetLookAt(pWaypoint->GetOrigin());
			m_pSchedules->Add(new CBotDefendSched(pWaypoint->GetOrigin(), ((m_iClass == TF_CLASS_MEDIC) || HasSomeConditions(CONDITION_DEFENSIVE)) ? RandomFloat(4.0f, 8.0f) : 0.0f));
			RemoveCondition(CONDITION_DEFENSIVE);
			RemoveCondition(CONDITION_PUSH);
			return true;
		}
	}
	break;
	case BOT_UTIL_DEFEND_FLAG_LASTKNOWN:
		// find our flag waypoint
	{
		SetLookAt(m_vLastKnownTeamFlagPoint);

		m_pSchedules->Add(new CBotDefendSched(m_vLastKnownTeamFlagPoint, ((m_iClass == TF_CLASS_MEDIC) || HasSomeConditions(CONDITION_DEFENSIVE)) ? RandomFloat(4.0f, 8.0f) : 0.0f));

		RemoveCondition(CONDITION_DEFENSIVE);
		RemoveCondition(CONDITION_PUSH);
		return true;
	}
	break;
	case BOT_UTIL_ATTACK_POINT:

		/*pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_DEFEND,GetTeam(),m_iCurrentAttackArea,true);

		if ( pWaypoint )
		{
		m_pSchedules->Add(new CBotDefendSched(pWaypoint->GetOrigin()));
		return true;
		}*/

		pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT, 0, m_iCurrentAttackArea, true, this);

		if (pWaypoint && pWaypoint->CheckReachable())
		{
			CWaypoint *pRoute = NULL;
			Vector vRoute = Vector(0, 0, 0);
			bool bUseRoute = false;
			int iRouteWpt = -1;
			bool bNest = false;

			if ((m_fUseRouteTime < engine->Time()))
			{
				// find random route
				pRoute = CWaypoints::RandomRouteWaypoint(this, GetOrigin(), pWaypoint->GetOrigin(), GetTeam(), m_iCurrentAttackArea);

				if (pRoute)
				{
					bUseRoute = true;
					vRoute = pRoute->GetOrigin();
					m_fUseRouteTime = engine->Time() + RandomFloat(30.0f, 60.0f);
					iRouteWpt = CWaypoints::GetWaypointIndex(pRoute);

					bNest = ((m_pNavigator->GetBelief(iRouteWpt) / MAX_BELIEF) + (1.0f - GetHealthPercent()) > 0.75f);
				}
			}

			m_pSchedules->Add(new CBotAttackPointSched(pWaypoint->GetOrigin(), pWaypoint->GetRadius(), pWaypoint->GetArea(), bUseRoute, vRoute, bNest, m_pLastEnemySentry.Get()));
			RemoveCondition(CONDITION_PUSH);
			return true;
		}
		break;
	case BOT_UTIL_DEFEND_POINT:
	{
		float fprob;

		if ((CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE) || CTeamFortress2Mod::IsMapType(TF_MAP_CART)))
		{
			if (m_pDefendPayloadBomb != NULL)
			{
				static const float fSearchDist = 1500.0f;
				Vector vPayloadBomb = CBotGlobals::EntityOrigin(m_pDefendPayloadBomb);
				CWaypoint *pCapturePoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(vPayloadBomb, fSearchDist, -1, false, false, true, NULL, false, 0, true, false, Vector(0, 0, 0), CWaypointTypes::W_FL_CAPPOINT));

				if (pCapturePoint)
				{
					float fDistance = pCapturePoint->DistanceFrom(vPayloadBomb);

					if (fDistance == 0)
						fprob = 1.0f;
					else
						fprob = 1.0f - (fDistance / fSearchDist);
				}
				else // no where near the capture point 
				{
					extern ConVar bot_defrate;

					fprob = bot_defrate.GetFloat();
				}
			}
			else
			{
				fprob = 0.05f;
			}
		}
		else
		{
			int capindex = CTeamFortress2Mod::m_ObjectiveResource.m_WaypointAreaToIndexTranslation[m_iCurrentDefendArea];
			int enemyteam = CTeamFortress2Mod::GetEnemyTeam(m_iTeam);
			if (CTeamFortress2Mod::m_ObjectiveResource.GetCappingTeam(capindex) == enemyteam)
			{
				if (CTeamFortress2Mod::m_ObjectiveResource.GetNumPlayersInArea(capindex, enemyteam) > 0)
					fprob = 1.0f;
				else
					fprob = 0.9f;
			}
			else
			{

				float fTime = bot_tf2_protect_cap_time.GetFloat();
				// chance of going to point
				fprob = (fTime - (engine->Time() - CTeamFortress2Mod::m_ObjectiveResource.GetLastCaptureTime(m_iCurrentDefendArea))) / fTime;

				if (fprob < bot_tf2_protect_cap_percent.GetFloat())
					fprob = bot_tf2_protect_cap_percent.GetFloat();
			}
		}


		if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM) && CTeamFortress2Mod::TF2_IsPlayerInvuln(m_pEdict) && HasEnemy())
		{
			// move towards enemy if invuln
			pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(m_pEnemy), 1000.0f, -1));
			fprob = 1.0f;
		}
		else
			pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT, 0, m_iCurrentDefendArea, true, this);

		if (!pWaypoint->CheckReachable() || (RandomFloat(0.0, 1.0f) > fprob))
		{
			pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_DEFEND, GetTeam(), m_iCurrentDefendArea, true, this, false);

			if (pWaypoint)
			{
				if ((m_iClass == TF_CLASS_DEMOMAN) && WantToShoot() && RandomInt(0, 1)) //(m_fLastSeeEnemy + 30.0f > engine->Time()) )
				{
					CBotWeapon *pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_GRENADELAUNCHER));

					if (pWeapon && !pWeapon->OutOfAmmo(this))
					{
						CBotTF2Spam *spam = new CBotTF2Spam(this, pWaypoint->GetOrigin(), pWaypoint->GetAimYaw(), pWeapon);

						if (spam->GetDistance() > 600)
						{
							CFindPathTask *path = new CFindPathTask(CWaypoints::GetWaypointIndex(pWaypoint));

							CBotSchedule *newSched = new CBotSchedule();

							newSched->PassVector(spam->GetTarget());

							newSched->AddTask(path);
							newSched->AddTask(spam);
							return true;
						}
						else
							delete spam;
					}
				}

				m_pSchedules->Add(new CBotDefendSched(pWaypoint->GetOrigin(), (HasSomeConditions(CONDITION_DEFENSIVE) || (m_iClass == TF_CLASS_MEDIC)) ? RandomFloat(5.0f, 10.0f) : 0.0f));
				RemoveCondition(CONDITION_PUSH);

				RemoveCondition(CONDITION_DEFENSIVE);

				return true;
			}
		}

		if (pWaypoint)
		{
			m_pSchedules->Add(new CBotDefendPointSched(pWaypoint->GetOrigin(), pWaypoint->GetRadius(), pWaypoint->GetArea()));
			RemoveCondition(CONDITION_PUSH);
			RemoveCondition(CONDITION_DEFENSIVE);
			return true;
		}
	}
	break;
	case BOT_UTIL_CAPTURE_FLAG:
		pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT, GetTeam());

		if (pWaypoint)
		{
			m_pSchedules->Add(new CBotGotoOriginSched(pWaypoint->GetOrigin()));
			RemoveCondition(CONDITION_PUSH);
			return true;
		}
		break;
	case BOT_UTIL_ENGI_DESTROY_ENTRANCE: // destroy and rebuild sentry elsewhere
		EngineerBuild(ENGI_ENTRANCE, ENGI_DESTROY);
	case BOT_UTIL_BUILDTELENT:
		pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(m_vTeleportEntrance, 300, -1, true, false, true, NULL, false, GetTeam(), true));

		if (pWaypoint)
		{
			m_pSchedules->Add(new CBotTFEngiBuild(this, ENGI_ENTRANCE, pWaypoint));
			m_iTeleEntranceArea = pWaypoint->GetArea();
			return true;
		}

		break;
	case BOT_UTIL_BUILDTELENT_SPAWN:
	{
		Vector vOrigin = GetOrigin();

		pWaypoint = CWaypoints::GetWaypoint(CWaypoints::NearestWaypointGoal(CWaypointTypes::W_FL_TELE_ENTRANCE, vOrigin, 4096.0f, GetTeam()));

		if (pWaypoint)
		{
			CBotTFEngiBuildTask *buildtask = new CBotTFEngiBuildTask(ENGI_ENTRANCE, pWaypoint);

			CBotSchedule *newSched = new CBotSchedule();

			newSched->AddTask(new CFindPathTask(CWaypoints::GetWaypointIndex(pWaypoint))); // first
			newSched->AddTask(buildtask);
			newSched->AddTask(new CFindPathTask(util->GetIntData()));
			buildtask->OneTryOnly();

			m_pSchedules->Add(newSched);
			m_iTeleEntranceArea = pWaypoint->GetArea();
			return true;
		}
	}

	break;
	case BOT_UTIL_ATTACK_SENTRY:
	{
		CBotWeapon *pWeapon = m_pWeapons->GetPrimaryWeapon();

		edict_t *pSentry = INDEXENT(util->GetIntData());

		m_pSchedules->Add(new CBotTF2AttackSentryGun(pSentry, pWeapon));
	}
	break;
	case BOT_UTIL_ENGI_DESTROY_EXIT: // destroy and rebuild sentry elsewhere
		EngineerBuild(ENGI_EXIT, ENGI_DESTROY);
	case BOT_UTIL_BUILDTELEXT:

		if (m_bTeleportExitVectorValid)
		{
			pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(m_vTeleportExit, 150, -1, true, false, true, NULL, false, GetTeam(), true, false, Vector(0, 0, 0), CWaypointTypes::W_FL_TELE_EXIT));

			if (CWaypoints::GetWaypointIndex(pWaypoint) == m_iLastFailTeleExitWpt)
			{
				pWaypoint = NULL;
				m_bTeleportExitVectorValid = false;
			}
			// no use going back to this waypoint
			else if (pWaypoint && (pWaypoint->GetArea() > 0) && (pWaypoint->GetArea() != m_iCurrentAttackArea) && (pWaypoint->GetArea() != m_iCurrentDefendArea))
			{
				pWaypoint = NULL;
				m_bTeleportExitVectorValid = false;
			}
		}

		if (pWaypoint == NULL)
		{
			if (CTeamFortress2Mod::IsAttackDefendMap())
			{
				if (GetTeam() == TF2_TEAM_BLUE)
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_TELE_EXIT, GetTeam(), m_iCurrentAttackArea, true, this, false);
				else
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_TELE_EXIT, GetTeam(), m_iCurrentDefendArea, true, this, false);
			}

			if (!pWaypoint)
			{
				int area = (RandomInt(0, 1) == 1) ? m_iCurrentAttackArea : m_iCurrentDefendArea;

				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_TELE_EXIT, GetTeam(), area, true, this, false, 0, m_iLastFailTeleExitWpt);//CTeamFortress2Mod::GetArea());

				if (!pWaypoint)
				{
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_TELE_EXIT, GetTeam(), 0, false, this, false, 0, m_iLastFailTeleExitWpt);//CTeamFortress2Mod::GetArea());

					if (!pWaypoint)
						m_iLastFailTeleExitWpt = -1;
				}
			}
		}

		if (pWaypoint)
		{
			m_bTeleportExitVectorValid = true;
			m_vTeleportExit = pWaypoint->GetOrigin() + pWaypoint->ApplyRadius();
			UpdateCondition(CONDITION_COVERT); // sneak around to get there
			m_pSchedules->Add(new CBotTFEngiBuild(this, ENGI_EXIT, pWaypoint));
			m_iTeleExitArea = pWaypoint->GetArea();
			m_iLastFailTeleExitWpt = CWaypoints::GetWaypointIndex(pWaypoint);
			return true;
		}

		break;
	case BOT_UTIL_ENGI_DESTROY_SENTRY: // destroy and rebuild sentry elsewhere
		EngineerBuild(ENGI_SENTRY, ENGI_DESTROY);
	case BOT_UTIL_BUILDSENTRY:

		pWaypoint = NULL;

		// did someone destroy my sentry at the last sentry point? -- build it again
		if (m_bSentryGunVectorValid)
		{
			pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(m_vSentryGun, 150, m_iLastFailSentryWpt, true, false, true, NULL, false, GetTeam(), true, false, Vector(0, 0, 0), CWaypointTypes::W_FL_SENTRY));

			if (pWaypoint && CTeamFortress2Mod::BuildingNearby(m_iTeam, pWaypoint->GetOrigin()))
				pWaypoint = NULL;
			// no use going back to this waypoint
			if (pWaypoint && (pWaypoint->GetArea() > 0) && (pWaypoint->GetArea() != m_iCurrentAttackArea) && (pWaypoint->GetArea() != m_iCurrentDefendArea))
				pWaypoint = NULL;
		}

		if (pWaypoint == NULL)
		{
			if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
			{
				pWaypoint = CTeamFortress2Mod::GetBestWaypointMVM(this, CWaypointTypes::W_FL_SENTRY);
				/*
				Vector vFlagLocation;

				if ( CTeamFortress2Mod::getFlagLocation(TF2_TEAM_BLUE,&vFlagLocation) )
				{
				pWaypoint = CWaypoints::randomWaypointGoalNearestArea(CWaypointTypes::W_FL_SENTRY,m_iTeam,0,false,this,true,&vFlagLocation);
				}*/
			}
			else if (CTeamFortress2Mod::IsAttackDefendMap())
			{
				if (GetTeam() == TF2_TEAM_BLUE)
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SENTRY, GetTeam(), m_iCurrentAttackArea, true, this, false, WPT_SEARCH_AVOID_SENTRIES, m_iLastFailSentryWpt);
				else
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SENTRY, GetTeam(), m_iCurrentDefendArea, true, this, true, WPT_SEARCH_AVOID_SENTRIES, m_iLastFailSentryWpt);
			}

			if (!pWaypoint)
			{
				int area = (RandomInt(0, 1) == 1) ? m_iCurrentAttackArea : m_iCurrentDefendArea;
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SENTRY, GetTeam(), area, true, this, area == m_iCurrentDefendArea, WPT_SEARCH_AVOID_SENTRIES, m_iLastFailSentryWpt);

				if (!pWaypoint)
				{
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SENTRY, GetTeam(), 0, false, this, true, WPT_SEARCH_AVOID_SENTRIES, m_iLastFailSentryWpt);
				}
			}

		}

		if (pWaypoint)
		{
			m_iLastFailSentryWpt = CWaypoints::GetWaypointIndex(pWaypoint);
			m_vSentryGun = pWaypoint->GetOrigin() + pWaypoint->ApplyRadius();
			m_bSentryGunVectorValid = true;
			UpdateCondition(CONDITION_COVERT); // sneak around to get there
			m_pSchedules->Add(new CBotTFEngiBuild(this, ENGI_SENTRY, pWaypoint));
			m_iSentryArea = pWaypoint->GetArea();
			return true;
		}
		else
			m_iLastFailSentryWpt = -1;
		break;
	case BOT_UTIL_BACKSTAB:
		if (m_pEnemy  && CBotGlobals::IsAlivePlayer(m_pEnemy))
		{
			m_pSchedules->Add(new CBotBackstabSched(m_pEnemy));
			return true;
		}
		else if (m_pLastEnemy &&  CBotGlobals::IsAlivePlayer(m_pLastEnemy))
		{
			m_pSchedules->Add(new CBotBackstabSched(m_pLastEnemy));
			return true;
		}

		break;
	case  BOT_UTIL_REMOVE_TMTELE_SAPPER:
		UpdateCondition(CONDITION_PARANOID);
		m_pSchedules->Add(new CBotRemoveSapperSched(m_pNearestTeleEntrance, ENGI_TELE));
		return true;

	case BOT_UTIL_REMOVE_SENTRY_SAPPER:
		UpdateCondition(CONDITION_PARANOID);
		m_pSchedules->Add(new CBotRemoveSapperSched(m_pSentryGun, ENGI_SENTRY));
		return true;

	case BOT_UTIL_REMOVE_DISP_SAPPER:
		UpdateCondition(CONDITION_PARANOID);
		m_pSchedules->Add(new CBotRemoveSapperSched(m_pDispenser, ENGI_DISP));
		return true;

	case BOT_UTIL_REMOVE_TMSENTRY_SAPPER:
		UpdateCondition(CONDITION_PARANOID);
		m_pSchedules->Add(new CBotRemoveSapperSched(m_pNearestAllySentry, ENGI_SENTRY));
		return true;

		break;
	case BOT_UTIL_REMOVE_TMDISP_SAPPER:
		UpdateCondition(CONDITION_PARANOID);
		m_pSchedules->Add(new CBotRemoveSapperSched(m_pNearestDisp, ENGI_DISP));
		return true;

		break;
	case BOT_UTIL_ENGI_DESTROY_DISP:
		EngineerBuild(ENGI_DISP, ENGI_DESTROY);
	case BOT_UTIL_BUILDDISP:
		pWaypoint = NULL;
		if (m_bDispenserVectorValid)
		{
			pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(m_vDispenser, 150, -1, true, false, true, NULL, false, GetTeam(), true, false, Vector(0, 0, 0), CWaypointTypes::W_FL_SENTRY));

			// no use going back to this waypoint
			if (pWaypoint && (pWaypoint->GetArea() > 0) && (pWaypoint->GetArea() != m_iCurrentAttackArea) && (pWaypoint->GetArea() != m_iCurrentDefendArea))
				pWaypoint = NULL;
		}
		else if (m_pSentryGun.Get() != NULL)
			pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(m_pSentryGun), 150, -1, true, false, true, NULL, false, GetTeam(), true));

		if (pWaypoint)
		{
			m_vDispenser = pWaypoint->GetOrigin();
			m_bDispenserVectorValid = true;
			UpdateCondition(CONDITION_COVERT);
			m_pSchedules->Add(new CBotTFEngiBuild(this, ENGI_DISP, pWaypoint));
			m_iDispenserArea = pWaypoint->GetArea();
			return true;
		}
		break;
	case BOT_UTIL_HIDE_FROM_ENEMY:
	{
		CBotSchedule *pSchedule = new CBotSchedule();

		pSchedule->SetID(SCHED_GOOD_HIDE_SPOT);

		// run at flank while shooting	
		CFindPathTask *pHideGoalPoint = new CFindPathTask();
		Vector vOrigin = CBotGlobals::EntityOrigin(m_pEnemy);


		pSchedule->AddTask(new CFindGoodHideSpot(vOrigin));
		pSchedule->AddTask(pHideGoalPoint);
		pSchedule->AddTask(new CBotNest());


		// no interrupts, should be a quick waypoint path anyway
		pHideGoalPoint->SetNoInterruptions();
		// get vector from good hide spot task
		pHideGoalPoint->GetPassedVector();
		// Makes sure bot stopes trying to cover if ubered
		pHideGoalPoint->SetInterruptFunction(new CBotTF2CoverInterrupt());
		//pSchedule->setID(SCHED_HIDE_FROM_ENEMY);

		m_pSchedules->RemoveSchedule(SCHED_GOOD_HIDE_SPOT);
		m_pSchedules->AddFront(pSchedule);

		return true;
	}
	case BOT_UTIL_MEDIC_HEAL:
		if (m_pHeal)
		{
			m_pSchedules->Add(new CBotTF2HealSched(m_pHeal));
			return true;
		}
	case BOT_UTIL_MEDIC_HEAL_LAST:
		if (m_pLastHeal)
		{
			m_pSchedules->Add(new CBotTF2HealSched(m_pLastHeal));
			return true;
		}
	case BOT_UTIL_UPGTMSENTRY:
		if (m_pNearestAllySentry)
		{
			m_pSchedules->Add(new CBotTFEngiUpgrade(this, m_pNearestAllySentry));
			return true;
		}
	case BOT_UTIL_UPGTMDISP:
		if (m_pNearestDisp)
		{
			m_pSchedules->Add(new CBotTFEngiUpgrade(this, m_pNearestDisp));
			return true;
		}
	case BOT_UTIL_UPGSENTRY:
		if (m_pSentryGun)
		{
			m_pSchedules->Add(new CBotTFEngiUpgrade(this, m_pSentryGun));
			return true;
		}
	case BOT_UTIL_UPGTELENT:
		if (m_pTeleEntrance)
		{
			m_pSchedules->Add(new CBotTFEngiUpgrade(this, m_pTeleEntrance));
			return true;
		}
	case BOT_UTIL_UPGTELEXT:
		if (m_pTeleExit)
		{
			m_pSchedules->Add(new CBotTFEngiUpgrade(this, m_pTeleExit));
			return true;
		}
	case BOT_UTIL_UPGDISP:
		if (m_pDispenser)
		{
			m_pSchedules->Add(new CBotTFEngiUpgrade(this, m_pDispenser));
			return true;
		}
	case BOT_UTIL_GETAMMODISP:
		if (m_pDispenser)
		{
			m_pSchedules->Add(new CBotGetMetalSched(CBotGlobals::EntityOrigin(m_pDispenser)));
			return true;
		}
	case BOT_UTIL_GOTORESUPPLY_FOR_HEALTH:
	{
		CWaypoint *pWaypointResupply = CWaypoints::GetWaypoint(util->GetIntData());

		m_pSchedules->Add(new CBotTF2GetHealthSched(pWaypointResupply->GetOrigin()));
	}
	return true;
	case BOT_UTIL_GOTORESUPPLY_FOR_AMMO:
	{
		CWaypoint *pWaypointResupply = CWaypoints::GetWaypoint(util->GetIntData());

		m_pSchedules->Add(new CBotTF2GetAmmoSched(pWaypointResupply->GetOrigin()));
	}
	return true;
	case BOT_UTIL_FIND_NEAREST_HEALTH:
	{
		CWaypoint *pWaypointHealth = CWaypoints::GetWaypoint(util->GetIntData());

		m_pSchedules->Add(new CBotTF2GetHealthSched(pWaypointHealth->GetOrigin()));
	}
	return true;
	case BOT_UTIL_FIND_NEAREST_AMMO:
	{
		CWaypoint *pWaypointAmmo = CWaypoints::GetWaypoint(util->GetIntData());

		m_pSchedules->Add(new CBotTF2GetAmmoSched(pWaypointAmmo->GetOrigin()));
	}
	return true;
	case BOT_UTIL_GOTODISP:
		m_pSchedules->RemoveSchedule(SCHED_USE_DISPENSER);
		m_pSchedules->AddFront(new CBotUseDispSched(this, m_pNearestDisp));

		m_fPickupTime = engine->Time() + RandomFloat(6.0f, 20.0f);
		return true;
	case BOT_UTIL_ENGI_MOVE_SENTRY:
		if (m_pSentryGun.Get())
		{
			Vector vSentry = CBotGlobals::EntityOrigin(m_pSentryGun);

			CWaypoint *pWaypoint = NULL;

			if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
			{
				pWaypoint = CTeamFortress2Mod::GetBestWaypointMVM(this, CWaypointTypes::W_FL_SENTRY);
				/*
				Vector vFlagLocation;

				if ( CTeamFortress2Mod::GetFlagLocation(TF2_TEAM_BLUE, &vFlagLocation) )
				{
				pWaypoint = CWaypoints::RandomWaypointGoalNearestArea(CWaypointTypes::W_FL_SENTRY ,m_iTeam, 0, false, this, true, &vFlagLocation);
				}*/
			}
			else if (CTeamFortress2Mod::IsAttackDefendMap())
			{
				if (GetTeam() == TF2_TEAM_BLUE)
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SENTRY, GetTeam(), m_iCurrentAttackArea, true, this, false, WPT_SEARCH_AVOID_SENTRIES, m_iLastFailSentryWpt);
				else
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SENTRY, GetTeam(), m_iCurrentDefendArea, true, this, true, WPT_SEARCH_AVOID_SENTRIES, m_iLastFailSentryWpt);
			}

			if (!pWaypoint)
			{
				int iCappingTeam = 0;
				bool bAllowAttack = ((m_iCurrentAttackArea == 0) ||
					((iCappingTeam = CTeamFortress2Mod::m_ObjectiveResource.GetCappingTeam(
					CTeamFortress2Mod::m_ObjectiveResource.m_WaypointAreaToIndexTranslation[m_iCurrentAttackArea])) != CTeamFortress2Mod::GetEnemyTeam(m_iTeam)));

				int area = 0;

				if (bAllowAttack)
				{
					if (iCappingTeam == m_iTeam) // Move Up Our team is attacking!!!
						area = m_iCurrentAttackArea;
					else
						area = (RandomInt(0, 1) == 1) ? m_iCurrentAttackArea : m_iCurrentDefendArea;
				}
				else
					area = m_iCurrentDefendArea;

				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SENTRY, GetTeam(), area, true, this, area == m_iCurrentDefendArea, WPT_SEARCH_AVOID_SENTRIES, m_iLastFailSentryWpt);

				if (!pWaypoint)
				{
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SENTRY, GetTeam(), 0, false, this, true, WPT_SEARCH_AVOID_SENTRIES, m_iLastFailSentryWpt);
				}
			}


			if (pWaypoint && (pWaypoint->DistanceFrom(vSentry) > bot_move_dist.GetFloat()))
			{
				UpdateCondition(CONDITION_COVERT);
				m_pSchedules->Add(new CBotEngiMoveBuilding(m_pEdict, m_pSentryGun.Get(), ENGI_SENTRY, pWaypoint->GetOrigin(), m_bIsCarryingSentry));
				m_iSentryArea = pWaypoint->GetArea();
				return true;
			}
			//else
			//	DestroySentry();
		}
		return false;
	case BOT_UTIL_SPYCHECK_AIR:
		m_pSchedules->Add(new CBotSchedule(new CSpyCheckAir()));
		return true;
	case BOT_UTIL_PLACE_BUILDING:
		if (m_bIsCarryingObj)
		{
			PrimaryAttack(); // just press attack to place
			return true;
		}
		return false;
	case BOT_UTIL_ENGI_MOVE_DISP:
		if (m_pSentryGun.Get() && m_pDispenser.Get())
		{
			Vector vDisp = CBotGlobals::EntityOrigin(m_pDispenser);
			CWaypoint *pWaypoint = NULL;

			if (m_pSentryGun)
				pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(m_pSentryGun), 150, -1, true, false, true, NULL, false, GetTeam(), true));
			else
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SENTRY, GetTeam(), 0, false, this);

			if (pWaypoint && (pWaypoint->DistanceFrom(vDisp) > bot_move_dist.GetFloat()))
			{
				UpdateCondition(CONDITION_COVERT);
				m_pSchedules->Add(new CBotEngiMoveBuilding(m_pEdict, m_pDispenser.Get(), ENGI_DISP, pWaypoint->GetOrigin(), m_bIsCarryingDisp));
				m_iDispenserArea = pWaypoint->GetArea();
				return true;
			}
		}
		return false;
	case BOT_UTIL_ENGI_MOVE_ENTRANCE:

		if (m_pTeleEntrance.Get())
		{
			Vector vTele = CBotGlobals::EntityOrigin(m_pTeleEntrance);
			CWaypoint *pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(m_vTeleportEntrance, 512, -1, true, false, true, NULL, false, GetTeam(), true));

			if (pWaypoint && (pWaypoint->DistanceFrom(vTele) > bot_move_dist.GetFloat()))
			{
				UpdateCondition(CONDITION_COVERT);
				m_pSchedules->Add(new CBotEngiMoveBuilding(m_pEdict, m_pTeleEntrance.Get(), ENGI_ENTRANCE, pWaypoint->GetOrigin(), m_bIsCarryingTeleEnt));
				m_iTeleEntranceArea = pWaypoint->GetArea();
				return true;
			}
		}

		return false;
	case BOT_UTIL_ENGI_MOVE_EXIT:

		if (m_pTeleExit.Get())
		{
			Vector vTele = CBotGlobals::EntityOrigin(m_pTeleExit);

			if (CTeamFortress2Mod::IsAttackDefendMap())
			{
				if (GetTeam() == TF2_TEAM_BLUE)
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_TELE_EXIT, GetTeam(), m_iCurrentAttackArea, true, this, true);
				else
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_TELE_EXIT, GetTeam(), m_iCurrentDefendArea, true, this, true);
			}

			if (!pWaypoint)
			{
				int area = (RandomInt(0, 1) == 1) ? m_iCurrentAttackArea : m_iCurrentDefendArea;

				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_TELE_EXIT, GetTeam(), area, true, this);//CTeamFortress2Mod::GetArea());

				if (!pWaypoint)
				{
					pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_TELE_EXIT, GetTeam(), 0, false, this);//CTeamFortress2Mod::GetArea());
				}
			}

			if (pWaypoint && (pWaypoint->DistanceFrom(vTele) > bot_move_dist.GetFloat()))
			{
				UpdateCondition(CONDITION_COVERT);
				m_pSchedules->Add(new CBotEngiMoveBuilding(m_pEdict, m_pTeleExit.Get(), ENGI_EXIT, pWaypoint->GetOrigin(), m_bIsCarryingTeleExit));
				m_iTeleExitArea = pWaypoint->GetArea();
				return true;
			}
		}
		return false;
	case BOT_UTIL_FIND_MEDIC_FOR_HEALTH:
	{
		Vector vLoc = m_pLastSeeMedic.GetLocation();
		int iWpt = CWaypointLocations::NearestWaypoint(vLoc, 400, -1, true, false, true, 0, false, GetTeam(), true);
		if (iWpt != -1)
		{
			CFindPathTask *findpath = new CFindPathTask(iWpt, LOOK_WAYPOINT);
			CTaskVoiceCommand *shoutMedic = new CTaskVoiceCommand(TF_VC_MEDIC);
			CBotTF2WaitHealthTask *wait = new CBotTF2WaitHealthTask(vLoc);
			CBotSchedule *newSched = new CBotSchedule();

			findpath->SetCompleteInterrupt(0, CONDITION_NEED_HEALTH);
			shoutMedic->SetCompleteInterrupt(0, CONDITION_NEED_HEALTH);
			wait->SetCompleteInterrupt(0, CONDITION_NEED_HEALTH);

			newSched->AddTask(findpath);
			newSched->AddTask(shoutMedic);
			newSched->AddTask(wait);
			m_pSchedules->AddFront(newSched);

			return true;
		}

		return false;
	}
	case BOT_UTIL_GETHEALTHKIT:
		m_pSchedules->RemoveSchedule(SCHED_PICKUP);
		m_pSchedules->AddFront(new CBotPickupSched(m_pHealthkit));

		m_fPickupTime = engine->Time() + RandomFloat(5.0f, 10.0f);

		return true;
	case BOT_UTIL_DEMO_STICKYTRAP_LASTENEMY:
	case BOT_UTIL_DEMO_STICKYTRAP_FLAG:
	case BOT_UTIL_DEMO_STICKYTRAP_FLAG_LASTKNOWN:
	case BOT_UTIL_DEMO_STICKYTRAP_POINT:
	case BOT_UTIL_DEMO_STICKYTRAP_PL:
		// to do
	{

		Vector vStand;
		Vector vPoint;
		Vector vDemoStickyPoint;
		eDemoTrapType iDemoTrapType = TF_TRAP_TYPE_NONE;

		if (id == BOT_UTIL_DEMO_STICKYTRAP_LASTENEMY)
		{
			pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(m_vLastSeeEnemy, 400, -1, true, false, true, 0, false, GetTeam(), true));
			iDemoTrapType = TF_TRAP_TYPE_WPT;
		}
		else if (id == BOT_UTIL_DEMO_STICKYTRAP_FLAG)
		{
			pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_FLAG, CTeamFortress2Mod::GetEnemyTeam(GetTeam()));
			iDemoTrapType = TF_TRAP_TYPE_FLAG;
		}
		else if (id == BOT_UTIL_DEMO_STICKYTRAP_FLAG_LASTKNOWN)
		{
			pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(m_vLastKnownTeamFlagPoint, 400, -1, true, false, true, 0, false, GetTeam(), true));
			iDemoTrapType = TF_TRAP_TYPE_FLAG;
		}
		else if (id == BOT_UTIL_DEMO_STICKYTRAP_POINT)
		{
			if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT);
			else
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT, 0, m_iCurrentDefendArea, true);

			iDemoTrapType = TF_TRAP_TYPE_POINT;
		}
		else if (id == BOT_UTIL_DEMO_STICKYTRAP_PL)
		{
			pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(m_pDefendPayloadBomb), 400, -1, true, false, true, 0, false, GetTeam(), true));
			iDemoTrapType = TF_TRAP_TYPE_PL;
		}

		if (pWaypoint)
		{
			CWaypoint *pStand = NULL;
			CWaypoint *pTemp;
			float fDist = 9999.0f;
			float fClosest = 9999.0f;

			vPoint = pWaypoint->GetOrigin();

			dataUnconstArray<int> m_iVisibles;
			dataUnconstArray<int> m_iInvisibles;

			int iWptFrom = CWaypointLocations::NearestWaypoint(vPoint, 2048.0, -1, true, true, true, NULL, false, 0, false);

			//int m_iVisiblePoints[CWaypoints::MAX_WAYPOINTS]; // make searching quicker

			CWaypointLocations::GetAllVisible(iWptFrom, iWptFrom, vPoint, vPoint, 2048.0, &m_iVisibles, &m_iInvisibles);

			for (int i = 0; i < m_iVisibles.Size(); i++)
			{
				if (m_iVisibles[i] == CWaypoints::GetWaypointIndex(pWaypoint))
					continue;

				pTemp = CWaypoints::GetWaypoint(m_iVisibles[i]);

				if (pTemp->DistanceFrom(pWaypoint) < 512)
				{
					fDist = DistanceFrom(pTemp->GetOrigin());

					if (fDist < fClosest)
					{
						fClosest = fDist;
						pStand = pTemp;
					}
				}
			}

			m_iVisibles.Destroy();
			m_iInvisibles.Destroy();

			if (!pStand)
			{
				pStand = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(pWaypoint->GetOrigin(), 400, CWaypoints::GetWaypointIndex(pWaypoint), true, false, true, 0, false, GetTeam(), true));
			}

			if (pStand)
			{
				vStand = pStand->GetOrigin();

				if (pWaypoint)
				{
					m_pSchedules->Add(new CBotTF2DemoPipeTrapSched(iDemoTrapType, vStand, vPoint, Vector(150, 150, 20), false, pWaypoint->GetArea()));
					return true;
				}
			}

		}
	}
	break;
	// dispenser
	case  BOT_UTIL_SAP_NEAREST_DISP:
		m_pSchedules->Add(new CBotSpySapBuildingSched(m_pNearestEnemyDisp, ENGI_DISP));
		return true;
	case BOT_UTIL_SAP_ENEMY_DISP:
		m_pSchedules->Add(new CBotSpySapBuildingSched(m_pEnemy, ENGI_DISP));
		return true;
	case BOT_UTIL_SAP_LASTENEMY_DISP:
		m_pSchedules->Add(new CBotSpySapBuildingSched(m_pLastEnemy, ENGI_DISP));
		return true;
		// Teleporter
	case  BOT_UTIL_SAP_NEAREST_TELE:
		m_pSchedules->Add(new CBotSpySapBuildingSched(m_pNearestEnemyTeleporter, ENGI_TELE));
		return true;
	case BOT_UTIL_SAP_ENEMY_TELE:
		m_pSchedules->Add(new CBotSpySapBuildingSched(m_pEnemy, ENGI_TELE));
		return true;
	case BOT_UTIL_SAP_LASTENEMY_TELE:
		m_pSchedules->Add(new CBotSpySapBuildingSched(m_pLastEnemy, ENGI_TELE));
		return true;
	case BOT_UTIL_SAP_LASTENEMY_SENTRY:
		m_pSchedules->Add(new CBotSpySapBuildingSched(m_pLastEnemySentry, ENGI_SENTRY));
		return true;
	case BOT_UTIL_SAP_ENEMY_SENTRY:
		m_pSchedules->Add(new CBotSpySapBuildingSched(m_pEnemy, ENGI_SENTRY));
		return true;
	case BOT_UTIL_SAP_NEAREST_SENTRY:
		m_pSchedules->Add(new CBotSpySapBuildingSched(m_pNearestEnemySentry, ENGI_SENTRY));
		return true;
	case BOT_UTIL_SPAM_NEAREST_SENTRY:
	case BOT_UTIL_SPAM_LAST_ENEMY:
	case BOT_UTIL_SPAM_LAST_ENEMY_SENTRY:
	{
		Vector vLoc;
		Vector vEnemy;
		edict_t *pEnemy;

		vLoc = GetOrigin();

		if (id == BOT_UTIL_SPAM_NEAREST_SENTRY)
		{
			pEnemy = m_pNearestEnemySentry;
			vEnemy = CBotGlobals::EntityOrigin(m_pNearestEnemySentry);
		}
		else if (id == BOT_UTIL_SPAM_LAST_ENEMY_SENTRY)
		{
			pEnemy = m_pLastEnemySentry;
			vEnemy = CBotGlobals::EntityOrigin(m_pLastEnemySentry);
			vLoc = m_vLastDiedOrigin;
		}
		else
		{
			pEnemy = m_pLastEnemy;
			vEnemy = m_vLastSeeEnemy;
		}

		/*

		CWaypoint *pWptBlast = CWaypoints::GetWaypoint(CWaypointLocations::NearestBlastWaypoint(vEnemy, vLoc, 4096.0, -1, true, true, false, false, GetTeam(), false));

		if ( pWptBlast )
		{
		CWaypoint *pWpt = CWaypoints::GetWaypoint(CWaypointLocations::NearestBlastWaypoint(vLoc, pWptBlast->GetOrigin(), 4096.0, CWaypoints::GetWaypointIndex(pWptBlast), true, false, true, false, GetTeam(), true, 1024.0f));
		*/
		int iAiming;
		CWaypoint *pWpt = CWaypoints::NearestPipeWaypoint(vEnemy, GetOrigin(), &iAiming);

		if (pWpt)
		{
			CFindPathTask *findpath = new CFindPathTask(pEnemy);
			CBotTask *pipetask = new CBotTF2Spam(pWpt->GetOrigin(), vEnemy, util->GetWeaponChoice());
			CBotSchedule *pipesched = new CBotSchedule();

			pipesched->AddTask(new CBotTF2FindPipeWaypoint(vLoc, vEnemy));
			pipesched->AddTask(findpath);
			pipesched->AddTask(pipetask);

			m_pSchedules->Add(pipesched);

			findpath->GetPassedIntAsWaypointId();
			findpath->CompleteIfSeeTaskEdict();
			findpath->DontGoToEdict();
			findpath->SetDangerPoint(CWaypointLocations::NearestWaypoint(vEnemy, 200.0f, -1));


			return true;
		}
		//}
	}
	break;
	case BOT_UTIL_PIPE_NEAREST_SENTRY:
	case BOT_UTIL_PIPE_LAST_ENEMY:
	case BOT_UTIL_PIPE_LAST_ENEMY_SENTRY:
	{
		Vector vLoc;
		Vector vEnemy;
		edict_t *pEnemy;

		vLoc = GetOrigin();

		if (id == BOT_UTIL_PIPE_NEAREST_SENTRY)
		{
			pEnemy = m_pNearestEnemySentry;
			vEnemy = CBotGlobals::EntityOrigin(m_pNearestEnemySentry);
		}
		else if (id == BOT_UTIL_PIPE_LAST_ENEMY_SENTRY)
		{
			pEnemy = m_pLastEnemySentry;
			vEnemy = CBotGlobals::EntityOrigin(m_pLastEnemySentry);
			vLoc = m_vLastDiedOrigin;
		}
		else
		{
			pEnemy = m_pLastEnemy;
			vEnemy = m_vLastSeeEnemy;
		}

		int iAiming;
		CWaypoint *pWpt = CWaypoints::NearestPipeWaypoint(vEnemy, GetOrigin(), &iAiming);

		if (pWpt)
		{


			CFindPathTask *findpath = new CFindPathTask(pEnemy);
			CBotTask *pipetask = new CBotTF2DemomanPipeEnemy(GetWeapons()->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_PIPEBOMBS)), vEnemy, pEnemy);
			CBotSchedule *pipesched = new CBotSchedule();

			pipetask->SetInterruptFunction(new CBotTF2HurtInterrupt(this));
			pipesched->AddTask(new CBotTF2FindPipeWaypoint(vLoc, vEnemy));
			pipesched->AddTask(findpath);
			pipesched->AddTask(pipetask);

			m_pSchedules->Add(pipesched);

			findpath->GetPassedIntAsWaypointId();
			findpath->SetDangerPoint(CWaypointLocations::NearestWaypoint(vEnemy, 200.0f, -1));
			findpath->CompleteIfSeeTaskEdict();
			findpath->DontGoToEdict();

			return true;
		}
	}
	break;
	case BOT_UTIL_GETAMMOKIT:
		m_pSchedules->RemoveSchedule(SCHED_PICKUP);
		m_pSchedules->AddFront(new CBotPickupSched(m_pAmmo));

		m_fPickupTime = engine->Time() + RandomFloat(5.0f, 10.0f);
		return true;
	case BOT_UTIL_SNIPE_CROSSBOW:

		if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
		{
			pWaypoint = CTeamFortress2Mod::GetBestWaypointMVM(this, CWaypointTypes::W_FL_SNIPER);
			/*
			Vector vFlagLocation;

			if ( CTeamFortress2Mod::getFlagLocation(TF2_TEAM_BLUE,&vFlagLocation) )
			{
			pWaypoint = CWaypoints::randomWaypointGoalNearestArea(CWaypointTypes::W_FL_SNIPER,m_iTeam,0,false,this,true,&vFlagLocation);
			}*/
		}
		else if (CTeamFortress2Mod::IsAttackDefendMap())
		{
			if (GetTeam() == TF2_TEAM_RED)
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SNIPER, GetTeam(), m_iCurrentDefendArea, true, this, true, WPT_SEARCH_AVOID_SNIPERS);
			else
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SNIPER, GetTeam(), m_iCurrentAttackArea, false, this, false, WPT_SEARCH_AVOID_SNIPERS);
		}

		if (!pWaypoint)
		{
			int area = (RandomInt(0, 1) == 1) ? m_iCurrentAttackArea : m_iCurrentDefendArea;

			pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SNIPER, GetTeam(), area, true, this, area == m_iCurrentDefendArea, WPT_SEARCH_AVOID_SNIPERS);

			if (!pWaypoint)
			{
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SNIPER, GetTeam(), 0, false, this, false, WPT_SEARCH_AVOID_SNIPERS);
			}
		}

		if (pWaypoint)
		{
			m_pSchedules->Add(new CBotTF2SnipeCrossBowSched(pWaypoint->GetOrigin(), CWaypoints::GetWaypointIndex(pWaypoint)));
			return true;
		}
		break;
	case BOT_UTIL_SNIPE:

		if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
		{
			pWaypoint = CTeamFortress2Mod::GetBestWaypointMVM(this, CWaypointTypes::W_FL_SNIPER);
			/*
			Vector vFlagLocation;

			if ( CTeamFortress2Mod::getFlagLocation(TF2_TEAM_BLUE,&vFlagLocation) )
			{
			pWaypoint = CWaypoints::randomWaypointGoalNearestArea(CWaypointTypes::W_FL_SNIPER,m_iTeam,0,false,this,true,&vFlagLocation);
			}*/
		}
		else if (CTeamFortress2Mod::IsAttackDefendMap())
		{
			if (GetTeam() == TF2_TEAM_RED)
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SNIPER, GetTeam(), m_iCurrentDefendArea, true, this, true, WPT_SEARCH_AVOID_SNIPERS);
			else
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SNIPER, GetTeam(), m_iCurrentAttackArea, false, this, false, WPT_SEARCH_AVOID_SNIPERS);
		}

		if (!pWaypoint)
		{
			int area = (RandomInt(0, 1) == 1) ? m_iCurrentAttackArea : m_iCurrentDefendArea;

			pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SNIPER, GetTeam(), area, true, this, area == m_iCurrentDefendArea, WPT_SEARCH_AVOID_SNIPERS);

			if (!pWaypoint)
			{
				pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_SNIPER, GetTeam(), 0, false, this, false, WPT_SEARCH_AVOID_SNIPERS);
			}
		}

		if (pWaypoint)
		{
			m_pSchedules->Add(new CBotTF2SnipeSched(pWaypoint->GetOrigin(), CWaypoints::GetWaypointIndex(pWaypoint)));
			return true;
		}
		break;
	case BOT_UTIL_GETFLAG_LASTKNOWN:
		pWaypoint = CWaypoints::GetWaypoint(CWaypoints::NearestWaypointGoal(-1, m_vLastKnownFlagPoint, 512.0, GetTeam()));

		if (pWaypoint)
		{
			m_pSchedules->Add(new CBotTF2FindFlagSched(m_vLastKnownFlagPoint));
			return true;
		}
		break;
	case BOT_UTIL_MEDIC_FINDPLAYER_AT_SPAWN:
	{
		CWaypoint *pWaypoint = NULL;

		pWaypoint = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(m_vTeleportEntrance, 300, -1, true, false, true, NULL, false, GetTeam(), true));

		//if ( pWaypoint && randomInt(0,1) )
		//	pWaypoint = CWaypoints::getPinchPointFromWaypoint(getOrigin(),pWaypoint->getOrigin());

		if (pWaypoint)
		{
			SetLookAt(pWaypoint->GetOrigin());
			m_pSchedules->Add(new CBotDefendSched(pWaypoint->GetOrigin(), RandomFloat(10.0f, 25.0f)));
			RemoveCondition(CONDITION_PUSH);
			return true;
		}
	}
	break;
	case BOT_UTIL_MEDIC_FINDPLAYER:
	{
		m_pSchedules->Add(new CBotTF2HealSched(m_pLastCalledMedic));
		// roam
		//pWaypoint = CWaypoints::randomWaypointGoal(-1,getTeam(),0,false,this);

		//if ( pWaypoint )
		//{
		//	m_pSchedules->add(new CBotGotoOriginSched(pWaypoint->getOrigin()));
		//	return true;
		//}
	}
	break;
	case BOT_UTIL_MESSAROUND:
	{
		// find a nearby friendly
		int i = 0;
		edict_t *pEdict;
		edict_t *pNearby = NULL;
		float fMaxDistance = 500;
		float fDistance;

		for (i = 1; i < MAX_PLAYERS; i++)
		{
			pEdict = INDEXENT(i);

			if (CBotGlobals::EntityIsValid(pEdict))
			{
				if (CClassInterface::GetTeam(pEdict) == GetTeam())
				{
					if ((fDistance = DistanceFrom(pEdict)) < fMaxDistance)
					{
						if (IsVisible(pEdict))
						{
							// add a little bit of randomness
							if (!pNearby || RandomInt(0, 1))
							{
								pNearby = pEdict;
								fMaxDistance = fDistance;
							}
						}
					}
				}
			}
		}

		if (pNearby)
		{
			m_pSchedules->Add(new CBotTF2MessAroundSched(pNearby, TF_VC_INVALID));
			return true;
		}

		return false;
	}
	break;
	case BOT_UTIL_GETFLAG:
		pWaypoint = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_FLAG, GetTeam());

		if (pWaypoint)
		{
			CWaypoint *pRoute = NULL;
			Vector vRoute = Vector(0, 0, 0);
			bool bUseRoute = false;

			if ((m_fUseRouteTime < engine->Time()))
			{
				// find random route
				pRoute = CWaypoints::RandomRouteWaypoint(this, GetOrigin(), pWaypoint->GetOrigin(), GetTeam(), m_iCurrentAttackArea);

				if (pRoute)
				{
					bUseRoute = true;
					vRoute = pRoute->GetOrigin();
					m_fUseRouteTime = engine->Time() + RandomFloat(30.0f, 60.0f);
				}
			}

			m_pSchedules->Add(new CBotTF2GetFlagSched(pWaypoint->GetOrigin(), bUseRoute, vRoute));

			return true;
		}

		break;
	case BOT_UTIL_FIND_SQUAD_LEADER:
	{
		Vector pos = m_pSquad->GetFormationVector(m_pEdict);
		CBotTask *findTask = new CFindPathTask(pos);
		RemoveCondition(CONDITION_SEE_SQUAD_LEADER);
		RemoveCondition(CONDITION_SQUAD_LEADER_INRANGE);
		findTask->SetCompleteInterrupt(CONDITION_SEE_SQUAD_LEADER | CONDITION_SQUAD_LEADER_INRANGE);

		m_pSchedules->Add(new CBotSchedule(findTask));

		return true;
	}
	break;
	case BOT_UTIL_FOLLOW_SQUAD_LEADER:
	{
		Vector pos = m_pSquad->GetFormationVector(m_pEdict);

		m_pSchedules->Add(new CBotSchedule(new CBotFollowSquadLeader(m_pSquad)));

		return true;
	}
	break;
	case BOT_UTIL_ROAM:
		// roam
		pWaypoint = CWaypoints::RandomWaypointGoal(-1, GetTeam(), 0, false, this);

		if (pWaypoint)
		{
			m_pSchedules->Add(new CBotGotoOriginSched(pWaypoint->GetOrigin()));
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}

void CBotTF2::TouchedWpt(CWaypoint *pWaypoint, int iNextWaypoint, int iPrevWaypoint)
{
	static int wptindex;

	CBot::TouchedWpt(pWaypoint);

	if (CanGotoWaypoint(GetOrigin(), pWaypoint))
	{
		if (pWaypoint->HasFlag(CWaypointTypes::W_FL_ROCKET_JUMP))
		{
			if (GetNavigator()->HasNextPoint())
			{
				CBotWeapon *pWeapon = NULL;

				if (GetClass() == TF_CLASS_SOLDIER)
				{
					pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_ROCKETLAUNCHER));

					if (pWeapon && pWeapon->HasWeapon())
						m_pSchedules->AddFront(new CBotSchedule(new CBotTFRocketJump()));
				}
				else if ((GetClass() == TF_CLASS_DEMOMAN) && bot_demo_jump.GetBool())
				{
					pWeapon = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_PIPEBOMBS));

					if (pWeapon && pWeapon->HasWeapon())
						m_pSchedules->AddFront(new CBotSchedule(new CBotTF2DemomanPipeJump(this, pWaypoint->GetOrigin(), GetNavigator()->GetNextPoint(), GetWeapons()->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_PIPEBOMBS)))));
				}
			}
		}
		else if (pWaypoint->HasFlag(CWaypointTypes::W_FL_DOUBLEJUMP))
		{
			extern ConVar bot_scoutdj;
			m_pButtons->Tap(IN_JUMP);
			m_fDoubleJumpTime = engine->Time() + bot_scoutdj.GetFloat();
		}
		else if (pWaypoint->GetFlags() == 0)
		{
			if (GetNavigator()->HasNextPoint() && (GetClass() == TF_CLASS_SCOUT))
			{
				if (RandomFloat(0.0f, 100.0f) > (m_pProfile->m_fBraveness * 10))
				{
					float fVel = m_vVelocity.Length();

					if (fVel > 0.0f)
					{
						Vector v_next = GetNavigator()->GetNextPoint();
						Vector v_org = GetOrigin();
						Vector v_comp = v_next - v_org;
						float fDist = v_comp.Length();

						Vector v_vel = (m_vVelocity / fVel) * fDist;

						if ((v_next - (v_org + v_vel)).Length() <= 24.0f)
							m_pButtons->Tap(IN_JUMP);
					}
				}
			}
		}
		else if (pWaypoint->HasFlag(CWaypointTypes::W_FL_FALL))
		{
			// jump to avoid being hurt (scouts can jump in the air)
			if (fabs(m_vVelocity.z) > 1)
				Jump();
		}
	}

	// only good for spies so they know when to cloak better
	if (GetClass() == TF_CLASS_SPY)
	{
		wptindex = CWaypoints::GetWaypointIndex(pWaypoint);

		if (m_pEnemy && HasSomeConditions(CONDITION_SEE_CUR_ENEMY))
		{
			m_pNavigator->BeliefOne(wptindex, BELIEF_DANGER, DistanceFrom(m_pEnemy));
		}
		else
			m_pNavigator->BeliefOne(wptindex, BELIEF_SAFETY, 0);
	}
}

void CBotTF2::ModAim(edict_t *pEntity, Vector &v_origin, Vector *v_desired_offset, Vector &v_size, float fDist, float fDist2D)
{
	static CBotWeapon *pWp;
	static float fTime;

	extern ConVar bot_supermode;

	pWp = GetCurrentWeapon();

	CBot::ModAim(pEntity, v_origin, v_desired_offset, v_size, fDist, fDist2D);

	if (pWp)
	{
		if (m_iClass == TF_CLASS_SNIPER)
		{
			if (pWp->GetID() == TF2_WEAPON_BOW)
			{
				if (pWp->GetProjectileSpeed() > 0)
				{
					extern ConVar *sv_gravity;
					Vector vVelocity;
					CClassInterface::GetVelocity(pEntity, &vVelocity);

					// the arrow will arc
					fTime = fDist2D / (pWp->GetProjectileSpeed()*0.707);

					if (bot_supermode.GetBool())
						*v_desired_offset = *v_desired_offset + ((vVelocity*fTime));
					else
						*v_desired_offset = *v_desired_offset + ((vVelocity*fTime)*m_pProfile->m_fAimSkill);

					if (sv_gravity != NULL)
						v_desired_offset->z += ((pow(2, fTime) - 1.0f)*(sv_gravity->GetFloat()*0.1f));// - (GetOrigin().z - v_origin.z);

					v_desired_offset->z *= 0.6f;
				}
			}
			else if ((v_desired_offset->z < 64.0f) && !HasSomeConditions(CONDITION_SEE_ENEMY_GROUND))
			{
				if (bot_supermode.GetBool())
					v_desired_offset->z += 15.0f;
				else
					v_desired_offset->z += RandomFloat(0.0f, 16.0f);
			}
		}
		else if (m_iClass == TF_CLASS_MEDIC)
		{
			if (pWp->GetID() == TF2_WEAPON_SYRINGEGUN)
				v_desired_offset->z += sqrt(fDist) * 2;
		}
		else if (m_iClass == TF_CLASS_HWGUY)
		{
			if (pWp->GetID() == TF2_WEAPON_MINIGUN)
			{
				extern ConVar bot_heavyaimoffset;

				Vector vForward;
				Vector vRight;
				Vector vUp;

				QAngle eyes = CBotGlobals::PlayerAngles(pEntity);

				// in fov? Check angle to edict
				AngleVectors(eyes, &vForward, &vRight, &vUp);

				*v_desired_offset = *v_desired_offset + (((vRight * 24) - Vector(0, 0, 24))* bot_heavyaimoffset.GetFloat());
			}
		}
		else if ((m_iClass == TF_CLASS_SOLDIER) || (m_iClass == TF_CLASS_DEMOMAN))
		{
			//			int iSpeed = 0;

			switch (pWp->GetID())
			{
			case TF2_WEAPON_ROCKETLAUNCHER:
				// fall through
			case TF2_WEAPON_COWMANGLER:
			case TF2_WEAPON_FLAREGUN:
			case TF2_WEAPON_GRENADELAUNCHER:
			{
				extern ConVar *sv_gravity;
				Vector vVelocity;
				CClassInterface::GetVelocity(pEntity, &vVelocity);

				if (pWp->GetProjectileSpeed() > 0)
				{
					if ((pWp->GetID() == TF2_WEAPON_GRENADELAUNCHER))
						fTime = fDist2D / (pWp->GetProjectileSpeed()*0.707);
					else
						fTime = fDist / (pWp->GetProjectileSpeed());

					if (bot_supermode.GetBool())
						*v_desired_offset = *v_desired_offset + ((vVelocity*fTime));
					else
						*v_desired_offset = *v_desired_offset + ((vVelocity*fTime)*m_pProfile->m_fAimSkill);

					if ((sv_gravity != NULL) && (pWp->GetID() == TF2_WEAPON_GRENADELAUNCHER))
						v_desired_offset->z += ((pow(2, fTime) - 1.0f)*(sv_gravity->GetFloat()*0.1f));// - (GetOrigin().z - v_origin.z);

					if ((pWp->GetID() == TF2_WEAPON_GRENADELAUNCHER) && HasSomeConditions(CONDITION_SEE_ENEMY_GROUND))
						v_desired_offset->z -= RandomFloat(8.0f, 32.0f); // aim for ground - with grenade launcher
					else if ((pWp->GetID() == TF2_WEAPON_ROCKETLAUNCHER) || (pWp->GetID() == TF2_WEAPON_COWMANGLER) || (v_origin.z > (GetOrigin().z + 16.0f)))
						v_desired_offset->z += RandomFloat(8.0f, 32.0f); // aim for body, not ground
				}
			}
			break;
			}
		}
	}
}
/*
Vector CBotTF2::GetAimVector (edict_t *pEntity)
{
extern ConVar bot_aimsmoothing;

static CBotWeapon *pWp;
static float fDist;
static float fTime;

pWp = getCurrentWeapon();
fDist = distanceFrom(pEntity);

if ( m_fNextUpdateAimVector > engine->Time() )
{
//if ( m_bPrevAimVectorValid && bot_aimsmoothing.GetBool() )
//	return BOTUTIL_SmoothAim(m_vPrevAimVector,m_vAimVector,m_fStartUpdateAimVector,engine->Time(),m_fNextUpdateAimVector);

return m_vAimVector;
}

Vector vAim = CBot::getAimVector(pEntity);

if ( pWp )
{

if ( m_iClass == TF_CLASS_MEDIC )
{
if ( pWp->getID() == TF2_WEAPON_SYRINGEGUN )
vAim = vAim + Vector(0,0,sqrt(fDist)*2);
}
else if ( m_iClass == TF_CLASS_HWGUY )
{
if ( pWp->getID() == TF2_WEAPON_MINIGUN )
{
extern ConVar bot_heavyaimoffset;

Vector vForward;
Vector vRight;
QAngle eyes;

eyes = eyeAngles();

// in fov? Check angle to edict
AngleVectors(eyes,&vForward);
vForward = vForward.NormalizeInPlace();

vRight = vForward.Cross(Vector(0,0,1));

vAim = vAim + (((vRight * 24) - Vector(0,0,24))* bot_heavyaimoffset.GetFloat());
}
}
else if ( (m_iClass == TF_CLASS_SOLDIER) || (m_iClass == TF_CLASS_DEMOMAN) )
{
int iSpeed = 0;

switch ( pWp->getID() )
{
case TF2_WEAPON_ROCKETLAUNCHER:
{
iSpeed = TF2_ROCKETSPEED;

if ( vAim.z <= getOrigin().z )
vAim = vAim - Vector(0,0,randomFloat(8.0f,24.0f));
}
// fall through
case TF2_WEAPON_GRENADELAUNCHER:
{
CClient *pClient = CClients::get(pEntity);
Vector vVelocity;

if ( iSpeed == 0 )
iSpeed = TF2_GRENADESPEED;

if ( pClient )
{
if ( CClassInterface :: getVelocity(pEntity,&vVelocity) )
{
if ( pClient && (vVelocity == Vector(0,0,0)) )
vVelocity = pClient->getVelocity();
}
else if ( pClient )
vVelocity = pClient->getVelocity();

// speed = distance/time
// .'.
// time = distance/speed
fTime = fDist/iSpeed;

vAim = vAim + ((vVelocity*fTime)*m_pProfile->m_fAimSkill );
}

if ( pWp->getID() == TF2_WEAPON_GRENADELAUNCHER )
vAim = vAim + Vector(0,0,sqrt(fDist));
}
break;
}
}
}

m_fStartUpdateAimVector = engine->Time();
m_vAimVector = vAim;

return m_vAimVector;
}
*/
void CBotTF2::CheckDependantEntities()
{
	CBotFortress::CheckDependantEntities();
}

eBotFuncState CBotTF2::RocketJump(int *iState, float *fTime)
{
	extern ConVar bot_rj;

	SetLookAtTask(LOOK_GROUND);
	m_bIncreaseSensitivity = true;

	switch (*iState)
	{
	case 0:
	{
		QAngle pAngle = CBotGlobals::PlayerAngles(m_pEdict);
		if ((GetSpeed() > 100) && (pAngle.x > 86.0f))
		{
			m_pButtons->Tap(IN_JUMP);
			*iState = *iState + 1;
			*fTime = engine->Time() + bot_rj.GetFloat();//randomFloat(0.08,0.5);

			return BOT_FUNC_CONTINUE;
		}
	}
	break;
	case 1:
	{
		if (*fTime < engine->Time())
		{
			m_pButtons->Tap(IN_ATTACK);

			return BOT_FUNC_COMPLETE;
		}
	}
	break;
	}

	return BOT_FUNC_CONTINUE;
}


// return true if the enemy is ok to shoot, return false if there is a problem (e.g. weapon problem)
bool CBotTF2::HandleAttack(CBotWeapon *pWeapon, edict_t *pEnemy)
{
	extern ConVar bot_enemyshootfov;
	static float fDistance;

	fDistance = DistanceFrom(pEnemy);

	if ((fDistance > 128) && (DotProductFromOrigin(m_vAimVector) < bot_enemyshootfov.GetFloat()))
		return true; // keep enemy / don't shoot : until angle between enemy is less than 45 degrees

	/* Handle Spy Attacking Choice here */
	if (m_iClass == TF_CLASS_SPY)
	{
		if (IsDisguised())
		{
			if (((fDistance < bot_tf2_spy_kill_on_cap_dist.GetFloat()) && CTeamFortress2Mod::IsCapping(pEnemy)) ||
				((fDistance < 130) && CBotGlobals::IsAlivePlayer(pEnemy) &&
				(fabs(CBotGlobals::YawAngleFromEdict(pEnemy, GetOrigin())) > bot_spyknifefov.GetFloat())))
			{
				; // ok attack
			}
			else if (m_fFrenzyTime < engine->Time())
				return true; // return but don't attack
			else if (CBotGlobals::IsPlayer(pEnemy) && (CClassInterface::GetTF2Class(pEnemy) == TF_CLASS_ENGINEER) &&
				(
				CTeamFortress2Mod::IsMySentrySapped(pEnemy) ||
				CTeamFortress2Mod::IsMyTeleporterSapped(pEnemy) ||
				CTeamFortress2Mod::IsMyDispenserSapped(pEnemy)
				)
				)
			{
				return true;  // return but don't attack
			}
		}
	}

	if (pWeapon)
	{
		Vector vEnemyOrigin;
		bool bSecAttack = false;
		extern ConVar bot_tf2_pyro_airblast;
		bool bIsPlayer = false;

		ClearFailedWeaponSelect();

		if (pWeapon->IsMelee())
		{
			SetMoveTo(CBotGlobals::EntityOrigin(pEnemy));
			//setLookAt(m_vAimVector);
			SetLookAtTask(LOOK_ENEMY);
			// dontAvoid my enemy
			m_fAvoidTime = engine->Time() + 1.0f;
		}

		// Airblast an enemy rocket , ubered player, or capping or defending player if they are on fire
		// First put them on fire then airblast them!
		if ((pEnemy == m_NearestEnemyRocket.Get()) || (pEnemy == m_pNearestPipeGren.Get()) ||
			(((bIsPlayer = CBotGlobals::IsPlayer(pEnemy)) == true) &&
			(CTeamFortress2Mod::TF2_IsPlayerInvuln(pEnemy) ||
			(CTeamFortress2Mod::TF2_IsPlayerOnFire(pEnemy) && ((CTeamFortress2Mod::IsFlagCarrier(pEnemy)) || (m_iCurrentDefendArea && CTeamFortress2Mod::IsCapping(pEnemy)) ||
			(m_iCurrentAttackArea && CTeamFortress2Mod::IsDefending(pEnemy)))))))
		{
			if ((bIsPlayer || (fDistance > 80)) && (fDistance < 400) && pWeapon->CanDeflectRockets() && (pWeapon->GetAmmo(this) >= bot_tf2_pyro_airblast.GetInt()))
				bSecAttack = true;
			else if ((pEnemy == m_NearestEnemyRocket.Get()) || (pEnemy == m_pNearestPipeGren.Get()))
				return false; // don't attack the rocket anymore
		}

		if ((m_iClass == TF_CLASS_SNIPER) && (pWeapon->GetID() == TF2_WEAPON_BOW))
		{
			StopMoving();

			if (m_fSnipeAttackTime > engine->Time())
			{
				PrimaryAttack(true);
			}
			else
			{
				float fDistFactor = DistanceFrom(pEnemy) / pWeapon->GetPrimaryMaxRange();
				float fRandom = RandomFloat(m_pProfile->m_fAimSkill, 1.0f);
				float fSkill = 1.0f - fRandom;

				m_fSnipeAttackTime = engine->Time() + (fSkill*1.0f) + ((4.0f*fDistFactor)*fSkill);
				m_pButtons->LetGo(IN_ATTACK);
			}
		}
		else if ((m_iClass == TF_CLASS_SNIPER) && (pWeapon->GetID() == TF2_WEAPON_SNIPERRIFLE))
		{
			StopMoving();

			if (m_fSnipeAttackTime < engine->Time())
			{
				if (CTeamFortress2Mod::TF2_IsPlayerZoomed(m_pEdict))
					PrimaryAttack(); // shoot
				else
					SecondaryAttack(); // zoom

				m_fSnipeAttackTime = engine->Time() + RandomFloat(0.5f, 3.0f);
			}
		}
		else if (!bSecAttack)
		{
			if (pWeapon->MustHoldAttack())
				PrimaryAttack(true);
			else
				PrimaryAttack();
		}
		else
		{
			TapButton(IN_ATTACK2);
		}

		vEnemyOrigin = CBotGlobals::EntityOrigin(pEnemy);
		// enemy below me!
		if (pWeapon->IsMelee() && (DistanceFrom2D(pEnemy) < 64.0f) && (vEnemyOrigin.z < GetOrigin().z) && (vEnemyOrigin.z >(GetOrigin().z - 128)))
		{
			Duck();
		}

		if ((!pWeapon->IsMelee() || pWeapon->IsSpecial()) && pWeapon->OutOfAmmo(this))
			return false; // change weapon/enemy
	}
	else
		PrimaryAttack();

	m_pAttackingEnemy = pEnemy;

	return true;
}

int CBotFortress::GetMetal()
{
	if (m_iClass == TF_CLASS_ENGINEER)
	{
		CBotWeapon *pWrench = m_pWeapons->GetWeapon(CWeapons::GetWeapon(TF2_WEAPON_WRENCH));

		if (pWrench)
		{
			return pWrench->GetAmmo(this);
		}
	}

	return 0;
}

bool CBotTF2::UpgradeBuilding(edict_t *pBuilding, bool removesapper)
{
	Vector vOrigin = CBotGlobals::EntityOrigin(pBuilding);

	CBotWeapon *pWeapon = GetCurrentWeapon();
	int iMetal = 0;

	WantToListen(false);

	if (!pWeapon)
		return false;

	iMetal = pWeapon->GetAmmo(this);

	if (pWeapon->GetID() != TF2_WEAPON_WRENCH)
	{
		if (!Select_CWeapon(CWeapons::GetWeapon(TF2_WEAPON_WRENCH)))
			return false;
	}
	else if (!removesapper && (iMetal == 0)) // finished / out of metal // dont need metal to remove sapper
		return true;
	else
	{
		ClearFailedWeaponSelect();

		if (DistanceFrom(vOrigin) > 85)
		{
			SetMoveTo(vOrigin);
		}
		else
		{
			Duck(true);
			PrimaryAttack();
		}
	}

	LookAtEdict(pBuilding);
	m_fLookSetTime = 0;
	SetLookAtTask(LOOK_EDICT);
	m_fLookSetTime = engine->Time() + RandomFloat(3.0, 8.0);

	return true;
}

void CBotFortress::TeamFlagPickup()
{
	if (CTeamFortress2Mod::IsMapType(TF_MAP_SD) && m_pSchedules->HasSchedule(SCHED_TF2_GET_FLAG))
		m_pSchedules->RemoveSchedule(SCHED_TF2_GET_FLAG);
}

void CBotTF2::RoundWon(int iTeam, bool bFullRound)
{
	m_pSchedules->FreeMemory();

	if (bFullRound)
	{
		m_fChangeClassTime = engine->Time();
	}

	UpdateCondition(CONDITION_PUSH);
	RemoveCondition(CONDITION_PARANOID);
	RemoveCondition(CONDITION_BUILDING_SAPPED);
	RemoveCondition(CONDITION_COVERT);
}

void CBotTF2::WaitRemoveSap()
{
	// this gives engi bot some time to attack spy that has been sapping a sentry
	m_fRemoveSapTime = engine->Time() + RandomFloat(2.5f, 4.0f);
	// TO DO :: add spy check task 
}

void CBotTF2::RoundReset(bool bFullReset)
{
	m_pRedPayloadBomb = NULL;
	m_pBluePayloadBomb = NULL;
	m_fLastKnownTeamFlagTime = 0.0f;
	m_bEntranceVectorValid = false;
	m_bSentryGunVectorValid = false;
	m_bDispenserVectorValid = false;
	m_bTeleportExitVectorValid = false;
	m_pPrevSpy = NULL;
	m_pHeal = NULL;
	m_pSentryGun = NULL;
	m_pDispenser = NULL;
	m_pTeleEntrance = NULL;
	m_pTeleExit = NULL;
	m_pAmmo = NULL;
	m_pHealthkit = NULL;
	m_pNearestDisp = NULL;
	m_pNearestEnemySentry = NULL;
	m_pNearestAllySentry = NULL;
	m_pNearestEnemyTeleporter = NULL;
	m_pNearestEnemyDisp = NULL;
	m_pNearestPipeGren = NULL;
	m_pFlag = NULL;
	m_pPrevSpy = NULL;
	m_iSentryKills = 0;
	m_fSentryPlaceTime = 0.0;
	m_fDispenserPlaceTime = 0.0f;
	m_fDispenserHealAmount = 0.0f;
	m_fTeleporterEntPlacedTime = 0.0f;
	m_fTeleporterExtPlacedTime = 0.0f;
	m_iTeleportedPlayers = 0;

	FlagReset();
	TeamFlagReset();

	m_pNavigator->Clear();

	m_iTeam = GetTeam();
	// fix : reset current areas
	UpdateAttackDefendPoints();
	//CPoints::GetAreas(GetTeam(), &m_iCurrentDefendArea, &m_iCurrentAttackArea);

	//m_pPayloadBomb = NULL;
}

void CBotTF2::UpdateAttackDefendPoints()
{
	m_iTeam = GetTeam();
	m_iCurrentAttackArea = CTeamFortress2Mod::m_ObjectiveResource.GetRandomValidPointForTeam(m_iTeam, TF2_POINT_ATTACK);
	m_iCurrentDefendArea = CTeamFortress2Mod::m_ObjectiveResource.GetRandomValidPointForTeam(m_iTeam, TF2_POINT_DEFEND);
}

void CBotTF2::PointsUpdated()
{
	if (m_iClass == TF_CLASS_ENGINEER)
	{
		extern ConVar bot_move_sentry_time;
		extern ConVar bot_move_disp_time;
		extern ConVar bot_move_tele_time;

		//m_pPayloadBomb = NULL;
		bool bMoveSentry = (m_iSentryArea != m_iCurrentAttackArea) && (m_iSentryArea != m_iCurrentDefendArea) && ((m_iTeam == TF2_TEAM_BLUE) || !CTeamFortress2Mod::IsAttackDefendMap());
		bool bMoveTeleEntrance = (m_iTeam == TF2_TEAM_BLUE) || !CTeamFortress2Mod::IsAttackDefendMap();
		bool bMoveTeleExit = (m_iTeleExitArea != m_iCurrentAttackArea) && (m_iTeleExitArea != m_iCurrentDefendArea) && ((m_iTeam == TF2_TEAM_BLUE) || !CTeamFortress2Mod::IsAttackDefendMap());
		bool bMoveDisp = bMoveSentry;

		// think about moving stuff now
		if (bMoveSentry && m_pSentryGun.Get() && ((m_fSentryPlaceTime + bot_move_sentry_time.GetFloat()) > engine->Time()))
			m_fSentryPlaceTime = engine->Time() - bot_move_sentry_time.GetFloat();
		if (bMoveDisp && m_pDispenser.Get() && ((m_fDispenserPlaceTime + bot_move_disp_time.GetFloat()) > engine->Time()))
			m_fDispenserPlaceTime = engine->Time() - bot_move_disp_time.GetFloat();
		if (bMoveTeleEntrance && m_pTeleEntrance.Get() && ((m_fTeleporterEntPlacedTime + bot_move_tele_time.GetFloat()) > engine->Time()))
			m_fTeleporterEntPlacedTime = engine->Time() - bot_move_tele_time.GetFloat();
		if (bMoveTeleExit && m_pTeleExit.Get() && ((m_fTeleporterExtPlacedTime + bot_move_tele_time.GetFloat()) > engine->Time()))
			m_fTeleporterExtPlacedTime = engine->Time() - bot_move_tele_time.GetFloat();

		// rethink everything
		UpdateCondition(CONDITION_CHANGED);
	}
}

void CBotTF2::UpdateAttackPoints()
{
	int iPrev = m_iCurrentAttackArea;

	m_iTeam = GetTeam();

	m_iCurrentAttackArea = CTeamFortress2Mod::m_ObjectiveResource.GetRandomValidPointForTeam(m_iTeam, TF2_POINT_ATTACK);

	if (iPrev != m_iCurrentAttackArea)
	{
		UpdateCondition(CONDITION_CHANGED);
	}
}

void CBotTF2::UpdateDefendPoints()
{
	int iPrev = m_iCurrentDefendArea;

	m_iTeam = GetTeam();

	m_iCurrentDefendArea = CTeamFortress2Mod::m_ObjectiveResource.GetRandomValidPointForTeam(m_iTeam, TF2_POINT_DEFEND);

	if (iPrev != m_iCurrentDefendArea)
	{
		UpdateCondition(CONDITION_CHANGED);
	}
}

/// TO DO : list of areas
void CBotTF2::GetDefendArea(vector<int> *m_iAreas)
{
	m_iCurrentDefendArea = CTeamFortress2Mod::m_ObjectiveResource.GetRandomValidPointForTeam(m_iTeam, TF2_POINT_DEFEND);
}

void CBotTF2::GetAttackArea(vector <int> *m_iAreas)
{
	m_iCurrentAttackArea = CTeamFortress2Mod::m_ObjectiveResource.GetRandomValidPointForTeam(m_iTeam, TF2_POINT_ATTACK);
}

void CBotTF2::PointCaptured(int iPoint, int iTeam, const char *szPointName)
{
	m_pRedPayloadBomb = NULL;
	m_pBluePayloadBomb = NULL;
}

// Is Enemy Function
// take a pEdict entity to check if its an enemy
// return TRUE to "OPEN FIRE" (Attack)
// return FALSE to ignore
#define RCBOT_ISENEMY_UNDEF -1
#define RCBOT_ISENEMY_TRUE 1
#define RCBOT_ISENEMY_FALSE 0

bool CBotTF2::IsEnemy(edict_t *pEdict, bool bCheckWeapons)
{
	extern ConVar bot_notarget;

	static short int iEnemyTeam;
	static bool bIsPipeBomb;
	static bool bIsRocket;
	static int bValid;
	static bool bIsBoss;
	static bool bIsGrenade;

	bIsPipeBomb = false;
	bIsRocket = false;
	bValid = false;
	bIsBoss = false;
	bIsGrenade = false;

	if (!pEdict || !pEdict->GetUnknown())
		return false;

	if (!CBotGlobals::EntityIsAlive(pEdict))
		return false;

	if (bot_notarget.GetBool() && (ENTINDEX(pEdict) == 1))
		return false;

	if (CBotGlobals::IsPlayer(pEdict))
	{
		if (CBotGlobals::GetTeam(pEdict) != GetTeam())
		{
			if (m_iClass == TF_CLASS_SPY)
			{
				edict_t *pSentry = NULL;

				if (!bCheckWeapons)
					return true;

				if (IsDisguised())
				{
					if ((pSentry = m_pNearestEnemySentry.Get()) != NULL)
					{
						// If I'm disguised don't attack any player until nearby sentry is disabled
						if (!CTeamFortress2Mod::IsSentrySapped(pSentry) && IsVisible(pSentry) && (DistanceFrom(pSentry) < TF2_MAX_SENTRYGUN_RANGE))
							return false;
					}

					if ((pSentry = m_pLastEnemySentry.Get()) != NULL)
					{
						// If I'm disguised don't attack any player until nearby sentry is disabled
						if (!CTeamFortress2Mod::IsSentrySapped(pSentry) && IsVisible(pSentry) && (DistanceFrom(pSentry) < TF2_MAX_SENTRYGUN_RANGE))
							return false;
					}
				}
			}

			if (CClassInterface::GetTF2Class(pEdict) == (int)TF_CLASS_SPY)
			{
				static float fMinReaction;
				static float fMaxReaction;
				static int dteam, dclass, dhealth, dindex;
				static bool bfoundspy;
				static float fSpyAttackTime;
				static edict_t *pDisguisedAs;

				bfoundspy = true; // shout found spy if true

				fSpyAttackTime = engine->Time() - m_fSpyList[ENTINDEX(pEdict) - 1];

				if (CClassInterface::GetTF2SpyDisguised(pEdict, &dclass, &dteam, &dindex, &dhealth))
				{
					pDisguisedAs = (dindex > 0) ? (INDEXENT(dindex)) : (NULL);

					if (CTeamFortress2Mod::TF2_IsPlayerCloaked(pEdict)) // if he is cloaked -- can't see him
					{
						bValid = false;
						// out of cloak charge or on fire -- i will see him move 
						if ((CClassInterface::GetTF2SpyCloakMeter(pEdict) == 0.0f) || CTeamFortress2Mod::TF2_IsPlayerOnFire(pEdict)) // if he is on fire and cloaked I can see him
						{
							// if I saw my team mate shoot him within the last 5 seconds, he's a spy!
							// or no spies on team that can do this!
							bValid = (fSpyAttackTime < 5.0f) || !IsClassOnTeam(TF_CLASS_SPY, GetTeam());
						}
					}
					else if (dteam == 0) // not disguised
					{
						bValid = true;
					}
					else if (dteam != GetTeam())
					{
						bValid = true;
						bfoundspy = false; // disguised as enemy!
					}
					else if (dindex == ENTINDEX(m_pEdict)) // if he is disguised as me -- he must be a spy!
					{
						bValid = true;
					}
					else if (!IsClassOnTeam(dclass, GetTeam()))
					{// be smart - check if player disguised as a class that exists on my team
						bValid = true;
					}
					else if (dhealth <= 0)
					{
						// be smart - check if player's health is below 0
						bValid = true;
					}
					// if he is on fire and I saw my team mate shoot him within the last 5 seconds, he's a spy!
					else if (CTeamFortress2Mod::TF2_IsPlayerOnFire(pEdict) && (fSpyAttackTime < 5.0f))
					{
						bValid = true;
					}
					// a. I can see the player he is disguised as 
					// b. and the spy was shot recently by a teammate
					// = possibly by the player he was disguised as!
					else if (pDisguisedAs && IsVisible(pDisguisedAs) && (fSpyAttackTime < 5.0f))
					{
						bValid = true;
					}
					else
						bValid = ThinkSpyIsEnemy(pEdict, (TFClass)dclass);

					if (bValid && bCheckWeapons && bfoundspy)
						FoundSpy(pEdict, (TFClass)dclass);
				}
			}
			else if ((m_iClass == TF_CLASS_ENGINEER) && (m_pSentryGun.Get() != NULL))
			{
				edict_t *pSentryEnemy = CClassInterface::GetSentryEnemy(m_pSentryGun);

				if (pSentryEnemy == NULL)
					bValid = true; // attack
				else if (pSentryEnemy == pEdict)
				{
					// let my sentry gun do the work
					bValid = false; // don't attack
				}
				else if ((CBotGlobals::EntityOrigin(pSentryEnemy) - CBotGlobals::EntityOrigin(pEdict)).Length() < 200)
					bValid = false; // sentry gun already shooting near this guy -- don't attack - let sentry gun do it
				else
					bValid = true;
			}
			else
				bValid = true;
		}
	}
	else if (CTeamFortress2Mod::IsMapType(TF_MAP_RD) && !strcmp(pEdict->GetClassName(), "tf_robot_destruction_robot") && (CClassInterface::GetTeam(pEdict) != m_iTeam))
	{
		bValid = true;
	}
	else if (CTeamFortress2Mod::IsBoss(pEdict))
	{
		bIsBoss = bValid = true;
	}
	// "FrenzyTime" is the time it takes for the bot to check out where he got hurt
	else if ((m_iClass != TF_CLASS_SPY) || (m_fFrenzyTime > engine->Time()))
	{
		iEnemyTeam = CTeamFortress2Mod::GetEnemyTeam(GetTeam());

		// don't attack sentries if spy, just sap them
		if (((m_iClass != TF_CLASS_SPY) && CTeamFortress2Mod::IsSentry(pEdict, iEnemyTeam)) ||
			CTeamFortress2Mod::IsDispenser(pEdict, iEnemyTeam) ||
			CTeamFortress2Mod::IsTeleporter(pEdict, iEnemyTeam)
			/*CTeamFortress2Mod::isTeleporterExit(pEdict,iEnemyTeam)*/)
		{
			bValid = true;
		}
		else if (CTeamFortress2Mod::IsPipeBomb(pEdict, iEnemyTeam))
			bIsPipeBomb = bValid = true;
		else if (CTeamFortress2Mod::IsRocket(pEdict, iEnemyTeam))
			bIsRocket = bValid = true;
		else if (CTeamFortress2Mod::IsHurtfulPipeGrenade(pEdict, m_pEdict, false))
			bIsGrenade = bValid = true;
	}

	if (bValid)
	{
		if (bCheckWeapons)
		{
			CBotWeapon *pWeapon = m_pWeapons->GetBestWeapon(pEdict);

			if (pWeapon == NULL)
			{
				return false;
			}
			else
			{
				if (bIsPipeBomb && !pWeapon->CanDestroyPipeBombs())
					return false;
				else if (bIsRocket && !pWeapon->CanDeflectRockets())
					return false;
				else if (bIsGrenade && !pWeapon->CanDeflectRockets())
					return false;
				else if (bIsBoss && pWeapon->IsMelee())
					return false;
			}
		}

		return true;
	}

	return false;
}


////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// FORTRESS FOREVER


void CBotFF::ModThink()
{
	// mod specific think code here
	CBotFortress::ModThink();
}

bool CBotFF::IsEnemy(edict_t *pEdict, bool bCheckWeapons)
{
	if (pEdict == m_pEdict)
		return false;

	if (!ENTINDEX(pEdict) || (ENTINDEX(pEdict) > MAX_PLAYERS))
		return false;

	if (CBotGlobals::GetTeam(pEdict) == GetTeam())
		return false;

	return true;
}

void CBotTF2::MannVsMachineWaveComplete()
{
	if ((m_iClass != TF_CLASS_ENGINEER) || !IsCarrying())
		m_pSchedules->FreeMemory();

	SetLastEnemy(NULL);

	Reload();

	m_fSentryPlaceTime = 1.0f;
	m_fLastSentryEnemyTime = 0.0f;

	m_fDispenserPlaceTime = 1.0f;
}

void CBotTF2::MannVsMachineAlarmTriggered(Vector vLoc)
{

	if (m_iClass == TF_CLASS_ENGINEER)
	{
		edict_t *pSentry;

		if ((pSentry = m_pSentryGun.Get()) != NULL)
		{
			if (CTeamFortress2Mod::GetSentryLevel(pSentry) < 3)
				return;

			// don't defend work on sentry!!!
		}

		if (IsCarrying())
			return;
	}

	float fDefTime = RandomFloat(10.0f, 20.0f);

	m_fDefendTime = engine->Time() + fDefTime + 1.0f;

	CBotSchedule *newSched = new CBotDefendSched(vLoc, m_fDefendTime / 2);

	m_pSchedules->FreeMemory();
	m_pSchedules->Add(newSched);
	newSched->SetID(SCHED_RETURN_TO_INTEL);
}

// Go back to Cap/Flag to 
void CBotTF2::EnemyAtIntel(Vector vPos, int type, int iArea)
{

	if (m_pSchedules->GetCurrentSchedule())
	{
		if (m_pSchedules->GetCurrentSchedule()->IsID(SCHED_RETURN_TO_INTEL))
		{
			// already going back to intel
			return;
		}
	}

	if (CBotGlobals::EntityIsValid(m_pDefendPayloadBomb) && (CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE)))
	{
		vPos = CBotGlobals::EntityOrigin(m_pDefendPayloadBomb);
	}

	if ((m_iClass == TF_CLASS_DEMOMAN) && (m_iTrapType != TF_TRAP_TYPE_NONE) && (m_iTrapType != TF_TRAP_TYPE_WPT))
	{
		if ((m_iTrapType != TF_TRAP_TYPE_POINT) || (iArea == m_iTrapCPIndex))
		{
			// Stickies at PL Capture or bomb point
			if (((m_iTrapType == TF_TRAP_TYPE_POINT) || (m_iTrapType == TF_TRAP_TYPE_PL)) && (CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE)))
			{
				edict_t *pBomb;

				// get enemy pl bomb
				if ((pBomb = m_pDefendPayloadBomb.Get()) != NULL)
				{
					if ((m_vStickyLocation - CBotGlobals::EntityOrigin(pBomb)).Length() < (BLAST_RADIUS * 2))
						DetonateStickies();
				}
			}
			else
				DetonateStickies();
		}
	}

	m_fRevMiniGunTime = engine->Time() - m_fNextRevMiniGunTime;

	if (!m_pPI)
		return;

	if (!IsAlive())
		return;

	if (HasFlag())
		return;

	if (m_fDefendTime > engine->Time())
		return;

	if (m_iClass == TF_CLASS_ENGINEER)
		return; // got work to do...

	// bot is already capturing a point
	if (m_pSchedules && m_pSchedules->IsCurrentSchedule(SCHED_ATTACKPOINT))
	{
		// already attacking a point 
		int capindex = CTeamFortress2Mod::m_ObjectiveResource.m_WaypointAreaToIndexTranslation[m_iCurrentAttackArea];

		if (capindex >= 0)
		{
			Vector vCapAttacking = CTeamFortress2Mod::m_ObjectiveResource.GetControlPointWaypoint(capindex);

			if (DistanceFrom(vPos) > DistanceFrom(vCapAttacking))
				return;
		}
		else if ((DistanceFrom(vPos) > CWaypointLocations::REACHABLE_RANGE))
			return; // too far away, i should keep attacking
	}

	if ((vPos == Vector(0, 0, 0)) && (type == EVENT_CAPPOINT))
	{
		if (!m_iCurrentDefendArea)
			return;

		CWaypoint *pWpt = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT, CTeamFortress2Mod::GetEnemyTeam(GetTeam()), m_iCurrentDefendArea, true);

		if (!pWpt)
		{
			return;
		}

		vPos = pWpt->GetOrigin();
	}

	// everyone go back to cap point unless doing something important
	if ((type == EVENT_CAPPOINT) || (!m_pNavigator->HasNextPoint() || ((m_pNavigator->GetGoalOrigin() - GetOrigin()).Length() > ((vPos - GetOrigin()).Length()))))
	{
		dataUnconstArray<int> *failed;
		m_pNavigator->GetFailedGoals(&failed);
		CWaypoint *pWpt = NULL;
		int iIgnore = -1;

		if ((iArea >= 0) && (iArea < MAX_CONTROL_POINTS))
		{
			// get control point waypoint
			int iWpt = CTeamFortress2Mod::m_ObjectiveResource.GetControlPointWaypoint(iArea);
			pWpt = CWaypoints::GetWaypoint(iWpt);

			if (pWpt && !pWpt->CheckReachable())
			{
				iIgnore = iWpt;
				// go to nearest defend waypoint
				pWpt = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(vPos, 1024.0f, iIgnore, false, false, true, failed, false, m_iTeam, true, false, Vector(0, 0, 0), CWaypointTypes::W_FL_DEFEND, m_pEdict));
			}

		}

		if (pWpt == NULL)
			pWpt = CWaypoints::GetWaypoint(CWaypointLocations::NearestWaypoint(vPos, 400, iIgnore, false, false, true, failed, false, GetTeam(), true));

		if (pWpt)
		{
			CBotSchedule *newSched = new CBotDefendSched(CWaypoints::GetWaypointIndex(pWpt), RandomFloat(1.0f, 6.0f));
			m_pSchedules->FreeMemory();
			m_pSchedules->Add(newSched);
			newSched->SetID(SCHED_RETURN_TO_INTEL);
			m_fDefendTime = engine->Time() + RandomFloat(10.0f, 20.0f);
		}
	}

}

void CBotTF2::BuildingSapped(eEngiBuild building, edict_t *pSapper, edict_t *pSpy)
{
	static edict_t *pBuilding;

	m_pSchedules->FreeMemory();

	if (IsVisible(pSpy))
	{
		FoundSpy(pSpy, CTeamFortress2Mod::GetSpyDisguise(pSpy));
	}
	else
	{
		pBuilding = CTeamFortress2Mod::GetBuilding(building, m_pEdict);

		if (pBuilding)
		{
			m_vLastSeeSpy = CBotGlobals::EntityOrigin(pBuilding);
		}

		m_fLastSeeSpyTime = engine->Time();
	}
}

void CBotTF2::SapperDestroyed(edict_t *pSapper)
{
	m_pSchedules->FreeMemory();
}

CBotWeapon *CBotTF2::getCurrentWeapon()
{
	edict_t *pWeapon = CClassInterface::TF2_GetActiveWeapon(m_pEdict);

	if (pWeapon && !pWeapon->IsFree())
	{
		const char *pszClassname = pWeapon->GetClassName();

		if (pszClassname && *pszClassname)
		{
			return m_pWeapons->GetActiveWeapon(pszClassname, pWeapon, OverrideAmmoTypes());
		}
	}

	return NULL;
}

CBotTF2::CBotTF2()
{
	CBotFortress();
	m_iDesiredResistType = 0;
	m_pSecondary = NULL;
	m_pPrimary = NULL;
	m_pMelee = NULL;
	m_fDispenserPlaceTime = 0.0f;
	m_fDispenserHealAmount = 0.0f;
	m_fTeleporterEntPlacedTime = 0;
	m_fTeleporterExtPlacedTime = 0;
	m_iTeleportedPlayers = 0;
	m_fDoubleJumpTime = 0;
	m_fSpySapTime = 0;
	m_iCurrentDefendArea = 0;
	m_iCurrentAttackArea = 0;
	//m_bBlockPushing = false;
	//m_fBlockPushTime = 0;
	m_pDefendPayloadBomb = NULL;
	m_pPushPayloadBomb = NULL;
	m_pRedPayloadBomb = NULL;
	m_pBluePayloadBomb = NULL;
	m_iTrapType = TF_TRAP_TYPE_NONE;
	m_pLastEnemySentry = MyEHandle(NULL);
	m_prevSentryHealth = 0;
	m_prevDispHealth = 0;
	m_prevTeleExtHealth = 0;
	m_prevTeleEntHealth = 0;
	m_fHealClickTime = 0;
	m_fCheckHealTime = 0;

	m_fAttackPointTime = 0; // used in cart maps

	m_prevSentryHealth = 0;
	m_prevDispHealth = 0;
	m_prevTeleExtHealth = 0;
	m_prevTeleEntHealth = 0;

	m_iSentryArea = 0;
	m_iDispenserArea = 0;
	m_iTeleEntranceArea = 0;
	m_iTeleExitArea = 0;

	for (unsigned int i = 0; i < 10; i++)
		m_fClassDisguiseFitness[i] = 1.0f;

	memset(m_fClassDisguiseTime, 0, sizeof(float) * 10);
}

void CBotTF2::Init(bool bVarInit)
{
	if (bVarInit)
	{
		CBotTF2();
	}

	CBotFortress::Init(bVarInit);
}

bool CBotFortress::GetIgnoreBox(Vector *vLoc, float *fSize)
{
	if ((m_iClass == TF_CLASS_ENGINEER) && vLoc)
	{
		edict_t *pSentry;

		if ((pSentry = m_pSentryGun.Get()) != NULL)
		{
			*vLoc = CBotGlobals::EntityOrigin(pSentry);
			*fSize = pSentry->GetCollideable()->OBBMaxs().Length() / 2;
			return true;
		}
	}
	else if ((m_iClass == TF_CLASS_SPY) && vLoc)
	{
		edict_t *pSentry;

		if ((pSentry = m_pNearestEnemySentry.Get()) != NULL)
		{
			*vLoc = CBotGlobals::EntityOrigin(pSentry);
			*fSize = pSentry->GetCollideable()->OBBMaxs().Length() / 2;
			return true;
		}
	}

	return false;
}
