#ifndef GAME_SERVER_LASTDAY_WEAPONS_HAMMER_PLUS_H
#define GAME_SERVER_LASTDAY_WEAPONS_HAMMER_PLUS_H

#include <game/server/lastday/weapon.h>

class CWeaponHammerPlus : public CWeapon
{
public:
    CWeaponHammerPlus(CGameContext *pGameServer);

    virtual void Fire(int Owner, vec2 Dir, vec2 Pos);
};

#endif