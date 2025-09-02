/* (c) Antonio Ianzano. See licence.txt and the readme.txt in the root of the distribution for more information. */
#include <engine/server/databases/connection.h>
#include <engine/server/databases/connection_pool.h>

#include <map>
#include "item.h"

struct CAccountLoginResult : ISqlResult
{
	CAccountLoginResult() :
		m_BanExpiration(0),
        m_Level(0),
        m_Exp(0),
        m_Vip(0),
        m_VipExpiration(0),
        m_Staff(0),
        m_EmailVerified(false)
	{
	}

	uint64_t m_BanExpiration;

    uint64_t m_Level;
    uint64_t m_Exp;
    
    uint8_t m_Vip; // vip level
    uint64_t m_VipExpiration; // vip expiration time

    uint8_t m_Staff; // admin level

    std::string m_Email;
    bool m_EmailVerified;

    bool m_Processed = false;

    std::map<EItem, uint64_t> m_Items;
};

struct CSqlAccountLoginRequest : ISqlData
{
	CSqlAccountLoginRequest(std::shared_ptr<CAccountLoginResult> pResult) :
		ISqlData(std::move(pResult))
	{
	}

	char m_Username[64];
	char m_Password[64];
};