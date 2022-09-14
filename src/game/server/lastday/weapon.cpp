#include <game/server/gamecontext.h>
#include "weapon.h"

IWeapon::IWeapon(CGameContext *pGameServer, int WeaponID, int ShowType) :
    m_pGameServer(pGameServer),
    m_WeaponID(WeaponID),
    m_ShowType(ShowType)
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

CWeapon::CWeapon(CGameContext *pGameServer, int WeaponID, int ShowType) :
    IWeapon(pGameServer, WeaponID, ShowType)
{
}

void CWeapon::OnFire(int Owner, vec2 Dir, vec2 Pos)
{
    Fire(Owner, Dir, Pos);
}