// Made by qxdFox
#ifndef GAME_SERVER_FOXNET_COSMETICS_DOT_TRAIL_H
#define GAME_SERVER_FOXNET_COSMETICS_DOT_TRAIL_H

#include <game/server/entities/sheep/cosmetics/cosmetic.h>

#include <game/server/gameworld.h>
#include <base/vmath.h>

class CDotTrail : public CCosmetic
{
public:
	CDotTrail(CPlayer* Owner);
	virtual void Snap(int SnappingClient) override;
};

#endif // GAME_SERVER_FOXNET_COSMETICS_DOT_TRAIL_H
