#include <new>

#include <game/server/gamecontext.h>
#include <game/generated/protocol.h>
#include <engine/shared/config.h>

#include "define.h"
#include "ldgamecontroller.h"

CLastDayGameController::CLastDayGameController(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "LastDay";
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
}
