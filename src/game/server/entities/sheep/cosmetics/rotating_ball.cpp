#include "game/server/entities/character.h"
#include <game/server/entity.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/gameworld.h>
#include <game/server/player.h>
#include <game/server/teams.h>

#include <engine/shared/config.h>

#include <base/vmath.h>

#include "rotating_ball.h"

CRotatingBall::CRotatingBall(CCharacter* pCharacter) :
	CEntityOwned(CGameWorld::ENTTYPE_ROTATING_BALL, EItemVariant::ITEM_ROTATING_BALL, pCharacter, 1, pCharacter->GetPos())
{
	m_IsRotating = true;

	m_RotateDelay = Server()->TickSpeed() + 10;
	m_LaserDirAngle = 0;
	m_LaserInputDir = 0;

	m_TableDirV[0][0] = 5;
	m_TableDirV[0][1] = 12;
	m_TableDirV[1][0] = -12;
	m_TableDirV[1][1] = -5;
}

void CRotatingBall::Tick() {
	if(HasReset() || !Character())
		return;

	m_Pos = Character()->GetPos();
	
	m_RotateDelay--;
	if(m_RotateDelay <= 0) {
		m_IsRotating ^= true;

		int DirSelect = rand() % 2;
		m_LaserInputDir = rand() % (m_TableDirV[DirSelect][1] - m_TableDirV[DirSelect][0] + 1) + m_TableDirV[DirSelect][0];
		m_RotateDelay = m_IsRotating ? Server()->TickSpeed() + (rand() % (7 - 3 + 1) + 3) : Server()->TickSpeed() + (rand() % (20 - 5 + 1) + 5);
	}

	if(m_IsRotating)
		m_LaserDirAngle += m_LaserInputDir;

	m_LaserPos.x = Character()->GetPos().x + 65 * sin(m_LaserDirAngle * pi / 180.0f);
	m_LaserPos.y = Character()->GetPos().y + 65 * cos(m_LaserDirAngle * pi / 180.0f);

	m_ProjPos.x = m_LaserPos.x + 22 * sin(Server()->Tick() * 13 * pi / 180.0f);
	m_ProjPos.y = m_LaserPos.y + 22 * cos(Server()->Tick() * 13 * pi / 180.0f);
}

void CRotatingBall::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	const CPlayer *pSnapPlayer = GameServer()->m_apPlayers[SnappingClient];

	if(!Character() || Character()->IsPaused() || !pSnapPlayer || pSnapPlayer->GetCharacter() && !Character()->CanSnapCharacter(SnappingClient))
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

	vec2 Pos = m_ProjPos + Character()->GetVelocity();
	vec2 LaserPos = m_LaserPos + Character()->GetVelocity();
	if(g_Config.m_SvSheepExperimentalPrediction && !Player()->IsPaused()) {
		const double Pred = Player()->m_PredLatency;
		const float dist = distance(Character()->m_Pos, Character()->m_PrevPos);
		const vec2 nVel = normalize(Character()->GetVelocity()) * Pred * dist / 2.0f;
		Pos = m_ProjPos + nVel;
		LaserPos = m_LaserPos + nVel;
	}

	// if(g_Config.m_SvCorruptPickupPet && pSnapPlayer->m_Cosmetics.m_PickupPet)
		// Owner = -1; // Sets the pickuppet to the laser sprite for some reason

	GameServer()->SnapLaserObject(CSnapContext(SnapVer, SixUp, SnappingClient), GetId(), LaserPos, LaserPos, Server()->Tick(), Player()->GetCid(), LASERTYPE_GUN, -1, -1, LASERFLAG_NO_PREDICT);

	CNetObj_DDNetProjectile *pProj = Server()->SnapNewItem<CNetObj_DDNetProjectile>(m_SnapIds[0]);
	if(!pProj)
		return;

	pProj->m_X = round_to_int(Pos.x * 100.0f);
	pProj->m_Y = round_to_int(Pos.y * 100.0f);
	pProj->m_Type = WEAPON_HAMMER;
	pProj->m_Owner = Player()->GetCid();
	pProj->m_StartTick = 0;
	pProj->m_VelX = 0;
	pProj->m_VelY = 0;
}
