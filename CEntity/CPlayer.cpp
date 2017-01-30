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

#include "CHelpers.h"
#include "shareddefs.h"
#include "in_buttons.h"

SH_DECL_MANUALHOOK3(FVisible, 0, 0, 0, bool, CBaseEntity *, int, CBaseEntity **);
SH_DECL_MANUALHOOK1(FInViewCone, 0, 0, 0, bool, CBaseEntity *);
SH_DECL_MANUALHOOK1(FInAimCone, 0, 0, 0, bool, CBaseEntity *);
SH_DECL_MANUALHOOK2(IsLookingTowards, 0, 0, 0, bool, const CBaseEntity *, float);
SH_DECL_MANUALHOOK1(IsInFieldOfView, 0, 0, 0, bool, CBaseEntity *);
SH_DECL_MANUALHOOK5_void(ProcessUsercmds, 0, 0, 0, CUserCmd *, int, int, int, bool);
SH_DECL_MANUALHOOK0_void(Jump, 0, 0, 0);
SH_DECL_MANUALHOOK0_void(Duck, 0, 0, 0);
SH_DECL_MANUALHOOK2(Weapon_Switch, 0, 0, 0, bool, CBaseEntity *, int);
SH_DECL_MANUALHOOK1_void(Weapon_Equip, 0, 0, 0, CBaseEntity *);
SH_DECL_MANUALHOOK1(Weapon_GetSlot, 0, 0, 0, CBaseEntity *, int);
SH_DECL_MANUALHOOK0(EyeAngles, 0, 0, 0, QAngle *);

DECLARE_HOOK(FVisible, CPlayer);
DECLARE_HOOK(FInViewCone, CPlayer);
DECLARE_HOOK(FInAimCone, CPlayer);
DECLARE_HOOK(IsLookingTowards, CPlayer);
DECLARE_HOOK(IsInFieldOfView, CPlayer);
DECLARE_HOOK(ProcessUsercmds, CPlayer);
DECLARE_HOOK(Jump, CPlayer);
DECLARE_HOOK(Duck, CPlayer);
DECLARE_HOOK(Weapon_Switch, CPlayer);
DECLARE_HOOK(Weapon_Equip, CPlayer);
DECLARE_HOOK(Weapon_GetSlot, CPlayer);
DECLARE_HOOK(EyeAngles, CPlayer);

DECLARE_DETOUR(HandleCommand_JoinClass, CPlayer);

LINK_ENTITY_TO_INTERNAL_CLASS(CTFPlayer, CPlayer);

//Sendprops
DEFINE_PROP(m_flNextAttack, CPlayer);
DEFINE_PROP(m_hActiveWeapon, CPlayer);
DEFINE_PROP(m_hMyWeapons, CPlayer);
DEFINE_PROP(m_hVehicle, CPlayer);
DEFINE_PROP(m_iHealth, CPlayer);
DEFINE_PROP(m_lifeState, CPlayer);
DEFINE_PROP(m_iClass, CPlayer);
DEFINE_PROP(m_iDesiredPlayerClass, CPlayer);
DEFINE_PROP(m_nPlayerCond, CPlayer);
DEFINE_PROP(m_bJumping, CPlayer);
DEFINE_PROP(m_nPlayerState, CPlayer);
DEFINE_PROP(m_nDisguiseTeam, CPlayer);
DEFINE_PROP(m_nDisguiseClass, CPlayer);
DEFINE_PROP(m_iDisguiseTargetIndex, CPlayer);
DEFINE_PROP(m_iDisguiseHealth, CPlayer);
DEFINE_PROP(m_flMaxspeed, CPlayer);
DEFINE_PROP(m_iObserverMode, CPlayer);
DEFINE_PROP(m_hObserverTarget, CPlayer);
DEFINE_PROP(m_hRagdoll, CPlayer);

//Datamaps
DEFINE_PROP(m_nButtons, CPlayer);

DECLARE_DEFAULTHANDLER_DETOUR_void(CPlayer, HandleCommand_JoinClass, (const char *pClass, bool bAllowSpawn), (pClass, bAllowSpawn));

extern IServerGameClients *gameclients;

void CPlayer::Spawn()
{
	BaseClass::Spawn();
}

bool CPlayer::IsPlayer()
{
	return true;
}

bool CPlayer::IsAlive()
{
	return m_lifeState == LIFE_ALIVE;
}

void CPlayer::SetPlayerClass(int playerclass, bool persistant)
{
	*m_iClass = playerclass;
	if (persistant)
		*m_iDesiredPlayerClass = playerclass;
}

int CPlayer::GetPlayerClass()
{
	return *m_iClass;
}

int CPlayer::GetPlayerCond()
{
	return m_nPlayerCond;
}

bool CPlayer::IsDisguised()
{
	return (m_nPlayerCond & PLAYERCOND_DISGUISED) == PLAYERCOND_DISGUISED;
}

int CPlayer::GetDisguisedTeam()
{
	return m_nDisguiseTeam;
}

int CPlayer::GetButtons()
{
	return m_nButtons;
}

float CPlayer::GetMovementSpeed()
{
	return m_flMaxspeed;
}

void CPlayer::SetMovementSpeed(float speed)
{
	m_flMaxspeed = speed;
}

int CPlayer::GetHealth()
{
	return m_iHealth;
}

void CPlayer::SetHealth(int health)
{
	m_iHealth = health;
}

int CPlayer::GetObserverMode()
{
	return m_iObserverMode;
}

CEntity *CPlayer::GetObserverTarget()
{
	return Instance(m_hObserverTarget);
}

Vector CPlayer::EyePosition()
{
	Vector pos;
	gameclients->ClientEarPosition(this->m_pEdict, &pos);
	return (pos + (pos.x + 1.5f));
}

CEntity *CPlayer::GetAimTarget(bool playersOnly = false)
{
	QAngle eye_angles;

	Vector eye_position = EyePosition();

	QAngle *angles = EyeAngles();
	eye_angles.Init(angles->x, angles->y, angles->z);

	trcontents_t *tr;
	tr = pHelpers->TR_TraceRayFilter(eye_position, eye_angles, MASK_SHOT, RayType_Infinite, this);

	if (tr->entity != NULL)
	{
		CEntity *Ent = CEntity::Instance(tr->entity);
		int index = Ent->entindex();

		if (playersOnly && (index < 1 || index > MAX_PLAYERS))
			return NULL;
		else
			return Ent;
	}

	return NULL;
}

CEntity *CPlayer::GetRagdoll()
{
	return Instance(m_hRagdoll);
}

DECLARE_DEFAULTHANDLER_void(CPlayer, Weapon_Equip, (CBaseEntity *pWeapon), (pWeapon));

DECLARE_DEFAULTHANDLER(CPlayer, Weapon_GetSlot, CBaseEntity *, (int slot), (slot));

QAngle *CPlayer::EyeAngles()
{
	if (!m_bInEyeAngles)
	{
		QAngle *ret = SH_MCALL(BaseEntity(), EyeAngles)();
		return ret;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	RETURN_META_VALUE(MRES_SUPERCEDE, (thisptr->*(__SoureceHook_FHM_GetRecallMFPEyeAngles(thisptr)))());
}

QAngle *CPlayer::InternalEyeAngles()
{
	SET_META_RESULT(MRES_SUPERCEDE);
	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, &QAngle(0.0f, 0.0f, 0.0f));
	}

	int index = pEnt->entindex();
	pEnt->m_bInEyeAngles = true;
	QAngle *ret = pEnt->EyeAngles();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInEyeAngles = false;

	return ret;
}

bool CPlayer::FVisible(CEntity *pEntity, int traceMask, CEntity **ppBlocker)
{
	if (!m_bInFVisible)
	{
		CBaseEntity *pCopyBack;
		bool ret = SH_MCALL(BaseEntity(), FVisible)(*pEntity, traceMask, &pCopyBack);
		if (ppBlocker)
			*ppBlocker = CEntity::Instance(pCopyBack);

		return ret;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	CBaseEntity *pCopyBack = NULL;
	bool ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPFVisible(thisptr)))(*pEntity, traceMask, &pCopyBack);
	SET_META_RESULT(MRES_SUPERCEDE);

	if (ppBlocker && pCopyBack)
		*ppBlocker = CEntity::Instance(pCopyBack);

	return ret;
}

bool CPlayer::InternalFVisible(CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	// HACK: Fix this properly
	CEntity *pFirstParam = CEntity::Instance(pEntity);
	if (!pFirstParam)
	{
		//g_pSM->LogError(myself, "No matching CEntity found for *pEntity in CPlayer::InternalFVisible, aborting call.");
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	int index = pEnt->entindex();
	pEnt->m_bInFVisible = true;
	CEntity *pCopyBack;

	bool ret = pEnt->FVisible(/* *pEntity */ pFirstParam, traceMask, &pCopyBack);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInFVisible = false;

	if (ppBlocker)
		*ppBlocker = *pCopyBack;

	return ret;
}

bool CPlayer::FInViewCone(CEntity *pEntity)
{
	if (!m_bInFInViewCone)
	{
		bool ret = SH_MCALL(BaseEntity(), FInViewCone)(*pEntity);
		return ret;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	bool ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPFInViewCone(thisptr)))(*pEntity);
	SET_META_RESULT(MRES_SUPERCEDE);

	return ret;
}

bool CPlayer::InternalFInViewCone(CBaseEntity *pEntity)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	CEntity *pFirstParam = CEntity::Instance(pEntity);
	if (!pFirstParam)
	{
		//g_pSM->LogError(myself, "No matching CEntity found for *pEntity in CPlayer::InternalFVisible, aborting call.");
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	int index = pEnt->entindex();
	pEnt->m_bInFInViewCone = true;
	bool ret = pEnt->FInViewCone(pFirstParam);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInFInViewCone = false;

	return ret;
}

bool CPlayer::FInAimCone(CEntity *pEntity)
{
	if (!m_bInFInAimCone)
	{
		bool ret = SH_MCALL(BaseEntity(), FInAimCone)(*pEntity);
		return ret;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	bool ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPFInAimCone(thisptr)))(*pEntity);
	SET_META_RESULT(MRES_SUPERCEDE);

	return ret;
}

bool CPlayer::InternalFInAimCone(CBaseEntity *pEntity)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	CEntity *pFirstParam = CEntity::Instance(pEntity);
	if (!pFirstParam)
	{
		//g_pSM->LogError(myself, "No matching CEntity found for *pEntity in CPlayer::InternalFVisible, aborting call.");
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	int index = pEnt->entindex();
	pEnt->m_bInFInAimCone = true;
	bool ret = pEnt->FInAimCone(pFirstParam);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInFInAimCone = false;

	return ret;
}

bool CPlayer::IsLookingTowards(const CEntity *pEntity, float fTolerance)
{
	if (!m_bInIsLookingTowards)
	{
		bool ret = SH_MCALL(BaseEntity(), IsLookingTowards)(*(CEntity *)pEntity, fTolerance);
		return ret;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	bool ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPIsLookingTowards(thisptr)))(*(CEntity *)pEntity, fTolerance);
	SET_META_RESULT(MRES_SUPERCEDE);

	return ret;
}

bool CPlayer::InternalIsLookingTowards(const CBaseEntity *pEntity, float fTolerance)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	CEntity *pFirstParam = CEntity::Instance((CBaseEntity *)pEntity);
	if (!pFirstParam)
	{
		//g_pSM->LogError(myself, "No matching CEntity found for *pEntity in CPlayer::InternalFVisible, aborting call.");
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	int index = pEnt->entindex();
	pEnt->m_bInIsLookingTowards = true;
	bool ret = pEnt->FInViewCone(pFirstParam);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInIsLookingTowards = false;

	return ret;
}

bool CPlayer::IsInFieldOfView(CEntity *pEntity)
{
	if (!m_bInIsInFieldOfView)
	{
		bool ret = SH_MCALL(BaseEntity(), IsInFieldOfView)(*pEntity);
		return ret;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	bool ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPIsInFieldOfView(thisptr)))(*pEntity);
	SET_META_RESULT(MRES_SUPERCEDE);

	return ret;
}

bool CPlayer::InternalIsInFieldOfView(CBaseEntity *pEntity)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	CEntity *pFirstParam = CEntity::Instance(pEntity);
	if (!pFirstParam)
	{
		//g_pSM->LogError(myself, "No matching CEntity found for *pEntity in CPlayer::InternalFVisible, aborting call.");
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	int index = pEnt->entindex();
	pEnt->m_bInIsInFieldOfView = true;
	bool ret = pEnt->FInViewCone(pFirstParam);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInIsInFieldOfView = false;

	return ret;
}

void CPlayer::ProcessUsercmds(CUserCmd *cmds, int numcmds, int totalcmds, int dropped_packets, bool paused)
{
	if (!m_bInProcessUsercmds)
	{
		SH_MCALL(BaseEntity(), ProcessUsercmds)(cmds, numcmds, totalcmds, dropped_packets, paused);
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPProcessUsercmds(thisptr)))(cmds, numcmds, totalcmds, dropped_packets, paused);
	SET_META_RESULT(MRES_SUPERCEDE);

	return;
}

void CPlayer::InternalProcessUsercmds(CUserCmd *cmds, int numcmds, int totalcmds, int dropped_packets, bool paused)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInProcessUsercmds = true;
	pEnt->ProcessUsercmds(cmds, numcmds, totalcmds, dropped_packets, paused);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInProcessUsercmds = false;

	return;
}

void CPlayer::Jump()
{
	if (!m_bInJump)
	{
		SH_MCALL(BaseEntity(), Jump)();
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPJump(thisptr)))();
	SET_META_RESULT(MRES_SUPERCEDE);

	return;
}

void CPlayer::InternalJump()
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInJump = true;
	pEnt->Jump();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInJump = false;

	return;
}

void CPlayer::Duck()
{
	if (!m_bInJump)
	{
		SH_MCALL(BaseEntity(), Duck)();
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPDuck(thisptr)))();
	SET_META_RESULT(MRES_SUPERCEDE);

	return;
}

void CPlayer::InternalDuck()
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInJump = true;
	pEnt->Jump();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInJump = false;

	return;
}

bool CPlayer::Weapon_Switch(CBaseEntity *pWeapon, int viewmodelindex)
{
	if (!m_bInWeapon_Switch)
	{
		bool ret = SH_MCALL(BaseEntity(), Weapon_Switch)(pWeapon, viewmodelindex);
		return ret;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	bool ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPWeapon_Switch(thisptr)))(pWeapon, viewmodelindex);
	SET_META_RESULT(MRES_SUPERCEDE);

	return ret;
}

bool CPlayer::InternalWeapon_Switch(CBaseEntity *pWeapon, int viewmodelindex)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CPlayer *pEnt = dynamic_cast<CPlayer *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	int index = pEnt->entindex();
	pEnt->m_bInWeapon_Switch = true;
	bool ret = pEnt->Weapon_Switch(pWeapon, viewmodelindex);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInWeapon_Switch = false;

	return ret;
}
