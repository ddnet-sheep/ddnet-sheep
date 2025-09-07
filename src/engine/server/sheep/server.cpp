#include "server.h"
#include <engine/shared/sheep/protocol_ex_msgs.h>

#include <base/log.h>

CServerSheep::CServerSheep(IServer *pServer) {
    m_pServer = pServer;
}

void CServerSheep::OnSystemMessage(int ClientId, int Msg) {
    switch(Msg) {
        case NETMSG_IAM_QXD:
            str_copy(m_aClients[ClientId].m_CustomClient, "E-Client");
            break;
        case NETMSG_IAM_AIODOB:
            str_copy(m_aClients[ClientId].m_CustomClient, "A-Client");
            break;
        case NETMSG_IAM_TATER:
            str_copy(m_aClients[ClientId].m_CustomClient, "T-Client");
            break;
        case NETMSG_IAM_CHILLERBOT:
            str_copy(m_aClients[ClientId].m_CustomClient, "ChillerBot");
            break;
        case NETMSG_IAM_CACTUS:
            str_copy(m_aClients[ClientId].m_CustomClient, "Cactus");
            break;
        case NETMSG_IAM_FEX:
            str_copy(m_aClients[ClientId].m_CustomClient, "FeX");
            break;
        case NETMSG_IAM_STA:
            str_copy(m_aClients[ClientId].m_CustomClient, "Sta");
            break;
        case NETMSG_IAM_SCLIENT:
            str_copy(m_aClients[ClientId].m_CustomClient, "S-Client");
            break;
        case NETMSG_IAM_NOFIS:
            str_copy(m_aClients[ClientId].m_CustomClient, "Nofis");
            break;
        case NETMSG_IAM_JSCLIENT:
            str_copy(m_aClients[ClientId].m_CustomClient, "JS-Client");
            break;
        case NETMSG_IAM_PULSE:
            str_copy(m_aClients[ClientId].m_CustomClient, "Pulse");
            break;
    }
}
