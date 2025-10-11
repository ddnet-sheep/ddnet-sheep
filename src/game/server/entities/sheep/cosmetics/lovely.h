#ifndef GAME_SERVER_FOXNET_COSMETICS_LOVELY_H
#define GAME_SERVER_FOXNET_COSMETICS_LOVELY_H

#include <game/server/entities/sheep/cosmetics/cosmetic.h>

#include <base/vmath.h>
#include <engine/shared/protocol.h>
#include <game/server/gameworld.h>

class CLovely : public CCosmetic
{
	static const int MAX_HEARTS = 4;
	
	float m_SpawnDelay;

	struct SLovelyData
	{
		int m_Id;
		vec2 m_Pos;
		float m_Lifespan;
	};
	SLovelyData m_aData[MAX_HEARTS];
	void SpawnNewHeart();

public:
	CLovely(CPlayer* Owner);

	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;
};

#endif // GAME_SERVER_FOXNET_COSMETICS_LOVELY_H
