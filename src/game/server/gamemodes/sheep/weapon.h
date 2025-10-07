/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_WEAPON_H
#define GAME_SERVER_GAMEMODES_SHEEP_WEAPON_H

#include <generated/protocol.h>
#include <string.h>

#include <base/system.h>
#include <base/log.h>

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
				case WEAPON_GRAVITYGUN: return "Gravity";
				case WEAPON_HEARTGUN: return "Heart";
				case WEAPON_LIGHTSABER: return "Lightsaber";
				case WEAPON_PORTALGUN: return "Portal";
				default: return "Unknown";
			}
		}

		static int GetId(const char* Type) {
			if(!str_comp_nocase(Type, "hammer")) return WEAPON_HAMMER;
			if(!str_comp_nocase(Type, "gun")) return WEAPON_GUN;
			if(!str_comp_nocase(Type, "shotgun")) return WEAPON_SHOTGUN;
			if(!str_comp_nocase(Type, "grenade")) return WEAPON_GRENADE;
			if(!str_comp_nocase(Type, "laser")) return WEAPON_LASER;
			if(!str_comp_nocase(Type, "ninja")) return WEAPON_NINJA;
			if(!str_comp_nocase(Type, "gravity")) return WEAPON_GRAVITYGUN;
			if(!str_comp_nocase(Type, "heart")) return WEAPON_HEARTGUN;
			if(!str_comp_nocase(Type, "lightsaber")) return WEAPON_LIGHTSABER;
			if(!str_comp_nocase(Type, "portal")) return WEAPON_PORTALGUN;
			return -2;
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
