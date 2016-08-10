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

#include "bot.h"
#include "bot_client.h"
#include "bot_waypoint_locations.h"
#include "bot_accessclient.h"
#include "bot_globals.h"
#include "bot_waypoint.h"
#include "ndebugoverlay.h"
// autowaypoint
#include "bot_getprop.h"
#include "bot_waypoint_locations.h"
//#include "bot_hooks.h"
#include "in_buttons.h"
#include "../extension.h"

// setup static client array
CClient CClients::m_Clients[MAX_PLAYERS];

void CClient::Init()
{
	m_fMonitorHighFiveTime = 0;
	m_pPlayer = NULL;
	m_szSteamID = NULL;
}

void CClient::SetEdict(edict_t *pPlayer)
{
	m_pPlayer = pPlayer;
	m_pPI = playerinfomanager->GetPlayerInfo(pPlayer);
}

void CClient::AutoEventWaypoint(int iType, float fRadius, bool bAtOtherOrigin, int iTeam, Vector vOrigin, bool bIgnoreTeam, bool bAutoType)
{
	m_iAutoEventWaypoint = iType;
	m_fAutoEventWaypointRadius = fRadius;

	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	m_bAutoEventWaypointAutoType = bAutoType;

	if (bAtOtherOrigin)
	{
		m_vAutoEventWaypointOrigin = vOrigin;
	}
	else
	{
		m_vAutoEventWaypointOrigin = GetOrigin();
		iTeam = CClassInterface::GetTeam(m_pPlayer);
	}

	if (bIgnoreTeam)
		m_iAutoEventWaypointTeam = 0;
	else
	{
		pMod->GetTeamOnlyWaypointFlags(iTeam, &m_iAutoEventWaypointTeamOn, &m_iAutoEventWaypointTeamOff);
		m_iAutoEventWaypointTeam = iTeam;
	}
}

void CClient::TeleportTo(Vector vOrigin)
{
	m_bIsTeleporting = true;
	m_fTeleportTime = engine->Time() + 0.1f;

	Vector *v_origin = CClassInterface::GetOrigin(m_pPlayer);

	byte *pMoveType = CClassInterface::GetMoveTypePointer(m_pPlayer);
	int *pPlayerFlags = CClassInterface::GetPlayerFlagsPointer(m_pPlayer);

	*pMoveType &= ~15;
	*pMoveType |= MOVETYPE_FLYGRAVITY;

	*pPlayerFlags &= ~FL_ONGROUND;
	*pPlayerFlags |= FL_FLY;

	*v_origin = vOrigin;
}

class CBotFunc_HighFiveSearch : public IBotFunction
{
public:
	CBotFunc_HighFiveSearch(edict_t *pPlayer, int iTeam)
	{
		m_pPlayer = pPlayer;
		m_iTeam = iTeam;
		m_pNearestBot = NULL;
		m_fNearestDist = 0;
	}

	void Execute(CBot *pBot)
	{
		if ((pBot->GetEdict() != m_pPlayer) && (pBot->GetTeam() == m_iTeam) && pBot->IsVisible(m_pPlayer))
		{
			float fDist = pBot->DistanceFrom(m_pPlayer);

			if (!m_pNearestBot || (fDist < m_fNearestDist))
			{
				m_pNearestBot = pBot;
				m_fNearestDist = fDist;
			}
		}
	}

	CBot *GetNearestBot()
	{
		return m_pNearestBot;
	}

private:
	edict_t *m_pPlayer;
	int m_iTeam;
	CBot *m_pNearestBot;
	float m_fNearestDist;
};

// called each frame
void CClient::Think()
{
	if ((m_pPlayer != NULL) && (m_pPI == NULL))
	{
		m_pPI = playerinfomanager->GetPlayerInfo(m_pPlayer);
	}

	if (CBotGlobals::IsMod(MOD_TF2))
	{
		if ((m_fMonitorHighFiveTime < engine->Time()) && (m_pPlayer != NULL) && (m_pPI != NULL) && m_pPI->IsConnected() &&
			!m_pPI->IsDead() && m_pPI->IsPlayer() && !m_pPI->IsObserver() &&
			CClassInterface::GetTF2HighFiveReady(m_pPlayer))
		{
			m_fMonitorHighFiveTime = engine->Time() + 0.25f;

			if (CClassInterface::GetHighFivePartner(m_pPlayer) == NULL)
			{
				// wanting high five partner
				// search for bots nearby who can see this player
				CBotFunc_HighFiveSearch *newFunc = new CBotFunc_HighFiveSearch(m_pPlayer, CClassInterface::GetTeam(m_pPlayer));

				CBots::BotFunction(newFunc);

				CBot *pBot = newFunc->GetNearestBot();

				if (pBot != NULL)
				{
					((CBotTF2*)pBot)->HighFivePlayer(m_pPlayer, CClassInterface::GetTF2TauntYaw(m_pPlayer));
					m_fMonitorHighFiveTime = engine->Time() + 3.0f;
				}

				delete newFunc;
			}
		}
	}

	if (m_bIsTeleporting)
	{
		if (m_fTeleportTime < engine->Time())
		{
			m_bIsTeleporting = false;
			m_fTeleportTime = 0;
			//reset movetypes
			byte *pMoveType = CClassInterface::GetMoveTypePointer(m_pPlayer);
			int *pPlayerFlags = CClassInterface::GetPlayerFlagsPointer(m_pPlayer);

			*pMoveType &= ~15;
			*pMoveType |= MOVETYPE_WALK;

			*pPlayerFlags &= ~FL_FLY;
			*pPlayerFlags |= FL_ONGROUND;
		}
	}
}

const char *CClient::GetName()
{
	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo(m_pPlayer);

	if (playerinfo)
		return playerinfo->GetName();

	return NULL;
}

void CClient::SetTeleportVector()
{
	m_vTeleportVector = GetOrigin();
	m_bTeleportVectorValid = true;
}

void CClient::ClientActive()
{
	// get steam id
	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo(m_pPlayer);

	m_szSteamID = NULL;

	if (playerinfo)
	{
		// store steam id
		m_szSteamID = (char*)playerinfo->GetNetworkIDString();
	}
}
// this player joins with pPlayer edict
void CClient::ClientConnected(edict_t *pPlayer)
{
	Init();
	// set player edict
	SetEdict(pPlayer);
}

// this player disconnects
void CClient::ClientDisconnected()
{
	// is bot?
	CBot *pBot = CBots::GetBotPointer(m_pPlayer);

	if (pBot != NULL)
	{
		if (pBot->InUse())
		{
			// free bots memory and other stuff
			pBot->FreeAllMemory();
		}
	}

	Init();
}

bool CClient::IsUsed()
{
	return (m_pPlayer != NULL);
}

Vector CClient::GetOrigin()
{
	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo(m_pPlayer);

	if (playerinfo)
	{
		return  playerinfo->GetAbsOrigin() + Vector(0, 0, 32);
	}

	return CBotGlobals::EntityOrigin(m_pPlayer) + Vector(0, 0, 32);
}

void CClients::ClientActive(edict_t *pPlayer)
{
	CClient *pClient = &m_Clients[SlotOfEdict(pPlayer)];

	pClient->ClientActive();
}

CClient *CClients::ClientConnected(edict_t *pPlayer)
{
	CClient *pClient = &m_Clients[SlotOfEdict(pPlayer)];

	pClient->ClientConnected(pPlayer);

	return pClient;
}

void CClients::Init(edict_t *pPlayer)
{
	m_Clients[SlotOfEdict(pPlayer)].Init();
}

void CClients::ClientDisconnected(edict_t *pPlayer)
{
	CClient *pClient = &m_Clients[SlotOfEdict(pPlayer)];

	pClient->ClientDisconnected();
}

void CClients::ClientThink()
{
	static CClient *pClient;

	edict_t *pPlayer;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pClient = &m_Clients[i];

		if (!pClient->IsUsed())
			continue;

		pPlayer = pClient->GetPlayer();

		if (pPlayer && pPlayer->GetIServerEntity())
			pClient->Think();
	}
}

CClient *CClients::FindClientBySteamID(char *szSteamID)
{
	CClient *pClient;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pClient = &m_Clients[i];

		if (pClient->IsUsed())
		{
			if (FStrEq(pClient->GetSteamID(), szSteamID))
				return pClient;
		}
	}

	return NULL;
}

const char *g_szDebugTags[15] =
{
	"GAME_EVENT",
	"NAV",
	"SPEED",
	"VIS",
	"TASK",
	"BUTTONS",
	"USERCMD",
	"UTIL",
	"PROFILE",
	"EDICTS",
	"THINK",
	"LOOK",
	"HUD",
	"AIM",
	"CHAT"
};

// get index in array
int CClients::SlotOfEdict(edict_t *pPlayer)
{
	return ENTINDEX(pPlayer) - 1;
}
