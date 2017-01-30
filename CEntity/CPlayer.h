/**
* =============================================================================
* CEntity Entity Handling Framework
* Copyright (C) 2011 Matt Woodrow.  All rights reserved.
* =============================================================================
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _INCLUDE_CPLAYER_H_
#define _INCLUDE_CPLAYER_H_

#include "CEntity.h"
#include "sh_list.h"
#include "shareddefs.h"
#include "usercmd.h"
#include "mathlib.h"

class CPlayer : public CEntity
{
public:
	DECLARE_CLASS(CPlayer, CEntity);
	
	virtual bool IsPlayer();

	bool IsAlive();
	void SetPlayerClass(int playerclass, bool persistant = true);
	int GetPlayerClass();
	int GetPlayerCond();
	bool IsDisguised();
	int GetDisguisedTeam();
	int GetButtons();
	virtual void Spawn();

public:
	int GetHealth();
	void SetHealth(int health);
	float GetMovementSpeed();
	void SetMovementSpeed(float speed);

	Vector EyePosition();
	virtual QAngle *EyeAngles();
	int GetObserverMode();
	CEntity* GetObserverTarget();
	CEntity* GetAimTarget(bool playersOnly);
	CEntity* GetRagdoll();

public: // CBasePlayer virtuals
	virtual	bool FVisible(CEntity *pEntity, int traceMask = MASK_BLOCKLOS, CEntity **ppBlocker = NULL);
	virtual bool FInViewCone(CEntity *pEntity);
	virtual bool FInAimCone(CEntity *pEntity);
	virtual bool IsLookingTowards(const CEntity *pEntity, float fTolerance);
	virtual bool IsInFieldOfView(CEntity *pEntity);
	virtual void ProcessUsercmds(CUserCmd *cmds, int numcmds, int totalcmds, int dropped_packets, bool paused);
	virtual void Jump(void);
	virtual void Duck(void);
	virtual bool Weapon_Switch(CBaseEntity *pWeapon, int viewmodelindex);
	virtual void HandleCommand_JoinClass(const char *pClass, bool bAllowSpawn);

public: // CTFPlayer virtuals
	virtual void Weapon_Equip(CBaseEntity *pWeapon);
	virtual CBaseEntity *Weapon_GetSlot(int slot);

public: //Autohandlers
	DECLARE_DEFAULTHEADER(FVisible, bool, (CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker));
	DECLARE_DEFAULTHEADER(FInViewCone, bool, (CBaseEntity *pEntity));
	DECLARE_DEFAULTHEADER(FInAimCone, bool, (CBaseEntity *pEntity));
	DECLARE_DEFAULTHEADER(IsLookingTowards, bool, (const CBaseEntity *pEntity, float fTolerance));
	DECLARE_DEFAULTHEADER(IsInFieldOfView, bool, (CBaseEntity *pEntity));
	DECLARE_DEFAULTHEADER(ProcessUsercmds, void, (CUserCmd *cmds, int numcmds, int totalcmds, int dropped_packets, bool paused));
	DECLARE_DEFAULTHEADER(Jump, void, ());
	DECLARE_DEFAULTHEADER(Duck, void, ());
	DECLARE_DEFAULTHEADER(IsReadyToSpawn, bool, ());
	DECLARE_DEFAULTHEADER(CanSpeakVoiceCommand, bool, ());
	DECLARE_DEFAULTHEADER(RemovePlayerItem, bool, (CBaseEntity *pItem));
	DECLARE_DEFAULTHEADER(Weapon_Switch, bool, (CBaseEntity /*CBaseCombatWeapon*/ *pWeapon, int viewmodelindex));
	DECLARE_DEFAULTHEADER(Weapon_Equip, void, (CBaseEntity *pWeapon));
	DECLARE_DEFAULTHEADER(Weapon_GetSlot, CBaseEntity *, (int slot));
	DECLARE_DEFAULTHEADER(EyePosition, Vector &, ());
	DECLARE_DEFAULTHEADER(EyeAngles, QAngle *, ());
	DECLARE_DEFAULTHEADER_DETOUR(HandleCommand_JoinClass, void, (const char *pClass, bool bAllowSpawn));

protected: // Sendprops
	DECLARE_SENDPROP(float, m_flNextAttack);
	DECLARE_SENDPROP(CBaseHandle, m_hActiveWeapon);
	DECLARE_SENDPROP(CBaseHandle, m_hMyWeapons);
	DECLARE_SENDPROP(CBaseHandle, m_hVehicle);
	DECLARE_SENDPROP(uint16_t, m_iHealth);
	DECLARE_SENDPROP(uint8_t, m_lifeState);
	DECLARE_SENDPROP(uint8_t, m_iClass);
	DECLARE_SENDPROP(uint8_t, m_iDesiredPlayerClass);
	DECLARE_SENDPROP(uint16_t, m_nPlayerCond);
	DECLARE_SENDPROP(bool, m_bJumping);
	DECLARE_SENDPROP(uint8_t, m_nNumHealers);
	DECLARE_SENDPROP(uint8_t, m_nPlayerState);
	DECLARE_SENDPROP(uint8_t, m_nDisguiseTeam);
	DECLARE_SENDPROP(uint8_t, m_nDisguiseClass);
	DECLARE_SENDPROP(uint8_t, m_iDisguiseTargetIndex);
	DECLARE_SENDPROP(uint16_t, m_iDisguiseHealth);
	DECLARE_SENDPROP(float, m_flMaxspeed);
	DECLARE_SENDPROP(uint16_t, m_iObserverMode);
	DECLARE_SENDPROP(CBaseHandle, m_hObserverTarget);
	DECLARE_SENDPROP(CBaseHandle, m_hRagdoll);

protected:
	DECLARE_DATAMAP(int, m_nButtons);
};

#define PLAYERCLASS_UNKNOWN 0
#define PLAYERCLASS_SCOUT 1
#define PLAYERCLASS_SNIPER 2
#define PLAYERCLASS_SOLDIER 3
#define PLAYERCLASS_DEMOMAN 4
#define PLAYERCLASS_MEDIC 5
#define PLAYERCLASS_HEAVY 6
#define PLAYERCLASS_PYRO 7
#define PLAYERCLASS_SPY 8
#define PLAYERCLASS_ENGINEER 9

#define WEAPONSLOT_PRIMARY 0
#define WEAPONSLOT_SECONDARY 1
#define WEAPONSLOT_MELEE 2
#define WEAPONSLOT_GRENADE 3
#define WEAPONSLOT_BUILDING 4
#define WEAPONSLOT_PDA 5
#define WEAPONSLOT_ITEM1 6
#define WEAPONSLOT_ITEM2 7

#define LOADOUTSLOT_PRIMARY 0
#define LOADOUTSLOT_SECONDARY 1
#define LOADOUTSLOT_MELEE 2
#define LOADOUTSLOT_GRENADE 3
#define LOADOUTSLOT_BUILDING 4
#define LOADOUTSLOT_PDA 5
#define LOADOUTSLOT_PDA2 6
#define LOADOUTSLOT_HEAD 7
#define LOADOUTSLOT_MISC 8
#define LOADOUTSLOT_ACTION 9

#define TEAM_UNASSIGNED 0
#define TEAM_SPEC 1
#define TEAM_RED 2
#define TEAM_BLUE 3

#define PLAYERCOND_SLOWED (1<<0)
#define PLAYERCOND_ZOOMED (1<<1)
#define PLAYERCOND_DISGUISING (1<<2)
#define PLAYERCOND_DISGUISED (1<<3)
#define PLAYERCOND_CLOAKED (1<<4)
#define PLAYERCOND_UBERCHARGED (1<<5)
#define PLAYERCOND_TELEPORTEDGLOW (1<<6)
#define PLAYERCOND_TAUNTING (1<<7)
#define PLAYERCOND_UBERCHARGEFADING (1<<8)
#define PLAYERCOND_CLOAKFLICKER (1<<9)
#define PLAYERCOND_TELEPORTING (1<<10)
#define PLAYERCOND_KRITZKRIEGED (1<<11)
// (1<<12)
#define PLAYERCOND_DEADRINGERED (1<<13)
#define PLAYERCOND_BONKED (1<<14)
#define PLAYERCOND_DAZED (1<<15)
#define PLAYERCOND_BUFFED (1<<16)
#define PLAYERCOND_CHARGING (1<<17)
#define PLAYERCOND_DEMOBUFF (1<<18)
#define PLAYERCOND_CRITCOLA (1<<19)
#define PLAYERCOND_HEALING (1<<20)
// (1<<21)
#define PLAYERCOND_ONFIRE (1<<22)
#define PLAYERCOND_OVERHEALED (1<<23)
#define PLAYERCOND_JARATED (1<<24)
#define PLAYERCOND_BLEEDING (1<<25)
#define PLAYERCOND_DEFENSEBUFFED (1<<26)
#define PLAYERCOND_MILKED  (1<<27)

#endif // _INCLUDE_CPLAYER_H_
