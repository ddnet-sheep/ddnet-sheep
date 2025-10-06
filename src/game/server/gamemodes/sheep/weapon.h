/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_WEAPON_H
#define GAME_SERVER_GAMEMODES_SHEEP_WEAPON_H

#include <generated/protocol.h>
#include <string.h>

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
				case WEAPON_GRAVITYGUN: return "Gravitygun";
				case WEAPON_HEARTGUN: return "Heartgun";
				case WEAPON_LIGHTSABER: return "Lightsaber";
				case WEAPON_PORTALGUN: return "Portalgun";
				default: return "Unknown";
			}
		}

		static int GetId(const char* Type) {
			// char LowerType[64] = "";
			// for(size_t i = 0; i < sizeof(Type); i++)
			// 	LowerType[i] = tolower(Type[i]);

			if(!strcmp(Type, "hammer")) return WEAPON_HAMMER;
			if(!strcmp(Type, "gun")) return WEAPON_GUN;
			if(!strcmp(Type, "shotgun")) return WEAPON_SHOTGUN;
			if(!strcmp(Type, "grenade")) return WEAPON_GRENADE;
			if(!strcmp(Type, "laser")) return WEAPON_LASER;
			if(!strcmp(Type, "ninja")) return WEAPON_NINJA;
			if(!strcmp(Type, "gravity")) return WEAPON_GRAVITYGUN;
			if(!strcmp(Type, "heart")) return WEAPON_HEARTGUN;
			if(!strcmp(Type, "lightsaber")) return WEAPON_LIGHTSABER;
			if(!strcmp(Type, "portal")) return WEAPON_PORTALGUN;
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
