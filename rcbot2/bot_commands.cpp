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
#include "bot_strings.h"
#include "bot_commands.h"
#include "bot_globals.h"
//#include "bot_accessclient.h"
#include "bot_schedule.h"
#include "bot_waypoint.h" // for waypoint commands
#include "bot_waypoint_locations.h" // for waypoint commands
#include "ndebugoverlay.h"
#include "bot_waypoint_visibility.h"
#include "bot_getprop.h"
#include "bot_weapons.h"
#include "bot_menu.h"

#include "bot_tf2_points.h"

#ifdef GetClassName
#undef GetClassName
#endif

CBotCommandContainer *CBotGlobals::m_pCommands = new CRCBotCommand();

///////////////////////////////////////////////////
// Setup commands

CRCBotCommand::CRCBotCommand()
{
	SetName("afkbot");
	SetAccessLevel(0);
	Add(new CDebugCommand());
	Add(new CPrintCommands());
	Add(new CAFKOnCommand());
	Add(new CAFKOffCommand());
}

CDebugCommand::CDebugCommand()
{
	SetName("debug");
	SetAccessLevel(CMD_ACCESS_DEBUG);

	Add(new CDebugGameEventCommand());
	Add(new CDebugNavCommand());
	Add(new CDebugVisCommand());
	Add(new CDebugThinkCommand());
	Add(new CDebugLookCommand());
	Add(new CDebugHudCommand());
	Add(new CDebugAimCommand());
	Add(new CDebugChatCommand());
	Add(new CBotGoto());
	Add(new CBotFlush());
	Add(new CDebugTaskCommand());
	Add(new CBotTaskCommand());
	Add(new CDebugButtonsCommand());
	Add(new CDebugSpeedCommand());
	Add(new CDebugUsercmdCommand());
	Add(new CDebugUtilCommand());
	Add(new CDebugProfilingCommand());
	Add(new CDebugEdictsCommand());
	Add(new CDebugMemoryScanCommand());
	Add(new CDebugMemoryCheckCommand());

}
///////////////

eBotCommandResult CAFKOnCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	edict_t *pEntity = NULL;

	if (pClient)
		pEntity = pClient->GetPlayer();

	CBotGlobals::BotMessage(pEntity, 0, "AFK Enabled");

	CBots::MakeBot(pEntity);

	return COMMAND_ACCESSED;
}
///////////////

eBotCommandResult CAFKOffCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	edict_t *pEntity = NULL;

	if (pClient)
		pEntity = pClient->GetPlayer();

	CBotGlobals::BotMessage(pEntity, 0, "AFK Disabled");

	CBots::MakeNotBot(pEntity);

	return COMMAND_ACCESSED;
}
////////////////////////////////

//edits schedules

eBotCommandResult CBotTaskCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
#ifndef __linux__

	if (pClient && pClient->GetDebugBot() != NULL)
	{
		edict_t *pEdict = pClient->GetDebugBot();
		CBot *pBot = CBots::GetBotPointer(pEdict);

		if (pBot->InUse())
		{
			CBotSchedules *pSched = pBot->GetSchedule();

			if (pcmd && *pcmd)
			{
				//int task = atoi(pcmd);

				pSched->FreeMemory();

				// 83
				if (!strcmp(pcmd, "pipe"))
				{
					CBotUtility util = CBotUtility(pBot, BOT_UTIL_PIPE_LAST_ENEMY, true, 1.0f);
					pBot->SetLastEnemy(pClient->GetPlayer());
					pBot->GetSchedule()->FreeMemory();
					((CBotTF2*)pBot)->ExecuteAction(&util);
				}
				// 71
				/*else if ( !strcmp(pcmd,"gren") )
				{
				CBotWeapons *pWeapons;
				CBotWeapon *gren;

				pWeapons = pBot->getWeapons();
				gren = pWeapons->getGrenade();

				if ( gren )
				{
				CBotSchedule *sched = new CBotSchedule(new CThrowGrenadeTask(gren,pBot->getAmmo(gren->getWeaponInfo()->getAmmoIndex1()),pClient->getOrigin()));
				pSched->Add(sched);
				}
				}*/
				else if (!strcmp(pcmd, "snipe"))
				{
					if (pClient)
					{

						CWaypoint *pWaypoint = CWaypoints::GetWaypoint(CWaypoints::NearestWaypointGoal(CWaypointTypes::W_FL_SNIPER, pClient->GetOrigin(), 200.0f, pBot->GetTeam()));

						if (pWaypoint)
						{
							if (CBotGlobals::IsMod(MOD_TF2))
							{
								//if ( CClassInterface::getTF2Class() )
							}
							else
							{
								/*CBotWeapon *pWeapon;
								CBotWeapons *m_pWeapons;
								CBotSchedule *snipe = new CBotSchedule();
								CBotTask *findpath = new CFindPathTask(CWaypoints::getWaypointIndex(pWaypoint));
								CBotTask *snipetask;

								m_pWeapons = pBot->getWeapons();
								pWeapon = m_pWeapons->hasWeapon(DOD_WEAPON_K98_SCOPED) ? m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_K98_SCOPED)) : m_pWeapons->getWeapon(CWeapons::getWeapon(DOD_WEAPON_SPRING));

								if ( pWeapon )
								{
								// linux fix - copy origin onto vector here
								Vector vOrigin = pWaypoint->getOrigin();
								snipetask = new CBotDODSnipe(pWeapon,vOrigin,pWaypoint->getAimYaw(),false,0,pWaypoint->getFlags());

								findpath->setCompleteInterrupt(CONDITION_PUSH);
								snipetask->setCompleteInterrupt(CONDITION_PUSH);

								snipe->setID(SCHED_DEFENDPOINT);
								snipe->addTask(findpath);
								snipe->addTask(snipetask);

								pSched->Add(snipe);
								}
								else
								CBotGlobals::BotMessage(NULL,0,"Bot is not a sniper");*/
							}
						}
						else
							CBotGlobals::BotMessage(NULL, 0, "Sniper waypoint not found");

					}
				}
			}

		}
	}

#endif
	return COMMAND_ACCESSED;

}
//////////////////////
//clear bots schedules

eBotCommandResult CBotFlush::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (pClient && pClient->GetDebugBot() != NULL)
	{
		edict_t *pEdict = pClient->GetDebugBot();
		CBot *pBot = CBots::GetBotPointer(pEdict);

		if (pBot->InUse())
		{
			CBotSchedules *pSched = pBot->GetSchedule();
			pSched->FreeMemory();
		}
	}

	return COMMAND_ACCESSED;
}
///////////////////////
eBotCommandResult CBotGoto::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (pClient && pClient->GetDebugBot() != NULL)
	{
		edict_t *pEdict = pClient->GetDebugBot();
		CBot *pBot = CBots::GetBotPointer(pEdict);

		if (pBot->InUse())
		{
			int iWpt;

			if (pcmd && *pcmd)
			{
				iWpt = atoi(pcmd);

				if ((iWpt < 0) || (iWpt >= CWaypoints::NumWaypoints()))
					iWpt = -1;
			}
			else
				iWpt = pClient->CurrentWaypoint();

			if (iWpt != -1)
				pBot->ForceGotoWaypoint(iWpt);
		}
	}

	return COMMAND_ACCESSED;
}
/////////////////////

//usage \"memorycheck <classname> <offset> <type>\"");
eBotCommandResult CDebugMemoryCheckCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	// pcmd = classname
	// arg1 = offset
	// arg2 = type
	NEED_ARG(pcmd);
	NEED_ARG(arg1);
	NEED_ARG(arg2);
	// find edict
	edict_t *pEdict = CClassInterface::FindEntityByClassnameNearest(pClient->GetOrigin(), pcmd);

	if (pEdict == NULL)
	{
		CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "Edict not found");
		return COMMAND_ERROR;
	}

	CBaseEntity *pent = pEdict->GetUnknown()->GetBaseEntity();

	unsigned int offset = atoi(arg1);

	if ((strcmp(arg2, "bool") == 0) || (strcmp(arg2, "byte") == 0))
	{
		CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "%s - offset %d - Value(byte) = %d", pcmd, offset, *(byte*)(((unsigned long)pent) + offset));
	}
	else if (strcmp(arg2, "int") == 0)
	{
		CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "%s - offset %d - Value(int) = %d", pcmd, offset, *(int*)(((unsigned long)pent) + offset));
		/*
		if ( strcmp(pcmd,"team_control_point_master") == 0 )
		{
		CTeamControlPointMaster *p;
		CTeamControlPointMaster check;

		unsigned int knownoffset = (unsigned int)&check.m_iCurrentRoundIndex - (unsigned int)&check;

		p = (CTeamControlPointMaster*)((((unsigned long)pent) + offset) - knownoffset); //MAP_CLASS(CTeamControlPoint,(((unsigned long)pent) + offset),knownoffset);
		}
		else if ( strcmp(pcmd,"team_control_point") == 0 )
		{
		extern ConVar bot_const_point_offset;
		extern ConVar bot_const_point_data_offset;

		CTeamControlPoint *p = (CTeamControlPoint*)((((unsigned long)pent) + bot_const_point_offset.GetInt())); //MAP_CLASS(CTeamControlPoint,(((unsigned long)pent) + offset),knownoffset);
		//			CTeamControlPointData *d = (CTeamControlPointData*)((((unsigned long)pent) + bot_const_point_data_offset.GetInt()));

		CBotGlobals::BotMessage(NULL,0,"NULL MSG");
		}*/

	}
	else if (strcmp(arg2, "float") == 0)
		CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "%s - offset %d - Value(float) = %0.6f", pcmd, offset, *(float*)(((unsigned long)pent) + offset));
	else if (strcmp(arg2, "string") == 0)
	{
		string_t *str = (string_t*)(((unsigned long)pent) + offset);
		if (str)
			CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "%s - offset %d - Value(string) = %s", pcmd, offset, STRING(*str));
		else
			CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "%s - offset %d - INVALID string", pcmd, offset);
	}
	else
		return COMMAND_ERROR;

	return COMMAND_ACCESSED;
}

#define MEMSEARCH_BYTE 1
#define MEMSEARCH_INT 2
#define MEMSEARCH_FLOAT 3
#define MEMSEARCH_STRING 4

eBotCommandResult CDebugMemoryScanCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	//pcmd = classname
	// arg1 = value
	// arg2 = size in bytes
	// arg3 = want to remember offsets or not

	NEED_ARG(pcmd);
	NEED_ARG(arg1);
	NEED_ARG(arg2);

	unsigned int m_prev_size = m_size;

	if ((strcmp(arg2, "bool") == 0) || (strcmp(arg2, "byte") == 0))
		m_size = MEMSEARCH_BYTE;
	else if (strcmp(arg2, "int") == 0)
		m_size = MEMSEARCH_INT;
	else if (strcmp(arg2, "float") == 0)
		m_size = MEMSEARCH_FLOAT;
	else if (strcmp(arg2, "string") == 0)
		m_size = MEMSEARCH_STRING;
	else
		m_size = 0;

	if ((m_prev_size != m_size) || ((m_size == 0) || !arg3 || !*arg3) || (atoi(arg3) == 0))
	{
		memset(stored_offsets, 0, sizeof(u_MEMSEARCH)*MAX_MEM_SEARCH);
	}


	// find edict
	edict_t *pEdict = CClassInterface::FindEntityByClassnameNearest(pClient->GetOrigin(), pcmd);

	if (pEdict == NULL)
	{
		CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "Edict not found");
		return COMMAND_ERROR;
	}

	// begin memory scan
	CBaseEntity *pent = pEdict->GetUnknown()->GetBaseEntity();

	byte *mempoint = (byte*)pent;
	byte value = (byte)atoi(arg1);
	int ivalue = (atoi(arg1));
	float fvalue = (atof(arg1));

	bool bfound;

	for (int i = 0; i < MAX_MEM_SEARCH; i++) // 2KB search
	{
		bfound = false;

		if (m_size == MEMSEARCH_BYTE)
			bfound = (value == *mempoint);
		else if (m_size == MEMSEARCH_INT)
			bfound = (ivalue == *(int*)mempoint);
		else if (m_size == MEMSEARCH_FLOAT)
			bfound = (fvalue == *(float*)mempoint);
		else if (m_size == MEMSEARCH_STRING)
		{
			try
			{
				string_t *str = (string_t*)mempoint;

				if (str != NULL)
				{
					const char *pszstr = STRING(*str);

					if (pszstr)
						bfound = (strcmp(pszstr, arg1) == 0);
				}
			}
			catch (...)
			{
				CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "Invalid string");
			}
		}

		if (bfound)
		{
			if (!stored_offsets[i].b1.searched)
				stored_offsets[i].b1.found = 1;
		}
		else if (stored_offsets[i].b1.searched)
			stored_offsets[i].b1.found = 0;

		stored_offsets[i].b1.searched = 1;

		mempoint++;
	}

	// Current valid offsets print
	for (int i = 0; i < MAX_MEM_SEARCH; i++)
	{
		if (stored_offsets[i].data != 0)
		{
			if (stored_offsets[i].b1.found)
				CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "%d", i);
		}
	}
	// 

	return COMMAND_ACCESSED;
}


eBotCommandResult CDebugGameEventCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_GAME_EVENT, atoi(pcmd)>0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugVisCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_VIS, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugThinkCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_THINK, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugLookCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_LOOK, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugHudCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_HUD, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}
eBotCommandResult CDebugAimCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_AIM, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugChatCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_CHAT, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugProfilingCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_PROFILE, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugNavCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_NAV, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugTaskCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_TASK, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugUtilCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_UTIL, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugEdictsCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_EDICTS, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugSpeedCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_SPEED, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}

eBotCommandResult CDebugUsercmdCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_USERCMD, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}


eBotCommandResult CDebugButtonsCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
		return COMMAND_ERROR;

	pClient->SetDebug(BOT_DEBUG_BUTTONS, atoi(pcmd) > 0);

	return COMMAND_ACCESSED;
}


// kickbot
/*eBotCommandResult CKickBotCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!pcmd || !*pcmd)
	{
		//remove random bot
		CBots::KickRandomBot();
	}
	else
	{
		int team = atoi(pcmd);

		CBots::KickRandomBotOnTeam(team);
	}


	return COMMAND_ACCESSED;
}*/

eBotCommandResult CSearchCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	int i = 0;

	edict_t *pPlayer = pClient->GetPlayer();
	edict_t *pEdict;
	float fDistance;
	string_t model;

	for (i = 0; i < gpGlobals->maxEntities; i++)
	{
		pEdict = INDEXENT(i);

		if (pEdict)
		{
			if (!pEdict->IsFree())
			{
				if (pEdict->m_pNetworkable && pEdict->GetIServerEntity())
				{
					if ((fDistance = (CBotGlobals::EntityOrigin(pEdict) - CBotGlobals::EntityOrigin(pPlayer)).Length()) < 128)
					{
						float fVelocity;
						Vector v;

						if (CClassInterface::GetVelocity(pEdict, &v))
							fVelocity = v.Length();
						else
							fVelocity = 0;

						model = pEdict->GetIServerEntity()->GetModelName();

						CBotGlobals::BotMessage(pPlayer, 0, "(%d) D:%0.2f C:'%s', Mid:%d, Mn:'%s' Health=%d, Tm:%d, Fl:%d, Spd=%0.2f", i, fDistance, pEdict->GetClassName(), pEdict->GetIServerEntity()->GetModelIndex(), model.ToCStr(), (int)CClassInterface::GetPlayerHealth(pEdict), (int)CClassInterface::GetTeam(pEdict), pEdict->m_fStateFlags, fVelocity);
					}
				}
			}
		}
	}

	return COMMAND_ACCESSED;

}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CBotCommand::SetName(char *szName)
{
	m_szCommand = CStrings::GetString(szName);
}

void CBotCommand::SetHelp(char *szHelp)
{
	m_szHelp = CStrings::GetString(szHelp);
}

void CBotCommand::SetAccessLevel(int iAccessLevel)
{
	m_iAccessLevel = iAccessLevel;
}

CBotCommand::CBotCommand(char *szCommand, int iAccessLevel)
{
	m_szCommand = CStrings::GetString(szCommand);
	m_iAccessLevel = iAccessLevel;
}

void CBotCommand::FreeMemory()
{
	// nothing to free -- done in CStrings
}

/*bool CBotCommand :: hasAccess ( CClient *pClient )
{
return (m_iAccessLevel & pClient->accessLevel()) == m_iAccessLevel;
}*/

bool CBotCommand::IsCommand(const char *szCommand)
{
	return FStrEq(szCommand, m_szCommand);
}

eBotCommandResult CBotCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	return COMMAND_NOT_FOUND;
}


////////////////////////////
// container of commands
eBotCommandResult CBotCommandContainer::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	for (unsigned int i = 0; i < m_theCommands.size(); i++)
	{
		CBotCommand *pCommand = m_theCommands[i];

		if (pCommand->IsCommand(pcmd))
		{
			/*if (pClient && !pCommand->HasAccess(pClient))
				return COMMAND_REQUIRE_ACCESS;*/
			if (!pClient && !CanbeUsedDedicated())
			{
				CBotGlobals::BotMessage(NULL, 0, "Sorry, this command cannot be used on a dedicated server");
				return COMMAND_ERROR;
			}
			// move arguments
			eBotCommandResult iResult = pCommand->Execute(pClient, arg1, arg2, arg3, arg4, arg5, NULL);

			if (iResult == COMMAND_ERROR)
			{
				if (pClient)
					pCommand->PrintHelp(pClient->GetPlayer());
				else
					pCommand->PrintHelp(NULL);
			}

			return COMMAND_ACCESSED;
		}
	}

	if (pClient)
		PrintHelp(pClient->GetPlayer());
	else
		PrintHelp(NULL);

	return COMMAND_NOT_FOUND;
}

void CBotCommandContainer::FreeMemory()
{
	for (unsigned int i = 0; i < m_theCommands.size(); i++)
	{
		m_theCommands[i]->FreeMemory();
		delete m_theCommands[i];
		m_theCommands[i] = NULL;
	}

	m_theCommands.clear();
}
//////////////////////////////////////////

eBotCommandResult CPrintCommands::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (pClient != NULL)
	{
		CBotGlobals::BotMessage(pClient->GetPlayer(), 0, "All bot commands:");
		CBotGlobals::m_pCommands->PrintCommand(pClient->GetPlayer());
	}
	else
	{
		CBotGlobals::BotMessage(NULL, 0, "All bot commands:");
		CBotGlobals::m_pCommands->PrintCommand(NULL);
	}

	return COMMAND_ACCESSED;
}

///////////////////////////////////////////

void CBotCommand::PrintCommand(edict_t *pPrintTo, int indent)
{
	if (indent)
	{
		const int maxIndent = 64;
		char szIndent[maxIndent];
		int i;

		for (i = 0; (i < (indent * 2)) && (i < maxIndent - 1); i++)
			szIndent[i] = ' ';

		szIndent[maxIndent - 1] = 0;
		szIndent[i] = 0;

		if (!pPrintTo && !CanbeUsedDedicated())
			CBotGlobals::BotMessage(pPrintTo, 0, "%s%s [can't use]", szIndent, m_szCommand);
		else
			CBotGlobals::BotMessage(pPrintTo, 0, "%s%s", szIndent, m_szCommand);
	}
	else
	{
		if (!pPrintTo && !CanbeUsedDedicated())
			CBotGlobals::BotMessage(pPrintTo, 0, "%s [can't use]", m_szCommand);
		else
			CBotGlobals::BotMessage(pPrintTo, 0, m_szCommand);
	}
}

void CBotCommand::PrintHelp(edict_t *pPrintTo)
{
	if (m_szHelp)
		CBotGlobals::BotMessage(pPrintTo, 0, m_szHelp);
	else
		CBotGlobals::BotMessage(pPrintTo, 0, "Sorry, no help for this command (yet)");

	return;
}

void CBotCommandContainer::PrintCommand(edict_t *pPrintTo, int indent)
{
	//char cmd1[512];
	//char cmd2[512];

	//sprintf(cmd1,"%%%ds",indent);
	//sprintf(cmd2,cmd1,m_szCommand);

	if (indent)
	{
		const int maxIndent = 64;
		char szIndent[maxIndent];

		int i;

		for (i = 0; (i < (indent * 2)) && (i < maxIndent - 1); i++)
			szIndent[i] = ' ';

		szIndent[maxIndent - 1] = 0;
		szIndent[i] = 0;

		CBotGlobals::BotMessage(pPrintTo, 0, "%s[%s]", szIndent, m_szCommand);
	}
	else
		CBotGlobals::BotMessage(pPrintTo, 0, "[%s]", m_szCommand);

	for (unsigned int i = 0; i < m_theCommands.size(); i++)
	{
		m_theCommands[i]->PrintCommand(pPrintTo, indent + 1);
	}
}

void CBotCommandContainer::PrintHelp(edict_t *pPrintTo)
{
	PrintCommand(pPrintTo);
	return;
}


eBotCommandResult CTestCommand::Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	// for developers
	return COMMAND_NOT_FOUND;
}
