/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
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

bool VerifyPassword(const char *pExpected, const char* pPassword)
{
	SHA256_DIGEST Expected;
	sha256_from_str(&Expected, pExpected);
	return sha256_comp(HashPassword(pPassword), Expected) == 0;
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

void GenerateAccountLoginResult(IDbConnection *pSqlServer, const ISqlData *pGameData) {
	auto *pResult = dynamic_cast<CAccountLoginResult *>(pGameData->m_pResult.get());
	const auto *pData = dynamic_cast<const CSqlAccountCredentialsRequest *>(pGameData);

	str_copy(pResult->m_Username, pData->m_Username);
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
	
	pSqlServer->GetString(9, pResult->m_PasswordHash, sizeof(pResult->m_PasswordHash));
	
	pResult->m_AccountId = pSqlServer->GetInt64(10);
}

bool CGameControllerSheep::ExecuteLogin(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	auto *pResult = dynamic_cast<CAccountLoginResult *>(pGameData->m_pResult.get());

	if(!pSqlServer->PrepareStatement(
		"SELECT ban_expiration, level, exp, vip, vip_expiration, staff_level, email, email_verified, password, id "
		"FROM sheep_accounts WHERE name=?", 
		pError, ErrorSize)
	) {
		str_format(pResult->m_Message, sizeof(pResult->m_Message), "Database error (1).");
		return false;
	}

	const auto *pData = dynamic_cast<const CSqlAccountCredentialsRequest *>(pGameData);
	pSqlServer->BindString(1, pData->m_Username);

	bool End;
	if(!pSqlServer->Step(&End, pError, ErrorSize) || End) {
		str_format(pResult->m_Message, sizeof(pResult->m_Message), "User does not exist.");
		return false;
	}

	char aPasswordHash[65];
	pSqlServer->GetString(9, aPasswordHash, sizeof(aPasswordHash));

	if (!VerifyPassword(aPasswordHash, pData->m_Password)) {
		str_format(pResult->m_Message, sizeof(pResult->m_Message), "Invalid password.");
		return false;
	}

	int BanExpiration = pSqlServer->GetInt(1);
	if (BanExpiration != 0 && BanExpiration > time(0)) {
		str_format(pResult->m_Message, sizeof(pResult->m_Message), "Banned.");
		return false;
	}

	str_format(pResult->m_Message, sizeof(pResult->m_Message), "Successfully logged in.");

	GenerateAccountLoginResult(pSqlServer, pGameData);

	return true;
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
	str_format(Tmp->m_Password, sizeof(Tmp->m_Password), "%s", pResult->GetString(1));
	// Tmp->m_Artificial = true;

	CGameControllerSheep *pController = (CGameControllerSheep *)pSelf->m_pController;
	pController->m_pPool->Execute(CGameControllerSheep::ExecuteRegister, std::move(Tmp), "account register");
}

bool CGameControllerSheep::ExecuteRegister(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	auto *pResult = dynamic_cast<CAccountLoginResult *>(pGameData->m_pResult.get());

	if(!pSqlServer->PrepareStatement(
		"INSERT INTO sheep_accounts (name, password) VALUES (?, ?) RETURNING "
		"ban_expiration, level, exp, vip, vip_expiration, staff_level, email, email_verified, password, id", 
		pError, ErrorSize)
	) {
		str_format(pResult->m_Message, sizeof(pResult->m_Message), "Database error (1).");
		return false;
	}

	const auto *pData = dynamic_cast<const CSqlAccountCredentialsRequest *>(pGameData);
	pSqlServer->BindString(1, pData->m_Username);

	char aHash[65];
	sha256_str(HashPassword(pData->m_Password), aHash, sizeof(aHash));
	pSqlServer->BindString(2, aHash);

	bool End;
	if(!pSqlServer->Step(&End, pError, ErrorSize) || End) {
		str_format(pResult->m_Message, sizeof(pResult->m_Message), "User does already exist.");
		return false;
	}

	str_format(pResult->m_Message, sizeof(pResult->m_Message), "Successfully registered.");

	GenerateAccountLoginResult(pSqlServer, pGameData);

	return true;
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

bool CGameControllerSheep::ExecutePassword(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "UPDATE sheep_accounts SET password=SHA2(?, 256) WHERE name=?");
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

void CGameControllerSheep::ConPassword(IConsole::IResult *pResult, void *pUserData) {
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

	if (!VerifyPassword(pPlayer->m_AccountLoginResult->m_PasswordHash, pResult->GetString(0))) {
		pSelf->SendChatTarget(pResult->m_ClientId, "Current password is incorrect.");
		return;
	}

	pPlayer->m_PasswordChangeSuccessResult = std::make_shared<CSqlSuccessResult>();

	auto Tmp = std::make_unique<CSqlAccountCredentialsRequest>(pPlayer->m_PasswordChangeSuccessResult);
	str_copy(Tmp->m_Username, pPlayer->m_AccountLoginResult->m_Username, sizeof(Tmp->m_Username));
	str_copy(Tmp->m_Password, pResult->GetString(1), sizeof(Tmp->m_Password));

	CGameControllerSheep *pController = (CGameControllerSheep *)pSelf->m_pController;
	pController->m_pPool->Execute(CGameControllerSheep::ExecutePassword, std::move(Tmp), "account set password");
}