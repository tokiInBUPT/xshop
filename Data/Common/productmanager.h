#ifndef PRODUCTMANAGER_H
#define PRODUCTMANAGER_H
#include <map>
#include "../../Common/product.h"
using namespace std;
namespace Common
{
    /*
     * 商品管理接口抽象类
     */
    class ProductManager
    {
    public:
        virtual ~ProductManager(){};
        /*
         * 列出商品列表，返回Map
         */
        virtual map<string,Product*> listProduct() = 0;
        /*
         * 获取单个商品
         */
        virtual Product* getProductById(string id) const = 0;
        /*
         * 添加商品并设置所有者为当前用户
         */
        virtual bool addProduct(Product* product) = 0;
        /*
         * 修改当前用户创建的商品
         */
        virtual bool replaceProduct(Product* product) = 0;
        /*
         * 删除当前用户创建的商品
         */
        virtual bool deleteProduct(Product* product) = 0;
        /*
         * 计算价格
         */
        virtual int getPrice(const Product *product) const = 0;
    };
}
#endif // PRODUCTMANAGER_H
