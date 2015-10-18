
#include "icvar.h"
//#include "iconvar.h"
#include "convar.h"

#include "bot_cvars.h"

static ICvar *s_pCVar;

ConVar rcbot_tf2_debug_spies_cloakdisguise("rcbot_tf2_debug_spies_cloakdisguise","1",0,"Debug command : allow spy bots to cloak and disguise");
ConVar rcbot_tf2_medic_letgotime("rcbot_tf2_medic_letgotime","0.4",0,"Time for medic to let go of medigun to switch players");
ConVar rcbot_const_round_offset("rcbot_const_round_offset","844",0,"TF2 OFFSET for Round Class");
ConVar rcbot_const_point_master_offset("rcbot_const_mstr_offset","844",0,"TF2 OFFSET for Point Master Class");
//ConVar rcbot_const_point_offset("rcbot_const_pnt_offset","1140",0,"TF2 OFFSET for Point Class");
//ConVar rcbot_const_point_data_offset("rcbot_const_pnt_data_offset","1642",0,"TF2 OFFSET for Point Class data");
ConVar rcbot_tf2_pyro_airblast("rcbot_tf2_pyro_airblast_ammo","50",0,"Ammo must be above this to airblast -- if 200 airblast will be disabled");
ConVar rcbot_projectile_tweak("rcbot_projtweak","0.05",0,"Tweaks the bots knowledge of projectiles and gravity");
//ConVar bot_sv_cheat_warning("rcbot_sv_cheats_warning","0",0,"If disabled, bots will try to spawn even if sv_cheats is 0 - use only with admin cvar plugin");
ConVar bot_cmd_enable_wpt_sounds("rcbot_enable_wpt_sounds","1",0,"Enable/disable sound effects when editing waypoints");
//ConVar bot_cmd_nocheats("rcbot_botcmd_nocheats","1",0,"If 1 bots don't need cheats to play");
ConVar bot_general_difficulty("rcbot_skill","0.6",0,"General difficulty of the bots. 0.5 = stock, < 0.5 easier, > 0.5 = harder");
//ConVar bot_sv_cheats_auto("rcbot_sv_cheats_auto","0",0,"automatically put sv_cheats on and off for when adding bots only");
ConVar bot_visrevs_clients("rcbot_visrevs_clients","4",0,"how many revs the bot searches for visible players and enemies, lower to reduce cpu usage");
ConVar bot_spyknifefov("rcbot_spyknifefov","80",0,"the FOV from the enemy that spies must backstab from");
ConVar bot_visrevs("rcbot_visrevs","9",0,"how many revs the bot searches for visible monsters, lower to reduce cpu usage min:5");
ConVar bot_pathrevs("rcbot_pathrevs","40",0,"how many revs the bot searches for a path each frame, lower to reduce cpu usage, but causes bots to stand still more");
ConVar bot_command("rcbot_cmd","",0,"issues a command to all bots");
ConVar bot_attack( "rcbot_flipout", "0", 0, "Rcbots all attack" );
ConVar bot_scoutdj( "rcbot_scoutdj", "0.28", 0, "time scout uses to double jump" );
ConVar bot_anglespeed( "rcbot_anglespeed", "0.21", 0, "smaller number will make bots turn slower (1 = instant turn but may overshoot)" );
ConVar bot_stop( "rcbot_stop", "0", 0, "Make bots stop thinking!");
ConVar bot_waypointpathdist("rcbot_wpt_pathdist","400",0,"Length for waypoints to automatically add paths at");
ConVar bot_rj("rcbot_rj","0.01",0,"time for soldier to fire rocket after jumping");
ConVar bot_defrate("rcbot_defrate","0.24",0,"rate for bots to defend");
ConVar bot_beliefmulti("rcbot_beliefmulti","20.0",0,"multiplier for increasing bot belief");
ConVar bot_belief_fade("rcbot_belief_fade","0.75",0,"the multiplayer rate bot belief decreases");
ConVar bot_change_class("rcbot_change_classes","0",0,"bots change classes at random intervals");
ConVar bot_use_vc_commands("rcbot_voice_cmds","1",0,"bots use voice commands e.g. medic/spy etc");
ConVar bot_use_disp_dist("rcbot_disp_dist","800.0",0,"distance that bots will go back to use a dispenser");
ConVar bot_max_cc_time("rcbot_max_cc_time","240",0,"maximum time for bots to consider changing class <seconds>");
ConVar bot_min_cc_time("rcbot_min_cc_time","60",0,"minimum time for bots to consider changing class <seconds>");
ConVar bot_avoid_radius("rcbot_avoid_radius","80",0,"radius in units for bots to avoid things");
ConVar bot_avoid_strength("rcbot_avoid_strength","100",0,"strength of avoidance (0 = disable)");
ConVar bot_messaround("rcbot_messaround","1",0,"bots mess around at start up");
ConVar bot_heavyaimoffset("rcbot_heavyaimoffset","0.1",0,"fraction of how much the heavy aims at a diagonal offset");
ConVar bot_aimsmoothing("rcbot_aimsmooting","1",0,"(0 = no smoothing)");
ConVar bot_bossattackfactor("rcbot_bossattackfactor","1.0",0,"the higher the more often the bots will shoot the boss");
ConVar rcbot_enemyshootfov("rcbot_enemyshootfov","0.97",0,"the fov dot product before the bot shoots an enemy 0.7 = 45 degrees");
ConVar rcbot_enemyshoot_gravgun_fov("rcbot_enemyshoot_gravgun_fov","0.98",0,"the fov dot product before the bot shoots an enemy 0.98 = 11 degrees");
ConVar rcbot_wptplace_width("rcbot_wpt_width","48",0,"width of the player, automatic paths won't connect unless there is enough space for a player");
ConVar rcbot_wpt_autoradius("rcbot_wpt_autoradius","0",0,"every waypoint added is given this radius, 0 = no radius");
ConVar rcbot_wpt_autotype("rcbot_wpt_autotype","1",0,"If 1, types will be automatically added to waypoints when they are added (only for resupply/health/capture/flag etc)\nIf 2: types will autoamtically be added even if the waypoint is cut/paste");
ConVar rcbot_move_sentry_time("rcbot_move_sentry_time","120",0,"seconds for bots to start thinking about moving sentries");
ConVar rcbot_move_sentry_kpm("rcbot_move_sentry_kpm","1",0,"kpm = kills per minute, if less than this, bots will think about moving the sentry");
ConVar rcbot_smoke_time("rcbot_smoke_time","10",0,"seconds a smoke grenade stays active");
ConVar rcbot_move_disp_time("rcbot_move_disp_time","120",0,"seconds for bots to start thinking about moving dispensers");
ConVar rcbot_move_disp_healamount("rcbot_move_disp_healamount","100",0,"if dispenser heals less than this per minute, bot will move the disp");
ConVar rcbot_demo_runup_dist("rcbot_demo_runup","99.0",0,"distance the demo bot will take to run up for a pipe jump");
ConVar rcbot_demo_jump("rcbot_enable_pipejump","1",0,"Enable experimental pipe jumping at rocket jump waypoints");
ConVar rcbot_move_tele_time("rcbot_move_tele_time","120",0,"seconds for bots to start thinking about moving teleporters");
ConVar rcbot_move_tele_tpm("rcbot_move_tele_tpm","1",0,"if no of players teleported per minute is less than this, bot will move the teleport");
ConVar rcbot_tf2_protect_cap_time("rcbot_tf2_prot_cap_time","12.5",0,"time that the bots will spen more attention to the cap point if attacked");
ConVar rcbot_tf2_protect_cap_percent("rcbot_tf2_protect_cap_percent","0.25",0,"the percentage that bots defend the capture point by standing on the point");
ConVar rcbot_tf2_spy_kill_on_cap_dist("rcbot_tf2_spy_kill_on_cap_dist","200.0",0,"the distance for spy bots to attack players capturing a point");
ConVar rcbot_move_dist("rcbot_move_dist","800",0,"minimum distance to move objects to");
ConVar rcbot_shoot_breakables("rcbot_shoot_breakables","1",0,"if 1, bots will shoot breakable objects");
ConVar rcbot_shoot_breakable_dist("rcbot_shoot_breakable_dist","128.0",0,"The distance bots will shoot breakables at");
ConVar rcbot_shoot_breakable_cos("rcbot_shoot_breakable_cos","0.9",0,"The cosine of the angle bots should worry about breaking objects at (default = 0.9) ~= 25 degrees");
ConVar rcbot_move_obj("rcbot_move_obj","1",0,"if 1 rcbot engineers will move objects around");
ConVar rcbot_taunt("rcbot_taunt","0",0,"enable/disable bots taunting");
ConVar rcbot_notarget("rcbot_notarget","0",0,"bots don't shoot the host!");
ConVar rcbot_nocapturing("rcbot_dontcapture","0",0,"bots don't capture flags in DOD:S");
ConVar rcbot_jump_obst_dist("rcbot_jump_obst_dist","80",0,"the distance from an obstacle the bot will jump");
ConVar rcbot_jump_obst_speed("rcbot_jump_obst_speed","100",0,"the speed of the bot for the bot to jump an obstacle");
ConVar rcbot_speed_boost("rcbot_speed_boost","1",0,"multiplier for bots speed");
ConVar rcbot_melee_only("rcbot_melee_only","0",0,"if 1 bots will only use melee weapons");
ConVar rcbot_debug_iglev("rcbot_debug_iglev","0",0,"bot think ignores functions to test cpu speed");
ConVar rcbot_dont_move("rcbot_dontmove","0",0,"if 1 , bots will all move forward");
ConVar rcbot_runplayercmd_dods("rcbot_runplayer_cmd_dods","417",0,"offset of the DOD:S PlayerRunCommand function");
ConVar rcbot_runplayercmd_tf2("rcbot_runplayer_cmd_tf2","418",0,"offset of the TF2 PlayerRunCommand function");
ConVar rcbot_runplayercmd_hookonce("rcbot_runplayer_hookonce","1",0,"function will hook only once, if 0 it will unook and rehook after every map");
ConVar rcbot_ladder_offs("rcbot_ladder_offs","42",0,"difference in height for bot to think it has touched the ladder waypoint");
ConVar rcbot_ffa("rcbot_ffa","0",0,"Free for all mode -- bots shoot everyone");
ConVar rcbot_prone_enemy_only("rcbot_prone_enemy_only","1",0,"if 1 bots only prone in DOD:S when they have an enemy");
ConVar rcbot_menu_update_time1("rcbot_menu_update_time1","0.04",0,"time to update menus [displaying message]");
ConVar rcbot_menu_update_time2("rcbot_menu_update_time2","0.2",0,"time to update menus [interval]");
ConVar rcbot_autowaypoint_dist("rcbot_autowpt_dist","150.0",0,"distance for autowaypoints to be placed");
ConVar rcbot_stats_inrange_dist("rcbot_stats_inrange_dist","320.0",0,"distance for bots to realise they have other players in range (for particular radio commands in DOD:S)");
ConVar rcbot_squad_idle_time("rcbot_squad_idle_time","3.0",0,"time for bots to do other things if squad leader is idle for a short time");
ConVar rcbot_bots_form_squads("rcbot_bots_form_squads","1",0,"if 1, bots will form their own squads via voice commands");
ConVar rcbot_listen_dist("rcbot_listen_dist","512",0,"the distance for bots to hear things");
ConVar rcbot_footstep_speed("rcbot_footstep_speed","250",0,"the speed players can go when you first hear them make footsteps");
ConVar rcbot_bot_squads_percent("rcbot_bot_squads_percent","50",0,"the percentage of time bots make squads with other bots");
ConVar rcbot_tooltips("rcbot_tooltips","1",0,"Enables/disables help tooltips");
ConVar rcbot_debug_notasks("rcbot_debug_notasks","0",0,"Debug command, stops bots from doing tasks by themselves");
ConVar rcbot_debug_dont_shoot("rcbot_debug_dont_shoot","0",0,"Debug command, stops bots from shooting everyone");
ConVar rcbot_debug_show_route("rcbot_debug_show_route","0",0,"Debug command, shows waypoint route to host");
ConVar rcbot_tf2_autoupdate_point_time("rcbot_tf2_autoupdate_point_time","60",0,"Time to automatically update points in TF2 for any changes");
ConVar rcbot_tf2_payload_dist_retreat("rcbot_tf2_payload_dist_retreat","512.0",0,"Distance for payload bomb to be greater than at cap before defend team retreats");
ConVar rcbot_spy_runaway_health("rcbot_spy_runaway_health","70",0,"health which spies run away after attacking");
ConVar rcbot_supermode("rcbot_supermode","0",0,"If 1 will make every bot skill and reaction much higher");
ConVar rcbot_addbottime("rcbot_addbottime","0.83",0,"The time in seconds for bots to be added after another");
ConVar rcbot_customloadouts("rcbot_customloadouts","0",0,"if 1 bots can use custom weapons");
ConVar rcbot_givenameditem_offset("rcbot_givenameditem_offset","471",0,"offset of the GiveNamedItem function");
ConVar rcbot_equipwearable_offset("rcbot_equipwearable_offset","426",0,"offset of the EquipWearable function");
ConVar rcbot_rmplayeritem_offset("rcbot_rmplayeritem_offset","270",0,"offset of the RemovePlayerItem function");
ConVar rcbot_enable_attributes("rcbot_enable_attributes","1",0,"Enable/disable attributes on TF2 weapon loadouts");
ConVar rcbot_force_generation("rcbot_force_generation","0",0,"force generation of weapons");
ConVar rcbot_equiphats("rcbot_equiphats", "0", 0, "Allow bots to quip a random hat (unstable)");
ConVar rcbot_getweaponslot_offset("rcbot_getweaponslot_offset", "268", 0, "offset of the getweaponslot function");
ConVar rcbot_removewearable_offset("rcbot_removewearable_offset", "427", 0, "offset of the removewearable function");
ConVar rcbot_weaponequip_offset("rcbot_weaponequip_offset", "261", 0, "offset for weapon equip function");
ConVar rcbot_gamerules_offset("rcbot_gamerules_offset", "5", 0, "offset for gamerules object");
ConVar rcbot_weaponswitch_offset("rcbot_weaponswitch_offset", "264", 0, "offset for weapon Switch function");
//ConVar rcbot_bot_add_cmd("rcbot_bot_add_cmd","bot",0,"command to add puppet bots");
//ConVar rcbot_bot_add_cmd("rcbot_hook_engine","1",0,"command to add puppet bots");
ConVar *sv_gravity = NULL;
ConVar *sv_cheats = NULL;//("sv_cheats");
ConVar *mp_teamplay = NULL;
ConVar *sv_tags = NULL;
ConVar *mp_friendlyfire = NULL;
ConCommandBase *puppet_bot_cmd = NULL;
ConVar *mp_stalemate_enable = NULL;
ConVar *mp_stalemate_meleeonly = NULL;

void RCBOT2_Cvar_setup (ICvar *cvar)
{
	mp_stalemate_enable = cvar->FindVar("mp_stalemate_enable");
	mp_stalemate_meleeonly = cvar->FindVar("mp_stalemate_meleeonly");
	sv_cheats = cvar->FindVar("sv_cheats");
	sv_gravity = cvar->FindVar("sv_gravity");
	mp_friendlyfire = cvar->FindVar("mp_friendlyfire");
	sv_tags = cvar->FindVar("sv_tags");
	puppet_bot_cmd = cvar->FindCommand("bot");
	mp_teamplay = cvar->FindVar("mp_teamplay");

	if ( sv_tags != NULL )
	{
		char sv_tags_str[512];
	
		strcpy(sv_tags_str,sv_tags->GetString());

		// fix
		if ( strstr(sv_tags_str,"rcbot2") == NULL )
		{

			if ( sv_tags_str[0] == 0 )
				strcat(sv_tags_str,"rcbot2");
			else
				strcat(sv_tags_str,",rcbot2");

			sv_tags->SetValue(sv_tags_str);

		}
	}

}
