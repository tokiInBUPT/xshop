#ifndef USERMANAGER_H
#define USERMANAGER_H
#include "../../Common/user.h"
#include "../../Common/product.h"
#include <iostream>
using namespace std;
namespace Common {
    class UserManager {
    public:
        virtual ~UserManager(){};
        virtual bool reg(User *user) = 0;
        virtual bool login(string username, string password) = 0;
        virtual bool logout() = 0;
        virtual bool addFund(int amount) = 0;
        virtual bool useFund(int amount) = 0;
        virtual bool setDiscount(PRODUCTTYPE type,float discount) = 0;
        virtual bool changePassword(string password) = 0;
        virtual int UserManager::getInCart(string id) = 0;
        virtual int addToCart(string id,int count) = 0;
        virtual int setToCart(string id,int count) = 0;
        virtual bool emptyCart() = 0;
        virtual User* getUserByName(string username) const = 0;
        User *currentUser;
    };
}
#endif // USERMANAGER_H
