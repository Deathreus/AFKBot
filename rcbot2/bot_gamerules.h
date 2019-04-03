/**
* vim: set ts=4 :
* =============================================================================
* SourceMod
* Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
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
*
* As a special exception, AlliedModders LLC gives you permission to link the
* code of this program (as well as its derivative works) to "Half-Life 2," the
* "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
* by the Valve Corporation.  You must obey the GNU General Public License in
* all respects for all other code used.  Additionally, AlliedModders LLC grants
* this exception to all derivative works.  AlliedModders LLC defines further
* exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
* or <http://www.sourcemod.net/license.php>.
*
* Version: $Id$
*/
#ifndef __BOT_GAMERULES_H__
#define __BOT_GAMERULES_H__

#include <sp_vm_types.h>

enum eRoundState
{
	// Initialize the game, create teams
	RoundState_Init,

	// Before players have joined the game. Periodically checks to see if enough players are ready
	// to start a game. Also reverts to this when there are no active players
	RoundState_Pregame,

	// The game is about to start, wait a bit and spawn everyone
	RoundState_StartGame,

	// All players are respawned, frozen in place
	RoundState_Preround,

	// Round is on, playing normally
	RoundState_RoundRunning,

	// Someone has won the round
	RoundState_TeamWin,

	// Noone has won, manually restart the game, reset scores
	RoundState_Restart,

	// Noone has won, restart the game
	RoundState_Stalemate,

	// Game is over, showing the scoreboard etc
	RoundState_GameOver,

	// Game is over, doing bonus round stuff
	RoundState_Bonus,

	// Between rounds
	RoundState_BetweenRounds,
};

class CGameRulesObject
{
public:
	// Returns an integer from the gamerules entity
	static int32_t GetProperty(const char *prop, int size = 4, int element = 0);

	static void *GetGameRules();

	static bool GetGameRules(char *error, size_t maxlen);

	inline static void FreeMemory() { g_pGameRules = NULL; }

	static void *g_pGameRules;
};

#endif