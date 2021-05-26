#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H
#include "../Common/product.h"
namespace Common {
    class OrderManager {
    public:
        virtual bool buy(Product *product, int amount) = 0;
    };
}
#endif // ORDERMANAGER_H
