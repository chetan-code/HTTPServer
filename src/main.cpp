#include <WS2tcpip.h>
#include <winsock2.h>

#include <iostream>
#include <string>

#include "TcpServer.h"
using namespace std;

// need to link with Ws_32.lib
#pragma comment(lib, "Ws2_32.lib")

int main() {
  TcpServer server(8080);
  server.start();
}
