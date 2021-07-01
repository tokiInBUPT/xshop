#ifndef C_PRODUCTMANAGER_H
#define C_PRODUCTMANAGER_H
#include "../../Common/Product/productbook.h"
#include "../../Common/Product/productcloth.h"
#include "../../Common/Product/productfood.h"
#include "../Common/productmanager.h"
#include "httpclient.h"
#include <map>
#include <string>
using namespace std;

namespace Client {
    class ProductManager : Common::ProductManager {
    public:
        ProductManager(xHttpClient* client);
        ~ProductManager();
        map<string, Product *> listProduct();
        Product *getProductById(string id) const;
        bool addProduct(Product *product);
        bool replaceProduct(Product *product);
        bool deleteProduct(Product *product);
        bool decAmount(Product *product, int amount);
        int getPrice(const Product *product) const;
        void load();

    private:
        xHttpClient* client;
        map<string, Product *> productList;
        Product *copy(Product *product) const;
        Product *_getProductById(string id) const;
    };
}
#endif // C_PRODUCTMANAGER_H
