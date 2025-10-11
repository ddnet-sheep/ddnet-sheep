// enum EAreas
// {
// 	AREA_GAME = 0,
// 	AREA_ROULETTE = 1,
// 	NUM_AREAS
// };

enum EHookTypes
{
	HOOK_NORMAL = 0,
	HOOK_BLOODY,
	HOOK_RAINBOW,
	NUM_HOOKS
};

enum EIndicators
{
	IND_NONE = 0,
	IND_CLOCKWISE,
	IND_COUNTERWISE,
	IND_INWARD,
	IND_OUTWARD,
	IND_LINE,
	IND_CRISSCROSS,
	NUM_DAMAGE_IND
};

enum EDeathEffects
{
	DEATH_NONE = 0,
	DEATH_HAMMERHIT,
	DEATH_EXPLOSION,
	DEATH_DAMAGEIND,
	DEATH_LASER,
	NUM_DEATHS
};

struct SLaserDeath {
    std::vector<int> m_vIds;

    int m_Owner;
    int m_Remaining;
    vec2 m_Pos;
    std::vector<vec2> m_vFrom;
    std::vector<vec2> m_vTo;
    std::vector<int64_t> m_vStartTick;
    int64_t m_EndTick;
    CClientMask m_Mask;
    int m_Sound;
};

enum ETrailTypes
{
	TRAIL_NONE = 0,
	TRAIL_STAR,
	TRAIL_DOT,
	NUM_TRAILS
};

struct CCosmetics
{
	int m_HookPower = 0;
	bool m_InverseAim = false;
	
	int m_DamageIndType = 0;

	// Guns
	int m_EmoticonGun = 0;
	bool m_ConfettiGun = false;
	bool m_PhaseGun = false;
	
	bool m_PickupPet = false;
};
