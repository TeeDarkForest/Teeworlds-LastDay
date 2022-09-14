#ifndef GAME_SERVER_LASTDAY_WEAPONS_SHOTGUN_H
#define GAME_SERVER_LASTDAY_WEAPONS_SHOTGUN_H

#include <game/server/lastday/weapon.h>

class CWeaponShotgun : public CWeapon
{
public:
    CWeaponShotgun(CGameContext *pGameServer);

    virtual void Fire(int Owner, vec2 Dir, vec2 Pos);
};

#endif