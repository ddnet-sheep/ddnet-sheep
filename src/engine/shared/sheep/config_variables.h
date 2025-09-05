
/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
// This helps IDEs properly syntax highlight the uses of the macro below.
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Save, Desc)
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Save, Desc)
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Save, Desc)
#endif

MACRO_CONFIG_STR(SvSheepDiscordToken, sv_sheep_discord_token, 128, "", CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, "Discord Bot Token")
MACRO_CONFIG_STR(SvSheepDiscordServerChannelId, sv_sheep_discord_server_channel_id, 128, "", CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, "Discord Server Channel ID")

MACRO_CONFIG_INT(SvSheepWeaponDrops, sv_sheep_weapon_drops, 0, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "Enable weapon drops")