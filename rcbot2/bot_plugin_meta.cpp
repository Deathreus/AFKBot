/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Sample Plugin
 * Written by AlliedModders LLC.
 * ======================================================
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software.
 *
 * This sample plugin is public domain.
 */

#include <stdio.h>

#include "bot_plugin_meta.h"

#include "filesystem.h"
#include "interface.h"

#ifdef __linux__
#include "shake.h"    //bir3yk
#endif
#include "IEffects.h"
#include "igameevents.h"
#include "IEngineTrace.h"

#include "Color.h"
#include "ndebugoverlay.h"
#include "server_class.h"
#include "time.h"
#include "irecipientfilter.h"

#include "KeyValues.h"

#include "bot_cvars.h"

// for IServerTools
#include "bot.h"
#include "bot_configfile.h"
#include "bot_globals.h"
#include "bot_profile.h"
#include "bot_waypoint.h"
#include "bot_menu.h"
#include "bot_getprop.h"
#include "bot_fortress.h"
#include "bot_event.h"
#include "bot_profiling.h"
#include "bot_wpt_dist.h"
#include "bot_squads.h"
#include "bot_accessclient.h"
#include "bot_weapons.h"
#include "bot_waypoint_visibility.h"
#include "bot_kv.h"
#include "bot_sigscan.h"

//#include "ndebugoverlay.h"
CBotTF2 *g_pLastBot;

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const *, char const *, char const *, char const *, bool, bool);
SH_DECL_HOOK3_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, 0, edict_t *, int, int);
SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);
SH_DECL_HOOK2_void(IServerGameClients, ClientActive, SH_NOATTRIB, 0, edict_t *, bool);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t *);
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, edict_t *, char const *);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, 0, int);
SH_DECL_HOOK1_void(IServerGameClients, ClientSettingsChanged, SH_NOATTRIB, 0, edict_t *);
SH_DECL_HOOK5(IServerGameClients, ClientConnect, SH_NOATTRIB, 0, bool, edict_t *, const char*, const char *, char *, int);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);


#if SOURCE_ENGINE >= SE_ORANGEBOX
SH_DECL_HOOK2_void(IServerGameClients, NetworkIDValidated, SH_NOATTRIB, 0, const char *, const char *);
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &);
#else
SH_DECL_HOOK1_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *);
#endif

SH_DECL_MANUALHOOK2_void(MHook_PlayerRunCmd, 0, 0, 0, CUserCmd*, IMoveHelper*);  

/*
SH_DECL_HOOK1_void(bf_write, WriteChar, SH_NOATTRIB, 0, int);
SH_DECL_HOOK1_void(bf_write, WriteShort, SH_NOATTRIB, 0, int);
SH_DECL_HOOK1_void(bf_write, WriteByte, SH_NOATTRIB, 0, int);
SH_DECL_HOOK1_void(bf_write, WriteFloat, SH_NOATTRIB, 0, float);
SH_DECL_HOOK1(bf_write, WriteString, SH_NOATTRIB, 0, bool, const char *);

SH_DECL_HOOK2(IVEngineServer, UserMessageBegin, SH_NOATTRIB, 0, bf_write*, IRecipientFilter*, int);
SH_DECL_HOOK0_void(IVEngineServer, MessageEnd, SH_NOATTRIB, 0);

bf_write *current_msg = NULL;

#define BUF_SIZ 1024
char current_msg_buffer[BUF_SIZ];
*/

CBaseEntity* (CBaseEntity::*TF2PlayerWeaponSlot)(int) = 0x0;
void (CBaseEntity::*TF2WeaponEquip)(CBaseEntity*) = 0x0;

IServerGameDLL *server = NULL;
IGameEventManager2 *gameevents = NULL;
IServerPluginCallbacks *vsp_callbacks = NULL;
ICvar *icvar = NULL;
IVEngineServer *engine = NULL;  // helper functions (messaging clients, loading content, making entities, running commands, etc)
IFileSystem *filesystem = NULL;  // file I/O 
IGameEventManager2 *gameeventmanager = NULL;
IGameEventManager *gameeventmanager1 = NULL;  // game events interface
IPlayerInfoManager *playerinfomanager = NULL;  // game dll interface to interact with players
IServerPluginHelpers *helpers = NULL;  // special 3rd party plugin helpers from the engine
IServerGameClients* gameclients = NULL;
IEngineTrace *enginetrace = NULL;
IEffects *g_pEffects = NULL;
CGlobalVars *gpGlobals = NULL;
IVDebugOverlay *debugoverlay = NULL;
IServerGameEnts *servergameents = NULL; // for accessing the server game entities
IServerGameDLL *servergamedll = NULL;
IServerTools *servertools = NULL;


AFKBot g_AFKBot;

PLUGIN_EXPOSE(AFKBot, g_AFKBot);

static ConVar afkbot_ver_cvar(BOT_VER_CVAR, BOT_VER, FCVAR_REPLICATED, BOT_NAME_VER);

CBaseEntity *AFKBot::TF2_getPlayerWeaponSlot(edict_t *pPlayer, int iSlot)
{
	CBaseEntity *pEnt = servergameents->EdictToBaseEntity(pPlayer);
	unsigned int *mem = (unsigned int*)*(unsigned int*)pEnt;
	int offset = rcbot_getweaponslot_offset.GetInt();
	
	*(unsigned int*)&TF2PlayerWeaponSlot = mem[offset];

	return (*pEnt.*TF2PlayerWeaponSlot)(iSlot);
}

class CBotRecipientFilter : public IRecipientFilter
{
public:
	CBotRecipientFilter(edict_t *pPlayer)
	{
		m_iPlayerSlot = ENTINDEX(pPlayer);
	}

	bool IsReliable(void) const { return false; }
	bool IsInitMessage(void) const { return false; }

	int	GetRecipientCount(void) const { return 1; }
	int	GetRecipientIndex(int slot) const { return m_iPlayerSlot; }

private:
	int m_iPlayerSlot;
};

class CClientBroadcastRecipientFilter : public IRecipientFilter
{
public:

	CClientBroadcastRecipientFilter() {
		m_iMaxCount = 0;

		for (int i = 0; i < MAX_PLAYERS; ++i) {
			CClient* client = CClients::get(i);

			if (client->isUsed()) {
				IPlayerInfo *p = playerinfomanager->GetPlayerInfo(client->getPlayer());

				if (p->IsConnected() && !p->IsFakeClient()) {
					m_iPlayerSlot[m_iMaxCount] = i;
					m_iMaxCount++;
				}
			}
		}
	}

	bool IsReliable(void) const { return false; }
	bool IsInitMessage(void) const { return false; }

	int	GetRecipientCount(void) const { return m_iMaxCount; }
	int	GetRecipientIndex(int slot) const { return m_iPlayerSlot[slot] + 1; }

private:

	int m_iMaxCount;
	int m_iPlayerSlot[MAX_PLAYERS];
};

///////////////
// hud message
///////////////
void AFKBot::HudTextMessage(edict_t *pEntity, const char *szMessage)
{
	int msgid = 0;
	int imsgsize = 0;
	char msgbuf[64];
	bool bOK;

	int hint = -1;
	int say = -1;

	while ((bOK = servergamedll->GetUserMessageInfo(msgid, msgbuf, 63, imsgsize)) == true)
	{
		if (strcmp(msgbuf, "HintText") == 0)
			hint = msgid;
		else if (strcmp(msgbuf, "SayText") == 0)
			say = msgid;

		msgid++;
	}

	if (msgid == 0)
		return;

	// if (!bOK)
	// return;

	CBotRecipientFilter *filter = new CBotRecipientFilter(pEntity);

	bf_write *buf = nullptr;

	if (hint > 0) {
		buf = engine->UserMessageBegin(filter, hint);
		buf->WriteString(szMessage);
		engine->MessageEnd();
	}

	if (say > 0) {
		char chatline[128];
		snprintf(chatline, sizeof(chatline), "\x01\x04[RCBot2]\x01 %s\n", szMessage);

		buf = engine->UserMessageBegin(filter, say);
		buf->WriteString(chatline);
		engine->MessageEnd();
	}

	delete filter;
}

//////////////////////////
// chat broadcast message
//////////////////////////
void AFKBot::BroadcastTextMessage(const char *szMessage)
{
	int msgid = 0;
	int imsgsize = 0;
	char msgbuf[64];
	bool bOK;

	int hint = -1;
	int say = -1;

	while ((bOK = servergamedll->GetUserMessageInfo(msgid, msgbuf, 63, imsgsize)) == true)
	{
		if (strcmp(msgbuf, "HintText") == 0)
			hint = msgid;
		else if (strcmp(msgbuf, "SayText") == 0)
			say = msgid;

		msgid++;
	}

	if (msgid == 0)
		return;

	CClientBroadcastRecipientFilter *filter = new CClientBroadcastRecipientFilter();

	bf_write *buf = nullptr;

	if (say > 0) {
		char chatline[128];
		snprintf(chatline, sizeof(chatline), "\x01\x04[RCBot2]\x01 %s\n", szMessage);

		buf = engine->UserMessageBegin(filter, say);
		buf->WriteString(chatline);
		engine->MessageEnd();
	}

	delete filter;
}

void AFKBot::TF2_equipWeapon(edict_t *pPlayer, CBaseEntity *pWeapon)
{
	CBaseEntity *pEnt = servergameents->EdictToBaseEntity(pPlayer);
	unsigned int *mem = (unsigned int*)*(unsigned int*)pEnt;
	int offset = rcbot_weaponequip_offset.GetInt();

	*(unsigned int*)&TF2WeaponEquip = mem[offset];

	(*pEnt.*TF2WeaponEquip)(pWeapon);
}

void AFKBot::Hook_PlayerRunCmd(CUserCmd *ucmd, IMoveHelper *moveHelper)
{
	static CBot *pBot;

	CBaseEntity *pEnt = META_IFACEPTR(CBaseEntity);

	edict_t *pEdict = servergameents->BaseEntityToEdict(pEnt);

	pBot = CBots::getBotPointer(pEdict);
	
	if ( pBot )
	{
		static CUserCmd *cmd;
		
		cmd = pBot->getUserCMD();

		// put the bot's commands into this move frame
		ucmd->buttons = cmd->buttons;
		ucmd->forwardmove = cmd->forwardmove;
		ucmd->impulse = cmd->impulse;
		ucmd->sidemove = cmd->sidemove;
		ucmd->upmove = cmd->upmove;
		ucmd->viewangles = cmd->viewangles;
		ucmd->weaponselect = cmd->weaponselect;
		ucmd->weaponsubtype = cmd->weaponsubtype;
		ucmd->tick_count = cmd->tick_count;
		ucmd->command_number = cmd->command_number;

		servertools->SnapPlayerToPosition(NULL, cmd->viewangles, NULL);

		g_pLastBot = (CBotTF2*)pBot;

		RETURN_META(MRES_OVERRIDE);
	}

//g_pSM->LogMessage(NULL, "H %i %i %f %f %f %f %i", ucmd->command_number, ucmd->tick_count, ucmd->viewangles.x, ucmd->viewangles.y, ucmd->viewangles.z, ucmd->forwardmove, ucmd->buttons); 

RETURN_META(MRES_IGNORED); 
}

/** 
 * Something like this is needed to register cvars/CON_COMMANDs.
 */
class BaseAccessor : public IConCommandBaseAccessor
{
public:
	bool RegisterConCommandBase(ConCommandBase *pCommandBase)
	{
		/* Always call META_REGCVAR instead of going through the engine. */
		return META_REGCVAR(pCommandBase);
	}
} s_BaseAccessor;

// --- you're going to take over message begin
bf_write *AFKBot::Hook_MessageBegin(IRecipientFilter *filter, int msg_type)
{
	/*
	bool bfound = false;

	for (int i = 0; i < filter->GetRecipientCount(); i++)
	{
		if (filter->GetRecipientIndex(i) == 1)
		{
			bfound = true;
			break;
		}
	}

	if (bfound)
	{
		
		int msgid = 0;
		int imsgsize = 0;
		char msgbuf[64];
		bool bOK;

		if (servergamedll->GetUserMessageInfo(msg_type, msgbuf, 63, imsgsize))
		{
			sprintf(current_msg_buffer, "MessageBegin() msg_type = %d name = %s\n", msg_type,msgbuf);
		}

	}
	else
		current_msg_buffer[0] = 0;
	
	current_msg = SH_CALL(engine, &IVEngineServer::UserMessageBegin)(filter, msg_type);

	if (current_msg)
	{
		SH_ADD_HOOK_MEMFUNC(bf_write, WriteString, current_msg, this, &AFKBot::Hook_WriteString, true);
		SH_ADD_HOOK_MEMFUNC(bf_write, WriteByte, current_msg, this, &AFKBot::Hook_WriteByte, true);
		SH_ADD_HOOK_MEMFUNC(bf_write, WriteChar, current_msg, this, &AFKBot::Hook_WriteChar, true);
		SH_ADD_HOOK_MEMFUNC(bf_write, WriteShort, current_msg, this, &AFKBot::Hook_WriteShort, true);
		SH_ADD_HOOK_MEMFUNC(bf_write, WriteFloat, current_msg, this, &AFKBot::Hook_WriteFloat, true);
	}

	//
	RETURN_META_VALUE(MRES_SUPERCEDE, current_msg);*/

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

void AFKBot::Hook_WriteChar(int val)
{
	/*char tocat[64];

	sprintf(tocat, "\nWriteChar(%c)", (char)val);
	strcat(current_msg_buffer, tocat);*/
}
void AFKBot::Hook_WriteShort(int val)
{
	/*char tocat[64];

	sprintf(tocat, "\nWriteShort(%d)", val);
	strcat(current_msg_buffer, tocat);*/
}
void AFKBot::Hook_WriteByte(int val)
{
	/*char tocat[64];

	sprintf(tocat, "\nWriteByte(%d)", val);
	strcat(current_msg_buffer, tocat);*/
}
void AFKBot::Hook_WriteFloat(float val)
{
	/*char tocat[64];

	sprintf(tocat, "\nWriteFloat(%0.1f)", val);
	strcat(current_msg_buffer, tocat);*/
}

bool AFKBot::Hook_WriteString(const char *pStr)
{
	/*char *tocat = new char[strlen(pStr) + 16];
	
	sprintf(tocat, "\nWriteString(%s)", pStr);
	strcat(current_msg_buffer, tocat);
	
	delete tocat;*/

	RETURN_META_VALUE(MRES_IGNORED, false);
}

void AFKBot::Hook_MessageEnd()
{
	// probe the current_msg m_pData
	// deep copy the data because it might free itself later
	//strncpy(current_msg_buffer, (char*)current_msg->m_pData, BUF_SIZ - 1);
	//current_msg_buffer[BUF_SIZ - 1] = 0;
	/*if (current_msg)
	{
		SH_REMOVE_HOOK_MEMFUNC(bf_write, WriteString, current_msg, this, &AFKBot::Hook_WriteString, true);
		SH_REMOVE_HOOK_MEMFUNC(bf_write, WriteByte, current_msg, this, &AFKBot::Hook_WriteByte, true);
		SH_REMOVE_HOOK_MEMFUNC(bf_write, WriteChar, current_msg, this, &AFKBot::Hook_WriteChar, true);
		SH_REMOVE_HOOK_MEMFUNC(bf_write, WriteShort, current_msg, this, &AFKBot::Hook_WriteShort, true);
		SH_REMOVE_HOOK_MEMFUNC(bf_write, WriteFloat, current_msg, this, &AFKBot::Hook_WriteFloat, true);
	}

	current_msg_buffer[0] = 0;*/

	RETURN_META(MRES_IGNORED);
}

bool AFKBot::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	extern MTRand_int32 irand;

	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, enginetrace, IEngineTrace, INTERFACEVERSION_ENGINETRACE_SERVER);	
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

	GET_V_IFACE_ANY(GetEngineFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION)

	GET_V_IFACE_ANY(GetServerFactory, servergameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, playerinfomanager, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);

	GET_V_IFACE_ANY(GetServerFactory, g_pEffects, IEffects, IEFFECTS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);

#ifndef __linux__
	GET_V_IFACE_CURRENT(GetEngineFactory,debugoverlay, IVDebugOverlay, VDEBUG_OVERLAY_INTERFACE_VERSION);
#endif
	GET_V_IFACE_ANY(GetServerFactory, servergamedll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);

	gpGlobals = ismm->GetCGlobals();

	META_LOG(g_PLAPI, "Starting plugin.");

	/* Load the VSP listener.  This is usually needed for IServerPluginHelpers. */
	if ((vsp_callbacks = ismm->GetVSPInfo(NULL)) == NULL)
	{
		ismm->AddListener(this, this);
		ismm->EnableVSPListener();
	}


	/*SH_ADD_HOOK_MEMFUNC(IVEngineServer, UserMessageBegin, engine, this, &AFKBot::Hook_MessageBegin, false);
	SH_ADD_HOOK_MEMFUNC(IVEngineServer, MessageEnd, engine, this, &AFKBot::Hook_MessageEnd, false);*/
	
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, server, this, &AFKBot::Hook_LevelInit, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, server, this, &AFKBot::Hook_ServerActivate, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &AFKBot::Hook_GameFrame, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, server, this, &AFKBot::Hook_LevelShutdown, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &AFKBot::Hook_ClientActive, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &AFKBot::Hook_ClientDisconnect, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &AFKBot::Hook_ClientPutInServer, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, gameclients, this, &AFKBot::Hook_SetCommandClient, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &AFKBot::Hook_ClientSettingsChanged, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientConnect, gameclients, this, &AFKBot::Hook_ClientConnect, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &AFKBot::Hook_ClientCommand, false);
	//Hook FireEvent to our function
	SH_ADD_HOOK_MEMFUNC(IGameEventManager2, FireEvent, gameevents, this, &AFKBot::FireGameEvent, false);


#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = icvar;
	ConVar_Register(0, &s_BaseAccessor);
#else
	ConCommandBaseMgr::OneTimeInit(&s_BaseAccessor);
#endif


	// Read Signatures and Offsets
	CBotGlobals::initModFolder();
	CBotGlobals::readRCBotFolder();

	char filename[512];
	// Load RCBOT2 hook data
	CBotGlobals::buildFileName(filename, "hookinfo", BOT_CONFIG_FOLDER, "ini");

	FILE *fp = fopen(filename, "r");

	CAFKBotKeyValueList *pKVL = new CAFKBotKeyValueList();

	if (fp)
		pKVL->parseFile(fp);

	void *gameServerFactory = reinterpret_cast<void*>(ismm->GetServerFactory(false));

	int val;

#ifdef _WIN32

	if (pKVL->getInt("runplayermove_tf2_win", &val))
		rcbot_runplayercmd_tf2.SetValue(val);
	if (pKVL->getInt("runplayermove_dods_win", &val))
		rcbot_runplayercmd_dods.SetValue(val);
	if (pKVL->getInt("getweaponslot_win", &val))
		rcbot_getweaponslot_offset.SetValue(val);
	if (pKVL->getInt("weaponequip_win", &val))
		rcbot_weaponequip_offset.SetValue(val);
	if (pKVL->getInt("gamerules_win", &val))
		rcbot_gamerules_offset.SetValue(val);
	if (pKVL->getInt("mstr_offset_win", &val)) {
		rcbot_const_point_master_offset.SetValue(val);
		rcbot_const_round_offset.SetValue(val);
	}
#else

	if (pKVL->getInt("runplayermove_tf2_linux", &val))
		rcbot_runplayercmd_tf2.SetValue(val);
	if (pKVL->getInt("runplayermove_dods_linux", &val))
		rcbot_runplayercmd_dods.SetValue(val);
	if (pKVL->getInt("getweaponslot_linux", &val))
		rcbot_getweaponslot_offset.SetValue(val);
	if (pKVL->getInt("weaponequip_linux", &val))
		rcbot_weaponequip_offset.SetValue(val);
	if (pKVL->getInt("mstr_offset_linux", &val)) {
		rcbot_const_point_master_offset.SetValue(val);
		rcbot_const_round_offset.SetValue(val);
	}
#endif

	g_pGameRules_Obj = new CGameRulesObject(pKVL, gameServerFactory);
	g_pGameRules_Create_Obj = new CCreateGameRulesObject(pKVL, gameServerFactory);

	delete pKVL;

	if (fp)
		fclose(fp);

	if (!CBotGlobals::gameStart())
		return false;

	CBotMod *pMod = CBotGlobals::getCurrentMod();

	if (pMod->getModId() == MOD_TF2)
		SH_MANUALHOOK_RECONFIGURE(MHook_PlayerRunCmd, rcbot_runplayercmd_tf2.GetInt(), 0, 0);
	else if (pMod->getModId() == MOD_DOD)
		SH_MANUALHOOK_RECONFIGURE(MHook_PlayerRunCmd, rcbot_runplayercmd_dods.GetInt(), 0, 0);

	ENGINE_CALL(LogPrint)("All hooks started!\n");



	//MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
	//ConVar_Register( 0 );
	//InitCVars( interfaceFactory ); // register any cvars we have defined

	srand( (unsigned)time(NULL) );  // initialize the random seed
	irand.seed( (unsigned)time(NULL) );

	// Find the RCBOT2 Path from metamod VDF
	extern IFileSystem *filesystem;
	KeyValues *mainkv = new KeyValues("metamodplugin");
	
	const char *rcbot2path;
	CBotGlobals::botMessage(NULL, 0, "Reading rcbot2 path from VDF...");
	
	mainkv->LoadFromFile(filesystem, "addons/metamod/afkbot.vdf", "MOD");
	
	mainkv = mainkv->FindKey("Metamod Plugin");

	if (mainkv)
		rcbot2path = mainkv->GetString("rcbot2path", "\0");

	mainkv->deleteThis();
	//eventListener2 = new CRCBotEventListener();

	// Initialize bot variables
	CBotProfiles::setupProfiles();


	//CBotEvents::setupEvents();
	CWaypointTypes::setup();
	CWaypoints::setupVisibility();

	CBotConfigFile::reset();	
	CBotConfigFile::load();

	CBotMenuList::setupMenus();

	//CRCBotPlugin::ShowLicense();	

	//RandomSeed((unsigned int)time(NULL));

	CClassInterface::init();

	RCBOT2_Cvar_setup(g_pCVar);

	if (fp) {
		fclose(fp);
	}

	return true;
}


bool AFKBot::FireGameEvent(IGameEvent * pevent, bool bDontBroadcast)
{
	static char szKey[128];
	static char szValue[128];

	CBotEvents::executeEvent((void*)pevent,TYPE_IGAMEEVENT);	

RETURN_META_VALUE(MRES_IGNORED, true);
}

bool AFKBot::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelInit, server, this, &AFKBot::Hook_LevelInit, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, server, this, &AFKBot::Hook_ServerActivate, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &AFKBot::Hook_GameFrame, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, server, this, &AFKBot::Hook_LevelShutdown, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &AFKBot::Hook_ClientActive, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &AFKBot::Hook_ClientDisconnect, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &AFKBot::Hook_ClientPutInServer, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, gameclients, this, &AFKBot::Hook_SetCommandClient, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &AFKBot::Hook_ClientSettingsChanged, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientConnect, gameclients, this, &AFKBot::Hook_ClientConnect, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &AFKBot::Hook_ClientCommand, false);
	
	//SH_REMOVE_MANUALHOOK(MHook_PlayerRunCmd, player_vtable, SH_STATIC(Hook_Function2), false);

	// if another instance is running dont run through this
	//if ( !bInitialised )
	//	return;
	
	CBots::freeAllMemory();
	CStrings::freeAllMemory();
	CBotGlobals::freeMemory();
	CBotMods::freeMemory();
	//CAccessClients::freeMemory();
	CBotEvents::freeMemory();
	CWaypoints::freeMemory();
	CWaypointTypes::freeMemory();
	CBotProfiles::deleteProfiles();
	CWeapons::freeMemory();
	CBotMenuList::freeMemory();

	//unloadSignatures();

	//UnhookPlayerRunCommand();
	//UnhookGiveNamedItem();

	//ConVar_Unregister();

	//if ( gameevents )
	//	gameevents->RemoveListener(this);

	// Reset Cheat Flag
	if ( puppet_bot_cmd != NULL )
	{
		if ( !puppet_bot_cmd->IsFlagSet(FCVAR_CHEAT) )
		{
			int *m_nFlags = (int*)((unsigned long)puppet_bot_cmd + BOT_CONVAR_FLAGS_OFFSET); // 20 is offset to flags
			
			*m_nFlags |= FCVAR_CHEAT;
		}
	}

	ConVar_Unregister( );

	return true;
}

void AFKBot::OnVSPListening(IServerPluginCallbacks *iface)
{
	vsp_callbacks = iface;
}

void AFKBot::Hook_ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	META_LOG(g_PLAPI, "ServerActivate() called: edictCount = %d, clientMax = %d", edictCount, clientMax);

	//CAccessClients::load();

	CBotGlobals::setClientMax(clientMax);
}

void AFKBot::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

void AFKBot::Hook_ClientActive(edict_t *pEntity, bool bLoadGame)
{
	META_LOG(g_PLAPI, "Hook_ClientActive(%d, %d)", IndexOfEdict(pEntity), bLoadGame);

	CClients::clientActive(pEntity);
}

#if SOURCE_ENGINE >= SE_ORANGEBOX
void AFKBot::Hook_ClientCommand(edict_t *pEntity, const CCommand &args)
#else
void AFKBot::Hook_ClientCommand(edict_t *pEntity)
#endif
{
	static CBotMod *pMod = NULL;

#if SOURCE_ENGINE <= SE_DARKMESSIAH
	CCommand args;
#endif

	const char *pcmd = args.Arg(0);

	if (!pEntity || pEntity->IsFree())
	{
		return;
	}

	CClient *pClient = CClients::get(pEntity);

	// is bot command?
	if ( CBotGlobals::m_pCommands->isCommand(pcmd) )
	{
		eBotCommandResult iResult = CBotGlobals::m_pCommands->execute(pClient,args.Arg(1),args.Arg(2),args.Arg(3),args.Arg(4),args.Arg(5),args.Arg(6));

		if ( iResult == COMMAND_ACCESSED )
		{
			// ok
		}
		else if ( iResult == COMMAND_REQUIRE_ACCESS )
		{
			CBotGlobals::botMessage(pEntity,0,"You do not have access to this command");
		}
		else if ( iResult == COMMAND_NOT_FOUND )
		{
			CBotGlobals::botMessage(pEntity,0,"bot command not found");	
		}
		else if ( iResult == COMMAND_ERROR )
		{
			CBotGlobals::botMessage(pEntity,0,"bot command returned an error");	
		}

		RETURN_META(MRES_SUPERCEDE);
	}
	else if ( strncmp(pcmd,"menuselect",10) == 0 ) // menu command
	{
		if ( pClient->isUsingMenu() )
		{
			int iCommand = atoi(args.Arg(1));

			// format is 1.2.3.4.5.6.7.8.9.0
			if ( iCommand == 0 )
				iCommand = 9;
			else
				iCommand --;

			pClient->getCurrentMenu()->selectedMenu(pClient,iCommand);
		}
	}

	// command capturing
	pMod = CBotGlobals::getCurrentMod();

	// capture some client commands e.g. voice commands
	pMod->clientCommand(pEntity,args.ArgC(),pcmd,args.Arg(1),args.Arg(2));

	RETURN_META(MRES_IGNORED); 
}

void AFKBot::Hook_ClientSettingsChanged(edict_t *pEdict)
{

}

bool AFKBot::Hook_ClientConnect(edict_t *pEntity,
									const char *pszName,
									const char *pszAddress,
									char *reject,
									int maxrejectlen)
{
	META_LOG(g_PLAPI, "Hook_ClientConnect(%d, \"%s\", \"%s\")", IndexOfEdict(pEntity), pszName, pszAddress);

	CClients::init(pEntity);

	return true;
}

void AFKBot::Hook_ClientPutInServer(edict_t *pEntity, char const *playername)
{
	CBaseEntity *pEnt = servergameents->EdictToBaseEntity(pEntity);

	CClient *pClient = CClients::clientConnected(pEntity);

	/*if ( CBots::controlBots() )
		is_Rcbot = CBots::handlePlayerJoin(pEntity,playername);
	
	if ( !is_Rcbot && pClient )
	{	
		if ( !engine->IsDedicatedServer() )
		{
			if ( CClients::noListenServerClient() )
			{
				// give listenserver client all access to bot commands
				CClients::setListenServerClient(pClient);		
				pClient->setAccessLevel(CMD_ACCESS_ALL);
				pClient->resetMenuCommands();
			}
		}
	}*/

	CBotMod *pMod = CBotGlobals::getCurrentMod();

	pMod->playerSpawned(pEntity);

	if ( pEnt )
	{
		//if (CBots::controlBots())
		SH_ADD_MANUALHOOK_MEMFUNC(MHook_PlayerRunCmd, pEnt, this, &AFKBot::Hook_PlayerRunCmd, false);
	}
}

void AFKBot::Hook_ClientDisconnect(edict_t *pEntity)
{
	CBaseEntity *pEnt = servergameents->EdictToBaseEntity(pEntity);

	if ( pEnt )
	{
		CBotMod *pMod = CBotGlobals::getCurrentMod();

		//if (CBots::controlBots())
		SH_REMOVE_MANUALHOOK_MEMFUNC(MHook_PlayerRunCmd, pEnt, this, &AFKBot::Hook_PlayerRunCmd, false);
	}

	CClients::clientDisconnected(pEntity);

	META_LOG(g_PLAPI, "Hook_ClientDisconnect(%d)", IndexOfEdict(pEntity));
}

void AFKBot::Hook_GameFrame(bool simulating)
{
	/**
	 * simulating:
	 * ***********
	 * true  | game is ticking
	 * false | game is not ticking
	 */

	static CBotMod *currentmod;

	if ( simulating && CBotGlobals::IsMapRunning() )
	{
		CBots::botThink();
		//if ( !CBots::controlBots() )
			//gameclients->PostClientMessagesSent();
		CClients::clientThink();

		if ( CWaypoints::getVisiblity()->needToWorkVisibility() )
		{
			CWaypoints::getVisiblity()->workVisibility();
		}

		// Profiling
#ifdef _DEBUG
		if ( CClients::clientsDebugging(BOT_DEBUG_PROFILE) )
		{
			CProfileTimers::updateAndDisplay();
		}
#endif

		// Config Commands
		CBotConfigFile::doNextCommand();
		currentmod = CBotGlobals::getCurrentMod();

		currentmod->modFrame();
	}
}

bool AFKBot::Hook_LevelInit(const char *pMapName,
								char const *pMapEntities,
								char const *pOldLevel,
								char const *pLandmarkName,
								bool loadGame,
								bool background)
{
	META_LOG(g_PLAPI, "Hook_LevelInit(%s)", pMapName);

		//CClients::initall();
	// Must set this
	CBotGlobals::setMapName(pMapName);

	Msg( "Level \"%s\" has been loaded\n", pMapName );

	CWaypoints::precacheWaypointTexture();

	CWaypointDistances::reset();

	CProfileTimers::reset();

	CWaypoints::init();
	CWaypoints::load();

	CBotGlobals::setMapRunning(true);
	CBotConfigFile::reset();
	
	if ( mp_teamplay )
		CBotGlobals::setTeamplay(mp_teamplay->GetBool());
	else
		CBotGlobals::setTeamplay(false);

	CBotEvents::setupEvents();

	CBots::mapInit();

	CBotMod *pMod = CBotGlobals::getCurrentMod();
	
	if ( pMod )
		pMod->mapInit();

	CBotSquads::FreeMemory();

	CClients::setListenServerClient(NULL);

	// Setup game rules
	extern void **g_pGameRules;

	if (g_pGameRules_Obj && g_pGameRules_Obj->found())
	{
		g_pGameRules = g_pGameRules_Obj->getGameRules();
	}
	else if (g_pGameRules_Create_Obj && g_pGameRules_Create_Obj->found())
	{
		g_pGameRules = g_pGameRules_Create_Obj->getGameRules();
	}

	return true;
}

void AFKBot::Hook_LevelShutdown()
{
	META_LOG(g_PLAPI, "Hook_LevelShutdown()");

	CClients::initall();
	CWaypointDistances::save();

	CBots::freeMapMemory();	
	CWaypoints::init();

	CBotGlobals::setMapRunning(false);
	CBotEvents::freeMemory();
}

void AFKBot::Hook_SetCommandClient(int index)
{
	// META_LOG(g_PLAPI, "Hook_SetCommandClient(%d)", index);
}

bool AFKBot::Pause(char *error, size_t maxlen)
{
	return true;
}

bool AFKBot::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *AFKBot::GetLicense()
{
	return "GPL General Public License v3";
}

const char *AFKBot::GetVersion()
{
	return "1.0.0";
}

const char *AFKBot::GetDate()
{
	return __DATE__;
}

const char *AFKBot::GetLogTag()
{
	return "AFKBOT";
}

const char *AFKBot::GetAuthor()
{
	return "Deathreus, Cheeseh, Nightc0re";
}

const char *AFKBot::GetDescription()
{
	return "AFK player bot for most source games";
}

const char *AFKBot::GetName()
{
	return "AFK Bot";
}

const char *AFKBot::GetURL()
{
	return "";
}
