// made by fokkonaut

#include "lightsaber.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include <game/server/gamecontext.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/gamemodes/sheep/sheep.h>
#include <game/server/teams.h>

#include <generated/protocol.h>
#include <generated/server_data.h>

#include <base/vmath.h>
#include <game/gamecore.h>
#include <game/mapitems.h>
#include <game/server/entity.h>
#include <game/server/gameworld.h>
#include <game/teamscore.h>

CLightsaber::CLightsaber(CCharacter* pCharacter) :
	CEntityOwned(CGameWorld::ENTTYPE_LIGHTSABER, -1, pCharacter, 0, pCharacter->m_Pos) 
{
	m_From = pCharacter->m_Pos;
	m_To = pCharacter->m_Pos;
}

void CLightsaber::Reset() {
    if(GameServer()->Sheep()->m_pLightsabers[m_Player->GetCid()] != nullptr)
        GameServer()->Sheep()->m_pLightsabers[m_Player->GetCid()] = nullptr;

	CEntityOwned::Reset();
}

void CLightsaber::OnFire() {
	if(m_State == STATE_RETRACTED || m_State == STATE_RETRACTING)
		m_State = STATE_EXTENDING;
	else if(m_State == STATE_EXTENDED || m_State == STATE_EXTENDING)
		m_State = STATE_RETRACTING;
}

void CLightsaber::Tick() {
	if(!Character()) {
		Reset();
		return;
	}

	if(Character()->GetActiveWeapon() != WEAPON_LIGHTSABER) {
		if(m_Length == 0) {
			Reset();
			return;
		}

		m_State = STATE_RETRACTING;
	}

	if((Character()->m_FreezeTime > 0 || Character()->IsPaused()) && m_Length > 0)
		m_State = STATE_RETRACTING;

	if(m_State == STATE_EXTENDING) {
		if(Server()->Tick() % 5 == 0)
			GameServer()->CreateSound(m_Pos, SOUND_LASER_BOUNCE, Character()->TeamMask());
		m_Length += LIGHTSABER_SPEED;
		if(m_Length > LIGHTSABER_MAX_LENGTH) {
			m_Length = LIGHTSABER_MAX_LENGTH;
			m_State = STATE_EXTENDED;
		}
	} else if(m_State == STATE_RETRACTING) {
		if(Server()->Tick() % 5 == 0)
			GameServer()->CreateSound(m_Pos, SOUND_HOOK_LOOP, Character()->TeamMask());
		m_Length -= LIGHTSABER_SPEED;
		if(m_Length < 0) {
			m_Length = 0;
			m_State = STATE_RETRACTED;
		}
	}
	m_Pos = Character()->m_Pos;
	m_From = Character()->m_Pos;
	vec2 WantedTo = m_Pos + normalize(vec2(Character()->Input()->m_TargetX, Character()->Input()->m_TargetY)) * m_Length;
	GameServer()->Collision()->IntersectLine(m_Pos, WantedTo, &m_To, 0);

	for(CCharacter *pHit : GameWorld()->IntersectedCharacters(m_From, m_To, 6.0f, Character())) {
		if(Character()->Team() != pHit->Team())
			continue;

		pHit->SetEmote(EMOTE_PAIN, Server()->Tick() + 2);
		if((Server()->Tick() % Server()->TickSpeed()) % 20 == 0) {
			GameServer()->CreateDamageInd(pHit->m_Pos, 90 + 45, 1, Character()->TeamMask());
			GameServer()->CreateSound(pHit->m_Pos, SOUND_PLAYER_PAIN_SHORT, Character()->TeamMask());
		}
	}
}

void CLightsaber::Snap(int SnappingClient) {
	if(NetworkClipped(SnappingClient))
		return;

	if(!Character())
		return;

	if(SnappingClient != SERVER_DEMO_CLIENT) {
		if(!Character()->TeamMask().test(SnappingClient))
			return;

		CCharacter *pSnapper = GameServer()->GetPlayerChar(SnappingClient);

		CPlayer *pSnapPlayer = GameServer()->m_apPlayers[SnappingClient];
		if(!pSnapPlayer)
			return;
	
		if(pSnapPlayer->GetCharacter() && Character())
			if(!Character()->CanSnapCharacter(SnappingClient))
				return;

		// if(pOwnerChar->GetPlayer()->m_Vanish && SnappingClient != pOwnerChar->GetPlayer()->GetCid() && SnappingClient != -1)
		// 	if(!pSnapPlayer->m_Vanish && Server()->GetAuthedState(SnappingClient) < AUTHED_ADMIN)
		// 		return;
	}

	if(m_Length <= 0)
		return;


	vec2 From = m_To + Character()->Core()->m_Vel / 2;
	vec2 To = m_From + Character()->Core()->m_Vel / 2;
	if(SnappingClient == Player()->GetCid())
	{
		From = m_To + Character()->Core()->m_Vel;
		To = m_From + Character()->Core()->m_Vel;
	}

	const int SnapVer = Server()->GetClientVersion(SnappingClient);
	bool SixUp = Server()->IsSixup(SnappingClient);

	GameServer()->SnapLaserObject(CSnapContext(SnapVer, SixUp, SnappingClient), GetId(), To, From, Server()->Tick() - 3, Player()->GetCid(), LASERTYPE_GUN);
}