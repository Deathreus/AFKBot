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

#ifndef _INCLUDE_CRECIPIENTFILTER_H_
#define _INCLUDE_CRECIPIENTFILTER_H_

#include "CPlayer.h"
#include <irecipientfilter.h>
#include "isoundemittersystembase.h"
#include "IEngineSound.h"

extern ISoundEmitterSystemBase *soundemitterbase;

//-----------------------------------------------------------------------------
// Purpose: A generic filter for determining whom to send message/sounds etc. to and
//  providing a bit of additional state information
//-----------------------------------------------------------------------------
class CRecipientFilter : public IRecipientFilter
{
public:
	CRecipientFilter();
	virtual 		~CRecipientFilter();

	virtual bool	IsReliable(void) const;
	virtual bool	IsInitMessage(void) const;

	virtual int		GetRecipientCount(void) const;
	virtual int		GetRecipientIndex(int slot) const;

public:
	void			CopyFrom(const CRecipientFilter& src);
	void			Reset(void);

	void			MakeInitMessage(void);
	void			MakeReliable(void);

	void			AddAllPlayers(void);
	void			AddRecipientsByPVS(const Vector& origin);
	void			RemoveRecipientsByPVS(const Vector& origin);
	void			AddRecipientsByPAS(const Vector& origin);
	void			AddRecipient(CPlayer *player);
	void			AddRecipientByPlayerIndex(int iTeam);
	void			RemoveAllRecipients(void);
	void			RemoveRecipient(CPlayer *player);
	void			RemoveRecipientByPlayerIndex(int playerindex);
	void			AddRecipientsByTeam(int iTeam /*CTeam *team*/);
	void			RemoveRecipientsByTeam(int iTeam /*CTeam *team*/);
	void			RemoveRecipientsNotOnTeam(int iTeam /*CTeam *team*/);

	void			AddPlayersFromBitMask(CBitVec< ABSOLUTE_PLAYER_LIMIT >& playerbits);
	void			RemovePlayersFromBitMask(CBitVec< ABSOLUTE_PLAYER_LIMIT >& playerbits);

private:

	bool				m_bReliable;
	bool				m_bInitMessage;
	CUtlVector< int >	m_Recipients;
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for a single player ( unreliable )
//-----------------------------------------------------------------------------
class CSingleUserRecipientFilter : public CRecipientFilter
{
public:
	CSingleUserRecipientFilter(CPlayer *player)
	{
		AddRecipient(player);
	}
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for all players on a given team 
//-----------------------------------------------------------------------------
class CTeamRecipientFilter : public CRecipientFilter
{
public:
	CTeamRecipientFilter(int team, bool isReliable = false);
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for all players ( unreliable )
//-----------------------------------------------------------------------------
class CBroadcastRecipientFilter : public CRecipientFilter
{
public:
	CBroadcastRecipientFilter(void)
	{
		AddAllPlayers();
	}
};

//-----------------------------------------------------------------------------
// Purpose: Simple class to create a filter for all players ( reliable )
//-----------------------------------------------------------------------------
class CReliableBroadcastRecipientFilter : public CBroadcastRecipientFilter
{
public:
	CReliableBroadcastRecipientFilter(void)
	{
		MakeReliable();
	}
};

//-----------------------------------------------------------------------------
// Purpose: Add players in PAS to recipient list (unreliable)
//-----------------------------------------------------------------------------
class CPASFilter : public CRecipientFilter
{
public:
	CPASFilter(void)
	{
	}

	CPASFilter(const Vector& origin)
	{
		AddRecipientsByPAS(origin);
	}
};

//-----------------------------------------------------------------------------
// Purpose: Add players in PAS to list and if not in single player, use attenuation
//  to remove those that are too far away from source origin
// Source origin can be stated as an entity or just a passed in origin
// (unreliable)
//-----------------------------------------------------------------------------
class CPASAttenuationFilter : public CPASFilter
{
public:
	CPASAttenuationFilter(void)
	{
	}

	CPASAttenuationFilter(CEntity *entity, soundlevel_t soundlevel) :
		CPASFilter(static_cast<const Vector&>(entity->GetSoundEmissionOrigin()))
	{
		Filter(entity->GetSoundEmissionOrigin(), SNDLVL_TO_ATTN(soundlevel));
	}

	CPASAttenuationFilter(CEntity *entity, float attenuation = ATTN_NORM) :
		CPASFilter(static_cast<const Vector&>(entity->GetSoundEmissionOrigin()))
	{
		Filter(entity->GetSoundEmissionOrigin(), attenuation);
	}

	CPASAttenuationFilter(const Vector& origin, soundlevel_t soundlevel) :
		CPASFilter(origin)
	{
		Filter(origin, SNDLVL_TO_ATTN(soundlevel));
	}

	CPASAttenuationFilter(const Vector& origin, float attenuation = ATTN_NORM) :
		CPASFilter(origin)
	{
		Filter(origin, attenuation);
	}

	CPASAttenuationFilter(CEntity *entity, const char *lookupSound) :
		CPASFilter(static_cast<const Vector&>(entity->GetSoundEmissionOrigin()))
	{
		soundlevel_t level = soundemitterbase->LookupSoundLevel(lookupSound);
		float attenuation = SNDLVL_TO_ATTN(level);
		Filter(entity->GetSoundEmissionOrigin(), attenuation);
	}

	CPASAttenuationFilter(const Vector& origin, const char *lookupSound) :
		CPASFilter(origin)
	{
		soundlevel_t level = soundemitterbase->LookupSoundLevel(lookupSound);
		float attenuation = SNDLVL_TO_ATTN(level);
		Filter(origin, attenuation);
	}

	CPASAttenuationFilter(CEntity *entity, const char *lookupSound, HSOUNDSCRIPTHANDLE& handle) :
		CPASFilter(static_cast<const Vector&>(entity->GetSoundEmissionOrigin()))
	{
		soundlevel_t level = soundemitterbase->LookupSoundLevelByHandle(lookupSound, handle);
		float attenuation = SNDLVL_TO_ATTN(level);
		Filter(entity->GetSoundEmissionOrigin(), attenuation);
	}

	CPASAttenuationFilter(const Vector& origin, const char *lookupSound, HSOUNDSCRIPTHANDLE& handle) :
		CPASFilter(origin)
	{
		soundlevel_t level = soundemitterbase->LookupSoundLevelByHandle(lookupSound, handle);
		float attenuation = SNDLVL_TO_ATTN(level);
		Filter(origin, attenuation);
	}

public:
	void Filter(const Vector& origin, float attenuation = ATTN_NORM);
};

//-----------------------------------------------------------------------------
// Purpose: Simple PVS based filter ( unreliable )
//-----------------------------------------------------------------------------
class CPVSFilter : public CRecipientFilter
{
public:
	CPVSFilter(const Vector& origin)
	{
		AddRecipientsByPVS(origin);
	}
};

#endif //_INCLUDE_CRECIPIENTFILTER_H_
