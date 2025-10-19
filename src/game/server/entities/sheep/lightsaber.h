// Made by qxdFox
#ifndef GAME_SERVER_ENTITIES_SHEEP_LIGHTSABER_H
#define GAME_SERVER_ENTITIES_SHEEP_LIGHTSABER_H

#include <game/server/entities/sheep/entity_owned.h>

#include <base/vmath.h>
#include <game/server/gameworld.h>

constexpr float LIGHTSABER_SPEED = 10.0f;
constexpr float LIGHTSABER_MAX_LENGTH = 220.0f;

class CLightsaber : public CEntityOwned {
	vec2 m_From;
	vec2 m_To;

	float m_Length = 0;

	enum States
	{
		STATE_RETRACTED = 0,
		STATE_RETRACTING,
		STATE_EXTENDING,
		STATE_EXTENDED,
	};

	int m_State = 0;

public:
	CLightsaber(CCharacter* pCharacter);

	void OnFire();

	virtual void Reset() override;
	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;
};

#endif
