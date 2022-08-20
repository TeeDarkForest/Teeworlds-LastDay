/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_VARIABLES_H
#define GAME_VARIABLES_H
#undef GAME_VARIABLES_H // this file will be included several times

/* SQL */
MACRO_CONFIG_STR(SvSqlUser, sv_sql_user, 256, "", CFGFLAG_SERVER, "SQL User")
MACRO_CONFIG_STR(SvSqlPassword, sv_sql_password, 256, "", CFGFLAG_SERVER, "SQL Password")
MACRO_CONFIG_STR(SvSqlIp, sv_sql_ip, 256, "127.0.0.1", CFGFLAG_SERVER, "SQL Database IP")
MACRO_CONFIG_INT(SvSqlPort, sv_sql_port, 3306, 0, 65535, CFGFLAG_SERVER, "SQL Database port")
MACRO_CONFIG_STR(SvSqlDatabase, sv_sql_database, 256, "", CFGFLAG_SERVER, "SQL Database name")
MACRO_CONFIG_STR(SvSqlPrefix, sv_sql_prefix, 16, "twld", CFGFLAG_SERVER, "SQL Database table prefix")


MACRO_CONFIG_INT(LdMaxZombieSpawn, ld_max_zombie_spawn, 1, 1, 20, CFGFLAG_SERVER, "Number of the max spawn tries of all zombies per tick")
MACRO_CONFIG_INT(LdMaxZombieNum, ld_max_zombie_num, 24, 1, 20, CFGFLAG_SERVER, "Number of the max spawn tries of all zombies per tick")
MACRO_CONFIG_INT(LdMaxHumanHP, ld_max_human_hp, 100, 10, 200, CFGFLAG_SERVER, "Number of the max health")
MACRO_CONFIG_INT(LdMaxHumanArmor, ld_max_human_armor, 20, 10, 200, CFGFLAG_SERVER, "Number of the max armor")

MACRO_CONFIG_INT(LdItemLifeSpan, ld_item_life_span, 240, 0, 1200, CFGFLAG_SERVER, "Item life span in second")
MACRO_CONFIG_INT(LdHealthRespan, ld_health_respan, 15, 0, 1200, CFGFLAG_SERVER, "Health respan in second")
MACRO_CONFIG_INT(LdArmorRespan, ld_armor_respan, 15, 0, 1200, CFGFLAG_SERVER, "Armor respan in second")

#endif