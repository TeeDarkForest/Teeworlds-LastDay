#include <new>

#include <game/server/gamecontext.h>
#include <game/generated/protocol.h>
#include <engine/shared/config.h>

#include "define.h"
#include "ldgamecontroller.h"

#include "weapons/hammer.h"
#include "weapons/gun.h"
#include "weapons/shotgun.h"
#include "weapons/grenade.h"
#include "weapons/rifle.h"
#include "weapons/ninja.h"

#include "weapons/fire-gun.h"
#include "weapons/hammer-plus.h"

CLastDayGameController::CLastDayGameController(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "LastDay";
	// init weapon
	InitWeapon();
}

void CLastDayGameController::Tick()
{
	int Players=0;
	for(int i = 0;i < MAX_CLIENTS-g_Config.m_LastDayMaxZombNum;i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			Players++;
		}
	}

	if(Players)
	{
		CheckZombie();
	}

	IGameController::Tick();
}

void CLastDayGameController::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	pChr->IncreaseHealth(10);

	// give default weapons
	pChr->GiveWeapon(TWS_WEAPON_HAMMER, 200);
	pChr->GiveWeapon(TWS_WEAPON_NINJA, 200);
	pChr->GiveWeapon(TWS_WEAPON_GUN, 10);
	pChr->GiveWeapon(TWS_WEAPON_SHOTGUN, 10);
	pChr->GiveWeapon(TWS_WEAPON_GRENADE, 10);
	pChr->GiveWeapon(TWS_WEAPON_RIFLE, 10);

	pChr->GiveWeapon(LD_WEAPON_HAMMER_PLUS, 400);
	pChr->GiveWeapon(LD_WEAPON_FIREGUN, 400);
}

void CLastDayGameController::InitWeapon()
{
	g_Weapons.InitWeapon(TWS_WEAPON_HAMMER, new CWeaponHammer(GameServer()));
	g_Weapons.InitWeapon(TWS_WEAPON_NINJA, new CWeaponNinja(GameServer()));
	g_Weapons.InitWeapon(TWS_WEAPON_GUN, new CWeaponGun(GameServer()));
	g_Weapons.InitWeapon(TWS_WEAPON_SHOTGUN, new CWeaponShotgun(GameServer()));
	g_Weapons.InitWeapon(TWS_WEAPON_GRENADE, new CWeaponGrenade(GameServer()));
	g_Weapons.InitWeapon(TWS_WEAPON_RIFLE, new CWeaponRifle(GameServer()));

	g_Weapons.InitWeapon(LD_WEAPON_HAMMER_PLUS, new CWeaponHammerPlus(GameServer()));
	g_Weapons.InitWeapon(LD_WEAPON_FIREGUN, new CWeaponFireGun(GameServer()));
}