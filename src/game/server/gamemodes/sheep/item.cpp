/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#include "sheep.h"
#include "item.h"

#include <game/server/player.h>

#include <base/log.h>

void CGameControllerSheep::LoadItems() {
    m_ItemsResult = std::make_shared<CItemsResult>();
    auto Tmp = std::make_unique<ISqlData>(m_ItemsResult);
	m_pPool->Execute(CGameControllerSheep::ExecuteLoadItems, std::move(Tmp), "load items");
}

bool CGameControllerSheep::ExecuteLoadItems(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
    auto *pResult = dynamic_cast<CItemsResult *>(pGameData->m_pResult.get());

	if(!pSqlServer->PrepareStatement("SELECT id, name, description FROM sheep_items", pError, ErrorSize))
	    return false;

    bool End;
    do {
        pSqlServer->Step(&End, pError, ErrorSize);
        
        if(End)
            break;

        CItem item;
        item.m_Type = static_cast<EItemType>(pSqlServer->GetInt(1));
        pSqlServer->GetString(2, item.m_Name, sizeof(item.m_Name));
        pSqlServer->GetString(3, item.m_Description, sizeof(item.m_Description));

        pResult->m_Items[item.m_Type] = item;
    } while(!End);

    for (const auto& [type, item] : pResult->m_Items) {
        char aBuf[512];
        str_format(aBuf, sizeof(aBuf), "Loaded item %d: %s - %s", static_cast<int>(type), item.m_Name, item.m_Description);
        log_error("sheep", aBuf);
    }

	return true;
}

void CGameControllerSheep::LoadAccountItem(CPlayer* pPlayer) {
    pPlayer->m_AccountItemResult = std::make_shared<CAccountItemResult>();
    auto Tmp = std::make_unique<CSqlAccountItemRequest>(pPlayer->m_AccountItemResult);
    Tmp->m_AccountId = pPlayer->m_AccountLoginResult->m_AccountId;
	m_pPool->Execute(CGameControllerSheep::ExecuteLoadAccountItem, std::move(Tmp), "load account items");
}

bool CGameControllerSheep::ExecuteLoadAccountItem(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
    auto *pResult = dynamic_cast<CAccountItemResult *>(pGameData->m_pResult.get());

	if(!pSqlServer->PrepareStatement("SELECT item_id, amount FROM sheep_account_item WHERE account_id = ?", pError, ErrorSize))
	    return false;

    const auto *pData = dynamic_cast<const CSqlAccountItemRequest *>(pGameData);
    pSqlServer->BindInt(1, pData->m_AccountId);

    bool End;
    do {
        pSqlServer->Step(&End, pError, ErrorSize);
        
        if(End)
            break;

        CAccountItem item;
        item.m_Amount = pSqlServer->GetInt64(2);
        item.m_Type = static_cast<EItemType>(pSqlServer->GetInt(1));

        pResult->m_AccountItem[item.m_Type] = item;
    } while(!End);

    // for (const auto& [type, item] : pResult->m_AccountItem) {
    //     char aBuf[512];
    //     str_format(aBuf, sizeof(aBuf), "Loaded account item %d: %d", static_cast<int>(type), item.m_Amount);
    //     log_error("sheep", aBuf);
    // }

	return true;
}
