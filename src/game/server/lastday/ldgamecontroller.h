#ifndef GAME_SERVER_LASTDAY_GAMECONTROLLER_H
#define GAME_SERVER_LASTDAY_GAMECONTROLLER_H
#include <game/server/gamecontroller.h>

class CLastDayGameController : public IGameController
{
public:
	CLastDayGameController(class CGameContext *pGameServer);
	virtual void Tick();
	virtual void OnCharacterSpawn(class CCharacter *pChr);
};
#endif
