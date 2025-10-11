/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_H
#define GAME_SERVER_GAMEMODES_SHEEP_H

#include <vector>
#include <unordered_map>

#include <base/log.h>

#include <engine/server/server.h>
#include <engine/server/databases/connection.h>

#include <game/server/gamecontroller.h>

#include "item.h"
#include "vote.h"
#include "commands.h"

#include <game/server/entities/sheep/weapon_drop.h>
#include <game/server/entities/sheep/portal.h>
#include <game/server/entities/sheep/lightsaber.h>
#include <game/server/entities/sheep/powerup.h>

#include <game/server/gamemodes/sheep/message.h>

#undef log_error
#include <dpp/dpp.h>
#define log_error(sys, ...) log_log(LEVEL_ERROR, sys, __VA_ARGS__)

enum CAccountActions {
	ACTION_ENTER,
	ACTION_JOIN,
	ACTION_ENTER_AND_JOIN,
	ACTION_LOGOUT,
	ACTION_LEAVE
};

class CGameControllerSheep : public IGameController
{
public:
	CGameControllerSheep(class CGameContext *pGameServer);
	~CGameControllerSheep();

	
	// functions
	void DiscordInit();
	void DiscordShutdown();
	void SendDiscordChat(int ChatterClientId, int Team, const char *pText, int SpamProtectionClientId, int VersionFlags);
	
	void SendActionMessage(CPlayer *pPlayer, enum CAccountActions Action, char* pExtra = "");
	
	void LoadItems();
	void SpawnCosmetics(CPlayer *pPlayer);
	void DespawnCosmetics(CPlayer *pPlayer);
	void AuthPlayer(CPlayer *pPlayer);
	void SaveAccount(CPlayer* pPlayer);
	static bool ExecuteLoadItems(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	void LoadAccountItem(class CPlayer* pPlayer);
	static bool ExecuteLoadAccountItem(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	
	int CalcPlayerNeededExp(CPlayer *pPlayer);
	std::optional<vec2> GetRandomAccessablePos();

	void CreateLaserDeath(int Type, int pOwner, vec2 pPos, CClientMask pMask);

	// database
    static bool ExecuteLogin(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool ExecuteRegister(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool ExecutePassword(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool ExecuteSave(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	
	// user commands
	static void ConLogin(IConsole::IResult *pResult, void *pUserData);
	static void ConRegister(IConsole::IResult *pResult, void *pUserData);
	static void ConPassword(IConsole::IResult *pResult, void *pUserData);
    static void ConLogout(IConsole::IResult *pResult, void *pUserData);
	static void ConStats(IConsole::IResult *pResult, void *pUserData);
	
	static void ConLaserText(IConsole::IResult *pResult, void *pUserData);
	static void ConProjectileText(IConsole::IResult *pResult, void *pUserData);
	
	static void ConRainbowBody(IConsole::IResult *pResult, void *pUserData);
	static void ConRainbowFeet(IConsole::IResult *pResult, void *pUserData);
	static void ConRainbowSpeed(IConsole::IResult *pResult, void *pUserData);
	static void ConSparkle(IConsole::IResult *pResult, void *pUserData);
	static void ConDotTrail(IConsole::IResult *pResult, void *pUserData);
	static void ConStarTrail(IConsole::IResult *pResult, void *pUserData);
	static void ConInverseAim(IConsole::IResult *pResult, void *pUserData);
	static void ConLovely(IConsole::IResult *pResult, void *pUserData);
	static void ConRotatingBall(IConsole::IResult *pResult, void *pUserData);
	static void ConEpicCircle(IConsole::IResult *pResult, void *pUserData);
	static void ConBloody(IConsole::IResult *pResult, void *pUserData);
	static void ConHeartHat(IConsole::IResult *pResult, void *pUserData);
	static void ConStaffInd(IConsole::IResult *pResult, void *pUserData);
	static void ConDeathEffect(IConsole::IResult *pResult, void *pUserData);
	static void ConDamageIndEffect(IConsole::IResult *pResult, void *pUserData);
	
	// admin commands
	static void ConIgnoreInvisible(IConsole::IResult *pResult, void *pUserData);
	static void ConVanish(IConsole::IResult *pResult, void *pUserData);
	static void ConInvisible(IConsole::IResult *pResult, void *pUserData);
	
	static void ConGiveExp(IConsole::IResult *pResult, void *pUserData);
	static void ConSetMoney(IConsole::IResult *pResult, void *pUserData);
	static void ConSetLevel(IConsole::IResult *pResult, void *pUserData);
	static void ConSetVip(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTitle(IConsole::IResult *pResult, void *pUserData);
	static void ConSetStaff(IConsole::IResult *pResult, void *pUserData);
	
	static void ConRedirect(IConsole::IResult *pResult, void *pUserData);
	
	// static void ConSync(IConsole::IResult *pResult, void *pUserData);
	static void ConForceLogout(IConsole::IResult *pResult, void *pUserData);
	static void ConForceLogin(IConsole::IResult *pResult, void *pUserData);
	static void ConWeapon(IConsole::IResult *pResult, void *pUserData);
	
	// chains
	static void ConChainSheepDiscordTokenChange(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	
	// sheep hooks
	void OnPlayerLogin(CPlayer *pPlayer, bool Autologin);
	void OnPlayerLogout(CPlayer *pPlayer, const char *pReason, bool Silent);
	
	int CalcPlayerExpPerMinute(CPlayer *pPlayer);
	void GivePlayerMoney(CPlayer* pPlayer, int64_t Amount, const char *pReason);
	void GivePlayerExp(CPlayer *pPlayer, int Exp, char* pReason = "");
	void GivePlayerPlaytime(CPlayer *pPlayer, int Minutes);
	
	bool OnCharacterPowerup(CCharacter *pChr, const SPowerupData *pData);
	
	// sheep lowlevel passthrough hooks
	bool IncludedInServerInfo(CPlayer* pPlayer);
	
	// custom ddnet hooks
	void SendChat(int ChatterClientId, int Team, const char *pText, int SpamProtectionClientId, int VersionFlags);
	void OnPostGlobalSnap();
	
	void OnPlayerTick(CPlayer *pPlayer);
	
	void OnCharacterTick(CCharacter *pCharacter);
	void OnCharacterVote(CCharacter *pPlayer, EVoteButton Button);
	bool OnCharacterWeaponFire(CCharacter *pCharacter, int Weapon, vec2 MouseTarget, vec2 Direction, vec2 ProjStartPos);
	void OnCharacterWeaponDrop(CCharacter *pCharacter, int Type, vec2 Vel, bool Death);
	void OnCharacterWeaponChanged(CCharacter *pCharacter);
	
	// ddnet
	CScore *Score();
	
	void HandleCharacterTiles(class CCharacter *pChr, int MapIndex) override;
	void SetArmorProgress(CCharacter *pCharacter, int Progress) override;
	
	void OnPlayerConnect(class CPlayer *pPlayer) override;
	void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason) override;
	bool OnEntity(int Index, int x, int y, int Layer, int Flags, bool Initial, int Number) override;
	
	void OnReset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;
	
	void DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg = true) override;
	
	// client bound
	CPortal *m_pPortals[MAX_CLIENTS] = {};
	CLightsaber *m_pLightsabers[MAX_CLIENTS] = {};
	int m_RainbowColor[MAX_CLIENTS] = {};
	std::unordered_map<CEntity*, CCharacter*> m_vGravityTarget;
	std::unordered_map<CPlayer*, std::unordered_map<EItemVariant, CEntity*>> m_Cosmetics; // todo: make this better, i dont like EItemVariant as key here, maybe use 
	
	// server bound
	std::vector<CWeaponDrop*> m_vWeaponDrops = {};
	std::vector<CPowerUp *> m_vPowerups;
	int64_t m_PowerupDelay;
	
private:
	// server bound
    dpp::cluster *m_DiscordBot = nullptr;
	std::shared_ptr<CItemsResult> m_ItemsResult;
	std::vector<CFakePlayerMessage> m_FakePlayerMessageQueue;
	
	std::vector<SLaserDeath> m_vLaserDeaths;

	// database
	CDbConnectionPool *m_pPool;
	CDbConnectionPool Pool();
};

#endif
