/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_ITEM_H
#define GAME_SERVER_GAMEMODES_SHEEP_ITEM_H

#include "sql.h"
#include <map>

// todo: make ECosmetics out of this, and move sheep_account_items to sheep_cosmetics, make class for each cosmetic
enum class EItemVariant
{
    ITEM_NONE,
    ITEM_HEART_HAT,
    ITEM_LOVELY,
    ITEM_DOT_TRAIL,
    ITEM_STAFF_IND,
    ITEM_ROTATING_BALL,
    ITEM_EPIC_CIRCLE,
    ITEM_SPARKLE,
    ITEM_BLOODY,
    ITEM_BLOODY_STRONG,
    ITEM_RAINBOW_BODY,
    ITEM_RAINBOW_FEET,
    ITEM_DEATH_EFFECT
};

enum class EItemType
{
    TYPE_PLACEHOLDER,
    TYPE_COSMETIC
};

enum class EItemState
{
    ITEM_STATE_NORMAL,
    ITEM_STATE_ACTIVE
};
struct CItem {
    EItemType m_Type;
    EItemVariant m_Variant;
    char m_Name[32];
    char m_Description[255];
};

struct CItemsResult : ISheepSqlResult
{
    std::unordered_map<EItemVariant, CItem> m_Items;
};

struct CAccountItem {
    EItemVariant m_Variant;
    uint64_t m_Amount;
    int m_State;
};

struct CAccountItemResult : ISheepSqlResult
{
    std::unordered_map<EItemVariant, CAccountItem> m_AccountItem;
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
