/* (c) Antonio Ianzano. See licence.txt and the readme.txt in the root of the distribution for more information. */
#include "sheep.h"
#include <game/server/player.h>

void CGameControllerSheep::ConLogin(IConsole::IResult *pResult, void *pUserData) {
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if (pPlayer->m_AccountLoginResult != nullptr) {
		pSelf->SendChatTarget(pResult->m_ClientId, "You are already logged in.");
		return;
	}

	pPlayer->m_AccountLoginResult = std::make_shared<CAccountLoginResult>();
	
	auto Tmp = std::make_unique<CSqlAccountLoginRequest>(pPlayer->m_AccountLoginResult);
	str_copy(Tmp->m_Username, pResult->GetString(0), sizeof(Tmp->m_Username));
	str_copy(Tmp->m_Password, pResult->GetString(1), sizeof(Tmp->m_Password));

	CGameControllerSheep *pController = (CGameControllerSheep *)pSelf->m_pController;
	pController->m_pPool->Execute(CGameControllerSheep::ExecuteLogin, std::move(Tmp), "account login");
}

void CGameControllerSheep::ConLogout(IConsole::IResult *pResult, void *pUserData) {
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;
	
	if (pPlayer->m_AccountLoginResult == nullptr) {
		pSelf->SendChatTarget(pResult->m_ClientId, "You are not logged in.");
		return;
	}
		
	CGameControllerSheep *pController = (CGameControllerSheep *)pSelf->m_pController;

	pPlayer->m_AccountLoginResult = nullptr;
	pPlayer->KillCharacter();
	pSelf->SendChatTarget(pPlayer->GetCid(), "You have been logged out.");
}

bool CGameControllerSheep::ExecuteLogin(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	const auto *pData = dynamic_cast<const CSqlAccountLoginRequest *>(pGameData);
	auto *pResult = dynamic_cast<CAccountLoginResult *>(pGameData->m_pResult.get());

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), 
		"SELECT "
		"ban_expiration, level, exp, vip, vip_expiration, staff_level, email, email_verified"
		" FROM sheep_accounts WHERE name=? AND password=?"
	);
	if(!pSqlServer->PrepareStatement(aBuf, pError, ErrorSize)) {
		return false;
	}
	pSqlServer->BindString(1, pData->m_Username);
	pSqlServer->BindString(2, pData->m_Password);

	bool End;
	if(!pSqlServer->Step(&End, pError, ErrorSize)) {
		return false;
	}
    if(End) {
        return false;
    }
	
    pResult->m_BanExpiration = pSqlServer->GetInt(1);
    pResult->m_Level = pSqlServer->GetInt(2);
    pResult->m_Exp = pSqlServer->GetInt(3);
    pResult->m_Vip = pSqlServer->GetInt(4);
    pResult->m_VipExpiration = pSqlServer->GetInt(5);
    pResult->m_Staff = pSqlServer->GetInt(6);

    if (!pSqlServer->IsNull(7)) {
        char mBuf[255];
        pSqlServer->GetString(7, mBuf, sizeof(mBuf));
        pResult->m_Email = mBuf;
    }
    
    pResult->m_EmailVerified = pSqlServer->GetInt(8) != 0;

    // m_Items

    return true;
}