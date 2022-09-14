#ifndef GAME_SERVER_LASTDAY_CONFIG_H
#define GAME_SERVER_LASTDAY_CONFIG_H
#undef GAME_SERVER_LASTDAY_CONFIG_H // this file will be included several times

MACRO_CONFIG_INT(SvFreezeHelpLen, sv_freezehelp_len, 96, 48, 240, CFGFLAG_SERVER, "Freeze help lenth")
MACRO_CONFIG_INT(SvFreezeTime, sv_freeze_time, 5, 3, 10, CFGFLAG_SERVER, "Freeze time")

MACRO_CONFIG_INT(LastDayMaxZombNum, ld_max_zomb_num, 64, 16, 64, CFGFLAG_SERVER, "Max zombie num in a time")
MACRO_CONFIG_INT(LastDayZombRefireTick, ld_zomb_refire_tick, 10, 10, 100, CFGFLAG_SERVER, "Zombie refire tick")

#endif