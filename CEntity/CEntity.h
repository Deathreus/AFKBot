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

/**
* CEntity Entity Handling Framework version 2.0 by Matt "pRED*" Woodrow
*
* - Credits:
*		- This is largely (or entirely) based on a concept by voogru - http://voogru.com/
*		- The virtual function hooking is powered by the SourceHook library by Pavol "PM OnoTo" Marko.
*		- Contains code contributed by Brett "Brutal" Powell.
*		- Contains code contributed by Asher "asherkin" Baker.
*
* - About:
*		- CEntity is (and its derived classes are) designed to emulate the CBaseEntity class from the HL2 SDK.
*		- Valve code (like entire class definitions and CBaseEntity functions) from the SDK should almost just work when copied into this.
*			- References to CBaseEntity need to be changed to CEntity.
*			- Sendprops and datamaps are pointers to the actual values so references to these need to be dereferenced.
*				- Code that uses unexposed data members won't work - Though you could reimplement these manually.
*		- Virtual functions handle identically to ones in a real derived class.
*			- Calls from valve code to a virtual in CEntity (with no derived versions) fall back directly to the valve code.
*			- Calls from valve code to a virtual (with a derived version) will call that code, and the valve code can be optionally run using BaseClass::Function().
*
*			- Calls from your code to a virtual in CEntity (with no derived versions) will make a call to the valve code.
*			- Calls from your code to a virtual (with a derived version) will call that code, and that derived handler can run the valve code optionally using BaseClass::Function().
*
*
* - Notes:
*		- If you inherit Init() or Destroy() in a derived class, I would highly recommend calling the BaseClass equivalent.
*
* - TODO (in no particular order):
*		- Add handling of custom keyvalues commands
*			- Add datamapping to class values so keyvalues can parse to them
*		- Include more CEntity virtuals and props/datamaps
*		- Create more derived classes
*		- Include more Think/Touch etc handlers
*			- Valve code now has lists of thinks, can we access this?
*		- Forcibly deleting entities? - Implemented AcceptInput("Kill"...), UTIL_Remove sig scan would be cleaner.
*		- Support mods other than TF2 (CPlayer should only contain CBasePlayer sdk stuff and create optional CTFPlayer/CCSPlayer derives)
*
*	- Change log
*		- 1.0
*			- Initial import of basic CEntity and CPlayer
*		- 1.x
*			- Unlogged fixes/changes added after original version. TODO: Grab these from the hg changelog sometime.
*		- 2.0
*			- Improved LINK_ENTITY_TO_CLASS to use DLL Classnames. tf_projectile_rocket changed to CTFProjectile_Rocket for example.
*			- Added the ability to handle entity Inputs/Outputs.
*			- Cleaned up Macros used for almost everything.
*			- Added many new hooks and props for CEntity and CPlayer.
*			- Support for custom classnames with LINK_ENTITY_TO_CUSTOM_CLASS.
*			- Added support for detours, needs CDetours folder in the parent directory.
*			- Added a helpers class that makes common functions easy (from CrimsonGT).
*			- CEconItemView and CScriptCreatedAttribute (from TF2Items).
*			- A new 'tracker', designed to get a generic pointer from a sig in a gamedata file.
*			- A lot of CPlayer defines, including PLAYERCONDs, WEAPONSLOTs and LOADOUTSLOTs.
*			- Added CAnimating with StudioFrameAdvance and Dissolve.
*			- Changed CPlayer to inherit from CAnimating.
*/

#ifndef _INCLUDE_CENTITY_H_
#define _INCLUDE_CENTITY_H_

#define NO_STRING_T

#include "../extension.h"
#include "CEntityBase.h"
#include "IEntityFactory.h"
#include "vector.h"
#include "server_class.h"

#include "ehandle.h"
class CBaseEntity;
typedef CHandle<CBaseEntity> EHANDLE;
#include "vphysics_interface.h"
#include <typeinfo>
#include <variant_t.h>
#include "macros.h"
#include "shareddefs.h"
#include "util.h"

extern variant_t g_Variant;

class CEntity;

#undef DEFINE_INPUTFUNC
#define DEFINE_INPUTFUNC(fieldtype, inputname, inputfunc) { fieldtype, #inputfunc, { 0, 0 }, 1, FTYPEDESC_INPUT, inputname, NULL, (inputfunc_t)(&classNameTypedef::inputfunc) }

extern CEntity *pEntityData[MAX_EDICTS + 1];

typedef void (CEntity::*BASEPTR)(void);
typedef void (CEntity::*ENTITYFUNCPTR)(CEntity *pOther);
typedef void (CEntity::*USEPTR)(CEntity *pActivator, CEntity *pCaller, USE_TYPE useType, float value);

#define DEFINE_THINKFUNC(function) DEFINE_FUNCTION_RAW(function, BASEPTR)
#define DEFINE_ENTITYFUNC(function) DEFINE_FUNCTION_RAW(function, ENTITYFUNCPTR)
#define DEFINE_USEFUNC(function) DEFINE_FUNCTION_RAW(function, USEPTR)

struct inputdata_t
{
	CBaseEntity *pActivator;		// The entity that initially caused this chain of output events.
	CBaseEntity *pCaller;			// The entity that fired this particular output.
	variant_t value;				// The data parameter for this output.
	int nOutputID;					// The unique ID of the output that was fired.
};

//template< class T >
class CFakeHandle;

#define DECLARE_DEFAULTHEADER(name, ret, params) \
	ret Internal##name params; \
	bool m_bIn##name;

#define DECLARE_DEFAULTHEADER_DETOUR(name, ret, params) \
	ret Internal##name params; \
	static ret (ThisClass::* name##_Actual) params;

#define SetThink(a) ThinkSet(static_cast <void (CEntity::*)(void)> (a), 0, NULL)

class CEntity // : public CBaseEntity  - almost.
{
public: // CEntity
	DECLARE_CLASS_NOBASE(CEntity);
	DECLARE_DATADESC();
	DECLARE_DEFAULTHEADER(GetDataDescMap, datamap_t *, ());

	virtual void Init(edict_t *pEdict, CBaseEntity *pBaseEntity);
	void InitHooks();
	void InitProps();
	void ClearFlags();
	virtual void Destroy();
	CBaseEntity *BaseEntity();

	operator CBaseEntity* ()
	{
		if (this == NULL)
		{
			return NULL;
		}

		return BaseEntity();
	}
	CEntity *operator=(CBaseEntity *rhs)
	{
		return CEntityLookup::Instance(rhs);
	}
	CEntity *operator=(CBaseHandle &rhs)
	{
		return CEntityLookup::Instance(rhs);
	}
	CEntity *operator=(CBaseHandle rhs)
	{
		return CEntityLookup::Instance(rhs);
	}
	CEntity *operator=(Redirect<CBaseHandle> &rhs)
	{
		return CEntityLookup::Instance(rhs);
	}
	CBaseEntity *operator=(CEntity *const pEnt)
	{
		return pEnt->BaseEntity();
	}

	/* Bcompat and it's just easier to refer to these as CEntity:: */
	static CEntity *Instance(const CBaseHandle &hEnt) { return CEntityLookup::Instance(hEnt); }
	static CEntity *Instance(const edict_t *pEnt)  { return CEntityLookup::Instance(pEnt); }
	static CEntity *Instance(edict_t *pEnt)  { return CEntityLookup::Instance(pEnt); }
	static CEntity *Instance(int iEnt)  { return CEntityLookup::Instance(iEnt); }
	static CEntity *Instance(CBaseEntity *pEnt)  { return CEntityLookup::Instance(pEnt); }

public: // CBaseEntity virtuals
	virtual void Teleport(const Vector *origin, const QAngle* angles, const Vector *velocity);
	virtual void UpdateOnRemove();
	virtual void Spawn();
	virtual void Think();
	virtual void StartTouch(CEntity *pOther);
	virtual void Touch(CEntity *pOther);
	virtual void EndTouch(CEntity *pOther);
	virtual Vector GetSoundEmissionOrigin();
	virtual ServerClass *GetServerClass();

public: // CBaseEntity non virtual helpers
	BASEPTR	ThinkSet(BASEPTR func, float thinkTime, const char *szContext);
	void SetNextThink(float thinkTime, const char *szContext = NULL);
	void CheckHasThinkFunction(bool isThinking);
	bool WillThink();

	void AddEFlags(int nEFlagMask);
	void RemoveEFlags(int nEFlagMask);
	bool IsEFlagSet(int nEFlagMask) const;

	const char* GetClassname();
	void SetClassname(const char *pClassName);
	const char* GetTargetName();
	void SetTargetName(const char *pTargetName);
	CEntity *GetOwner();
	void SetOwner(CEntity *pOwnerEntity);

	int GetTeamNumber()  const;
	virtual void ChangeTeam(int iTeamNum);
	bool InSameTeam(CEntity *pEntity) const;

	int GetMoveType() const;
	void SetMoveType(int MoveType);
	int GetMoveCollide() const;
	void SetMoveCollide(int MoveCollide);

	const Vector &GetAbsOrigin() const;
	const Vector &GetLocalOrigin() const;
	const Vector &GetAbsVelocity() const;
	const Vector &GetVelocity() const;

	CEntity *GetMoveParent();

	edict_t *edict();
	int entindex();

	virtual	bool IsPlayer();
	int GetTeam();

public: // All the internal hook implementations for the above virtuals
	DECLARE_DEFAULTHEADER(Teleport, void, (const Vector *origin, const QAngle* angles, const Vector *velocity));
	DECLARE_DEFAULTHEADER(UpdateOnRemove, void, ());
	DECLARE_DEFAULTHEADER(Spawn, void, ());
	DECLARE_DEFAULTHEADER(Think, void, ());
	DECLARE_DEFAULTHEADER(StartTouch, void, (CBaseEntity *pOther));
	DECLARE_DEFAULTHEADER(Touch, void, (CBaseEntity *pOther));
	DECLARE_DEFAULTHEADER(EndTouch, void, (CBaseEntity *pOther));
	DECLARE_DEFAULTHEADER(GetSoundEmissionOrigin, Vector, ());
	DECLARE_DEFAULTHEADER(GetServerClass, ServerClass *, ());

protected: // CEntity
	CBaseEntity *m_pEntity;
	edict_t *m_pEdict;

protected: //Sendprops
	DECLARE_SENDPROP(uint8_t, m_iTeamNum);
	DECLARE_SENDPROP(Vector, m_vecOrigin);
	DECLARE_SENDPROP(uint8_t, m_CollisionGroup);
	DECLARE_SENDPROP(CFakeHandle, m_hOwnerEntity);
	DECLARE_SENDPROP(uint16_t, m_fEffects);

protected: //Datamaps
	DECLARE_DATAMAP(Vector, m_vecAbsOrigin);
	DECLARE_DATAMAP(Vector, m_vecAbsVelocity);
	DECLARE_DATAMAP(string_t, m_iClassname);
	DECLARE_DATAMAP(matrix3x4_t, m_rgflCoordinateFrame);
	DECLARE_DATAMAP(Vector, m_vecVelocity);
	DECLARE_DATAMAP(Vector, m_vecAngVelocity);
	DECLARE_DATAMAP(Vector, m_vecBaseVelocity);
	DECLARE_DATAMAP(CFakeHandle, m_hMoveParent);
	DECLARE_DATAMAP(int, m_iEFlags);
	DECLARE_DATAMAP(IPhysicsObject *, m_pPhysicsObject);
	DECLARE_DATAMAP(int, m_nNextThinkTick);
	DECLARE_DATAMAP(CFakeHandle, m_pParent);
	DECLARE_DATAMAP(int, m_MoveType);
	DECLARE_DATAMAP(int, m_MoveCollide);
	DECLARE_DATAMAP(string_t, m_iName);

	/* Thinking Stuff */
	void (CEntity::*m_pfnThink)(void);
	void (CEntity::*m_pfnTouch)(CEntity *pOther);
};

/**
* Fake definition of CBaseEntity, as long as we don't declare any data member we should be fine with this.
* Also gives us access to IServerEntity and below without explicit casting.
*/
class CBaseEntity : public IServerEntity
{
public:
	CBaseEntity* operator= (CEntity* const input)
	{
		return input->BaseEntity();
	}
	operator CEntity* ()
	{
		return CEntityLookup::Instance(this);
	}
};

#endif // _INCLUDE_CENTITY_H_
