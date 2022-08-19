/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_VARIABLES_H
#define GAME_VARIABLES_H
#undef GAME_VARIABLES_H // this file will be included several times


MACRO_CONFIG_INT(LdMaxZombieSpawn, ld_max_zombie_spawn, 1, 1, 20, CFGFLAG_SERVER, "Number of the max spawn tries of all zombies per tick")
MACRO_CONFIG_INT(LdMaxZombieNum, ld_max_zombie_num, 24, 1, 20, CFGFLAG_SERVER, "Number of the max spawn tries of all zombies per tick")
MACRO_CONFIG_INT(LdMaxHumanHP, ld_max_human_hp, 100, 10, 200, CFGFLAG_SERVER, "Number of the max health")
MACRO_CONFIG_INT(LdMaxHumanArmor, ld_max_human_armor, 20, 10, 200, CFGFLAG_SERVER, "Number of the max armor")

#endif