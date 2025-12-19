#include "TcpServer.h"

#include <WS2tcpip.h>
#include <winsock2.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ThreadPool.h"

TcpServer::TcpServer(int port) : port(port), pool(4) {
  loadMessage();
  intiWinsock();
  createSocket();
  bindSocket();
  listenSocket();
}

TcpServer::~TcpServer() {
  closesocket(serverSocket);
  WSACleanup();
}

void TcpServer::start() {
  while (true) {
    // check for client now - accept requests
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == SOCKET_ERROR) {
      std::cerr << "client socket failed" << WSAGetLastError << "\n";
      continue;
      exit(1);
    }
    pool.enqueue([this, clientSocket] { this->handleClient(clientSocket); });
  }
  std::cout << "Client Connected!! \n";
}

void TcpServer::intiWinsock() {
  // winsock initialization
  // asking OS for socket permissions
  WSADATA wsaData;
  int result = WSAStartup(MAKEWORD(2, 3), &wsaData);
  if (result != 0) {
    std::cerr << "WSA startup failed" << result << "\n";
    exit(1);
  }
}

void TcpServer::createSocket() {
  // Create a socket
  // af inet == ipv4 and sock stream == tcp
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == INVALID_SOCKET) {
    std::cerr << "Socket creation failed" << WSAGetLastError() << "\n";
    WSACleanup();
    exit(1);
  }
}

void TcpServer::bindSocket() {
  // bind socket to ip and port
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);

  if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    std::cerr << "Binding failed" << WSAGetLastError() << "\n";
    closesocket(serverSocket);
    WSACleanup();
    exit(1);
  }
}

void TcpServer::listenSocket() {
  if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
    std::cerr << "listen failed" << WSAGetLastError << "\n";
    closesocket(serverSocket);
    WSACleanup();
    exit(1);
  }
}

std::pair<std::string, std::string> TcpServer::parseRequest(
    const std::string& buffer) const {
  std::stringstream ss(buffer);
  std::string temp;
  std::pair<std::string, std::string> res;
  ss >> res.first;   // method
  ss >> res.second;  // path
  return res;
}

std::string TcpServer::urlDecode(std::string& url) {
  std::string decoded;
  int n = url.size();
  for (int i = 0; i < n; i++) {
    // if we find a ascii character of 2 letters
    if (url[i] == '%' && i + 2 < n) {
      // convert it to assci
      int code = std::stoi(url.substr(i + 1, 2), nullptr, 16);
      std::string s = std::string(1, static_cast<char>(code));
      decoded += s;
      i += 2;  // skip next two character
    } else if (url[i] == '+') {
      decoded += ' ';  // space
    } else {
      decoded += url[i];
    }
  }
  return decoded;
}

std::string TcpServer::getMimeType(const std::string& path) const {
  if (path.find(".html") != std::string::npos) return "text/html";
  if (path.find(".css") != std::string::npos) return "text/css";
  if (path.find(".js") != std::string::npos) return "application/javascript";
  if (path.find(".png") != std::string::npos) return "image/png";
  if (path.find(".jpg") != std::string::npos) return "image/jpeg";
  if (path.find(".jpeg") != std::string::npos) return "image/jpeg";
  if (path.find(".jfif") != std::string::npos) return "image/jpeg";
  return "text/html";
}

std::string TcpServer::getRequestBody(const std::string& request) const {
  std::string delimiter = "\r\n\r\n";
  size_t pos = request.find(delimiter);
  if (pos == std::string::npos) {
    std::cerr << "No body in request";
    return "";
  }
  // body after the delimiter
  return request.substr(pos + delimiter.size());
}

// get key value pairs from request boyd
std::unordered_map<std::string, std::string> TcpServer::parseBody(
    const std::string& body) const {
  std::stringstream ss(body);
  std::unordered_map<std::string, std::string> data;
  std::string temp;
  while (std::getline(ss, temp, '&')) {
    size_t equalPos = temp.find('=');
    if (equalPos != std::string::npos) {
      std::string key = temp.substr(0, equalPos);
      std::string val = temp.substr(equalPos + 1);
      data[key] = val;
    }
  }
  return data;
}

void TcpServer::loadMessage() {
  std::ifstream input("messages.txt");
  if (!input.is_open()) {
    std::cerr << "Unable to open file to load older messages" << "\n";
    return;
  }
  std::string message;
  while (std::getline(input, message)) {
    if (message.size()) {
      msgBoard.push_back(message);
    }
  }
}

void TcpServer::saveMessage(const std::string& message) const {
  // append mode to write to file
  // this is not thread safe yet - use mtx and lock_guard
  std::ofstream file("messages.txt", std::ios::app);
  if (file.is_open()) {
    file << message << "\n";
  } else {
    std::cerr << "Error opening file for writing message : " << message << "\n";
  }
}

void TcpServer::handleClient(SOCKET clientSocket) {
  char buffer[4096];
  ZeroMemory(buffer, 4096);  // window specific used to clear junk
  int bytesRecieved = recv(clientSocket, buffer, 4096, 0);
  if (bytesRecieved <= 0) {
    std::cerr << "Not bytes recieved in request";
    closesocket(clientSocket);
    return;
  }
  std::string request(buffer,
                      bytesRecieved);  // char* do not end with /0 we
                                       // need to convert it into string
  auto [method, path] = parseRequest(request);

  std::string responseBody;
  std::string contentType = "text/html";
  int statusCode = 200;
  // POST request
  if (method == "POST" && path == "/api/message") {
    // get content form the body
    std::string body = getRequestBody(request);
    auto keyValues = parseBody(body);
    // if has a message with key "content"
    if (keyValues.count("content")) {
      // we decode to remove + %12
      std::string message = urlDecode(keyValues["content"]);
      // add it to database but make sure we avoid race condition
      {
        // this is to make sure not 2 thread push at same index
        std::lock_guard lock(msgMtx);
        msgBoard.push_back(message);
      }
      saveMessage(message);
    }
    responseBody =
        "<html><body><h1>Posted!</h1><a href='/'>Go Back</a></body></html>";
    contentType = "text/html";
  } else if (method == "POST" && path == "/api/login") {
    // extract payload
    std::string body = getRequestBody(request);
    std::cout << "recieved login data : " << body << "\n";
    // parse body and get the data
    auto mapData = parseBody(body);
    std::string username = mapData["username"];
    std::string password = mapData["password"];
    if (username == "chetan" && password == "password") {
      // dummy response
      responseBody =
          "<html><h1>Login success .Welcome " + username + "</h1></html>";
    } else {
      // dummy response
      responseBody =
          "<html><h1>Login failed invalid username and password</h1></html>";
    }

    contentType = "text/html";
  } else {
    // GET request and routing
    if (path == "/" || path == "/index.html") {
      // to display message
      std::string messages = "<ul>";  // tag used to start and end list
      // make sure to avoid race condtion while reading the vector
      {
        std::lock_guard lock(msgMtx);
        for (const std::string& msg : msgBoard) {
          messages += "<li>" + msg + "</li>";
        }
        // end the html tag
        messages += "</ul>";
      }
      path = "/index.html";
      std::string fullpath = "www" + path;
      std::ifstream file(fullpath, std::ios::in | std::ios::binary);
      // read the file and replace the placeholder with our message text
      if (file.is_open()) {
        std::cerr << "File is found at : " << fullpath << "\n";
        std::ostringstream outStream;
        outStream << file.rdbuf();
        std::string html = outStream.str();
        // find the placeholder in the html text and replce it
        std::string placeHolderText = "###";
        size_t placeHolderPos = html.find(placeHolderText);
        if (placeHolderPos != std::string::npos) {
          // replace place holder text with new messages;
          html.replace(placeHolderPos, placeHolderText.size(), messages);
        }
        responseBody = html;
        contentType = getMimeType(path);
      }
    } else {
      if (path == "/api/data") {
        contentType = "application/json";
        responseBody = "{ \"id\": 1, \"name\": \"C++ Server\" }";
      } else {
        statusCode = 404;
        responseBody = "<html><h1>404 Not Found</h1></html>";
      }
    }
  }
  // standard http response for head => body is just appended after this
  std::string responseHeader = "HTTP/1.1 " + std::to_string(statusCode) +
                               " OK\r\n"
                               "Content-Type:" +
                               contentType +
                               "\r\n"
                               "Content-Length: " +
                               std::to_string(responseBody.size()) +
                               "\r\n"
                               "\r\n";  // end of header

  // send header first
  send(clientSocket, responseHeader.c_str(), responseHeader.size(), 0);
  // send body after
  send(clientSocket, responseBody.data(), responseBody.size(), 0);
  std::cout << "handle client success : thread ran successfully!" << "\n";
  closesocket(clientSocket);
}
