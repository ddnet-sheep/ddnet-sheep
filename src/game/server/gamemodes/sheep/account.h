/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_ACCOUNT_H
#define GAME_SERVER_GAMEMODES_SHEEP_ACCOUNT_H

#include <engine/server/databases/connection.h>
#include <engine/server/databases/connection_pool.h>

#include <map>
#include <base/hash.h>
#include <base/hash_ctxt.h>
#include "item.h"
#include "sql.h"

struct CAccountLoginResult : ISheepSqlResult
{
	CAccountLoginResult() :
		m_BanExpiration(0),
        m_Level(0),
        m_Exp(0),
        m_Vip(0),
        m_VipExpiration(0),
        m_Staff(0),
        m_EmailVerified(false),
        m_Invisible(false),
        m_Vanish(false)
	{
	}

    uint64_t m_AccountId;

    char m_Username[64];
    char m_PasswordHash[65];

	uint64_t m_BanExpiration;

    uint64_t m_Level;
    uint64_t m_Exp;
    
    uint8_t m_Vip; // vip level
    uint64_t m_VipExpiration; // vip expiration time

    uint8_t m_Staff; // admin level

    std::string m_Email;
    bool m_EmailVerified;

    bool m_Invisible; // invisible physically
    bool m_Vanish; // silent join and not visible in the player list

    char m_Title[32];

    bool m_Processed = false;

    std::unordered_map<EItemType, uint64_t> m_Items;
};

struct CSqlAccountCredentialsRequest : ISqlData
{
	CSqlAccountCredentialsRequest(std::shared_ptr<ISqlResult> pResult) :
		ISqlData(std::move(pResult))
	{
	}

	char m_Username[64];
	char m_Password[64];
};

#endif
