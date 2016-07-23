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
#define CMD_ACCESS_WAYPOINT			(1<<0)
#define CMD_ACCESS_BOT				(1<<1)
#define CMD_ACCESS_UTIL				(1<<2)
#define CMD_ACCESS_CONFIG			(1<<3)
#define CMD_ACCESS_DEBUG			(1<<4)
#define CMD_ACCESS_USERS            (1<<5)

#define CMD_ACCESS_ALL (CMD_ACCESS_WAYPOINT|CMD_ACCESS_UTIL|CMD_ACCESS_BOT|CMD_ACCESS_CONFIG|CMD_ACCESS_DEBUG|CMD_ACCESS_USERS)

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
	virtual eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	// delete command
	virtual void FreeMemory();

	virtual void ShowStatus() { return; }
	virtual void ShowHelp() { return; }

	bool HasAccess(CClient *pClient);

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

	// call Execute command
	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

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
class CAFKOnCommand : public CBotCommand
{
public:
	CAFKOnCommand()
	{
		SetName("afk");
		SetAccessLevel(0);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	virtual bool CanbeUsedDedicated() { return true; }
};

class CAFKOffCommand : public CBotCommand
{
public:
	CAFKOffCommand()
	{
		SetName("back");
		SetAccessLevel(0);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	virtual bool CanbeUsedDedicated() { return true; }
};

class CRCBotCommand : public CBotCommandContainer
{
public:
	CRCBotCommand();
};

//clear bots schedules
class CBotTaskCommand : public CBotCommand
{
public:
	CBotTaskCommand()
	{
		SetName("givetask");
		SetAccessLevel(CMD_ACCESS_DEBUG);
		SetHelp("gives a bot a task : usage <id> <entity name - for reference>");
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

//clear bots schedules
class CBotFlush : public CBotCommand
{
public:
	CBotFlush()
	{
		SetName("bot_flush");
		SetAccessLevel(CMD_ACCESS_DEBUG);
		SetHelp("flush bot tasks");
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

//bot goto
class CBotGoto : public CBotCommand
{
public:
	CBotGoto()
	{
		SetName("bot_goto");
		SetAccessLevel(CMD_ACCESS_DEBUG);
		SetHelp("set a debug bot first and then stand near a waypoint to force your bot to go there");
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

/////////////////////////////////////////////////////
class CDebugCommand : public CBotCommandContainer
{
public:
	CDebugCommand();
};

class CDebugProfilingCommand : public CBotCommand
{
public:
	CDebugProfilingCommand()
	{
		SetName("profiling");
		SetHelp("usage \"profiling 1 or 0, 1 on, 0 off\" : shows performance profiling");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugMemoryCheckCommand : public CBotCommand
{
public:
	CDebugMemoryCheckCommand()
	{
		SetName("memorycheck");
		SetHelp("usage \"memorycheck <classname> <offset> <type>\"");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

#define MAX_MEM_SEARCH 8192

typedef union
{
	struct
	{
		unsigned searched : 1; // already searched
		unsigned found : 1; // offset found
		unsigned unused : 6;
	}b1;

	byte data;
}u_MEMSEARCH;

class CDebugMemoryScanCommand : public CBotCommand
{
public:
	CDebugMemoryScanCommand()
	{
		SetName("memoryscan");
		SetHelp("usage \"memoryscan <classname> <value> <type = 'bool/int/byte/float'> [store last = 1]\"");
		SetAccessLevel(CMD_ACCESS_DEBUG);
		memset(stored_offsets, 0, sizeof(u_MEMSEARCH)*MAX_MEM_SEARCH);
		m_size = 0;
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
private:

	u_MEMSEARCH stored_offsets[MAX_MEM_SEARCH];
	unsigned int m_size;
};

class CDebugNavCommand : public CBotCommand
{
public:
	CDebugNavCommand()
	{
		SetName("nav");
		SetHelp("usage \"nav 1 or 0, 1 on, 0 off\" : shows navigation output");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugTaskCommand : public CBotCommand
{
public:
	CDebugTaskCommand()
	{
		SetName("task");
		SetHelp("usage \"task 1 or 0, 1 on, 0 off\" : shows task output");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugUtilCommand : public CBotCommand
{
public:
	CDebugUtilCommand()
	{
		SetName("util");
		SetHelp("usage \"util 1 or 0, 1 on, 0 off\" : shows utility/action output");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};


class CDebugUsercmdCommand : public CBotCommand
{
public:
	CDebugUsercmdCommand()
	{
		SetName("usercmd");
		SetHelp("usage \"usercmd 1 or 0, 1 on, 0 off\" : shows last user command output");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugEdictsCommand : public CBotCommand
{
public:
	CDebugEdictsCommand()
	{
		SetName("edicts");
		SetHelp("usage \"edicts 1 or 0, 1 on, 0 off\" : shows allocated/freed edicts");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};


class CDebugSpeedCommand : public CBotCommand
{
public:
	CDebugSpeedCommand()
	{
		SetName("speed");
		SetHelp("usage \"speed 1 or 0, 1 on, 0 off\" : shows speed");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugAimCommand : public CBotCommand
{
public:
	CDebugAimCommand()
	{
		SetName("aim");
		SetHelp("usage \"aim 1 or 0, 1 on, 0 off\" : displays aiming accuracy info on the hud");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugChatCommand : public CBotCommand
{
public:
	CDebugChatCommand()
	{
		SetName("chat");
		SetHelp("usage \"chat 1 or 0, 1 on, 0 off\" : displays logs in chat");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugHudCommand : public CBotCommand
{
public:
	CDebugHudCommand()
	{
		SetName("hud");
		SetHelp("usage \"hud 1 or 0, 1 on, 0 off\" : displays most important info about bot on the hud");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugButtonsCommand : public CBotCommand
{
public:
	CDebugButtonsCommand()
	{
		SetName("buttons");
		SetHelp("usage \"buttons 1 or 0, 1 on, 0 off\" : shows buttons bitmask");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugLookCommand : public CBotCommand
{
public:
	CDebugLookCommand()
	{
		SetName("look");
		SetHelp("usage \"look 1 or 0, 1 on, 0 off\" : shows bot look output");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};


class CDebugThinkCommand : public CBotCommand
{
public:
	CDebugThinkCommand()
	{
		SetName("think");
		SetHelp("usage \"think 1 or 0, 1 on, 0 off\" : shows bot thinking output");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugVisCommand : public CBotCommand
{
public:
	CDebugVisCommand()
	{
		SetName("vis");
		SetHelp("usage \"vis 1 or 0, 1 on, 0 off\" : shows bot visibility output");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

class CDebugGameEventCommand : public CBotCommand
{
public:
	CDebugGameEventCommand()
	{
		SetName("gameevent");
		SetHelp("usage \"gameevent 1 or 0, 1 on, 0 off\" : shows event output");
		SetAccessLevel(CMD_ACCESS_DEBUG);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

//////////////////////////////////
class CSearchCommand : public CBotCommand
{
public:
	CSearchCommand()
	{
		SetName("search");
		SetAccessLevel(CMD_ACCESS_UTIL);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
};

///////////////////////////////////////
class CPrintCommands : public CBotCommand
{
public:
	CPrintCommands()
	{
		SetName("printcommands");
		SetAccessLevel(0);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	virtual bool CanbeUsedDedicated() { return true; }
};

class CTestCommand : public CBotCommand
{
public:
	CTestCommand()
	{
		SetName("test");
		SetAccessLevel(0);
	}

	eBotCommandResult Execute(CClient *pClient, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);

	virtual bool CanbeUsedDedicated() { return false; }
};
#endif
