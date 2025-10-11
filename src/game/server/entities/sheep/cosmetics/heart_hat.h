// Made by qxdFox
#ifndef GAME_SERVER_FOXNET_COSMETICS_HEARTHAT_H
#define GAME_SERVER_FOXNET_COSMETICS_HEARTHAT_H

#include <game/server/entities/sheep/cosmetics/cosmetic.h>

#include <game/server/entity.h>
#include <game/server/gameworld.h>

#include <base/vmath.h>

class CHeartHat : public CCosmetic
{
	enum
	{
		HEART_BACK = 0,
		HEART_MIDDLE = 1,
		HEART_FRONT = 2,
		NUM_HEARTS = 3
	};

	float m_Dist[NUM_HEARTS];
	bool m_switch;
	vec2 m_aPos[NUM_HEARTS];

public:
	CHeartHat(CPlayer* Owner);
	
	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;
};

#endif // GAME_SERVER_FOXNET_COSMETICS_HEARTHAT_H
