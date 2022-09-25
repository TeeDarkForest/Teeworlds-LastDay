/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "rain.h"

CRain::CRain(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_RAIN)
{
	m_Pos = Pos;
	m_Direction = Dir;

	GameWorld()->InsertEntity(this);
}

void CRain::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CRain::Tick()
{
	m_Pos += m_Direction;
	vec2 CurPos = m_Pos + m_Direction;

	int Collide = GameServer()->Collision()->IntersectLine(m_Pos, CurPos, &CurPos, 0);
	CCharacter *TargetChr = GameServer()->m_World.IntersectCharacter(m_Pos, CurPos, 6.0f, CurPos, 0x0);

	if(TargetChr || Collide || GameLayerClipped(m_Pos))
	{
		if(TargetChr)
		{
			TargetChr->m_ColdTick += 5;
		}
		Reset();
	}
}

void CRain::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient, m_Pos))
		return;

	CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if(!pProj)
		return;
	pProj->m_X = (int)m_Pos.x;
	pProj->m_Y = (int)m_Pos.y;
	pProj->m_VelX = (int)m_Direction.x;
	pProj->m_VelY = (int)m_Direction.y;
	pProj->m_Type = WEAPON_HAMMER;
}
