#ifndef B_USERMANAGER_H
#define B_USERMANAGER_H
#include "../../Common/userconsumer.h"
#include "../../Common/usermerchant.h"
#include "../Common/usermanager.h"
#include <map>
#include <string>
#include <vector>
using namespace std;
namespace Backend {

    class UserManager : Common::UserManager {
    public:
        UserManager(string userFile);
        ~UserManager();
        bool reg(User *user);
        bool login(string username, string password);
        bool logout();
        bool addFund(int amount);
        bool useFund(int amount);
        bool changePassword(string password);
        bool setDiscount(PRODUCTTYPE type,float discount);
        bool addToCart(string id,int count);
        bool setToCart(string id,int count);
        User* getUserByName(string username) const;

    private:
        string userFile;
        map<string, User *> userList;
        User *_getUserByName(string username) const;
        void save();
        void load();
    };
};
#endif // USERMANAGER_H
