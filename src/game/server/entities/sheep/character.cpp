#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <engine/shared/config.h>
#include <game/gamecore.h>
#include <game/server/entities/sheep/weapon_drop.h>
#include <game/server/gamecontext.h>
#include <game/server/gamemodes/sheep/sheep.h>

bool CCharacter::CanDropWeapon(EWeaponType Type) const {
	return g_Config.m_SvSheepWeaponDrops && Type != WEAPON_NINJA && Type != WEAPON_GUN;
}

void CCharacter::DropWeapon(EWeaponType Type, vec2 Vel, bool Death) {
	if(!CanDropWeapon(Type))
		return;

	CGameControllerSheep *pController = (CGameControllerSheep *)GameServer()->m_pController;

	CWeaponDrop *pWeaponDrop = new CWeaponDrop(GameWorld(), GetPlayer(), m_Pos, Team(), m_TeleCheckpoint, Vel, 300, Type);
	pController->m_vWeaponDrops.push_back(pWeaponDrop);

	if(!Death) {
		GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, TeamMask());
		GiveWeapon(Type, true);
	}

	// select next droppable weapon, if none only then select the gun
	for(int i = NUM_WEAPON_TYPES - 1; i >= 0; i--) {
		if (i == WEAPON_GUN)
			continue;
		
		if(m_Core.m_aWeapons[i].m_Got) {
			SetActiveWeapon(i);
			return;
		}
	}
	SetActiveWeapon(WEAPON_GUN);
}