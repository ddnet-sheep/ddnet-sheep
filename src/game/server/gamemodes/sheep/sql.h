/* (c) Antonio Ianzano. See licence.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_SQL_H
#define GAME_SERVER_GAMEMODES_SHEEP_SQL_H

#include <engine/server/databases/connection.h>
#include <engine/server/databases/connection_pool.h>

struct CSqlSuccessResult : ISqlResult
{
	CSqlSuccessResult() :
        m_Success(false)
	{
	}

    bool m_Success;
};

#endif