/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_H
#define GAME_SERVER_GAMEMODES_SHEEP_H

#include <vector>


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

#undef log_error
#include <dpp/dpp.h>
#define log_error(sys, ...) log_log(LEVEL_ERROR, sys, __VA_ARGS__)

struct CFakePlayerMessage {
	char pName[64];
	char pMessage[256];
	int ClientId = -1;
};

class CGameControllerSheep : public IGameController
{
public:
	CGameControllerSheep(class CGameContext *pGameServer);
	~CGameControllerSheep();

	// weapon drops
	std::vector<CWeaponDrop*> m_vWeaponDrops = {};

	// functions
	void DiscordInit();
	void DiscordShutdown();
	void SendDiscordChat(int ChatterClientId, int Team, const char *pText, int SpamProtectionClientId, int VersionFlags);
	
	
	void LoadItems();
	static bool ExecuteLoadItems(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	void LoadAccountItem(class CPlayer* pPlayer);
	static bool ExecuteLoadAccountItem(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	
	// database
    static bool ExecuteLogin(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool ExecuteRegister(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool ExecutePassword(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	
	// user commands
	static void ConLogin(IConsole::IResult *pResult, void *pUserData);
	static void ConRegister(IConsole::IResult *pResult, void *pUserData);
	static void ConPassword(IConsole::IResult *pResult, void *pUserData);
    static void ConLogout(IConsole::IResult *pResult, void *pUserData);
	
	// admin commands
	static void ConIgnoreInvisible(IConsole::IResult *pResult, void *pUserData);
	static void ConVanish(IConsole::IResult *pResult, void *pUserData);
	static void ConInvisible(IConsole::IResult *pResult, void *pUserData);
	
	// static void ConSync(IConsole::IResult *pResult, void *pUserData);
	static void ConForceLogout(IConsole::IResult *pResult, void *pUserData);
	static void ConForceLogin(IConsole::IResult *pResult, void *pUserData);
	static void ConWeapon(IConsole::IResult *pResult, void *pUserData);
	
	// chains
	static void ConChainSheepDiscordTokenChange(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	
	// sheep hooks
	void OnPlayerLogin(CPlayer *pPlayer);
	void PostPlayerLogin(CPlayer *pPlayer);
	void OnPlayerLogout(CPlayer *pPlayer, const char *pReason);
	void PostPlayerLogout(CPlayer *pPlayer, const char *pReason);
	
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

	void OnReset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	void DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg = true) override;

	// client bound
	CPortal *m_pPortals[MAX_CLIENTS] = {};
	CLightsaber *m_pLightsabers[MAX_CLIENTS] = {};
private:
	// server bound
    dpp::cluster *m_DiscordBot;
	std::shared_ptr<CItemsResult> m_ItemsResult;
	std::vector<CFakePlayerMessage> m_FakePlayerMessageQueue;

	// database
	CDbConnectionPool *m_pPool;
	CDbConnectionPool Pool();
};

#endif
