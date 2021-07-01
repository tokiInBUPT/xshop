#include "http.h"
#include <iostream>
#include <regex>
using namespace std;
xHttp xHttp::fromString(string data) {
    /* 最简单的HTTP解析，并未对错误格式做任何处理 */
    regex requestLine("(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH)\\s+(.*?)\\s+(HTTP\\/1\\.[01])");
    regex responseLine("HTTP\\/1\\.[1|0] (\\d+) (.*?)$");
    xHttp packet;
    /* 拆分段落 */
    size_t rnpos = data.find("\r\n");
    size_t bodypos = data.find("\r\n\r\n");
    string request = data.substr(0, rnpos);
    string header = data.substr(rnpos + 2, bodypos - rnpos - 2);
    packet.body = data.substr(bodypos + 4);
    size_t i = 0;
    /* 解析URL与Method */
    smatch res;
    regex_search(request, res, requestLine);
    if (res.size() > 0) {
        // 是Request
        packet.method = HTTP_UNKNOWN;
        if (res[1] == "GET") {
            packet.method = HTTP_GET;
        } else if (res[1] == "POST") {
            packet.method = HTTP_POST;
        } else if (res[1] == "PUT") {
            packet.method = HTTP_PUT;
        } else if (res[1] == "DELETE") {
            packet.method = HTTP_DELETE;
        } else if (res[1] == "HEAD") {
            packet.method = HTTP_HEAD;
        } else if (res[1] == "OPTIONS") {
            packet.method = HTTP_OPTIONS;
        } else if (res[1] == "PATCH") {
            packet.method = HTTP_PATCH;
        }
        packet.url = res[2];
        packet.code = 0;
    } else {
        regex_search(request, res, responseLine);
        if (res.size() > 0) {
            packet.code = stoi(res[1]);
            packet.method = HTTP_UNKNOWN;
        } else {
            packet.code = -1;
        }
    }
    /* 解析Headers */
    while (i < header.length()) {
        size_t mpos = header.find_first_of(':', i);
        size_t npos = header.find("\r\n", i);
        string key = header.substr(i, mpos - i);
        string value = header.substr(mpos + 1, npos - mpos);
        value.erase(0, value.find_first_not_of(" \r\n\t"));
        value.erase(value.find_last_not_of(" \r\n\t") + 1);
        transform(key.begin(), key.end(), key.begin(), xHttp::tolower);
        packet.headers.insert_or_assign(key, value);
        i = npos + 2;
        if (mpos == string::npos || npos == string::npos) {
            break;
        }
    }
    return packet;
}
char xHttp::tolower(char in) {
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}
string xHttp::toString() {
    string s = "";
    char *str[8] = {"UNKNOWN", "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"};
    if (this->code == 0) {
        s += str[this->method];
        s += " ";
        s += this->url;
        s += " HTTP/1.1\r\n";
    } else {
        s += "HTTP/1.1 ";
        s += to_string(this->code);
        s += " ";
        map<int, string> codeStr{{200, "OK"}, {404, "Not Found"}, {401, "Unauthorized"}, {403, "Forbridden"}, {500, "Internal Server Error"}};
        auto it = codeStr.find(this->code);
        if (it == codeStr.end()) {
            s += "Unknown";
        } else {
            s += it->second;
        }
        s += "\r\n";
    }
    for (auto kv : this->headers) {
        s += kv.first;
        s += ": ";
        s += kv.second;
        s += "\r\n";
    }
    s += "\r\n";
    s += this->body;
    return s;
}
