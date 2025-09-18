#include <game/server/player.h>
#include <game/server/gamecontext.h>

class CCommands {
public:
    static CPlayer *GetCaller(IConsole::IResult *pResult, void *pUserData) {
        if(!CheckClientId(pResult->m_ClientId))
            return nullptr;

        CGameContext *pGameServer = (CGameContext *)pUserData;
        return pGameServer->m_apPlayers[pResult->m_ClientId];
    }

    static bool ValidateAuthenticated(IConsole::IResult *pResult, void *pUserData) {
        return ValidateAuthenticated(GetCaller(pResult, pUserData));
    }

    static bool ValidateAuthenticated(CPlayer *pPlayer) {
        return pPlayer && pPlayer->m_AccountLoginResult;
    }

    static bool ValidateStaff(IConsole::IResult *pResult, void *pUserData, int Level) {
        return ValidateStaff(GetCaller(pResult, pUserData), Level);
    }

    static bool ValidateStaff(CPlayer *pPlayer, int Level) {
        return ValidateAuthenticated(pPlayer) && pPlayer->m_AccountLoginResult->m_Staff >= Level;
    }

    static CPlayer* ParseVictim(char* pStr, void *pUserData) {
        CGameContext* pGameServer = (CGameContext*)pUserData;

        for(int ClientId = 0; ClientId < MAX_CLIENTS; ClientId++)
            if(pGameServer->Server()->ClientIngame(ClientId) && str_comp(pStr, pGameServer->Server()->ClientName(ClientId)) == 0)
                return pGameServer->m_apPlayers[ClientId];

        return nullptr;
    }

	static CPlayer *GetVictimOrCaller(IConsole::IResult *pResult, void *pUserData, int MaxArgs = 1) {
        if(pResult->NumArguments() < MaxArgs)
            return GetCaller(pResult, pUserData);

        return ParseVictim((char*)pResult->GetString(0), pUserData);
	}

    static bool CommandValidateGuestForSelf(IConsole::IResult *pResult, void *pUserData) {
        if(CCommands::ValidateAuthenticated(pResult, pUserData)) {
            CGameContext *pGameServer = (CGameContext *)pUserData;
            pGameServer->SendChatTarget(pResult->m_ClientId, "You are already logged in.");
            return false;
        }
        return true;
    }

    static bool CommandValidateAuthForSelf(IConsole::IResult *pResult, void *pUserData) {
        if(!CCommands::ValidateAuthenticated(pResult, pUserData)) {
            CGameContext *pGameServer = (CGameContext *)pUserData;
            pGameServer->SendChatTarget(pResult->m_ClientId, "You are not logged in.");
            return false;
        }
        return true;
    }

    static bool CommandValidateStaffForVictim(IConsole::IResult *pResult, void *pUserData) {
        CGameContext *pGameServer = (CGameContext *)pUserData;
        if(!CCommands::ValidateStaff(pResult, pUserData, 1)) {
            pGameServer->SendChatTarget(pResult->m_ClientId, "You are not authorized to do that.");
            return false;
        }

        CPlayer *pVictim = CCommands::GetVictimOrCaller(pResult, pUserData);
        if(!pVictim) {
            pGameServer->SendChatTarget(pResult->m_ClientId, "Invalid client id.");
            return false;
        }

        if(!CCommands::ValidateAuthenticated(pVictim)) {
            pGameServer->SendChatTarget(pResult->m_ClientId, "The player is not logged in.");
            return false;
        }

        return true;
    }
};