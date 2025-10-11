#ifndef GAME_SERVER_FOXNET_COSMETICS_EPIC_CIRCLE_H
#define GAME_SERVER_FOXNET_COSMETICS_EPIC_CIRCLE_H

#include <game/server/entities/sheep/cosmetics/cosmetic.h>

#include <base/vmath.h>
#include <engine/shared/protocol.h>
#include <game/server/gameworld.h>
#include <game/server/entity.h>

class CEpicCircle : public CCosmetic
{
	static const int MAX_PARTICLES = 9;
	vec2 m_RotatePos[MAX_PARTICLES];

public:
	CEpicCircle(CPlayer* Owner);

	void Tick() override;
	virtual void Snap(int SnappingClient) override;
};

#endif // GAME_SERVER_FOXNET_COSMETICS_EPIC_CIRCLE_H
