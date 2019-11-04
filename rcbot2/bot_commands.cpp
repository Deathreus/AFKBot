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
#include <vector>

#include "bot_base.h"
#include "bot_strings.h"
#include "bot_commands.h"
#include "bot_globals.h"

#include "bot_waypoint.h"
#include "bot_waypoint_locations.h"

#include "../NavMesh/List.h"
#include "../NavMesh/NavMesh.h"
#include "../NavMesh/NavMeshLoader.h"
#include "../NavMesh/NavHintType.h"

CBotCommandContainer *CBotGlobals::m_pCommands = new CRCBotCommand();

#include <ndebugoverlay.h>

INavMesh *g_pNavMesh;

inline unsigned char HintNameToFlag(const char *name)
{
	if (FStrEq(name, "rally"))
		return NAV_HINT_RALLY;
	if (FStrEq(name, "sniper"))
		return NAV_HINT_SNIPE;
	if (FStrEq(name, "bomb"))
		return NAV_HINT_BOMB;
	if (FStrEq(name, "cp"))
		return NAV_HINT_CONTROLPOINT;
	if (FStrEq(name, "sentry"))
		return NAV_HINT_SENTRY;
	if (FStrEq(name, "tele"))
		return NAV_HINT_TELEPORTER;

	return 0;
}

extern IServerGameClients *gameclients;
extern IPlayerInfoManager *playerinfomanager;
extern IEngineTrace *engtrace;

///////////////////////////////////////////////////
// Setup commands
CRCBotCommand::CRCBotCommand()
{
	SetName("afkbot");
	SetAccessLevel(CMD_ACCESS_NONE);
	Add(new CWaypointCommand());
	Add(new CNavMeshCommand());
	Add(new CPrintCommands());
}

///////////////////////////////////////////////////

CWaypointCommand::CWaypointCommand()
{
	SetName("waypoint");
	SetAccessLevel(CMD_ACCESS_WAYPOINT);
	Add(new CWaypointLoadCommand());
	Add(new CWaypointSaveCommand());
	Add(new CWaypointResetCommand());
	Add(new CWaypointAddCommand());
	Add(new CWaypointRemoveCommand());
	Add(new CWaypointGiveTypeCommand());
	Add(new CWaypointSetYawCommand());
	Add(new CWaypointSetRadiusCommand());
}

eBotCommandResult CWaypointLoadCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if(CBotGlobals::IsMapRunning())
	{
		bool bSuccess = CWaypoints::Load(CBotGlobals::GetMapName());
		return bSuccess ? COMMAND_ACCESSED : COMMAND_ERROR;
	}

	CBotGlobals::BotMessage(pClient, 0, "Error loading waypoints: No map.");
	return COMMAND_ERROR;
}

eBotCommandResult CWaypointSaveCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if(CWaypoints::Save(false, pClient))
	{
		CBotGlobals::BotMessage(pClient, 0, "Successfully saved waypoint information.");
		return COMMAND_ACCESSED;
	}

	return COMMAND_ERROR;
}

eBotCommandResult CWaypointResetCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	CWaypoints::Init(CWaypoints::GetAuthor(), CWaypoints::GetModifier());
	CBotGlobals::BotMessage(pClient, 0, "Cleared waypoint information.");
	return COMMAND_ACCESSED;
}

eBotCommandResult CWaypointAddCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	CWaypoints::AddWaypoint(pClient, cmd, arg1, arg2, arg3);
	return COMMAND_ACCESSED;
}

eBotCommandResult CWaypointRemoveCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if(pClient)
	{
		if(cmd && *cmd)
		{
			float fRadius = atof(cmd);

			if(fRadius > 0)
			{
				std::vector<int> wpts;
				int numdeleted = 0;
				Vector vOrigin = CBotGlobals::EntityOrigin(pClient);

				CWaypointLocations::GetAllInArea(vOrigin, &wpts, -1);

				for(unsigned short int i = 0; i < wpts.size(); i++)
				{
					CWaypoint *pWaypoint = CWaypoints::GetWaypoint(wpts[i]);

					if(pWaypoint->DistanceFrom(vOrigin) < fRadius)
					{
						CWaypoints::DeleteWaypoint(wpts[i]);
						numdeleted++;
					}
				}

				if(numdeleted > 0)
				{
					CBotGlobals::BotMessage(pClient, 0, "%d waypoints within range of %0.0f deleted", numdeleted, fRadius);
					engine->ClientCommand(pClient, "play \"buttons/combine_button_locked\"");
				}
				else
				{
					CBotGlobals::BotMessage(pClient, 0, "No waypoints within range of %0.0f", fRadius);
					engine->ClientCommand(pClient, "play \"weapons/wpn_denyselect\"");
				}
			}
		}
		else
		{
			int iWpt = CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(pClient), 50, -1, false, true, false, NULL, false, 0, false, false);

			if(CWaypoints::ValidWaypointIndex(iWpt))
			{
				CWaypoints::DeleteWaypoint(iWpt);
				CBotGlobals::BotMessage(pClient, 0, "Waypoint %d deleted", iWpt);
				engine->ClientCommand(pClient, "play \"buttons/combine_button_locked\"");
			}
			else
			{
				CBotGlobals::BotMessage(pClient, 0, "No waypoint nearby to delete");
				engine->ClientCommand(pClient, "play \"weapons/wpn_denyselect\"");
			}
		}

		return COMMAND_ACCESSED;
	}

	return COMMAND_ERROR;
}

eBotCommandResult CWaypointGiveTypeCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if(pClient)
	{
		if(cmd && *cmd)
		{
			int iWpt = CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(pClient) + Vector(0, 0, 8), 80, -1, false, true, false, NULL, false, 0, false, false);
			if(iWpt == -1)
			{
				CBotGlobals::BotMessage(pClient, 0, "No waypoints near you, get closer to one and try again.");
				return COMMAND_ACCESSED;
			}

			CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);
			char *type = NULL;

			for(int i = 0; i < 6; i++)
			{
				switch(i)
				{
					case 0:
						type = (char*)cmd;
						break;
					case 1:
						type = (char*)arg1;
						break;
					case 2:
						type = (char*)arg2;
						break;
					case 3:
						type = (char*)arg3;
						break;
					case 4:
						type = (char*)arg4;
						break;
					case 5:
						type = (char*)arg5;
						break;
				}

				if(!type || !*type)
					break;

				CWaypointType *pType = CWaypointTypes::GetType(type);

				if(pType)
				{
					unsigned int bits = pType->GetBits();

					if(pWpt)
					{
						if(pWpt->HasFlag(bits))
						{
							pWpt->RemoveFlag(bits);
							CBotGlobals::BotMessage(pClient, 0, "Type %s removed from waypoint %d", type, iWpt);
							engine->ClientCommand(pClient, "play \"UI/buttonrollover\"");
						}
						else
						{
							pWpt->AddFlag(bits);

							if(bits & CWaypointTypes::W_FL_UNREACHABLE)
							{
								CWaypoints::DeletePathsTo(iWpt);
								CWaypoints::DeletePathsFrom(iWpt);
							}

							CBotGlobals::BotMessage(pClient, 0, "Type %s added to waypoint %d", type, iWpt);
							engine->ClientCommand(pClient, "play \"UI/buttonclickrelease\"");
						}
					}
				}
				else
				{
					CBotGlobals::BotMessage(pClient, 0, "Type '%s' not found", type);
					CWaypointTypes::ShowTypesOnConsole(pClient);
				}
			}
		}
		else
		{
			CWaypointTypes::ShowTypesOnConsole(pClient);
		}

		return COMMAND_ACCESSED;
	}

	return COMMAND_ERROR;
}

eBotCommandResult CWaypointSetYawCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if(pClient)
	{
		int iWpt = CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(pClient) + Vector(0, 0, 8), 80, -1, false, true, false, NULL, false, 0, false, false);
		if(iWpt == -1)
		{
			CBotGlobals::BotMessage(pClient, 0, "No waypoints near you, get closer to one and try again.");
			return COMMAND_ACCESSED;
		}

		CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);

		if(cmd && *cmd)
		{
			float fYaw = atof(cmd);
			pWpt->SetAim(clamp(fYaw, -180.0f, 180.0f));
			CBotGlobals::BotMessage(pClient, 0, "Set aim yaw of waypoint %d to %.2f°.", iWpt, clamp(fYaw, -180.0f, 180.0f));
		}
		else
		{
			float fYaw = CBotGlobals::PlayerAngles(pClient).y;
			pWpt->SetAim(clamp(fYaw, -180.0f, 180.0f));
			CBotGlobals::BotMessage(pClient, 0, "Updated aim yaw of waypoint %d to %.2f°.", iWpt, clamp(fYaw, -180.0f, 180.0f));
		}

		return COMMAND_ACCESSED;
	}

	return COMMAND_ERROR;
}

eBotCommandResult CWaypointSetRadiusCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if(pClient)
	{
		int iWpt = CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(pClient) + Vector(0, 0, 8), 80, -1, false, true, false, NULL, false, 0, false, false);
		if(iWpt == -1)
		{
			CBotGlobals::BotMessage(pClient, 0, "No waypoints near you, get closer to one and try again.");
			return COMMAND_ACCESSED;
		}

		CWaypoint *pWpt = CWaypoints::GetWaypoint(iWpt);

		if(cmd && *cmd)
		{
			float fRadius = atof(cmd);
			pWpt->SetRadius(clamp(fRadius, 0.0f, 1024.0f));
			CBotGlobals::BotMessage(pClient, 0, "Set radius of waypoint %d to %.2f.", iWpt, clamp(fRadius, 0.0f, 1024.0f));
		}
		else
		{
			pWpt->SetRadius(0.0f);
			CBotGlobals::BotMessage(pClient, 0, "Removed radius of waypoint %d.", iWpt);
		}

		return COMMAND_ACCESSED;
	}

	return COMMAND_ERROR;
}

///////////////////////////////////////////////////

CNavMeshCommand::CNavMeshCommand()
{
	SetName("navmesh");
	SetAccessLevel(CMD_ACCESS_WAYPOINT);
	Add(new CNavMeshAddHintCommand());
	Add(new CNavMeshRemoveHintCommand());
	Add(new CNavMeshLoadCommand());
	Add(new CNavMeshSaveCommand());
	Add(new CNavMeshHintListCommand());
	Add(new CNavMeshGenWaypointsCommand());
}

eBotCommandResult CNavMeshAddHintCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (pClient)
	{
		NEED_ARG(arg1);
		
		Vector vPos = playerinfomanager->GetPlayerInfo(pClient)->GetAbsOrigin();
		float fYaw = gameclients->GetPlayerState(pClient)->v_angle[YAW];
		unsigned char flags = HintNameToFlag(arg1);

		char message[256];
		smutils->Format(message, sizeof(message), "Adding a hint at |%.3f %.3f %.3f| facing %.1f degrees of type '%s'.\n", vPos.x, vPos.y, vPos.z, fYaw, arg1);
		CBotGlobals::BotMessage(pClient, 0, message);

		g_pNavMesh->AddHint(vPos, fYaw, flags);
	}

	return COMMAND_ERROR;
}

eBotCommandResult CNavMeshRemoveHintCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (pClient)
	{
		NEED_ARG(arg1);

		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(pClient);
		IPlayerInfo *pPI = pPlayer->GetPlayerInfo();
		if(!pPI || !pPI->IsPlayer())
		{
			CBotGlobals::BotMessage(0, 0, "This command can only be used in game.");
			return COMMAND_ERROR;
		}

		Vector vStart;
		Vector vEnd;
		Vector vDir;

		static Ray_t ray;
		static CTraceFilterWorldAndPropsOnly filter;

		CPlayerState *pPS = gameclients->GetPlayerState(pPlayer->GetEdict());

		gameclients->ClientEarPosition(pPlayer->GetEdict(), &vStart);

		AngleVectors(pPS->v_angle, &vDir);

		vEnd = vStart + vDir * 99999.0f;

		ray.Init(vStart, vEnd);

		trace_t tr;
		engtrace->TraceRay(ray, MASK_SOLID, &filter, &tr);
		if(tr.DidHit())
		{
			if(g_pNavMesh->RemoveHint(tr.endpos))
				pPlayer->PrintToConsole("Removed hint closest to your crosshair.\n");
			else
				pPlayer->PrintToConsole("No hints found to remove.\n");
		}
	}

	return COMMAND_ERROR;
}

eBotCommandResult CNavMeshLoadCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	char error[256];
	g_pNavMesh = CNavMeshLoader::Load(error, sizeof(error));
	if (!g_pNavMesh || (error && *error))
	{
		CBotGlobals::BotMessage(pClient, 0, error);
		return COMMAND_ERROR;
	}

	return COMMAND_ACCESSED;
}

eBotCommandResult CNavMeshSaveCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (!g_pNavMesh)
	{
		CBotGlobals::BotMessage(pClient, 0, "The navmesh isn't loaded!");
		return COMMAND_ERROR;
	}

	if(CNavMeshLoader::Save(g_pNavMesh))
		return COMMAND_ACCESSED;
	
	return COMMAND_ERROR;
}

eBotCommandResult CNavMeshGenWaypointsCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if(!g_pNavMesh)
	{
		CBotGlobals::BotMessage(pClient, 0, "The navmesh isn't loaded!");
		return COMMAND_ERROR;
	}

	if(CWaypoints::NumWaypoints() > 64)
	{
		CBotGlobals::BotMessage(pClient, 0, "Waypoints may have already been generated, automatic generation isn't possible");
		return COMMAND_ERROR;
	}

	CWaypoints::PrepareGeneration();
	CWaypoints::WantToGenerate(true);

	return COMMAND_ACCESSED;
}

eBotCommandResult CNavMeshHintListCommand::Execute(edict_t *pClient, const char *cmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	CBotGlobals::BotMessage(pClient, 0, "Hint types:\n"
										 "rally\n"
										 "sniper\n"
										 "bomb\n"
										 "cp\n"
										 "sentry\n"
										 "tele");
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

bool CBotCommand::HasAccess(edict_t *pClient)
{
	return adminsys->CheckAdminFlags(playerhelpers->GetGamePlayer(pClient)->GetAdminId() , m_iAccessLevel);
}

bool CBotCommand::IsCommand(const char *szCommand)
{
	return FStrEq(szCommand, m_szCommand);
}

eBotCommandResult CBotCommand::Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	return COMMAND_NOT_FOUND;
}


////////////////////////////
// container of commands
eBotCommandResult CBotCommandContainer::Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	for (unsigned int i = 0; i < m_theCommands.size(); i++)
	{
		CBotCommand *pCommand = m_theCommands[i];

		if (pCommand->IsCommand(pcmd))
		{
			if (pClient && !pCommand->HasAccess(pClient))
				return COMMAND_REQUIRE_ACCESS;
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
					pCommand->PrintHelp(pClient);
				else
					pCommand->PrintHelp(NULL);
			}

			return COMMAND_ACCESSED;
		}
	}

	if (pClient)
		PrintHelp(pClient);
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

eBotCommandResult CPrintCommands::Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (pClient != NULL)
	{
		CBotGlobals::BotMessage(pClient, 0, "All bot commands:");
		CBotGlobals::m_pCommands->PrintCommand(pClient);
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
