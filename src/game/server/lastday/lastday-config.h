#ifndef GAME_SERVER_LASTDAY_CONFIG_H
#define GAME_SERVER_LASTDAY_CONFIG_H
#undef GAME_SERVER_LASTDAY_CONFIG_H // this file will be included several times

// lastday config
MACRO_CONFIG_INT(SvFreezeHelpLen, sv_freezehelp_len, 96, 48, 240, CFGFLAG_SERVER, "Freeze help lenth")
MACRO_CONFIG_INT(SvFreezeTime, sv_freeze_time, 5, 3, 10, CFGFLAG_SERVER, "Freeze time")

MACRO_CONFIG_INT(LDMaxZombNum, ld_max_zomb_num, 16, 0, MAX_CLIENTS-1, CFGFLAG_SERVER, "Max zombie num in a time")
MACRO_CONFIG_INT(LDZombAttackProba, ld_zomb_attack_proba, 4, 0, 100, CFGFLAG_SERVER, "Zombie attack not 0 proba")

MACRO_CONFIG_INT(LDRainSpawnProba, ld_rain_spawn_proba, 5, 0, 100, CFGFLAG_SERVER, "Rain spawn proba")

// Zombie AI
MACRO_CONFIG_INT(LDZombMinRefireTick, ld_zomb_min_refire_tick, 10, 10, 100, CFGFLAG_SERVER, "Zombie refire tick")
MACRO_CONFIG_INT(LDZombMaxRefireTick, ld_zomb_max_refire_tick, 50, 10, 1000, CFGFLAG_SERVER, "Zombie refire tick")
MACRO_CONFIG_INT(LDZombMinRehookTick, ld_zomb_min_rehook_tick, 20, 10, 100, CFGFLAG_SERVER, "Zombie rehook tick")
MACRO_CONFIG_INT(LDZombMaxRehookTick, ld_zomb_max_rehook_tick, 70, 10, 1000, CFGFLAG_SERVER, "Zombie rehook tick")

#endif