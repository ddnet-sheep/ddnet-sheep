/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_COSMETICS_H
#define GAME_SERVER_GAMEMODES_SHEEP_COSMETICS_H

// enum EAreas
// {
// 	AREA_GAME = 0,
// 	AREA_ROULETTE = 1,
// 	NUM_AREAS
// };

enum
{
    COSMETIC_HEART_HAT,
    COSMETIC_LOVELY,
    COSMETIC_DOT_TRAIL,
    COSMETIC_STAFF_IND,
    COSMETIC_ROTATING_BALL,
    COSMETIC_EPIC_CIRCLE,
    COSMETIC_SPARKLE,
    COSMETIC_BLOODY,
    COSMETIC_BLOODY_STRONG,
    COSMETIC_RAINBOW_BODY,
    COSMETIC_RAINBOW_FEET,
    COSMETIC_DEATH_EFFECT,
    NUM_COSMETICS
};

enum
{
    COSMETIC_STATE_OFF,
    COSMETIC_STATE_DEFAULT
};

struct CCosmetics
{
	static int GetId(const char* Type) {
        if(!str_comp_nocase(Type, "hearthat")) return COSMETIC_HEART_HAT;
        if(!str_comp_nocase(Type, "lovely")) return COSMETIC_LOVELY;
        if(!str_comp_nocase(Type, "dottrail")) return COSMETIC_DOT_TRAIL;
        if(!str_comp_nocase(Type, "staffind")) return COSMETIC_STAFF_IND;
        if(!str_comp_nocase(Type, "rotatingball")) return COSMETIC_ROTATING_BALL;
        if(!str_comp_nocase(Type, "epiccircle")) return COSMETIC_EPIC_CIRCLE;
        if(!str_comp_nocase(Type, "sparkle")) return COSMETIC_SPARKLE;
        if(!str_comp_nocase(Type, "bloody")) return COSMETIC_BLOODY;
        if(!str_comp_nocase(Type, "bloodystrong")) return COSMETIC_BLOODY_STRONG;
        if(!str_comp_nocase(Type, "rainbowbody")) return COSMETIC_RAINBOW_BODY;
        if(!str_comp_nocase(Type, "rainbowfeet")) return COSMETIC_RAINBOW_FEET;
        if(!str_comp_nocase(Type, "deatheffect")) return COSMETIC_DEATH_EFFECT;
        return -1;
    }

    static const char* GetName(int Type) {
        switch(Type) {
            case COSMETIC_HEART_HAT: return "HeartHat";
            case COSMETIC_LOVELY: return "Lovely";
            case COSMETIC_DOT_TRAIL: return "DotTrail";
            case COSMETIC_STAFF_IND: return "StaffInd";
            case COSMETIC_ROTATING_BALL: return "RotatingBall";
            case COSMETIC_EPIC_CIRCLE: return "EpicCircle";
            case COSMETIC_SPARKLE: return "Sparkle";
            case COSMETIC_BLOODY: return "Bloody";
            case COSMETIC_BLOODY_STRONG: return "BloodyStrong";
            case COSMETIC_RAINBOW_BODY: return "RainbowBody";
            case COSMETIC_RAINBOW_FEET: return "RainbowFeet";
            case COSMETIC_DEATH_EFFECT: return "DeathEffect";
            default: return "None";
        }
    }

	// todo: remove
	// int m_HookPower = 0;
	// bool m_InverseAim = false;
	
	// int m_DamageIndType = 0;

	// // Guns
	// int m_EmoticonGun = 0;
	// bool m_ConfettiGun = false;
	// bool m_PhaseGun = false;
	
	// bool m_PickupPet = false;
};

struct CCosmeticsResult : ISheepSqlResult
{
    int m_State[NUM_COSMETICS];
};

struct CSqlCosmeticsRequest : ISqlData
{
	CSqlCosmeticsRequest(std::shared_ptr<ISqlResult> pResult) :
		ISqlData(std::move(pResult))
	{
	}

	int m_AccountId;
    int m_State[NUM_COSMETICS];
};


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

#endif
