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
#ifndef __BOT_FORTRESS_H__
#define __BOT_FORTRESS_H__

#include "bot_utility.h"

//#include <stack>
//using namespace std;

#define TF2_ROCKETSPEED   1100
#define TF2_GRENADESPEED  1065 // TF2 wiki
#define TF2_MAX_SENTRYGUN_RANGE 1024
#define TF2_STICKYGRENADE_MAX_DISTANCE 1600

class CBotWeapon;
class CWaypoint;
class CBotUtility;

#define TF2_SLOT_PRMRY 0 // primary
#define TF2_SLOT_SCNDR 1 // secondary
#define TF2_SLOT_MELEE 2
#define TF2_SLOT_PDA 3
#define TF2_SLOT_PDA2 4
#define TF2_SLOT_HAT 5
#define TF2_SLOT_MISC 6
#define TF2_SLOT_ACTION 7
#define TF2_SLOT_MAX 8

#define TF2_TEAM_BLUE 3
#define TF2_TEAM_RED 2

#define RESIST_BULLETS 0
#define RESIST_EXPLO 1
#define RESIST_FIRE 2

#define TF2_SENTRY_LEVEL1_HEALTH 150
#define TF2_SENTRY_LEVEL2_HEALTH 180
#define TF2_SENTRY_LEVEL3_HEALTH 216

#define TF2_DISPENSER_LEVEL1_HEALTH 150
#define TF2_DISPENSER_LEVEL2_HEALTH 180
#define TF2_DISPENSER_LEVEL3_HEALTH 216

// Naris @ Alliedmodders.net
/*
enum TFCond
{
	TFCond_Slowed = 0,
	TFCond_Zoomed,
	TFCond_Disguising,
	TFCond_Disguised,
	TFCond_Cloaked,
	TFCond_Ubercharged,
	TFCond_TeleportedGlow,
	TFCond_Taunting,
	TFCond_UberchargeFading,
	TFCond_Unknown1, //9
	TFCond_CloakFlicker = 9,
	TFCond_Teleporting,
	TFCond_Kritzkrieged,
	TFCond_Unknown2, //12
	TFCond_TmpDamageBonus = 12,
	TFCond_DeadRingered,
	TFCond_Bonked,
	TFCond_Dazed,
	TFCond_Buffed,
	TFCond_Charging,
	TFCond_DemoBuff,
	TFCond_CritCola,
	TFCond_InHealRadius,
	TFCond_Healing,
	TFCond_OnFire,
	TFCond_Overhealed,
	TFCond_Jarated,
	TFCond_Bleeding,
	TFCond_DefenseBuffed,
	TFCond_Milked,
	TFCond_MegaHeal,
	TFCond_RegenBuffed,
	TFCond_MarkedForDeath,
	TFCond_NoHealingDamageBuff,
	TFCond_SpeedBuffAlly, // 32
	TFCond_HalloweenCritCandy,
	TFCond_CritCanteen,
	TFCond_CritDemoCharge,
	TFCond_CritHype,
	TFCond_CritOnFirstBlood,
	TFCond_CritOnWin,
	TFCond_CritOnFlagCapture,
	TFCond_CritOnKill,
	TFCond_RestrictToMelee,
	TFCond_DefenseBuffNoCritBlock,
	TFCond_Reprogrammed,
	TFCond_CritMmmph,
	TFCond_DefenseBuffMmmph,
	TFCond_FocusBuff,
	TFCond_DisguiseRemoved,
	TFCond_MarkedForDeathSilent,
	TFCond_DisguisedAsDispenser,
	TFCond_Sapped,
	TFCond_UberchargedHidden,
	TFCond_UberchargedCanteen,
	TFCond_HalloweenBombHead,
	TFCond_HalloweenThriller,
	TFCond_RadiusHealOnDamage,
	TFCond_CritOnDamage,
	TFCond_UberchargedOnTakeDamage,
	TFCond_UberBulletResist,
	TFCond_UberBlastResist,
	TFCond_UberFireResist,
	TFCond_SmallBulletResist,
	TFCond_SmallBlastResist,
	TFCond_SmallFireResist,
	TFCond_Stealthed, // 64
	TFCond_MedigunDebuff,
	TFCond_StealthedUserBuffFade,
	TFCond_BulletImmune,
	TFCond_BlastImmune,
	TFCond_FireImmune,
	TFCond_PreventDeath,
	TFCond_MVMBotRadiowave,
	TFCond_HalloweenSpeedBoost,
	TFCond_HalloweenQuickHeal,
	TFCond_HalloweenGiant,
	TFCond_HalloweenTiny,
	TFCond_HalloweenInHell,
	TFCond_HalloweenGhostMode,

	TFCond_DodgeChance = 79,
	TFCond_Parachute,
	TFCond_BlastJumping,
	TFCond_HalloweenKart,
	TFCond_HalloweenKartDash,
	TFCond_BalloonHead,
	TFCond_MeleeOnly,
	TFCond_SwimmingCurse,
	TFCond_HalloweenKartNoTurn,
	TFCond_HalloweenKartCage,
	TFCond_HasRune,
	TFCond_RuneStrength,
	TFCond_RuneHaste,
	TFCond_RuneRegen,
	TFCond_RuneResist,
	TFCond_RuneVampire,
	TFCond_RuneWarlock,
	TFCond_RunePrecision, // 96
	TFCond_RuneAgility,
};*/

#define TF2_PLAYER_BONKED		(1<<14)
#define TF2_PLAYER_SLOWED       (1 << 0)    // 1
#define TF2_PLAYER_ZOOMED       (1 << 1)    // 2
#define TF2_PLAYER_DISGUISING   (1 << 2)    // 4
#define TF2_PLAYER_DISGUISED	(1 << 3)    // 8
#define TF2_PLAYER_CLOAKED      (1 << 4)    // 16
#define TF2_PLAYER_INVULN       (1 << 5)    // 32
#define TF2_PLAYER_TELEGLOW     (1 << 6)    // 64
#define TF2_PLAYER_KRITS		524288
#define TF2_PLAYER_HEALING	    2097152    
#define TF2_PLAYER_TAUNTING	    (1 << 7)    // 128
#define TF2_PLAYER_TELEPORTING	(1<<10)    // 1024 Player is teleporting
#define TF2_PLAYER_ONFIRE	    4194304 // fix may 2013

//#define TF2_SPY_FOV_KNIFEATTACK 90.0f

typedef enum
{
    TF_VC_MEDIC = 0,
	TF_VC_INCOMING = 1,
	TF_VC_HELP = 2,
    TF_VC_THANKS = 4,
	TF_VC_SPY = 5,
	TF_VC_BATTLECRY = 6,
    TF_VC_GOGOGO = 8,
    TF_VC_SENTRYAHEAD = 9,
	TF_VC_CHEERS = 10,
    TF_VC_MOVEUP = 12,
    TF_VC_TELEPORTERHERE =13,
    TF_VC_JEERS = 14,
    TF_VC_GOLEFT = 16,
	TF_VC_DISPENSERHERE = 17,
	TF_VC_POSITIVE = 18,
    TF_VC_GORIGHT = 20,
    TF_VC_SENTRYHERE = 21,
    TF_VC_NEGATIVE = 22,
    TF_VC_YES = 24,
    TF_VC_ACTIVATEUBER = 25,
    TF_VC_NICESHOT = 26,
    TF_VC_NO = 28,
    TF_VC_UBERREADY = 29,
    TF_VC_GOODJOB = 30,
	TF_VC_INVALID = 31
}eTFVoiceCMD;


typedef enum
{
	TF_TRAP_TYPE_NONE,
	TF_TRAP_TYPE_WPT,
	TF_TRAP_TYPE_POINT,
	TF_TRAP_TYPE_FLAG,
	TF_TRAP_TYPE_PL,
	TF_TRAP_TYPE_ENEMY
}eDemoTrapType;

typedef enum
{
	TF_CLASS_CIVILIAN = 0,
	TF_CLASS_SCOUT,
	TF_CLASS_SNIPER,
	TF_CLASS_SOLDIER,
	TF_CLASS_DEMOMAN,
	TF_CLASS_MEDIC,
	TF_CLASS_HWGUY,
	TF_CLASS_PYRO,
	TF_CLASS_SPY,
	TF_CLASS_ENGINEER,
	TF_CLASS_MAX
}TF_Class;

/*enum
{
	TF_TEAM_SPEC = 0,
	TF_TEAM_BLUE = 1,
	TF_TEAM_RED = 2,
	TF_TEAM_GREEN = 3,
	TF_TEAM_YELLOW = 4
};

typedef enum
{
	ENGI_DISP = 0,
	ENGI_ENTRANCE,
	ENGI_EXIT,
	ENGI_SENTRY,
	ENGI_SAPPER
}eEngiBuild;*/
typedef enum
{
	ENGI_DISP = 0,
	ENGI_TELE,
	ENGI_SENTRY,
	ENGI_SAPPER,
	ENGI_EXIT,
	ENGI_ENTRANCE,
}eEngiBuild;

typedef enum
{
	ENGI_BUILD,
	ENGI_DESTROY
}eEngiCmd;

class CBotTF2FunctionEnemyAtIntel : public IBotFunction
{
public:
	CBotTF2FunctionEnemyAtIntel( int iTeam, Vector vPos, int type, edict_t *pPlayer = NULL, int capindex = -1 ){m_iTeam = iTeam;m_vPos = vPos;m_iType = type; m_pPlayer = pPlayer; m_iCapIndex = capindex; }

	void execute (CBot *pBot);
private:
	int m_iTeam;
	Vector m_vPos;
	int m_iType;
	edict_t *m_pPlayer;
	int m_iCapIndex;
};

class CBroadcastSpySap : public IBotFunction
{
public:
	CBroadcastSpySap (edict_t *pSpy) { m_pSpy = pSpy; }
	void execute ( CBot *pBot );

private:
	edict_t *m_pSpy;
};

class CBroadcastOvertime : public IBotFunction
{
public:
	CBroadcastOvertime () {};
	void execute (CBot *pBot);
};

class CBroadcastFlagReturned : public IBotFunction
{
public:
	CBroadcastFlagReturned (int iTeam) { m_iTeam = iTeam; }
	void execute ( CBot *pBot );

private:
	int m_iTeam;
};

class CBroadcastFlagDropped : public IBotFunction
{
public:
	CBroadcastFlagDropped (int iTeam, Vector origin) { m_iTeam = iTeam; m_vOrigin = origin; }
	void execute ( CBot *pBot );

private:
	Vector m_vOrigin;
	int m_iTeam;
};

class CBroadcastFlagCaptured : public IBotFunction
{
public:
	CBroadcastFlagCaptured(int iTeam) { m_iTeam = iTeam; }

	void execute ( CBot *pBot );
private:
	int m_iTeam;
};

class CBroadcastRoundStart : public IBotFunction
{
public:
	CBroadcastRoundStart ( bool bFullReset ) { m_bFullReset = bFullReset; }
	void execute ( CBot *pBot );
private:
	bool m_bFullReset;
};

class CBroadcastCapturedPoint : public IBotFunction
{
public:
	CBroadcastCapturedPoint ( int iPoint, int iTeam, const char *szName );

	void execute ( CBot *pBot );
private:
	int m_iPoint;
	int m_iTeam;
	const char *m_szName;
};


class CBasePlayer;

template< class T, class I = int >
class CUtlMemoryTF2Items : public CUtlMemory< T, I >
{
public:
	CUtlMemoryTF2Items( int nGrowSize = 0, int nInitSize = 0 ) { CUtlMemory< T, I >( nGrowSize, nInitSize ); }
    CUtlMemoryTF2Items( T* pMemory, int numElements ) { CUtlMemory< T, I >( pMemory, numElements ); }
    CUtlMemoryTF2Items( const T* pMemory, int numElements ) { CUtlMemory< T, I >( pMemory, numElements ); }
    //~CUtlMemoryTF2Items() { ~CUtlMemory< T, I >(); }
    
	void Purge()
	{
		if ( !CUtlMemory< T, I >::IsExternallyAllocated() )
		{
			if (CUtlMemory< T, I >::m_pMemory)
			{
				UTLMEMORY_TRACK_FREE();
				//free( (void*)m_pMemory );
#ifdef TF2ITEMS_DEBUG_ITEMS
				META_CONPRINTF("CUtlMemory tried to be freed!\n");
#endif
				CUtlMemory< T, I >::m_pMemory = 0;
			}
			CUtlMemory< T, I >::m_nAllocationCount = 0;
		}
	}
};

// Taken from the TF2Items extension by Asher "asherkin" Baker
class CAttributeManager;

class CEconItemAttribute
{
public:
	CEconItemAttribute() {};

	CEconItemAttribute(uint16 iAttributeDefinitionIndex, float flValue/* = -1.0*/)
	{
		this->m_iAttributeDefinitionIndex = iAttributeDefinitionIndex;
		this->m_flValue = flValue;
		this->m_nRefundableCurrency = 0;
		this->m_pVTable = NULL;
	}

public:
	void *m_pVTable;

	uint16 m_iAttributeDefinitionIndex;
	float m_flValue;
	int32 m_nRefundableCurrency;
};

class CEconItem;

class CEconItemHandle
{
public:
	void *m_pVTable;

	CEconItem *m_pItem;

	int64 m_ulItemID;
	uint64 m_SteamID;
};

class CAttributeList
{
public:
	void *m_pVTable;

	CUtlVector<CEconItemAttribute, CUtlMemoryTF2Items<CEconItemAttribute> > m_Attributes;
	void *m_pAttributeManager;


public:
	CAttributeList& operator=(const CAttributeList &other)
	{
		m_pVTable = other.m_pVTable;

		m_Attributes = other.m_Attributes;
		m_pAttributeManager = other.m_pAttributeManager;


		return *this;
	}
};


class ITexture;
class ITextureCompositor;
class CEconItemView
{
public:
	void *m_pVTable; //0

	uint16 m_iItemDefinitionIndex; //4

	int32 m_iEntityQuality; //8
	uint32 m_iEntityLevel; //12

	uint64 m_iItemID; //16
	uint32 m_iItemIDHigh; //24
	uint32 m_iItemIDLow; //28

	uint32 m_iAccountID; //32

	uint32 m_iInventoryPosition; //36

	CEconItemHandle m_ItemHandle; //40 (44, 48, 52, 56, 60)

	bool m_bColorInit; //64
	bool m_bPaintOverrideInit; //65
	bool m_bHasPaintOverride; //66
	//67

	float m_flOverrideIndex; //68
	uint32 m_unRGB; //72
	uint32 m_unAltRGB; //76

	int32 m_iTeamNumber; //80

	bool m_bInitialized; //84

	CAttributeList m_AttributeList; //88 (92, 96, 100, 104, 108, 112)
	CAttributeList m_NetworkedDynamicAttributesForDemos; //116 (120, 124, 128, 132, 136, 140)

	bool m_bDoNotIterateStaticAttributes; //144
};

#define EVENT_FLAG_PICKUP 0
#define EVENT_CAPPOINT    1


class CBotFortress : public CBot
{
public:	

	CBotFortress();

	//virtual bool wantToZoom () { return m_bWantToZoom; }

	//virtual void wantToZoom ( bool bSet ) { m_bWantToZoom = bSet; }

	virtual void enemyLost (edict_t *pEnemy);

	virtual void updateConditions();

	virtual void shot ( edict_t *pEnemy );

	virtual int engiBuildObject ( int *iState, eEngiBuild iObject, float *fTime, int *iTries );

	virtual float getEnemyFactor ( edict_t *pEnemy ) { return CBot::getEnemyFactor(pEnemy); }

	virtual void checkDependantEntities();

	int getMetal ();

	//virtual Vector getAimVector ( edict_t *pEntity ) { return CBot::getAimVector(pEntity); }

	virtual void modAim ( edict_t *pEntity, Vector &v_origin, Vector *v_desired_offset, Vector &v_size, float fDist, float fDist2D )
	{
		CBot::modAim(pEntity,v_origin,v_desired_offset,v_size,fDist,fDist2D);
	}

	virtual void touchedWpt ( CWaypoint *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1 ) { CBot::touchedWpt(pWaypoint); }

	inline edict_t *getHealingEntity () { return m_pHeal; }

	inline void clearHealingEntity () { m_pHeal = NULL; }

	virtual unsigned int maxEntityIndex ( ) { return gpGlobals->maxEntities; }

	virtual void init (bool bVarInit=false);

	virtual void foundSpy (edict_t *pEdict, TF_Class iDisguise );

	virtual void getTasks ( unsigned int iIgnore = 0 ) { CBot :: getTasks(iIgnore); }

	virtual void died ( edict_t *pKiller, const char *pszWeapon );

	virtual void killed ( edict_t *pVictim, char *weapon );

	virtual void modThink ();

	bool isBuilding ( edict_t *pBuilding );

	float getHealFactor ( edict_t *pPlayer );

	virtual bool wantToFollowEnemy ();

	virtual void checkBuildingsValid (bool bForce = false) {};

	virtual void checkHealingValid ();

// linux fix 2
	virtual edict_t *findEngineerBuiltObject ( eEngiBuild iBuilding, int index ) { return NULL; }

	virtual void engineerBuild ( eEngiBuild iBuilding, eEngiCmd iEngiCmd ) {};

	virtual void spyDisguise ( int iTeam, int iClass ) {};

	virtual bool lookAfterBuildings (float *fTime) { return false; }

	inline void nextLookAfterSentryTime ( float fTime ) { m_fLookAfterSentryTime = fTime; }

	inline edict_t *getSentry () { return m_pSentryGun; }

	virtual bool hasEngineerBuilt ( eEngiBuild iBuilding ) {return false;}

	virtual void engiBuildSuccess ( eEngiBuild iObject, int index ) {};

	virtual bool healPlayer ( edict_t *pPlayer, edict_t *pPrevPlayer ) { return false; }
	virtual bool upgradeBuilding ( edict_t *pBuilding, bool removesapper = false) {return false;}

	virtual bool isCloaked () { return false; }
	virtual bool isDisguised () { return false; }

	virtual CBotWeapon *getCurrentWeapon()
	{
		return CBot::getCurrentWeapon();
	}

	virtual bool handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy ) { return CBot::handleAttack(pWeapon,pEnemy); }
	
	void resetAttackingEnemy() { m_pAttackingEnemy = NULL; }

	virtual bool setVisible ( edict_t *pEntity, bool bVisible );

	virtual void setClass ( TF_Class _class );

	inline edict_t *seeFlag ( bool reset = false ) { if ( reset ) { m_pFlag = NULL; } return m_pFlag; }

	virtual bool canAvoid ( edict_t *pEntity );

	virtual bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	virtual bool startGame ();

	virtual void spawnInit ();

	bool isTF () { return true; }

	virtual bool isTF2 () { return false; }

	virtual bool hurt ( edict_t *pAttacker, int iHealthNow, bool bDontHide  = false )
	{
		return CBot::hurt(pAttacker,iHealthNow,bDontHide);
	}

	void chooseClass ();

	virtual TF_Class getClass () { return TF_CLASS_CIVILIAN; }

	virtual void updateClass () { };

	virtual void currentlyDead ();

	virtual void onInventoryApplication (){}

	void pickedUpFlag ();

	inline bool hasFlag () { return m_bHasFlag; }

	inline void droppedFlag () { m_bHasFlag = false; }

	void medicCalled ( edict_t *pPlayer );

	bool isAlive ();

	void enemyDown (edict_t *pEnemy) 
	{ 
		CBot::enemyDown(pEnemy);

		if ( pEnemy == m_pPrevSpy )
		{
			m_pPrevSpy = NULL;
			m_fSeeSpyTime = 0.0f;
		}
	}

	bool isTeleporterUseful ( edict_t *pTele );

	bool waitForFlag ( Vector *vOrigin, float *fWait, bool bFindFlag );

	void flagDropped ( Vector vOrigin );
	void teamFlagDropped ( Vector vOrigin );
	void teamFlagPickup ();

	virtual bool wantToListenToPlayer ( edict_t *pPlayer, int iWeaponID = -1 ) { return true; }
	virtual bool wantToListenToPlayerFootsteps ( edict_t *pPlayer ) { return true; }
	virtual bool wantToInvestigateSound () { return true; }

	inline void flagReset () { m_fLastKnownFlagTime = 0.0f; }
	inline void teamFlagReset () { m_fLastKnownTeamFlagTime = 0.0f; }

	virtual bool canGotoWaypoint ( Vector vPrevWaypoint, CWaypoint *pWaypoint, CWaypoint *pPrev = NULL )
	{
		return CBot::canGotoWaypoint(vPrevWaypoint,pWaypoint,pPrev);
	}

	virtual void setup ();

	virtual bool needHealth();

	virtual bool needAmmo ();

	void waitBackstab ();

	void wantToDisguise ( bool bSet );

	virtual bool select_CWeapon ( CWeapon *pWeapon ) { return CBot::select_CWeapon(pWeapon); }
	virtual bool selectBotWeapon ( CBotWeapon *pBotWeapon ) { return CBot::selectBotWeapon(pBotWeapon); }

	virtual bool getIgnoreBox ( Vector *vLoc, float *fSize );

	// found a new enemy
	virtual void enemyFound (edict_t *pEnemy){CBot::enemyFound(pEnemy); }

	bool wantToNest ();

	bool overrideAmmoTypes () { return false; }

	bool wantToCloak();

	bool wantToUnCloak();

	bool someoneCalledMedic ();

	void waitCloak ();

	void detectedAsSpy ( edict_t *pDetector, bool bDisguiseComprimised );

	// return an enemy sentry gun / special visible (e.g.) for quick checking
	virtual edict_t *getVisibleSpecial ();

	inline bool isBeingHealed () { return m_bIsBeingHealed; }

	virtual void handleWeapons () { CBot::handleWeapons(); }

	virtual void seeFriendlyDie ( edict_t *pDied, edict_t *pKiller, CWeapon *pWeapon ) { CBot::seeFriendlyDie(pDied,pKiller,pWeapon); }
	virtual void seeFriendlyKill ( edict_t *pTeamMate, edict_t *pDied, CWeapon *pWeapon ) { CBot::seeFriendlyKill(pTeamMate,pDied,pWeapon); }

	virtual void voiceCommand ( int cmd ) { };

	virtual void seeFriendlyHurtEnemy ( edict_t *pTeammate, edict_t *pEnemy, CWeapon *pWeapon );
	
	bool incomingRocket ( float fRange );

	virtual void hearPlayerAttack( edict_t *pAttacker, int iWeaponID ) { CBot::hearPlayerAttack(pAttacker,iWeaponID); }
protected:
	virtual void selectTeam ();

	virtual void selectClass ();

	virtual void callMedic ();

	static bool isClassOnTeam ( int iClass, int iTeam );

	int getSpyDisguiseClass ( int iTeam );

	virtual bool thinkSpyIsEnemy ( edict_t *pEdict, TF_Class iDisguise );

	virtual bool checkStuck ( void ) { return CBot::checkStuck(); }

	float m_fCallMedic;
	float m_fTauntTime;
	float m_fTaunting;
	float m_fDefendTime;

	float m_fHealFactor;

	MyEHandle m_pHeal;
	MyEHandle m_pLastHeal;
	MyEHandle m_pSentryGun;
	MyEHandle m_pDispenser;
	MyEHandle m_pTeleEntrance;
	MyEHandle m_pTeleExit;

	MyEHandle m_pAmmo;
	MyEHandle m_pHealthkit;

	MyEHandle m_pNearestDisp;
	MyEHandle m_pNearestEnemySentry;
	MyEHandle m_pNearestAllySentry;
	MyEHandle m_pNearestEnemyTeleporter;
	MyEHandle m_pNearestEnemyDisp;
	MyEHandle m_pNearestTeleEntrance;
	MyEHandle m_pNearestPipeGren;
	MyEHandle m_pAttackingEnemy;

	MyEHandle m_pFlag;
	MyEHandle m_pPrevSpy;

	float m_fFrenzyTime;
	float m_fSpyCloakTime;
	float m_fSpyUncloakTime;
	float m_fSeeSpyTime;
	float m_fLastSeeSpyTime;
	float m_fSpyDisguiseTime;
	float m_fLastSaySpy;
	float m_fPickupTime;
	float m_fLookAfterSentryTime;

	TF_Class m_iPrevSpyDisguise;

	Vector m_vLastSeeSpy;

	// valid flag point area
	Vector m_vLastKnownFlagPoint;
	Vector m_vLastKnownTeamFlagPoint;

	Vector m_vTeleportEntrance;
	bool m_bEntranceVectorValid;
	Vector m_vSentryGun;
	bool m_bSentryGunVectorValid;
	Vector m_vDispenser;
	bool m_bDispenserVectorValid;
	Vector m_vTeleportExit;
	bool m_bTeleportExitVectorValid;

	// 1 minute wait
	float m_fLastKnownFlagTime;
	float m_fLastKnownTeamFlagTime;

	float m_fBackstabTime;

	TF_Class m_iClass;

	float m_fUpdateClass;
	float m_fUseTeleporterTime;

	bool m_bHasFlag;	
	float m_fSnipeAttackTime;

	// time left before the bot decides if it wants to change class
	float m_fChangeClassTime;
	// bot should check if he can change class now
	bool m_bCheckClass;
	MyEHandle m_pLastCalledMedic;
	CBotLastSee m_pLastSeeMedic;
	/*MyEHandle m_pLastSeeMedic;
	Vector m_vLastSeeMedic;
	float m_fLastSeeMedicTime;*/
	float m_fLastCalledMedicTime;
	bool m_bIsBeingHealed;
	float m_fMedicUpdatePosTime;
	Vector m_vMedicPosition;

	bool m_bCanBeUbered;
	float m_fCheckHealTime;

	float m_fClassDisguiseFitness[10]; // classes disguised as fitness
	float m_fClassDisguiseTime[10];
	float m_fDisguiseTime;
	unsigned short m_iDisguiseClass;
	float m_fSentryPlaceTime;
	unsigned int m_iSentryKills;
	float m_fTeleporterEntPlacedTime;
	float m_fTeleporterExtPlacedTime;
	unsigned m_iTeleportedPlayers;

	// list of spies who I saw were attacked by my team-mates recently
	// for use with spy checking
	float m_fSpyList[MAX_PLAYERS];

	int m_iTeam;

	float m_fWaitTurnSentry;			// amount of time to wait before engineer turns their sentry before building

	// currently unused
	float m_fCallMedicTime[MAX_PLAYERS]; // for every player ID is kept the last time they called medic

	int m_iLastFailSentryWpt;
	int m_iLastFailTeleExitWpt;

	MyEHandle m_pHealer;

	float m_fHealingMoveTime;

	MyEHandle m_pLastEnemySentry;
	MyEHandle m_NearestEnemyRocket;
	MyEHandle m_NearestEnemyGrenade;

	float m_fLastSentryEnemyTime;
	//bool m_bWantToZoom;
};
//
//
//
//
class CTF2Loadout;

class CTF2LoadoutAdded
{
public:
	CTF2LoadoutAdded ( CBaseEntity *pEnt, CTF2Loadout *pLoadout )
	{
		m_pEnt = pEnt;
		m_loadout = pLoadout;
	}

	CBaseEntity *m_pEnt;
	CTF2Loadout *m_loadout;
	
};

class CBotTF2 : public CBotFortress
{
public:

	// 
	CBotTF2();

	virtual CBotWeapon *getCurrentWeapon();

	void onInventoryApplication();

	void giveWeapon ( int slot, int index );
	void PostGiveNamedItem ( CEconItemView *cscript );

	void MannVsMachineWaveComplete();
	void MannVsMachineAlarmTriggered (Vector vLoc);

	bool sentryRecentlyHadEnemy ();

	void highFivePlayer ( edict_t *pPlayer, float fYaw );

	virtual bool hurt ( edict_t *pAttacker, int iHealthNow, bool bDontHide  = false );

	void updateAttackDefendPoints ();

	void updateAttackPoints ();
	void updateDefendPoints ();

	// found a new enemy
	void enemyFound (edict_t *pEnemy);

	void enemyAtIntel ( Vector vPos, int type = EVENT_FLAG_PICKUP, int iArea = -1 );

	bool isTF2 () { return true; }

	void checkDependantEntities ();

	virtual bool wantToListenToPlayerAttack ( edict_t *pPlayer, int iWeaponID = -1 );
	virtual bool wantToListenToPlayerFootsteps ( edict_t *pPlayer );

	bool wantToInvestigateSound ();

	void getDefendArea ( vector<int> *m_iAreas );

	void getAttackArea ( vector <int> *m_iAreas );

	int getCurrentAttackArea () { return m_iCurrentAttackArea; }
	int getCurrentDefendArea () { return m_iCurrentDefendArea; }

	void pointsUpdated ( );

	eBotFuncState rocketJump(int *iState,float *fTime);

	virtual bool wantToFollowEnemy ();

	void resetCloakTime () { m_fSpyCloakTime = 0.0f; }

	float getEnemyFactor ( edict_t *pEnemy );

	void foundSpy (edict_t *pEdict, TF_Class iDisguise);

	void touchedWpt ( CWaypoint *pWaypoint, int iNextWaypoint = -1, int iPrevWaypoint = -1 );

	bool handleAttack ( CBotWeapon *pWeapon, edict_t *pEnemy );

	void engiBuildSuccess ( eEngiBuild iObject, int index );

	bool lookAfterBuildings (float *fTime);

	void spawnInit ();

	bool setVisible ( edict_t *pEntity, bool bVisible );

	//Vector getAimVector ( edict_t *pEntity );
	virtual void modAim ( edict_t *pEntity, Vector &v_origin, 
		Vector *v_desired_offset, Vector &v_size,
		float fDist, float fDist2D);

	void modThink ();

	bool isCloaked ();

	bool executeAction ( CBotUtility *util );//eBotAction id, CWaypoint *pWaypointResupply, CWaypoint *pWaypointHealth, CWaypoint *pWaypointAmmo );

	void setClass ( TF_Class _class );

	bool isDisguised ();

	void checkBuildingsValid (bool bForce = false);

	edict_t *findEngineerBuiltObject ( eEngiBuild iBuilding, int index );

	bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	bool isTF () { return true; }

	void taunt ( bool bOverride = false );

	void callMedic ();

	void roundReset (bool bFullReset);

	void pointCaptured ( int iPoint, int iTeam, const char *szPointName );

	void engineerBuild ( eEngiBuild iBuilding, eEngiCmd iEngiCmd );

	void spyDisguise ( int iTeam, int iClass );

	bool hasEngineerBuilt ( eEngiBuild iBuilding );

	void getTasks ( unsigned int iIgnore = 0 );

	void died ( edict_t *pKiller, const char *pszWeapon );

	void killed ( edict_t *pVictim, char *weapon );

	void capturedFlag ();

	void pointCaptured ();

	void waitRemoveSap ();
	
	void roundWon ( int iTeam, bool bFullRound );

	void changeClass ();

	virtual bool needAmmo();

	void buildingDestroyed ( int iType, edict_t *pAttacker, edict_t *pEdict );

	TF_Class getClass ();

	void updateClass ();

	bool healPlayer ( );
	
	bool upgradeBuilding ( edict_t *pBuilding, bool removesapper = false );

	void setup ();

	void buildingSapped ( eEngiBuild building, edict_t *pSapper, edict_t *pSpy );

	void sapperDestroyed ( edict_t *pSapper );
	
	bool canGotoWaypoint ( Vector vPrevWaypoint, CWaypoint *pWaypoint, CWaypoint *pPrev = NULL );

	bool deployStickies ( eDemoTrapType type, Vector vStand, Vector vLocation, Vector vSpread, Vector *vPoint, int *iState, int *iStickyNum, bool *bFail, float *fTime, int wptindex );

	void detonateStickies (bool isJumping = false);

	void setStickyTrapType ( Vector vLocation, eDemoTrapType iTrapType ) { m_vStickyLocation = vLocation; m_iTrapType = iTrapType; }

	bool canDeployStickies ();

	bool thinkSpyIsEnemy ( edict_t *pEdict, TF_Class iDisguise );

	void seeFriendlyDie ( edict_t *pDied, edict_t *pKiller, CWeapon *pWeapon );
	void seeFriendlyKill ( edict_t *pTeamMate, edict_t *pDied, CWeapon *pWeapon );

	void voiceCommand ( int cmd );

	void handleWeapons ( void ) ;

	virtual bool select_CWeapon ( CWeapon *pWeapon );
	virtual bool selectBotWeapon ( CBotWeapon *pBotWeapon );

	void checkStuckonSpy ( void );

	bool checkStuck ( void );

	void init (bool bVarInit=false);

	bool checkAttackPoint ( void );

	bool canAvoid ( edict_t *pEntity );

	void hearVoiceCommand ( edict_t *pPlayer, byte cmd );
		
	void checkBeingHealed ( );

	void spyCloak ();

	void spyUnCloak ();

	void healedPlayer ( edict_t *pPlayer, float fAmount );

	void teleportedPlayer ( void );

	void upgradeWeapon(int iSlot);

	inline bool isCarrying () { return m_bIsCarryingObj; }

	void updateCarrying ();

	inline void resetCarryTime () { m_fCarryTime = engine->Time(); }

	void MvM_Upgrade ();

	//void addLoadoutWeapon ( CTF2Loadout *weap );
private:
	// time for next jump
	float m_fDoubleJumpTime;
	// time bot has taken to sap something
	float m_fSpySapTime;
	// 
	int m_iCurrentDefendArea;
	int m_iCurrentAttackArea;
	//
	//bool m_bBlockPushing;
	//float m_fBlockPushTime;
	//
	MyEHandle m_pDefendPayloadBomb;
	MyEHandle m_pPushPayloadBomb;
	MyEHandle m_pRedPayloadBomb;
	MyEHandle m_pBluePayloadBomb;
	//

	// if demoman has already deployed stickies this is true
	// once the demoman explodes them then this becomes false
	// and it can deploy stickies again
	//bool m_bDeployedStickies;
	eDemoTrapType m_iTrapType;
	int m_iTrapCPIndex;
	Vector m_vStickyLocation;
	float m_fRemoveSapTime;
	float m_fRevMiniGunTime;
	float m_fNextRevMiniGunTime;

	float m_fRevMiniGunBelief;
	float m_fCloakBelief;
	
	//
	MyEHandle m_pCloakedSpy;

	float m_fAttackPointTime; // used in cart maps

	 float m_prevSentryHealth;
	 float m_prevDispHealth;
	 float m_prevTeleExtHealth;
	 float m_prevTeleEntHealth;

	 float m_fDispenserHealAmount;
	 float m_fDispenserPlaceTime;

	 int m_iSentryArea;
	 int m_iDispenserArea;
	 int m_iTeleEntranceArea;
	 int m_iTeleExitArea;

	 eTFVoiceCMD m_nextVoicecmd;

	bool m_bIsCarryingTeleExit;
	bool m_bIsCarryingSentry;
	bool m_bIsCarryingDisp;
	bool m_bIsCarryingTeleEnt;
	bool m_bIsCarryingObj;

	float m_fCarryTime;

	float m_fCheckNextCarrying;
	float m_fEquipHatTime;

	bool m_bHatEquipped;

	//stack<CTF2LoadoutAdded*> m_toApply;

	void *m_pVTable;
	void *m_pVTable_Attributes;

	float m_fUseBuffItemTime;

	CTF2Loadout *m_pHat;
	CTF2Loadout *m_pMisc;
	CTF2Loadout *m_pMelee;
	CTF2Loadout *m_pPrimary;
	CTF2Loadout *m_pSecondary;

	int m_iDesiredResistType;
	//dataStack<CTF2LoadoutAdded*> m_LoadoutsApplyAttributes;
};

class CBotFF : public CBotFortress
{
public:

	CBotFF() { CBotFortress(); }

	void modThink ();

	bool isEnemy ( edict_t *pEdict,bool bCheckWeapons = true );

	bool isTF () { return true; }

};

#endif
