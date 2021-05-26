#ifndef PRODUCTFOOD_H
#define PRODUCTFOOD_H

#include "../product.h"

class ProductFood : public Product
{
public:
    ProductFood();
    PRODUCTTYPE getProductType() const;
};

#endif // PRODUCTFOOD_H
