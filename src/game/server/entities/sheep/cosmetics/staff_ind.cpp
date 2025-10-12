#include "staff_ind.h"

#include "game/server/entities/character.h"
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/gameworld.h>
#include <game/server/player.h>
#include <game/server/teams.h>
#include <generated/protocol.h>

#include <engine/shared/config.h>
#include <engine/shared/protocol.h>

#include <base/math.h>
#include <base/vmath.h>

#include <algorithm>
#include <iterator>


CStaffInd::CStaffInd(CCharacter* pCharacter) :
	CEntityOwned(CGameWorld::ENTTYPE_STAFF_IND, EItemVariant::ITEM_STAFF_IND, pCharacter, NUM_IDS, pCharacter->GetPos())
{
	m_Dist = 0.f;
	m_BallFirst = true;
}

void CStaffInd::Tick() {
	if(HasReset() || !Character())
		return;
	
	m_Pos = Character()->GetPos();

	m_aPos[ARMOR] = vec2(m_Pos.x, m_Pos.y - 70.f);

	if(m_BallFirst)
	{
		m_Dist += 0.9f;
		if(m_Dist > 25.f)
			m_BallFirst = false;
	} else {
		m_Dist -= 0.9f;
		if(m_Dist < -25.f)
			m_BallFirst = true;
	}

	m_aPos[BALL] = vec2(m_Pos.x + m_Dist, m_aPos[ARMOR].y);
}

void CStaffInd::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	const CPlayer *pSnapPlayer = GameServer()->m_apPlayers[SnappingClient];

	if(!Character() || !pSnapPlayer || pSnapPlayer->GetCharacter() && !Character()->CanSnapCharacter(SnappingClient))
		return;

	CGameTeams Teams = GameServer()->m_pController->Teams();
	const int Team = Character()->Team();

	if(!TeamMask().test(SnappingClient))
		return;

	// todo: vanish
	// if(pOwnerChr->GetPlayer()->m_Vanish && SnappingClient != pOwnerChr->GetPlayer()->GetCid() && SnappingClient != -1)
	// 	if(!pSnapPlayer->m_Vanish && Server()->GetAuthedState(SnappingClient) < AUTHED_ADMIN)
	// 		return;

	const int SnapVer = Server()->GetClientVersion(SnappingClient);
	const bool SixUp = Server()->IsSixup(SnappingClient);
	const int BallId = m_BallFirst ? m_SnapIds[BALL_FRONT] : m_SnapIds[BALL];

	vec2 Pos = m_aPos[ARMOR] + Character()->GetVelocity();
	vec2 LaserPos = m_aPos[BALL] + Character()->GetVelocity();
	if(g_Config.m_SvSheepExperimentalPrediction && !Player()->IsPaused()) {
		const double Pred = Player()->m_PredLatency;
		const float dist = distance(Character()->m_Pos, Character()->m_PrevPos);
		const vec2 nVel = normalize(Character()->GetVelocity()) * Pred * dist / 2.0f;
		Pos = m_aPos[ARMOR] + nVel;
		LaserPos = m_aPos[BALL] + nVel;
	}
	GameServer()->SnapPickup(CSnapContext(SnapVer, SixUp, SnappingClient), m_SnapIds[ARMOR], Pos, POWERUP_ARMOR, -1, -1, PICKUPFLAG_NO_PREDICT);
	GameServer()->SnapLaserObject(CSnapContext(SnapVer, SixUp, SnappingClient), BallId, LaserPos, LaserPos, Server()->Tick(), Player()->GetCid(), LASERTYPE_GUN, -1, -1, LASERFLAG_NO_PREDICT);
}
