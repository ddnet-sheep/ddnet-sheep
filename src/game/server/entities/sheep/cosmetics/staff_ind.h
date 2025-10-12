#ifndef GAME_SERVER_FOXNET_COSMETICS_STAFF_IND_H
#define GAME_SERVER_FOXNET_COSMETICS_STAFF_IND_H

#include <game/server/entities/sheep/entity_owned.h>

#include <base/vmath.h>
#include <engine/shared/protocol.h>
#include <game/server/gameworld.h>

class CStaffInd : public CEntityOwned
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
	CStaffInd(CCharacter* pCharacter);

	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;
};

#endif // GAME_SERVER_FOXNET_COSMETICS_STAFF_IND_H
