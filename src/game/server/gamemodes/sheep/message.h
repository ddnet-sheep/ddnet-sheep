/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_MESSAGE_H
#define GAME_SERVER_GAMEMODES_SHEEP_MESSAGE_H

struct CFakePlayerMessage {
	int m_Team = 0; // 0 = all, 1 = team, 2 = system
	char m_aName[64];
	char m_aMessage[256] = "";
	int m_SenderId = -1;
	int m_ReceiverId = -1;
};

#endif
