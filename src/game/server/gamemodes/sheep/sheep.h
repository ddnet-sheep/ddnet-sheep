/* (c) Antonio Ianzano. See licence.txt and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_SHEEP_H
#define GAME_SERVER_GAMEMODES_SHEEP_H

#include <base/log.h>

#include <game/server/gamecontroller.h>
#include <engine/server/databases/connection.h>

#undef log_error
#include <dpp/dpp.h>
#define log_error(sys, ...) log_log(LEVEL_ERROR, sys, __VA_ARGS__)

class CGameControllerSheep : public IGameController
{
public:
	CGameControllerSheep(class CGameContext *pGameServer);
	~CGameControllerSheep();

	// discord
	void DiscordInit();
	void DiscordShutdown();
	static void ConChainSheepDiscordTokenChange(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	void SendDiscordChat(int ChatterClientId, int Team, const char *pText, int SpamProtectionClientId, int VersionFlags);

	// database
    static bool ExecuteLogin(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool ExecuteRegister(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool ExecutePassword(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);

	// commands
	static void ConLogin(IConsole::IResult *pResult, void *pUserData);
	static void ConRegister(IConsole::IResult *pResult, void *pUserData);
	static void ConPassword(IConsole::IResult *pResult, void *pUserData);
    static void ConLogout(IConsole::IResult *pResult, void *pUserData);
	
	// custom hooks
	void SendChat(int ChatterClientId, int Team, const char *pText, int SpamProtectionClientId, int VersionFlags);
	void TickPlayer(CPlayer *pPlayer);

	// ddnet
	CScore *Score();

	void HandleCharacterTiles(class CCharacter *pChr, int MapIndex) override;
	void SetArmorProgress(CCharacter *pCharacter, int Progress) override;

	void OnPlayerConnect(class CPlayer *pPlayer) override;
	void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason) override;

	void OnReset() override;
	void Tick() override;

	void DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg = true) override;
private:
	// discord
    dpp::cluster *m_DiscordBot;

	// database
	CDbConnectionPool *m_pPool;
	CDbConnectionPool Pool();
};
#endif // GAME_SERVER_GAMEMODES_SHEEP_H
