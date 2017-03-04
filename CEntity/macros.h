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

#ifndef _INCLUDE_MACROS_H_
#define _INCLUDE_MACROS_H_

#include "detours.h"

#undef DECLARE_CLASS
#define DECLARE_CLASS( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass; \
	virtual bool IsBase() { return false; } \
	virtual void InitDataMap() \
	{ \
		if (BaseClass::IsBase()) \
		{ \
			ThisClass::m_DataMap.baseMap = BaseClass::GetDataDescMap(); \
		} \
		datamap_t *pMap = gamehelpers->GetDataMap(BaseEntity()); \
		if (eventFuncs == NULL) \
		{ \
			if (pMap) \
			{ \
				typedescription_t *typedesc = gamehelpers->FindInDataMap(pMap, "m_OnUser1"); \
				if (typedesc != NULL) \
					eventFuncs = typedesc->pSaveRestoreOps; \
			} \
		} \
		if (eventFuncs == NULL) \
			smutils->LogError(myself, "[CENTITY] Could not lookup ISaveRestoreOps for Outputs"); \
		UTIL_PatchOutputRestoreOps(pMap); \
	}

#undef DECLARE_CLASS_NOBASE
#define DECLARE_CLASS_NOBASE( className ) \
	typedef className ThisClass; \
	virtual bool IsBase() { return true; } \
	virtual void InitDataMap() {};

class IHookTracker
{
public:
	IHookTracker()
	{
		m_Next = m_Head;
		m_Head = this;
	}
	virtual void ReconfigureHook(IGameConfig *pConfig) =0;
	virtual void AddHook(CEntity *pEnt) =0;
	virtual void ClearFlag(CEntity *pEnt) =0;
public:
	static IHookTracker *m_Head;
	IHookTracker *m_Next;
public:
	int m_bShouldHook;
};

#define DECLARE_HOOK(name, cl) \
class name##cl##HookTracker : public IHookTracker \
{ \
public: \
	void ReconfigureHook(IGameConfig *pConfig) \
	{ \
		int offset; \
		if (!pConfig->GetOffset(#name, &offset)) \
		{ \
			smutils->LogError(myself, "[CENTITY] Failed to retrieve offset %s from gamedata file", #name); \
			m_bShouldHook = false; \
		} else { \
			SH_MANUALHOOK_RECONFIGURE(name, offset, 0, 0); \
			m_bShouldHook = true; \
		} \
	} \
	void AddHook(CEntity *pEnt) \
	{ \
		cl *pThisType = dynamic_cast<cl *>(pEnt); \
		if (pThisType && m_bShouldHook) \
		{ \
			SH_ADD_MANUALVPHOOK(name, pEnt->BaseEntity(), SH_MEMBER(pThisType, &cl::Internal##name), false); \
			/* META_CONPRINTF("Hooked %s on %s.\n", #name, pEnt->GetClassname()); */ \
		} \
	} \
	void ClearFlag(CEntity *pEnt) \
	{ \
		cl *pThisType = dynamic_cast<cl *>(pEnt); \
		if (pThisType) \
		{ \
			pThisType->m_bIn##name = false; \
		} \
	} \
}; \
name##cl##HookTracker name##cl##HookTrackerObj;

class IDetourTracker
{
public:
	IDetourTracker()
	{
		m_Next = m_Head;
		m_Head = this;
	}

	virtual void AddHook() = 0;
	virtual void RemoveHook() = 0;

	static IDetourTracker *m_Head;
	IDetourTracker *m_Next;

public:
	CDetour *m_Detour; \
};

#define DECLARE_DETOUR(name, cl) \
class name##cl##DetourTracker : public IDetourTracker \
{ \
public: \
	void AddHook() \
	{ \
		void *callback = (void *)GetCodeAddress(&cl::Internal##name); \
		void **trampoline = (void **)(&cl::name##_Actual); \
		m_Detour = CDetourManager::CreateDetour(callback, trampoline, #name); \
		if (m_Detour) \
		{ \
			m_Detour->EnableDetour(); \
		} \
	} \
	void RemoveHook() \
	{ \
		if (m_Detour) \
		{ \
			m_Detour->Destroy(); \
			m_Detour = NULL; \
		} \
	} \
}; \
name##cl##DetourTracker name##cl##DetourTrackerObj;

class ISigOffsetTracker
{
public:
	ISigOffsetTracker()
	{
		m_Next = m_Head;
		m_Head = this;
	}

	virtual void FindSig(IGameConfig *pConfig) = 0;

	static ISigOffsetTracker *m_Head;
	ISigOffsetTracker *m_Next;

public:
	void *pPointer;
};

#define DECLARE_SIGOFFSET(name) \
class name##SigOffsetTracker : public ISigOffsetTracker \
{ \
public: \
	void FindSig(IGameConfig *pConfig) \
	{ \
		if (!pConfig->GetMemSig(#name, &pPointer)) \
		{ \
			g_pSM->LogError(myself, "[CENTITY] Failed to retrieve %s from gamedata file", #name); \
		} else if (!pPointer) { \
			g_pSM->LogError(myself, "[CENTITY] Failed to retrieve pointer from %s", #name); \
		} \
	} \
}; \
name##SigOffsetTracker name##SigOffsetTrackerObj;

#define GET_SIGOFFSET(name) name##SigOffsetTrackerObj.pPointer

#define PROP_SEND 0
#define PROP_DATA 1

#define DECLARE_SENDPROP(typeof, name) DECLARE_PROP(typeof, name, PROP_SEND)
#define DECLARE_DATAMAP(typeof, name) DECLARE_PROP(typeof, name, PROP_DATA)

#define DEFINE_PROP(name, cl)	cl::name##PropTracker cl::name##PropTrackerObj;

template <typename T>
class Redirect
{
public:
	Redirect& operator =(const T& input)
	{
		*ptr = input;
		return *this;
	}
	operator T& () const
	{
		return *ptr;
	}
	operator T* () const
	{
		return ptr;
	}
	T* operator->() 
	{
		return ptr;
	}
	T& operator [] (unsigned i)
	{
		return ptr[i];
	}

public:
	T* ptr;
};

class CFakeHandle : public CBaseHandle
{
public:
	CEntity* Get() const
	{
		return CEntityLookup::Instance(*this);
	}
	operator CEntity*()
	{
		return Get();
	}
	operator CEntity*() const
	{
		return Get();
	}
	CEntity* operator->() const
	{
		return Get();
	}
};

template<>
class Redirect <CFakeHandle>
{
public:
	Redirect& operator =(const CFakeHandle& input)
	{
		*ptr = input;
		return *this;
	}
	bool operator ==(const CEntity *lhs)
	{
		return (*ptr == lhs);
	}
	bool operator !=(const CEntity *lhs)
	{
		return (*ptr != lhs);
	}
	operator CFakeHandle& () const
	{
		return *ptr;
	}
	operator CFakeHandle* () const
	{
		return ptr;
	}
	operator CEntity* () const
	{
		return ptr->Get();
	}
	CEntity* operator->() 
	{
		return CEntityLookup::Instance(*ptr);
	}
	bool operator == (void *rhs )
	{
		return (ptr == rhs);
	}

public:
	CFakeHandle* ptr;
};



class IPropTracker
{
public:
	IPropTracker()
	{
		m_Next = m_Head;
		m_Head = this;
	}
	virtual void InitProp(CEntity *pEnt) =0;

	bool GetSendPropOffset(const char *classname, const char *name, unsigned int &offset)
	{
		sm_sendprop_info_t info;
		if (!gamehelpers->FindSendPropInfo(classname, name, &info))
		{
			return false;
		}

		offset = info.actual_offset;
		return true;
	}

	bool GetDataMapOffset(CBaseEntity *pEnt, const char *name, unsigned int &offset)
	{
		datamap_t *pMap = gamehelpers->GetDataMap(pEnt);
		if (!pMap)
		{
			return false;
		}
		
		typedescription_t *typedesc = gamehelpers->FindInDataMap(pMap, name);
		
		if (typedesc == NULL)
		{
			return false;
		}

		offset = typedesc->fieldOffset[TD_OFFSET_NORMAL];
		return true;
	}
public:
	static IPropTracker *m_Head;
	IPropTracker *m_Next;
};

#define DECLARE_PROP(typeof, name, search) \
Redirect<typeof> name; \
friend class name##PropTracker; \
class name##PropTracker : public IPropTracker \
{ \
public: \
	name##PropTracker() \
	{ \
		lookup = false; \
		found = false; \
	} \
	void InitProp(CEntity *pEnt) \
	{ \
		ThisClass *pThisType = dynamic_cast<ThisClass *>(pEnt); \
		if (pThisType) \
		{ \
			if (!lookup) \
			{ \
				if (search == PROP_SEND) \
				{ \
					found = GetSendPropOffset(pEnt->edict()->GetNetworkable()->GetServerClass()->GetName(), #name, offset); \
				} \
				else \
				{ \
					found = GetDataMapOffset(pEnt->BaseEntity(), #name, offset); \
				} \
				if (!found) \
				{ \
					g_pSM->LogError(myself,"[CENTITY] Failed lookup of prop %s on entity %s", #name, pEnt->GetClassname()); \
				} \
				lookup = true; \
			} \
			if (found) \
			{ \
				pThisType->name.ptr = (typeof *)(((uint8_t *)(pEnt->BaseEntity())) + offset); \
			} \
			else \
			{ \
				pThisType->name.ptr = NULL; \
			} \
		} \
	} \
	unsigned int GetOffset() { return offset; } \
private: \
	unsigned int offset; \
	bool lookup; \
	bool found; \
}; \
static name##PropTracker name##PropTrackerObj;

#define DECLARE_DEFAULTHANDLER(type, name, ret, params, paramscall) \
ret type::name params \
{ \
	if (!m_bIn##name) \
		return SH_MCALL(BaseEntity(), name) paramscall; \
	SET_META_RESULT(MRES_IGNORED); \
	SH_GLOB_SHPTR->DoRecall(); \
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr()); \
	RETURN_META_VALUE(MRES_SUPERCEDE, (thisptr->*(__SoureceHook_FHM_GetRecallMFP##name(thisptr))) paramscall); \
} \
ret type::Internal##name params \
{ \
	SET_META_RESULT(MRES_SUPERCEDE); \
	type *pEnt = (type *)CEntity::Instance(META_IFACEPTR(CBaseEntity)); \
	if (!pEnt) \
		RETURN_META_VALUE(MRES_IGNORED, (ret)0); \
	int index = pEnt->entindex(); \
	pEnt->m_bIn##name = true; \
	ret retvalue = pEnt->name paramscall; \
	pEnt = (type *)CEntity::Instance(index); \
	if (pEnt) \
		pEnt->m_bIn##name = false; \
	return retvalue; \
}

#define DECLARE_DEFAULTHANDLER_void(type, name, params, paramscall) \
void type::name params \
{ \
	if (!m_bIn##name) \
	{ \
		SH_MCALL(BaseEntity(), name) paramscall; \
		return; \
	} \
	SET_META_RESULT(MRES_IGNORED); \
	SH_GLOB_SHPTR->DoRecall(); \
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr()); \
	(thisptr->*(__SoureceHook_FHM_GetRecallMFP##name(thisptr))) paramscall; \
	SET_META_RESULT(MRES_SUPERCEDE); \
} \
void type::Internal##name params \
{ \
	SET_META_RESULT(MRES_SUPERCEDE); \
	type *pEnt = (type *)CEntity::Instance(META_IFACEPTR(CBaseEntity)); \
	if (!pEnt) \
		RETURN_META(MRES_IGNORED); \
	int index = pEnt->entindex(); \
	pEnt->m_bIn##name = true; \
	pEnt->name paramscall; \
	pEnt = (type *)CEntity::Instance(index); \
	if (pEnt) \
		pEnt->m_bIn##name = false; \
}

#define DECLARE_DEFAULTHANDLER_DETOUR_void(type, name, params, paramscall) \
void type::name params \
{ \
	(((type *)BaseEntity())->*name##_Actual) paramscall; \
} \
void type::Internal##name params \
{ \
	type *pEnt = (type *)CEntity::Instance((CBaseEntity *)this); \
	assert(pEnt); \
	pEnt->name paramscall; \
} \
void (type::* type::name##_Actual) params = NULL; \

#define DECLARE_DEFAULTHANDLER_DETOUR(type, name, ret, params, paramscall) \
ret type::name params \
{ \
	return (((type *)BaseEntity())->*name##_Actual) paramscall; \
} \
ret type::Internal##name params \
{ \
	type *pEnt = (type *)CEntity::Instance((CBaseEntity *)this); \
	assert(pEnt); \
	return pEnt->name paramscall; \
} \
ret (type::* type::name##_Actual) params = NULL;



#endif // _INCLUDE_MACROS_H_
