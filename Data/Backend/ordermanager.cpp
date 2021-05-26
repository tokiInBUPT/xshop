#include "ordermanager.h"
#include "../DataManager.h"
#include "productmanager.h"
#include "usermanager.h"

namespace Backend {
    OrderManager::OrderManager(string orderFile) {
    }
    bool OrderManager::buy(Product *product, int amount) {
        User *currentUser = DataManager::getInstance()->user->currentUser;
        int realPrice = product->getRealPrice() * amount;
        if (currentUser == NULL || currentUser->balance < realPrice || product->stock < amount) {
            return false;
        }
        DataManager::getInstance()->user->useFund(realPrice);
        ((Backend::ProductManager *)DataManager::getInstance()->product)->decAmount(product, amount);
        return true;
    }
};