#ifndef C_ORDERMANAGER_H
#define C_ORDERMANAGER_H
#include "../Common/ordermanager.h"
#include "../../Common/user.h"
#include "./productmanager.h"
#include "./usermanager.h"
#include "httpclient.h"
#include <map>
#include <string>
#include <vector>
using namespace std;
namespace Client {
    class OrderManager : Common::OrderManager {
    public:
        OrderManager(xHttpClient* client);
        ~OrderManager();
        bool buy(Product *product, int amount);
        string checkout(string *message);
        bool pay(string id);
        bool cancel(string id);
        ORDER getById(string id);
        map<string,ORDER> list();
        void load();
    private:
        xHttpClient* client;
        map<string, ORDER> orders;
    };
};

#endif // B_ORDERMANAGER_H
