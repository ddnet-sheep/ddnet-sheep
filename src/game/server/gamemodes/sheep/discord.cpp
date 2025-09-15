/* (c) Antonio Ianzano. See license.txt and the readme.txt in the root of the distribution for more information. */
#include "sheep.h"
#include <engine/shared/config.h>
#include <engine/server/server.h>
#include <game/server/gamecontext.h>

void CGameControllerSheep::DiscordInit() {
	m_DiscordBot = new dpp::cluster(g_Config.m_SvSheepDiscordToken, dpp::i_default_intents | dpp::i_message_content);
	m_DiscordBot->start(dpp::st_return);

	m_DiscordBot->on_message_create([this](const dpp::message_create_t &event) {
		std::string channelName;

		if(event.msg.author.id == m_DiscordBot->me.id) {
			return;
		}

		if(event.msg.channel_id != dpp::snowflake(g_Config.m_SvSheepDiscordServerChannelId)) {
			return;
		}

		CFakePlayerMessage Message;
		str_copy(Message.pName, event.msg.author.global_name.c_str());
		str_copy(Message.pMessage, event.msg.content.c_str());
		m_FakePlayerMessageQueue.push_back(Message);
	});
}

void CGameControllerSheep::DiscordShutdown() {
    if(m_DiscordBot != nullptr)
	{
		m_DiscordBot->shutdown();
		delete m_DiscordBot;
	}
}

void CGameControllerSheep::ConChainSheepDiscordTokenChange(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	CServer *pSelf = (CServer *)pUserData;
    CGameContext *pGameServer = (CGameContext *)pSelf->GameServer();
    CGameControllerSheep *pController = (CGameControllerSheep *)pGameServer->m_pController;

	if(pResult->NumArguments())
	{
        pfnCallback(pResult, pCallbackUserData);
        // TODO: fix
		pController->DiscordShutdown();
	    pController->DiscordInit();
	}
}

void CGameControllerSheep::SendDiscordChat(int ChatterClientId, int Team, const char *pText, int SpamProtectionClientId, int VersionFlags) {
    char aBuf[256]; 
	bool isDiscordMessage = str_startswith(pText, "[DC]");
	if (!isDiscordMessage) {
		str_format(aBuf, sizeof(aBuf), "__*%s*__", pText);
	}

	if(ChatterClientId >= 0 && ChatterClientId < MAX_CLIENTS) {
		const char* pUsername = Server()->ClientName(ChatterClientId);
		str_format(aBuf, sizeof(aBuf), "**%s > **%s", pUsername, pText);
	}

	if(!isDiscordMessage) {
		m_DiscordBot->message_create(dpp::message(dpp::snowflake(g_Config.m_SvSheepDiscordServerChannelId), aBuf), [this](const dpp::confirmation_callback_t &event) {
			if(event.is_error())
			{
				log_error("discord", "Failed to transmit message. Reason: %s", event.get_error().message.c_str());
			}
		});
	}
}
