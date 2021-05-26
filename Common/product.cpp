#include "product.h"
#include "../Data/DataManager.h"

Product::Product() {
}
int Product::getRealPrice() const {
    return DataManager::getInstance()->product->getPrice(this);
}