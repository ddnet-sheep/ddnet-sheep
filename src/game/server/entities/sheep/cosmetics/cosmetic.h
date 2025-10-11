#ifndef GAME_SERVER_ENTITIES_SHEEP_COSMETICS_COSMETIC_H
#define GAME_SERVER_ENTITIES_SHEEP_COSMETICS_COSMETIC_H

#include <engine/server.h>
#include <game/server/entity.h>
#include <game/server/player.h>
#include <game/server/gamemodes/sheep/sheep.h>

class CCosmetic : public CEntity
{    
protected:
    CPlayer* m_Player;
    EItemVariant m_ItemVariant;
    std::vector<int> m_SnapIds;

    bool HasReset();
    CGameControllerSheep* Controller() { return (CGameControllerSheep *)GameServer()->m_pController; }
public:
    CCosmetic(CGameWorld *pGameWorld, int EntityId, EItemVariant ItemVariant, CPlayer* Owner, vec2 Pos, int ExtraIds);

    CPlayer* Player() const { return m_Player; }
    CCharacter* Character() const { return m_Player ? m_Player->GetCharacter() : nullptr; }

    void Tick() override;
    void Reset() override;
};

#endif
