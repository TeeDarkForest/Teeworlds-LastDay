#include <game/server/lastday/define.h>
#include <game/server/gamecontext.h>
#include "bullets/projectile.h"
#include "ninja.h"

CWeaponNinja::CWeaponNinja(CGameContext *pGameServer)
    : CWeapon(pGameServer, TWS_WEAPON_NINJA, WEAPON_NINJA, 800)
{
}

void CWeaponNinja::Fire(int Owner, vec2 Dir, vec2 Pos)
{
    CCharacter *pOwnerChr = GameServer()->GetPlayerChar(Owner);
    if(!pOwnerChr)
        return;
    pOwnerChr->m_Ninja.m_ActivationDir = Dir;
    pOwnerChr->m_Ninja.m_CurrentMoveTime = g_pData->m_Weapons.m_Ninja.m_Movetime * GameServer()->Server()->TickSpeed() / 1000;
    pOwnerChr->m_Ninja.m_OldVelAmount = length(pOwnerChr->m_Core.m_Vel);

    GameServer()->CreateSound(Pos, SOUND_NINJA_FIRE);
    
    return;
}