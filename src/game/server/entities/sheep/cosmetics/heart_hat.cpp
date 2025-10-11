// Made by qxdFox
#include "heart_hat.h"

#include "game/server/entities/character.h"
#include <game/server/entity.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/gameworld.h>
#include <game/server/player.h>
#include <game/server/teams.h>

#include <generated/protocol.h>

#include <engine/shared/config.h>

#include <base/vmath.h>

CHeartHat::CHeartHat(CPlayer* Owner) :
	CCosmetic(Owner->GetCharacter()->GameWorld(), CGameWorld::ENTTYPE_HEART_HAT, EItemVariant::ITEM_HEART_HAT, Owner, Owner->GetCharacter()->GetPos(), NUM_HEARTS)
{
}

void CHeartHat::Tick() {
	if(HasReset() || !Character())
		return;

	m_Pos = Character()->GetPos();

	if(!m_switch) {
		m_Dist[HEART_BACK] += 1.80f;
		m_Dist[HEART_FRONT] += 1.80f;

		m_Dist[HEART_MIDDLE] -= 1.80f;
		if(m_Dist[HEART_BACK] > 24.0f)
			m_switch = true;
	} else {
		m_Dist[HEART_BACK] -= 1.68f;
		m_Dist[HEART_FRONT] -= 1.68f;

		m_Dist[HEART_MIDDLE] += 1.68f;
		if(m_Dist[HEART_BACK] < -24.0f)
			m_switch = false;
	}

	for(int i = 0; i < NUM_HEARTS; i++) {
		m_aPos[i] = Character()->GetPos();
		m_aPos[i].x += (int)m_Dist[i];
	}
}

void CHeartHat::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	const CPlayer *pSnapPlayer = GameServer()->m_apPlayers[SnappingClient];

	if(!Character() || Character()->IsPaused() || !pSnapPlayer || pSnapPlayer->GetCharacter() && !Character()->CanSnapCharacter(SnappingClient))
		return;

	CGameTeams Teams = GameServer()->m_pController->Teams();
	const int Team = Character()->Team();

	// todo: mask
	// if(!Teams.SetMask(SnappingClient, Team))
	// 	return;

	// todo: vanish
	// if(Character()->GetPlayer()->m_Vanish && SnappingClient != Character()->GetPlayer()->GetCid() && SnappingClient != -1)
	// 	if(!pSnapPlayer->m_Vanish && Server()->GetAuthedState(SnappingClient) < AUTHED_ADMIN)
	// 		return;

	for(int i = 0; i < NUM_HEARTS; i++) {
		if(m_switch && i == HEART_FRONT)
			continue;

		vec2 Pos = m_aPos[i] + Character()->GetVelocity() + vec2(0, -42);

		if(g_Config.m_SvSheepExperimentalPrediction && !Player()->IsPaused()) {
			const double Pred = Player()->m_PredLatency;
			const float dist = distance(Character()->m_Pos, Character()->m_PrevPos);
			const vec2 nVel = normalize(Character()->GetVelocity()) * Pred * dist / 2.0f;
			Pos = m_aPos[i] + vec2(0, -42) + nVel;
		}

		const int SnapVer = Server()->GetClientVersion(SnappingClient);
		const bool SixUp = Server()->IsSixup(SnappingClient);
		GameServer()->SnapPickup(CSnapContext(SnapVer, SixUp, SnappingClient), m_SnapIds[i], Pos, POWERUP_HEALTH, -1, -1, PICKUPFLAG_NO_PREDICT);
	}
}
