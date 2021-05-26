#include "usermerchant.h"

UserMerchant::UserMerchant() {
    this->discount[0] = 1;
    this->discount[1] = 1;
    this->discount[2] = 1;
}

USERTYPE UserMerchant::getUserType() {
    return MERCHANT;
}