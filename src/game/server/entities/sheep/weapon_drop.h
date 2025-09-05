/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_ENTITIES_SHEEP_WEAPON_DROP_H
#define GAME_SERVER_ENTITIES_SHEEP_WEAPON_DROP_H

#include <base/vmath.h>
#include <game/mapitems.h>
#include <game/server/entity.h>
#include <game/server/gameworld.h>

#include <game/server/player.h>

#include <game/server/gamemodes/sheep/weapon.h>

class CWeaponDrop : public CEntity
{
    CPlayer* m_Dropper;

	uint m_Lifetime; // In ticks
	uint m_PickupDelay; // In ticks
	
    EWeaponType m_Type;

	std::vector<int> m_vIds = std::vector<int>();
	int m_Team;

	vec2 m_GroundElasticity;

    int m_TeleCheckpoint;
	int m_TileIndex;
	int m_TileFIndex;
	int m_TuneZone;
	int m_MoveRestrictions;

	vec2 m_PrevPos;
	vec2 m_Vel;

	bool m_InsideFreeze;

	static bool IsSwitchActiveCb(int Number, void *pUser);
	void HandleTiles(int Index);

	bool CollectItem();
public:
	CWeaponDrop(CGameWorld *pGameWorld, CPlayer* Dropper, vec2 Pos, int Team, int TeleCheckpoint, vec2 Vel, int Lifetime, EWeaponType Type);

	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;
	virtual void Reset() override { Reset(false); }
	
	void Reset(bool PickedUp);
};

#endif
