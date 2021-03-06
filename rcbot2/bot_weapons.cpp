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
#include "engine_wrappers.h"

#include "bot_base.h"
#include "bot_globals.h"
#include "bot_weapons.h"
#include "bot_getprop.h"

#ifdef GetClassName
 #undef GetClassName
#endif

extern IFileSystem* filesystem;

const char *g_szTF2Weapons[] =
{
	"tf_weapon_bat",
	"tf_weapon_bottle",
	"tf_weapon_fireaxe",
	"tf_weapon_club",
	"tf_weapon_knife",
	"tf_weapon_fists",
	"tf_weapon_shovel",
	"tf_weapon_wrench",
	"tf_weapon_bonesaw",
	"tf_weapon_shotgun_primary",
	"tf_weapon_shotgun_soldier",
	"tf_weapon_shotgun_hwg",
	"tf_weapon_shotgun_pyro",
	"tf_weapon_scattergun",
	"tf_weapon_sniperrifle",
	"tf_weapon_minigun",
	"tf_weapon_smg",
	"tf_weapon_syringegun_medic",
	"tf_weapon_rocketlauncher",
	"tf_weapon_grenadelauncher",
	"tf_weapon_pipebomblauncher",
	"tf_weapon_flamethrower",
	"tf_weapon_pistol",
	"tf_weapon_pistol_scout",
	"tf_weapon_revolver",
	"tf_weapon_drg_pomson",
	"tf_weapon_sentry_revenge",
	"tf_weapon_builder",
	"tf_weapon_medigun",
	"tf_weapon_invis",
	"tf_weapon_buff_item",
	"tf_weapon_flaregun",
	"obj_sentrygun",
	"saxxy",
	"tf_weapon_bat_wood",
	"tf_weapon_lunchbox_drink",
	"tf_weapon_compound_bow",
	"tf_weapon_jar",
	"tf_weapon_bat_fish",
	"tf_weapon_rocketlauncher_directhit",
	"tf_weapon_sword",
	"tf_weapon_katana",
	"tf_weapon_particle_cannon",
	"tf_weapon_crossbow",
	"tf_weapon_cleaver",
	"tf_weapon_bat_giftwrap",
	"tf_weapon_raygun"
};

const char *g_szDODWeapons[] =
{
	"weapon_amerknife",
	"weapon_spade",
	"weapon_colt",
	"weapon_p38",
	"weapon_m1carbine",
	"weapon_c96",
	"weapon_garand",
	"weapon_k98",
	"weapon_thompson",
	"weapon_mp40",
	"weapon_bar",
	"weapon_mp44",
	"weapon_spring",
	"weapon_k98_scoped",
	"weapon_30cal",
	"weapon_mg42",
	"weapon_bazooka",
	"weapon_pschreck",
	"weapon_riflegren_us",
	"weapon_riflegren_ger",
	"weapon_frag_us",
	"weapon_frag_ger",
	"weapon_smoke_us",
	"weapon_smoke_ger",
	"weapon_basebomb"
};

const char *g_szHL2DMWeapons[] =
{
	"weapon_pistol",
	"weapon_crowbar",
	"weapon_357",
	"weapon_smg1",
	"weapon_ar2",
	"weapon_frag",
	"weapon_stunstick",
	"weapon_crossbow",
	"weapon_rpg",
	"weapon_slam",
	"weapon_shotgun",
	"weapon_physcannon"
};

WeaponsData_t DODWeaps[] =
{
	/*
		slot, id , weapon name, flags, min dist, max dist, ammo index, preference
		*/
	{ 1, DOD_WEAPON_AMERKNIFE, g_szDODWeapons[0], WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 100, -1, 1, 0 },
	{ 1, DOD_WEAPON_SPADE, g_szDODWeapons[1], WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 100, -1, 1, 0 },
	{ 2, DOD_WEAPON_COLT, g_szDODWeapons[2], WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 800, 1, 2, 0 },
	{ 2, DOD_WEAPON_P38, g_szDODWeapons[3], WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 800, 2, 2, 0 },
	{ 3, DOD_WEAPON_M1, g_szDODWeapons[4], WEAP_FL_PRIM_ATTACK, 0, 1600, 6, 4, 0 },
	{ 3, DOD_WEAPON_C96, g_szDODWeapons[5], WEAP_FL_PRIM_ATTACK, 0, 1600, -1, 4, 0 },
	{ 3, DOD_WEAPON_GARAND, g_szDODWeapons[6], WEAP_FL_PRIM_ATTACK | WEAP_FL_ZOOMABLE, 0, 1600, -1, 3, 0 },
	{ 3, DOD_WEAPON_K98, g_szDODWeapons[7], WEAP_FL_PRIM_ATTACK | WEAP_FL_ZOOMABLE, 0, 1600, -1, 3, 0 },
	{ 3, DOD_WEAPON_THOMPSON, g_szDODWeapons[8], WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE_SEC_ATT, 0, 900, -1, 3, 0 },
	{ 3, DOD_WEAPON_MP40, g_szDODWeapons[9], WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE_SEC_ATT, 0, 1600, -1, 4, 0 },
	{ 3, DOD_WEAPON_BAR, g_szDODWeapons[10], WEAP_FL_PRIM_ATTACK, 0, 1600, -1, 3, 0 },
	{ 3, DOD_WEAPON_MP44, g_szDODWeapons[11], WEAP_FL_PRIM_ATTACK, 0, 1600, -1, 3, 0 },
	{ 3, DOD_WEAPON_SPRING, g_szDODWeapons[12], WEAP_FL_PRIM_ATTACK | WEAP_FL_SCOPE | WEAP_FL_CANTFIRE_NORM | WEAP_FL_ZOOMABLE, 0, 3200, -1, 3, 0 },
	{ 3, DOD_WEAPON_K98_SCOPED, g_szDODWeapons[13], WEAP_FL_PRIM_ATTACK | WEAP_FL_SCOPE | WEAP_FL_CANTFIRE_NORM | WEAP_FL_ZOOMABLE, 0, 3200, -1, 4, 0 },
	{ 3, DOD_WEAPON_20CAL, g_szDODWeapons[14], WEAP_FL_PRIM_ATTACK | WEAP_FL_DEPLOYABLE | WEAP_FL_HIGH_RECOIL, 0, 2000, -1, 4, 0 },
	{ 3, DOD_WEAPON_MG42, g_szDODWeapons[15], WEAP_FL_PRIM_ATTACK | WEAP_FL_DEPLOYABLE | WEAP_FL_HIGH_RECOIL, 0, 2000, -1, 4, 0 },
	{ 3, DOD_WEAPON_BAZOOKA, g_szDODWeapons[16], WEAP_FL_PROJECTILE | WEAP_FL_EXPLOSIVE | WEAP_FL_PRIM_ATTACK | WEAP_FL_CANTFIRE_NORM | WEAP_FL_DEPLOYABLE, 500, 3200, -1, 5, 1300 },
	{ 3, DOD_WEAPON_PSCHRECK, g_szDODWeapons[17], WEAP_FL_PROJECTILE | WEAP_FL_EXPLOSIVE | WEAP_FL_PRIM_ATTACK | WEAP_FL_CANTFIRE_NORM | WEAP_FL_DEPLOYABLE, 500, 3200, -1, 5, 1300 },
	{ 3, DOD_WEAPON_RIFLEGREN_US, g_szDODWeapons[18], WEAP_FL_EXPLOSIVE_SEC | WEAP_FL_PRIM_ATTACK, 500, 1800, -1, 4, 0 },
	{ 3, DOD_WEAPON_RIFLEGREN_GER, g_szDODWeapons[19], WEAP_FL_EXPLOSIVE_SEC | WEAP_FL_PRIM_ATTACK, 500, 1800, -1, 4, 0 },
	{ 3, DOD_WEAPON_FRAG_US, g_szDODWeapons[20], WEAP_FL_PROJECTILE | WEAP_FL_GRENADE | WEAP_FL_EXPLOSIVE | WEAP_FL_NONE, 0, 1200, -1, 1, 0 },
	{ 3, DOD_WEAPON_FRAG_GER, g_szDODWeapons[21], WEAP_FL_PROJECTILE | WEAP_FL_GRENADE | WEAP_FL_EXPLOSIVE | WEAP_FL_NONE, 0, 1200, -1, 1, 0 },
	{ 3, DOD_WEAPON_SMOKE_US, g_szDODWeapons[22], WEAP_FL_PROJECTILE | WEAP_FL_GRENADE, 0, 1200, -1, 1, 0 },
	{ 3, DOD_WEAPON_SMOKE_GER, g_szDODWeapons[23], WEAP_FL_PROJECTILE | WEAP_FL_GRENADE, 0, 1200, -1, 1, 0 },
	{ 3, DOD_WEAPON_BOMB, g_szDODWeapons[24], WEAP_FL_NONE, 0, 0, -1, 1, 0 },
	{ 0, 0, "\0", 0, 0, 0, 0, 0, 0 }//signal last weapon
};


WeaponsData_t HL2DMWeaps[] =
{
	/*
		slot, id , weapon name, flags, min dist, max dist, ammo index, preference
		*/
	{ 2, HL2DM_WEAPON_PISTOL, g_szHL2DMWeapons[0], WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 1000, -1, 1, 0 },
	{ 1, HL2DM_WEAPON_CROWBAR, g_szHL2DMWeapons[1], WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 128, -1, 1, 0 },
	{ 2, HL2DM_WEAPON_357, g_szHL2DMWeapons[2], WEAP_FL_PRIM_ATTACK, 0, 768, -1, 2, 0 },
	{ 3, HL2DM_WEAPON_SMG1, g_szHL2DMWeapons[3], WEAP_FL_PRIM_ATTACK | WEAP_FL_SEC_ATTACK, 0, 1400, -1, 2, 0 },
	{ 2, HL2DM_WEAPON_AR2, g_szHL2DMWeapons[4], WEAP_FL_PRIM_ATTACK | WEAP_FL_SEC_ATTACK, 0, 1400, -1, 3, 0 },
	{ 1, HL2DM_WEAPON_FRAG, g_szHL2DMWeapons[5], WEAP_FL_GRENADE | WEAP_FL_EXPLOSIVE, 0, 180, -1, 1, 0 },
	{ 2, HL2DM_WEAPON_STUNSTICK, g_szHL2DMWeapons[6], WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 128, -1, 1, 0 },
	{ 3, HL2DM_WEAPON_CROSSBOW, g_szHL2DMWeapons[7], WEAP_FL_PRIM_ATTACK | WEAP_FL_SCOPE | WEAP_FL_UNDERWATER, 0, 2000, -1, 2, 0 },
	{ 2, HL2DM_WEAPON_RPG, g_szHL2DMWeapons[8], WEAP_FL_PROJECTILE | WEAP_FL_EXPLOSIVE | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 400, 2000, -1, 3, 1000.0f },
	{ 1, HL2DM_WEAPON_SLAM, g_szHL2DMWeapons[9], WEAP_FL_EXPLOSIVE, 0, 180, -1, 1, 0 },
	{ 2, HL2DM_WEAPON_SHOTGUN, g_szHL2DMWeapons[10], WEAP_FL_PRIM_ATTACK, 0, 768, -1, 2, 0 },
	{ 1, HL2DM_WEAPON_PHYSCANNON, g_szHL2DMWeapons[11], WEAP_FL_GRAVGUN | WEAP_FL_PRIM_ATTACK, 0, 768, -1, 4, 0 },
	{ 0, 0, "\0", 0, 0, 0, 0, 0, 0 }//signal last weapon
};

WeaponsData_t TF2Weaps[] =
{
	/*
		slot, id, weapon name, flags, min dist, max dist, ammo index, preference
		*/
	{ TF2_SLOT_MELEE, TF2_WEAPON_BAT, "tf_weapon_bat", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_BOTTLE, "tf_weapon_bottle", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_FIREAXE, "tf_weapon_fireaxe", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_CLUB, "tf_weapon_club", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_KNIFE, "tf_weapon_knife", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 220, 0, 2, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_FISTS, "tf_weapon_fists", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_SHOVEL, "tf_weapon_shovel", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_WRENCH, "tf_weapon_wrench", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 3, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_BONESAW, "tf_weapon_bonesaw", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_SHOTGUN_PRIMARY, "tf_weapon_shotgun_primary", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 800, 1, 2, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_SHOTGUN_SOLDIER, "tf_weapon_shotgun_soldier", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 500, 2, 2, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_SHOTGUN_HWG, "tf_weapon_shotgun_hwg", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 800, 2, 2, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_SHOTGUN_PYRO, "tf_weapon_shotgun_pyro", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 800, 2, 2, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_SCATTERGUN, "tf_weapon_scattergun", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 800, 1, 3, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_SNIPERRIFLE, "tf_weapon_sniperrifle", WEAP_FL_SCOPE | WEAP_FL_PRIM_ATTACK, 1000, 4000, 1, 3, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_MINIGUN, "tf_weapon_minigun", WEAP_FL_PRIM_ATTACK | WEAP_FL_HOLDATTACK, 120, 1800, 1, 3, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_SMG, "tf_weapon_smg", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 1000, 2, 2, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_SYRINGEGUN, "tf_weapon_syringegun_medic", WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 1000, 1, 2, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_ROCKETLAUNCHER, "tf_weapon_rocketlauncher", WEAP_FL_PRIM_ATTACK | WEAP_FL_EXPLOSIVE | WEAP_FL_UNDERWATER, BLAST_RADIUS, 4096, 1, 3, TF2_ROCKETSPEED },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_GRENADELAUNCHER, "tf_weapon_grenadelauncher", WEAP_FL_PROJECTILE | WEAP_FL_PRIM_ATTACK | WEAP_FL_EXPLOSIVE | WEAP_FL_UNDERWATER, 100, 1200, 1, 2, TF2_GRENADESPEED },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_PIPEBOMBS, "tf_weapon_pipebomblauncher", WEAP_FL_NONE, 0, 1000, 2, 1, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_FLAMETHROWER, "tf_weapon_flamethrower", WEAP_FL_DEFLECTROCKETS | WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_HOLDATTACK | WEAP_FL_SPECIAL, 0, 400, 1, 3, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_PISTOL, "tf_weapon_pistol", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 2000, 2, 1, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_PISTOL_SCOUT, "tf_weapon_pistol_scout", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 1800, 2, 2, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_REVOLVER, "tf_weapon_revolver", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 1400, 2, 1, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_POMSON6000, "tf_weapon_drg_pomson", WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 800, 1, 2, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_FRONTIERJUSTICE, "tf_weapon_sentry_revenge", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 800, 1, 2, 0 },
	{ TF2_SLOT_PDA, TF2_WEAPON_BUILDER, "tf_weapon_builder", WEAP_FL_NONE, 0, 100, 0, 1, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_MEDIGUN, "tf_weapon_medigun", WEAP_FL_NONE, 0, 100, 0, 1, 0 },
	{ TF2_SLOT_PDA, TF2_WEAPON_INVIS, "tf_weapon_invis", WEAP_FL_NONE, 0, 100, 0, 1, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_BUFF_ITEM, "tf_weapon_buff_item", WEAP_FL_NONE, 0, 100, 0, 1, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_FLAREGUN, "tf_weapon_flaregun", WEAP_FL_PRIM_ATTACK, 0, 1600, 2, 2, TF2_GRENADESPEED },
	{ TF2_SLOT_PDA, TF2_WEAPON_SENTRYGUN, "obj_sentrygun", 0, 0, 0, 0, 0, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_SAXXY, "saxxy", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 150, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_BAT_WOOD, "tf_weapon_bat_wood", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_LUNCHBOX_DRINK, "tf_weapon_lunchbox_drink", WEAP_FL_NONE, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_BOW, "tf_weapon_compound_bow", WEAP_FL_SCOPE | WEAP_FL_PRIM_ATTACK | WEAP_FL_PROJECTILE, 400, 2500, 1, 3, 1875 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_JAR, "tf_weapon_jar", WEAP_FL_NONE, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_BAT_FISH, "tf_weapon_bat_fish", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 180, 0, 1, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_DIRECTHIT, "tf_weapon_rocketlauncher_directhit", WEAP_FL_PRIM_ATTACK | WEAP_FL_EXPLOSIVE | WEAP_FL_UNDERWATER, BLAST_RADIUS, 4096, 1, 3, TF2_ROCKETSPEED*1.8 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_SWORD, "tf_weapon_sword", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 190, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_KATANA, "tf_weapon_katana", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 210, 0, 1, 0 },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_COWMANGLER, "tf_weapon_particle_cannon", WEAP_FL_KILLPIPEBOMBS | WEAP_FL_PRIM_ATTACK | WEAP_FL_UNDERWATER, 0, 1500, 1, 2, TF2_ROCKETSPEED },
	{ TF2_SLOT_PRMRY, TF2_WEAPON_CROSSBOW, "tf_weapon_crossbow", WEAP_FL_PRIM_ATTACK, 600, 2500, 1, 3, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_CLEAVER, "tf_weapon_cleaver", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 150, 0, 1, 0 },
	{ TF2_SLOT_MELEE, TF2_WEAPON_BAT_GIFTWRAP, "tf_weapon_bat_giftwrap", WEAP_FL_PRIM_ATTACK | WEAP_FL_MELEE | WEAP_FL_UNDERWATER, 0, 150, 0, 1, 0 },
	{ TF2_SLOT_SCNDR, TF2_WEAPON_RAYGUN, "tf_weapon_raygun", WEAP_FL_PRIM_ATTACK, 100, 2000, 2, 2, 0 },
	{ 0, 0, "\0", 0, 0, 0, 0, 0, 0 }//signal last weapon
};

bool CBotWeapon::NeedToReload(CBot *pBot)
{
	if (m_iClip1)
	{
		return (*m_iClip1 == 0) && (GetAmmo(pBot) > 0);
	}

	return false;
}

// static init (all weapons in game)
std::vector<CWeapon*> CWeapons::m_theWeapons;

int CBotWeapon::GetAmmo(CBot *pBot, int type)
{
	if (type == AMMO_PRIM)
		return pBot->GetAmmo(m_pWeaponInfo->GetAmmoIndex1());

	if (type == AMMO_SEC)
		return pBot->GetAmmo(m_pWeaponInfo->GetAmmoIndex2());


	return 0;
}


bool CBotWeapons::HasExplosives(void)
{
	CBotWeapon *pWeapon;

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		pWeapon = &(m_theWeapons[i]);
		// find weapon info from weapon id
		if (pWeapon->HasWeapon() && pWeapon->IsExplosive())
		{
			if (pWeapon->GetAmmo(m_pBot) > 1)
				return true;
		}
	}

	return false;
}


bool CBotWeapons::HasWeapon(int id)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (m_theWeapons[i].GetWeaponInfo() == NULL)
			continue;
		if (m_theWeapons[i].HasWeapon() == false)
			continue;

		// find weapon info from weapon id
		if (m_theWeapons[i].GetID() == id)
		{
			return true;
		}
	}
	return false;
}

// Bot Weapons
CBotWeapons::CBotWeapons(CBot *pBot)
{
	m_pBot = pBot;

	ClearWeapons();
	/*for ( int i = 0; i < MAX_WEAPONS; i ++ )
	{
		// find weapon info from weapon id
		m_theWeapons[i].setWeapon(CWeapons::getWeapon(i));
	}*/

	m_fUpdateWeaponsTime = 0;
	m_iWeaponsSignature = 0x0;
}

edict_t *CWeapons::FindWeapon(edict_t *pPlayer, const char *pszWeaponName)
{
	register unsigned short int j;
	CBaseHandle *m_Weapons = CClassInterface::GetWeaponList(pPlayer);
	edict_t *pWeapon;
	CBaseHandle *m_Weapon_iter = m_Weapons;
	// loop through the weapons array and see if it is in the CBaseCombatCharacter

	pWeapon = NULL;

	for (j = 0; j < MAX_WEAPONS; j++)
	{
		pWeapon = INDEXENT(m_Weapon_iter->GetEntryIndex());

		if (strcmp(pWeapon->GetClassName(), pszWeaponName) == 0)
			return pWeapon;

		m_Weapon_iter++;
	}

	return NULL;
}

bool CBotWeapons::Update(bool bOverrideAllFromEngine)
{
	// create mask of weapons data
	short int i = 0;
	unsigned short int iWeaponsSignature = 0x0; // check sum of weapons
	edict_t *pWeapon;
	CBaseHandle *hWeapons = CClassInterface::GetWeaponList(m_pBot->GetEdict());
	CBaseHandle *hWeapon_iter;

	hWeapon_iter = hWeapons;

	for (i = 0; i < MAX_WEAPONS; i++)
	{
		// create a 'hash' of current weapons
		pWeapon = (hWeapon_iter == NULL) ? NULL : INDEXENT(hWeapon_iter->GetEntryIndex());
		iWeaponsSignature += ((unsigned int)pWeapon) + ((pWeapon == NULL) ? 0 : (unsigned int)CClassInterface::GetWeaponState(pWeapon));
		hWeapon_iter++;
	}

	// if weapons have changed this will be different
	if (iWeaponsSignature != m_iWeaponsSignature)
	{
		this->ClearWeapons();

		int iWeaponState;
		bool bFound;

		const char *pszClassname;

		hWeapons = CClassInterface::GetWeaponList(m_pBot->GetEdict());
		CBotWeapon *hBotWeapon_iter = m_theWeapons;

		// loop through the weapons array and see if it is in the CBaseCombatCharacter
		for (i = 0; i < MAX_WEAPONS; i++)
		{
			hWeapon_iter = &hWeapons[i];
			iWeaponState = 0;
			bFound = false;

			pWeapon = INDEXENT(hWeapon_iter->GetEntryIndex());

			if (!pWeapon || pWeapon->IsFree())
			{
				continue;
			}

			iWeaponState = CClassInterface::GetWeaponState(pWeapon);

			pszClassname = pWeapon->GetClassName();

			CWeapon *pWeaponInfo = CWeapons::GetWeapon(pszClassname);

			if (pWeaponInfo != NULL)
			{
				if (iWeaponState != WEAPON_NOT_CARRIED)
				{
					CBotWeapon *pAdded = AddWeapon(pWeaponInfo, i, pWeapon, bOverrideAllFromEngine);
					pAdded->SetHasWeapon(true);
				}
			}
		}

		// check again in 1 second
		m_fUpdateWeaponsTime = engine->Time() + 1.0f;

		m_iWeaponsSignature = iWeaponsSignature;

		return true; // change
	}

	return false;
}

CBotWeapon *CBotWeapons::GetBestWeapon(edict_t *pEnemy, bool bAllowMelee, bool bAllowMeleeFallback, bool bMeleeOnly, bool bExplosivesOnly, bool bIgnorePrimaryMinimum)
{
	CBotWeapon *m_theBestWeapon = NULL;
	CBotWeapon *m_FallbackMelee = NULL;
	int iBestPreference = 0;
	Vector vEnemyOrigin;

	if (pEnemy)
		vEnemyOrigin = CBotGlobals::EntityOrigin(pEnemy);
	else
		vEnemyOrigin = m_pBot->GetOrigin();

	float flDist = 0;

	if (pEnemy)
		flDist = m_pBot->DistanceFrom(vEnemyOrigin);

	for (unsigned int i = 0; i < MAX_WEAPONS; i++)
	{
		CBotWeapon *pWeapon = &(m_theWeapons[i]);

		if (!pWeapon)
			continue;

		if (!pWeapon->HasWeapon())
			continue;

		if (bMeleeOnly && !pWeapon->IsMelee())
			continue;

		if (!bAllowMelee && pWeapon->IsMelee())
			continue;

		if (bExplosivesOnly && !pWeapon->IsExplosive())
			continue;

		if (!pWeapon->IsMelee() || pWeapon->IsSpecial())
		{
			if (pWeapon->OutOfAmmo(m_pBot))
				continue;
		}

		if (!pWeapon->CanAttack())
			continue;

		if (m_pBot->IsUnderWater() && !pWeapon->CanUseUnderWater())
			continue;

		if (!pWeapon->PrimaryInRange(flDist))
		{
			if (pWeapon->IsMelee() && !pWeapon->IsSpecial())
				m_FallbackMelee = pWeapon;

			if (!pWeapon->IsExplosive() || !bIgnorePrimaryMinimum)
				continue; // ignore explosive range if I'm invincible
		}

		if (pWeapon->GetPreference() > iBestPreference)
		{
			iBestPreference = pWeapon->GetPreference();
			m_theBestWeapon = pWeapon;
		}
	}

	if (bMeleeOnly || (bAllowMeleeFallback && ((m_theBestWeapon == NULL) && (flDist < 400.0f) && (fabs(vEnemyOrigin.z - m_pBot->GetOrigin().z) < BOT_JUMP_HEIGHT))))
		m_theBestWeapon = m_FallbackMelee;

	return m_theBestWeapon;
}

void CBotWeapon::SetWeaponEntity(edict_t *pent, bool bOverrideAmmoTypes)
{
	m_pEnt = pent;
	m_iClip1 = CClassInterface::GetWeaponClip1Pointer(pent);
	m_iClip2 = CClassInterface::GetWeaponClip2Pointer(pent);

	if (bOverrideAmmoTypes)
	{
		int iAmmoType1, iAmmoType2;
		CClassInterface::GetAmmoTypes(pent, &iAmmoType1, &iAmmoType2);
		m_pWeaponInfo->SetAmmoIndex(iAmmoType1, iAmmoType2);
	}
	SetWeaponIndex(ENTINDEX(m_pEnt));
}


CBotWeapon *CBotWeapons::AddWeapon(CWeapon *pWeaponInfo, int iId, edict_t *pent, bool bOverrideAll)
{
	register int i = 0;
	Vector origin;
	const char *classname;
	edict_t *pEnt = NULL;

	m_theWeapons[iId].SetHasWeapon(true);
	m_theWeapons[iId].SetWeapon(pWeaponInfo);

	if (!m_theWeapons[iId].GetWeaponInfo())
		return NULL;

	classname = pWeaponInfo->GetWeaponName();

	origin = m_pBot->GetOrigin();

	// if entity comes from the engine use the entity
	if (pent)
	{
		m_theWeapons[iId].SetWeaponEntity(pent, bOverrideAll);
	}
	else // find the weapon entity
	{
		for (i = (MAX_PLAYERS + 1); i <= MAX_ENTITIES; i++)
		{
			pEnt = INDEXENT(i);

			if (pEnt && CBotGlobals::EntityIsValid(pEnt))
			{
				if (CBotGlobals::EntityOrigin(pEnt) == origin)
				{
					if (strcmp(pEnt->GetClassName(), classname) == 0)
					{
						m_theWeapons[iId].SetWeaponEntity(pEnt, bOverrideAll);// .setWeaponIndex(ENTINDEX(pEnt));

						return &m_theWeapons[iId];
					}
				}
			}
		}
	}

	return &m_theWeapons[iId];

}

CBotWeapon *CBotWeapons::GetWeapon(CWeapon *pWeapon)
{
	for (register unsigned int i = 0; i < MAX_WEAPONS; i++)
	{
		if (m_theWeapons[i].GetWeaponInfo() == pWeapon)
			return &(m_theWeapons[i]);
	}

	return NULL;
}

CBotWeapon *CBotWeapons::GetCurrentWeaponInSlot(int iSlot)
{
	for (register unsigned int i = 0; i < MAX_WEAPONS; i++)
	{
		if (m_theWeapons[i].HasWeapon() && m_theWeapons[i].GetWeaponInfo() && (m_theWeapons[i].GetWeaponInfo()->GetSlot() == iSlot))
			return &(m_theWeapons[i]);
	}

	return NULL;
}

const char *szWeaponFlags[] = {
	"primary_attack",
	"secondary_attack",
	"explosive",
	"melee",
	"underwater",
	"hold_attack",
	"special",
	"can_kill_pipes",
	"can_deflect_rockets",
	"is_grav_gun",
	"has_explosive_secondary",
	"is_zoomable",
	"is_deployable_dods",
	"has_melee_secondary",
	"has_fire_select_mode_dods",
	"cant_be_fired_unzoomed_undeployed_dods",
	"is_grenade",
	"has_high_recoil_dods",
	"has_scope",
	"weapon_fires_projectile",
	"\0"
};

void CWeapons::LoadWeapons(const char *szWeaponListName, WeaponsData_t *pDefault)
{
	if ((szWeaponListName != NULL) && (szWeaponListName[0] != 0))
	{
		KeyValues *kv = new KeyValues("Weapons");
		char szFilename[1024];

		smutils->BuildPath(Path_SM, szFilename, sizeof(szFilename), "data\\afkbot\\config\\weapons.cfg");

		if (kv)
		{
			if (kv->LoadFromFile(filesystem, szFilename, NULL))
			{
				kv = kv->FindKey(szWeaponListName);

				if (kv)
				{
					kv = kv->GetFirstSubKey();

					if (0)
						kv = kv->GetFirstTrueSubKey();

					while (kv != NULL)
					{
						WeaponsData_t newWeapon;

						memset(&newWeapon, 0, sizeof(WeaponsData_t));

						const char *szKeyName = kv->GetName();

						char lowered[64];

						strncpy(lowered, szKeyName, 63);
						lowered[63] = 0;

						__strlow(lowered);

						newWeapon.szWeaponName = CStrings::GetString(lowered);
						newWeapon.iId = kv->GetInt("id");
						newWeapon.iSlot = kv->GetInt("slot");
						newWeapon.minPrimDist = kv->GetFloat("minPrimDist");
						newWeapon.maxPrimDist = kv->GetFloat("maxPrimDist");
						newWeapon.m_fProjSpeed = kv->GetFloat("m_fProjSpeed");
						newWeapon.m_iAmmoIndex = kv->GetInt("m_iAmmoIndex");
						newWeapon.m_iPreference = kv->GetInt("m_iPreference");

						KeyValues *flags = kv->FindKey("flags");

						if (flags)
						{
							int i = 0;

							while (szWeaponFlags[i][0] != '\0')
							{
								if (flags->FindKey(szWeaponFlags[i]) && (flags->GetInt(szWeaponFlags[i]) == 1))
									newWeapon.m_iFlags |= (1 << i);

								i++;
							}
						}

						AddWeapon(new CWeapon(&newWeapon));

						kv = kv->GetNextTrueSubKey();
					}
				}

			}


			kv->deleteThis();

		}
	}

	if (pDefault != NULL)
	{
		// No weapons from INI file then add default
		if (m_theWeapons.size() == 0)
		{
			while (pDefault->szWeaponName[0] != '\0')
			{
				AddWeapon(new CWeapon(pDefault));
				pDefault++;
			}
		}
	}
}

void CBotWeapons::ClearWeapons()
{
	for (register unsigned short i = 0; i < MAX_WEAPONS; i++)
		memset(&m_theWeapons[i], 0, sizeof(CBotWeapon));
}

// returns weapon with highest priority even if no ammo
CBotWeapon *CBotWeapons::GetPrimaryWeapon()
{
	CBotWeapon *pBest = NULL;

	for (register unsigned short i = 0; i < MAX_WEAPONS; i++)
	{
		CBotWeapon *pWeap = &(m_theWeapons[i]);

		if (!pWeap->HasWeapon())
			continue;

		if ((pBest == NULL) || (pBest->GetPreference() < pWeap->GetPreference()))
		{
			pBest = pWeap;
		}
	}

	return pBest;
}

CBotWeapon *CBotWeapons::GetActiveWeapon(const char *szWeaponName, edict_t *pWeaponUpdate, bool bOverrideAmmoTypes)
{
	CBotWeapon *toReturn = NULL;

	if (szWeaponName && *szWeaponName)
	{
		CWeapon *pWeapon = CWeapons::GetWeapon(szWeaponName);

		if (pWeapon)
		{
			register unsigned short int i;

			for (i = 0; i < MAX_WEAPONS; i++)
			{
				CWeapon *p = m_theWeapons[i].GetWeaponInfo();

				if (!p)
					continue;

				if (strcmp(p->GetWeaponName(), szWeaponName) == 0)
				{
					toReturn = &m_theWeapons[i];
					break;
				}
			}

			if (pWeaponUpdate && toReturn)
				toReturn->SetWeaponEntity(pWeaponUpdate, bOverrideAmmoTypes);
		}
	}

	return toReturn;
}
/*
bool CBotWeaponGravGun::OutOfAmmo (CBot *pBot)
{
	if (m_pEnt)
		return (CClassInterface::GravityGunObject(m_pEnt)==NULL);

	return true;
}
*/
bool CBotWeapon::OutOfAmmo(CBot *pBot)
{
	// if I have something in my clip now
	// I am okay, otherwise return ammo in list
	if (m_iClip1 && (*m_iClip1 > 0))
		return false;

	return GetAmmo(pBot) == 0;
}
/*
bool CBotWeapon::NeedToReload(CBot *pBot)
{
	return GetAmmo(pBot)==0;
}*/
////////////////////////////////////////
// CWeapons

class IWeaponFunc
{
public:
	virtual void Execute(CWeapon *pWeapon) = 0;
};

class CGetWeapID : public IWeaponFunc
{
public:
	CGetWeapID(int iId)
	{
		m_iId = iId;
		m_pFound = NULL;
	}

	void Execute(CWeapon *pWeapon)
	{
		if (m_iId == pWeapon->GetID())
			m_pFound = pWeapon;
	}

	CWeapon *Get()
	{
		return m_pFound;
	}

private:
	int m_iId;
	CWeapon *m_pFound;
};

class CGetWeapCName : public IWeaponFunc
{
public:
	CGetWeapCName(const char *szWeapon)
	{
		m_pFound = NULL;
		m_szWeapon = szWeapon;
	}

	void Execute(CWeapon *pWeapon)
	{
		if (pWeapon->IsWeaponName(m_szWeapon))
			m_pFound = pWeapon;
	}

	CWeapon *Get()
	{
		return m_pFound;
	}
private:
	const char *m_szWeapon;
	CWeapon *m_pFound;
};

class CGetWeapShortName : public IWeaponFunc
{
public:
	CGetWeapShortName(const char *szWeapon)
	{
		m_pFound = NULL;
		m_szWeapon = szWeapon;
	}

	void Execute(CWeapon *pWeapon)
	{
		if (pWeapon->IsShortWeaponName(m_szWeapon))
			m_pFound = pWeapon;
	}

	CWeapon *Get()
	{
		return m_pFound;
	}
private:
	const char *m_szWeapon;
	CWeapon *m_pFound;
};

CWeapon *CWeapons::GetWeapon(const int iId)
{
	CGetWeapID pFunc = CGetWeapID(iId);
	EachWeapon(&pFunc);
	return pFunc.Get();
}

CWeapon *CWeapons::GetWeapon(const char *szWeapon)
{
	CGetWeapCName pFunc = CGetWeapCName(szWeapon);
	EachWeapon(&pFunc);
	return pFunc.Get();
}

CWeapon *CWeapons::GetWeaponByShortName(const char *szWeapon)
{
	CGetWeapShortName pFunc = CGetWeapShortName(szWeapon);
	EachWeapon(&pFunc);
	return pFunc.Get();
}

void CWeapons::EachWeapon(IWeaponFunc *pFunc)
{
	for (unsigned int i = 0; i < m_theWeapons.size(); i++)
	{
		pFunc->Execute(m_theWeapons[i]);
	}
}

void CWeapons::FreeMemory()
{
	for (unsigned int i = 0; i < m_theWeapons.size(); i++)
	{
		delete m_theWeapons[i];
		m_theWeapons[i] = NULL;
	}

	m_theWeapons.clear();
}