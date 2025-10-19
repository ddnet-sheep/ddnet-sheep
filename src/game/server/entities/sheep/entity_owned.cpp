#include "entity_owned.h"
#include <game/server/gamecontext.h>
#include <game/server/entities/character.h>
#include <game/server/gamemodes/sheep/sheep.h>

CEntityOwned::CEntityOwned(int EntityId, int ItemId, CCharacter* pCharacter, int ExtraIds, vec2 Pos, int ProximityRadius)
    : CEntity(pCharacter->GameWorld(), EntityId, Pos, ProximityRadius)
    , m_Player(pCharacter->GetPlayer())
    , m_ItemId(ItemId)
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

    m_SnapIds.clear();
    
	GameWorld()->RemoveEntity(this);
}

bool CEntityOwned::ShouldReset() {
    return m_ItemId > -1 && (!Player() || GameServer()->Sheep()->m_Cosmetics[Player()->GetCid()][m_ItemId] != this);
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