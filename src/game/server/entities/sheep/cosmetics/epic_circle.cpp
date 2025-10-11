#include "epic_circle.h"

#include "game/server/entities/character.h"
#include <game/server/entity.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/gameworld.h>
#include <game/server/player.h>
#include <game/server/teams.h>

#include <engine/shared/config.h>
#include <engine/shared/protocol.h>

#include <generated/protocol.h>

#include <base/math.h>
#include <base/vmath.h>


CEpicCircle::CEpicCircle(CPlayer* Owner) :
	CCosmetic(Owner->GetCharacter()->GameWorld(), CGameWorld::ENTTYPE_PROJECTILE, EItemVariant::ITEM_EPIC_CIRCLE, Owner, Owner->GetCharacter()->GetPos(), MAX_PARTICLES)
{
}

void CEpicCircle::Tick() {
	if(HasReset() || !Character())
		return;

	m_Pos = Character()->GetPos();

	for(int i = 0; i < MAX_PARTICLES; i++) {
		float rad = 16.0f * powf(sinf(Server()->Tick() / 30.0f), 3) * 1 + 75;
		float TurnFac = 0.025f;
		m_RotatePos[i].x = cosf(2 * pi * (i / (float)MAX_PARTICLES) + Server()->Tick() * TurnFac) * rad;
		m_RotatePos[i].y = sinf(2 * pi * (i / (float)MAX_PARTICLES) + Server()->Tick() * TurnFac) * rad;
	}
}

void CEpicCircle::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	const CPlayer *pSnapPlayer = GameServer()->m_apPlayers[SnappingClient];

	if(!Character() || Character()->IsPaused() || !pSnapPlayer)
		return;

	CGameTeams Teams = GameServer()->m_pController->Teams();
	const int Team = Character()->Team();

	// todo: mask
	// if(!Teams.SetMask(SnappingClient, Team))
	// 	return;

	// todo: vanish
	// if(pOwnerChr->GetPlayer()->m_Vanish && SnappingClient != pOwnerChr->GetPlayer()->GetCid() && SnappingClient != -1)
	// 	if(!pSnapPlayer->m_Vanish && Server()->GetAuthedState(SnappingClient) < AUTHED_ADMIN)
	// 		return;

	// if(pOwnerChr->GetPlayer()->m_Vanish && SnappingClient != pOwnerChr->GetPlayer()->GetCid() && SnappingClient != -1)
	// 	if(!pSnapPlayer->m_Vanish && Server()->GetAuthedState(SnappingClient) < AUTHED_ADMIN)
	// 		return;

	for(int i = 0; i < MAX_PARTICLES; i++)
	{
		CNetObj_DDNetProjectile *pProj = Server()->SnapNewItem<CNetObj_DDNetProjectile>(m_SnapIds[i]);
		if(!pProj)
			return;

		vec2 Pos = m_Pos + m_RotatePos[i] + Character()->GetVelocity();
		if(g_Config.m_SvSheepExperimentalPrediction && !Player()->IsPaused())
		{
			const double Pred = Player()->m_PredLatency;
			const float dist = distance(Character()->m_Pos, Character()->m_PrevPos);
			const vec2 nVel = normalize(Character()->GetVelocity()) * Pred * dist / 2.0f;
			Pos = m_Pos + m_RotatePos[i] + nVel;
		}

		pProj->m_X = round_to_int(Pos.x * 100.0f);
		pProj->m_Y = round_to_int(Pos.y * 100.0f);
		pProj->m_Type = WEAPON_HAMMER;
		pProj->m_Owner = Player()->GetCid();
		pProj->m_StartTick = 0;
		pProj->m_VelX = 0;
		pProj->m_VelY = 0;
	}
}
