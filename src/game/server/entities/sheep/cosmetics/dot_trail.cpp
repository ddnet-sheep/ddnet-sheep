// Made by qxdFox
#include "dot_trail.h"

#include <game/server/gamecontext.h>
#include <game/server/entities/character.h>
#include <game/server/teams.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <base/vmath.h>
#include <game/server/gamecontroller.h>

CDotTrail::CDotTrail(CCharacter* pCharacter)
	: CEntityOwned(CGameWorld::ENTTYPE_DOT_TRAIL, COSMETIC_DOT_TRAIL, pCharacter, 0, pCharacter->GetPos())
{
}

void CDotTrail::Snap(int SnappingClient)
{
	const CPlayer *pSnapPlayer = GameServer()->m_apPlayers[SnappingClient];

	if(!Character() || Character()->IsPaused() || !pSnapPlayer || pSnapPlayer->GetCharacter() && !Character()->CanSnapCharacter(SnappingClient))
		return;

	CGameTeams Teams = GameServer()->m_pController->Teams();
	const int Team = Character()->Team();

	if(!TeamMask().test(SnappingClient))
		return;

	// if(pOwnerChr->GetPlayer()->m_Vanish && SnappingClient != pOwnerChr->GetPlayer()->GetCid() && SnappingClient != -1)
	// 	if(!pSnapPlayer->m_Vanish && Server()->GetAuthedState(SnappingClient) < AUTHED_ADMIN)
	// 		return;

	CNetObj_DDNetProjectile *pProj = Server()->SnapNewItem<CNetObj_DDNetProjectile>(GetId());
	if(!pProj)
		return;

	vec2 Pos = m_Pos + Character()->GetVelocity();
	if(g_Config.m_SvSheepExperimentalPrediction && !Player()->IsPaused()) {
		const double Pred = Player()->m_PredLatency;
		const float dist = distance(Character()->m_Pos, Character()->m_PrevPos);
		const vec2 nVel = normalize(Character()->GetVelocity()) * Pred * dist / 2.0f;
		Pos = m_Pos + nVel;
	}

	pProj->m_X = round_to_int(Pos.x * 100.0f);
	pProj->m_Y = round_to_int(Pos.y * 100.0f);
	pProj->m_Type = WEAPON_HAMMER;
	pProj->m_Owner = Player()->GetCid();
	pProj->m_StartTick = 0;
	pProj->m_VelX = 0;
	pProj->m_VelY = 0;
}
