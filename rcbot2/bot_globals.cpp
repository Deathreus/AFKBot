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

#ifndef __linux__
// for file stuff
#include <windows.h>
#define WIN32_LEAN_AND_MEAN

#ifdef GetClassName
#undef GetClassName
#endif

#include <conio.h>

#endif

#include "bot.h"
#include "bot_globals.h"
#include "bot_strings.h"
#include "bot_waypoint_locations.h"
#include "bot_getprop.h"
#include "bot_weapons.h"

#include "ndebugoverlay.h"

#ifndef __linux__
#include <direct.h> // for mkdir
#else
#include <fcntl.h>
#include <sys/stat.h>
#endif

///////////
trace_t CBotGlobals::m_TraceResult;
char * CBotGlobals::m_szGameFolder = NULL;
char * CBotGlobals::m_szModFolder = NULL;
eModId CBotGlobals::m_iCurrentMod = MOD_UNSUPPORTED;
CBotMod *CBotGlobals::m_pCurrentMod = NULL;
bool CBotGlobals::m_bMapRunning = false;
int CBotGlobals::m_iMaxClients = 0;
int CBotGlobals::m_iEventVersion = 1;
int CBotGlobals::m_iWaypointDisplayType = 0;
char CBotGlobals::m_szMapName[MAX_MAP_STRING_LEN];
bool CBotGlobals::m_bTeamplay = false;
char *CBotGlobals::m_szRCBotFolder = NULL;

///////////

extern IVDebugOverlay *debugoverlay;

class CTraceFilterVis : public CTraceFilter
{
public:
	CTraceFilterVis(edict_t *pPlayer, edict_t *pHit = NULL)
	{
		m_pPlayer = pPlayer;
		m_pHit = pHit;
	}

	virtual bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
	{
		if (m_pPlayer && (pServerEntity == (IHandleEntity*)m_pPlayer->GetIServerEntity()))
			return false;

		if (m_pHit && (pServerEntity == (IHandleEntity*)m_pHit->GetIServerEntity()))
			return false;

		return true;
	}

	virtual TraceType_t	GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}
private:
	edict_t *m_pPlayer;
	edict_t *m_pHit;
};

CBotGlobals::CBotGlobals()
{
	Init();
}

void CBotGlobals::Init()
{
	m_iCurrentMod = MOD_UNSUPPORTED;
	m_szModFolder[0] = 0;
	m_szGameFolder[0] = 0;
}

bool CBotGlobals::IsAlivePlayer(edict_t *pEntity)
{
	return pEntity && ENTINDEX(pEntity) && (ENTINDEX(pEntity) <= gpGlobals->maxClients) && (EntityIsAlive(pEntity));
}

//new map
void CBotGlobals::SetMapName(const char *szMapName)
{
	strncpy(m_szMapName, szMapName, MAX_MAP_STRING_LEN - 1);
	m_szMapName[MAX_MAP_STRING_LEN - 1] = 0;
}

char *CBotGlobals::GetMapName()
{
	return m_szMapName;
}

bool CBotGlobals::IsCurrentMod(eModId modid)
{
	return m_pCurrentMod->GetModId() == modid;
}

int CBotGlobals::NumPlayersOnTeam(int iTeam, bool bAliveOnly)
{
	int i = 0;
	int num = 0;
	edict_t *pEdict;

	for (i = 1; i <= CBotGlobals::NumClients(); i++)
	{
		pEdict = INDEXENT(i);

		if (CBotGlobals::EntityIsValid(pEdict))
		{
			if (CClassInterface::GetTeam(pEdict) == iTeam)
			{
				if (bAliveOnly)
				{
					if (CBotGlobals::EntityIsAlive(pEdict))
						num++;
				}
				else
					num++;
			}
		}
	}
	return num;
}

void CBotGlobals::ReadRCBotFolder()
{
	char folder[256] = "\0";
	const char *szRCBotFolder = "\0";

	snprintf(folder, sizeof(folder), "%s/addons/sourcemod/configs/afkbot", CBotGlobals::ModFolder());

	szRCBotFolder = CStrings::GetString(folder);
	CBotGlobals::BotMessage(NULL, 0, "RCBot Folder -> trying %s", szRCBotFolder);

	if (str_is_empty(szRCBotFolder))
	{
		CBotGlobals::BotMessage(NULL, 0, "RCBot Folder -> not found ...");
	}

	m_szRCBotFolder = CStrings::GetString(szRCBotFolder);
}

float CBotGlobals::GrenadeWillLand(Vector vOrigin, Vector vEnemy, float fProjSpeed, float fGrenadePrimeTime, float *fAngle)
{
	static float g;
	extern ConVar *sv_gravity;
	Vector v_comp = vEnemy - vOrigin;
	float fDistance = v_comp.Length();

	v_comp = v_comp / fDistance;

	g = sv_gravity->GetFloat();

	if (fAngle == NULL)
	{
		return false;
	}
	else
	{
		// use angle -- work out time
		// work out angle
		float vhorz;
		float vvert;

		SinCos(DEG2RAD(*fAngle), &vvert, &vhorz);

		vhorz *= fProjSpeed;
		vvert *= fProjSpeed;

		float t = fDistance / vhorz;

		// within one second of going off
		if (fabs(t - fGrenadePrimeTime) < 1.0f)
		{
			float ffinaly = vOrigin.z + (vvert*t) - ((g*0.5)*(t*t));

			return (fabs(ffinaly - vEnemy.z) < BLAST_RADIUS); // ok why not
		}
	}

	return false;
}

// TO DO :: put in CClients ?
edict_t *CBotGlobals::FindPlayerByTruncName(const char *name)
// find a player by a truncated name "name".
// e.g. name = "Jo" might find a player called "John"
{
	edict_t *pent = NULL;
	IPlayerInfo *pInfo;
	int i;

	for (i = 1; i <= MaxClients(); i++)
	{
		pent = INDEXENT(i);

		if (pent && CBotGlobals::IsNetworkable(pent))
		{
			int length = strlen(name);

			char arg_lwr[128];
			char pent_lwr[128];

			strcpy(arg_lwr, name);

			pInfo = playerinfomanager->GetPlayerInfo(pent);

			if (pInfo == NULL)
				continue;

			strcpy(pent_lwr, pInfo->GetName());

			__strlow(arg_lwr);
			__strlow(pent_lwr);

			if (strncmp(arg_lwr, pent_lwr, length) == 0)
			{
				return pent;
			}
		}
	}

	return NULL;
}

class CTraceFilterHitAllExceptPlayers : public CTraceFilter
{
public:
	virtual bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
	{
		return pServerEntity->GetRefEHandle().GetEntryIndex() <= gpGlobals->maxClients;
	}
};

//-----------------------------------------------------------------------------
// traceline methods
//-----------------------------------------------------------------------------
class CTraceFilterSimple : public CTraceFilter
{
public:

	CTraceFilterSimple(const IHandleEntity *passentity1, const IHandleEntity *passentity2, int collisionGroup)
	{
		m_pPassEnt1 = passentity1;

		if (passentity2)
			m_pPassEnt2 = passentity2;

		m_collisionGroup = collisionGroup;
	}
	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		if (m_pPassEnt1 == pHandleEntity)
			return false;
		if (m_pPassEnt2 == pHandleEntity)
			return false;
#if defined(_DEBUG) && !defined(__linux__)
		if ( CClients::clientsDebugging(BOT_DEBUG_VIS) )
		{
			static edict_t *edict;

			edict = INDEXENT(pHandleEntity->GetRefEHandle().GetEntryIndex());

			debugoverlay->AddTextOverlayRGB(CBotGlobals::entityOrigin(edict),0,2.0f,255,100,100,200,"Traceline hit %s",edict->GetClassName());
		}
#endif
		return true;
	}
	//virtual void SetPassEntity( const IHandleEntity *pPassEntity ) { m_pPassEnt = pPassEntity; }
	//virtual void SetCollisionGroup( int iCollisionGroup ) { m_collisionGroup = iCollisionGroup; }

	//const IHandleEntity *GetPassEntity( void ){ return m_pPassEnt;}

private:
	const IHandleEntity *m_pPassEnt1;
	const IHandleEntity *m_pPassEnt2;
	int m_collisionGroup;
};

bool CBotGlobals::CheckOpensLater(Vector vSrc, Vector vDest)
{
	CTraceFilterSimple traceFilter(NULL, NULL, MASK_PLAYERSOLID);

	TraceLine(vSrc, vDest, MASK_PLAYERSOLID, &traceFilter);

	return (TraceVisible(NULL));
}


bool CBotGlobals::IsVisibleHitAllExceptPlayer(edict_t *pPlayer, Vector vSrc, Vector vDest, edict_t *pDest)
{
	const IHandleEntity *ignore = pPlayer->GetIServerEntity();

	CTraceFilterSimple traceFilter(ignore, ((pDest == NULL) ? NULL : pDest->GetIServerEntity()), MASK_ALL);

	TraceLine(vSrc, vDest, MASK_SHOT | MASK_VISIBLE, &traceFilter);

	return (TraceVisible(pDest));
}

bool CBotGlobals::IsVisible(edict_t *pPlayer, Vector vSrc, Vector vDest)
{
	CTraceFilterWorldAndPropsOnly filter;

	TraceLine(vSrc, vDest, MASK_SOLID_BRUSHONLY | CONTENTS_OPAQUE, &filter);

	return (TraceVisible(NULL));
}

bool CBotGlobals::IsVisible(edict_t *pPlayer, Vector vSrc, edict_t *pDest)
{
	//CTraceFilterWorldAndPropsOnly filter;//	CTraceFilterHitAll filter;

	CTraceFilterWorldAndPropsOnly filter;

	TraceLine(vSrc, EntityOrigin(pDest), MASK_SOLID_BRUSHONLY | CONTENTS_OPAQUE, &filter);

	return (TraceVisible(pDest));
}

bool CBotGlobals::IsShotVisible(edict_t *pPlayer, Vector vSrc, Vector vDest, edict_t *pDest)
{
	//CTraceFilterWorldAndPropsOnly filter;//	CTraceFilterHitAll filter;

	CTraceFilterVis filter = CTraceFilterVis(pPlayer, pDest);

	TraceLine(vSrc, vDest, MASK_SHOT, &filter);

	return (TraceVisible(pDest));
}

bool CBotGlobals::IsVisible(Vector vSrc, Vector vDest)
{
	CTraceFilterWorldAndPropsOnly filter;

	TraceLine(vSrc, vDest, MASK_SOLID_BRUSHONLY | CONTENTS_OPAQUE, &filter);

	return TraceVisible(NULL);
}

void CBotGlobals::TraceLine(Vector vSrc, Vector vDest, unsigned int mask, ITraceFilter *pFilter)
{
	Ray_t ray;
	memset(&m_TraceResult, 0, sizeof(trace_t));
	ray.Init(vSrc, vDest);
	enginetrace->TraceRay(ray, mask, pFilter, &m_TraceResult);
}

float CBotGlobals::QuickTraceline(edict_t *pIgnore, Vector vSrc, Vector vDest)
{
	CTraceFilterVis filter = CTraceFilterVis(pIgnore);

	Ray_t ray;
	memset(&m_TraceResult, 0, sizeof(trace_t));
	ray.Init(vSrc, vDest);
	enginetrace->TraceRay(ray, MASK_NPCSOLID_BRUSHONLY, &filter, &m_TraceResult);
	return m_TraceResult.fraction;
}

float CBotGlobals::DotProductFromOrigin(edict_t *pEnemy, Vector pOrigin)
{
	static Vector vecLOS;
	static float flDot;
	IPlayerInfo *p;

	Vector vForward;
	QAngle eyes;

	p = playerinfomanager->GetPlayerInfo(pEnemy);

	if (!p)
		return 0;

	eyes = p->GetAbsAngles();

	// in fov? Check angle to edict
	AngleVectors(eyes, &vForward);

	vecLOS = pOrigin - CBotGlobals::EntityOrigin(pEnemy);
	vecLOS = vecLOS / vecLOS.Length();

	flDot = DotProduct(vecLOS, vForward);

	return flDot;
}


float CBotGlobals::DotProductFromOrigin(Vector vPlayer, Vector vFacing, QAngle eyes)
{
	static Vector vecLOS;
	static float flDot;

	Vector vForward;

	// in fov? Check angle to edict
	AngleVectors(eyes, &vForward);

	vecLOS = vFacing - vPlayer;
	vecLOS = vecLOS / vecLOS.Length();

	flDot = DotProduct(vecLOS, vForward);

	return flDot;
}

bool CBotGlobals::TraceVisible(edict_t *pEnt)
{
	return (m_TraceResult.fraction >= 1.0) || (m_TraceResult.m_pEnt && pEnt && (m_TraceResult.m_pEnt == pEnt->GetUnknown()->GetBaseEntity()));
}

void CBotGlobals::FreeMemory()
{
	m_pCommands->FreeMemory();
}

bool CBotGlobals::InitModFolder() {
	char szGameFolder[512];
	engine->GetGameDir(szGameFolder, 512);

	int iLength = strlen(CStrings::GetString(szGameFolder));
	int pos = iLength - 1;

	while ((pos > 0) && (szGameFolder[pos] != '\\') && (szGameFolder[pos] != '/')) {
		pos--;
	}
	pos++;

	m_szModFolder = CStrings::GetString(&szGameFolder[pos]);
	return true;
}

bool CBotGlobals::GameStart()
{
	char szGameFolder[512];
	engine->GetGameDir(szGameFolder, 512);
	char szSteamFolder[512];

	V_GetCurrentDirectory(szSteamFolder, 512);

	int iLength = strlen(szSteamFolder);

	int pos = iLength - 1;

	while ((pos > 0) && (szSteamFolder[pos] != '\\') && (szSteamFolder[pos] != '/'))
	{
		pos--;
	}
	pos++;

	m_szGameFolder = CStrings::GetString(&szSteamFolder[pos]);

	iLength = strlen(CStrings::GetString(szGameFolder));

	pos = iLength - 1;

	while ((pos > 0) && (szGameFolder[pos] != '\\') && (szGameFolder[pos] != '/'))
	{
		pos--;
	}
	pos++;

	m_szModFolder = CStrings::GetString(&szGameFolder[pos]);

	CBotMods::ReadMods();

	m_pCurrentMod = CBotMods::GetMod(m_szModFolder, m_szGameFolder);

	if (m_pCurrentMod != NULL)
	{
		m_iCurrentMod = m_pCurrentMod->GetModId();

		m_pCurrentMod->InitMod();

		CBots::Init();

		return true;
	}
	else
	{
		Msg("[BOT ERROR] Mod not found. Please edit the bot_mods.ini in the bot config folder\nsteamdir = %s\ngamedir = %s\n", m_szGameFolder, m_szModFolder);

		return false;
	}
}

void CBotGlobals::LevelInit()
{

}

int CBotGlobals::CountTeamMatesNearOrigin(Vector vOrigin, float fRange, int iTeam, edict_t *pIgnore)
{
	int iCount = 0;
	IPlayerInfo *p;

	for (int i = 1; i <= CBotGlobals::MaxClients(); i++)
	{
		edict_t *pEdict = INDEXENT(i);

		if (pEdict->IsFree())
			continue;

		if (pEdict == pIgnore)
			continue;

		p = playerinfomanager->GetPlayerInfo(pEdict);

		if (!p || !p->IsConnected() || p->IsDead() || p->IsObserver() || !p->IsPlayer())
			continue;

		if (CClassInterface::GetTeam(pEdict) == iTeam)
		{
			Vector vPlayer = EntityOrigin(pEdict);

			if ((vOrigin - vPlayer).Length() <= fRange)
				iCount++;
		}
	}

	return iCount;
}

int CBotGlobals::NumClients()
{
	int iCount = 0;

	for (int i = 1; i <= CBotGlobals::MaxClients(); i++)
	{
		edict_t *pEdict = INDEXENT(i);

		if (pEdict)
		{
			if (engine->GetPlayerUserId(pEdict) > 0)
				iCount++;
		}
	}

	return iCount;
}

bool CBotGlobals::EntityIsAlive(edict_t *pEntity)
{
	static short int index;

	index = ENTINDEX(pEntity);

	if (index && (index <= gpGlobals->maxClients))
	{
		IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pEntity);

		if (!p)
			return false;

		return (!p->IsDead() && (p->GetHealth() > 0));
	}

	return (pEntity->GetIServerEntity() && pEntity->GetClassName() && *pEntity->GetClassName());
}

edict_t *CBotGlobals::PlayerByUserId(int iUserId)
{
	for (int i = 1; i <= MaxClients(); i++)
	{
		edict_t *pEdict = INDEXENT(i);

		if (pEdict)
		{
			if (engine->GetPlayerUserId(pEdict) == iUserId)
				return pEdict;
		}
	}

	return NULL;
}

int CBotGlobals::GetTeam(edict_t *pEntity)
{
	IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pEntity);
	return p->GetTeamIndex();
}

bool CBotGlobals::IsNetworkable(edict_t *pEntity)
{
	static IServerEntity *pServerEnt;

	pServerEnt = pEntity->GetIServerEntity();

	return (pServerEnt && (pServerEnt->GetNetworkable() != NULL));
}

void CBotGlobals::ServerSay(char *fmt, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, fmt);

	strcpy(string, "say \"");

	vsprintf(&string[5], fmt, argptr);

	va_end(argptr);

	strcat(string, "\"");

	engine->ServerCommand(string);
}

// TO DO :: put into CClient
bool CBotGlobals::SetWaypointDisplayType(int iType)
{
	if ((iType >= 0) && (iType <= 1))
	{
		m_iWaypointDisplayType = iType;
		return true;
	}

	return false;
}
// work on this
bool CBotGlobals::WalkableFromTo(edict_t *pPlayer, Vector v_src, Vector v_dest)
{
	extern ConVar bot_wptplace_width;
	CTraceFilterVis filter = CTraceFilterVis(pPlayer);
	float fDistance = sqrt((v_dest - v_src).LengthSqr());
	CClient *pClient = CClients::Get(pPlayer);
	Vector vcross = v_dest - v_src;
	Vector vleftsrc, vleftdest, vrightsrc, vrightdest;
	float fWidth = bot_wptplace_width.GetFloat();

	if (v_dest == v_src)
		return true;

	// minimum
	if (fWidth < 2.0f)
		fWidth = 2.0f;

	if (pClient->AutoWaypointOn())
		fWidth = 4.0f;

	vcross = vcross / vcross.Length();
	vcross = vcross.Cross(Vector(0, 0, 1));
	vcross = vcross * (fWidth*0.5f);

	vleftsrc = v_src - vcross;
	vrightsrc = v_src + vcross;

	vleftdest = v_dest - vcross;
	vrightdest = v_dest + vcross;

	if (fDistance > CWaypointLocations::REACHABLE_RANGE)
		return false;

	//if ( !CBotGlobals::isVisible(v_src,v_dest) )
	//	return false;

	// can swim there?
	if ((enginetrace->GetPointContents(v_src) == CONTENTS_WATER) &&
		(enginetrace->GetPointContents(v_dest) == CONTENTS_WATER))
	{
		return true;
	}

	// find the ground
	CBotGlobals::TraceLine(v_src, v_src - Vector(0, 0, 256.0), MASK_NPCSOLID_BRUSHONLY, &filter);
#ifndef __linux__
	debugoverlay->AddLineOverlay(v_src, v_src - Vector(0, 0, 256.0), 255, 0, 255, false, 3);
#endif
	Vector v_ground_src = CBotGlobals::GetTraceResult()->endpos + Vector(0, 0, 1);

	CBotGlobals::TraceLine(v_dest, v_dest - Vector(0, 0, 256.0), MASK_NPCSOLID_BRUSHONLY, &filter);
#ifndef __linux__
	debugoverlay->AddLineOverlay(v_dest, v_dest - Vector(0, 0, 256.0), 255, 255, 0, false, 3);
#endif
	Vector v_ground_dest = CBotGlobals::GetTraceResult()->endpos + Vector(0, 0, 1);

	if (!CBotGlobals::IsVisible(pPlayer, v_ground_src, v_ground_dest))
	{
#ifndef __linux__
		debugoverlay->AddLineOverlay(v_ground_src, v_ground_dest, 0, 255, 255, false, 3);
#endif
		trace_t *tr = CBotGlobals::GetTraceResult();

		// no slope there
		if (tr->endpos.z > v_src.z)
		{
#ifndef __linux__
			debugoverlay->AddTextOverlay((v_ground_src + v_ground_dest) / 2, 0, 3, "ground fail");
#endif

			CBotGlobals::TraceLine(tr->endpos, tr->endpos - Vector(0, 0, 45), MASK_NPCSOLID_BRUSHONLY, &filter);

			Vector v_jsrc = tr->endpos;

#ifndef __linux__
			debugoverlay->AddLineOverlay(v_jsrc, v_jsrc - Vector(0, 0, 45), 255, 255, 255, false, 3);
#endif
			// can't jump there
			if (((v_jsrc.z - tr->endpos.z) + (v_dest.z - v_jsrc.z)) > 45.0f)
			{
				//if ( (tr->endpos.z > (v_src.z+45)) && (fDistance > 64.0f) )
				//{
#ifndef __linux__
				debugoverlay->AddTextOverlay(tr->endpos, 0, 3, "jump fail");
#endif
				// check for slope or stairs
				Vector v_norm = v_dest - v_src;
				v_norm = v_norm / sqrt(v_norm.LengthSqr());

				for (float fDistCheck = 45.0f; fDistCheck < fDistance; fDistCheck += 45.0f)
				{
					Vector v_checkpoint = v_src + (v_norm * fDistCheck);

					// check jump height again
					CBotGlobals::TraceLine(v_checkpoint, v_checkpoint - Vector(0, 0, 45.0f), MASK_NPCSOLID_BRUSHONLY, &filter);

					if (CBotGlobals::TraceVisible(NULL))
					{
#ifndef __linux__
						debugoverlay->AddTextOverlay(tr->endpos, 0, 3, "step/jump fail");
#endif
						return false;
					}
				}
				//}
			}
		}
	}

	return CBotGlobals::IsVisible(pPlayer, vleftsrc, vleftdest) && CBotGlobals::IsVisible(pPlayer, vrightsrc, vrightdest);

	//return true;
}

bool CBotGlobals::BoundingBoxTouch2d(
	const Vector2D &a1, const Vector2D &a2,
	const Vector2D &bmins, const Vector2D &bmaxs)
{
	Vector2D amins = Vector2D(min(a1.x, a2.x), min(a1.y, a2.y));
	Vector2D amaxs = Vector2D(max(a1.x, a2.x), max(a1.y, a2.y));

	return (((bmins.x >= amins.x) && (bmins.y >= amins.y)) && ((bmins.x <= amaxs.x) && (bmins.y <= amaxs.y)) ||
		((bmaxs.x >= amins.x) && (bmaxs.y >= amins.y)) && ((bmaxs.x <= amaxs.x) && (bmaxs.y <= amaxs.y)));
}

bool CBotGlobals::BoundingBoxTouch3d(
	const Vector &a1, const Vector &a2,
	const Vector &bmins, const Vector &bmaxs)
{
	Vector amins = Vector(min(a1.x, a2.x), min(a1.y, a2.y), min(a1.z, a2.z));
	Vector amaxs = Vector(max(a1.x, a2.x), max(a1.y, a2.y), max(a1.z, a2.z));

	return (((bmins.x >= amins.x) && (bmins.y >= amins.y) && (bmins.z >= amins.z)) && ((bmins.x <= amaxs.x) && (bmins.y <= amaxs.y) && (bmins.z <= amaxs.z)) ||
		((bmaxs.x >= amins.x) && (bmaxs.y >= amins.y) && (bmaxs.z >= amins.z)) && ((bmaxs.x <= amaxs.x) && (bmaxs.y <= amaxs.y) && (bmaxs.z <= amaxs.z)));
}
bool CBotGlobals::OnOppositeSides2d(
	const Vector2D &amins, const Vector2D &amaxs,
	const Vector2D &bmins, const Vector2D &bmaxs)
{
	float g = (amaxs.x - amins.x) * (bmins.y - amins.y) -
		(amaxs.y - amins.y) * (bmins.x - amins.x);

	float h = (amaxs.x - amins.x) * (bmaxs.y - amins.y) -
		(amaxs.y - amins.y) * (bmaxs.x - amins.x);

	return (g * h) <= 0.0f;
}

bool CBotGlobals::OnOppositeSides3d(
	const Vector &amins, const Vector &amaxs,
	const Vector &bmins, const Vector &bmaxs)
{
	amins.Cross(bmins);
	amaxs.Cross(bmaxs);

	float g = (amaxs.x - amins.x) * (bmins.y - amins.y) * (bmins.z - amins.z) -
		(amaxs.z - amins.z) * (amaxs.y - amins.y) * (bmins.x - amins.x);

	float h = (amaxs.x - amins.x) * (bmaxs.y - amins.y) * (bmaxs.z - amins.z) -
		(amaxs.z - amins.z) * (amaxs.y - amins.y) * (bmaxs.x - amins.x);

	return (g * h) <= 0.0f;
}

bool CBotGlobals::LinesTouching2d(
	const Vector2D &amins, const Vector2D &amaxs,
	const Vector2D &bmins, const Vector2D &bmaxs)
{
	return OnOppositeSides2d(amins, amaxs, bmins, bmaxs) && BoundingBoxTouch2d(amins, amaxs, bmins, bmaxs);
}

bool CBotGlobals::LinesTouching3d(
	const Vector &amins, const Vector &amaxs,
	const Vector &bmins, const Vector &bmaxs)
{
	return OnOppositeSides3d(amins, amaxs, bmins, bmaxs) && BoundingBoxTouch3d(amins, amaxs, bmins, bmaxs);
}

void CBotGlobals::BotMessage(edict_t *pEntity, int iErr, char *fmt, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, fmt);
	vsprintf(string, fmt, argptr);
	va_end(argptr);

	const char *bot_tag = BOT_TAG;
	int len = strlen(string);
	int taglen = strlen(BOT_TAG);
	// add tag -- push tag into string
	for (int i = len + taglen; i >= taglen; i--)
		string[i] = string[i - taglen];

	string[len + taglen + 1] = 0;

	for (int i = 0; i < taglen; i++)
		string[i] = bot_tag[i];

	strcat(string, "\n");

	if (pEntity)
	{
		engine->ClientPrintf(pEntity, string);
	}
	else
	{
		if (iErr)
		{
			Warning(string);
		}
		else
			Msg(string);
	}
}

bool CBotGlobals::MakeFolders(char *szFile)
{
#ifndef __linux__
	char *delimiter = "\\";
#else
	char *delimiter = "/";
#endif

	char szFolderName[1024];
	int folderNameSize = 0;
	szFolderName[0] = 0;

	int iLen = strlen(szFile);

	int i = 0;

	while (i < iLen)
	{
		while ((i < iLen) && (szFile[i] != *delimiter))
		{
			szFolderName[folderNameSize++] = szFile[i];
			i++;
		}

		if (i == iLen)
			return true;

		i++;
		szFolderName[folderNameSize++] = *delimiter;//next
		szFolderName[folderNameSize] = 0;

#ifndef __linux__
		mkdir(szFolderName);
#else
		if ( mkdir(szFolderName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0 ) {
			BotMessage(NULL,0,"Trying to create folder '%s' successful",szFolderName);
		} else {
			if (!str_is_empty(szFolderName)) {
				BotMessage(NULL,0,"Folder '%s' already exists", szFolderName);
			} else {
				BotMessage(NULL,0,"Trying to create folder '%s' failed",szFolderName);
			}
		}
#endif   
	}

	return true;
}

void CBotGlobals::AddDirectoryDelimiter(char *szString)
{
#ifndef __linux__
	strcat(szString, "\\");
#else
	strcat(szString,"/");
#endif
}

bool CBotGlobals::IsBreakableOpen(edict_t *pBreakable)
{
	return ((CClassInterface::GetEffects(pBreakable) & EF_NODRAW) == EF_NODRAW);
}

Vector CBotGlobals::GetVelocity(edict_t *pPlayer)
{
	CClient *pClient = CClients::Get(pPlayer);

	if (pClient)
		return pClient->GetVelocity();

	return Vector(0, 0, 0);
}

FILE *CBotGlobals::OpenFile(char *szFile, char *szMode)
{
	FILE *fp = fopen(szFile, szMode);

	if (fp == NULL)
	{
		BotMessage(NULL, 0, "file not found/opening error '%s' mode %s", szFile, szMode);

		MakeFolders(szFile);

		// try again
		fp = fopen(szFile, szMode);

		if (fp == NULL)
			BotMessage(NULL, 0, "failed to make folders for %s", szFile);
	}

	return fp;
}

void CBotGlobals::BuildFileName(char *szOutput, const char *szFile, const char *szFolder, const char *szExtension, bool bModDependent)
{
	if (m_szRCBotFolder == NULL)
		strcat(szOutput, BOT_FOLDER);
	else
		strcpy(szOutput, m_szRCBotFolder);

	if ((szOutput[strlen(szOutput) - 1] != '\\') && (szOutput[strlen(szOutput) - 1] != '/'))
		AddDirectoryDelimiter(szOutput);

	if (szFolder)
	{
		strcat(szOutput, szFolder);
		AddDirectoryDelimiter(szOutput);
	}

	if (bModDependent)
	{
		strcat(szOutput, CBotGlobals::GameFolder());
		AddDirectoryDelimiter(szOutput);
		strcat(szOutput, CBotGlobals::ModFolder());
		AddDirectoryDelimiter(szOutput);
	}

	strcat(szOutput, szFile);

	if (szExtension)
	{
		strcat(szOutput, ".");
		strcat(szOutput, szExtension);
	}
}

QAngle CBotGlobals::PlayerAngles(edict_t *pPlayer)
{
	return gameclients->GetPlayerState(pPlayer)->v_angle;
}

QAngle CBotGlobals::EntityEyeAngles(edict_t *pEdict)
{
	return playerinfomanager->GetPlayerInfo(pEdict)->GetAbsAngles();
}

void CBotGlobals::FixFloatAngle(float *fAngle)
{
	if (*fAngle > 180)
	{
		*fAngle = *fAngle - 360;
	}
	else if (*fAngle < -180)
	{
		*fAngle = *fAngle + 360;
	}
}

void CBotGlobals::FixFloatDegrees360(float *pFloat)
{
	if (*pFloat > 360)
		*pFloat -= 360;
	else if (*pFloat < 0)
		*pFloat += 360;
}

void CBotGlobals::NormalizeAngle(QAngle &qAngle)
{
	while (qAngle.x > 89.0f)
	{
		qAngle.x -= 360.0f;
	}
	while (qAngle.x < -89.0f)
	{
		qAngle.x += 360.0f;
	}
	while (qAngle.y > 180.0f)
	{
		qAngle.y -= 360.0f;
	}
	while (qAngle.y < -180.0f)
	{
		qAngle.y += 360.0f;
	}
}

float CBotGlobals::YawAngleFromEdict(edict_t *pEntity, Vector vOrigin)
{
	float fAngle;
	float fYaw;
	QAngle qBotAngles = PlayerAngles(pEntity);
	Vector vPlayerOrigin = playerinfomanager->GetPlayerInfo(pEntity)->GetAbsOrigin();
	QAngle qAngles;
	Vector vAngles;

	vAngles = vOrigin - vPlayerOrigin;

	VectorAngles(vAngles / vAngles.Length(), qAngles);

	fYaw = qAngles.y;
	CBotGlobals::FixFloatAngle(&fYaw);

	fAngle = qBotAngles.y - fYaw;

	CBotGlobals::FixFloatAngle(&fAngle);

	return fAngle;
}

void CBotGlobals::TeleportPlayer(edict_t *pPlayer, Vector v_dest)
{
	CClient *pClient = CClients::Get(pPlayer);

	if (pClient)
		pClient->TeleportTo(v_dest);
}
