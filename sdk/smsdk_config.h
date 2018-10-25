/**
 * =============================================================================
 * AFK Bot Extension
 * Copyright (C) 2016 Deathreus.  All rights reserved.
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
 */

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_

/**
 * @file smsdk_config.h
 * @brief Contains macros for configuring basic extension information.
 */

/* Basic information exposed publicly */
#define SMEXT_CONF_NAME			"AFKBot"
#define SMEXT_CONF_DESCRIPTION	"Let's a bot take over when players are away. Requires an external AFK manager to automatically enable the bot!"
#define SMEXT_CONF_VERSION		"1.0"
#define SMEXT_CONF_AUTHOR		"Cheeseh & Deathreus"
#define SMEXT_CONF_URL			""
#define SMEXT_CONF_LOGTAG		SMEXT_CONF_NAME
#define SMEXT_CONF_LICENSE		"GPLv3"
#define SMEXT_CONF_DATESTRING	__DATE__

/** 
 * @brief Exposes plugin's main interface.
 */
#define SMEXT_LINK(name) SDKExtension *g_pExtensionIface = name;

#define SMEXT_CONF_METAMOD		

#define SMEXT_ENABLE_FORWARDSYS
#define SMEXT_ENABLE_PLAYERHELPERS
#define SMEXT_ENABLE_GAMECONF
#define SMEXT_ENABLE_GAMEHELPERS
#define SMEXT_ENABLE_TIMERSYS
#define SMEXT_ENABLE_ADMINSYS
#define SMEXT_ENABLE_TEXTPARSERS
#define SMEXT_ENABLE_USERMSGS

#endif // _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_