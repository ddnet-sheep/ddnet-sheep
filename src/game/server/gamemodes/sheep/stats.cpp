#include "sheep.h"

#include <engine/shared/config.h>
#include <game/server/entities/character.h>

#include <engine/server/databases/connection.h>
#include <engine/server/databases/connection_pool.h>

int CGameControllerSheep::CalcPlayerExpPerMinute(CPlayer *pPlayer) {
	int Exp = g_Config.m_SvSheepExpPerMinute;

    if(pPlayer->m_AccountLoginResult->m_Vip > 0) {
        Exp += pPlayer->m_AccountLoginResult->m_Vip; // vip 1 => +1 exp, vip 2 => +2 exp, ...
    }

    // todo: global exp multiplier

    return Exp;
}

// 1 => 10
// 2 => 40
// 3 => 90
// 4 => 160
// 5 => 250
// 6 => 360
int CGameControllerSheep::CalcPlayerNeededExp(CPlayer *pPlayer) {
    int NextLevel = pPlayer == nullptr || !pPlayer->IsLoggedIn() ? 1 : pPlayer->m_AccountLoginResult->m_Level + 1;
    return NextLevel * NextLevel * 10;
}

void CGameControllerSheep::GivePlayerExp(CPlayer *pPlayer, int Exp, char* pReason) {
	if (!pPlayer->IsLoggedIn()) {
        if(pReason && pReason[0] != '\0') {
            char aBuf[256];
            str_format(aBuf, sizeof(aBuf), "You missed %d EXP (%s) because you are not logged in.", Exp, pReason);
            GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
        }
		return;
    }

	pPlayer->m_AccountLoginResult->m_Exp += Exp;
	
	if (pPlayer->m_AccountLoginResult->m_Exp < 0)
        pPlayer->m_AccountLoginResult->m_Exp = 0;

    while (true) {
        int NeededExp = CalcPlayerNeededExp(pPlayer);
        if (pPlayer->m_AccountLoginResult->m_Exp < NeededExp)
            break;
        
        pPlayer->m_AccountLoginResult->m_Level++;
        pPlayer->m_AccountLoginResult->m_Exp -= NeededExp;

        char aBuf[256];
        str_format(aBuf, sizeof(aBuf), "%s has leveled up to level %ld", Server()->ClientName(pPlayer->GetCid()), pPlayer->m_AccountLoginResult->m_Level);
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(!Server()->ClientIngame(i))
                continue;

            GameServer()->SendChatTarget(i, aBuf);
        }

		if(pPlayer->GetCharacter() != nullptr)
			GameServer()->CreateBirthdayEffect(pPlayer->GetCharacter()->GetPos(), pPlayer->GetCharacter()->TeamMask());
    }
    
    SaveAccount(pPlayer);
}

void CGameControllerSheep::GivePlayerMoney(CPlayer* pPlayer, int64_t Amount, const char *pReason) {
	if(pPlayer == nullptr || !pPlayer->IsLoggedIn())
        return;

	pPlayer->m_AccountLoginResult->m_Money += Amount;

	char aBuf[256];
	if(pReason != nullptr && pReason[0] != '\0') {
		str_format(aBuf, sizeof(aBuf), "+%ld %s (%s)", Amount, g_Config.m_SvSheepMoneyName, pReason);
		GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
	}	

	CCharacter *pChr = pPlayer->GetCharacter();
	if(pChr) {
		const vec2 Pos = pChr->m_Pos + vec2(0, -74);
		char aText[66];
		str_format(aText, sizeof(aText), "+%ld", Amount);
		// new CProjectileText(pChr->GameWorld(), Pos, pPlayer->GetCid(), 175, aText, WEAPON_HAMMER);
		pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + 175);
	}

	SaveAccount(pPlayer);
}

void CGameControllerSheep::GivePlayerPlaytime(CPlayer *pPlayer, int Minutes) {
	if(!pPlayer->IsLoggedIn())
        return;

	pPlayer->m_AccountLoginResult->m_Playtime += Minutes;
	if(pPlayer->m_AccountLoginResult->m_Playtime % 60 == 0) {
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "for reaching %ld minutes of playtime!", pPlayer->m_AccountLoginResult->m_Playtime);
		GivePlayerMoney(pPlayer, g_Config.m_SvSheepMoneyPlaytime, aBuf);
	}
}

void CGameControllerSheep::ConStats(IConsole::IResult *pResult, void *pUserData) {
    CPlayer *pPlayer = CCommands::GetCaller(pResult, pUserData);
    CGameContext *pGameServer = (CGameContext *)pUserData;
    CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;

    if(pResult->NumArguments() == 0) {
        if(!pPlayer->IsLoggedIn()) {
            pGameServer->SendChatTarget(pPlayer->GetCid(), "You are not logged in.");
            return;
        }

        pPlayer->m_AccountStatsResult = pPlayer->m_AccountLoginResult;
    } else {
        for(CPlayer *pOther : pGameServer->m_apPlayers) {
            if(pOther != nullptr && pOther->IsLoggedIn() && !strcmp(pOther->m_AccountLoginResult->m_Username, pResult->GetString(0))) {
                pPlayer->m_AccountStatsResult = pOther->m_AccountLoginResult;
                return;
            }
        }

        for(CPlayer *pOther : pGameServer->m_apPlayers) {
            if(pOther != nullptr && pOther->IsLoggedIn() && !strcmp(pController->Server()->ClientName(pOther->GetCid()), pResult->GetString(0))) {
                pPlayer->m_AccountStatsResult = pOther->m_AccountLoginResult;
                return;
            }
        }

        pPlayer->m_AccountStatsResult = std::make_shared<CAccountDataResult>();

        auto Tmp = std::make_unique<CSqlAccountCredentialsRequest>(pPlayer->m_AccountStatsResult);
        Tmp->m_Type = CSqlAccountCredentialsRequest::TYPE_FORCED;
        str_copy(Tmp->m_Username, pResult->GetString(0), sizeof(Tmp->m_Username));

	    pController->m_pPool->Execute(CGameControllerSheep::ExecuteLogin, std::move(Tmp), "account stats");
    }
}