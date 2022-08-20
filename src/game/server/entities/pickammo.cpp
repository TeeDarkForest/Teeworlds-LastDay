/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>

#include "pickammo.h"

CPickAmmo::CPickAmmo(CGameWorld *pGameWorld, int Type, vec2 Pos, int Num)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_StartTick = Server()->Tick();
	m_Type = Type;
	m_Pos = Pos;
	m_Num = Num;
	m_ProximityRadius = PickupPhysSize;
	for(int i = 0;i < 8;i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}


	GameWorld()->InsertEntity(this);
}

CPickAmmo::~CPickAmmo()
{
	for(int i = 0;i < 8;i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
}

void CPickAmmo::Reset()
{
	dbg_msg("test", "ammo reset!!!!!!!!!Type=%d", m_Type);
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
			pChr->ShowInfo();
			GameServer()->SendChatTarget(pChr->GetPlayer()->GetCID(),
			 _("You got {int:Num} {str:Weapon} ammo"), &m_Num,
			 	GameServer()->Localize(pChr->GetPlayer()->GetLanguage(),
			 		GameServer()->GetWeaponName(m_Type)), NULL);
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);
			Reset();
		}
	}

	if((m_StartTick + (g_Config.m_LdItemLifeSpan * Server()->TickSpeed())) < Server()->Tick())
	{
		Reset();
	}
}

void CPickAmmo::Snap(int SnappingClient)
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
		pP->m_StartTick = Server()->Tick()-3;
		pP->m_Type = WEAPON_HAMMER;
		Angle += 360 / 8;
	}

	CNetObj_Projectile *pP = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_VelX = 0;
	pP->m_VelY = 0;
	pP->m_StartTick = Server()->Tick()-2;
	pP->m_Type = m_Type;
}
