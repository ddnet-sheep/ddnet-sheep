#include "lovely.h"

#include "game/server/entities/character.h"
#include <base/math.h>
#include <base/vmath.h>
#include <engine/shared/protocol.h>
#include <game/server/entity.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/gameworld.h>
#include <game/server/player.h>
#include <game/server/teams.h>
#include <generated/protocol.h>

CLovely::CLovely(CCharacter* pCharacter) :
	CEntityOwned(CGameWorld::ENTTYPE_LOVELY, COSMETIC_LOVELY, pCharacter, MAX_HEARTS, pCharacter->GetPos())
{
	m_SpawnDelay = 0;	
}

void CLovely::Tick() {
	if(HasReset() || !Character())
		return;

	m_Pos = Character()->GetPos();

	m_SpawnDelay--;
	if(m_SpawnDelay <= 0) {
		SpawnNewHeart();
		int SpawnTime = 45;
		m_SpawnDelay = Server()->TickSpeed() - (rand() % (SpawnTime - (SpawnTime - 10) + 1) + (SpawnTime - 10));
	}

	for(int i = 0; i < MAX_HEARTS; i++) {
		if(m_aData[i].m_Lifespan == -1)
			continue;

		m_aData[i].m_Lifespan--;
		m_aData[i].m_Pos.y -= 5.f;

		if(m_aData[i].m_Lifespan == 0 || GameServer()->Collision()->TestBox(m_aData[i].m_Pos, vec2(14.f, 14.f)))
			m_aData[i].m_Lifespan = -1;
	}
}

void CLovely::SpawnNewHeart()
{
	for(int i = 0; i < MAX_HEARTS; i++) {
		if(m_aData[i].m_Lifespan > 0)
			continue;

		m_aData[i].m_Lifespan = Server()->TickSpeed() / 2;
		m_aData[i].m_Pos = vec2(Character()->GetPos().x + (rand() % 50 - 25), Character()->GetPos().y - 30);
		Character()->SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
		break;
	}
}

void CLovely::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	const CPlayer *pSnapPlayer = GameServer()->m_apPlayers[SnappingClient];

	if(!Character() || Character()->IsPaused() || !pSnapPlayer || pSnapPlayer->GetCharacter() && !Character()->CanSnapCharacter(SnappingClient))
		return;

	if(!TeamMask().test(SnappingClient))
		return;

	// todo: vanish
	// if(Character()->GetPlayer()->m_Vanish && SnappingClient != Character()->GetPlayer()->GetCid() && SnappingClient != -1)
	// 	if(!pSnapPlayer->m_Vanish && Server()->GetAuthedState(SnappingClient) < AUTHED_ADMIN)
	// 		return;

	const int SnapVer = Server()->GetClientVersion(SnappingClient);
	const bool SixUp = Server()->IsSixup(SnappingClient);
	for(int i = 0; i < MAX_HEARTS; i++)
	{
		if(m_aData[i].m_Lifespan == -1)
			continue;
		GameServer()->SnapPickup(CSnapContext(SnapVer, SixUp, SnappingClient), m_SnapIds[i], m_aData[i].m_Pos, POWERUP_HEALTH, -1, -1, PICKUPFLAG_NO_PREDICT);
	}
}
