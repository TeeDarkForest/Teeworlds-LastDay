/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "item.h"

CItem::CItem(CGameWorld *pGameWorld, int Type, vec2 Pos, int Num)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_Type = Type;
	m_Pos = Pos;
	m_Num = Num;
	m_ProximityRadius = ItemPhysSize;
	for(int i = 0;i < 8;i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}

	GameWorld()->InsertEntity(this);
}

CItem::~CItem()
{
	for(int i = 0;i < 8;i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
}


void CItem::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CItem::Tick()
{
	// Check if a player intersected us
	CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 20.0f, 0);
	if(pChr && pChr->IsAlive() && !pChr->GetPlayer()->IsZomb())
	{
        pChr->GetPlayer()->m_aResource[m_Type].m_Num+=m_Num;
        GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);
        GameServer()->SendChatTarget(pChr->GetPlayer()->GetCID(), 
            _("You got {int:Num} {str:resource}"), "Num", &m_Num, 
            "resource", Server()->Localization()->Localize(pChr->GetPlayer()->GetLanguage(),
                GameServer()->GetResourceName(m_Type)), NULL);
        Reset();
	}
}

void CItem::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	float Radius = 16.0f;
	float Angle = 0;
	for(int i = 0;i < 8;i++)
	{
		vec2 Pos = m_Pos + (GetDir(Angle*pi/180) * Radius);
		CNetObj_Projectile *pP = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_IDs[i], sizeof(CNetObj_Projectile)));
		if(!pP)
			return;

		pP->m_X = (int)Pos.x;
		pP->m_Y = (int)Pos.y;
		pP->m_VelX = 0;
		pP->m_VelY = 0;
		pP->m_StartTick = Server()->Tick()-1;
		pP->m_Type = WEAPON_SHOTGUN;
		Angle += 360 / 8;
	}
	CNetObj_Projectile *pP = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_VelX = 0;
	pP->m_VelY = 0;
	pP->m_StartTick = Server()->Tick()-1;
	pP->m_Type = WEAPON_HAMMER;
}
