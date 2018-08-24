#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <tf2_stocks>
#include <afk_bot>

#undef REQUIRE_PLUGIN
#tryinclude <afk_manager>

#pragma newdecls required

public Plugin myinfo = 
{
	name 		= "AFK Bot Plugin",
	author 		= "Deathreus",
	description = "Supplimentary plugin to the extension",
	version 	= "1.0.0",
	url 		= "sourcemod.net"
}

public void OnPluginStart()
{
	CreateConVar("sm_afkbot_version", "1.0.0", "Plugin version.", FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY|FCVAR_DONTRECORD);
	
	RegAdminCmd("sm_afk", Command_AFK, ADMFLAG_RESERVATION);	// Can actually toggle, below is convenience
	RegConsoleCmd("sm_back", Command_Back);
	
	LoadTranslations("common.phrases.txt");
}

public void OnMapStart()
{
	#if defined _afk_manager_included
	CreateTimer(15.0, TellYourInAFKMODE, _, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
	#endif
}

public Action TellYourInAFKMODE(Handle hTimer)
{
	for(int iClient=MaxClients; iClient>0; iClient--)
	{
		if(IsValidClient(iClient) && IsClientAFKBot(iClient))
		{
			PrintToChat(iClient, "[AFK Bot] You are set AFK.\nType \"/back\" in chat to get out of it.");
			PrintCenterText(iClient, "You are being controlled by a bot. Type \"/back\" to exit.");
		}
	}
}

public Action Command_AFK(int iClient, int nArgs)
{
	if(!IsValidClient(iClient))
	{
		ReplyToCommand(iClient, "%t", "Command is in-game only");
		return Plugin_Handled;
	}
	
	if(nArgs != 0)
	{
		ReplyToCommand(iClient, "[SM] This command doesn't take arguments.");
		return Plugin_Handled;
	}
	
	if(IsClientAFKBot(iClient))
	{
		SetClientAFKBot(iClient, false);
	}
	else
	{
		SetClientAFKBot(iClient, true);
	}
	
	return Plugin_Handled;
}

public Action Command_Back(int iClient, int nArgs)
{
	if(!IsValidClient(iClient))
	{
		ReplyToCommand(iClient, "%t", "Command is in-game only");
		return Plugin_Handled;
	}
	
	if(nArgs != 0)
	{
		ReplyToCommand(iClient, "[SM] This command doesn't take arguments.");
		return Plugin_Handled;
	}
	
	if(IsClientAFKBot(iClient))
	{
		SetClientAFKBot(iClient, false);
		return Plugin_Handled;
	}
	
	return Plugin_Continue;
}

#if defined _afkmanager_included
public Action AFKM_OnAFKEvent(const char[] sName, int iClient)
{
	if(StrEqual(sName, "afk_spawn_move") || StrEqual(sName, "afk_move") || StrEqual(sName, "afk_kick"))
	{
		if(IsValidClient(iClient) && CheckCommandAccess(iClient, "sm_afk_access", 0))
		{
			LogMessage("[SM] Afk event enabled AFKBot for %L.", iClient);
			ForcePlayerSuicide(iClient);
		}

		return Plugin_Stop;
	}
	return Plugin_Continue;
}
#endif

stock bool IsValidClient(int iClient)
{
	return (0 < iClient <= MaxClients) && IsClientInGame(iClient) && !(IsClientSourceTV(iClient) || IsClientReplay(iClient) || IsFakeClient(iClient));
}
