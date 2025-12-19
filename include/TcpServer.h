#pragma once
#include <WS2tcpip.h>
#include <winsock2.h>

#include <iostream>
#include <string>

#include "ThreadPool.h"

class TcpServer {
 public:
  TcpServer(int port);
  ~TcpServer();
  void start();

 private:
  int port;
  // db for messages
  std::vector<std::string> msgBoard;
  std::mutex msgMtx;
  SOCKET serverSocket;
  ThreadPool pool;
  void intiWinsock();
  void createSocket();
  void bindSocket();
  void listenSocket();
  /* Parse request and return method and path*/
  std::pair<std::string, std::string> parseRequest(
      const std::string& buffer) const;
  std::string urlDecode(std::string& url);
  std::string getMimeType(const std::string& path) const;
  std::string getRequestBody(const std::string& request) const;
  std::unordered_map<std::string, std::string> parseBody(
      const std::string& body) const;
void loadMessage();
void saveMessage(const std::string& message) const;
void handleClient(SOCKET clientSocket);
};