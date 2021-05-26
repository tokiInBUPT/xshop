#ifndef PRODUCT_H
#define PRODUCT_H
#include <string>
using namespace std;

typedef enum PRODUCTTYPE
{
    BOOK,
    FOOD,
    CLOTH
} PRODUCTTYPE;

class Product
{
public:
    Product();
    /* 商品ID */
    string id;
    /* 商品名 */
    string name;
    /* 商品描述 */
    string description;
    /* 售价 单位为分 */
    int pricing;
    /* 库存量 */
    int stock;
    /* 所属商家用户名 */
    string owner;
    /* 用户类型 */
    virtual PRODUCTTYPE getProductType() const = 0;
    /* 计算折后价格 */
    virtual int getRealPrice() const;
};

#endif // PRODUCT_H
