#ifndef ENGINE_SERVER_SHEEP_SERVER_H
#define ENGINE_SERVER_SHEEP_SERVER_H

#include <engine/server.h>

class CClientSheep {
public:
    char m_CustomClient[24] = "unknown";
};

class CServerSheep {
public:
    CClientSheep m_aClients[MAX_CLIENTS];
    IServer *m_pServer;

    CServerSheep(class IServer *pServer);
    void OnSystemMessage(int ClientId, int Msg);

    const char *GetCustomClient(int ClientId) { return m_aClients[ClientId].m_CustomClient; }
};

#endif