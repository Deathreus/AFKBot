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
#include "bot_strings.h"
#include "bot_globals.h"
#include "bot_profile.h"
#include "bot_genclass.h"
#include "bot_visibles.h"
#include "bot_navigator.h"

vector <CBotProfile*> CBotProfiles::m_Profiles;
CBotProfile *CBotProfiles::m_pDefaultProfile = NULL;

CBotProfile::CBotProfile(CBotProfile &other)
{
	*this = other;

	m_szName = CStrings::GetString(other.m_szName);
	m_szModel = CStrings::GetString(other.m_szModel);
}

CBotProfile::CBotProfile(
	const char *szModel,
	int iVisionTicks,
	int iPathTicks,
	int iVisionTicksClients,
	int iSensitivity,
	float fBraveness,
	float fAimSkill,
	int iClass)
{
	m_iVisionTicksClients = iVisionTicksClients;
	m_iSensitivity = iSensitivity;
	m_fBraveness = fBraveness;
	m_fAimSkill = fAimSkill;
	m_szModel = CStrings::GetString(szModel);
	m_iPathTicks = iPathTicks;
	m_iVisionTicks = iVisionTicks;
	m_iClass = iClass;
}

void CBotProfiles::DeleteProfiles()
{
	for (unsigned int i = 0; i < m_Profiles.size(); i++)
	{
		delete m_Profiles[i];
		m_Profiles[i] = NULL;
	}

	m_Profiles.clear();

	delete m_pDefaultProfile;
	m_pDefaultProfile = NULL;
}

// find profiles and setup list
void CBotProfiles::SetupProfiles()
{
	extern ConVar bot_skill;
	extern ConVar bot_sensitivity;
	extern ConVar bot_braveness;
	extern ConVar bot_visrevs;
	extern ConVar bot_visrevs_client;
	extern ConVar bot_pathrevs;

	// Setup Default profile
	m_pDefaultProfile = new CBotProfile(
		"default", // model (team in HL2DM)
		bot_visrevs.GetInt(), // vis ticks
		bot_pathrevs.GetInt(), // path ticks
		bot_visrevs_client.GetInt(), // visrevs clients
		bot_sensitivity.GetFloat(), // sensitivity
		bot_braveness.GetFloat(), // braveness
		bot_skill.GetFloat(), // aim skill
		-1 // class
		);

}

CBotProfile *CBotProfiles::GetDefaultProfile()
{
	if (m_pDefaultProfile == NULL)
		CBotGlobals::BotMessage(NULL, 1, "Error, default profile is NULL (Caused by memory problem, bad initialisation or overwrite) Exiting..");

	return m_pDefaultProfile;
}

// return a profile unused by a bot
CBotProfile *CBotProfiles::GetRandomFreeProfile()
{
	unsigned int i;
	dataUnconstArray<int> iList;
	CBotProfile *found = NULL;

	for (i = 0; i < m_Profiles.size(); i++)
	{
		if (!CBots::FindBotByProfile(m_Profiles[i]))
			iList.Add(i);
	}

	if (iList.IsEmpty())
		return NULL;

	found = m_Profiles[iList.Random()];
	iList.Clear();

	return found;
}



