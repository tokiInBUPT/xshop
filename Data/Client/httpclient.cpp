#include "httpclient.h"
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <regex>
using namespace std;
int xHttpClient::sendRequest() {
    string base = this->base;
    this->request.code = 0;
    this->request.headers.insert_or_assign("User-Agent", "xshop-client");
    smatch res;
    regex urlRegex("http:\\/\\/(.*?)(:(\\d+))?(/(.*?))?$");
    regex_search(base, res, urlRegex);
    string host = res[1];
    string pstr = res[3];
    int port = stoi(pstr.length() <= 0 ? "80" : pstr);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(s, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "connect() " << WSAGetLastError() << endl;
        return WSAGetLastError();
    }
    string reqstr = this->request.toString();
    const char *req = reqstr.c_str();
    if (send(s, req, (int)reqstr.size(), 0) <= 0) {
        cout << "send() " << WSAGetLastError() << endl;
        return WSAGetLastError();
    }
    int size_recv = 0;
    string resStr;
    while (1) {
        char chunk[129];
        if ((size_recv = recv(s, chunk, 128, 0)) <= 0) {
            break;
        } else {
            chunk[size_recv] = '\0';
            resStr.append(chunk);
        }
    }
    this->response = xHttp::fromString(resStr);
    return 0;
}