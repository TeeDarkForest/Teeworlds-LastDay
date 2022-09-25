#include <new>

#include <game/server/gamecontext.h>
#include <game/generated/protocol.h>
#include <engine/shared/config.h>

#include <game/server/entities/rain.h>

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
	for(int i = 0;i < MAX_CLIENTS-g_Config.m_LDMaxZombNum;i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			Players++;
		}
	}

	if(Players && GameServer()->NumZombiesAlive() < g_Config.m_LDMaxZombNum)
	{
		CheckZombie();
	}

	if(random_int(1,100) < g_Config.m_LDRainSpawnProba)
		SpawnRain();

	IGameController::Tick();
}

void CLastDayGameController::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	pChr->IncreaseHealth(10);

	// give default weapons
	pChr->GiveWeapon(TWS_WEAPON_HAMMER, 400);
	pChr->GiveWeapon(TWS_WEAPON_GUN, 10);
}

bool CLastDayGameController::OnEntity(const char* pName, vec2 Pivot, vec2 P0, vec2 P1, vec2 P2, vec2 P3, int PosEnv)
{
	vec2 Pos = (P0 + P1 + P2 + P3)/4.0f;
	int Type = -1;
	int SubType = 0;
	if(str_comp(pName, "ldHuman") == 0)
	{
		m_aSpawnPoints[0].add(Pos);
		return true;
	}
	if(str_comp(pName, "ldZombie") == 0)
	{
		m_aSpawnPoints[1].add(Pos);
		return true;
	}
	if(str_comp(pName, "ldRain") == 0)
	{
		m_aRainSpawns.add(Pos);
		return true;
	}
	return false;
}

bool CLastDayGameController::IsSpawnable(vec2 Pos)
{
	//First check if there is a tee too close
	CCharacter *aEnts[MAX_CLIENTS];
	int Num = GameServer()->m_World.FindEntities(Pos, 64, (CEntity**)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	
	for(int c = 0; c < Num; ++c)
	{
		if(distance(aEnts[c]->m_Pos, Pos) <= 30.0f)
			return false;
	}
	
	//Check the center
	if(GameServer()->Collision()->CheckPoint(Pos))
		return false;
	
	return true;
}

bool CLastDayGameController::GetSpawn(vec2 *pPos, int Zomb)
{
	for(int i = 0;i < m_aSpawnPoints[Zomb].size();i ++)
	{
		if(IsSpawnable(m_aSpawnPoints[Zomb][i]))
		{
			*pPos = m_aSpawnPoints[Zomb][i];
			return true;
		}
	}

	return false;
}

void CLastDayGameController::SpawnRain()
{
	for(int i = 0;i < m_aRainSpawns.size();i ++)
	{
		if(random_int(1,100) < g_Config.m_LDRainSpawnProba)
		{
			new CRain(&GameServer()->m_World, m_aRainSpawns[i], vec2(0, 32));
		}
	}
}

void CLastDayGameController::CheckZombie()
{
	for(int i = 0; i < g_Config.m_LDMaxZombNum; i++)//...
	{
		if(!GameServer()->m_apPlayers[i])//Check if the CID is free
		{
			if(GameServer()->NumZombiesAlive() >= g_Config.m_LDMaxZombNum)
				break;
			
			int Attack = RandomZombieAttack();

			GameServer()->OnZombie(MAX_CLIENTS-i-1, Attack);//Create a Zombie Finally
		}
	}
}

int CLastDayGameController::RandomZombieAttack()
{
	int Attack;
	if(random_int(0, 100) > 100 - g_Config.m_LDZombAttackProba)
	{
		do
		{
			Attack = random_int(1, 15);
		}
		while(random_int(0, 100) > Attack * random_int(5, 7));
	}
	else Attack = random_int(0, 1);

	return Attack;
}