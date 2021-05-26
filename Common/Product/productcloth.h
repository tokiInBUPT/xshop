#ifndef PRODUCTCLOTH_H
#define PRODUCTCLOTH_H

#include "../product.h"

class ProductCloth : public Product
{
public:
    ProductCloth();
    PRODUCTTYPE getProductType() const;
};

#endif // PRODUCTCLOTH_H
