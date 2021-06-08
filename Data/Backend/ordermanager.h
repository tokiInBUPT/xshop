#ifndef B_ORDERMANAGER_H
#define B_ORDERMANAGER_H
#include "../Common/ordermanager.h"
#include "../Common/user.h"
#include "./productmanager.h"
#include "./usermanager.h"
#include <map>
#include <string>
#include <vector>
using namespace std;
namespace Backend {
    class OrderManager : Common::OrderManager {
    public:
        OrderManager(string orderFile);
        ~OrderManager();
        bool buy(Product *product, int amount);
        string checkout(string *message);
        bool pay(string id);
        bool cancel(string id);
        ORDER getById(string id);
        map<string,ORDER> list();
    private:
        string orderFile;
        map<string, ORDER> orders;
        void load();
        void save();
    };
};

#endif // B_ORDERMANAGER_H
