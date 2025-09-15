/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_WEAPON_H
#define GAME_SERVER_GAMEMODES_SHEEP_WEAPON_H

#include <generated/protocol.h>
#include <string.h>

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

		static const int GetId(const char* Type) {
			if(!strcmp(Type, "Hammer")) return WEAPON_HAMMER;
			if(!strcmp(Type, "Gun")) return WEAPON_GUN;
			if(!strcmp(Type, "Shotgun")) return WEAPON_SHOTGUN;
			if(!strcmp(Type, "Grenade")) return WEAPON_GRENADE;
			if(!strcmp(Type, "Laser")) return WEAPON_LASER;
			if(!strcmp(Type, "Ninja")) return WEAPON_NINJA;
			if(!strcmp(Type, "Gravitygun")) return WEAPON_GRAVITYGUN;
			if(!strcmp(Type, "Heartgun")) return WEAPON_HEARTGUN;
			if(!strcmp(Type, "Lightsaber")) return WEAPON_LIGHTSABER;
			if(!strcmp(Type, "Portalgun")) return WEAPON_PORTALGUN;
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
