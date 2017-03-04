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
#ifndef __BOT_COMMANDS_H__
#define __BOT_COMMANDS_H__

#include <vector>
using namespace std;

class CClient;

typedef enum
{
	COMMAND_NOT_FOUND,     // command not found
	COMMAND_ACCESSED,      // okay
	COMMAND_ERROR,		   // accessed but error occurred
	COMMAND_REQUIRE_ACCESS // dont have access to command
}eBotCommandResult;

#define NEED_ARG(x) if ( !x || !*x ) return COMMAND_ERROR;


#define CMD_ACCESS_NONE				0
#define CMD_ACCESS_WAYPOINT			ADMFLAG_RCON
#define CMD_ACCESS_BOT				ADMFLAG_CHEATS

#define CMD_ACCESS_ALL (CMD_ACCESS_WAYPOINT|CMD_ACCESS_BOT)

class CBotCommand
{
protected:
	CBotCommand()
	{
		m_iAccessLevel = 0;
		m_szCommand = NULL;
		m_szHelp = NULL;
	}
public:
	// initialise
	CBotCommand(char *szCommand, int iAccessLevel = 0);

	// check command name
	bool IsCommand(const char *szCommand);

	// execute command
	virtual eBotCommandResult Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	// delete command
	virtual void FreeMemory();

	virtual void ShowStatus() { return; }
	virtual void ShowHelp() { return; }

	bool HasAccess(edict_t *pClient);

	virtual void PrintCommand(edict_t *pPrintTo, int indent = 0);

	virtual void PrintHelp(edict_t *pPrintTo);

	virtual bool IsContainer() { return false; }

	virtual bool CanbeUsedDedicated() { return false; }
protected:
	void SetName(char *szName);
	void SetAccessLevel(int iAccessLevel);
	void SetHelp(char *pszHelp);

	int m_iAccessLevel;
	char *m_szCommand;
	char *m_szHelp;
};

// container of commands
class CBotCommandContainer : public CBotCommand
{
public:
	CBotCommandContainer() {};

	// call execute command
	eBotCommandResult Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	void FreeMemory();

	void Add(CBotCommand *newCommand) { m_theCommands.push_back(newCommand); }

	void PrintCommand(edict_t *pPrintTo, int indent = 0);

	bool IsContainer() { return true; }

	virtual void PrintHelp(edict_t *pPrintTo);

	virtual bool CanbeUsedDedicated() { return true; }
private:
	vector<CBotCommand*> m_theCommands;
};

/////////////////////////////////////////////////
class CRCBotCommand : public CBotCommandContainer
{
public:
	CRCBotCommand();
};

class CNavMeshCommand : public CBotCommandContainer
{
public:
	CNavMeshCommand();
};

class CNavMeshAddAttributeCommand : public CBotCommand
{
public:
	CNavMeshAddAttributeCommand();

	eBotCommandResult Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	virtual bool CanbeUsedDedicated() { return true; }
};

class CNavMeshRemoveAttributeCommand : public CBotCommand
{
public:
	CNavMeshRemoveAttributeCommand();

	eBotCommandResult Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	virtual bool CanbeUsedDedicated() { return true; }
};

class CNavMeshLoadCommand : public CBotCommand
{
public:
	CNavMeshLoadCommand();

	eBotCommandResult Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	virtual bool CanbeUsedDedicated() { return true; }
};

class CNavMeshSaveCommand : public CBotCommand
{
public:
	CNavMeshSaveCommand();

	eBotCommandResult Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	virtual bool CanbeUsedDedicated() { return true; }
};

class CPrintCommands : public CBotCommand
{
public:
	CPrintCommands()
	{
		SetName("printcommands");
		SetAccessLevel(0);
	}

	eBotCommandResult Execute(edict_t *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	virtual bool CanbeUsedDedicated() { return true; }
};

#endif
