// Made by qxdFox
#ifndef GAME_SERVER_ENTITIES_SHEEP_LIGHTSABER_H
#define GAME_SERVER_ENTITIES_SHEEP_LIGHTSABER_H

#include <base/vmath.h>
#include <game/server/entity.h>
#include <game/server/gameworld.h>

constexpr float LIGHTSABER_SPEED = 10.0f;
constexpr float LIGHTSABER_MAX_LENGTH = 220.0f;

class CLightsaber : public CEntity
{
	vec2 m_From;
	vec2 m_To;

	float m_Length = 0;

	int m_Owner;

	enum States
	{
		STATE_RETRACTED = 0,
		STATE_RETRACTING,
		STATE_EXTENDING,
		STATE_EXTENDED,
	};

	int m_State = 0;

public:
	CLightsaber(CGameWorld *pGameWorld, int Owner, vec2 Pos);

	void OnFire();

	virtual void Reset() override;
	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;
};

#endif
