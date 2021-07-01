#ifndef HANDLER_H
#define HANDLER_H
#include "../../Common/Protocol/http.h"
#include <vector>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMap>
#include <QString>
using namespace std;
class Handler {
public:
    Handler(xHttp request);
    xHttp getResponse();
    xHttp request;
    vector<string> paths;
private:
    QJsonArray getProducts();
    QJsonArray getUsers();
    QJsonArray getOrders();
    QJsonObject getCurrentUser();
    QJsonObject login();
    bool patchUser();
    bool reg();
    bool checkSession();
    int addProduct();
    int editProduct(string id);
    int deleteProduct(string id);
    QJsonObject checkout();
    int payOrder(string id);
    int deleteOrder(string id);
};
#endif