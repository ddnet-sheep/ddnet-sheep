#ifndef GAME_SERVER_ENTITIES_SHEEP_ENTITY_OWNED_H
#define GAME_SERVER_ENTITIES_SHEEP_ENTITY_OWNED_H

#include <game/server/entity.h>
#include <game/server/player.h>

class CEntityOwned : public CEntity
{    
protected:
    CPlayer* m_Player;
    int m_ItemId;
    std::vector<int> m_SnapIds;

    bool ShouldReset();

    bool HasReset();
public:
    CEntityOwned(int EntityId, int ItemId, CCharacter* pCharacter, int ExtraIds = 0, vec2 Pos = vec2(0, 0), int ProximityRadius = 0);

    CPlayer* Player() const { return m_Player; }
    CCharacter* Character() const { return m_Player ? m_Player->GetCharacter() : nullptr; }

    virtual void Tick() override;
    virtual void Reset() override;

    virtual int Team();
    CClientMask TeamMask();
};

#endif
