#ifndef GAME_SERVER_LASTDAY_WEAPON_H
#define GAME_SERVER_LASTDAY_WEAPON_H

#include "define.h"
#include <base/vmath.h>

class CGameContext;
class CGameWorld;

class IWeapon
{
private:
    int m_WeaponID;
    CGameContext *m_pGameServer;
    int m_ShowType;
    int m_FireDelay;
public:
    IWeapon(CGameContext *pGameServer, int WeaponID, int ShowType, int FireDelay);
    CGameContext *GameServer() const;
    CGameWorld *GameWorld() const;
    int GetWeaponID() const;
    int GetShowType() const;
    int GetFireDelay() const;
    virtual void OnFire(int Owner, vec2 Dir, vec2 Pos) = 0;
};

class CWeapon : public IWeapon
{
public:
    CWeapon(CGameContext *pGameServer, int WeaponID, int m_ShowType, int FireDelay);
    virtual void OnFire(int Owner, vec2 Dir, vec2 Pos);
    virtual void Fire(int Owner, vec2 Dir, vec2 Pos) = 0;
};

class WeaponSystem
{
public:
	void InitWeapon(int Number, IWeapon *pWeapon);
	IWeapon *m_apLastDayWeapon[NUM_LD_WEAPONS];
};

extern WeaponSystem g_Weapons;
#endif