// originally made by Loic KASSEL (Rei/ReiTw), updated by fokkonaut

#ifndef GAME_SERVER_ENTITIES_SPECIAL_LIGHTNINGLASER_H
#define GAME_SERVER_ENTITIES_SPECIAL_LIGHTNINGLASER_H

#include <game/server/entities/sheep/entity_owned.h>

#include <game/server/gamecontext.h>

class CLightningLaser : public CEntityOwned
{
	enum
	{
		POS_START = 0,
		POS_END,
		POS_COUNT
	};

	vec2 m_Dir;
	
	int m_Lifespan;
	int m_StartLifespan;
	
	float m_StartTick;

	// target
	struct STarget
	{
		bool m_IsAlive;
		int m_ID;
		vec2 m_Pos;
		bool m_BehindWall;

		void Reset()
		{
			m_IsAlive = false;
			m_ID = -1;
			m_Pos = vec2(0,0);
			m_BehindWall = false;
		}
	};

	STarget m_Target;

	vec2 **m_aaPositions;

	int m_Count;
	int m_Length;
	
public:
	CLightningLaser(CCharacter *pCharacter, vec2 Direction);
	~CLightningLaser();

	virtual void Tick();
	virtual void Snap(int SnappingClient);

	void InitTarget();
	void UpdateDirection(vec2 From);
	bool TargetBehindWall(vec2 From);
	void HitCharacter();
	void GenerateLights();

	bool TargetAlive() { return (m_Target.m_ID != -1 && GameServer()->GetPlayerChar(m_Target.m_ID)) ? true : false; }
};

#endif
