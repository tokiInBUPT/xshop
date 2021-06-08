#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H
#include "../Common/user.h"
#include "../Common/product.h"
#include <map>
#include <vector>
typedef enum {
    ORDER_PENDING,
    ORDER_PAID,
    ORDER_CANCELED
} ORDER_STATUS;
typedef struct ORDER {
    string id;
    string user;
    ORDER_STATUS status;
    int pricing;
    vector<USERCART> cart;
    map<string, int> itemPrice;
} ORDER;
namespace Common {
    class OrderManager {
    public:
        virtual bool buy(Product *product, int amount) = 0;
        virtual string checkout(string *message) = 0;
        virtual bool pay(string id) = 0;
        virtual bool cancel(string id) = 0;
        virtual ORDER getById(string id) = 0;
        virtual map<string,ORDER> list() = 0;
    };
}
#endif // ORDERMANAGER_H
