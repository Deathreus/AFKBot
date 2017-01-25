
#include "convar.h"

#include "bot_cvars.h"

ConVar bot_attack("afkbot_flipout", "0", 0, "bots all attack", true, 0.0f, true, 1.0f);
ConVar bot_stop("afkbot_stop", "0", 0, "Make bots stop thinking!", true, 0.0f, true, 1.0f);
ConVar bot_waypointpathdist("afkbot_wpt_pathdist", "400", 0, "Length for waypoints to automatically add paths at");
ConVar bot_avoid_radius("afkbot_avoid_radius", "80", 0, "radius in units for bots to avoid things");
ConVar bot_avoid_strength("afkbot_avoid_strength", "100", 0, "strength of avoidance (0 = disable)");
ConVar bot_messaround("afkbot_messaround", "1", 0, "bots mess around at start up", true, 0.0f, true, 1.0f);
ConVar bot_aimsmoothing("afkbot_aimsmoothing", "1", 0, "(0 = no smoothing)", true, 0.0f, true, 1.0f);
ConVar bot_wptplace_width("afkbot_wpt_width", "48", 0, "width of the player, automatic paths won't connect unless there is enough space for a player");
ConVar bot_smoke_time("afkbot_smoke_time", "10", 0, "seconds a smoke grenade stays active");
ConVar bot_shoot_breakables("afkbot_shoot_breakables", "1", 0, "if 1, bots will shoot breakable objects", true, 0.0f, true, 1.0f);
ConVar bot_shoot_breakable_dist("afkbot_shoot_breakable_dist", "128.0", 0, "The distance bots will shoot breakables at");
ConVar bot_shoot_breakable_cos("afkbot_shoot_breakable_cos", "0.9", 0, "The cosine of the angle bots should worry about breaking objects at (default = 0.9) ~= 25 degrees");
ConVar bot_notarget("afkbot_notarget", "0", 0, "bots don't shoot the host!", true, 0.0f, true, 1.0f);
ConVar bot_jump_obst_dist("afkbot_jump_obst_dist", "80", 0, "the distance from an obstacle the bot will jump");
ConVar bot_jump_obst_speed("afkbot_jump_obst_speed", "100", 0, "the speed of the bot for the bot to jump an obstacle");
ConVar bot_speed_boost("afkbot_speed_boost", "1", 0, "multiplier for bots speed");
ConVar bot_melee_only("afkbot_melee_only", "0", 0, "if 1 bots will only use only melee weapons", true, 0.0f, true, 1.0f);
ConVar bot_debug_iglev("afkbot_debug_iglev", "0", 0, "bot think ignores functions to test cpu speed", true, 0.0f, true, 1.0f);
ConVar bot_dont_move("afkbot_dontmove", "0", 0, "if 1, bots will all stop moving", true, 0.0f, true, 1.0f);
ConVar bot_runplayercmd_hookonce("afkbot_runplayer_hookonce", "1", 0, "function will hook only once, if 0 it will unook and rehook after every map", true, 0.0f, true, 1.0f);
ConVar bot_ladder_offs("afkbot_ladder_offs", "42", 0, "difference in height for bot to think it has touched the ladder waypoint");
ConVar bot_ffa("afkbot_ffa", "0", 0, "Free for all mode -- bots shoot everyone", true, 0.0f, true, 1.0f);
ConVar bot_debug_notasks("afkbot_debug_notasks", "0", 0, "Debug command, stops bots from doing tasks by themselves", true, 0.0f, true, 1.0f);
ConVar bot_debug_dont_shoot("afkbot_debug_dont_shoot", "0", 0, "Debug command, stops bots from shooting everyone", true, 0.0f, true, 1.0f);
ConVar bot_debug_show_route("afkbot_debug_show_route", "0", 0, "Debug command, shows waypoint route to host", true, 0.0f, true, 1.0f);
ConVar bot_supermode("afkbot_supermode", "0", 0, "If 1 will make every bot skill and reaction much higher", true, 0.0f, true, 1.0f);
ConVar bot_printstatus("afkbot_printafkstatus", "1", 0, "If 1 will print to everyone when someone becomes a bot or not", true, 0.0f, true, 1.0f);


// DOD vars
ConVar bot_nocapturing("afkbot_dontcapture", "0", 0, "bots don't capture flags in DOD:S", true, 0.0f, true, 1.0f);
ConVar bot_prone_enemy_only("afkbot_prone_enemy_only", "1", 0, "if 1 bots only prone in DOD:S when they have an enemy", true, 0.0f, true, 1.0f);
ConVar bot_stats_inrange_dist("afkbot_stats_inrange_dist", "320.0", 0, "distance for bots to realise they have other players in range (for particular radio commands in DOD:S)");
ConVar bot_squad_idle_time("afkbot_squad_idle_time", "3.0", 0, "time for bots to do other things if squad leader is idle for a short time");
ConVar bot_bots_form_squads("afkbot_bots_form_squads", "1", 0, "if 1, bots will form their own squads via voice commands", true, 0.0f, true, 1.0f);
ConVar bot_bot_squads_percent("afkbot_bot_squads_percent", "50", 0, "the percentage of time bots make squads with other bots");


// TF2 vars
ConVar bot_tf2_debug_spies_cloakdisguise("afkbot_tf2_debug_spies_cloakdisguise", "1", 0, "Debug command : allow spy bots to cloak and disguise", true, 0.0f, true, 1.0f);
ConVar bot_tf2_medic_letgotime("afkbot_tf2_medic_letgotime", "0.4", 0, "Time for medic to let go of medigun to switch players");
ConVar bot_tf2_pyro_airblast("afkbot_tf2_pyro_airblast_ammo", "50", 0, "Ammo must be above this to airblast -- if 200 airblast will be disabled");
ConVar bot_spyknifefov("afkbot_spyknifefov", "80", 0, "the FOV from the enemy that spies must backstab from");
ConVar bot_scoutdj("afkbot_scoutdj", "0.28", 0, "time scout uses to double jump");
ConVar bot_rj("afkbot_rj", "0.01", 0, "time for soldier to fire rocket after jumping");
ConVar bot_change_class("afkbot_change_classes", "0", 0, "bots change classes at random intervals", true, 0.0f, true, 1.0f);
ConVar bot_use_vc_commands("afkbot_voice_cmds", "1", 0, "bots use voice commands e.g. medic/spy etc", true, 0.0f, true, 1.0f);
ConVar bot_use_disp_dist("afkbot_disp_dist", "800.0", 0, "distance that bots will go back to use a dispenser");
ConVar bot_max_cc_time("afkbot_max_cc_time", "240", 0, "maximum time for bots to consider changing class <seconds>");
ConVar bot_min_cc_time("afkbot_min_cc_time", "60", 0, "minimum time for bots to consider changing class <seconds>");
ConVar bot_heavyaimoffset("afkbot_heavyaimoffset", "0.1", 0, "fraction of how much the heavy aims at a diagonal offset");
ConVar bot_move_sentry_time("afkbot_move_sentry_time", "120", 0, "seconds for bots to start thinking about moving sentries");
ConVar bot_move_sentry_kpm("afkbot_move_sentry_kpm", "1", 0, "kpm = kills per minute, if less than this, bots will think about moving the sentry");
ConVar bot_move_disp_time("afkbot_move_disp_time", "120", 0, "seconds for bots to start thinking about moving dispensers");
ConVar bot_move_disp_healamount("afkbot_move_disp_healamount", "100", 0, "if dispenser heals less than this per minute, bot will move the disp");
ConVar bot_demo_runup_dist("afkbot_demo_runup", "99.0", 0, "distance the demo bot will take to run up for a pipe jump");
ConVar bot_demo_jump("afkbot_enable_pipejump", "1", 0, "Enable experimental pipe jumping at rocket jump waypoints", true, 0.0f, true, 1.0f);
ConVar bot_move_tele_time("afkbot_move_tele_time", "120", 0, "seconds for bots to start thinking about moving teleporters");
ConVar bot_move_tele_tpm("afkbot_move_tele_tpm", "1", 0, "if no of players teleported per minute is less than this, bot will move the teleport");
ConVar bot_tf2_protect_cap_time("afkbot_tf2_prot_cap_time", "12.5", 0, "time that the bots will spen more attention to the cap point if attacked");
ConVar bot_tf2_protect_cap_percent("afkbot_tf2_prot_cap_percent", "0.25", 0, "the percentage that bots defend the capture point by standing on the point");
ConVar bot_tf2_spy_kill_on_cap_dist("afkbot_tf2_spy_kill_on_cap_dist", "200.0", 0, "the distance for spy bots to attack players capturing a point");
ConVar bot_move_dist("afkbot_move_dist", "800", 0, "minimum distance to move objects to");
ConVar bot_move_obj("afkbot_move_obj", "1", 0, "if 1 afkbot engineers will move objects around", true, 0.0f, true, 1.0f);
ConVar bot_taunt("afkbot_taunt", "0", 0, "enable/disable bots taunting", true, 0.0f, true, 1.0f);
ConVar bot_tf2_autoupdate_point_time("afkbot_tf2_autoupdate_point_time", "60", 0, "Time to automatically update points in TF2 for any changes");
ConVar bot_tf2_payload_dist_retreat("afkbot_tf2_payload_dist_retreat", "512.0", 0, "Distance for payload bomb to be greater than at cap before defend team retreats");
ConVar bot_spy_runaway_health("afkbot_spy_runaway_health", "70", 0, "health which spies run away after attacking");

// Default offsets
ConVar bot_const_point_master_offset("afkbot_const_mstr_offset", "856", 0, "TF2 OFFSET for Point Master Class");
ConVar bot_const_round_offset("afkbot_const_round_offset", "856", 0, "TF2 OFFSET for Round Class");

// Default skill settings
ConVar bot_skill_randomize("afkbot_randomizeskill", "1", FCVAR_NOTIFY, "randomize a bots skill level on activation?", true, 0.0f, true, 1.0f);
ConVar bot_skill_min("afkbot_aimskill_min", "0.5", FCVAR_NOTIFY, "minimum aim skill level", true, 0.0f, true, 1.0f);
ConVar bot_skill_max("afkbot_aimskill_max", "1.0", FCVAR_NOTIFY, "maximum aim skill level", true, 0.0f, true, 1.0f);
ConVar bot_sensitivity_min("afkbot_sensitivity_min", "8.0", FCVAR_NOTIFY, "minimum turning sensitivity", true, 0.0f, false, 0.0f);
ConVar bot_sensitivity_max("afkbot_sensitivity_max", "12.0", FCVAR_NOTIFY, "maximum turning sensitivity", true, 0.0f, false, 0.0f);
ConVar bot_braveness_min("afkbot_braveness_min", "0.5", FCVAR_NOTIFY, "minimum braveness", true, 0.0f, true, 1.0f);
ConVar bot_braveness_max("afkbot_braveness_max", "1.0", FCVAR_NOTIFY, "maximum braveness", true, 0.0f, true, 1.0f);
ConVar bot_visrevs_min("afkbot_visrevs_min", "4", FCVAR_NOTIFY, "minimum vision revisions", true, 0.0f, false, 0.0f);
ConVar bot_visrevs_max("afkbot_visrevs_max", "8", FCVAR_NOTIFY, "maximum vision revisions", true, 0.0f, false, 0.0f);
ConVar bot_visrevs_client_min("afkbot_visrevs_client_min", "6", FCVAR_NOTIFY, "minimum client vision revisions", true, 0.0f, false, 0.0f);
ConVar bot_visrevs_client_max("afkbot_visrevs_client_max", "9", FCVAR_NOTIFY, "maximum client vision revisions", true, 0.0f, false, 0.0f);
ConVar bot_pathrevs_min("afkbot_pathrevs_min", "128", FCVAR_NOTIFY, "minimum path revisions", true, 0.0f, false, 0.0f);
ConVar bot_pathrevs_max("afkbot_pathrevs_max", "160", FCVAR_NOTIFY, "maximum path revisions", true, 0.0f, false, 0.0f);

// TODO : Make these per bot?
ConVar bot_defrate("afkbot_defrate", "0.24", FCVAR_NOTIFY, "rate for bots to defend");
ConVar bot_beliefmulti("afkbot_beliefmulti", "20.0", FCVAR_NOTIFY, "multiplier for increasing bot belief");
ConVar bot_belief_fade("afkbot_belief_fade", "0.75", FCVAR_NOTIFY, "the multiplayer rate bot belief decreases");
ConVar bot_projectile_tweak("afkbot_projtweak", "0.05", FCVAR_NOTIFY, "Tweaks the bots knowledge of projectiles and gravity");
ConVar bot_general_difficulty("afkbot_skill", "0.6", FCVAR_NOTIFY, "General difficulty of the bots. 0.5 = stock, < 0.5 easier, > 0.5 = harder");
ConVar bot_bossattackfactor("afkbot_bossattackfactor", "1.0", FCVAR_NOTIFY, "the higher the more often the bots will shoot the boss");
ConVar bot_enemyshootfov("afkbot_enemyshootfov", "0.97", FCVAR_NOTIFY, "the fov dot product before the bot shoots an enemy 0.7 = 45 degrees");
ConVar bot_enemyshoot_gravgun_fov("afkbot_enemyshoot_gravgun_fov", "0.98", FCVAR_NOTIFY, "the fov dot product before the bot shoots an enemy 0.98 = 11 degrees");
ConVar bot_anglespeed("afkbot_anglespeed", "0.21", FCVAR_NOTIFY, "smaller number will make bots turn slower (1 = instant turn but may overshoot)");
ConVar bot_listen_dist("afkbot_listen_dist", "512", FCVAR_NOTIFY, "the distance for bots to hear things");
ConVar bot_footstep_speed("afkbot_footstep_speed", "250", FCVAR_NOTIFY, "the speed players can go when you first hear them make footsteps");


ConVar *sv_gravity = NULL;
ConVar *sv_cheats = NULL;
ConVar *mp_teamplay = NULL;
ConVar *sv_tags = NULL;
ConVar *mp_friendlyfire = NULL;
ConVar *mp_stalemate_enable = NULL;
ConVar *mp_stalemate_meleeonly = NULL;
