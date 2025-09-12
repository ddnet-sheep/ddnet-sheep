/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#include "weapon_drop.h"
#include <game/server/entities/character.h>
#include <game/server/player.h>

#include <game/server/gamecontext.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/teams.h>

#include <base/math.h>
#include <base/vmath.h>
#include <game/collision.h>
#include <game/gamecore.h>
#include <game/mapitems.h>
#include <game/server/entities/pickup.h>
#include <game/server/entity.h>
#include <game/server/gameworld.h>
#include <vector>
#include <game/teamscore.h>

#include <engine/shared/config.h>

#include <game/server/gamemodes/sheep/weapon.h>

#include <base/log.h>

static int GetNeededIds(int Type) {
	switch(Type) {
		case WEAPON_GRAVITYGUN:
		case WEAPON_HEARTGUN:
		case WEAPON_LIGHTSABER:
			return 1;
		case WEAPON_PORTALGUN:
			return 2;
		default:
			return 0;
	}
}

CWeaponDrop::CWeaponDrop(CGameWorld *pGameWorld, CPlayer* Dropper, vec2 Pos, int Team, int TeleCheckpoint, vec2 Vel, int Lifetime, int Type) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_WEAPON_DROP, Pos, 28)
{
    m_Dropper = Dropper;
    m_Team = Team;

    m_Type = Type;
	for(int i = 0; i < GetNeededIds(Type); i++)
		m_vIds.push_back(Server()->SnapNewId());

	m_PrevPos = m_Pos;
	m_Vel = Vel;
	m_GroundElasticity = vec2(0.5f, 0.5f);

	m_PickupDelay = Server()->TickSpeed();
	m_Lifetime = Lifetime * Server()->TickSpeed();
	
	m_TuneZone = -1;
	m_TeleCheckpoint = TeleCheckpoint;

	GameWorld()->InsertEntity(this);
}

void CWeaponDrop::Reset(bool PickedUp)
{
	for(int i = 0; i < m_vIds.size(); i++)
		Server()->SnapFreeId(m_vIds[i]);
	Server()->SnapFreeId(GetId());
	
	CClientMask TeamMask = CClientMask().set();
	if(!PickedUp)
		GameServer()->CreateDeath(m_Pos, m_Dropper->GetCid(), TeamMask);

	GameWorld()->RemoveEntity(this);
}

bool CWeaponDrop::IsSwitchActiveCb(int Number, void *pUser)
{
	CWeaponDrop *pThis = (CWeaponDrop *)pUser;
	auto &aSwitchers = pThis->Switchers();
	return !aSwitchers.empty() && pThis->m_Team != TEAM_SUPER && aSwitchers[Number].m_aStatus[pThis->m_Team];
}

void CWeaponDrop::Tick() {
	if(GameLayerClipped(m_Pos) || m_Lifetime <= 0) {
		Reset();
		return;
	}
	m_Lifetime--;

	if(CollectItem())
		return;

	int CurrentIndex = GameServer()->Collision()->GetMapIndex(m_Pos);
	m_TuneZone = GameServer()->Collision()->IsTune(CurrentIndex);

	m_MoveRestrictions = Collision()->GetMoveRestrictions(IsSwitchActiveCb, this, m_Pos, 18.0f, CurrentIndex);

	if(!m_TuneZone)
		m_Vel.y += GameServer()->Tuning()->m_Gravity;
	else
		m_Vel.y += GameServer()->TuningList()[m_TuneZone].m_Gravity;

	if(Collision()->IsSpeedup(CurrentIndex)) {
		vec2 Direction, TempVel = m_Vel;
		int Force, Type, MaxSpeed = 0;
		Collision()->GetSpeedup(CurrentIndex, &Direction, &Force, &MaxSpeed, &Type);

		if(Type == TILE_SPEED_BOOST_OLD) {
			float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
			if(Force == 255 && MaxSpeed) {
				m_Vel = Direction * (MaxSpeed / 5);
			} else {
				if(MaxSpeed > 0) {
					if(MaxSpeed < 5)
						MaxSpeed = 5;

					if(Direction.x > 0.0000001f)
						SpeederAngle = -std::atan(Direction.y / Direction.x);
					else if(Direction.x < 0.0000001f)
						SpeederAngle = std::atan(Direction.y / Direction.x) + 2.0f * std::asin(1.0f);
					else if(Direction.y > 0.0000001f)
						SpeederAngle = std::asin(1.0f);
					else
						SpeederAngle = std::asin(-1.0f);

					if(SpeederAngle < 0)
						SpeederAngle = 4.0f * std::asin(1.0f) + SpeederAngle;

					if(TempVel.x > 0.0000001f)
						TeeAngle = -std::atan(TempVel.y / TempVel.x);
					else if(TempVel.x < 0.0000001f)
						TeeAngle = std::atan(TempVel.y / TempVel.x) + 2.0f * std::asin(1.0f);
					else if(TempVel.y > 0.0000001f)
						TeeAngle = std::asin(1.0f);
					else
						TeeAngle = std::asin(-1.0f);

					if(TeeAngle < 0)
						TeeAngle = 4.0f * std::asin(1.0f) + TeeAngle;

					TeeSpeed = std::sqrt(std::pow(TempVel.x, 2) + std::pow(TempVel.y, 2));

					DiffAngle = SpeederAngle - TeeAngle;
					SpeedLeft = MaxSpeed / 5.0f - std::cos(DiffAngle) * TeeSpeed;
					if(absolute((int)SpeedLeft) > Force && SpeedLeft > 0.0000001f)
						TempVel += Direction * Force;
					else if(absolute((int)SpeedLeft) > Force)
						TempVel += Direction * -Force;
					else
						TempVel += Direction * SpeedLeft;
				} else
					TempVel += Direction * Force;

				m_Vel = ClampVel(m_MoveRestrictions, TempVel);
			}
		} else if(Type == TILE_SPEED_BOOST) {
			constexpr float MaxSpeedScale = 5.0f;
			if(MaxSpeed == 0) {
				float MaxRampSpeed = GetTuning(m_TuneZone)->m_VelrampRange / (50 * log(maximum((float)GetTuning(m_TuneZone)->m_VelrampCurvature, 1.01f)));
				MaxSpeed = maximum(MaxRampSpeed, GetTuning(m_TuneZone)->m_VelrampStart / 50) * MaxSpeedScale;
			}

			// (signed) length of projection
			float CurrentDirectionalSpeed = dot(Direction, m_Vel);
			float TempMaxSpeed = MaxSpeed / MaxSpeedScale;
			if(CurrentDirectionalSpeed + Force > TempMaxSpeed)
				TempVel += Direction * (TempMaxSpeed - CurrentDirectionalSpeed);
			else
				TempVel += Direction * Force;

			m_Vel = ClampVel(m_MoveRestrictions, TempVel);
		}
	}

	// tiles
	std::vector<int> vIndices = Collision()->GetMapIndices(m_PrevPos, m_Pos);
	if(!vIndices.empty()) {
		for(int &Index : vIndices)
			HandleTiles(Index);
	} else
		HandleTiles(CurrentIndex);

	bool Grounded = false;
	Collision()->MoveBox(&m_Pos, &m_Vel, CCharacterCore::PhysicalSizeVec2(), m_GroundElasticity, &Grounded);
	if(Grounded)
		m_Vel.x *= 0.88f;
	m_Vel.x *= 0.98f;

	if(m_InsideFreeze) {
		m_Vel.y -= 0.05f; // slowly float up
		if(!m_TuneZone)
			m_Vel.y -= GameServer()->Tuning()->m_Gravity;
		else
			m_Vel.y -= GameServer()->TuningList()[m_TuneZone].m_Gravity;
		m_InsideFreeze = false; // Reset for the next tick
	}

	m_PrevPos = m_Pos;
}

bool CWeaponDrop::CollectItem() {
	if(m_PickupDelay > 0) {
		m_PickupDelay--;
		return false;
	}

	float Radius = 32.0f;
	CCharacter *pCharacter = GameServer()->m_World.ClosestCharacter(m_Pos, Radius, this);
	if(!pCharacter || m_Team != pCharacter->Team() || pCharacter->Core()->m_aWeapons[m_Type].m_Got)
		return false;

	pCharacter->GiveWeapon(m_Type);
	pCharacter->SetActiveWeapon(m_Type);
	CClientMask TeamMask = CClientMask().set();
	GameServer()->CreateSound(pCharacter->m_Pos, SOUND_PICKUP_HEALTH, TeamMask);

	if(pCharacter->GetPlayer())
		GameServer()->SendWeaponPickup(pCharacter->GetPlayer()->GetCid(), m_Type);

	Reset(true);
	return true;
}

void CWeaponDrop::HandleTiles(int Index)
{
	int MapIndex = Index;
	// int PureMapIndex = Collision()->GetPureMapIndex(m_Pos);
	m_TileIndex = Collision()->GetTileIndex(MapIndex);
	m_TileFIndex = Collision()->GetFrontTileIndex(MapIndex);

	int TeleCheckpoint = Collision()->IsTeleCheckpoint(MapIndex);
	if(TeleCheckpoint)
		m_TeleCheckpoint = TeleCheckpoint;

	m_Vel = ClampVel(m_MoveRestrictions, m_Vel);
	// if(g_Config.m_SvDropsInFreezeFloat && (m_TileIndex == TILE_FREEZE || m_TileFIndex == TILE_FREEZE))
	// {
	// 	if(g_Config.m_SvDropsInFreezeFloat)
	// 		m_InsideFreeze = true;
	// }

	// teleporters
	int z = Collision()->IsTeleport(MapIndex);
	if(!g_Config.m_SvOldTeleportHook && !g_Config.m_SvOldTeleportWeapons && z && !Collision()->TeleOuts(z - 1).empty()) {
		int TeleOut = GameWorld()->m_Core.RandomOr0(Collision()->TeleOuts(z - 1).size());
		m_Pos = Collision()->TeleOuts(z - 1)[TeleOut];
		return;
	}
	int evilz = Collision()->IsEvilTeleport(MapIndex);
	if(evilz && !Collision()->TeleOuts(evilz - 1).empty()) {
		int TeleOut = GameWorld()->m_Core.RandomOr0(Collision()->TeleOuts(evilz - 1).size());
		m_Pos = Collision()->TeleOuts(evilz - 1)[TeleOut];
		if(!g_Config.m_SvOldTeleportHook && !g_Config.m_SvOldTeleportWeapons)
			m_Vel = vec2(0, 0);
		return;
	}
	if(Collision()->IsCheckEvilTeleport(MapIndex)) {
		// first check if there is a TeleCheckOut for the current recorded checkpoint, if not check previous checkpoints
		for(int k = m_TeleCheckpoint - 1; k >= 0; k--) {
			if(!Collision()->TeleCheckOuts(k).empty()) {
				int TeleOut = GameWorld()->m_Core.RandomOr0(Collision()->TeleCheckOuts(k).size());
				m_Pos = Collision()->TeleCheckOuts(k)[TeleOut];
				m_Vel = vec2(0, 0);

				return;
			}
		}
		// if no checkpointout have been found (or if there no recorded checkpoint), teleport to start
		vec2 SpawnPos;
		if(GameServer()->m_pController->CanSpawn(0, &SpawnPos, m_Team)) {
			m_Pos = SpawnPos;
			m_Vel = vec2(0, 0);
		}
		return;
	}
	if(Collision()->IsCheckTeleport(MapIndex)) {
		// first check if there is a TeleCheckOut for the current recorded checkpoint, if not check previous checkpoints
		for(int k = m_TeleCheckpoint - 1; k >= 0; k--) {
			if(!Collision()->TeleCheckOuts(k).empty()) {
				int TeleOut = GameWorld()->m_Core.RandomOr0(Collision()->TeleCheckOuts(k).size());
				m_Pos = Collision()->TeleCheckOuts(k)[TeleOut];
				return;
			}
		}
		// if no checkpointout have been found (or if there no recorded checkpoint), teleport to start
		vec2 SpawnPos;
		if(GameServer()->m_pController->CanSpawn(0, &SpawnPos, m_Team))
			m_Pos = SpawnPos;
		return;
	}
}

void CWeaponDrop::Snap(int ClientId) {
	if(NetworkClipped(ClientId))
		return;

	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];

	// TODO: mask
	// CGameTeams Teams = GameServer()->m_pController->Teams();
	// if(!Teams.SetMask(ClientId, m_Team))
	// 	return;

	// Make the pickup blink when about to disappear
	if(m_Lifetime < Server()->TickSpeed() * 10 && (Server()->Tick() / (Server()->TickSpeed() / 4)) % 2 == 0)
		return;

	CSnapContext SnapContext = CSnapContext(pPlayer->GetClientVersion(), Server()->IsSixup(ClientId), ClientId);

	int WeaponId = CWeapon::GetBaseWeapon(m_Type);
	GameServer()->SnapPickup(SnapContext, GetId(), m_Pos, POWERUP_WEAPON, WeaponId, -1, PICKUPFLAG_NO_PREDICT);

	vec2 Offset = vec2(0.0f, -32.0f);
	switch (m_Type) {
		case WEAPON_HEARTGUN:
			GameServer()->SnapPickup(SnapContext, m_vIds[0], m_Pos + Offset, POWERUP_HEALTH, 0, -1, PICKUPFLAG_NO_PREDICT);
			break;
		case WEAPON_LIGHTSABER:
			GameServer()->SnapLaserObject(SnapContext, m_vIds[0], m_Pos + Offset, m_Pos + Offset, Server()->Tick(), -1, LASERTYPE_GUN);
			break;
		case WEAPON_PORTALGUN:
			GameServer()->SnapLaserObject(SnapContext, m_vIds[0], m_Pos + Offset, m_Pos + Offset, Server()->Tick(), -1, LASERTYPE_GUN);
			vec2 Spin = vec2(cos(Server()->Tick() / 5.0f), sin(Server()->Tick() / 5.0f)) * 17.0f;
			Spin += Offset;

			CNetObj_Projectile *pProj = Server()->SnapNewItem<CNetObj_Projectile>(m_vIds[1]);
			if(!pProj)
				break;

			pProj->m_X = (int)(m_Pos.x + Spin.x);
			pProj->m_Y = (int)(m_Pos.y + Spin.y);
			pProj->m_VelX = 0;
			pProj->m_VelY = 0;
			pProj->m_StartTick = 0;
			pProj->m_Type = WEAPON_HAMMER;
			break;
	}
}
