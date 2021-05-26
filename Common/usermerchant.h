#ifndef USERMERCHANT_H
#define USERMERCHANT_H

#include "user.h"

class UserMerchant : public User
{
public:
    UserMerchant();
    /* 获取用户类型 */
    USERTYPE getUserType();
    /* 商家专属 - 商品折扣 */
    double discount[3];
};

#endif // USERMERCHANT_H
