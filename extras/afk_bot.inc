#if defined _afkbot_included
  #endinput
 #endif
#define _afkbot_included

/**
 * Gets the current AFK status of the client index.
 *
 * @param client	The client index of the player being checked.
 * @return			The clients AFK status.
 * @error			The client index is invalid.
 */
native bool IsClientAFKBot(int client);

/**
 * Sets the AFK status of the client index.
 *
 * @param client	The client index.
 * @param enabled	True to enable, false to disable
 * @error			The client index is invalid.
 * @noreturn
 */
native void SetClientAFKBot(int client, bool enabled);

/**
 * Do not edit below this line!
 */
public Extension __ext_afkbot = 
{
	name = "AFKBot",
	file = "AFKBot.ext.2.tf2",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
}

#if !defined REQUIRE_PLUGIN
public __pl_afkbot_SetNTVOptional()
{
	MarkNativeAsOptional("IsClientAFKBot");
	MarkNativeAsOptional("SetClientAFKBot");
}
#endif
