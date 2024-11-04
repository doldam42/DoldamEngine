#pragma once

#include "WinSock2.h"
#pragma comment(lib, "ws2_32")
#include "AppProtocol.h"
class ClientSocketManager
{
  public:
    void Receive();
    void Send(MYCMD cmd, char* buffer, int bufferSize);
    void ThreadAcceptLoop(void *param);
};
