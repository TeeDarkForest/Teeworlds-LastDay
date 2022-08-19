/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "lastday.h"

#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>
#include <new>

CGameControllerLastDay::CGameControllerLastDay(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	// Exchange this to a string that identifies your game mode.
	// DM, TDM and CTF are reserved for teeworlds original modes.
	m_pGameType = "LastDay";

	//m_GameFlags = GAMEFLAG_TEAMS; // GAMEFLAG_TEAMS makes it a two-team gamemode
}

void CGameControllerLastDay::Tick()
{
	// this is the main part of the gamemode, this function is run every tick
	int Players = 0;
	for(int i = 0;i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[Players])
		{
			if(!GameServer()->m_apPlayers[Players]->IsZomb())
			{
				Players++;
			}
		}
	}

	
	if(m_GameOverTick == -1)
	{
		CheckZombie();
	}
	IGameController::Tick();
}
