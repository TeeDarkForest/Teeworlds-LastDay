/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <bitset>

#include <base/tl/ic_array.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>

#include "character.h"

//input count
struct CInputCount
{
	int m_Presses;
	int m_Releases;
};

CInputCount CountInput(int Prev, int Cur)
{
	CInputCount c = {0, 0};
	Prev &= INPUT_STATE_MASK;
	Cur &= INPUT_STATE_MASK;
	int i = Prev;

	while(i != Cur)
	{
		i = (i+1)&INPUT_STATE_MASK;
		if(i&1)
			c.m_Presses++;
		else
			c.m_Releases++;
	}

	return c;
}


MACRO_ALLOC_POOL_ID_IMPL(CCharacter, MAX_CLIENTS)

// Character, "physical" player's part
CCharacter::CCharacter(CGameWorld *pWorld)
: CEntity(pWorld, CGameWorld::ENTTYPE_CHARACTER)
{
	m_ProximityRadius = ms_PhysSize;
	m_Health = 0;
	m_Armor = 0;
	for(int i = 0;i < 13;i++)
	{
		m_FakeIDs[i] = Server()->SnapNewID();
	}
	m_FreezeHelpID = Server()->SnapNewID();
}

CCharacter::~CCharacter()
{
	for(int i = 0;i < 13;i++)
	{
		Server()->SnapFreeID(m_FakeIDs[i]);
	}
	Server()->SnapFreeID(m_FreezeHelpID);
}

void CCharacter::Reset()
{
	Destroy();
}

bool CCharacter::Spawn(CPlayer *pPlayer, vec2 Pos)
{
	if(IsGrounded())

	m_EmoteStop = -1;
	m_LastAction = -1;
	m_LastNoAmmoSound = -1;
	m_ActiveWeapon = TWS_WEAPON_HAMMER;
	m_LastWeapon = TWS_WEAPON_HAMMER;
	m_QueuedWeapon = -1;

	m_pPlayer = pPlayer;
	m_Pos = Pos;

	for(int i=0;i < NUM_LD_WEAPONS;i++)
	{
		m_aWeapons[i].m_Ammo = -1;
	}

	m_Core.Reset();
	m_Core.Init(&GameServer()->m_World.m_Core, GameServer()->Collision());
	m_Core.m_Pos = m_Pos;
	GameServer()->m_World.m_Core.m_apCharacters[GetCID()] = &m_Core;

	m_ReckoningTick = 0;
	mem_zero(&m_SendCore, sizeof(m_SendCore));
	mem_zero(&m_ReckoningCore, sizeof(m_ReckoningCore));

	GameServer()->m_World.InsertEntity(this);
	m_Alive = true;

	GameServer()->m_pController->OnCharacterSpawn(this);

	m_FrozenTime = -1;

	m_AI.m_JumpedTick = 0;
	m_AI.m_FireTick = 0;
	return true;
}

void CCharacter::Destroy()
{
	GameServer()->m_World.m_Core.m_apCharacters[GetCID()] = 0;
	m_Alive = false;
}

void CCharacter::SetWeapon(int W)
{
	if(W == m_ActiveWeapon)
		return;

	m_LastWeapon = m_ActiveWeapon;
	m_QueuedWeapon = -1;
	m_ActiveWeapon = W;
	GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH);

	if(m_ActiveWeapon < 0 || m_ActiveWeapon >= NUM_LD_WEAPONS)
		m_ActiveWeapon = 0;
}

bool CCharacter::IsGrounded()
{
	if(GameServer()->Collision()->CheckPoint(m_Pos.x+m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x-m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;

	int MoveRestrictionsBelow = GameServer()->Collision()->GetMoveRestrictions(m_Pos + vec2(0, m_ProximityRadius / 2 + 4), 0.0f);
	return (MoveRestrictionsBelow & CANTMOVE_DOWN) != 0;
}

void CCharacter::HandleNinja()
{
	if(m_ActiveWeapon != TWS_WEAPON_NINJA)
		return;

	m_Ninja.m_CurrentMoveTime--;

	if (m_Ninja.m_CurrentMoveTime == 0)
	{
		// reset velocity
		m_Core.m_Vel = m_Ninja.m_ActivationDir*m_Ninja.m_OldVelAmount;
	}

	if (m_Ninja.m_CurrentMoveTime > 0)
	{
		// Set velocity
		m_Core.m_Vel = m_Ninja.m_ActivationDir * g_pData->m_Weapons.m_Ninja.m_Velocity;
		vec2 OldPos = m_Pos;
		GameServer()->Collision()->MoveBox(&m_Core.m_Pos, &m_Core.m_Vel, vec2(m_ProximityRadius, m_ProximityRadius), 0.f);

		// reset velocity so the client doesn't predict stuff
		m_Core.m_Vel = vec2(0.f, 0.f);

		// check if we Hit anything along the way
		{
			CCharacter *aEnts[MAX_CLIENTS];
			vec2 Dir = m_Pos - OldPos;
			float Radius = m_ProximityRadius * 2.0f;
			vec2 Center = OldPos + Dir * 0.5f;
			int Num = GameServer()->m_World.FindEntities(Center, Radius, (CEntity**)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			for (int i = 0; i < Num; ++i)
			{
				if (aEnts[i] == this)
					continue;

				// make sure we haven't Hit this object before
				bool bAlreadyHit = false;
				for (int j = 0; j < m_NumObjectsHit; j++)
				{
					if (m_apHitObjects[j] == aEnts[i])
						bAlreadyHit = true;
				}
				if (bAlreadyHit)
					continue;

				// check so we are sufficiently close
				if (distance(aEnts[i]->m_Pos, m_Pos) > (m_ProximityRadius * 2.0f))
					continue;

				// Hit a player, give him damage and stuffs...
				GameServer()->CreateSound(aEnts[i]->m_Pos, SOUND_NINJA_HIT);
				// set his velocity to fast upward (for now)
				if(m_NumObjectsHit < 10)
					m_apHitObjects[m_NumObjectsHit++] = aEnts[i];

				aEnts[i]->TakeDamage(vec2(0, -10.0f), g_pData->m_Weapons.m_Ninja.m_pBase->m_Damage, GetCID(), WEAPON_NINJA);
			}
		}

		return;
	}

	return;
}


void CCharacter::DoWeaponSwitch()
{
	// make sure we can switch
	if(m_ReloadTimer != 0 || m_QueuedWeapon == -1)
		return;

	// switch Weapon
	SetWeapon(m_QueuedWeapon);
	GameServer()->SendChatTarget(GetCID(), "Weapon: {int:Weapon}", "Weapon", &m_ActiveWeapon, NULL);
}

void CCharacter::HandleWeaponSwitch()
{
	int WantedWeapon = m_ActiveWeapon;
	if(m_QueuedWeapon != -1)
		WantedWeapon = m_QueuedWeapon;

	// select Weapon
	int Next = CountInput(m_LatestPrevInput.m_NextWeapon, m_LatestInput.m_NextWeapon).m_Presses;
	int Prev = CountInput(m_LatestPrevInput.m_PrevWeapon, m_LatestInput.m_PrevWeapon).m_Presses;

	if(Next < 128) // make sure we only try sane stuff
	{
		while(Next) // Next Weapon selection
		{
			WantedWeapon = (WantedWeapon+1)%NUM_LD_WEAPONS;
			if(m_aWeapons[WantedWeapon].m_Ammo > -1)
				Next--;
		}
	}

	if(Prev < 128) // make sure we only try sane stuff
	{
		while(Prev) // Prev Weapon selection
		{
			WantedWeapon = (WantedWeapon-1)<0?NUM_LD_WEAPONS-1:WantedWeapon-1;
			if(m_aWeapons[WantedWeapon].m_Ammo > -1)
				Prev--;
		}
	}

	// Direct Weapon selection
	if(m_LatestInput.m_WantedWeapon)
		WantedWeapon = m_Input.m_WantedWeapon-1;

	// check for insane values
	if(WantedWeapon >= 0 && WantedWeapon < NUM_LD_WEAPONS && WantedWeapon != m_ActiveWeapon && m_aWeapons[WantedWeapon].m_Ammo > -1)
		m_QueuedWeapon = WantedWeapon;

	DoWeaponSwitch();
}

void CCharacter::FireWeapon()
{
	if(m_ReloadTimer != 0)
		return;

	if(IsFrozen() || !IsAlive())
	{
		return;
	}

	DoWeaponSwitch();
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));

	bool FullAuto = false;
	if(m_ActiveWeapon == TWS_WEAPON_GRENADE || m_ActiveWeapon == TWS_WEAPON_SHOTGUN || m_ActiveWeapon == TWS_WEAPON_RIFLE)
		FullAuto = true;


	// check if we gonna fire
	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1) && m_aWeapons[m_ActiveWeapon].m_Ammo && !m_pPlayer->GetZomb())
		WillFire = true;

	if(!WillFire)
		return;

	// check for ammo
	if(!m_aWeapons[m_ActiveWeapon].m_Ammo && !m_pPlayer->GetZomb())
	{
		// 125ms is a magical limit of how fast a human can click
		m_ReloadTimer = 125 * Server()->TickSpeed() / 1000;
		if(m_LastNoAmmoSound+Server()->TickSpeed() <= Server()->Tick())
		{
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
			m_LastNoAmmoSound = Server()->Tick();
		}
		return;
	}
	IWeapon *pWeapon = GameServer()->GetLastDayWeapon(m_ActiveWeapon);
	if(!pWeapon)
		return;

	vec2 ProjStartPos = m_Pos+Direction*m_ProximityRadius*0.75f;
	int ClientID = GetCID();

	if(m_ActiveWeapon == TWS_WEAPON_HAMMER || m_ActiveWeapon == TWS_WEAPON_NINJA)
	{
		// reset objects Hit
		m_NumObjectsHit = 0;
	}

	pWeapon->OnFire(GetCID(), Direction, ProjStartPos);

	m_AttackTick = Server()->Tick();

	if(m_aWeapons[m_ActiveWeapon].m_Ammo > 0) // no ammo unlimited
		m_aWeapons[m_ActiveWeapon].m_Ammo--;

	if(!m_ReloadTimer)
		m_ReloadTimer = g_pData->m_Weapons.m_aId[m_ActiveWeapon].m_Firedelay * Server()->TickSpeed() / 1000;
}

void CCharacter::HandleWeapons()
{
	//ninja
	HandleNinja();

	// check reload timer
	if(m_ReloadTimer)
	{
		m_ReloadTimer--;
		return;
	}

	// fire Weapon, if wanted
	FireWeapon();

	return;
}

void CCharacter::GiveWeapon(int WeaponID, int Ammo)
{
	if(WeaponID < TWS_WEAPON_HAMMER && WeaponID >= NUM_LD_WEAPONS)
		return;
	if(m_aWeapons[WeaponID].m_Ammo < 0)
	{
		m_aWeapons[WeaponID].m_Ammo = 0;
	}
	m_aWeapons[WeaponID].m_Ammo += Ammo;
}

void CCharacter::GiveNinja()
{
	m_Ninja.m_ActivationTick = Server()->Tick();
	m_aWeapons[TWS_WEAPON_NINJA].m_Ammo = 1;
	if (m_ActiveWeapon != TWS_WEAPON_NINJA)
		m_LastWeapon = m_ActiveWeapon;
	m_ActiveWeapon = TWS_WEAPON_NINJA;

	GameServer()->CreateSound(m_Pos, SOUND_PICKUP_NINJA);
}

void CCharacter::SetEmote(int Emote, int Tick)
{
	m_EmoteType = Emote;
	m_EmoteStop = Tick;
}

void CCharacter::OnPredictedInput(CNetObj_PlayerInput *pNewInput)
{
	// check for changes
	if(mem_comp(&m_Input, pNewInput, sizeof(CNetObj_PlayerInput)) != 0)
		m_LastAction = Server()->Tick();

	// copy new input
	mem_copy(&m_Input, pNewInput, sizeof(m_Input));
	m_NumInputs++;

	// it is not allowed to aim in the center
	if(m_Input.m_TargetX == 0 && m_Input.m_TargetY == 0)
		m_Input.m_TargetY = -1;
}

void CCharacter::OnDirectInput(CNetObj_PlayerInput *pNewInput)
{
	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
	mem_copy(&m_LatestInput, pNewInput, sizeof(m_LatestInput));

	// it is not allowed to aim in the center
	if(m_LatestInput.m_TargetX == 0 && m_LatestInput.m_TargetY == 0)
		m_LatestInput.m_TargetY = -1;

	if(m_NumInputs > 2 && m_pPlayer->GetTeam() != TEAM_SPECTATORS)
	{
		HandleWeaponSwitch();
		FireWeapon();
	}

	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
}

void CCharacter::ResetInput()
{
	m_Input.m_Direction = 0;
	m_Input.m_Hook = 0;
	// simulate releasing the fire button
	if((m_Input.m_Fire&1) != 0)
		m_Input.m_Fire++;
	m_Input.m_Fire &= INPUT_STATE_MASK;
	m_Input.m_Jump = 0;
	m_LatestPrevInput = m_LatestInput = m_Input;
}

void CCharacter::Tick()
{
	if(!m_pPlayer || !IsAlive())//bugfix
		return;

	if(!IsFrozen())// Just For Zombie
		DoZombieAction();
	
	if(!IsAlive())//Boomer kills himself
		return;

	if(IsFrozen())
	{
		m_FrozenTime--;
		m_Input.m_Direction = 0;
		m_Input.m_Hook = 0;
		m_Input.m_Fire = 0;
		if(m_FrozenTime <= 0)
			Unfreeze();
		else
		{
			if (m_FrozenTime % Server()->TickSpeed() == Server()->TickSpeed() - 1)
				GameServer()->CreateDamageInd(m_Pos, 0, (m_FrozenTime + 1) / Server()->TickSpeed());		
		}
	}

	if(m_AI.m_FireTick)
	{
		m_AI.m_FireTick--;
	}

	if (m_EmoteStop < Server()->Tick())
	{
		m_EmoteType = EMOTE_NORMAL;
		m_EmoteStop = -1;
	}
	
	// snap
	for(int i = 0;i < MAX_CLIENTS;i++)
	{
		CCharacter *pNeedSnap = GameServer()->GetPlayerChar(i);
		m_RealSnapPlayer[i] = false;
		if(pNeedSnap)
		{
			m_RealSnapPlayer[i] = true;
		}
	}

	if(m_pPlayer->m_ForceBalanced)
	{
		char Buf[128];
		str_format(Buf, sizeof(Buf), "You were moved to %s due to team balancing", GameServer()->m_pController->GetTeamName(m_pPlayer->GetTeam()));
		GameServer()->SendBroadcast(Buf, GetCID());

		m_pPlayer->m_ForceBalanced = false;
	}

	UpdateTuningParam();

	m_Core.m_Input = m_Input;

	CCharacterCore::CParams CoreTickParams(m_pPlayer->GetNextTuningParams());

	m_Core.Tick(true, &CoreTickParams);

	// handle Weapons
	HandleWeapons();
	//Hanle zones
	HandleZones();

	// Previnput
	m_PrevInput = m_Input;

	return;
}

void CCharacter::TickDefered()
{
	// advance the dummy
	{
		CCharacterCore::CParams CoreTickParams(&GameWorld()->m_Core.m_Tuning);
		CWorldCore TempWorld;
		m_ReckoningCore.Init(&TempWorld, GameServer()->Collision());
		m_ReckoningCore.Tick(false, &CoreTickParams);
		m_ReckoningCore.Move(&CoreTickParams);
		m_ReckoningCore.Quantize();
	}

	CCharacterCore::CParams CoreTickParams(m_pPlayer->GetNextTuningParams());
	
	//lastsentcore
	vec2 StartPos = m_Core.m_Pos;
	vec2 StartVel = m_Core.m_Vel;
	bool StuckBefore = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));

	m_Core.Move(&CoreTickParams);
	bool StuckAfterMove = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Core.Quantize();
	bool StuckAfterQuant = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Pos = m_Core.m_Pos;

	if(!StuckBefore && (StuckAfterMove || StuckAfterQuant))
	{
		// Hackish solution to get rid of strict-aliasing warning
		union
		{
			float f;
			unsigned u;
		}StartPosX, StartPosY, StartVelX, StartVelY;

		StartPosX.f = StartPos.x;
		StartPosY.f = StartPos.y;
		StartVelX.f = StartVel.x;
		StartVelY.f = StartVel.y;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "STUCK!!! %d %d %d %f %f %f %f %x %x %x %x",
			StuckBefore,
			StuckAfterMove,
			StuckAfterQuant,
			StartPos.x, StartPos.y,
			StartVel.x, StartVel.y,
			StartPosX.u, StartPosY.u,
			StartVelX.u, StartVelY.u);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	}

	int Events = m_Core.m_TriggeredEvents;
	std::bitset<MAX_CLIENTS> Mask = CmaskAllExceptOne(GetCID());

	if(Events&COREEVENT_HOOK_ATTACH_PLAYER) GameServer()->CreateSound(m_Pos, SOUND_HOOK_ATTACH_PLAYER, CmaskAll());
	
	if(Events&COREEVENT_GROUND_JUMP) GameServer()->CreateSound(m_Pos, SOUND_PLAYER_JUMP, Mask);
	if(Events&COREEVENT_HOOK_ATTACH_GROUND) GameServer()->CreateSound(m_Pos, SOUND_HOOK_ATTACH_GROUND, Mask);
	if(Events&COREEVENT_HOOK_HIT_NOHOOK) GameServer()->CreateSound(m_Pos, SOUND_HOOK_NOATTACH, Mask);


	if(m_pPlayer->GetTeam() == TEAM_SPECTATORS)
	{
		m_Pos.x = m_Input.m_TargetX;
		m_Pos.y = m_Input.m_TargetY;
	}

	// update the m_SendCore if needed
	{
		CNetObj_Character Predicted;
		CNetObj_Character Current;
		mem_zero(&Predicted, sizeof(Predicted));
		mem_zero(&Current, sizeof(Current));
		m_ReckoningCore.Write(&Predicted);
		m_Core.Write(&Current);

		// only allow dead reackoning for a top of 3 seconds
		if(m_ReckoningTick+Server()->TickSpeed()*3 < Server()->Tick() || mem_comp(&Predicted, &Current, sizeof(CNetObj_Character)) != 0)
		{
			m_ReckoningTick = Server()->Tick();
			m_SendCore = m_Core;
			m_ReckoningCore = m_Core;
		}
	}
}

void CCharacter::TickPaused()
{
	++m_AttackTick;
	++m_DamageTakenTick;
	++m_Ninja.m_ActivationTick;
	++m_ReckoningTick;
	if(m_LastAction != -1)
		++m_LastAction;
	if(m_EmoteStop > -1)
		++m_EmoteStop;
}

bool CCharacter::IncreaseHealth(int Amount)
{
	if(m_Health >= 10)
		return false;
	m_Health = clamp(m_Health+Amount, 0, 10);
	return true;
}

bool CCharacter::IncreaseArmor(int Amount)
{
	if(m_Armor >= 10)
		return false;
	m_Armor = clamp(m_Armor+Amount, 0, 10);
	return true;
}

void CCharacter::Die(int Killer, int ShowWeapon, int Weapon)
{
	if(Weapon == -1)
	{
		Weapon = ShowWeapon;
	}

	// we got to wait 0.5 secs before respawning
	m_pPlayer->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
	
	if(Killer >= 0 && Killer < MAX_CLIENTS && GameServer()->m_apPlayers[Killer])
	{
		int ModeSpecial = GameServer()->m_pController->OnCharacterDeath(this, GameServer()->m_apPlayers[Killer], Weapon);
		if(!GameServer()->m_apPlayers[Killer]->GetZomb() && !m_pPlayer->GetZomb())
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "kill killer='%d:%s' victim='%d:%s' weapon=%d special=%d",
				Killer, Server()->ClientName(Killer),
				GetCID(), Server()->ClientName(GetCID()), Weapon, ModeSpecial);
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

			// send the kill message
			CNetMsg_Sv_KillMsg Msg;
			Msg.m_Killer = Killer;
			Msg.m_Victim = GetCID();
			Msg.m_Weapon = ShowWeapon;
			Msg.m_ModeSpecial = ModeSpecial;
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
		}
	}
	// a nice sound
	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_DIE);

	// this is for auto respawn after 1 secs
	m_pPlayer->m_DieTick = Server()->Tick();

	m_Alive = false;
	GameServer()->m_World.RemoveEntity(this);
	GameServer()->m_World.m_Core.m_apCharacters[GetCID()] = 0;
	GameServer()->CreateDeath(m_Pos, GetCID());

	if(m_pPlayer && m_pPlayer->m_Zomb)
		GameServer()->OnZombieKill(GetCID());//remove the player to get a new one
}

bool CCharacter::TakeDamage(vec2 Force, int Dmg, int From, int ShowWeapon, int Weapon)
{
	if(Weapon == -1)
	{
		Weapon = ShowWeapon;
	}

	m_Core.m_Vel += Force;

	CPlayer *pFrom = GameServer()->m_apPlayers[From];

	if(From == GetCID())
	{
		if(!m_pPlayer->GetZomb())
			return false;
	}
	else if(pFrom && pFrom->GetZomb() && m_pPlayer->GetZomb())
		return false;

	m_DamageTaken++;

	// create healthmod indicator
	if(Server()->Tick() < m_DamageTakenTick+25)
	{
		// make sure that the damage indicators doesn't group together
		GameServer()->CreateDamageInd(m_Pos, m_DamageTaken*0.25f, Dmg);
	}
	else
	{
		m_DamageTaken = 0;
		GameServer()->CreateDamageInd(m_Pos, 0, Dmg);
	}

	if(Dmg)
	{
		if(m_Armor)
		{
			if(Dmg > 1)
			{
				m_Health--;
				Dmg--;
			}

			if(Dmg > m_Armor)
			{
				Dmg -= m_Armor;
				m_Armor = 0;
			}
			else
			{
				m_Armor -= Dmg;
				Dmg = 0;
			}
		}

		m_Health -= Dmg;
	}

	m_DamageTakenTick = Server()->Tick();

	// do damage Hit sound
	if(From >= 0 && From != GetCID() && GameServer()->m_apPlayers[From])
	{
		std::bitset<MAX_CLIENTS> Mask = CmaskOne(From);
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->m_SpectatorID == From)
				Mask |= CmaskOne(i);
		}
		GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, Mask);
	}

	// check for death
	if(m_Health <= 0)
	{
		// set attacker's face to happy (taunt!)
		if (From >= 0 && From != GetCID() && GameServer()->m_apPlayers[From])
		{
			CCharacter *pChr = GameServer()->m_apPlayers[From]->GetCharacter();
			if (pChr)
			{
				pChr->m_EmoteType = EMOTE_HAPPY;
				pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed();
			}
		}

		Die(From, ShowWeapon, Weapon);

		return false;
	}

	if (Dmg > 2)
		GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
	else
		GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);

	m_EmoteType = EMOTE_PAIN;
	m_EmoteStop = Server()->Tick() + 500 * Server()->TickSpeed() / 1000;

	return true;
}

void CCharacter::Snap(int SnappingClient)
{
	int Id = GetCID();

	if (!Server()->Translate(Id, SnappingClient))
		return;

	if(NetworkClipped(SnappingClient))
	{
		return;
	}
	CCharacter *pSnappingChr = GameServer()->GetPlayerChar(SnappingClient);
	// If snapping client snap character >= 64, choose snap laser
	if(pSnappingChr && !pSnappingChr->m_RealSnapPlayer[Id])
	{
		CNetObj_Laser *pLaser = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_FakeIDs[0], sizeof(CNetObj_Laser)));
		if(!pLaser)
			return;
		pLaser->m_FromX = (int)m_Pos.x;
		pLaser->m_FromY = (int)m_Pos.y;
		pLaser->m_X = (int)m_Pos.x;
		pLaser->m_Y = (int)m_Pos.y;
		pLaser->m_StartTick = Server()->Tick();

		int Angle = 0;
		
		for(int i=1;i < 13;i++)
		{
			vec2 StartPos = m_Pos + (GetDir(Angle*pi/180) * (m_ProximityRadius-1));
			Angle += 360/12;
			vec2 EndPos = m_Pos + (GetDir(Angle*pi/180) * (m_ProximityRadius-1));
			CNetObj_Laser *pLaser = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_FakeIDs[i], sizeof(CNetObj_Laser)));
			if(!pLaser)
				continue;
			pLaser->m_FromX = (int)StartPos.x;
			pLaser->m_FromY = (int)StartPos.y;
			pLaser->m_X = (int)EndPos.x;
			pLaser->m_Y = (int)EndPos.y;
			pLaser->m_StartTick = Server()->Tick()-3;
		}
		return;
	}

	CNetObj_Character *pCharacter = static_cast<CNetObj_Character *>(Server()->SnapNewItem(NETOBJTYPE_CHARACTER, Id, sizeof(CNetObj_Character)));
	if(!pCharacter)
		return;

	// write down the m_Core
	if(!m_ReckoningTick || GameServer()->m_World.m_Paused)
	{
		// no dead reckoning when paused because the client doesn't know
		// how far to perform the reckoning
		pCharacter->m_Tick = 0;
		m_Core.Write(pCharacter);
	}
	else
	{
		pCharacter->m_Tick = m_ReckoningTick;
		m_SendCore.Write(pCharacter);
	}

	if (pCharacter->m_HookedPlayer != -1)
	{
		if (!Server()->Translate(pCharacter->m_HookedPlayer, SnappingClient))
			pCharacter->m_HookedPlayer = -1;
	}
	int EmoteType = m_EmoteType;
	if(IsFrozen())
	{
		EmoteType = EMOTE_PAIN;

		CNetObj_Laser *pFreeze = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_FreezeHelpID, sizeof(CNetObj_Laser)));
		vec2 StartPos = vec2(m_Pos.x - g_Config.m_SvFreezeHelpLen / 2, m_Pos.y - 20 - m_ProximityRadius);
		float FreezeHelpLen = g_Config.m_SvFreezeHelpLen;
		float FreezeTime = g_Config.m_SvFreezeTime;
		pFreeze->m_FromX = (int)StartPos.x;
		pFreeze->m_FromY = (int)StartPos.y;
		pFreeze->m_X = (int)StartPos.x + FreezeHelpLen / ((FreezeTime * Server()->TickSpeed()) / m_FrozenTime);
		pFreeze->m_Y = (int)StartPos.y;
		pFreeze->m_StartTick = Server()->Tick() - 3;
	}
	pCharacter->m_Emote = EmoteType;

	pCharacter->m_AmmoCount = 0;
	pCharacter->m_Health = 0;
	pCharacter->m_Armor = 0;

	pCharacter->m_Weapon = GameServer()->GetLastDayWeapon(m_ActiveWeapon)->GetShowType();
	pCharacter->m_AttackTick = m_AttackTick;

	pCharacter->m_Direction = m_Input.m_Direction;

	if(GetCID() == SnappingClient || SnappingClient == -1 ||
		(!g_Config.m_SvStrictSpectateMode && GetCID() == GameServer()->m_apPlayers[SnappingClient]->m_SpectatorID))
	{
		pCharacter->m_Health = m_Health;
		pCharacter->m_Armor = m_Armor;
		if(m_aWeapons[m_ActiveWeapon].m_Ammo > 0)
			pCharacter->m_AmmoCount = m_aWeapons[m_ActiveWeapon].m_Ammo;
	}

	if(pCharacter->m_Emote == EMOTE_NORMAL)
	{
		if(250 - ((Server()->Tick() - m_LastAction)%(250)) < 5)
			pCharacter->m_Emote = EMOTE_BLINK;
	}

	pCharacter->m_PlayerFlags = GetPlayer()->m_PlayerFlags;
}

void CCharacter::Teleport(vec2 Pos)
{
	m_Pos = Pos;
	m_Core.m_Pos = m_Pos;

	m_Core.m_HookPos = m_Pos;
	m_Core.m_HookedPlayer = -1;
	m_Core.Reset();
	
	m_Pos = Pos;
	m_Core.m_Pos = m_Pos;
}

void CCharacter::Freeze(float Time, int Reason)
{
	if(!IsFrozen())
		GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);
	m_Core.m_FreezeState = FREEZESTATE_NORMAL;
	m_FrozenTime = Server()->TickSpeed()*Time;

}

void CCharacter::Unfreeze()
{
	if(IsFrozen())
	{
		m_FrozenTime = -1;
		m_Core.m_FreezeState = FREEZESTATE_NOFREEZE;
		GameServer()->CreatePlayerSpawn(m_Pos);
	}
	return;
}

bool CCharacter::IsFrozen() const
{
	return m_FrozenTime > 0;
}

/* Last Day Start */
void CCharacter::HandleZones()
{
	ZoneData Data0;
	ZoneData Data1;
	ZoneData Data2;
	ZoneData Data3;

	GetZoneValueAt(GameServer()->m_ZoneHandle_LastDay, vec2(
		m_Pos.x + m_ProximityRadius / 3.f, m_Pos.y - m_ProximityRadius / 3.f), &Data0);
	
	GetZoneValueAt(GameServer()->m_ZoneHandle_LastDay, vec2(
		m_Pos.x + m_ProximityRadius / 3.f, m_Pos.y + m_ProximityRadius / 3.f), &Data1);
	
	GetZoneValueAt(GameServer()->m_ZoneHandle_LastDay, vec2(
		m_Pos.x - m_ProximityRadius / 3.f, m_Pos.y - m_ProximityRadius / 3.f), &Data2);
	
	GetZoneValueAt(GameServer()->m_ZoneHandle_LastDay, vec2(
		m_Pos.x - m_ProximityRadius / 3.f, m_Pos.y + m_ProximityRadius / 3.f), &Data3);

	icArray<int, 4> Indices;// this array is from infclassR by InfectionDust
	Indices.Add(Data0.Index);
	Indices.Add(Data1.Index);
	Indices.Add(Data2.Index);
	Indices.Add(Data3.Index);

	if(Indices.Contains(ZONE_LASTDAY_DEATH) || GameLayerClipped(m_Pos))
	{
		Die(GetCID(), WEAPON_WORLD);
		return;
	}
	if(Indices.Contains(ZONE_LASTDAY_FREEZE))
	{
		Freeze(g_Config.m_SvFreezeTime, FREEZEREASON_FREEZE_ZONE);
		return;
	}
	if(Indices.Contains(ZONE_LASTDAY_UNFREEZE))
	{
		Unfreeze();
		return;
	}

	HandleTeleports();
}

int CCharacter::GetZoneValueAt(int ZoneHandle, const vec2 &Pos, ZoneData *pData)
{
	return GameServer()->Collision()->GetZoneValueAt(ZoneHandle, Pos, pData);
}

void CCharacter::HandleTeleports()
{
	int Index = GameServer()->Collision()->GetPureMapIndex(m_Pos);

	CTeleTile *pTeleLayer = GameServer()->Collision()->TeleLayer();
	if(!pTeleLayer)
		return;

	int TeleNumber = pTeleLayer[Index].m_Number;
	int TeleType = pTeleLayer[Index].m_Type;
	if((TeleNumber > 0) && (TeleType != TILE_TELEOUT))
	{
		TeleToId(TeleNumber, TeleType);
	}
}

void CCharacter::TeleToId(int TeleNumber, int TeleType)
{
	const std::map<int, std::vector<vec2>> &AllTeleOuts = GameServer()->Collision()->GetTeleOuts();
	if(AllTeleOuts.find(TeleNumber) == AllTeleOuts.cend())
	{
		dbg_msg("LastDay", "No tele out for tele number: %d", TeleNumber);
		return;
	}
	const std::vector<vec2> Outs = AllTeleOuts.at(TeleNumber);
	if(Outs.empty())
	{
		dbg_msg("LastDay", "No tele out for tele number: %d", TeleNumber);
		return;
	}

	switch(TeleType)
	{
		case TILE_TELEINEVIL:
		case TILE_TELEIN:
			break;
		default:
			dbg_msg("LastDay", "Unsupported tele type: %d", TeleType);
			return;
	}

	int DestTeleNumber = random_int(0, Outs.size() - 1);
	vec2 DestPosition = Outs.at(DestTeleNumber);
	m_Core.m_Pos = DestPosition;
	if(TeleType == TILE_TELEINEVIL)
	{
		m_Core.m_Vel = vec2(0, 0);
		GameWorld()->ReleaseHooked(GetPlayer()->GetCID());
	}

	m_Core.m_HookedPlayer = -1;
	m_Core.m_HookState = HOOK_RETRACTED;
	m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
	m_Core.m_HookPos = m_Core.m_Pos;
}

void CCharacter::UpdateTuningParam()
{
	CTuningParams* pTuningParams = m_pPlayer->GetNextTuningParams();
	
	bool NoActions = false;

	if(IsFrozen())
	{
		NoActions = true;
	}
	
	if(NoActions)
	{
		pTuningParams->m_GroundControlAccel = 0.0f;
		pTuningParams->m_GroundJumpImpulse = 0.0f;
		pTuningParams->m_AirJumpImpulse = 0.0f;
		pTuningParams->m_AirControlAccel = 0.0f;
		pTuningParams->m_HookLength = 0.0f;
	}
	
}

void CCharacter::DoZombieAction()
{
	if(!m_pPlayer->GetZomb())
		return;
	CTuningParams *pTuning = m_pPlayer->GetNextTuningParams();
	
	int Radius = m_ProximityRadius*20, VimID=-1;
	CCharacter *pVim = 0;
	for(int i=0;i < MAX_CLIENTS;i ++)
	{
		CCharacter *pChr = GameServer()->GetPlayerChar(i);
		if(pChr && pChr->GetPlayer())
		{
			if(pChr->GetPlayer()->GetZomb())
				continue;
			int Len;
			Len = distance(pChr->m_Pos, m_Pos);
			if(Len < Radius)
			{
				VimID = i;
				pVim = pChr;
				Radius = Len;
			}
		}
	}

	if(!pVim)
	{
		return;
	}

	m_Input.m_Direction = 0;

	if(pVim->m_Pos.x < m_Pos.x)
	{
		m_Input.m_Direction = -1;
	}else if(pVim->m_Pos.x > m_Pos.x)
		m_Input.m_Direction = 1;

	m_Input.m_Jump = 0;
	
	if(pVim->m_Pos.y < m_Pos.y)
	{
		if(IsGrounded())
		{
			m_Input.m_Jump = 1;
		}
		else if(m_AI.m_JumpedTick + Server()->TickSpeed() * 1.5 < Server()->Tick())
		{
			m_Input.m_Jump = 1;
		}
		m_AI.m_JumpedTick = Server()->Tick();
	}

	m_Input.m_TargetX = (int)(pVim->m_Pos.x - m_Pos.x);
	m_Input.m_TargetY = (int)(pVim->m_Pos.y - m_Pos.y);
	m_LatestInput.m_TargetX = m_Input.m_TargetX;
	m_LatestInput.m_TargetY = m_Input.m_TargetY;

	if(m_pPlayer->GetZombValue(0))
	{
		if(((Radius <= pTuning->m_HookLength && Radius >= pTuning->m_HookLength - m_ProximityRadius * 3 && (m_Core.m_HookState == HOOK_IDLE || m_Core.m_HookState == HOOK_FLYING)) || m_Core.m_HookedPlayer >= 0))
		{
			m_Input.m_Hook = 1;
			m_LatestInput.m_Hook = 1;
		}else 
		{
			m_Input.m_Hook = 0;
			m_LatestInput.m_Hook = 0;
		}
	}
	int i = 0;

	while(!m_Input.m_Jump)
	{
		if(TileSafe(m_Pos.x, m_Pos.y + i * 32) == 0)
		{
			m_Input.m_Jump = 1;
		}
		if(TileSafe(m_Pos.x, m_Pos.y == 2))
		{
			break;
		}
		i++;
	}

	if(!m_AI.m_FireTick)
	{
		m_Input.m_Fire = 1;
		m_LatestPrevInput.m_Fire = 1;
		if(Radius < m_ProximityRadius * 3)
			m_ActiveWeapon = TWS_WEAPON_HAMMER;
		else if(Radius < m_ProximityRadius * 6 && m_pPlayer->GetZombValue(2))
			m_ActiveWeapon = TWS_WEAPON_SHOTGUN;
		else if(Radius > m_ProximityRadius * 6 && Radius < m_ProximityRadius * 12 && m_pPlayer->GetZombValue(3))
		{
			m_ActiveWeapon = TWS_WEAPON_GRENADE;
			vec2 Target = GetGrenadeAngle(m_Pos, pVim->m_Pos);
			m_Input.m_TargetX = Target.x;
			m_Input.m_TargetY = Target.y;
		}
		else if(m_pPlayer->GetZombValue(1))
			m_ActiveWeapon = TWS_WEAPON_GUN;
		
		m_AI.m_FireTick = g_Config.m_LastDayZombRefireTick;
	}
	else
	{
		m_Input.m_Fire = 0;
		m_LatestPrevInput.m_Fire = 0;
	}
	return;
}

vec2 CCharacter::GetGrenadeAngle(vec2 m_StartPos, vec2 m_ToShoot)
{
	if(m_ToShoot == vec2(0, 0))
	{
		return vec2(0, 0);
	}
	char aBuf[128];
	vec2 m_Direction;
	float Curvature = m_pPlayer->GetNextTuningParams()->m_GrenadeCurvature;

	m_Direction.x = (m_ToShoot.x - m_StartPos.x);
	m_Direction.y = (m_ToShoot.y - m_StartPos.y - 32*Curvature);
	str_format(aBuf, sizeof(aBuf), "AimPos %d %d", m_Direction.x, m_Direction.y);
	return m_Direction;
}

int CCharacter::TileSafe(float x, float y)
{
	ZoneData ZoneData;
	GetZoneValueAt(GameServer()->m_ZoneHandle_LastDay, vec2(x, y), &ZoneData);
	int Index = ZoneData.Index;

	switch (Index)
	{
		case ZONE_LASTDAY_DEATH:
		case ZONE_LASTDAY_FREEZE:
			return 0;
			break;
		case ZONE_NULL: if(!GameServer()->Collision()->CheckPoint(x, y)) return 1;
	}
	if(y/32.0 > GameServer()->Collision()->GetHeight())
	{
		return -1;
	}

	return 2;
}

int CCharacter::GetCID()
{
	return m_pPlayer->GetCID();
}
/*  Last Day End  */