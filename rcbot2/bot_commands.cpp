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
#include "bot_base.h"
#include "bot_strings.h"
#include "bot_commands.h"
#include "bot_globals.h"
//#include "bot_navmesh.h"
#include "ndebugoverlay.h"

CBotCommandContainer *CBotGlobals::m_pCommands = new CRCBotCommand();
extern IVDebugOverlay *debugoverlay;
///////////////////////////////////////////////////
// Setup commands
CRCBotCommand::CRCBotCommand()
{
	SetName("afkbot");
	SetAccessLevel(CMD_ACCESS_ALL);
	Add(new CNavMeshCommand());
	Add(new CPrintCommands());
}

CNavMeshCommand::CNavMeshCommand()
{
	SetName("navmesh");
	SetAccessLevel(CMD_ACCESS_WAYPOINT);
	Add(new CNavMeshAddAttributeCommand());
	Add(new CNavMeshRemoveAttributeCommand());
	Add(new CNavMeshLoadCommand());
	Add(new CNavMeshSaveCommand());
}

CNavMeshAddAttributeCommand::CNavMeshAddAttributeCommand()
{
	SetName("addattrib");
	SetHelp("Mark the area under your cursor with the given attribute.\nAttributes:\nteleentrence\nteleext\nsentrygun\nrocketjump\ndoublejump");
}

eBotCommandResult CNavMeshAddAttributeCommand::Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	if (pClient)
	{
		/*Vector vPos;
		INavMeshArea *area = m_pNavMesh->GetArea(vPos);

		if (area)
		{
			return COMMAND_ACCESSED;
		}*/
	}

	return COMMAND_ERROR;
}

CNavMeshRemoveAttributeCommand::CNavMeshRemoveAttributeCommand()
{
	SetName("removeattrib");
	SetHelp("Remove an attribute from the area under your cursor.\nAttributes:\nteleentrence\nteleext\nsentrygun\nrocketjump\ndoublejump");
}

eBotCommandResult CNavMeshRemoveAttributeCommand::Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	/*if (pClient)
	{
		Vector vPos;
		INavMeshArea *area = CNavMeshNavigator::GetArea(vPos);

		if (area)
		{
			return COMMAND_ACCESSED;
		}
	}*/

	return COMMAND_ERROR;
}

CNavMeshLoadCommand::CNavMeshLoadCommand()
{
	SetName("load");
	SetHelp("Load the navmesh into memory for editing.");
}

eBotCommandResult CNavMeshLoadCommand::Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	/*char *error;
	INavMeshLoader *loader = new CNavMeshLoader(gamehelpers->GetCurrentMap());
	m_pNavMesh = loader->Load(error, 255);*/
	return COMMAND_ACCESSED;
}

CNavMeshSaveCommand::CNavMeshSaveCommand()
{
	SetName("save");
	SetHelp("Save changes to the mesh.");
}

eBotCommandResult CNavMeshSaveCommand::Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
	
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
