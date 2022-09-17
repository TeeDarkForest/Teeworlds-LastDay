#include <game/server/lastday/define.h>
#include <game/server/gamecontext.h>
#include "bullets/fire.h"
#include "fire-gun.h"

CWeaponFireGun::CWeaponFireGun(CGameContext *pGameServer)
    : CWeapon(pGameServer, LD_WEAPON_FIREGUN, WEAPON_GRENADE, 20)
{
}

void CWeaponFireGun::Fire(int Owner, vec2 Dir, vec2 Pos)
{
    int ShotSpread = 2;

    for(int i = -ShotSpread; i <= ShotSpread; ++i)
    {
        float Spreading[] = {-0.185f, -0.070f, 0, 0.070f, 0.185f};
        float a = GetAngle(Dir);
        a += Spreading[i+2] + (random_int(10, 100) / 100.0);
        float v = 1-(absolute(i)/(float)ShotSpread);
        CFire *pProj = new CFire(GameWorld(),
            Owner,
            Pos,
            vec2(cosf(a), sinf(a))*0.2,
            (int)(GameServer()->Server()->TickSpeed()*0.5),
            1, GetShowType(), LD_WEAPON_FIREGUN);
    }
    
    return;
}