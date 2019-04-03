#if defined _afkbot_included
 #endinput
#endif
#define _afkbot_included

/**
 * Gets the current AFK status of the client index.
 *
 * @param client    The client index/uid of the player being checked.
 * @return          The clients AFK status.
 * @error           The client index is invalid.
 */
native bool IsClientAFKBot(int client);

/**
 * Sets the AFK status of the client index.
 *
 * @param client    The client index/uid.
 * @param enabled   True to enable, false to disable
 * @error           The client index is invalid.
 * @noreturn
 */
native void SetClientAFKBot(int client, bool enabled);

/**
 * Sets the minimum and maximum aim skill for random skill selection.
 *
 * @param float min    Minimum aiming skill, clamped to 0.0
 * @param float max    Maximum aiming skill, clamped to 1.0
 * @error              The value is below 0.0 or above 1.0, or skill randomizing is turned off.
 * @noreturn
 */
native void SetAFKBotMinMaxSkill(float min, float max);

/**
 * Sets skill level of the bot for the client index.
 *
 * @param int client       Client index/uid
 * @param float skill      Skill level
 * @error                  The client index is invalid, or the value is below 0.0 or above 1.0.
 * @noreturn
 */
native void SetClientBotSkill(int client, float skill);

/**
 * Sets the minimum and maximum turning sensitivty(speed) for random skill selection.
 *
 * The higher this number is the faster he will turn his viewpoint, however,
 * too fast and it will get bouncy(Which is to say, it will bounce back and forth over the
 * target while it tries to resolve the aim vector via smoothing).
 *
 * @param int min     Minimum sensitivty, clamped to 0
 * @param int max     Maximum sensitivty
 * @error             The value is below 0, or skill randomizing is turned off.
 * @noreturn
 */
native void SetAFKBotMinMaxSensitivity(int min, int max);

/**
 * Sets the turning sensitivty(speed) for the client index.
 *
 * @param int client        Client index/uid
 * @param int sensitivity   Turn speed
 * @error                   The client index is invalid, or the value is below 0.
 * @noreturn
 */
native void SetClientBotSensitivity(int client, int sensitivity);

/**
 * Sets the minimum and maximum braveness factor (How often or how little
 * the bot will try to accomplish something it deems too dangerous) for random skill selection.
 *
 * @param float min    Minimum braveness factor, clamped to 0.0
 * @param float max    Maximum braveness factor, clamped to 1.0
 * @error              The value is below 0.0 or above 1.0, or skill randomizing is turned off.
 * @noreturn
 */
native void SetAFKBotMinMaxBraveness(float min, float max);

/**
 * Sets braveness factor (How often or how little for the client index.
 *
 * @param int client        Client index/uid
 * @param float braveness   Braveness factor
 * @error                   The client index is invalid, or the value is below 0.0 or above 1.0.
 * @noreturn
 */
native void SetClientBotBraveness(int client, float braveness);

/**
 * Sets the minimum and maximum revisions the bot tries 
 * to do per vision check for random skill selection.
 *
 * This is a bit ambiguous, this is mostly running checks on what the bot can immediately
 * see around him, looking for buildings and NPC's.
 *
 * @param int min    Minimum revisions, clamped to 1
 * @param int max    Maximum revisions, for performance reasons, clamped to 24
 * @error            The value is below 1 or above 24, or skill randomizing is turned off.
 * @noreturn
 */
native void SetAFKBotMinMaxVisRevs(int min, int max);

/**
 * Sets number of revisions the bot tries 
 * to do per vision check for the client index.
 *
 * @param int client      Client index/uid
 * @param int revisions   Vision revisions
 * @error                 The client index is invalid, or the value is below 1 or above 24.
 * @noreturn
 */
native void SetClientBotVisRevs(int client, int revisions);

/**
 * Sets the minimum and maximum revisions the bot
 * tries to do per path check for random skill selection
 *
 * @param int min    Minimum revisions, clamped to 1
 * @param int max    Maximum revisions, for performance reasons, clamped to 256
 * @error            The value is below 1 or above 256, or skill randomizing is turned off.
 * @noreturn
 */
native void SetAFKBotMinMaxPathRevs(int min, int max);

/**
 * Sets the number of revisions the bot
 * tries to do per path check for the client index.
 *
 * @param int client      Client index/uid
 * @param int revisions   Path revisions
 * @error                 The client index is invalid, or the value is below 1 or above 256.
 * @noreturn
 */
native void SetClientBotPathRevs(int client, int revisions);

/**
 * Sets the minimum and maximum revisions the bot tries
 * to do per vision check for clients for random skill selection.
 *
 * THIS one is what's looking for players, but it puts a higher emphasis on bots.
 *
 * @param int min    Minimum revisions, clamped to 1
 * @param int max    Maximum revisions. for performance reasons, clamped to 16
 * @error            The value is below 1 or above 16, or skill randomizing is turned off.
 * @noreturn
 */
native void SetAFKBotMinMaxClientVisRevs(int min, int max);

/**
 * Sets the number of revisions the bot tries
 * to do per vision check for clients for the client index.
 *
 * @param int client      Client index/uid
 * @param int revisions   Vision revisions
 * @error                 The client index is invalid, or the value is below 1 or above 16.
 * @noreturn
 */
native void SetClientBotClientVisRevs(int client, int revisions);

/**
 * Called just before the bot is enabled on a client.
 * Return Plugin_Handled or higher to stop this from happening.
 *
 * @param int client     UserID of client about to be turned into a bot
 * @param bool enable    If the bot was being enabled or disabled.
 */
forward Action OnBotEnable(int client, bool enable);

/**
 * Do not edit below this line!
 */
public Extension __ext_afkbot = 
{
    name = "AFKBot",
    file = "afkbot.ext",
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
    MarkNativeAsOptional("SetAFKBotMinMaxSkill");
    MarkNativeAsOptional("SetClientBotSkill");
    MarkNativeAsOptional("SetAFKBotMinMaxSensitivity");
    MarkNativeAsOptional("SetClientBotSensitivity");
    MarkNativeAsOptional("SetAFKBotMinMaxBraveness");
    MarkNativeAsOptional("SetClientBotBraveness");
    MarkNativeAsOptional("SetAFKBotMinMaxVisRevs");
    MarkNativeAsOptional("SetClientBotVisRevs");
    MarkNativeAsOptional("SetAFKBotMinMaxClientVisRevs");
    MarkNativeAsOptional("SetClientBotClientVisRevs");
    MarkNativeAsOptional("SetAFKBotMinMaxPathRevs");
    MarkNativeAsOptional("SetClientBotPathRevs");
}
#endif
