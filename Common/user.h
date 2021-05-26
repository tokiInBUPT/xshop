#ifndef USER_H
#define USER_H
#include <string>
#include <vector>
using namespace std;

typedef enum USERTYPE {
    CONSUMER,
    MERCHANT
} USERTYPE;

typedef struct USERCART {
    string id = "";
    int quantity = 0;
} USERCART;

class User {
public:
    User();
    /* 用户名 */
    string username;
    /* 密码(MD5) */
    string passwordHash;
    /* 余额，按分存 */
    int balance;
    /* 购物车 */
    vector<USERCART> cart;
    /* 获取用户类型 */
    virtual USERTYPE getUserType() = 0;
    void setPassword(string password);
};

#endif // USER_H
