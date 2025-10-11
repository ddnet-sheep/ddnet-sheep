
/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
// This helps IDEs properly syntax highlight the uses of the macro below.
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Save, Desc)
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Save, Desc)
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Save, Desc)
#endif

MACRO_CONFIG_STR(SvSheepDiscordToken, sv_sheep_discord_token, 128, "", CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, "discord bot token")
MACRO_CONFIG_STR(SvSheepDiscordServerChannelId, sv_sheep_discord_server_channel_id, 128, "", CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, "discord server channel id")

MACRO_CONFIG_INT(SvSheepWeaponDrops, sv_sheep_weapon_drops, 0, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "enable weapon drops")
MACRO_CONFIG_INT(SvSheepEnforceAccount, sv_sheep_enforce_account, 0, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "force account to play")
MACRO_CONFIG_INT(SvSheepSpawnPowerups, sv_sheep_spawn_powerups, 1, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "spawn powerups")

MACRO_CONFIG_INT(SvSheepExpPerMinute, sv_sheep_exp_per_minute, 1, 0, 10, CFGFLAG_SERVER | CFGFLAG_GAME, "experience points received per minute")
MACRO_CONFIG_INT(SvSheepMoneyPlaytime, sv_sheep_money_playtime, 50, 0, 1000000, CFGFLAG_SERVER | CFGFLAG_GAME, "money received for each 60 minutes of playtime")

MACRO_CONFIG_STR(SvSheepMoneyName, sv_sheep_money_name, 128, "$", CFGFLAG_SERVER | CFGFLAG_GAME, "name of the currency")

MACRO_CONFIG_INT(SvSheepExperimentalPrediction, sv_sheep_experimental_prediction, 1, 0, 1, CFGFLAG_SERVER, "Experimental Prediction for cosmetics, tries to use clients ping to nudge cosmetics to the correct position")