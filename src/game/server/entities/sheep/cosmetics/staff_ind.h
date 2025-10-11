#ifndef GAME_SERVER_FOXNET_COSMETICS_STAFF_IND_H
#define GAME_SERVER_FOXNET_COSMETICS_STAFF_IND_H

#include <game/server/entities/sheep/cosmetics/cosmetic.h>

#include <base/vmath.h>
#include <engine/shared/protocol.h>
#include <game/server/gameworld.h>

class CStaffInd : public CCosmetic
{
	enum
	{
		BALL,
		ARMOR,
		BALL_FRONT,
		NUM_IDS
	};

	vec2 m_aPos[2];

	float m_Dist;
	bool m_BallFirst;

public:
	CStaffInd(CPlayer* Owner);

	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;
};

#endif // GAME_SERVER_FOXNET_COSMETICS_STAFF_IND_H
