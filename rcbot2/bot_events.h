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
#ifndef __BOT_EVENT_H__
#define __BOT_EVENT_H__

#include "bot_const.h"

#include <vector>

class IGameEventListener2;
class IGameEvent;

class CBotEvent
{
public:
	CBotEvent()
	{
		m_iEventId = -1;
		m_szType = NULL;
		m_iModId = MOD_ANY;
	}

	void SetMod(eModId iModId)
	{
		m_iModId = iModId;
	}

	const bool ForCurrentMod() const;

	void SetType(char *szType);

	const bool IsType(const char *szType) const;

	virtual void Execute(IGameEvent *pEvent) { return; }

	inline void SetEventId(int iEventId)
	{
		m_iEventId = iEventId;
	}

	inline const bool IsEventId(int iEventId) const
	{
		return ForCurrentMod() && (m_iEventId == iEventId);
	}

	inline const bool HasEventId() const
	{
		return (m_iEventId != -1);
	}

	inline const char *GetName() const
	{
		return m_szType;
	}

private:
	char *m_szType;
	int m_iEventId;
	eModId m_iModId;
};

#if defined USE_NAVMESH
class CNavAreaBlockedEvent : public CBotEvent
{
public:
	CNavAreaBlockedEvent()
	{
		SetType("nav_blocked");
		SetMod(MOD_ANY);
	}

	void Execute(IGameEvent *pEvent);
};
#endif

class CPlayerHurtEvent : public CBotEvent
{
public:
	CPlayerHurtEvent()
	{
		SetType("player_hurt");
		SetMod(MOD_ANY);
	}

	void Execute(IGameEvent *pEvent);
};

class CPlayerDeathEvent : public CBotEvent
{
public:
	CPlayerDeathEvent()
	{
		SetType("player_death");
		SetMod(MOD_ANY);
	}

	void Execute(IGameEvent *pEvent);
};

class CPlayerSpawnEvent : public CBotEvent
{
public:
	CPlayerSpawnEvent()
	{
		SetType("player_spawn");
		SetMod(MOD_ANY);
	}

	void Execute(IGameEvent *pEvent);
};

class CFlagEvent : public CBotEvent
{
public:
	CFlagEvent()
	{
		SetType("teamplay_flag_event");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class COverTimeBegin : public CBotEvent
{
public:
	COverTimeBegin()
	{
		SetType("teamplay_overtime_begin");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CPlayerHealed : public CBotEvent
{
public:
	CPlayerHealed()
	{
		SetType("player_healed");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2RoundWinEvent : public CBotEvent
{
public:
	CTF2RoundWinEvent()
	{
		SetType("teamplay_round_win");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CPlayerTeleported : public CBotEvent
{
public:
	CPlayerTeleported()
	{
		SetType("player_teleported");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2ObjectSapped : public CBotEvent
{
public:
	CTF2ObjectSapped()
	{
		SetType("player_sapped_object");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2ObjectDestroyed : public CBotEvent
{
public:
	CTF2ObjectDestroyed()
	{
		SetType("object_destroyed");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2PointCaptured : public CBotEvent
{
public:
	CTF2PointCaptured()
	{
		SetType("teamplay_point_captured");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2RoundActive : public CBotEvent
{
public:
	CTF2RoundActive()
	{
		SetType("teamplay_round_active");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2PointStopCapture : public CBotEvent
{
public:
	CTF2PointStopCapture()
	{
		SetType("teamplay_capture_broken");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2PointBlockedCapture : public CBotEvent
{
public:
	CTF2PointBlockedCapture()
	{
		SetType("teamplay_capture_blocked");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2PointStartCapture : public CBotEvent
{
public:
	CTF2PointStartCapture()
	{
		SetType("teamplay_point_startcapture");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};


class CTF2MVMWaveFailedEvent : public CBotEvent
{
public:
	CTF2MVMWaveFailedEvent()
	{
		SetType("mvm_wave_failed");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2MVMWaveCompleteEvent : public CBotEvent
{
public:
	CTF2MVMWaveCompleteEvent()
	{
		SetType("mvm_wave_complete");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2PointStartTouch : public CBotEvent
{
public:
	CTF2PointStartTouch()
	{
		SetType("controlpoint_starttouch");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2PointEndTouch : public CBotEvent
{
public:
	CTF2PointEndTouch()
	{
		SetType("controlpoint_endtouch");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2RoundStart : public CBotEvent
{
public:
	CTF2RoundStart()
	{
		SetType("teamplay_round_start");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2SetupFinished : public CBotEvent
{
public:
	CTF2SetupFinished()
	{
		SetType("teamplay_setup_finished");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2BuiltObjectEvent : public CBotEvent
{
public:
	CTF2BuiltObjectEvent()
	{
		SetType("player_builtobject");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2UpgradeObjectEvent : public CBotEvent
{
public:
	CTF2UpgradeObjectEvent()
	{
		SetType("player_upgradedobject");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2ChangeClass : public CBotEvent
{
public:
	CTF2ChangeClass()
	{
		SetType("player_changeclass");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CBossSummonedEvent : public CBotEvent
{
public:
	CBossSummonedEvent(char *psztype)
	{
		SetType(psztype);
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CBossKilledEvent : public CBotEvent
{
public:
	CBossKilledEvent(char *psztype)
	{
		SetType(psztype);
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2PointLocked : public CBotEvent
{
public:
	CTF2PointLocked()
	{
		SetType("teamplay_point_locked");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2PointUnlocked : public CBotEvent
{
public:
	CTF2PointUnlocked()
	{
		SetType("teamplay_point_unlocked");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CTF2MannVsMachineAlarm : public CBotEvent
{
public:
	CTF2MannVsMachineAlarm()
	{
		SetType("mvm_bomb_alarm_triggered");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CFlagCaptured : public CBotEvent
{
public:
	CFlagCaptured()
	{
		SetType("ctf_flag_captured");
		SetMod(MOD_TF2);
	}

	void Execute(IGameEvent *pEvent);
};

class CRoundStartEvent : public CBotEvent
{
public:
	CRoundStartEvent()
	{
		SetType("round_start");
		SetMod(MOD_CSS);
	}

	void Execute(IGameEvent *pEvent);
};

class CBulletImpactEvent : public CBotEvent
{
public:
	CBulletImpactEvent()
	{
		SetType("bullet_impact");
		SetMod(MOD_CSS);
	}

	void Execute(IGameEvent *pEvent);
};

class CPlayerFootstepEvent : public CBotEvent
{
public:
	CPlayerFootstepEvent()
	{
		SetType("player_footstep");
		SetMod(MOD_CSS);
	}

	void Execute(IGameEvent *pEvent);
};

class CBombPickupEvent : public CBotEvent
{
public:
	CBombPickupEvent()
	{
		SetType("bomb_pickup");
		SetMod(MOD_CSS);
	}

	void Execute(IGameEvent *pEvent);
};

class CBombDroppedEvent : public CBotEvent
{
public:
	CBombDroppedEvent()
	{
		SetType("bomb_dropped");
		SetMod(MOD_CSS);
	}

	void Execute(IGameEvent *pEvent);
};

/*class CDODFireWeaponEvent : public CBotEvent
{
public:
	CDODFireWeaponEvent()
	{
		SetType("dod_stats_weapon_attack");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};

class CDODBombExploded : public CBotEvent
{
public:
	CDODBombExploded()
	{
		SetType("dod_bomb_exploded");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};

class CDODBombPlanted : public CBotEvent
{
public:
	CDODBombPlanted()
	{
		SetType("dod_bomb_planted");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};

class CDODBombDefused : public CBotEvent
{
public:
	CDODBombDefused()
	{
		SetType("dod_bomb_defused");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};

class CDODPointCaptured : public CBotEvent
{
public:
	CDODPointCaptured()
	{
		SetType("dod_point_captured");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};

class CDODChangeClass : public CBotEvent
{
public:
	CDODChangeClass()
	{
		SetType("player_changeclass");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};

class CDODRoundStart : public CBotEvent
{
public:
	CDODRoundStart()
	{
		SetType("dod_round_start");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};

class CDODRoundActive : public CBotEvent
{
public:
	CDODRoundActive()
	{
		SetType("dod_round_active");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};

class CDODRoundWin : public CBotEvent
{
public:
	CDODRoundWin()
	{
		SetType("dod_round_win");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};

class CDODRoundOver : public CBotEvent
{
public:
	CDODRoundOver()
	{
		SetType("dod_game_over");
		SetMod(MOD_DOD);
	}

	void Execute(IGameEvent *pEvent);
};*/

class CBotEvents : public IGameEventListener2
{
public:
	static void SetupEvents();

	virtual void FireGameEvent(IGameEvent *pEvent);

	static void FreeMemory();

	static void AddEvent(CBotEvent *pEvent);

private:
	static std::vector<CBotEvent*> m_theEvents;
};
#endif