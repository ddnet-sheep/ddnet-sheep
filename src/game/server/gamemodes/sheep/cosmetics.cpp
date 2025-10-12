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

void CGameControllerSheep::ConInverseAim(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->NumArguments() ? pResult->GetVictim() : pResult->m_ClientId;

	if(pResult->GetInteger(0) == -1)
		Victim = pResult->m_ClientId;

	CPlayer *pPl = pSelf->m_apPlayers[Victim];

	if(!pPl)
		return;

	bool Set = !pPl->m_Cosmetics.m_InverseAim;
	pPl->SetInverseAim(Set);
	log_info("cosmetics", "Set inverse aim to %d for player %s", Set, pSelf->Server()->ClientName(Victim));
}

void CGameControllerSheep::ConDamageIndEffect(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	int Victim = pResult->NumArguments() > 1 ? pResult->GetVictim() : pResult->m_ClientId;

	if(pResult->GetInteger(1) == -1)
		Victim = pResult->m_ClientId;

	CPlayer *pPl = pSelf->m_apPlayers[Victim];

	if(!pPl)
		return;

	int Type = pResult->NumArguments() < 1 ? 0 : pResult->GetInteger(0);
	pPl->SetDamageIndType(Type);
	log_info("cosmetics", "Set damage ind to %d for player %s", Type, pSelf->Server()->ClientName(Victim));
}

void CGameControllerSheep::DespawnCosmetics(CPlayer *pPlayer) {
	if(!pPlayer)
		return;

	for (const auto& [Variant, Cosmetic] : m_Cosmetics[pPlayer])
		if(Cosmetic != nullptr)
			Cosmetic->Destroy();

	m_Cosmetics.erase(pPlayer);
}

CCosmetic* BuildCosmetic(CPlayer* pPlayer, EItemVariant Variant) {
	switch(Variant) {
		case EItemVariant::ITEM_HEART_HAT: return new CHeartHat(pPlayer);
		case EItemVariant::ITEM_LOVELY: return new CLovely(pPlayer);
		case EItemVariant::ITEM_DOT_TRAIL: return new CDotTrail(pPlayer);
		case EItemVariant::ITEM_STAFF_IND: return new CStaffInd(pPlayer);
		case EItemVariant::ITEM_ROTATING_BALL: return new CRotatingBall(pPlayer);
		case EItemVariant::ITEM_EPIC_CIRCLE: return new CEpicCircle(pPlayer);
	}
	return nullptr;
}

void CGameControllerSheep::SpawnCosmetics(CPlayer *pPlayer) {
	DespawnCosmetics(pPlayer);

	if(!pPlayer || !pPlayer->IsLoggedIn() || !pPlayer->IsItemsLoaded())
		return;
	
	for (const auto& [Type, AccountItem] : pPlayer->m_AccountItemResult->m_AccountItem) {
		auto Pair = m_ItemsResult->m_Items.find(Type);
        if(Pair == m_ItemsResult->m_Items.end() || Pair->second.m_Type != EItemType::TYPE_COSMETIC || AccountItem.m_State != (int)EItemState::ITEM_STATE_ACTIVE)
			continue;

		m_Cosmetics[pPlayer][AccountItem.m_Variant] = BuildCosmetic(pPlayer, AccountItem.m_Variant);
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