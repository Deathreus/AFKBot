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

#include "../extension.h"

#include "bot_base.h"
#include "bot_event.h"
#include "bot_strings.h"
#include "bot_globals.h"
#include "bot_fortress.h"
//#include "bot_dod_bot.h"
#include "bot_weapons.h"
#include "bot_Getprop.h"
//#include "bot_dod_bot.h"
#include "bot_squads.h"
#include "bot_schedule.h"
#include "bot_waypoint_locations.h"

vector<CBotEvent*> CBotEvents::m_theEvents;
extern ConVar bot_use_vc_commands;

extern IPlayerInfoManager* playerinfomanager;
///////////////////////////////////////////////////////

class CBotSeeFriendlyKill : public IBotFunction
{
public:
	CBotSeeFriendlyKill(edict_t *pTeammate, edict_t *pDied, const char *szKillerWeapon)
	{
		m_pTeammate = pTeammate;
		m_pWeapon = CWeapons::GetWeaponByShortName(szKillerWeapon);
		m_pDied = pDied;
	}
	void Execute(CBot *pBot)
	{
		if (CClassInterface::GetTeam(m_pTeammate) != pBot->GetTeam())
			return;

		if (pBot->GetEdict() != m_pTeammate)
		{
			if (pBot->IsVisible(m_pTeammate))
				pBot->SeeFriendlyKill(m_pTeammate, m_pDied, m_pWeapon);
		}
	}
private:
	edict_t *m_pTeammate;
	edict_t *m_pDied;
	CWeapon *m_pWeapon;
};

class CBotWaveCompleteMVM : public IBotFunction
{
public:

	void Execute(CBot *pBot)
	{
		((CBotTF2*)pBot)->MannVsMachineWaveComplete();
	}

};

class CBotSeeFriendlyHurtEnemy : public IBotFunction
{
public:
	CBotSeeFriendlyHurtEnemy(edict_t *pTeammate, edict_t *pEnemy, int iWeaponID)
	{
		m_pTeammate = pTeammate;
		m_pEnemy = pEnemy;
		m_pWeapon = CWeapons::GetWeapon(iWeaponID);
	}

	void Execute(CBot *pBot)
	{
		if (CClassInterface::GetTeam(m_pTeammate) != pBot->GetTeam())
			return;

		if (pBot->GetEdict() != m_pTeammate)
		{
			if (pBot->IsVisible(m_pTeammate) && pBot->IsVisible(m_pEnemy))
				pBot->SeeFriendlyHurtEnemy(m_pTeammate, m_pEnemy, m_pWeapon);
		}
	}
private:
	edict_t *m_pTeammate;
	edict_t *m_pEnemy;
	CWeapon *m_pWeapon;
};

class CBroadcastMVMAlarm : public IBotFunction
{
public:
	CBroadcastMVMAlarm(float fRadius)
	{
		m_bValid = CTeamFortress2Mod::GetMVMCapturePoint(&m_vLoc);
		m_fRadius = fRadius;
	}

	void Execute(CBot *pBot)
	{
		if (m_bValid)
			((CBotTF2*)pBot)->MannVsMachineAlarmTriggered(m_vLoc + Vector(RandomFloat(-m_fRadius, m_fRadius), RandomFloat(-m_fRadius, m_fRadius), 0));
	}
private:
	Vector m_vLoc;
	float m_fRadius;
	bool m_bValid;
};


class CBotSeeEnemyHurtFriendly : public IBotFunction
{
public:
	CBotSeeEnemyHurtFriendly(edict_t *pEnemy, edict_t *pTeammate, int iWeaponID)
	{
		m_pTeammate = pTeammate;
		m_pEnemy = pEnemy;
		m_pWeapon = CWeapons::GetWeapon(iWeaponID);
	}

	void Execute(CBot *pBot)
	{
		if (CClassInterface::GetTeam(m_pTeammate) != pBot->GetTeam())
			return;

		if (pBot->GetEdict() != m_pTeammate)
		{
			if (pBot->IsVisible(m_pTeammate))
				pBot->SeeEnemyHurtFriendly(m_pTeammate, m_pEnemy, m_pWeapon);
		}
	}
private:
	edict_t *m_pTeammate;
	edict_t *m_pEnemy;
	CWeapon *m_pWeapon;
};


class CBotSeeFriendlyDie : public IBotFunction
{
public:
	CBotSeeFriendlyDie(edict_t *pDied, edict_t *pKiller, const char *szKillerWeapon)
	{
		m_pDied = pDied;
		m_pWeapon = CWeapons::GetWeaponByShortName(szKillerWeapon);
		m_pKiller = pKiller;
	}
	void Execute(CBot *pBot)
	{
		if (CClassInterface::GetTeam(m_pDied) != pBot->GetTeam())
			return;

		if (pBot->GetEdict() != m_pDied)
		{
			if (pBot->IsVisible(m_pDied))
				pBot->SeeFriendlyDie(m_pDied, m_pKiller, m_pWeapon);
		}
	}
private:
	edict_t *m_pDied;
	edict_t *m_pKiller;
	CWeapon *m_pWeapon;
};

class CBotHearPlayerAttack : public IBotFunction
{
public:
	CBotHearPlayerAttack(edict_t *pAttacker, int iWeaponID)
	{
		m_pAttacker = pAttacker;
		m_iWeaponID = iWeaponID;
	}

	void Execute(CBot *pBot)
	{
		extern ConVar bot_listen_dist;

		if (!pBot->HasEnemy() && (pBot->WantToListen() || pBot->IsListeningToPlayer(m_pAttacker)) && pBot->WantToListenToPlayerAttack(m_pAttacker, m_iWeaponID))
		{
			float fDistance = pBot->DistanceFrom(m_pAttacker);

			// add some fuzz based on distance
			if (RandomFloat(0.0f, bot_listen_dist.GetFloat()) > fDistance)
				pBot->HearPlayerAttack(m_pAttacker, m_iWeaponID);
		}
	}
private:
	edict_t *m_pAttacker;
	int m_iWeaponID;
};

class CTF2BroadcastRoundWin : public IBotFunction
{
public:
	CTF2BroadcastRoundWin(int iTeamWon, bool bFullRound)
	{
		m_iTeam = iTeamWon;
		m_bFullRound = bFullRound;
	}

	void Execute(CBot *pBot)
	{
		((CBotTF2*)pBot)->RoundWon(m_iTeam, m_bFullRound);
	}
private:
	int m_iTeam;
	bool m_bFullRound;
};
////////////////////////////////////////////////


/*void CRoundStartEvent::Execute ( IBotEventInterface *pEvent )
{
	CBots::RoundStart();
}*/

void CPlayerHurtEvent::Execute(IBotEventInterface *pEvent)
{
	CBot *pBot = CBots::GetBotPointer(m_pActivator);
	int iAttacker = pEvent->GetInt("attacker", 0);
	int iWeaponId = pEvent->GetInt("weaponid", -1);

	if (iAttacker > 0)
	{
		edict_t *pAttacker = CBotGlobals::PlayerByUserId(iAttacker);
		if (m_pActivator != pAttacker)
		{
			if (pAttacker && (!pAttacker->m_pNetworkable || !pAttacker->m_NetworkSerialNumber))
				pAttacker = NULL;

			if (pBot)
			{
				pBot->Hurt(pAttacker, pEvent->GetInt("health"));
			}

			pBot = CBots::GetBotPointer(pAttacker);

			if (pBot)
			{
				pBot->Shot(m_pActivator);
			}

			if (CBotGlobals::IsPlayer(m_pActivator) && CBotGlobals::IsPlayer(pAttacker))
			{
				CBotSeeFriendlyHurtEnemy func1(pAttacker, m_pActivator, iWeaponId);
				CBotSeeEnemyHurtFriendly func2(pAttacker, m_pActivator, iWeaponId);

				CBots::BotFunction(&func1);
				CBots::BotFunction(&func2);
			}
		}

	}
	//CBots::BotFunction()
}

void CPlayerDeathEvent::Execute(IBotEventInterface *pEvent)
{
	CBot *pBot = CBots::GetBotPointer(m_pActivator);
	const char *weapon = pEvent->GetString("weapon", NULL);
	CBotSquad *pPrevSquadLeadersSquad = NULL;
	int iAttacker = pEvent->GetInt("attacker", 0);

	edict_t *pAttacker = (iAttacker > 0) ? CBotGlobals::PlayerByUserId(iAttacker) : NULL;

	if (pBot)
		pBot->Died(pAttacker, weapon);

	pBot = CBots::GetBotPointer(pAttacker);

	if (pBot)
	{
		pBot->Killed(m_pActivator, (char*)weapon);
		pBot->EnemyDown(m_pActivator);
	}

	if (m_pActivator && pAttacker) // not worldspawn
	{
		CBotSeeFriendlyDie func1(m_pActivator, pAttacker, weapon);
		CBotSeeFriendlyKill func2(pAttacker, m_pActivator, weapon);

		CBots::BotFunction(&func1);
		CBots::BotFunction(&func2);
	}

	if ((pPrevSquadLeadersSquad = CBotSquads::FindSquadByLeader(m_pActivator)) != NULL)
	{
		CBotSquads::ChangeLeader(pPrevSquadLeadersSquad);
	}
}

void CPlayerSpawnEvent::Execute(IBotEventInterface *pEvent)
{
	CBot *pBot = CBots::GetBotPointer(m_pActivator);

	if (pBot)
		pBot->SpawnInit();

	if (CBotGlobals::IsCurrentMod(MOD_TF2))
	{
		if (pEvent->GetInt("class") == TF_CLASS_MEDIC)
		{
			// find medigun
			CTeamFortress2Mod::FindMediGun(m_pActivator);
		}
	}
}

void CBulletImpactEvent::Execute(IBotEventInterface *pEvent)
{
	CBot *pBot = CBots::GetBotPointer(m_pActivator);

	if (pBot)
	{
		pBot->ShotMiss();
	}
}
/////////////////////////////////////////

void CTF2ObjectSapped::Execute(IBotEventInterface *pEvent)
{
	int owner = pEvent->GetInt("ownerid", -1);
	int building = pEvent->GetInt("object", -1);
	int sapperid = pEvent->GetInt("sapperid", -1);

	if (m_pActivator && (owner >= 0) && (building >= 0) && (sapperid >= 0))
	{
		edict_t *pSpy = m_pActivator;
		edict_t *pOwner = CBotGlobals::PlayerByUserId(owner);
		edict_t *pSapper = INDEXENT(sapperid);
		CBotTF2 *pBot = (CBotTF2*)CBots::GetBotPointer(pOwner);

		if (pBot)
		{
			pBot->BuildingSapped((eEngiBuild)building, pSapper, pSpy);
		}

		CTeamFortress2Mod::SapperPlaced(pOwner, (eEngiBuild)building, pSapper);

		CBroadcastSpySap spysap = CBroadcastSpySap(pSpy);

		CBots::BotFunction(&spysap);

	}
}

void CTF2RoundActive::Execute(IBotEventInterface *pEvent)
{
	if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
		CTeamFortress2Mod::RoundStarted();
	else
		CTeamFortress2Mod::ResetSetupTime();
}

void COverTimeBegin::Execute(IBotEventInterface *pEvent)
{
	CBroadcastOvertime function;

	CBots::BotFunction(&function);
}

void CBossSummonedEvent::Execute(IBotEventInterface *pEvent)
{
	CTeamFortress2Mod::InitBoss(true);
}

void CBossKilledEvent::Execute(IBotEventInterface *pEvent)
{
	CTeamFortress2Mod::InitBoss(false);
}

void CPlayerTeleported::Execute(IBotEventInterface *pEvent)
{
	int builderid = pEvent->GetInt("builderid", -1);

	if (builderid >= 0)
	{
		edict_t *pPlayer = CBotGlobals::PlayerByUserId(builderid);

		CBot *pBot = CBots::GetBotPointer(pPlayer);

		if (pBot)
		{
			((CBotTF2*)pBot)->TeleportedPlayer();
		}

		CTeamFortress2Mod::UpdateTeleportTime(pPlayer);

	}
}

void CPlayerHealed::Execute(IBotEventInterface *pEvent)
{
	int patient = pEvent->GetInt("patient", -1);
	int healer = pEvent->GetInt("healer", -1);
	int amount = pEvent->GetFloat("amount", 0);

	if ((healer != -1) && (patient != -1) && (healer != patient))
	{
		m_pActivator = CBotGlobals::PlayerByUserId(patient);

		if (m_pActivator)
		{
			CBot *pBot = CBots::GetBotPointer(m_pActivator);

			if (pBot)
			{
				CBotTF2 *pBotTF2 = (CBotTF2*)pBot;

				if (pBotTF2 && RandomInt(0, 1))
					pBotTF2->AddVoiceCommand(TF_VC_THANKS);
			}
		}

		CBot *pBot = CBots::GetBotPointer(CBotGlobals::PlayerByUserId(healer));

		if (pBot && pBot->IsTF2())
		{
			((CBotTF2*)pBot)->HealedPlayer(m_pActivator, amount);
		}
	}
}

void CTF2ObjectDestroyed::Execute(IBotEventInterface *pEvent)
{
	int type = pEvent->GetInt("objecttype", -1);
	int index = pEvent->GetInt("index", -1);
	int was_building = pEvent->GetInt("was_building", -1);
	int iAttacker = pEvent->GetInt("attacker", -1);

	if (iAttacker != -1)
	{
		edict_t *pAttacker = CBotGlobals::PlayerByUserId(iAttacker);

		if (pAttacker && m_pActivator && (type >= 0) && (index >= 0) && (was_building >= 0))
		{
			//if ( !was_building )
			//{ // could be a sapper
			if ((eEngiBuild)type == ENGI_SAPPER)
			{
				edict_t *pOwner = pAttacker;
				edict_t *pSapper = INDEXENT(index);
				CBotTF2 *pBot = (CBotTF2*)CBots::GetBotPointer(pOwner);

				if (pBot)
					pBot->SapperDestroyed(pSapper);

				CTeamFortress2Mod::SapperDestroyed(pOwner, (eEngiBuild)type, pSapper);
			}
			else
			{
				CBotTF2 *pBot = (CBotTF2*)CBots::GetBotPointer(m_pActivator);

				if (pBot)
				{
					edict_t *pBuilding = INDEXENT(index);

					pBot->BuildingDestroyed(type, pAttacker, pBuilding);
				}
			}
			//}
		}

	}


}

void CTF2UpgradeObjectEvent::Execute(IBotEventInterface *pEvent)
{
	if (bot_use_vc_commands.GetBool() && RandomInt(0, 1))
	{
		eEngiBuild object = (eEngiBuild)pEvent->GetInt("object", 0);
		bool isbuilder = (pEvent->GetInt("isbuilder") > 0);
		short index = pEvent->GetInt("index");

		if (!isbuilder)
		{
			// see if builder is a bot
			edict_t *pOwner = CTeamFortress2Mod::GetBuildingOwner(object, index);
			CBotTF2 *pBot;

			if ((pBot = (CBotTF2*)CBots::GetBotPointer(pOwner)) != NULL)
			{
				pBot->AddVoiceCommand(TF_VC_THANKS);
			}
		}
	}
}

void CTF2RoundWinEvent::Execute(IBotEventInterface *pEvent)
{
	int iWinningTeam = pEvent->GetInt("team");
	CTF2BroadcastRoundWin *function = new CTF2BroadcastRoundWin(iWinningTeam, pEvent->GetInt("full_round") == 1);

	CBots::BotFunction(function);

	CTeamFortress2Mod::RoundWon(iWinningTeam);

	delete function;
}


void CTF2SetupFinished::Execute(IBotEventInterface *pEvent)
{
	CTeamFortress2Mod::RoundStarted();
}

void CTF2BuiltObjectEvent::Execute(IBotEventInterface *pEvent)
{
	eEngiBuild type = (eEngiBuild)pEvent->GetInt("object");
	int index = pEvent->GetInt("index");
	edict_t *pBuilding = INDEXENT(index);
	CBot *pBot = CBots::GetBotPointer(m_pActivator);

	if (type == ENGI_TELE)
	{
		CTeamFortress2Mod::TeleporterBuilt(m_pActivator, type, pBuilding);
	}

	if (type == ENGI_SENTRY)
	{
		CTeamFortress2Mod::SentryBuilt(m_pActivator, type, pBuilding);
	}

	if (type == ENGI_DISP)
	{
		CTeamFortress2Mod::DispenserBuilt(m_pActivator, type, pBuilding);
	}

	if (pBot && pBot->IsTF())
	{
		((CBotFortress*)pBot)->EngiBuildSuccess((eEngiBuild)pEvent->GetInt("object"), pEvent->GetInt("index"));
	}
}

void CTF2ChangeClass::Execute(IBotEventInterface *pEvent)
{
	CBot *pBot = CBots::GetBotPointer(m_pActivator);

	if (pBot && pBot->IsTF())
	{

		int _class = pEvent->GetInt("class");

		((CBotFortress*)pBot)->SetClass((TFClass)_class);

	}
}

void CTF2MVMWaveCompleteEvent::Execute(IBotEventInterface *pEvent)
{
	CBotWaveCompleteMVM func;

	CTeamFortress2Mod::MVMAlarmReset();
	CTeamFortress2Mod::RoundReset();

	CBots::BotFunction(&func);
}

void CTF2MVMWaveFailedEvent::Execute(IBotEventInterface *pEvent)
{
	CTeamFortress2Mod::MVMAlarmReset();
	CTeamFortress2Mod::RoundReset();
}

void CTF2RoundStart::Execute(IBotEventInterface *pEvent)
{
	// 04/07/09 : add full reset

	CBroadcastRoundStart roundstart = CBroadcastRoundStart(pEvent->GetInt("full_reset") == 1);

	if (pEvent->GetInt("full_reset") == 1)
	{
		//CPoints::resetPoints();
	}

	// MUST BE BEFORE RESET SETUP TIME
	CTeamFortress2Mod::SetPointOpenTime(30.0f);

	// MUST BE AFTER RESETPOINTS
	CTeamFortress2Mod::RoundReset();

	CTeamFortress2Mod::ResetSetupTime();

	CBots::BotFunction(&roundstart);

}

void CTF2PointStopCapture::Execute(IBotEventInterface *pEvent)
{
	int capindex = pEvent->GetInt("cp", 0);

	CTeamFortress2Mod::RemoveCappers(capindex);

}

void CTF2PointBlockedCapture::Execute(IBotEventInterface *pEvent)
{
	int capindex = pEvent->GetInt("cp", 0);

	CTeamFortress2Mod::RemoveCappers(capindex);
}
void CTF2PointUnlocked::Execute(IBotEventInterface *pEvent)
{
	CTeamFortress2Mod::SetPointOpenTime(0);
}

void CTF2PointLocked::Execute(IBotEventInterface *pEvent)
{
	//
}

void CTF2PointStartTouch::Execute(IBotEventInterface *pEvent)
{
	int capindex = pEvent->GetInt("area", 0);
	int iplayerIndex = pEvent->GetInt("player", -1);
	//	const char *cpname = pEvent->GetString("cpname");

	edict_t *pPlayer = INDEXENT(iplayerIndex);

	if ((capindex >= 0) && (CTeamFortress2Mod::m_ObjectiveResource.GetNumControlPoints() > 0) &&
		CTeamFortress2Mod::m_ObjectiveResource.GetOwningTeam(capindex) == CClassInterface::GetTeam(pPlayer))
	{
		CTeamFortress2Mod::AddCapDefender(pPlayer, capindex);
	}
}

void CTF2PointEndTouch::Execute(IBotEventInterface *pEvent)
{
	int capindex = pEvent->GetInt("area", 0);
	int iplayerIndex = pEvent->GetInt("player", -1);
	//	const char *cpname = pEvent->GetString("cpname");

	edict_t *pPlayer = INDEXENT(iplayerIndex);

	if ((capindex >= 0) && (CTeamFortress2Mod::m_ObjectiveResource.GetNumControlPoints() > 0) &&
		CTeamFortress2Mod::m_ObjectiveResource.GetOwningTeam(capindex) == CClassInterface::GetTeam(pPlayer))
	{
		CTeamFortress2Mod::RemoveCapDefender(pPlayer, capindex);
	}
}

void CTF2PointStartCapture::Execute(IBotEventInterface *pEvent)
{
	int capteam = pEvent->GetInt("capteam", 0);
	int capindex = pEvent->GetInt("cp", 0);
	const char *cappers = pEvent->GetString("cappers", NULL);
	//	const char *cpname = pEvent->GetString("cpname");

	if (cappers)
	{
		int i = 0;

		while (cappers[i] != 0)
		{
			CTeamFortress2Mod::AddCapper(capindex, (int)cappers[i]);
			i++;
		}
	}

	CTeamFortress2Mod::m_ObjectiveResource.UpdateCaptureTime(capindex);
	//CPoints::pointBeingCaptured(capteam,cpname,cappers[0]);

	CBotTF2FunctionEnemyAtIntel *function = new CBotTF2FunctionEnemyAtIntel(capteam, CTeamFortress2Mod::m_ObjectiveResource.GetCPPosition(capindex), EVENT_CAPPOINT, NULL, capindex);

	CBots::BotFunction(function);

	delete function;

}

void CTF2MannVsMachineAlarm::Execute(IBotEventInterface *pEvent)
{
	CBroadcastMVMAlarm alarm = CBroadcastMVMAlarm(CTeamFortress2Mod::GetMVMCapturePointRadius());
	// MUST BE AFTER POINTS HAVE BEEN UPDATED!
	CBots::BotFunction(&alarm);

	CTeamFortress2Mod::MVMAlarmSounded();
}

void CTF2PointCaptured::Execute(IBotEventInterface *pEvent)
{
	CBroadcastCapturedPoint cap = CBroadcastCapturedPoint(pEvent->GetInt("cp"), pEvent->GetInt("team"), pEvent->GetString("cpname"));

	//CTeamFortress2Mod::m_Resource.debugprint();
	CTeamFortress2Mod::UpdatePointMaster();

	// update points
	CTeamFortress2Mod::m_ObjectiveResource.m_fUpdatePointTime = 0;
	CTeamFortress2Mod::m_ObjectiveResource.m_fNextCheckMonitoredPoint = engine->Time() + 0.2f;

	// MUST BE AFTER POINTS HAVE BEEN UPDATED!
	CBots::BotFunction(&cap);

}

/* Flag has been picked up or dropped */
#define FLAG_PICKUP		1
#define FLAG_CAPTURED	2
#define FLAG_DEFEND		3
#define FLAG_DROPPED	4
#define FLAG_RETURN		5

void CFlagEvent::Execute(IBotEventInterface *pEvent)
{
	// dropped / picked up ID
	int type = pEvent->GetInt("eventtype");
	// player id
	int player = pEvent->GetInt("player");

	edict_t *pPlayer = NULL;
	CBot *pBot = NULL;

	// Crash fix
	if (player)
	{
		pPlayer = INDEXENT(player);
		pBot = CBots::GetBotPointer(pPlayer);
	}

	switch (type)
	{
	case FLAG_PICKUP: // pickup
		if (pBot && pBot->IsTF())
		{
			((CBotTF2*)pBot)->PickedUpFlag();
		}

		if (pPlayer)
		{
			int iTeam = CTeamFortress2Mod::GetTeam(pPlayer);
			CTeamFortress2Mod::FlagPickedUp(iTeam, pPlayer);
		}

		break;
	case FLAG_CAPTURED: // captured
	{
		IPlayerInfo *p = NULL;

		if (pPlayer)
		{
			p = playerinfomanager->GetPlayerInfo(pPlayer);

			if (p)
			{
				CBroadcastFlagCaptured captured = CBroadcastFlagCaptured(p->GetTeamIndex());
				CBots::BotFunction(&captured);
			}
		}

		if (pBot && pBot->IsTF())
		{
			((CBotTF2*)pBot)->CapturedFlag();
			((CBotTF2*)pBot)->DroppedFlag();
		}

		if (pPlayer)
		{
			int iTeam = CTeamFortress2Mod::GetTeam(pPlayer);
			CTeamFortress2Mod::FlagDropped(iTeam, Vector(0, 0, 0));
		}

		CTeamFortress2Mod::ResetFlagStateToDefault();

	}
	break;
	case FLAG_DROPPED: // drop
	{
		IPlayerInfo *p = playerhelpers->GetGamePlayer(pPlayer)->GetPlayerInfo();
		Vector vLoc;

		if (p)
		{
			vLoc = CBotGlobals::EntityOrigin(pPlayer);
			CBroadcastFlagDropped dropped = CBroadcastFlagDropped(p->GetTeamIndex(), vLoc);
			CBots::BotFunction(&dropped);
		}

		if (pBot && pBot->IsTF())
			((CBotTF2*)pBot)->DroppedFlag();


		if (pPlayer)
			CTeamFortress2Mod::FlagDropped(CTeamFortress2Mod::GetTeam(pPlayer), vLoc);
	}
	break;
	case FLAG_RETURN:
	{
		if (CTeamFortress2Mod::IsMapType(TF_MAP_SD))
		{
			CBroadcastFlagReturned returned = CBroadcastFlagReturned(CTeamFortress2Mod::GetFlagCarrierTeam());
			CBots::BotFunction(&returned);
		}
		CTeamFortress2Mod::ResetFlagStateToDefault();

		CTeamFortress2Mod::FlagReturned(0); // for special delivery
		//p->GetTeamIndex(),CBotGlobals::EntityOrigin(pPlayer));
	}
	break;
	default:
		break;
	}

}

void CFlagCaptured::Execute(IBotEventInterface *pEvent)
{

}
/////////////////////////////////////////////////
/*void CDODPointCaptured :: Execute ( IBotEventInterface *pEvent )
{
int cp = pEvent->GetInt("cp");
const char *szCappers = pEvent->GetString("cappers",NULL);
edict_t *pPlayer;

// Get a capper
int userid = szCappers[0];

int team = 0;

// find the team - should be a player index
if ( (userid >= 0) && (userid <= MAX_PLAYERS) )
{
pPlayer = INDEXENT(userid);
team = CClassInterface::GetTeam(pPlayer);

CClient *pClient = CClients::Get(pPlayer);

if ( pClient && pClient->autoWaypointOn() )
{
pClient->autoEventWaypoint(CWaypointTypes::W_FL_CAPPOINT,150.0f,false,0,Vector(0,0,0),true);
}
}

if ( team )
{
CBroadcastBombEvent func(DOD_POINT_CAPTURED,cp,team);

CBots::botFunction(&func);
}
}

void CDODBombExploded :: Execute ( IBotEventInterface *pEvent )
{
int cp = pEvent->GetInt("cp");
int team = CClassInterface::GetTeam(m_pActivator);

if ( m_pActivator )
{
CBroadcastBombEvent func(DOD_BOMB_EXPLODED,cp,team);

CBots::botFunction(&func);
}

CDODMod::m_Flags.setBombPlanted(cp,false);
}

void CDODBombDefused :: Execute ( IBotEventInterface *pEvent )
{
int cp = pEvent->GetInt("cp");
int team = pEvent->GetInt("team");

CDODMod::m_Flags.setBombPlanted(cp,false);

CBroadcastBombEvent func(DOD_BOMB_DEFUSE,cp,team);

CBots::botFunction(&func);
}

void CDODBombPlanted :: Execute ( IBotEventInterface *pEvent )
{
int cp = pEvent->GetInt("cp");
int team = pEvent->GetInt("team");

CBroadcastBombEvent func(DOD_BOMB_PLANT,cp,team);

/*	if ( m_pActivator )
{
CBot *pBot;

if ( (pBot = CBots::GetBotPointer(m_pActivator)) != NULL )
{
// hack
((CDODBot*)pBot)->removeBomb();
}
}*/

/*CBots::botFunction(&func);

CDODMod::m_Flags.setBombPlanted(cp,true);

}

void CDODRoundStart :: Execute ( IBotEventInterface *pEvent )
{
CDODMod::roundStart();
}

void CDODRoundActive :: Execute ( IBotEventInterface *pEvent )
{

}

void CDODRoundWin :: Execute ( IBotEventInterface *pEvent )
{
//CDODMod::m_Flags.reset();
}

void CDODRoundOver :: Execute ( IBotEventInterface *pEvent )
{
//CDODMod::m_Flags.reset();
}

void CDODChangeClass :: Execute ( IBotEventInterface *pEvent )
{
if ( m_pActivator )
{
CBot *pBot = CBots::GetBotPointer(m_pActivator);

if ( pBot )
{
CDODBot *pDODBot = (CDODBot*)pBot;

pDODBot->selectedClass(pEvent->GetInt("class"));
}
}
}

void CDODFireWeaponEvent :: Execute ( IBotEventInterface *pEvent )
{
int iAttacker = pEvent->GetInt("attacker",-1);

if ( iAttacker >= 0 )
{
edict_t *pAttacker = CBotGlobals::PlayrByUserId(iAttacker);
int iWeaponID = pEvent->GetInt("weapon",-1);

CBotHearPlayerAttack *func = new CBotHearPlayerAttack(pAttacker,iWeaponID);

CBots::botFunction(func);
delete func;
}


}*/

///////////////////////////////////////////////////////

void CBotEvent::SetType(char *szType)
{
	m_szType = CStrings::GetString(szType);
}

bool CBotEvent::ForCurrentMod()
{
	return ((m_iModId == MOD_ANY) || (CBotGlobals::IsMod(m_iModId)));
}
// should we Execute this ??
inline bool CBotEvent::IsType(const char *szType)
{
	return ForCurrentMod() && FStrEq(m_szType, szType);
}

///////////////////////////////////////////////////////
void CBotEvents::SetupEvents()
{
	AddEvent(new CPlayerHurtEvent());
	AddEvent(new CPlayerDeathEvent());
	AddEvent(new CBulletImpactEvent());
	AddEvent(new CFlagEvent());
	AddEvent(new CPlayerSpawnEvent());

	////////////// css
	/*AddEvent(new CRoundStartEvent());
	AddEvent(new CBombPickupEvent());
	AddEvent(new CBombDroppedEvent());*/

	////////////// dods
	/*AddEvent(new CDODChangeClass());
	AddEvent(new CDODBombPlanted());
	AddEvent(new CDODBombExploded());
	AddEvent(new CDODBombDefused());
	AddEvent(new CDODPointCaptured());
	AddEvent(new CDODFireWeaponEvent());
	AddEvent(new CDODRoundStart());
	AddEvent(new CDODRoundActive());
	AddEvent(new CDODRoundWin());
	AddEvent(new CDODRoundOver());*/

	////////////// tf2
	AddEvent(new CTF2BuiltObjectEvent());
	AddEvent(new CTF2ChangeClass());
	AddEvent(new CTF2RoundStart());
	AddEvent(new CTF2PointCaptured());
	AddEvent(new CTF2PointStartCapture());
	AddEvent(new CTF2ObjectSapped());
	AddEvent(new CTF2ObjectDestroyed());
	AddEvent(new CTF2PointStopCapture());
	AddEvent(new CTF2PointBlockedCapture());
	AddEvent(new CTF2UpgradeObjectEvent());
	AddEvent(new CTF2SetupFinished());
	AddEvent(new COverTimeBegin());
	AddEvent(new CPlayerHealed());
	AddEvent(new CPlayerTeleported());
	AddEvent(new CTF2RoundWinEvent());
	AddEvent(new CTF2PointUnlocked());
	AddEvent(new CTF2PointLocked());
	AddEvent(new CTF2MannVsMachineAlarm());
	AddEvent(new CTF2MVMWaveCompleteEvent());
	AddEvent(new CTF2MVMWaveFailedEvent());
	AddEvent(new CTF2RoundActive());
	AddEvent(new CTF2PointStartTouch());
	AddEvent(new CTF2PointEndTouch());

	AddEvent(new CBossSummonedEvent("pumpkin_lord_summoned"));
	AddEvent(new CBossSummonedEvent("merasmus_summoned"));
	AddEvent(new CBossSummonedEvent("eyeball_boss_summoned"));
	AddEvent(new CBossKilledEvent("pumpkin_lord_killed"));
	AddEvent(new CBossKilledEvent("merasmus_killed"));
	AddEvent(new CBossKilledEvent("merasmus_escaped"));
	AddEvent(new CBossKilledEvent("eyeball_boss_killed"));
	AddEvent(new CBossKilledEvent("eyeball_boss_escaped"));
}

void CBotEvents::AddEvent(CBotEvent *pEvent)
{
	m_theEvents.push_back(pEvent);
}

void CBotEvents::FreeMemory()
{
	for (unsigned int i = 0; i < m_theEvents.size(); i++)
	{
		delete m_theEvents[i];
		m_theEvents[i] = NULL;
	}
	m_theEvents.clear();
}

void CBotEvents::ExecuteEvent(IGameEvent *pEvent, eBotEventType iType)
{
	CBotEvent *pFound;
	int iEventId = -1;
	bool bFound;

	IBotEventInterface *pInterface = new CGameEventInterface2(pEvent);

	if (pInterface == NULL)
		return;

	if (iType != TYPE_IGAMEEVENT)
		iEventId = pInterface->GetInt("eventid");

	for (register unsigned short int i = 0; i < m_theEvents.size(); i++)
	{
		pFound = m_theEvents[i];
		bFound = pFound->ForCurrentMod() && pFound->IsType(pInterface->GetName());

		if (bFound)
		{
			int userid = pInterface->GetInt("userid", -1);
			// set pEvent id for quick checking
			pFound->SetEventId(iEventId);

			pFound->SetActivator((userid >= 0) ? CBotGlobals::PlayerByUserId(userid) : NULL);

			pFound->Execute(pInterface);

			break;
		}
	}

	delete pInterface;
}
