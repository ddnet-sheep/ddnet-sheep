/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_ITEM_H
#define GAME_SERVER_GAMEMODES_SHEEP_ITEM_H

#include "sql.h"

enum class EItemType
{
    ITEM_PLACEHOLDER,
};

struct CItem {
    EItemType m_Type;
    char m_Name[32];
    char m_Description[255];
};

struct CItemsResult : ISheepSqlResult
{
    std::map<EItemType, CItem> m_Items;
};

struct CAccountItem {
    EItemType m_Type;
    uint64_t m_Amount;
};

struct CAccountItemResult : ISheepSqlResult
{
    std::map<EItemType, CAccountItem> m_AccountItem;
};

struct CSqlAccountItemRequest : ISqlData
{
    uint64_t m_AccountId;

    CSqlAccountItemRequest(std::shared_ptr<ISqlResult> pResult) :
        ISqlData(std::move(pResult))
    {
    }
};

#endif