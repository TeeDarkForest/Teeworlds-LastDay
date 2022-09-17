#include <game/server/lastday/define.h>
#include <game/server/gamecontext.h>
#include "bullets/tws-laser.h"
#include "hammer-plus.h"

CWeaponHammerPlus::CWeaponHammerPlus(CGameContext *pGameServer)
    : CWeapon(pGameServer, LD_WEAPON_HAMMER_PLUS, WEAPON_HAMMER, 125)
{
}

void CWeaponHammerPlus::Fire(int Owner, vec2 Dir, vec2 Pos)
{
    CCharacter *pOwnerChr = GameServer()->GetPlayerChar(Owner);
    if(!pOwnerChr)
        return;

	GameServer()->CreateSound(Pos, SOUND_HAMMER_FIRE);

    CCharacter *apEnts[MAX_CLIENTS];
    int Hits = 0;
    int Num = GameServer()->m_World.FindEntities(Pos, pOwnerChr->m_ProximityRadius*0.5f, (CEntity**)apEnts,
                                                MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

    for (int i = 0; i < Num; ++i)
    {
        CCharacter *pTarget = apEnts[i];

        if ((pTarget == pOwnerChr) || GameServer()->Collision()->IntersectLine(Pos, pTarget->m_Pos, NULL, NULL))
            continue;

        // set his velocity to fast upward (for now)
        if(length(pTarget->m_Pos-Pos) > 0.0f)
            GameServer()->CreateHammerHit(pTarget->m_Pos-normalize(pTarget->m_Pos-Pos)*pOwnerChr->m_ProximityRadius*0.5f);
        else
            GameServer()->CreateHammerHit(Pos);

        vec2 Dir;
        if (length(pTarget->m_Pos - pOwnerChr->m_Pos) > 0.0f)
            Dir = normalize(pTarget->m_Pos - pOwnerChr->m_Pos);
        else
            Dir = vec2(0.f, -1.f);

        pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage,
            pOwnerChr->GetPlayer()->GetCID(), GetShowType(), GetWeaponID());
        Hits++;
    }

    for(int i = 0; i < 8;i ++)
    {
        CTWSLaser *pLaser = new CTWSLaser(GameWorld(),
            pOwnerChr->m_Pos,
            GetDirection(random_int(0, 1280)),
            GameServer()->Tuning()->m_LaserReach/2,
            Owner);
    }
    // if we Hit anything, we have to wait for the reload
    if(Hits)
        pOwnerChr->m_ReloadTimer = GameServer()->Server()->TickSpeed()/3;

    
    return;
}