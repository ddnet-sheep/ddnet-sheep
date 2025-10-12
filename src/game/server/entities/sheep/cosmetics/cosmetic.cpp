#include "cosmetic.h"
#include <game/server/gamecontext.h>
#include <game/server/entities/character.h>
#include <game/server/gamemodes/sheep/sheep.h>

CCosmetic::CCosmetic(CGameWorld *pGameWorld, int EntityId, EItemVariant ItemVariant, CPlayer* Player, vec2 Pos, int ExtraIds)
    : CEntity(pGameWorld, EntityId, Pos), 
    m_Player(Player),
    m_ItemVariant(ItemVariant)
{
    GameWorld()->InsertEntity(this);

    m_SnapIds.push_back(GetId());
    for(int i = 0; i < ExtraIds; i++)
        m_SnapIds.push_back(Server()->SnapNewId());
}

void CCosmetic::Reset() {
    for(int id : m_SnapIds)
        Server()->SnapFreeId(id);
	GameWorld()->RemoveEntity(this);
}

bool CCosmetic::ShouldReset() {
    return
        !Player() ||
        GameServer()->Sheep()->m_Cosmetics.find(Player()) == GameServer()->Sheep()->m_Cosmetics.end() ||
        GameServer()->Sheep()->m_Cosmetics[Player()].find(m_ItemVariant) == GameServer()->Sheep()->m_Cosmetics[Player()].end() ||
        GameServer()->Sheep()->m_Cosmetics[Player()][m_ItemVariant] != this
    ;
}

bool CCosmetic::HasReset() {
    if(ShouldReset()) {
        Reset();
        return true;
    }
    return false;
}

bool CCosmetic::ShouldSnap() {
    // todo: mask
    return true;
}

void CCosmetic::Tick() {
    if(HasReset() || !Character())
        return;

	m_Pos = Character()->GetPos();
}