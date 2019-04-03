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
#ifndef __BOT_CONST_H__
#define __BOT_CONST_H__

#include <shareddefs.h>


#define __to_lower(a) (((a) >= 'A') && ((a) <= 'Z')) ? ('a' + ((a) - 'A')) : (a)
#define __strlow(str) { char *__strx = str; while ( __strx && *__strx ) { *__strx = __to_lower(*__strx); __strx++; } }

//#define RANDOM_INT(min, max) (round( ( (float)rand() / RAND_MAX ) * (float)( max - min ) + min ) )
//#define RANDOM_FLOAT(min, max) (min + ( (float)rand() / RAND_MAX ) * (float)( max - min ) )

#define BOT_DEBUG_GAME_EVENT	0 
#define BOT_DEBUG_NAV			1 
#define BOT_DEBUG_SPEED			2 
#define BOT_DEBUG_VIS			3
#define BOT_DEBUG_TASK			4 
#define BOT_DEBUG_BUTTONS		5  
#define BOT_DEBUG_USERCMD		6 
#define BOT_DEBUG_UTIL			7
#define BOT_DEBUG_PROFILE		8 
#define BOT_DEBUG_EDICTS		9 
#define BOT_DEBUG_THINK			10 
#define BOT_DEBUG_LOOK			11 
#define BOT_DEBUG_HUD			12 
#define BOT_DEBUG_AIM			13 
#define BOT_DEBUG_CHAT			14

typedef enum
{
	LOOK_NONE = 0,
	LOOK_VECTOR,
	LOOK_WAYPOINT,
	LOOK_WAYPOINT_NEXT_ONLY,
	LOOK_AROUND,
	LOOK_ENEMY,
	LOOK_LAST_ENEMY,
	LOOK_HURT_ORIGIN,
	LOOK_EDICT,
	LOOK_GROUND,
	LOOK_SNIPE,
	LOOK_WAYPOINT_AIM,
	LOOK_BUILD,
	LOOK_NOISE,
	LOOK_MAX
}eLookTask;

extern const char *const g_szLookTaskToString[LOOK_MAX];

#define BOT_CONFIG_EXTENSION "cfg"
#define BOT_BELIEF_EXTENTION "rcb"
#define BOT_WAYPOINT_EXTENSION "rcw"
#define BOT_WAYPOINT_DISTANCE_EXTENSION "rcd"
#define BOT_WAYPOINT_VISIBILITY_EXTENSION "rcv"
#define BOT_WAYPOINT_FILE_TYPE "RCBot2\0" // for waypoint file header

#define BOT_TAG "[AFKBot] " // for printing messages

typedef enum
{
	MOD_UNSUPPORTED = 0,
	MOD_HLDM2,
	MOD_CSS,
	MOD_FF,
	MOD_TF2,
	MOD_SVENCOOP2,
	MOD_TIMCOOP,
	MOD_HL1DMSRC,
	MOD_NS2,
	MOD_SYNERGY,
	MOD_DOD,
	MOD_CUSTOM,
	MOD_ANY,
	MOD_MAX
}eModId;

#define BITS_MOD_ALL ~(1<<MOD_MAX)

#define BOT_DEFAULT_FOV 75

#define BOT_JUMP_HEIGHT 45

#define MIN_COVER_MOVE_DIST 128

#define BOT_WPT_TOUCH_DIST 72 // distance for bot to touch waypoint

#define INDEXENT(iEdictNum) engine->PEntityOfEntIndex(iEdictNum)
#define ENTINDEX(pEdict) engine->IndexOfEdict(pEdict)

#define BOT_NAME "AFKBot"
#define BOT_NAME_VER "AFKBot version"
#define BOT_VER_CVAR "afkbot_version"

typedef enum
{
	BOT_FUNC_FAIL = 0,
    BOT_FUNC_CONTINUE,
	BOT_FUNC_COMPLETE,
}eBotFuncState;

//////////////////////////////////
#define CONDITION_ENEMY_OBSCURED		1			// bit 0 - bot lost sight of enemy and can't see clearly
#define CONDITION_NO_WEAPON				2			// bit 1 - bot doesn't have a weapon
#define CONDITION_OUT_OF_AMMO			4			// bit 2 - bot has no ammo
#define CONDITION_SEE_CUR_ENEMY			8			// bit 3 - bot can see current enemy
#define CONDITION_ENEMY_DEAD			16			// bit 4 - bot s enemy is dead
#define CONDITION_SEE_WAYPOINT			32			// bit 5 - bot can see the current waypoint
#define CONDITION_NEED_AMMO				64			// bit 6 - bot needs ammo (low)
#define CONDITION_NEED_HEALTH			128			// bit 7 - bot needs health
#define CONDITION_SEE_LOOK_VECTOR		256			// bit 8 - bot can see his 'look' vector
#define CONDITION_POINT_CAPTURED		512			// bit 9 a point has been captured recently
#define CONDITION_PUSH					1024		// bit 10 - bots are more likely to attack and stop sniping etc
#define CONDITION_LIFT					2048		// bit 11 - bot is on a lift
#define CONDITION_SEE_HEAL				4096		// bit 12 - medic bot (tf2) can see his player he wants to heal
#define CONDITION_SEE_PLAYERTOHELP		4096		// same thing as heal for other mods
#define CONDITION_SEE_LAST_ENEMY_POS	8192		// bit 13 - bots can see the last place they saw an enemy
#define CONDITION_CHANGED				16384		// bit 14 - bots want to change their course of action
#define CONDITION_COVERT				32768		// bit 15 - bots are more sensitive to enemies and more likely to take alternate paths
#define CONDITION_RUN					65536		// bit 16 - bots have to run e.g. there is a grenade nearby
#define CONDITION_GREN					131072		// bit 17 - bots will be more likely to throw a grenade
#define CONDITION_NEED_BOMB				262144		// bit 18 - bot needs a bomb for dod:s bomb maps
#define CONDITION_SEE_ENEMY_HEAD		524288		// bit 19 - bot can aim for a headshot
#define CONDITION_PRONE					1048576		// bit 20 - bot needs to go prone (lie down)
#define CONDITION_PARANOID				2097152		// bit 21 - bot is paranoid of spies or unknown enemy
#define CONDITION_DEFENSIVE				4194304		// bit 22 - bot leader told me to defend
#define CONDITION_BUILDING_SAPPED		8388608		// bit 23 - one of engineers buildings sapped
#define CONDITION_SEE_ENEMY_GROUND		16777216	// bit 24 - can see enemy ground so aim for it if i have explosive
#define CONDITION_MAX					CONDITION_SEE_ENEMY_GROUND
#define CONDITION_MAX_BITS				24

typedef enum
{
    BELIEF_NONE = 0,
	BELIEF_DANGER = (1<<0),
	BELIEF_SAFETY = (1<<1)
}BotBelief;

#define MAX_MAP_STRING_LEN 64

#define MAX_ENTITIES (1<<MAX_EDICT_BITS)

#define MAX_AMMO_TYPES 32

#define MAX_VOICE_CMDS 32

#define BLAST_RADIUS 200

#define MIN_WPT_TOUCH_DIST 16.0f

#define MOVELOOK_DEFAULT	0
#define MOVELOOK_THINK		1
#define MOVELOOK_MODTHINK	2
#define MOVELOOK_TASK		3
#define MOVELOOK_LISTEN		4
#define MOVELOOK_EVENT		5
#define MOVELOOK_ATTACK		6
#define MOVELOOK_OVERRIDE	6

#define DEG_TO_RAD(x) (x)*0.0174533
#define RAD_TO_DEG(x) (x)*57.29578

#define TIME_NOW	(gpGlobals->curtime + FLT_EPSILON)

#define DOT_1DEGREE   0.9998476951564
#define DOT_2DEGREE   0.9993908270191
#define DOT_3DEGREE   0.9986295347546
#define DOT_4DEGREE   0.9975640502598
#define DOT_5DEGREE   0.9961946980917
#define DOT_6DEGREE   0.9945218953683
#define DOT_7DEGREE   0.9925461516413
#define DOT_8DEGREE   0.9902680687416
#define DOT_9DEGREE   0.9876883405951
#define DOT_10DEGREE  0.9848077530122
#define DOT_15DEGREE  0.9659258262891
#define DOT_20DEGREE  0.9396926207859
#define DOT_25DEGREE  0.9063077870367
#define DOT_30DEGREE  0.866025403784
#define DOT_45DEGREE  0.707106781187

#define VIEW_FIELD_FULL			(float)-1.0 // +-180 degrees
#define	VIEW_FIELD_WIDE			(float)-0.7 // +-135 degrees 0.1 // +-85 degrees, used for full FOV checks 
#define	VIEW_FIELD_NARROW		(float)0.7 // +-45 degrees, more narrow check used to set up ranged attacks
#define	VIEW_FIELD_ULTRA_NARROW	(float)0.9 // +-25 degrees, more narrow check used to set up ranged attacks

///////////////////////
typedef enum 
{
	STATE_IDLE = 0,
	STATE_RUNNING,
	STATE_FAIL,
	STATE_COMPLETE
}eTaskState;
////////////////////

#endif