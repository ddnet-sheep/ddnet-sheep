#include "entity_owned.h"
#include <game/server/gamecontext.h>
#include <game/server/entities/character.h>
#include <game/server/gamemodes/sheep/sheep.h>

CEntityOwned::CEntityOwned(int EntityId, EItemVariant ItemVariant, CCharacter* pCharacter, int ExtraIds, vec2 Pos, int ProximityRadius)
    : CEntity(pCharacter->GameWorld(), EntityId, Pos, ProximityRadius)
    , m_Player(pCharacter->GetPlayer())
    , m_ItemVariant(ItemVariant)
{
    if(!pCharacter)
        return;

    GameWorld()->InsertEntity(this);

    m_SnapIds.push_back(GetId());
    for(int i = 0; i < ExtraIds; i++)
        m_SnapIds.push_back(Server()->SnapNewId());
}

void CEntityOwned::Reset() {
    for(int id : m_SnapIds)
        Server()->SnapFreeId(id);
	GameWorld()->RemoveEntity(this);
}

bool CEntityOwned::ShouldReset() {
    return
        m_ItemVariant != EItemVariant::ITEM_NONE && (
            !Player() ||
            GameServer()->Sheep()->m_Cosmetics.find(Player()) == GameServer()->Sheep()->m_Cosmetics.end() ||
            GameServer()->Sheep()->m_Cosmetics[Player()].find(m_ItemVariant) == GameServer()->Sheep()->m_Cosmetics[Player()].end() ||
            GameServer()->Sheep()->m_Cosmetics[Player()][m_ItemVariant] != this
        )
    ;
}

bool CEntityOwned::HasReset() {
    if(ShouldReset()) {
        Reset();
        return true;
    }
    return false;
}

int CEntityOwned::Team() {
    return Character() ? Character()->Team() : -1;
}

CClientMask CEntityOwned::TeamMask() {
	return GameServer()->Sheep()->Teams().TeamMask(Team());
}

void CEntityOwned::Tick() {
    if(HasReset() || !Character())
        return;

	m_Pos = Character()->GetPos();
}