// Made by qxdFox
#ifndef GAME_SERVER_FOXNET_COSMETICS_DOT_TRAIL_H
#define GAME_SERVER_FOXNET_COSMETICS_DOT_TRAIL_H

#include <game/server/entities/sheep/entity_owned.h>

#include <game/server/gameworld.h>
#include <base/vmath.h>

class CDotTrail : public CEntityOwned
{
public:
	CDotTrail(CCharacter* pCharacter);
	virtual void Snap(int SnappingClient) override;
};

#endif // GAME_SERVER_FOXNET_COSMETICS_DOT_TRAIL_H
