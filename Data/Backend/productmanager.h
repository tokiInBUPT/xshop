#ifndef B_PRODUCTMANAGER_H
#define B_PRODUCTMANAGER_H
#include "../../Common/Product/productbook.h"
#include "../../Common/Product/productcloth.h"
#include "../../Common/Product/productfood.h"
#include "../Common/productmanager.h"
#include <map>
#include <string>
using namespace std;

namespace Backend {
    class ProductManager : Common::ProductManager {
    public:
        ProductManager(string productFile);
        ~ProductManager();
        map<string, Product *> listProduct() const;
        Product *getProductById(string id) const;
        bool addProduct(Product *product);
        bool replaceProduct(Product *product);
        bool deleteProduct(Product *product);
        bool decAmount(Product *product, int amount);
        int getPrice(const Product *product) const;

    private:
        string productFile;
        map<string, Product *> productList;
        void load();
        void save();
        Product *copy(Product *product) const;
        bool addOrReplace(Product *product);
        Product *_getProductById(string id) const;
    };
}
#endif // B_PRODUCTMANAGER_H
