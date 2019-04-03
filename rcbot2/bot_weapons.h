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
#ifndef __BOT_WEAPONS_H__
#define __BOT_WEAPONS_H__

class CBot;

extern int m_TF2AmmoIndices[];

typedef struct
{
	int iSlot;
	int iId;
	const char *szWeaponName;
	int m_iFlags;
	float minPrimDist;
	float maxPrimDist;
	int m_iAmmoIndex;
	int m_iPreference;
	float m_fProjSpeed;
}WeaponsData_t;

enum
{
	TF2_WEAPON_BAT = 0,
	TF2_WEAPON_BOTTLE,
	TF2_WEAPON_FIREAXE,
	TF2_WEAPON_CLUB,
	TF2_WEAPON_KNIFE,
	TF2_WEAPON_FISTS,
	TF2_WEAPON_SHOVEL,
	TF2_WEAPON_WRENCH,
	TF2_WEAPON_BONESAW,
	TF2_WEAPON_SHOTGUN_PRIMARY,
	TF2_WEAPON_SHOTGUN_SOLDIER,
	TF2_WEAPON_SHOTGUN_HWG,
	TF2_WEAPON_SHOTGUN_PYRO,
	TF2_WEAPON_SCATTERGUN,
	TF2_WEAPON_SNIPERRIFLE,
	TF2_WEAPON_MINIGUN,
	TF2_WEAPON_SMG,
	TF2_WEAPON_SYRINGEGUN,
	TF2_WEAPON_ROCKETLAUNCHER,
	TF2_WEAPON_GRENADELAUNCHER,
	TF2_WEAPON_PIPEBOMBS,
	TF2_WEAPON_FLAMETHROWER,
	TF2_WEAPON_PISTOL,
	TF2_WEAPON_PISTOL_SCOUT,
	TF2_WEAPON_REVOLVER,
	TF2_WEAPON_POMSON6000,
	TF2_WEAPON_FRONTIERJUSTICE,
	TF2_WEAPON_BUILDER,
	TF2_WEAPON_MEDIGUN,
	TF2_WEAPON_INVIS,
	TF2_WEAPON_FLAREGUN,
	TF2_WEAPON_BUFF_ITEM,
	TF2_WEAPON_SAXXY,
	TF2_WEAPON_SENTRYGUN,
	TF2_WEAPON_BAT_WOOD,
	TF2_WEAPON_LUNCHBOX_DRINK,
	TF2_WEAPON_BOW,
	TF2_WEAPON_JAR,
	TF2_WEAPON_DIRECTHIT,
	TF2_WEAPON_SWORD,
	TF2_WEAPON_BAT_FISH,
	TF2_WEAPON_KATANA,
	TF2_WEAPON_COWMANGLER,
	TF2_WEAPON_CROSSBOW,
	TF2_WEAPON_CLEAVER,
	TF2_WEAPON_BAT_GIFTWRAP,
	TF2_WEAPON_RAYGUN,
	TF2_WEAPON_MAX
};

enum
{
	HL2DM_WEAPON_PISTOL = 0,
	HL2DM_WEAPON_CROWBAR,
	HL2DM_WEAPON_357,
	HL2DM_WEAPON_SMG1,
	HL2DM_WEAPON_AR2,
	HL2DM_WEAPON_FRAG,
	HL2DM_WEAPON_STUNSTICK,
	HL2DM_WEAPON_CROSSBOW,
	HL2DM_WEAPON_RPG,
	HL2DM_WEAPON_SLAM,
	HL2DM_WEAPON_SHOTGUN,
	HL2DM_WEAPON_PHYSCANNON,
	HL2DM_WEAPON_MAX
};

enum
{
	DOD_WEAPON_AMERKNIFE = 0,
	DOD_WEAPON_SPADE,
	DOD_WEAPON_COLT,
	DOD_WEAPON_P38,
	DOD_WEAPON_M1,
	DOD_WEAPON_C96,
	DOD_WEAPON_GARAND,
	DOD_WEAPON_K98,
	DOD_WEAPON_THOMPSON,
	DOD_WEAPON_MP40,
	DOD_WEAPON_BAR,
	DOD_WEAPON_MP44,
	DOD_WEAPON_SPRING,
	DOD_WEAPON_K98_SCOPED,
	DOD_WEAPON_20CAL,
	DOD_WEAPON_MG42,
	DOD_WEAPON_BAZOOKA,
	DOD_WEAPON_PSCHRECK,
	DOD_WEAPON_RIFLEGREN_US,
	DOD_WEAPON_RIFLEGREN_GER,
	DOD_WEAPON_FRAG_US,
	DOD_WEAPON_FRAG_GER,
	DOD_WEAPON_SMOKE_US,
	DOD_WEAPON_SMOKE_GER,
	DOD_WEAPON_BOMB,
	DOD_WEAPON_MAX
};


#define WEAP_FL_NONE			0
#define WEAP_FL_PRIM_ATTACK		1
#define WEAP_FL_SEC_ATTACK		2
#define WEAP_FL_EXPLOSIVE		4 // weapon is an explosive weapon eg. rpg
#define WEAP_FL_MELEE			8 //
#define WEAP_FL_UNDERWATER		16 // weapon can be used under water
#define WEAP_FL_HOLDATTACK		32 // weapon must hold attack (e.g. minigun)
#define WEAP_FL_SPECIAL			64 //
#define WEAP_FL_KILLPIPEBOMBS	128 // weapon can destroy pipe bombs (tf2)
#define WEAP_FL_DEFLECTROCKETS	256 // weapon can deflect rocekts (tf2)
#define WEAP_FL_GRAVGUN			512 // weapon is a grav gun
#define WEAP_FL_EXPLOSIVE_SEC	1024 // weapon has an explosive secondary attack
#define WEAP_FL_ZOOMABLE		2048 // weapon can be zoomed
#define WEAP_FL_DEPLOYABLE		4096 // weapon can be deployed
#define WEAP_FL_MELEE_SEC_ATT	8192 // weapon has a melee secondary attack
#define WEAP_FL_FIRE_SELECT		16384 // weapon can choose fire mode
#define WEAP_FL_CANTFIRE_NORM	32768 // weapon can't be fired normally, needs to be zoomed/deployed
#define WEAP_FL_GRENADE			65536
#define WEAP_FL_HIGH_RECOIL		131072 // can't be fired at long distance, but ok when deployed
#define WEAP_FL_SCOPE			262144 // has a scope . i.e. sniper rifle
#define WEAP_FL_PROJECTILE		524288 // affected by gravity

extern WeaponsData_t TF2Weaps[];
extern WeaponsData_t HL2DMWeaps[];
extern WeaponsData_t DODWeaps[];

class CWeapon
{
public:
	CWeapon(WeaponsData_t *data)
	{
		m_iSlot = data->iSlot;
		SetID(data->iId);
		SetName(data->szWeaponName);

		SetFlags(data->m_iFlags);

		// shoot distance (default)
		m_fPrimMinWeaponShootDist = data->minPrimDist;
		m_fPrimMaxWeaponShootDist = data->maxPrimDist;

		m_fSecMinWeaponShootDist = 0.0f;
		m_fSecMaxWeaponShootDist = 512.0f;

		m_fProjectileSpeed = data->m_fProjSpeed;

		m_iAmmoIndex1 = data->m_iAmmoIndex;
		m_iAmmoIndex2 = -1;

		m_iPreference = data->m_iPreference;

#if defined _DEBUG
		char fmt[24];
		ke::SafeSprintf(fmt, sizeof(fmt), "Projectile Speed: %.3f", m_fProjectileSpeed);
		AFKBot::DebugMessage("Parsed data for '%s'\nSlot: %i\nID: %i\nEngage Distance: %.2f - %.2f\nPreference: %i\n%s", 
			m_szWeaponName,
			m_iSlot,
			m_iWeaponId,
			m_fPrimMinWeaponShootDist,
			m_fPrimMaxWeaponShootDist,
			m_iPreference,
			(m_iFlags & WEAP_FL_PROJECTILE) ? fmt : "");
#endif
	}

	/*CWeapon( int iSlot, const char *szWeaponName, int iId, int iFlags = 0, int iAmmoIndex = -1, float minPrim =0.0f, float maxPrim = 4096.0f, int iPref = 0, int iAmmoIndex2 = -1 )
	{
		m_iSlot = iSlot;
		setID(iId);
		setName(szWeaponName);

		setFlags(iFlags);

		// shoot distance (default)
		m_fPrimMinWeaponShootDist = minPrim;
		m_fPrimMaxWeaponShootDist = maxPrim;

		m_fSecMinWeaponShootDist = 0.0f;
		m_fSecMaxWeaponShootDist = 512.0f;
		m_iAmmoIndex1 = iAmmoIndex;
		m_iAmmoIndex2 = iAmmoIndex2;

		m_iPreference = iPref;
	}*/

	inline void SetName(const char *szWeaponName)
	{
		m_szWeaponName = szWeaponName;
	}

	inline bool IsWeaponName(const char *szWeaponName)
	{
		return !strcmp(szWeaponName, GetWeaponName());
	}

	inline bool IsShortWeaponName(const char *szWeaponName)
	{
		static int start;

		start = strlen(m_szWeaponName) - strlen(szWeaponName);

		if (start < 0)
			return false;

		return !strcmp(&m_szWeaponName[start], szWeaponName);
	}

	inline bool CanDestroyPipeBombs()
	{
		return HasAllFlags(WEAP_FL_KILLPIPEBOMBS);
	}

	inline bool IsScoped()
	{
		return HasAllFlags(WEAP_FL_SCOPE);
	}

	inline bool CanDeflectRockets()
	{
		return HasAllFlags(WEAP_FL_DEFLECTROCKETS);
	}

	inline void SetID(const int iId)
	{
		m_iWeaponId = iId;
	}

	inline void SetFlags(const int iFlags)
	{
		m_iFlags = iFlags;
	}

	inline bool PrimaryInRange(float fDistance)
	{
		return (fDistance > m_fPrimMinWeaponShootDist) && (fDistance < m_fPrimMaxWeaponShootDist);
	}

	inline bool PrimaryGreaterThanRange(float fDistance)
	{
		return (fDistance < m_fPrimMaxWeaponShootDist);
	}

	inline float PrimaryMaxRange()
	{
		return (m_fPrimMaxWeaponShootDist);
	}

	inline bool HasHighRecoil()
	{
		return HasAllFlags(WEAP_FL_HIGH_RECOIL);
	}

	inline bool IsZoomable()
	{
		return HasAllFlags(WEAP_FL_ZOOMABLE);
	}

	inline bool IsProjectile()
	{
		return HasAllFlags(WEAP_FL_PROJECTILE);
	}

	inline bool IsExplosive()
	{
		return HasAllFlags(WEAP_FL_EXPLOSIVE);
	}

	inline bool IsDeployable()
	{
		return HasAllFlags(WEAP_FL_DEPLOYABLE);
	}

	inline bool CanUseUnderWater()
	{
		return HasAllFlags(WEAP_FL_UNDERWATER);
	}

	inline bool IsGravGun()
	{
		return HasAllFlags(WEAP_FL_GRAVGUN);
	}

	inline bool MustHoldAttack()
	{
		return HasAllFlags(WEAP_FL_HOLDATTACK);
	}

	inline bool IsGrenade()
	{
		return HasAllFlags(WEAP_FL_GRENADE);
	}

	inline bool IsMelee()
	{
		return HasAllFlags(WEAP_FL_MELEE);
	}

	inline bool IsMeleeSecondary()
	{
		return HasAllFlags(WEAP_FL_MELEE_SEC_ATT);
	}

	inline bool NeedsDeployedOrZoomed()
	{
		return HasAllFlags(WEAP_FL_CANTFIRE_NORM);
	}

	inline bool CanAttack()
	{
		return HasAllFlags(WEAP_FL_PRIM_ATTACK);
	}

	inline bool IsSpecial()
	{
		return HasAllFlags(WEAP_FL_SPECIAL);
	}

	inline bool SecondaryInRange(float fDistance)
	{
		return (fDistance > m_fSecMinWeaponShootDist) && (fDistance < m_fSecMaxWeaponShootDist);
	}

	inline int GetPreference()
	{
		return m_iPreference;
	}

	inline const char *GetWeaponName() const
	{
		return m_szWeaponName;
	}

	inline const int GetID() const
	{
		return m_iWeaponId;
	}

	inline void SetPrimaryRange(float fMinRange, float fMaxRange)
	{
		m_fPrimMinWeaponShootDist = fMinRange;
		m_fPrimMaxWeaponShootDist = fMaxRange;
	}

	inline void SetSecondaryRange(float fMinRange, float fMaxRange)
	{
		m_fSecMinWeaponShootDist = fMinRange;
		m_fSecMaxWeaponShootDist = fMaxRange;
	}

	inline int GetAmmoIndex1()
	{
		return m_iAmmoIndex1;
	}

	inline int GetAmmoIndex2()
	{
		return m_iAmmoIndex2;
	}

	inline int GetSlot()
	{
		return m_iSlot;
	}

	void SetAmmoIndex(int iAmmoIndex1, int iAmmoIndex2 = -1)
	{
		m_iAmmoIndex1 = iAmmoIndex1;
		m_iAmmoIndex2 = iAmmoIndex2;
	}

	inline bool CanUseSecondary()
	{
		return HasSomeFlags(WEAP_FL_SEC_ATTACK);
	}

	inline float GetProjectileSpeed()
	{
		return m_fProjectileSpeed;
	}

private:

	inline bool HasAllFlags(int iFlags) const
	{
		return (m_iFlags & iFlags) == iFlags;
	}

	inline bool HasSomeFlags(int iFlags) const
	{
		return (m_iFlags & iFlags) > 0;
	}

	const char *m_szWeaponName; // classname

	int m_iWeaponId;			// identification
	int m_iFlags;				// flags
	int m_iAmmoIndex1;
	int m_iAmmoIndex2;
	int m_iPreference;
	int m_iSlot;

	// shoot distance
	float m_fPrimMinWeaponShootDist;
	float m_fPrimMaxWeaponShootDist;

	float m_fSecMinWeaponShootDist;
	float m_fSecMaxWeaponShootDist;

	float m_fProjectileSpeed;
};

class IWeaponFunc;

class CWeapons
{
public:
	CWeapons()
	{
		m_theWeapons.clear();
	}

	static inline void AddWeapon(CWeapon *pWeapon) { m_theWeapons.push_back(pWeapon); }

	static CWeapon *GetWeapon(const int iId);

	static CWeapon *GetWeapon(const char *szWeapon);

	static CWeapon *GetWeaponByShortName(const char *pszWeapon);

	static void EachWeapon(IWeaponFunc *pFunc);

	static void FreeMemory();

	static edict_t *FindWeapon(edict_t *pPlayer, const char *szWeaponName);

	static void LoadWeapons(const char *szWeaponListName, WeaponsData_t *pDefault);

private:
	// available weapons in game
	static std::vector<CWeapon*> m_theWeapons;
};

#define AMMO_PRIM 1
#define AMMO_SEC 2

////////////////////////////////////////////////////////////
// Weapon but with bot holding it and ammo information etc
////////////////////////////////////////////////////////////
class CBotWeapon
{
public:
	CBotWeapon()
	{
		m_pWeaponInfo = NULL;
		m_bHasWeapon = false;
		m_iWeaponIndex = 0;
		m_pEnt = NULL;
		m_iClip1 = NULL;
		m_iClip2 = NULL;
	}

	inline void SetWeapon(CWeapon *pWeapon)
	{
		m_pWeaponInfo = pWeapon;
	}

	inline float GetPrimaryMaxRange()
	{
		return m_pWeaponInfo->PrimaryMaxRange();
	}

	inline bool PrimaryInRange(float fDistance)
	{
		return m_pWeaponInfo->PrimaryInRange(fDistance);
	}

	inline bool PrimaryGreaterThanRange(float fDistance)
	{
		return m_pWeaponInfo->PrimaryGreaterThanRange(fDistance);
	}

	inline bool NeedsDeployedOrZoomed()
	{
		return m_pWeaponInfo->NeedsDeployedOrZoomed();
	}

	inline bool IsGravGun()
	{
		return m_pWeaponInfo->IsGravGun();
	}

	inline bool IsExplosive()
	{
		return m_pWeaponInfo->IsExplosive();
	}

	inline bool IsZoomable()
	{
		return m_pWeaponInfo->IsZoomable();
	}

	inline bool CanUseUnderWater()
	{
		return m_pWeaponInfo->CanUseUnderWater();
	}

	inline bool HasHighRecoil()
	{
		return m_pWeaponInfo->HasHighRecoil();
	}

	inline int GetID()
	{
		return m_pWeaponInfo->GetID();
	}

	inline bool IsSpecial()
	{
		return m_pWeaponInfo->IsSpecial();
	}

	inline float PrimaryMaxRange()
	{
		return (m_pWeaponInfo->PrimaryMaxRange());
	}

	inline bool IsDeployable()
	{
		return m_pWeaponInfo->IsDeployable();
	}

	inline bool MustHoldAttack()
	{
		return m_pWeaponInfo->MustHoldAttack();
	}

	inline bool CanDestroyPipeBombs()
	{
		return m_pWeaponInfo->CanDestroyPipeBombs();
	}

	inline bool IsProjectile()
	{
		return m_pWeaponInfo->IsProjectile();
	}

	inline float GetProjectileSpeed()
	{
		return m_pWeaponInfo->GetProjectileSpeed();
	}

	inline bool CanDeflectRockets()
	{
		return m_pWeaponInfo->CanDeflectRockets();
	}

	inline bool CanUseSecondary()
	{
		return m_pWeaponInfo->CanUseSecondary();
	}

	inline bool IsMelee()
	{
		return m_pWeaponInfo->IsMelee();
	}

	inline bool IsMeleeSecondary()
	{
		return m_pWeaponInfo->IsMeleeSecondary();
	}

	inline bool SecondaryInRange(float fDistance)
	{
		return m_pWeaponInfo->SecondaryInRange(fDistance);
	}

	inline int GetPreference()
	{
		return m_pWeaponInfo->GetPreference();
	}

	bool OutOfAmmo(CBot *pBot);

	bool NeedToReload(CBot *pBot);

	inline void SetHasWeapon(bool bHas)
	{
		m_bHasWeapon = bHas;
	}

	inline bool HasWeapon()
	{
		return m_bHasWeapon;
	}

	inline bool CanAttack()
	{
		return m_pWeaponInfo->CanAttack();
	}

	int GetAmmo(CBot *pBot, int type = AMMO_PRIM);

	int GetClip1(CBot *pBot)
	{
		if (m_iClip1)
			return *m_iClip1;

		return 0;
	}

	int GetClip2(CBot *pBot)
	{
		if (m_iClip2)
			return *m_iClip2;

		return 0;
	}

	CWeapon *GetWeaponInfo() { return m_pWeaponInfo; }

	inline int GetWeaponIndex() { return m_iWeaponIndex; }

	inline void SetWeaponIndex(int iIndex) { m_iWeaponIndex = iIndex; } // Entity Index

	void SetWeaponEntity(edict_t *pent, bool bOverrideAmmoTypes = true);

	inline edict_t *GetWeaponEntity() { return m_pEnt; }


private:

	// link to weapon info
	CWeapon *m_pWeaponInfo;

	int m_iWeaponIndex;

	bool m_bHasWeapon;

	edict_t *m_pEnt;

	int *m_iClip1;
	int *m_iClip2;
};

// Weapons that
class CBotWeapons
{
public:
	/////////////////////////////////////
	CBotWeapons(CBot *pBot);    // // constructor
	/////////////////////////////////////
	CBotWeapon *GetBestWeapon(edict_t *pEnemy, bool bAllowMelee = true, bool bAllowMeleeFallback = true, bool bMeleeOnly = false, bool bExplosivesOnly = false, bool bIgnorePrimaryMinimum = false);

	CBotWeapon *AddWeapon(CWeapon *pWeaponInfo, int iId, edict_t *pent, bool bOverrideAll = true);

	CBotWeapon *GetWeapon(CWeapon *pWeapon);

	CBotWeapon *GetActiveWeapon(const char *szWeaponName, edict_t *pWeaponUpdate = NULL, bool bOverrideAmmoTypes = true);

	CBotWeapon *GetCurrentWeaponInSlot(int iSlot);

	CBotWeapon *GetGrenade()
	{
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			if (m_theWeapons[i].HasWeapon())
			{
				if (m_theWeapons[i].GetWeaponInfo())
				{
					if (m_theWeapons[i].GetWeaponInfo()->IsGrenade())
						return &(m_theWeapons[i]);
				}
			}
		}

		return NULL;
	}

	void ClearWeapons();

	bool HasWeapon(int id);

	bool HasExplosives(void);

	// returns true if there is a change to the weapons
	bool Update(bool bOverrideAllFromEngine = true); // update from sendprop

	CBotWeapon *GetPrimaryWeapon(); // return the most important weapon bot is holding if if out o ammo

private:
	// bot that has these weapons
	CBot *m_pBot;

	// mask of weapons bot already has so we know if we need to update or not
	unsigned int m_iWeaponsSignature;

	// weapons local to the bot only 
	// (holds ammo/preference etc and link to actual weapon)
	CBotWeapon m_theWeapons[MAX_WEAPONS];//[MAX_WEAPONS];

	float m_fUpdateWeaponsTime;
};

#endif