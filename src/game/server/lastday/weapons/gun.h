#ifndef GAME_SERVER_LASTDAY_WEAPONS_GUN_H
#define GAME_SERVER_LASTDAY_WEAPONS_GUN_H

#include <game/server/lastday/weapon.h>

class CWeaponGun : public CWeapon
{
public:
    CWeaponGun(CGameContext *pGameServer);

    virtual void Fire(int Owner, vec2 Dir, vec2 Pos);
};

#endif