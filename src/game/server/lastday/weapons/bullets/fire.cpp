/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "fire.h"

CFire::CFire(CGameWorld *pGameWorld, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, int ShowWeapon, int Weapon)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_FIRE)
{
	m_Pos = Pos;
	m_Direction = Dir;
	m_LifeSpan = Span;
	m_Owner = Owner;
	m_Damage = Damage;
    m_ShowWeapon = ShowWeapon;
	m_Weapon = Weapon;
	m_StartTick = Server()->Tick();

	GameWorld()->InsertEntity(this);
}

void CFire::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

vec2 CFire::GetPos(float Time)
{
	float Curvature = 0;
	float Speed = 0;
    
	Curvature = 0;
	Speed = 6000.0f;

	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}


void CFire::Tick()
{
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetPos(Pt);
	vec2 CurPos = GetPos(Ct);
	int Collide = GameServer()->Collision()->IntersectLine(PrevPos, CurPos, &CurPos, 0);
	CCharacter *OwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *TargetChr = GameServer()->m_World.IntersectCharacter(PrevPos, CurPos, 6.0f, CurPos, OwnerChar);

	m_LifeSpan--;

	if(TargetChr || Collide || m_LifeSpan < 0 || GameLayerClipped(CurPos))
	{
		if(TargetChr)
			TargetChr->TakeDamage(m_Direction, m_Damage, m_Owner, m_ShowWeapon, m_Weapon);

		GameServer()->m_World.DestroyEntity(this);
	}
}

void CFire::TickPaused()
{
	++m_StartTick;
}

void CFire::FillInfo(CNetObj_Projectile *pProj)
{
	vec2 Pos = GetPos((Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed());
	pProj->m_X = (int)Pos.x;
	pProj->m_Y = (int)Pos.y;
	pProj->m_VelX = (int)(m_Direction.x*100.0f);
	pProj->m_VelY = (int)(m_Direction.y*100.0f);
	pProj->m_StartTick = Server()->Tick();
	pProj->m_Type = WEAPON_HAMMER;
}

void CFire::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();

	if(NetworkClipped(SnappingClient, GetPos(Ct)))
		return;

	CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if(pProj)
		FillInfo(pProj);
}
