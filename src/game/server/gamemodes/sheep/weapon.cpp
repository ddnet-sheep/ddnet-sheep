#include "sheep.h"
#include <game/server/entities/character.h>

void CGameControllerSheep::ConGiveWeapon(IConsole::IResult *pResult, void *pUserData) {
    CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;
        
	CCharacter *pChr = pSelf->GetPlayerChar(pResult->m_ClientId);
	if(!pChr)
		return;

	int WeaponId = pResult->GetInteger(0);
    if(std::clamp(WeaponId, -1, NUM_WEAPONS - 1) != WeaponId) {
        pSelf->SendChatTarget(pResult->m_ClientId, "Invalid weapon ID");
        return;
    }

	const bool GotWeapon = pChr->GetWeaponGot(WeaponId);
	pChr->GiveWeapon(WeaponId, GotWeapon);
	if(!GotWeapon)
		pChr->SetActiveWeapon(WeaponId);
}