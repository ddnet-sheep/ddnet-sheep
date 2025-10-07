#include <game/server/gamemodes/sheep/sheep.h>
#include <game/server/entities/character.h>
#include <game/server/entities/sheep/text/text.h>

void CGameControllerSheep::ConLaserText(IConsole::IResult *pResult, void *pUserData) {
	CGameContext *pSelf = (CGameContext *)pUserData;
	const int ClientId = pResult->m_ClientId;

	if(!CheckClientId(ClientId))
		return;

	CCharacter *pChr = pSelf->GetPlayerChar(ClientId);
	if(!pChr)
		return;

	const vec2 Pos = pChr->m_Pos + vec2(0, -100);

	const char *pText = pResult->NumArguments() ? pResult->GetString(0) : "noob";

	new CLaserText(&pSelf->m_World, Pos, ClientId, 250, pText);
}

void CGameControllerSheep::ConProjectileText(IConsole::IResult *pResult, void *pUserData) {
	CGameContext *pSelf = (CGameContext *)pUserData;
	const int ClientId = pResult->m_ClientId;

	if(!CheckClientId(ClientId))
		return;

	CCharacter *pChr = pSelf->GetPlayerChar(ClientId);
	if(!pChr)
		return;

	const vec2 Pos = pChr->m_Pos + vec2(0, -60);
	const char *pText = pResult->GetString(0);
	new CProjectileText(&pSelf->m_World, Pos, ClientId, 250, pText, WEAPON_HAMMER);
}