#ifndef B_ORDERMANAGER_H
#define B_ORDERMANAGER_H
#include "../Common/ordermanager.h"
#include "./productmanager.h"
#include "./usermanager.h"
#include <string>
using namespace std;

namespace Backend {
    class OrderManager : Common::OrderManager {
    public:
        OrderManager(string orderFile);
        bool buy(Product *product, int amount);
    };
};

#endif // B_ORDERMANAGER_H
