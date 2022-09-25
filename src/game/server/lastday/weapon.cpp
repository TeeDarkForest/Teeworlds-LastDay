#include <game/server/gamecontext.h>
#include "weapon.h"

IWeapon::IWeapon(CGameContext *pGameServer, int WeaponID, int ShowType, int FireDelay) :
    m_pGameServer(pGameServer),
    m_WeaponID(WeaponID),
    m_ShowType(ShowType),
    m_FireDelay(FireDelay)
{
}

CGameContext *IWeapon::GameServer() const
{
    return m_pGameServer;
}

CGameWorld *IWeapon::GameWorld() const
{
    return &m_pGameServer->m_World;
}

int IWeapon::GetWeaponID() const
{
    return m_WeaponID;
}

int IWeapon::GetShowType() const
{
    return m_ShowType;
}

int IWeapon::GetFireDelay() const
{
    return m_FireDelay;
}

CWeapon::CWeapon(CGameContext *pGameServer, int WeaponID, int ShowType, int FireDelay) :
    IWeapon(pGameServer, WeaponID, ShowType, FireDelay)
{
}

void CWeapon::OnFire(int Owner, vec2 Dir, vec2 Pos)
{
    Fire(Owner, Dir, Pos);
}

void WeaponSystem::InitWeapon(int Number, IWeapon *pWeapon)
{
	m_aWeapons[Number] = pWeapon;
}

WeaponSystem g_Weapons;