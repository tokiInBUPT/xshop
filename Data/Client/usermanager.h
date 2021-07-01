#ifndef C_USERMANAGER_H
#define C_USERMANAGER_H
#include "../../Common/userconsumer.h"
#include "../../Common/usermerchant.h"
#include "../Common/usermanager.h"
#include "httpclient.h"
#include <map>
#include <string>
#include <vector>
using namespace std;
namespace Client {
    class UserManager : Common::UserManager {
    public:
        UserManager(xHttpClient* client);
        ~UserManager();
        bool reg(User *user);
        bool login(string username, string password);
        bool logout();
        bool addFund(int amount);
        bool useFund(int amount);
        bool addFundToUser(string username,int amount);
        bool changePassword(string password);
        bool setDiscount(PRODUCTTYPE type,float discount);
        int UserManager::getInCart(string id);
        int addToCart(string id,int count);
        int setToCart(string id,int count);
        bool emptyCart();
        User* getUserByName(string username) const;
        void load();
        void loadCurrentUser();
    private:
        xHttpClient* client;
        map<string, User *> userList;
        User *_getUserByName(string username) const;
        void saveCart();
    };
};
#endif
