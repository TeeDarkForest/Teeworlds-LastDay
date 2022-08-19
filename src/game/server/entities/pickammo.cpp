/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "pickammo.h"

CPickAmmo::CPickAmmo(CGameWorld *pGameWorld, int Type, vec2 Pos, int Num)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_Type = Type;
	m_Pos = Pos;
	m_Num = Num;
	m_ProximityRadius = PickupPhysSize;

	GameWorld()->InsertEntity(this);
}

void CPickAmmo::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CPickAmmo::Tick()
{
	// Check if a player intersected us
	CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 20.0f, 0);
	if(pChr && pChr->IsAlive() && !pChr->GetPlayer()->IsZomb())
	{
		if(pChr->GetPlayer()->m_aWeapons[m_Type].m_Got == true)
		{
			pChr->GetPlayer()->m_aWeapons[m_Type].m_Ammo+=m_Num;
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);
			Reset();
		}
	}
}

void CPickAmmo::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = POWERUP_WEAPON;
	pP->m_Subtype = m_Type;
}
