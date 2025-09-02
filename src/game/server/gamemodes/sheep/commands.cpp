/* (c) Antonio Ianzano. See licence.txt and the readme.txt in the root of the distribution for more information. */
#include "sheep.h"

#include <game/server/gamecontext.h>
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