#ifndef USERCONSUMER_H
#define USERCONSUMER_H

#include "user.h"

class UserConsumer : public User
{
public:
    UserConsumer();
    /* 获取用户类型 */
    USERTYPE getUserType();
};

#endif // USERCONSUMER_H
