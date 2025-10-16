#include <game/server/gamemodes/sheep/sheep.h>

#include <game/server/entities/character.h>

#include <game/server/entities/sheep/cosmetics/dot_trail.h>
#include <game/server/entities/sheep/cosmetics/heart_hat.h>
#include <game/server/entities/sheep/cosmetics/staff_ind.h>
#include <game/server/entities/sheep/cosmetics/firework.h>
#include <game/server/entities/sheep/cosmetics/rotating_ball.h>
#include <game/server/entities/sheep/cosmetics/lovely.h>
#include <game/server/entities/sheep/cosmetics/epic_circle.h>

#include <random>

void CGameControllerSheep::LoadCosmetics(CPlayer* pPlayer) {
    m_CosmeticsResult[pPlayer->GetCid()] = std::make_shared<CCosmeticsResult>();
    auto Tmp = std::make_unique<CSqlAccountIdRequest>(m_CosmeticsResult[pPlayer->GetCid()]);
    Tmp->m_AccountId = pPlayer->m_AccountLoginResult->m_AccountId;
	m_pPool->Execute(CGameControllerSheep::ExecuteLoadCosmetics, std::move(Tmp), "load cosmetics");
}

bool CGameControllerSheep::ExecuteLoadCosmetics(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize) {
    auto *pResult = dynamic_cast<CCosmeticsResult *>(pGameData->m_pResult.get());

	if(!pSqlServer->PrepareStatement("SELECT item_id, state FROM sheep_cosmetics WHERE account_id = ?", pError, ErrorSize))
	    return false;

    const auto *pData = dynamic_cast<const CSqlAccountIdRequest *>(pGameData);
    pSqlServer->BindInt(1, pData->m_AccountId);

	for(int i = 0; i < NUM_COSMETICS; i++)
		pResult->m_State[i] = 0;

    bool End;
    do {
        pSqlServer->Step(&End, pError, ErrorSize);
        
        if(End)
            break;

        pResult->m_State[pSqlServer->GetInt(1)] = pSqlServer->GetInt(2);
    } while(!End);

	return true;
}

void CGameControllerSheep::DespawnCosmetics(int ClientId) {
	for(CEntity* Cosmetic : m_Cosmetics[ClientId])
		if(Cosmetic != nullptr)
			Cosmetic->Destroy();

	for(int i = 0; i < NUM_COSMETICS; i++)
		m_Cosmetics[ClientId][i] = nullptr;
}

CEntityOwned* BuildCosmetic(CCharacter* pCharacter, int Cosmetic) {
	switch(Cosmetic) {
		case COSMETIC_HEART_HAT: return new CHeartHat(pCharacter);
		case COSMETIC_LOVELY: return new CLovely(pCharacter);
		case COSMETIC_DOT_TRAIL: return new CDotTrail(pCharacter);
		case COSMETIC_STAFF_IND: return new CStaffInd(pCharacter);
		case COSMETIC_ROTATING_BALL: return new CRotatingBall(pCharacter);
		case COSMETIC_EPIC_CIRCLE: return new CEpicCircle(pCharacter);
	}
	return nullptr;
}

void CGameControllerSheep::SpawnCosmetics(int ClientId) {
	DespawnCosmetics(ClientId);

	CPlayer* pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!GameServer()->GetPlayerChar(ClientId) || !pPlayer->IsLoggedIn())
		return;

	for (int i = 0; i < NUM_COSMETICS; i++) {
		if(m_CosmeticsResult[ClientId]->m_State[i] > 0)
			m_Cosmetics[ClientId][i] = BuildCosmetic(GameServer()->GetPlayerChar(ClientId), i);
	}
}

void CGameControllerSheep::CreateLaserDeath(int Type, int pOwner, vec2 pPos, CClientMask pMask) {
	SLaserDeath effect;

	std::random_device rd;
	std::uniform_int_distribution<long> dist(5.0, 50.0);

	effect.m_Pos = pPos;
	effect.m_Mask = pMask;
	effect.m_Owner = pOwner;

	effect.m_Remaining = 15;
	effect.m_EndTick = Server()->Tick() + (Server()->TickSpeed() / 4.5f * effect.m_Remaining);
	effect.m_Sound = SOUND_HOOK_LOOP;
	for(int Num = 0; Num < effect.m_Remaining; Num++)
	{
		long Random = dist(rd) + Num;

		vec2 Pos = pPos + random_direction() * Random;

		effect.m_vIds.push_back(Server()->SnapNewId());

		effect.m_vFrom.push_back(Pos);
		effect.m_vTo.push_back(Pos);
		effect.m_vStartTick.push_back(Server()->Tick() + Server()->TickSpeed() / 5 * Num);
	}

	m_vLaserDeaths.push_back(effect);
}

void CGameControllerSheep::ConCosmetic(IConsole::IResult *pResult, void *pUserData) {
	CPlayer* pVictim = CCommands::GetVictimOrCaller(pResult, pUserData, 2);
	CGameContext *pGameServer = (CGameContext *)pUserData;
	if(pVictim == nullptr) {
		pGameServer->SendChatTarget(pResult->m_ClientId, "Invalid player");
		return;
	}

	if(!pVictim->IsLoggedIn() || pGameServer->Sheep()->m_CosmeticsResult[pVictim->GetCid()] == nullptr) {
		pGameServer->SendChatTarget(pResult->m_ClientId, "Player is not logged in");
		return;
	}

	int CosmeticId = CCosmetics::GetId(pResult->GetString(pResult->NumArguments() - 1));
	if(std::clamp(CosmeticId, 0, NUM_COSMETICS - 1) != CosmeticId) {
		char aBuf[256] = "Invalid cosmetic. Available cosmetics: ";
		for (int i = 0; i < NUM_COSMETICS; i++) {
			str_append(aBuf, CCosmetics::GetName(i), sizeof(aBuf));
			if (i == NUM_COSMETICS - 1) break;
			str_append(aBuf, ", ", sizeof(aBuf));
		}

		pGameServer->SendChatTarget(pResult->m_ClientId, aBuf);
		return;
	}

	if(pGameServer->Sheep()->m_CosmeticsResult[pVictim->GetCid()]->m_State[CosmeticId] == 0) {
		pGameServer->Sheep()->m_CosmeticsResult[pVictim->GetCid()]->m_State[CosmeticId] = 1;
		pGameServer->SendChatTarget(pResult->m_ClientId, "Gave cosmetic");
	} else {
		pGameServer->Sheep()->m_CosmeticsResult[pVictim->GetCid()]->m_State[CosmeticId] = 0;
		pGameServer->SendChatTarget(pResult->m_ClientId, "Removed cosmetic");
	}

	pGameServer->Sheep()->SpawnCosmetics(pVictim->GetCid());
}

void CGameControllerSheep::SaveCosmetics(CPlayer *pPlayer) {
	if(!pPlayer->IsLoggedIn())
		return;

	std::shared_ptr<ISheepSqlResult> Result = std::make_shared<ISheepSqlResult>();

	auto Tmp = std::make_unique<CSqlCosmeticsRequest>(Result);
	Tmp->m_AccountId = pPlayer->m_AccountLoginResult->m_AccountId;

	for(int i = 0; i < NUM_COSMETICS; i++)
		Tmp->m_State[i] = m_CosmeticsResult[pPlayer->GetCid()]->m_State[i];

	m_pPool->ExecuteWrite(CGameControllerSheep::ExecuteSaveCosmetic, std::move(Tmp), "cosmetics save");
}

bool CGameControllerSheep::ExecuteSaveCosmetic(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize) {
	if(Write::NORMAL != w)
		return true;
		
	auto *pResult = dynamic_cast<CAccountDataResult *>(pGameData->m_pResult.get());
	const auto *pData = dynamic_cast<const CSqlCosmeticsRequest *>(pGameData);

	char aSql[2048] = "INSERT INTO sheep_cosmetics (account_id, item_id, state, created_at, updated_at) VALUES ";
	int Bind = 1;
	for(int i = 0; i < NUM_COSMETICS; i++) {
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "(%d, %d, %d, NOW(), NOW())", pData->m_AccountId, i, pData->m_State[i]);
		str_append(aSql, aBuf, sizeof(aSql));

		if(i < NUM_COSMETICS - 1)
			str_append(aSql, ", ");
	}


	str_append(aSql, " ON DUPLICATE KEY UPDATE state=VALUES(state), updated_at=NOW()");

	if(!pSqlServer->PrepareStatement(aSql, pError, ErrorSize)) {
		return false;
	}
	
	int NumUpdated;
	if(!pSqlServer->ExecuteUpdate(&NumUpdated, pError, ErrorSize)) {
		return false;
	}

	return NumUpdated != 0;
}