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

#include "CRecipientFilter.h"
#include "CHelpers.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CRecipientFilter::CRecipientFilter()
{
	Reset();
}

CRecipientFilter::~CRecipientFilter()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : src - 
//-----------------------------------------------------------------------------
void CRecipientFilter::CopyFrom(const CRecipientFilter& src)
{
	m_bReliable = src.IsReliable();
	m_bInitMessage = src.IsInitMessage();

	int c = src.GetRecipientCount();
	for (int i = 0; i < c; ++i)
	{
		m_Recipients.AddToTail(src.GetRecipientIndex(i));
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRecipientFilter::Reset(void)
{
	m_bReliable = false;
	m_bInitMessage = false;
	m_Recipients.RemoveAll();
}

void CRecipientFilter::MakeReliable(void)
{
	m_bReliable = true;
}

bool CRecipientFilter::IsReliable(void) const
{
	return m_bReliable;
}

int CRecipientFilter::GetRecipientCount(void) const
{
	return m_Recipients.Size();
}

int	CRecipientFilter::GetRecipientIndex(int slot) const
{
	if (slot < 0 || slot >= GetRecipientCount())
		return -1;

	return m_Recipients[slot];
}

void CRecipientFilter::AddAllPlayers(void)
{
	m_Recipients.RemoveAll();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CPlayer *pPlayer = pHelpers->UTIL_PlayerByIndex(i);
		if (!pPlayer)
		{
			AddRecipient(pPlayer);
			continue;
		}
	}
}

void CRecipientFilter::AddRecipient(CPlayer *player)
{
	Assert(player);

	int index = player->entindex();

	// Already in list
	if (m_Recipients.Find(index) != m_Recipients.InvalidIndex())
		return;

	m_Recipients.AddToTail(index);
}

void CRecipientFilter::AddRecipientByPlayerIndex(int playerindex)
{
	Assert(playerindex >= 1 && playerindex <= ABSOLUTE_PLAYER_LIMIT);

	if (m_Recipients.Find(playerindex) != m_Recipients.InvalidIndex())
	{
		m_Recipients.AddToTail(playerindex);
	}
}

void CRecipientFilter::RemoveAllRecipients(void)
{
	m_Recipients.RemoveAll();
}

void CRecipientFilter::RemoveRecipient(CPlayer *player)
{
	Assert(player);
	if (player)
	{
		int index = player->entindex();

		// Remove it if it's in the list
		m_Recipients.FindAndRemove(index);
	}
}

void CRecipientFilter::RemoveRecipientByPlayerIndex(int playerindex)
{
	Assert(playerindex >= 1 && playerindex <= ABSOLUTE_PLAYER_LIMIT);

	m_Recipients.FindAndRemove(playerindex);
}

void CRecipientFilter::AddRecipientsByTeam(int iTeam /*CTeam *team*/)
{
	int i;
	//int c = team->GetNumPlayers();
	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CPlayer *player = pHelpers->UTIL_PlayerByIndex(i);
		if (!player)
			continue;

		if (player->GetTeamNumber() == iTeam)
		{
			AddRecipient(player);
		}
	}
}

void CRecipientFilter::RemoveRecipientsByTeam(int iTeam /*CTeam *team*/)
{
	int i;
	//int c = team->GetNumPlayers();
	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CPlayer *player = pHelpers->UTIL_PlayerByIndex(i);
		if (!player)
			continue;

		if (player->GetTeamNumber() == iTeam)
		{
			RemoveRecipient(player);
		}
	}
}

void CRecipientFilter::RemoveRecipientsNotOnTeam(int iTeam /*CTeam *team*/)
{
	int i;
	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CPlayer *player = pHelpers->UTIL_PlayerByIndex(i);
		if (!player)
			continue;

		if (player->GetTeamNumber() != iTeam)
		{
			RemoveRecipient(player);
		}
	}
}

void CRecipientFilter::AddPlayersFromBitMask(CBitVec< ABSOLUTE_PLAYER_LIMIT >& playerbits)
{
	int index = playerbits.FindNextSetBit(0);

	while (index > -1)
	{
		CPlayer *pPlayer = pHelpers->UTIL_PlayerByIndex(index + 1);
		if (pPlayer)
		{
			AddRecipient(pPlayer);
		}

		index = playerbits.FindNextSetBit(index + 1);
	}
}

void CRecipientFilter::RemovePlayersFromBitMask(CBitVec< ABSOLUTE_PLAYER_LIMIT >& playerbits)
{
	int index = playerbits.FindNextSetBit(0);

	while (index > -1)
	{
		CPlayer *pPlayer = pHelpers->UTIL_PlayerByIndex(index + 1);
		if (pPlayer)
		{
			RemoveRecipient(pPlayer);
		}

		index = playerbits.FindNextSetBit(index + 1);
	}
}

void CRecipientFilter::AddRecipientsByPVS(const Vector& origin)
{
	if (gpGlobals->maxClients == 1)
	{
		AddAllPlayers();
	}
	else
	{
		CBitVec< ABSOLUTE_PLAYER_LIMIT > playerbits;
		engine->Message_DetermineMulticastRecipients(false, origin, playerbits);
		AddPlayersFromBitMask(playerbits);
	}
}

void CRecipientFilter::RemoveRecipientsByPVS(const Vector& origin)
{
	if (gpGlobals->maxClients == 1)
	{
		m_Recipients.RemoveAll();
	}
	else
	{
		CBitVec< ABSOLUTE_PLAYER_LIMIT > playerbits;
		engine->Message_DetermineMulticastRecipients(false, origin, playerbits);
		RemovePlayersFromBitMask(playerbits);
	}
}



void CRecipientFilter::AddRecipientsByPAS(const Vector& origin)
{
	if (gpGlobals->maxClients == 1)
	{
		AddAllPlayers();
	}
	else
	{
		CBitVec< ABSOLUTE_PLAYER_LIMIT > playerbits;
		engine->Message_DetermineMulticastRecipients(true, origin, playerbits);
		AddPlayersFromBitMask(playerbits);
	}
}

bool CRecipientFilter::IsInitMessage(void) const
{
	return m_bInitMessage;
}

void CRecipientFilter::MakeInitMessage(void)
{
	m_bInitMessage = true;
}

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for all players on a given team 
//-----------------------------------------------------------------------------
CTeamRecipientFilter::CTeamRecipientFilter(int team, bool isReliable)
{
	if (isReliable)
		MakeReliable();

	RemoveAllRecipients();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CPlayer *pPlayer = pHelpers->UTIL_PlayerByIndex(i);

		if (!pPlayer)
		{
			continue;
		}

		if (pPlayer->GetTeamNumber() != team)
		{
			//If we're in the spectator team then we should be getting whatever messages the person I'm spectating gets.
			if (pPlayer->GetTeamNumber() == TEAM_SPECTATOR && (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE || pPlayer->GetObserverMode() == OBS_MODE_CHASE))
			{
				if (pPlayer->GetObserverTarget())
				{
					if (pPlayer->GetObserverTarget()->GetTeamNumber() != team)
						continue;
				}
			}
			else
			{
				continue;
			}
		}

		AddRecipient(pPlayer);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : origin - 
//			ATTN_NORM - 
//-----------------------------------------------------------------------------
void CPASAttenuationFilter::Filter(const Vector& origin, float attenuation /*= ATTN_NORM*/)
{
	// Don't crop for attenuation in single player
	if (gpGlobals->maxClients == 1)
		return;

	// CPASFilter adds them by pure PVS in constructor
	if (attenuation <= 0)
		return;

	// Now remove recipients that are outside sound radius
	float distance, maxAudible;
	Vector vecRelative;

	int c = GetRecipientCount();

	for (int i = c - 1; i >= 0; i--)
	{
		int index = GetRecipientIndex(i);

		CEntity *ent = CEntity::Instance(index);
		if (!ent || !ent->IsPlayer())
		{
			Assert(0);
			continue;
		}

		CPlayer *player = dynamic_cast<CPlayer *>(ent);
		if (!player)
		{
			Assert(0);
			continue;
		}

		Vector pos = player->EyePosition();
		VectorSubtract(pos, origin, vecRelative);
		distance = VectorLength(vecRelative);
		maxAudible = (2 * SOUND_NORMAL_CLIP_DIST) / attenuation;
		if (distance <= maxAudible)
			continue;

		RemoveRecipient(player);
	}
}
