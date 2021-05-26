#ifndef PRODUCTBOOK_H
#define PRODUCTBOOK_H

#include "../product.h"

class ProductBook : public Product
{
public:
    ProductBook();
    PRODUCTTYPE getProductType() const;
};

#endif // PRODUCTBOOK_H
