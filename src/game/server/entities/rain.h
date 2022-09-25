#ifndef GAME_SERVER_LASTDAY_BULLETS_FIRE_H
#define GAME_SERVER_LASTDAY_BULLETS_FIRE_H

#include <game/server/entity.h>

class CRain : public CEntity
{
public:
	CRain(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir);

	vec2 GetPos(float Time);

	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
private:
	vec2 m_Direction;
};

#endif
