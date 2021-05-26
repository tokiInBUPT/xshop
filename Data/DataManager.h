#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <iostream>
#include <memory>

#include "Backend/ordermanager.h"
#include "Backend/productmanager.h"
#include "Backend/usermanager.h"
#include "Common/ordermanager.h"
#include "Common/productmanager.h"
#include "Common/usermanager.h"

using namespace std;
using namespace Common;

typedef enum MGRTYPE {
    UNDEFINED,
    STANDALONE,
    ONLINE
} MGRTYPE;

class DataManager {
private:
    DataManager(){};
    DataManager(const DataManager &);
    DataManager &operator=(const DataManager &);
    ~DataManager() {
        delete this->user;
        delete this->order;
        delete this->product;
    };
    static void Destory(DataManager *p) {
        delete p;
    };
    static shared_ptr<DataManager> instance;

public:
    MGRTYPE type = UNDEFINED;
    UserManager *user;
    ProductManager *product;
    OrderManager *order;
    void *conn;
    static shared_ptr<DataManager> getInstance() {
        return instance;
    }
    /*
     * 初始化为独立模式或服务端
     */
    bool init(string userFile, string productFile, string orderFile) {
        if (type != UNDEFINED) {
            return false;
        }
        this->user = (UserManager *)new Backend::UserManager(userFile);
        this->order = (OrderManager *)new Backend::OrderManager(orderFile);
        this->product = (ProductManager *)new Backend::ProductManager(productFile);
        return true;
    }
    /*
     * 初始化为联网模式客户端
     */
    bool init(string serverAddr) {
        if (type != UNDEFINED) {
            return false;
        }
        return true;
    }
};

#endif