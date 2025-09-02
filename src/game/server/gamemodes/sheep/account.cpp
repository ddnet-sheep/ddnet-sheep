/* (c) Antonio Ianzano. See licence.txt and the readme.txt in the root of the distribution for more information. */
#include "sheep.h"
#include "sql.h"
#include <game/server/player.h>

SHA256_DIGEST HashPassword(const char *pPassword)
{
	SHA256_CTX Sha256Ctx;
	sha256_init(&Sha256Ctx);
	sha256_update(&Sha256Ctx, pPassword, str_length(pPassword));
	return sha256_finish(&Sha256Ctx);
}

bool VerifyPassword(CPlayer *pPlayer, const char *pPassword)
{
	return sha256_comp(pPlayer->m_AccountLoginResult->m_Password, HashPassword(pPassword)) != 0;
}

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
	
	auto Tmp = std::make_unique<CSqlAccountCredentialsRequest>(pPlayer->m_AccountLoginResult);
	str_copy(Tmp->m_Username, pResult->GetString(0), sizeof(Tmp->m_Username));
	str_copy(Tmp->m_Password, pResult->GetString(1), sizeof(Tmp->m_Password));

	CGameControllerSheep *pController = (CGameControllerSheep *)pSelf->m_pController;
	pController->m_pPool->Execute(CGameControllerSheep::ExecuteLogin, std::move(Tmp), "account login");
}

bool AccountQuery(char* aBuf, IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	if(!pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
		return false;

	const auto *pData = dynamic_cast<const CSqlAccountCredentialsRequest *>(pGameData);
	pSqlServer->BindString(1, pData->m_Username);
	pSqlServer->BindString(2, pData->m_Password);

	bool End;
	if(!pSqlServer->Step(&End, pError, ErrorSize) || End)
		return false;
	
	auto *pResult = dynamic_cast<CAccountLoginResult *>(pGameData->m_pResult.get());
	str_copy(pResult->m_Username, pData->m_Username);
	pResult->m_Password = HashPassword(pData->m_Password);
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

bool CGameControllerSheep::ExecuteLogin(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {	
	return AccountQuery(
		"SELECT ban_expiration, level, exp, vip, vip_expiration, staff_level, email, email_verified "
		"FROM sheep_accounts WHERE name=? AND password=?", 
		pSqlServer, pGameData, pError, ErrorSize
	);
}

void CGameControllerSheep::ConRegister(IConsole::IResult *pResult, void *pUserData) {
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
	
	auto Tmp = std::make_unique<CSqlAccountCredentialsRequest>(pPlayer->m_AccountLoginResult);
	str_copy(Tmp->m_Username, pResult->GetString(0), sizeof(Tmp->m_Username));
	str_format(Tmp->m_Password, sizeof(Tmp->m_Password), "%s", HashPassword(pResult->GetString(1)).data);
	log_error("test", Tmp->m_Password);

	CGameControllerSheep *pController = (CGameControllerSheep *)pSelf->m_pController;
	pController->m_pPool->Execute(CGameControllerSheep::ExecuteRegister, std::move(Tmp), "account register");
}

bool CGameControllerSheep::ExecuteRegister(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	return AccountQuery(
		"INSERT INTO sheep_accounts (name, password) VALUES (?, SHA2(?, 256)) RETURNING "
		"ban_expiration, level, exp, vip, vip_expiration, staff_level, email, email_verified", 
		pSqlServer, pGameData, pError, ErrorSize
	);
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

bool CGameControllerSheep::ExecuteChangePassword(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "UPDATE sheep_accounts SET password=? WHERE name=?");
	if(!pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
		return false;

	const auto *pData = dynamic_cast<const CSqlAccountCredentialsRequest *>(pGameData);
	pSqlServer->BindString(1, pData->m_Password);
	pSqlServer->BindString(2, pData->m_Username);

	int NumUpdated;
	if(!pSqlServer->ExecuteUpdate(&NumUpdated, pError, ErrorSize))
		return false;
	
	auto *pResult = dynamic_cast<CSqlSuccessResult *>(pGameData->m_pResult.get());
	pResult->m_Success = NumUpdated != 0;

	return pResult->m_Success;
}

void CGameControllerSheep::ConChangePassword(IConsole::IResult *pResult, void *pUserData) {
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

	if (!VerifyPassword(pPlayer, pResult->GetString(0))) {
		pSelf->SendChatTarget(pResult->m_ClientId, "Current password is incorrect.");
		return;
	}

	auto Tmp = std::make_unique<CSqlAccountCredentialsRequest>(pPlayer->m_PasswordChangeSuccessResult);
	
	str_copy(Tmp->m_Username, pPlayer->m_AccountLoginResult->m_Username, sizeof(Tmp->m_Username));

	char hash[64];
	str_copy(hash, reinterpret_cast<const char*>(HashPassword(pResult->GetString(1)).data), sizeof(hash));
	str_copy(Tmp->m_Password, hash, sizeof(Tmp->m_Password));

	CGameControllerSheep *pController = (CGameControllerSheep *)pSelf->m_pController;
	pController->m_pPool->Execute(CGameControllerSheep::ExecuteChangePassword, std::move(Tmp), "account change password");
}