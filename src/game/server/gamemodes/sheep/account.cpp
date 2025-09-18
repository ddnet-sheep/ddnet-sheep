/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#include "sheep.h"
#include "sql.h"
#include <game/server/player.h>
#include <engine/shared/config.h>

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

void CGameControllerSheep::OnPlayerLogin(CPlayer *pPlayer, bool Autologin) {
	int ClientId = pPlayer->GetCid();
	CServer *pServer = (CServer*)Server();
	CServer::CClient* pClient = &pServer->m_aClients[ClientId];

	LoadAccountItem(pPlayer);

	if(pPlayer->GetTeam() == TEAM_SPECTATORS)
		pPlayer->SetTeam(TEAM_FLOCK);

	// rcon auth
	if (pPlayer->m_AccountLoginResult->m_Staff > 0) {
		if(!Server()->IsSixup(ClientId)) {
			CMsgPacker Msgp(NETMSG_RCON_AUTH_STATUS, true);
			Msgp.AddInt(1); //authed
			Msgp.AddInt(1); //cmdlist
			Server()->SendMsg(&Msgp, MSGFLAG_VITAL, ClientId);
		} else {
			CMsgPacker Msgp(protocol7::NETMSG_RCON_AUTH_ON, true, true);
			Server()->SendMsg(&Msgp, MSGFLAG_VITAL, ClientId);
		}

		int AuthLevel = AUTHED_ADMIN;

		pClient->m_Authed = AuthLevel; // Keeping m_Authed around is unwise...
		CServer* pServer = (CServer*)Server();
		pClient->m_AuthKey = pServer->m_AuthManager.DefaultKey(AuthLevel);

		pServer->m_aClients[ClientId].m_pRconCmdToSend = pServer->Console()->FirstCommandInfo(pServer->ConsoleAccessLevel(ClientId), CFGFLAG_SERVER);
		pServer->SendRconCmdGroupStart(ClientId);
		if(pServer->m_aClients[ClientId].m_pRconCmdToSend == nullptr)
		{
			pServer->SendRconCmdGroupEnd(ClientId);
		}

		// DDRace
		GameServer()->OnSetAuthed(ClientId, AuthLevel);
	}

	if (!pPlayer->m_AccountLoginResult->m_Vanish) {
		SendActionMessage(pPlayer, Autologin ? ACTION_ENTER_AND_JOIN : ACTION_JOIN);
	}
}

void CGameControllerSheep::SendActionMessage(CPlayer *pPlayer, enum CAccountActions Action, char *pExtra) {
	int ClientId = pPlayer->GetCid();
	CServer *pServer = (CServer*)Server();

	bool IncludeClient = true;
	char aAction[64];
	switch (Action) {
		case ACTION_ENTER:
			str_copy(aAction, "entered", sizeof(aAction));
			break;
		case ACTION_JOIN:
			IncludeClient = false;
			str_copy(aAction, "joined", sizeof(aAction));
			break;
		case ACTION_ENTER_AND_JOIN:
			str_copy(aAction, "entered and joined", sizeof(aAction));
			break;
		case ACTION_LOGOUT:
			IncludeClient = false;
			str_copy(aAction, "logged out of", sizeof(aAction));
			break;
		case ACTION_LEAVE:
			IncludeClient = false;
			str_copy(aAction, "left", sizeof(aAction));
			break;
		default:
			IncludeClient = false;
			str_copy(aAction, "did something unknown to", sizeof(aAction));
			break;
	}

	char aExtra[128] = "";
	if(pExtra && pExtra[0] != '\0') {
		str_format(aExtra, sizeof(aExtra), " (%s)", pExtra);
	} else if(IncludeClient) {
		IServer::CClientInfo Info;
		if(Server()->GetClientInfo(ClientId, &Info) && Info.m_GotDDNetVersion)
			str_format(aExtra, sizeof(aExtra), " (%s %d)", pServer->m_aClients[ClientId].m_ClientName, Info.m_DDNetVersion);
	}

	char aBuf[512];
	char aTitle[33];
	if(pPlayer->m_AccountLoginResult && pPlayer->m_AccountLoginResult->m_Title[0] != '\0') {
		str_format(aTitle, sizeof(aTitle), "%s ", pPlayer->m_AccountLoginResult->m_Title);
	} else {
		aTitle[0] = '\0';
	}
	str_format(aBuf, sizeof(aBuf), "%s'%s' %s the game%s", aTitle, Server()->ClientName(ClientId), aAction, aExtra);
	GameServer()->SendChat(-1, TEAM_ALL, aBuf, -1, CGameContext::FLAG_SIX);
}

void CGameControllerSheep::OnPlayerLogout(CPlayer *pPlayer, const char *pReason, bool Silent = false) {
	pPlayer->SetTeam(TEAM_SPECTATORS);

	int ClientId = pPlayer->GetCid();
	CServer *pServer = (CServer*)Server();
	CServer::CClient* pClient = &pServer->m_aClients[ClientId];

	if(pClient->m_Authed > AUTHED_NO) {
		if(!Server()->IsSixup(ClientId)) {
			CMsgPacker Msgp(NETMSG_RCON_AUTH_STATUS, true);
			Msgp.AddInt(0); //authed
			Msgp.AddInt(0); //cmdlist
			Server()->SendMsg(&Msgp, MSGFLAG_VITAL, ClientId);
		} else {
			CMsgPacker Msgp(protocol7::NETMSG_RCON_AUTH_OFF, true, true);
			Server()->SendMsg(&Msgp, MSGFLAG_VITAL, ClientId);
		}

		int AuthLevel = AUTHED_NO;

		pClient->m_Authed = AuthLevel; // Keeping m_Authed around is unwise...
		pClient->m_AuthKey = -1;

		// DDRace
		GameServer()->OnSetAuthed(ClientId, AuthLevel);
	}

	if (!Silent) {
		SendActionMessage(pPlayer, ACTION_LOGOUT);
	}

	pPlayer->m_AccountLoginResult = nullptr;

	GameServer()->SendChatTarget(pPlayer->GetCid(), "You have been logged out.");
}

void CGameControllerSheep::ConLogin(IConsole::IResult *pResult, void *pUserData) {
	if (!CCommands::CommandValidateGuestForSelf(pResult, pUserData))
		return;
	
	CGameContext *pGameServer = (CGameContext *)pUserData;
	char aUsername[64];
	char aPassword[64];
	switch(pResult->NumArguments()) {
		case 2:
			str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
			str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));
			break;
		case 1:
			str_copy(aUsername, pGameServer->Server()->ClientName(pResult->m_ClientId), sizeof(aUsername));
			str_copy(aPassword, pResult->GetString(0), sizeof(aPassword));
			break;
		default:
			pGameServer->SendChatTarget(pResult->m_ClientId, "Usage: /login <password> or /login <username> <password>");
			return;
	}
	
	for (CPlayer *pPlayer : pGameServer->m_apPlayers) {
		if (pPlayer && pPlayer->m_AccountLoginResult != nullptr && !strcmp(pPlayer->m_AccountLoginResult->m_Username, aUsername)) {
			pGameServer->SendChatTarget(pResult->m_ClientId, "This account is already logged in.");
			return;
		}
	}
	
	CPlayer *pPlayer = CCommands::GetCaller(pResult, pUserData);
	pPlayer->m_AccountLoginResult = std::make_shared<CAccountLoginResult>();
	
	auto Tmp = std::make_unique<CSqlAccountCredentialsRequest>(pPlayer->m_AccountLoginResult);
	Tmp->m_Type = CSqlAccountCredentialsRequest::TYPE_PASSWORD;
	str_copy(Tmp->m_Username, aUsername, sizeof(Tmp->m_Username));
	str_copy(Tmp->m_Password, aPassword, sizeof(Tmp->m_Password));
	str_copy(Tmp->m_IP, pGameServer->Server()->ClientAddrString(pResult->m_ClientId, false), sizeof(Tmp->m_IP));
	
	CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;
	pController->m_pPool->Execute(CGameControllerSheep::ExecuteLogin, std::move(Tmp), "account login");
}

void GenerateAccountLoginResult(IDbConnection *pSqlServer, const ISqlData *pGameData) {
	auto *pResult = dynamic_cast<CAccountLoginResult *>(pGameData->m_pResult.get());

	pSqlServer->GetString(15, pResult->m_Username, sizeof(pResult->m_Username));
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

	pResult->m_Invisible = pSqlServer->GetInt(11) != 0;
	pResult->m_Vanish = pSqlServer->GetInt(12) != 0;

	pSqlServer->GetString(13, pResult->m_Title, sizeof(pResult->m_Title));
}

const char* Fields() {
	return "ban_expiration, level, exp, vip, vip_expiration, staff_level, email, email_verified, password, id, invisible, vanish, title, ip, name";
}

bool CGameControllerSheep::ExecuteLogin(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	auto *pResult = dynamic_cast<CAccountLoginResult *>(pGameData->m_pResult.get());
	const auto *pData = dynamic_cast<const CSqlAccountCredentialsRequest *>(pGameData);
	
	pResult->m_Type = pData->m_Type;

	char aSql[1024];
	str_format(aSql, sizeof(aSql), "SELECT %s FROM sheep_accounts WHERE name=?", Fields());
	if(!pSqlServer->PrepareStatement(aSql, pError, ErrorSize)) {
		str_copy(pResult->m_Message, "Database error (1).");
		return false;
	}

	pSqlServer->BindString(1, pData->m_Username);

	bool End;
	if(!pSqlServer->Step(&End, pError, ErrorSize) || End) {
		str_copy(pResult->m_Message, "User does not exist. Please /register <password>");
		return false;
	}

	if(pData->m_Type == CSqlAccountCredentialsRequest::TYPE_PASSWORD) {
		char aPasswordHash[65];
		pSqlServer->GetString(9, aPasswordHash, sizeof(aPasswordHash));

		if (!VerifyPassword(aPasswordHash, pData->m_Password)) {
			str_copy(pResult->m_Message, "Login: Invalid password.");
			return false;
		}
	} else if(pData->m_Type == CSqlAccountCredentialsRequest::TYPE_IP) {
		char aIP[64];
		pSqlServer->GetString(14, aIP, sizeof(aIP));

		if (strcmp(aIP, pData->m_IP) != 0) {
			str_copy(pResult->m_Message, "Autologin: IP mismatch. Please /login <password>.");
			return false;
		}
	}

	int BanExpiration = pSqlServer->GetInt(1);
	if (BanExpiration != 0 && BanExpiration > time(0)) {
		int Bantime = BanExpiration - time(0);
		str_format(pResult->m_Message, sizeof(pResult->m_Message), "Your account is banned. Time left: %d seconds.", Bantime);
		return false;
	}

	str_copy(pResult->m_Message, "Successfully logged in.");

	GenerateAccountLoginResult(pSqlServer, pGameData);

	// update ip
	if(pData->m_Type == CSqlAccountCredentialsRequest::TYPE_PASSWORD) {
		if(!pSqlServer->PrepareStatement("UPDATE sheep_accounts SET ip=? WHERE name=?", pError, ErrorSize))
			return true;

		pSqlServer->BindString(1, pData->m_IP);
		pSqlServer->BindString(2, pData->m_Username);

		int NumUpdated;
		pSqlServer->ExecuteUpdate(&NumUpdated, pError, ErrorSize);
	}
	return true;
}

void CGameControllerSheep::ConRegister(IConsole::IResult *pResult, void *pUserData) {
	if (!CCommands::CommandValidateGuestForSelf(pResult, pUserData))
		return;
	
	CGameContext *pGameServer = (CGameContext *)pUserData;
		
	char aUsername[64];
	char aPassword[64];
	switch(pResult->NumArguments()) {
		case 2:
			str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
			str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));
			break;
		case 1:
			str_copy(aUsername, pGameServer->Server()->ClientName(pResult->m_ClientId), sizeof(aUsername));
			str_copy(aPassword, pResult->GetString(0), sizeof(aPassword));
			break;
		default:
			pGameServer->SendChatTarget(pResult->m_ClientId, "Usage: /register <password> or /register <username> <password>");
			return;
	}
		
	CPlayer *pPlayer = CCommands::GetCaller(pResult, pUserData);
	pPlayer->m_AccountLoginResult = std::make_shared<CAccountLoginResult>();

	auto Tmp = std::make_unique<CSqlAccountCredentialsRequest>(pPlayer->m_AccountLoginResult);
	Tmp->m_Type = CSqlAccountCredentialsRequest::TYPE_PASSWORD;
	str_copy(Tmp->m_Username, aUsername, sizeof(Tmp->m_Username));
	str_copy(Tmp->m_Password, aPassword, sizeof(Tmp->m_Password));
	
	CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;
	pController->m_pPool->Execute(CGameControllerSheep::ExecuteRegister, std::move(Tmp), "account register");
}

bool CGameControllerSheep::ExecuteRegister(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	auto *pResult = dynamic_cast<CAccountLoginResult *>(pGameData->m_pResult.get());

	char aSql[1024];
	str_format(aSql, sizeof(aSql), "INSERT INTO sheep_accounts (name, password) VALUES (?, ?) RETURNING %s", Fields());
	if(!pSqlServer->PrepareStatement(aSql, pError, ErrorSize)) {
		str_copy(pResult->m_Message, "Database error (1).");
		return false;
	}

	const auto *pData = dynamic_cast<const CSqlAccountCredentialsRequest *>(pGameData);
	pSqlServer->BindString(1, pData->m_Username);

	char aHash[65];
	sha256_str(HashPassword(pData->m_Password), aHash, sizeof(aHash));
	pSqlServer->BindString(2, aHash);

	bool End;
	if(!pSqlServer->Step(&End, pError, ErrorSize) || End) {
		str_copy(pResult->m_Message, "User does already exist.");
		return false;
	}

	str_copy(pResult->m_Message, "Successfully registered.");

	GenerateAccountLoginResult(pSqlServer, pGameData);

	// update ip
	if(!pSqlServer->PrepareStatement("UPDATE sheep_accounts SET ip=? WHERE name=?", pError, ErrorSize))
		return true;

	pSqlServer->BindString(1, pData->m_IP);
	pSqlServer->BindString(2, pData->m_Username);

	int NumUpdated;
	pSqlServer->ExecuteUpdate(&NumUpdated, pError, ErrorSize);

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
	pController->OnPlayerLogout(pPlayer, "logged out");
}

bool CGameControllerSheep::ExecutePassword(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
	char aBuf[256];
	str_copy(aBuf, "UPDATE sheep_accounts SET password=SHA2(?, 256) WHERE name=?");
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

	// TODO: change to sync logic, make instant sync

	return pResult->m_Success;
}

void CGameControllerSheep::ConPassword(IConsole::IResult *pResult, void *pUserData) {
	if(!CCommands::CommandValidateAuthForSelf(pResult, pUserData))
		return;
	
	CGameContext *pGameServer = (CGameContext *)pUserData;
	CPlayer *pVictim = pGameServer->m_apPlayers[pResult->m_ClientId];

	if (!VerifyPassword(pVictim->m_AccountLoginResult->m_PasswordHash, pResult->GetString(0))) {
		pGameServer->SendChatTarget(pResult->m_ClientId, "Current password is incorrect.");
		return;
	}

	// TODO: change to sync logic, make instant sync

	pVictim->m_PasswordChangeSuccessResult = std::make_shared<CSqlSuccessResult>();

	auto Tmp = std::make_unique<CSqlAccountCredentialsRequest>(pVictim->m_PasswordChangeSuccessResult);
	Tmp->m_Type = CSqlAccountCredentialsRequest::TYPE_PASSWORD;
	str_copy(Tmp->m_Username, pVictim->m_AccountLoginResult->m_Username, sizeof(Tmp->m_Username));
	str_copy(Tmp->m_Password, pResult->GetString(1), sizeof(Tmp->m_Password));

	CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;
	pController->m_pPool->Execute(CGameControllerSheep::ExecutePassword, std::move(Tmp), "account set password");
}

void CGameControllerSheep::ConVanish(IConsole::IResult *pResult, void *pUserData) {
	if(!CCommands::CommandValidateStaffForVictim(pResult, pUserData))
		return;
	
	CGameContext *pGameServer = (CGameContext *)pUserData;
	CPlayer *pVictim = CCommands::GetVictimOrCaller(pResult, pUserData);
	CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;

	pVictim->m_AccountLoginResult->m_Vanish = !pVictim->m_AccountLoginResult->m_Vanish;
	pController->SendActionMessage(pVictim, pVictim->m_AccountLoginResult->m_Vanish ? ACTION_LEAVE : ACTION_ENTER_AND_JOIN);

	if (pVictim->GetCid() != pResult->m_ClientId) {
		pGameServer->SendChatTarget(pResult->m_ClientId, pVictim->m_AccountLoginResult->m_Vanish ? "They are now vanished." : "They are no longer vanished.");
	}
	pGameServer->SendChatTarget(pVictim->GetCid(), pVictim->m_AccountLoginResult->m_Vanish ? "You are now vanished." : "You are no longer vanished.");

	CServer *pServer = (CServer *)pGameServer->Server();
	pServer->m_ServerInfoNeedsUpdate = true;
}

void CGameControllerSheep::ConInvisible(IConsole::IResult *pResult, void *pUserData) {
	if(!CCommands::CommandValidateStaffForVictim(pResult, pUserData))
		return;
	
	CGameContext *pGameServer = (CGameContext *)pUserData;
	CPlayer *pVictim = CCommands::GetVictimOrCaller(pResult, pUserData);
	CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;

	pVictim->m_AccountLoginResult->m_Invisible = !pVictim->m_AccountLoginResult->m_Invisible;
	if (pVictim->GetCid() != pResult->m_ClientId) {
		pGameServer->SendChatTarget(pResult->m_ClientId, pVictim->m_AccountLoginResult->m_Invisible ? "They are now invisible." : "They are no longer invisible.");
	}
	pGameServer->SendChatTarget(pVictim->GetCid(), pVictim->m_AccountLoginResult->m_Invisible ? "You are now invisible." : "You are no longer invisible.");
}

void CGameControllerSheep::ConIgnoreInvisible(IConsole::IResult *pResult, void *pUserData) {
	if(!CCommands::CommandValidateStaffForVictim(pResult, pUserData))
		return;
	
	CGameContext *pGameServer = (CGameContext *)pUserData;
	CPlayer *pVictim = CCommands::GetVictimOrCaller(pResult, pUserData);
	CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;

	pVictim->m_AccountLoginResult->m_IgnoreInvisible = !pVictim->m_AccountLoginResult->m_IgnoreInvisible;
	if (pVictim->GetCid() != pResult->m_ClientId) {
		pGameServer->SendChatTarget(pResult->m_ClientId, pVictim->m_AccountLoginResult->m_IgnoreInvisible ? "They are now ignoring invisible players." : "They are no longer ignoring invisible players.");
	}
	pGameServer->SendChatTarget(pVictim->GetCid(), pVictim->m_AccountLoginResult->m_IgnoreInvisible ? "You are now ignoring invisible players." : "You are no longer ignoring invisible players.");
}

void CGameControllerSheep::ConForceLogin(IConsole::IResult *pResult, void *pUserData) {
	CGameContext *pGameServer = (CGameContext *)pUserData;
	if(!CCommands::ValidateStaff(pResult, pUserData, 1)) {
		pGameServer->SendChatTarget(pResult->m_ClientId, "You are not authorized to do that.");
		return;
	}

	CPlayer *pVictim = CCommands::GetVictimOrCaller(pResult, pUserData);
	if(!pVictim) {
		pGameServer->SendChatTarget(pResult->m_ClientId, "Invalid client id.");
		return;
	}

	char aUsername[64];
	str_copy(aUsername, pResult->GetString(pResult->NumArguments() - 1), sizeof(aUsername));
	for (CPlayer *pPlayer : pGameServer->m_apPlayers) {
		if (pPlayer && pPlayer->m_AccountLoginResult != nullptr && !strcmp(pPlayer->m_AccountLoginResult->m_Username, aUsername)) {
			pGameServer->SendChatTarget(pResult->m_ClientId, "This account is already logged in.");
			return;
		}
	}
	
	CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;
	if(pVictim->m_AccountLoginResult) {
		pController->OnPlayerLogout(pVictim, nullptr);
	}

	pVictim->m_AccountLoginResult = std::make_shared<CAccountLoginResult>();
	
	auto Tmp = std::make_unique<CSqlAccountCredentialsRequest>(pVictim->m_AccountLoginResult);
	Tmp->m_Type = CSqlAccountCredentialsRequest::TYPE_FORCED;
	str_copy(Tmp->m_Username, aUsername, sizeof(Tmp->m_Username));

	pController->m_pPool->Execute(CGameControllerSheep::ExecuteLogin, std::move(Tmp), "account login");
}

void CGameControllerSheep::ConForceLogout(IConsole::IResult *pResult, void *pUserData) {
	CGameContext *pGameServer = (CGameContext *)pUserData;
	if(!CCommands::ValidateStaff(pResult, pUserData, 1)) {
		pGameServer->SendChatTarget(pResult->m_ClientId, "You are not authorized to do that.");
		return;
	}

	CPlayer *pVictim = CCommands::GetVictimOrCaller(pResult, pUserData);
	if(!pVictim) {
		pGameServer->SendChatTarget(pResult->m_ClientId, "Invalid client id.");
		return;
	}
	
	CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;
	if(!pVictim->m_AccountLoginResult) {
		pGameServer->SendChatTarget(pResult->m_ClientId, "Player is not logged in.");
		return;
	}

	pController->OnPlayerLogout(pVictim, nullptr);
}