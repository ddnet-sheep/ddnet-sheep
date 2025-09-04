/* (c) Antonio Ianzano. See licence.txt and the readme.txt in the root of the distribution for more information. */
#include "sheep.h"

#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/version.h>

#include <engine/server/server.h>

#define GAME_TYPE_NAME "DDraceNetwork"

CGameControllerSheep::CGameControllerSheep(class CGameContext *pGameServer) :
	IGameController(pGameServer),
	m_pPool(((CServer *)Server())->DbPool())
{

	/*
		CREATE TABLE `sheep_accounts` ( 
			`id` INT AUTO_INCREMENT NOT NULL PRIMARY KEY,
			`name` VARCHAR(32) NOT NULL UNIQUE,
			`password` char(64) NOT NULL,
			`ban_expiration` INT UNSIGNED NOT NULL DEFAULT 0 ,
			`level` INT UNSIGNED NOT NULL DEFAULT 0 ,
			`exp` INT NOT NULL DEFAULT 0 ,
			`vip` INT UNSIGNED NOT NULL DEFAULT 0 ,
			`vip_expiration` INT NOT NULL DEFAULT 0 ,
			`staff_level` INT UNSIGNED NOT NULL DEFAULT 0 ,
			`email` VARCHAR(255) NULL,
			`email_verified` TINYINT NOT NULL DEFAULT 0
		);

		CREATE TABLE `sheep_items` ( 
			`id` INT AUTO_INCREMENT NOT NULL PRIMARY KEY,
			`name` VARCHAR(32) NOT NULL UNIQUE,
			`description` VARCHAR(255) NOT NULL DEFAULT ""
		);

		CREATE TABLE `sheep_account_item` ( 
			`account_id` INT NOT NULL,
			`item_id` INT NOT NULL,
			`amount` INT NOT NULL DEFAULT 1,
			PRIMARY KEY (`account_id`, `item_id`),
			CONSTRAINT `fk_account_id` FOREIGN KEY (`account_id`) REFERENCES `sheep_accounts` (`id`) ON DELETE CASCADE ON UPDATE NO ACTION,
			CONSTRAINT `fk_item_id` FOREIGN KEY (`item_id`) REFERENCES `sheep_items` (`id`) ON DELETE CASCADE ON UPDATE NO ACTION
		)
	*/

	m_pGameType = GAME_TYPE_NAME;
	m_GameFlags = protocol7::GAMEFLAG_RACE;

	DiscordInit();

	GameServer()->Console()->Register("login", "s[account name] s[password]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogin, GameServer(), "Logs you into your account");
	GameServer()->Console()->Register("register", "s[account name] s[password]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRegister, GameServer(), "Registers a new account");
	GameServer()->Console()->Register("password", "s[old password] s[new password]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPassword, GameServer(), "Changes the password");
	GameServer()->Console()->Register("logoff", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogout, GameServer(), "Logs you out of your your account");

	GameServer()->Console()->Chain("sv_sheep_discord_token", ConChainSheepDiscordTokenChange, this);
}

CGameControllerSheep::~CGameControllerSheep() {
	DiscordShutdown();
}

CScore *CGameControllerSheep::Score()
{
	return GameServer()->Score();
}

void CGameControllerSheep::HandleCharacterTiles(CCharacter *pChr, int MapIndex)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	const int ClientId = pPlayer->GetCid();

	int TileIndex = GameServer()->Collision()->GetTileIndex(MapIndex);
	int TileFIndex = GameServer()->Collision()->GetFrontTileIndex(MapIndex);

	//Sensitivity
	int S1 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x + pChr->GetProximityRadius() / 3.f, pChr->GetPos().y - pChr->GetProximityRadius() / 3.f));
	int S2 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x + pChr->GetProximityRadius() / 3.f, pChr->GetPos().y + pChr->GetProximityRadius() / 3.f));
	int S3 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x - pChr->GetProximityRadius() / 3.f, pChr->GetPos().y - pChr->GetProximityRadius() / 3.f));
	int S4 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x - pChr->GetProximityRadius() / 3.f, pChr->GetPos().y + pChr->GetProximityRadius() / 3.f));
	int Tile1 = GameServer()->Collision()->GetTileIndex(S1);
	int Tile2 = GameServer()->Collision()->GetTileIndex(S2);
	int Tile3 = GameServer()->Collision()->GetTileIndex(S3);
	int Tile4 = GameServer()->Collision()->GetTileIndex(S4);
	int FTile1 = GameServer()->Collision()->GetFrontTileIndex(S1);
	int FTile2 = GameServer()->Collision()->GetFrontTileIndex(S2);
	int FTile3 = GameServer()->Collision()->GetFrontTileIndex(S3);
	int FTile4 = GameServer()->Collision()->GetFrontTileIndex(S4);

	const ERaceState PlayerDDRaceState = pChr->m_DDRaceState;
	bool IsOnStartTile = (TileIndex == TILE_START) || (TileFIndex == TILE_START) || FTile1 == TILE_START || FTile2 == TILE_START || FTile3 == TILE_START || FTile4 == TILE_START || Tile1 == TILE_START || Tile2 == TILE_START || Tile3 == TILE_START || Tile4 == TILE_START;
	// start
	if(IsOnStartTile && PlayerDDRaceState != ERaceState::CHEATED)
	{
		const int Team = GameServer()->GetDDRaceTeam(ClientId);
		if(Teams().GetSaving(Team))
		{
			GameServer()->SendStartWarning(ClientId, "You can't start while loading/saving of team is in progress");
			pChr->Die(ClientId, WEAPON_WORLD);
			return;
		}
		if(g_Config.m_SvTeam == SV_TEAM_MANDATORY && (Team == TEAM_FLOCK || Teams().Count(Team) <= 1))
		{
			GameServer()->SendStartWarning(ClientId, "You have to be in a team with other tees to start");
			pChr->Die(ClientId, WEAPON_WORLD);
			return;
		}
		if(g_Config.m_SvTeam != SV_TEAM_FORCED_SOLO && Team > TEAM_FLOCK && Team < TEAM_SUPER && Teams().Count(Team) < g_Config.m_SvMinTeamSize && !Teams().TeamFlock(Team))
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "Your team has fewer than %d players, so your team rank won't count", g_Config.m_SvMinTeamSize);
			GameServer()->SendStartWarning(ClientId, aBuf);
		}
		if(g_Config.m_SvResetPickups)
		{
			pChr->ResetPickups();
		}

		Teams().OnCharacterStart(ClientId);
		pChr->m_LastTimeCp = -1;
		pChr->m_LastTimeCpBroadcasted = -1;
		for(float &CurrentTimeCp : pChr->m_aCurrentTimeCp)
		{
			CurrentTimeCp = 0.0f;
		}
	}

	// finish
	if(((TileIndex == TILE_FINISH) || (TileFIndex == TILE_FINISH) || FTile1 == TILE_FINISH || FTile2 == TILE_FINISH || FTile3 == TILE_FINISH || FTile4 == TILE_FINISH || Tile1 == TILE_FINISH || Tile2 == TILE_FINISH || Tile3 == TILE_FINISH || Tile4 == TILE_FINISH) && PlayerDDRaceState == ERaceState::STARTED)
		Teams().OnCharacterFinish(ClientId);

	// unlock team
	else if(((TileIndex == TILE_UNLOCK_TEAM) || (TileFIndex == TILE_UNLOCK_TEAM)) && Teams().TeamLocked(GameServer()->GetDDRaceTeam(ClientId)))
	{
		Teams().SetTeamLock(GameServer()->GetDDRaceTeam(ClientId), false);
		GameServer()->SendChatTeam(GameServer()->GetDDRaceTeam(ClientId), "Your team was unlocked by an unlock team tile");
	}

	// solo part
	if(((TileIndex == TILE_SOLO_ENABLE) || (TileFIndex == TILE_SOLO_ENABLE)) && !Teams().m_Core.GetSolo(ClientId))
	{
		GameServer()->SendChatTarget(ClientId, "You are now in a solo part");
		pChr->SetSolo(true);
	}
	else if(((TileIndex == TILE_SOLO_DISABLE) || (TileFIndex == TILE_SOLO_DISABLE)) && Teams().m_Core.GetSolo(ClientId))
	{
		GameServer()->SendChatTarget(ClientId, "You are now out of the solo part");
		pChr->SetSolo(false);
	}
}

void CGameControllerSheep::SetArmorProgress(CCharacter *pCharacter, int Progress)
{
	pCharacter->SetArmor(std::clamp(10 - (Progress / 15), 0, 10));
}

void CGameControllerSheep::OnPlayerConnect(CPlayer *pPlayer)
{
	IGameController::OnPlayerConnect(pPlayer);
	int ClientId = pPlayer->GetCid();

	// init the player
	Score()->PlayerData(ClientId)->Reset();

	// Can't set score here as LoadScore() is threaded, run it in
	// LoadScoreThreaded() instead
	Score()->LoadPlayerData(ClientId);

	if(!Server()->ClientPrevIngame(ClientId))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s", Server()->ClientName(ClientId), GetTeamName(pPlayer->GetTeam()));
		GameServer()->SendChat(-1, TEAM_ALL, aBuf, -1, CGameContext::FLAG_SIX);
	}
}

void CGameControllerSheep::OnPlayerDisconnect(CPlayer *pPlayer, const char *pReason)
{
	int ClientId = pPlayer->GetCid();
	bool WasModerator = pPlayer->m_Moderating && Server()->ClientIngame(ClientId);

	IGameController::OnPlayerDisconnect(pPlayer, pReason);

	if(!GameServer()->PlayerModerating() && WasModerator)
		GameServer()->SendChat(-1, TEAM_ALL, "Server kick/spec votes are no longer actively moderated.");

	if(g_Config.m_SvTeam != SV_TEAM_FORCED_SOLO)
		Teams().SetForceCharacterTeam(ClientId, TEAM_FLOCK);

	for(int Team = TEAM_FLOCK + 1; Team < TEAM_SUPER; Team++)
		if(Teams().IsInvited(Team, ClientId))
			Teams().SetClientInvited(Team, ClientId, false);
}

void CGameControllerSheep::OnReset()
{
	IGameController::OnReset();
	Teams().Reset();
}

void CGameControllerSheep::Tick()
{
	IGameController::Tick();
	Teams().ProcessSaveTeam();
	Teams().Tick();
}

void CGameControllerSheep::DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg)
{
	Team = ClampTeam(Team);
	if(Team == pPlayer->GetTeam())
		return;

	CCharacter *pCharacter = pPlayer->GetCharacter();

	if(Team == TEAM_SPECTATORS)
	{
		if(g_Config.m_SvTeam != SV_TEAM_FORCED_SOLO && pCharacter)
		{
			// Joining spectators should not kill a locked team, but should still
			// check if the team finished by you leaving it.
			int DDRTeam = pCharacter->Team();
			Teams().SetForceCharacterTeam(pPlayer->GetCid(), TEAM_FLOCK);
			Teams().CheckTeamFinished(DDRTeam);
		}
	}

	IGameController::DoTeamChange(pPlayer, Team, DoChatMsg);
}

void CGameControllerSheep::SendChat(int ChatterClientId, int Team, const char *pText, int SpamProtectionClientId, int VersionFlags) {
	SendDiscordChat(ChatterClientId, Team, pText, SpamProtectionClientId, VersionFlags);
}

void CGameControllerSheep::TickPlayer(CPlayer *pPlayer) {
	if (pPlayer->m_PasswordChangeSuccessResult != nullptr && pPlayer->m_PasswordChangeSuccessResult->m_Completed) {
		if (pPlayer->m_PasswordChangeSuccessResult->m_Success) {
			GameServer()->SendChatTarget(pPlayer->GetCid(), "Password changed successfully.");
		} else {
			GameServer()->SendChatTarget(pPlayer->GetCid(), "Failed to change password.");
		}
		pPlayer->m_PasswordChangeSuccessResult = nullptr;
	}

	if (pPlayer->m_AccountLoginResult != nullptr && pPlayer->m_AccountLoginResult->m_Completed && !pPlayer->m_AccountLoginResult->m_Processed) {
		GameServer()->SendChatTarget(pPlayer->GetCid(), pPlayer->m_AccountLoginResult->m_Message);
		if (pPlayer->m_AccountLoginResult->m_Success) {
			pPlayer->m_AccountLoginResult->m_Processed = true;
		} else {
			pPlayer->m_AccountLoginResult = nullptr;
		}
	}
}