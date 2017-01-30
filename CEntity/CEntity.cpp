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

#include "CEntity.h"
#include "shareddefs.h"
#include "CEntityManager.h"
#include "CPlayer.h"


IHookTracker *IHookTracker::m_Head = NULL;
IPropTracker *IPropTracker::m_Head = NULL;
IDetourTracker *IDetourTracker::m_Head = NULL;
ISigOffsetTracker *ISigOffsetTracker::m_Head = NULL;

ISaveRestoreOps *eventFuncs = NULL;

SH_DECL_MANUALHOOK3_void(Teleport, 0, 0, 0, const Vector *, const QAngle *, const Vector *);
SH_DECL_MANUALHOOK0_void(UpdateOnRemove, 0, 0, 0);
SH_DECL_MANUALHOOK0_void(Spawn, 0, 0, 0);
SH_DECL_MANUALHOOK0_void(Think, 0, 0, 0);
SH_DECL_MANUALHOOK0(GetDataDescMap, 0, 0, 0, datamap_t *);
SH_DECL_MANUALHOOK1_void(StartTouch, 0, 0, 0, CBaseEntity *);
SH_DECL_MANUALHOOK1_void(Touch, 0, 0, 0, CBaseEntity *);
SH_DECL_MANUALHOOK1_void(EndTouch, 0, 0, 0, CBaseEntity *);
SH_DECL_MANUALHOOK0(GetSoundEmissionOrigin, 0, 0, 0, Vector);
SH_DECL_MANUALHOOK0(GetServerClass, 0, 0, 0, ServerClass *);

DECLARE_HOOK(Teleport, CEntity);
DECLARE_HOOK(UpdateOnRemove, CEntity);
DECLARE_HOOK(Spawn, CEntity);
DECLARE_HOOK(Think, CEntity);
DECLARE_HOOK(GetDataDescMap, CEntity);
DECLARE_HOOK(StartTouch, CEntity);
DECLARE_HOOK(Touch, CEntity);
DECLARE_HOOK(EndTouch, CEntity);
DECLARE_HOOK(GetSoundEmissionOrigin, CEntity);
DECLARE_HOOK(GetServerClass, CEntity);

//Sendprops
DEFINE_PROP(m_iTeamNum, CEntity);
DEFINE_PROP(m_vecOrigin, CEntity);
DEFINE_PROP(m_CollisionGroup, CEntity);
DEFINE_PROP(m_hOwnerEntity, CEntity);
DEFINE_PROP(m_fEffects, CEntity);
DEFINE_PROP(m_vecVelocity, CEntity);

//Datamaps
DEFINE_PROP(m_vecAbsOrigin, CEntity);
DEFINE_PROP(m_vecAbsVelocity, CEntity);
DEFINE_PROP(m_nNextThinkTick, CEntity);
DEFINE_PROP(m_iClassname, CEntity);
DEFINE_PROP(m_rgflCoordinateFrame, CEntity);
DEFINE_PROP(m_vecAngVelocity, CEntity);
DEFINE_PROP(m_vecBaseVelocity, CEntity);
DEFINE_PROP(m_hMoveParent, CEntity);
DEFINE_PROP(m_iEFlags, CEntity);
DEFINE_PROP(m_pPhysicsObject, CEntity);
DEFINE_PROP(m_pParent, CEntity);
DEFINE_PROP(m_MoveType, CEntity);
DEFINE_PROP(m_MoveCollide, CEntity);
DEFINE_PROP(m_iName, CEntity);

/* Hacked Datamap declaration to fallback to the corresponding real entities one */
datamap_t CEntity::m_DataMap = { 0, 0, "CEntity", NULL };
datamap_t *CEntity::GetBaseMap() { return NULL; }
BEGIN_DATADESC_GUTS(CEntity)
END_DATADESC()

PhysIsInCallbackFuncType PhysIsInCallback;

datamap_t* CEntity::GetDataDescMap()
{
	if (!m_bInGetDataDescMap)
		return SH_MCALL(BaseEntity(), GetDataDescMap)();

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	RETURN_META_VALUE(MRES_SUPERCEDE, (thisptr->*(__SoureceHook_FHM_GetRecallMFPGetDataDescMap(thisptr)))());
}

datamap_t* CEntity::InternalGetDataDescMap()
{
	SET_META_RESULT(MRES_SUPERCEDE);
	CEntity *pEnt = (CEntity *)CEntity::Instance(META_IFACEPTR(CBaseEntity));
	if (!pEnt)
		RETURN_META_VALUE(MRES_IGNORED, (datamap_t *)0);
	int index = pEnt->entindex();
	pEnt->m_bInGetDataDescMap = true;
	datamap_t* retvalue = pEnt->GetDataDescMap();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInGetDataDescMap = false;
	return retvalue;
}

LINK_ENTITY_TO_INTERNAL_CLASS(CBaseEntity, CEntity);

variant_t g_Variant;

void CEntity::Init(edict_t *pEdict, CBaseEntity *pBaseEntity)
{
	m_pEntity = pBaseEntity;
	m_pEdict = pEdict;

	assert(!pEntityData[entindex()]);

	pEntityData[entindex()] = this;

	if (!m_pEntity || !m_pEdict)
		return;

	m_pfnThink = NULL;
	m_pfnTouch = NULL;
}

void CEntity::Destroy()
{
	pEntityData[entindex()] = NULL;
	delete this;
}

CBaseEntity *CEntity::BaseEntity()
{
	return m_pEntity;
}

/* Expanded handler for readability and since this one actually does something */
void CEntity::UpdateOnRemove()
{
	if (!m_bInUpdateOnRemove)
	{
		SH_MCALL(BaseEntity(), UpdateOnRemove);
		return;
	}

	SET_META_RESULT(MRES_IGNORED);

	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPUpdateOnRemove(thisptr)))();

	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalUpdateOnRemove()
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = CEntity::Instance(META_IFACEPTR(CBaseEntity));
	if (!pEnt)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInUpdateOnRemove = true;
	pEnt->UpdateOnRemove();
	if (pEnt == CEntity::Instance(index))
	{
		pEnt->m_bInUpdateOnRemove = false;
		pEnt->Destroy();
	}
}

DECLARE_DEFAULTHANDLER_void(CEntity, Teleport, (const Vector *origin, const QAngle* angles, const Vector *velocity), (origin, angles, velocity));
DECLARE_DEFAULTHANDLER_void(CEntity, Spawn, (), ());
DECLARE_DEFAULTHANDLER(CEntity, GetServerClass, ServerClass *, (), ());

void CEntity::StartTouch(CEntity *pOther)
{
	if (!m_bInStartTouch)
	{
		SH_MCALL(BaseEntity(), StartTouch)(*pOther);
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPStartTouch(thisptr)))(*pOther);
	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalStartTouch(CBaseEntity *pOther)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = CEntity::Instance(META_IFACEPTR(CBaseEntity));
	CEntity *pEntOther = CEntity::Instance(pOther);
	if (!pEnt || !pEntOther)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInStartTouch = true;
	pEnt->StartTouch(pEntOther);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInStartTouch = false;
}

void CEntity::EndTouch(CEntity *pOther)
{
	if (!m_bInEndTouch)
	{
		SH_MCALL(BaseEntity(), EndTouch)(*pOther);
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPEndTouch(thisptr)))(*pOther);
	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalEndTouch(CBaseEntity *pOther)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = CEntity::Instance(META_IFACEPTR(CBaseEntity));
	CEntity *pEntOther = CEntity::Instance(pOther);
	if (!pEnt || !pEntOther)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInEndTouch = true;
	pEnt->EndTouch(pEntOther);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInEndTouch = false;
}

void CEntity::Touch(CEntity *pOther)
{
	if (m_pfnTouch)
		(this->*m_pfnTouch)(pOther);

	if (!m_bInTouch)
	{
		SH_MCALL(BaseEntity(), Touch)(*pOther);
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPTouch(thisptr)))(*pOther);
	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalTouch(CBaseEntity *pOther)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = CEntity::Instance(META_IFACEPTR(CBaseEntity));
	CEntity *pEntOther = CEntity::Instance(pOther);
	if (!pEnt || !pEntOther)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInTouch = true;
	pEnt->Touch(pEntOther);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInTouch = false;
}

Vector CEntity::GetSoundEmissionOrigin()
{
	if (!m_bInGetSoundEmissionOrigin)
	{
		Vector ret = SH_MCALL(BaseEntity(), GetSoundEmissionOrigin)();
		return ret;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	Vector ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPGetSoundEmissionOrigin(thisptr)))();
	SET_META_RESULT(MRES_SUPERCEDE);

	return ret;
}

Vector CEntity::InternalGetSoundEmissionOrigin()
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = CEntity::Instance(META_IFACEPTR(CBaseEntity));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, NULL);
	}

	int index = pEnt->entindex();
	pEnt->m_bInGetSoundEmissionOrigin = true;
	Vector ret = pEnt->GetSoundEmissionOrigin();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInGetSoundEmissionOrigin = false;

	return ret;
}

void CEntity::Think()
{
	if (m_pfnThink)
	{
		(this->*m_pfnThink)();
	}

	if (!m_bInThink)
	{
		SH_MCALL(BaseEntity(), Think)();
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPThink(thisptr)))();
	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalThink()
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = CEntity::Instance(META_IFACEPTR(CBaseEntity));
	if (!pEnt)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInThink = true;
	pEnt->Think();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInThink = false;
}


BASEPTR	CEntity::ThinkSet(BASEPTR func, float thinkTime, const char *szContext)
{
	if (!szContext)
	{
		m_pfnThink = func;
		return m_pfnThink;
	}

	return NULL;
}

void CEntity::SetNextThink(float thinkTime, const char *szContext)
{
	int thinkTick = (thinkTime == TICK_NEVER_THINK) ? TICK_NEVER_THINK : TIME_TO_TICKS(thinkTime);

	// Are we currently in a think function with a context?
	if (!szContext)
	{
		// Old system
		m_nNextThinkTick = thinkTick;
		CheckHasThinkFunction(thinkTick == TICK_NEVER_THINK ? false : true);
		return;
	}
}

void CEntity::AddEFlags(int nEFlagMask)
{
	m_iEFlags |= nEFlagMask;
}

void CEntity::RemoveEFlags(int nEFlagMask)
{
	m_iEFlags &= ~nEFlagMask;
}

bool CEntity::IsEFlagSet(int nEFlagMask) const
{
	return (m_iEFlags & nEFlagMask) != 0;
}

void CEntity::CheckHasThinkFunction(bool isThinking)
{
	if (IsEFlagSet(EFL_NO_THINK_FUNCTION) && isThinking)
	{
		RemoveEFlags(EFL_NO_THINK_FUNCTION);
	}
	else if (!isThinking && !IsEFlagSet(EFL_NO_THINK_FUNCTION) && !WillThink())
	{
		AddEFlags(EFL_NO_THINK_FUNCTION);
	}
}

bool CEntity::WillThink()
{
	if (m_nNextThinkTick > 0)
		return true;

	return false;
}

const char* CEntity::GetClassname()
{
	return STRING(m_iClassname);
}

void CEntity::SetClassname(const char *pClassName)
{
	m_iClassname = MAKE_STRING(pClassName);
}

const char* CEntity::GetTargetName()
{
	return STRING(m_iName);
}

void CEntity::SetTargetName(const char *pTargetName)
{
	m_iName = MAKE_STRING(pTargetName);
}

void CEntity::ChangeTeam(int iTeamNum)
{
	m_iTeamNum = iTeamNum;
	edict()->StateChanged(m_iTeamNumPropTrackerObj.GetOffset());
}

int CEntity::GetTeamNumber(void) const
{
	return m_iTeamNum;
}

bool CEntity::InSameTeam(CEntity *pEntity) const
{
	if (!pEntity)
		return false;

	return (pEntity->GetTeamNumber() == GetTeamNumber());
}

const Vector &CEntity::GetLocalOrigin(void) const
{
	return m_vecOrigin;
}

const Vector &CEntity::GetAbsOrigin(void) const
{
	return m_vecAbsOrigin;
}

const Vector &CEntity::GetAbsVelocity() const
{
	if (IsEFlagSet(EFL_DIRTY_ABSVELOCITY))
	{
		//const_cast<CEntity*>(this)->CalcAbsoluteVelocity();
	}
	return m_vecAbsVelocity;
}

const Vector &CEntity::GetVelocity() const
{
	return m_vecVelocity;
}

CEntity *CEntity::GetMoveParent(void)
{
	return Instance(m_hMoveParent);
}

edict_t *CEntity::edict()
{
	return m_pEdict;
}

int CEntity::entindex()
{
	return BaseEntity()->GetRefEHandle().GetEntryIndex();
}

bool CEntity::IsPlayer()
{
	return false;
}

int CEntity::GetTeam()
{
	return m_iTeamNum;
}

void CEntity::InitHooks()
{
	IHookTracker *pTracker = IHookTracker::m_Head;
	while (pTracker)
	{
		pTracker->AddHook(this);
		pTracker = pTracker->m_Next;
	}
}

void CEntity::InitProps()
{
	IPropTracker *pTracker = IPropTracker::m_Head;
	while (pTracker)
	{
		pTracker->InitProp(this);
		pTracker = pTracker->m_Next;
	}
}

void CEntity::ClearFlags()
{
	IHookTracker *pTracker = IHookTracker::m_Head;
	while (pTracker)
	{
		pTracker->ClearFlag(this);
		pTracker = pTracker->m_Next;
	}
}

CEntity *CEntity::GetOwner()
{
	return m_hOwnerEntity;
}

void CEntity::SetOwner(CEntity *pOwnerEntity)
{
	(*m_hOwnerEntity.ptr).Set(pOwnerEntity->edict()->GetIServerEntity());
}

int CEntity::GetMoveType() const
{
	return *m_MoveType;
}

void CEntity::SetMoveType(int MoveType)
{
	*m_MoveType = MoveType;
}

int CEntity::GetMoveCollide() const
{
	return *m_MoveCollide;
}

void CEntity::SetMoveCollide(int MoveCollide)
{
	*m_MoveCollide = MoveCollide;
}
