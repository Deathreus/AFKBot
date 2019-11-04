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

#include "bot_base.h"
#include "bot_mods.h"
#include "bot_globals.h"
#include "bot_fortress.h"
#include "bot_weapons.h"
#include "bot_getprop.h"
//#include "bot_dod_bot.h"

#include <vector>

//std::vector<edict_wpt_pair_t> CHalfLifeDeathmatchMod::m_LiftWaypoints;

void CBotMods::ReadMods()
{
	//m_Mods.push_back(new CDODMod());

	//m_Mods.push_back(new CCounterStrikeSourceMod());
	//m_Mods.push_back(new CHalfLifeDeathmatchMod());

	//m_Mods.push_back(new CFortressForeverMod());

	m_Mods.push_back(new CTeamFortress2Mod());

	//m_Mods.push_back(new CHLDMSourceMod());
}

//////////////////////////////////////////////////////////////////////////////

void CBotMod::Setup(const char *szModFolder, const char *szSteamFolder, eModId iModId, eBotType iBotType, const char *szWeaponListName)
{
	m_szModFolder = CStrings::GetString(szModFolder);
	m_szSteamFolder = CStrings::GetString(szSteamFolder);
	m_iModId = iModId;
	m_iBotType = iBotType;

	if (szWeaponListName && *szWeaponListName)
		m_szWeaponListName = CStrings::GetString(szWeaponListName);
}

bool CBotMod::IsSteamFolder(char *szSteamFolder)
{
	return FStrEq(m_szSteamFolder, szSteamFolder);
}

bool CBotMod::IsModFolder(char *szModFolder)
{
	return FStrEq(m_szModFolder, szModFolder);
}

char *CBotMod::GetSteamFolder()
{
	return m_szSteamFolder;
}

char *CBotMod::GetModFolder()
{
	return m_szModFolder;
}

eModId CBotMod::GetModId()
{
	return m_iModId;
}

//
// MOD LIST

std::vector<CBotMod*> CBotMods::m_Mods;

void CBotMods::FreeMemory()
{
	for (unsigned int i = 0; i < m_Mods.size(); i++)
	{
		m_Mods[i]->FreeMemory();
		delete m_Mods[i];
		m_Mods[i] = NULL;
	}

	m_Mods.clear();
}

CBotMod *CBotMods::GetMod(char *szModFolder, char *szSteamFolder)
{
	for (unsigned int i = 0; i < m_Mods.size(); i++)
	{
		if (m_Mods[i]->IsModFolder(szModFolder) && m_Mods[i]->IsSteamFolder(szSteamFolder))
		{
			CBotGlobals::BotMessage(NULL, 1, "HL2 MOD ID %d (Steam Folder = %s) (Game Folder = %s) FOUND", m_Mods[i]->GetModId(), szSteamFolder, szModFolder);

			return m_Mods[i];
		}
	}

	CBotGlobals::BotMessage(NULL, 1, "HL2 MODIFICATION \"%s/%s\" NOT FOUND, EXITING... see bot_mods.cfg in bot config folder", szSteamFolder, szModFolder);

	return NULL;
}

void CBotMod::InitMod()
{
	CWeapons::LoadWeapons(m_szWeaponListName, NULL);
}

void CBotMod::MapInit()
{
	
}

bool CBotMod::PlayerSpawned(CBaseEntity *pEntity)
{
	return true;
}

/*bool CHalfLifeDeathmatchMod::PlayerSpawned ( edict_t *pPlayer )
{
	if ( CBotMod::PlayerSpawned(pPlayer) )
	{
		m_LiftWaypoints.Clear();

		CWaypoints::UpdateWaypointPairs(&m_LiftWaypoints, CWaypointTypes::W_FL_LIFT, "func_button");
	}

	return true;
}

void CHalfLifeDeathmatchMod::InitMod ()
{
	CBots::ControlBotSetup(false);

	CWeapons::LoadWeapons((m_szWeaponListName==NULL)?"HL2DM":m_szWeaponListName, HL2DMWeaps);

	//for ( i = 0; i < HL2DM_WEAPON_MAX; i ++ )
	//	CWeapons::AddWeapon(new CWeapon(HL2DMWeaps[i]));//.iSlot, HL2DMWeaps[i].szWeaponName, HL2DMWeaps[i].iId, HL2DMWeaps[i].m_iFlags, HL2DMWeaps[i].m_iAmmoIndex, HL2DMWeaps[i].minPrimDist, HL2DMWeaps[i].maxPrimDist, HL2DMWeaps[i].m_iPreference, HL2DMWeaps[i].m_fProjSpeed));
}

void CHalfLifeDeathmatchMod::MapInit ()
{
	CBotMod::MapInit();

	m_LiftWaypoints.Clear();
}*/