#ifndef GAME_SERVER_LASTDAY_BULLETS_FIRE_H
#define GAME_SERVER_LASTDAY_BULLETS_FIRE_H

#include <game/server/entity.h>

class CFire : public CEntity
{
public:
	CFire(CGameWorld *pGameWorld, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, int ShowWeapon, int Weapon);

	vec2 GetPos(float Time);
	void FillInfo(CNetObj_Projectile *pProj);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

private:
	vec2 m_Direction;
	int m_LifeSpan;
	int m_Owner;
	int m_Damage;
    int m_ShowWeapon;
	int m_Weapon;
	int m_StartTick;
};

#endif
