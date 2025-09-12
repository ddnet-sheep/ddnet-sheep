/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_WEAPON_H
#define GAME_SERVER_GAMEMODES_SHEEP_WEAPON_H

#include <generated/protocol.h>

class CWeapon {
	public:
		static const char* GetName(int Type) {
			switch(Type) {
				case WEAPON_HAMMER: return "Hammer";
				case WEAPON_GUN: return "Gun";
				case WEAPON_SHOTGUN: return "Shotgun";
				case WEAPON_GRENADE: return "Grenade";
				case WEAPON_LASER: return "Laser";
				case WEAPON_NINJA: return "Ninja";
				case WEAPON_GRAVITYGUN: return "Gravity gun";
				case WEAPON_HEARTGUN: return "Heart gun";
				case WEAPON_LIGHTSABER: return "Lightsaber";
				case WEAPON_PORTALGUN: return "Portal gun";
				default: return "Unknown";
			}
		}

		static int GetBaseWeapon(int Type) {
			switch(Type) {
				case WEAPON_GRAVITYGUN:
					return WEAPON_NINJA;
				case WEAPON_HEARTGUN:
				case WEAPON_LIGHTSABER:
					return WEAPON_GUN;
				case WEAPON_PORTALGUN:
					return WEAPON_LASER;
				default:
					return Type;
			}
		}
};

#endif
