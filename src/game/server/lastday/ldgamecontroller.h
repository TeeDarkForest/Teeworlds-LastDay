#ifndef GAME_SERVER_LASTDAY_GAMECONTROLLER_H
#define GAME_SERVER_LASTDAY_GAMECONTROLLER_H
#include <game/server/gamecontroller.h>

class CLastDayGameController : public IGameController
{
	array<vec2> m_aRainSpawns;
public:
	void InitWeapon();

	CLastDayGameController(class CGameContext *pGameServer);
	virtual void Tick();
	virtual void OnCharacterSpawn(class CCharacter *pChr);
	virtual bool OnEntity(const char* pName, vec2 Pivot, vec2 P0, vec2 P1, vec2 P2, vec2 P3, int PosEnv);
	
	virtual bool GetSpawn(vec2 *pPos, int Zomb);
	bool IsSpawnable(vec2 Pos);

	virtual void SpawnRain();

	virtual void CheckZombie();
	virtual int RandomZombieAttack();
};
#endif
