#ifndef HTTP_H
#define HTTP_H
#include <map>
#include <regex>
typedef enum HTTP_METHOD {
    HTTP_UNKNOWN,
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_HEAD,
    HTTP_OPTIONS,
    HTTP_PATCH
} HTTP_METHOD;
class xHttp {
public:
    int code;
    HTTP_METHOD method;

    std::string url;

    std::map<std::string, std::string> headers;

    std::string body;

    std::string toString();

    static xHttp fromString(std::string str);
    static char tolower(char in);
    
};
#endif