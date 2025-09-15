#include "sheep.h"
#include <game/server/entities/character.h>

void CGameControllerSheep::ConWeapon(IConsole::IResult *pResult, void *pUserData) {
	CPlayer* pVictim;
	if (pResult->NumArguments() > 1) {
		if(!CCommands::CommandValidateStaffForVictim(pResult, pUserData))
			return;
		pVictim = CCommands::GetVictimOrCaller(pResult, pUserData);
	} else {
		if(!CCommands::ValidateStaff(pResult, pUserData, 1))
			return;
		pVictim = CCommands::GetCaller(pResult, pUserData);
	}

	CCharacter *pVictimChar = pVictim->GetCharacter();
	CGameContext *pGameServer = (CGameContext *)pUserData;

	if(!pVictimChar) {
		pGameServer->SendChatTarget(pResult->m_ClientId, "The player is not spawned");
		return;
	}

	int WeaponId = CWeapon::GetId(pResult->GetString(pResult->NumArguments() - 1));
	if(std::clamp(WeaponId, -1, NUM_WEAPONS - 1) != WeaponId) {
		char aBuf[256] = "Invalid weapon. Available weapons: ";
		for (int i = 0; i < NUM_WEAPONS; i++) {
			str_append(aBuf, CWeapon::GetName(i), sizeof(aBuf));
			if (i == NUM_WEAPONS - 1) break;
			str_append(aBuf, ", ", sizeof(aBuf));
		}

		pGameServer->SendChatTarget(pResult->m_ClientId, aBuf);
		return;
	}

	const bool GotWeapon = pVictimChar->GetWeaponGot(WeaponId);
	pVictimChar->GiveWeapon(WeaponId, GotWeapon);
	if(!GotWeapon)
		pVictimChar->SetActiveWeapon(WeaponId);

	pGameServer->SendChatTarget(pResult->m_ClientId, GotWeapon ? "Removed weapon" : "Gave weapon");
}