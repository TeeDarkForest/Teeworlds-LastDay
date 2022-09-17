#ifndef GAME_SERVER_LASTDAY_WEAPONS_FIREGUN_H
#define GAME_SERVER_LASTDAY_WEAPONS_FIREGUN_H

#include <game/server/lastday/weapon.h>

class CWeaponFireGun : public CWeapon
{
public:
    CWeaponFireGun(CGameContext *pGameServer);

    virtual void Fire(int Owner, vec2 Dir, vec2 Pos);
};

#endif