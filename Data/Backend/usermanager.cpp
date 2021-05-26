#include "usermanager.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMap>
#include <QString>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;
namespace Backend {
    UserManager::UserManager(string userFile) {
        this->userFile = userFile;
        this->currentUser = NULL;
        this->load();
    }
    UserManager::~UserManager() {
        this->save();
        for (auto v : this->userList) {
            delete v.second;
        }
        this->currentUser = NULL;
        this->userList.empty();
    }
    /*
     * 从文件加载所有用户数据
     */
    void UserManager::load() {
        string strData;
        ifstream s(this->userFile, ios_base::binary);
        if (s.fail()) {
            // 文件读取失败
            // 错误时直接return userList就会是空的
            cout << "Open File Error" << endl;
            return;
        } else {
            std::stringstream buffer;
            buffer << s.rdbuf();
            strData = buffer.str();
        }
        s.close();
        QJsonParseError jsonError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(strData), &jsonError);
        if (QJsonParseError::NoError != jsonError.error) {
            // 格式解析失败
            // 错误时直接return userList就会是空的
            cout << jsonError.errorString().toStdString() << endl;
            return;
        }
        QJsonArray jsonArray = jsonDoc.array();
        for (auto v : jsonArray) {
            QJsonObject obj = v.toObject();
            User *user = NULL;
            if (obj["type"].toString() == "MERCHANT") {
                // 折扣是商家的特殊参数
                UserMerchant *um = new UserMerchant;
                QJsonArray discount = obj["discount"].toArray();
                for (int i = 0; i < 3; i++) {
                    um->discount[i] = discount[i].toDouble();
                }
                user = (User *)um;
            } else {
                user = (User *)new UserConsumer;
            }
            user->username = obj["username"].toString().toStdString();
            user->passwordHash = obj["passwordHash"].toString().toStdString();
            user->balance = obj["balance"].toInt();
            this->userList.insert(pair<string, User *>(user->username, user));
        }
    }
    /*
     * 保存所有用户数据到文件
     */
    void UserManager::save() {
        cout << "Backend::UserManager::save()" << endl;
        QJsonArray jsonArray;
        for (auto kv : this->userList) {
            User *v = kv.second;
            QJsonObject obj;
            obj["username"] = QJsonValue(QString::fromStdString(v->username));
            obj["passwordHash"] = QJsonValue(QString::fromStdString(v->passwordHash));
            obj["balance"] = QJsonValue(v->balance);
            obj["type"] = QJsonValue(v->getUserType() == MERCHANT ? "MERCHANT" : "CONSUMER");
            if (v->getUserType() == MERCHANT) {
                // 折扣是商家的特殊参数
                QJsonArray discount = QJsonArray();
                for (int i = 0; i < 3; i++) {
                    discount.push_back(QJsonValue(((UserMerchant *)v)->discount[i]));
                }
                obj["discount"] = discount;
            }
            jsonArray.push_back(obj);
        }
        string strData = QJsonDocument(jsonArray).toJson().toStdString();
        ofstream s(this->userFile, ios::out);
        if (s.fail()) {
            // 文件保存失败，报错
            throw "Error saving user data";
        } else {
            s << strData;
        }
        s.close();
    }
    /*
     * 注册
     */
    bool UserManager::reg(User *user) {
        User *foundUser = this->_getUserByName(user->username);
        if (foundUser == NULL) {
            User *copyUser = NULL;
            if (user->getUserType() == MERCHANT) {
                copyUser = new UserMerchant;
                for (int i = 0; i < 3; i++) {
                    ((UserMerchant *)copyUser)->discount[i] = ((UserMerchant *)user)->discount[i];
                }
            } else {
                copyUser = new UserConsumer;
            }
            copyUser->username = user->username;
            copyUser->passwordHash = user->passwordHash;
            copyUser->balance = user->balance;
            this->userList.insert(pair<string, User *>(user->username, copyUser));
            this->save();
            return true;
        }
        return false;
    }
    /*
     * 通过用户名获取用户信息 - 公开
     * 只会返回用户名，以及商家用户的折扣数据
     */
    User *UserManager::getUserByName(string username) const {
        User *foundUser = this->_getUserByName(username);
        if (foundUser == NULL) {
            return NULL;
        }
        User *foundUserBrief = NULL;
        if (foundUser->getUserType() == MERCHANT) {
            foundUserBrief = new UserMerchant;
            for (int i = 0; i < 3; i++) {
                ((UserMerchant *)foundUserBrief)->discount[i] = ((UserMerchant *)foundUser)->discount[i];
            }
        } else {
            foundUserBrief = new UserConsumer;
        }
        foundUserBrief->username = foundUser->username;
        foundUserBrief->passwordHash = "";
        return foundUserBrief;
    }
    /*
     * 通过用户名获取用户信息 - 私有
     * 返回所有用户数据，包括密码
     */
    User *UserManager::_getUserByName(string username) const {
        auto it = this->userList.find(username);
        if (it == this->userList.end()) {
            return NULL;
        } else {
            return it->second;
        }
    }
    /*
     * 登录
     */
    bool UserManager::login(string username, string password) {
        User *foundUser = this->_getUserByName(username);
        if (foundUser == NULL) {
            return false;
        }
        QByteArray hash = QCryptographicHash::hash(QByteArray::fromStdString(password), QCryptographicHash::Sha256);
        string phash = hash.toHex().toStdString();
        if (phash != foundUser->passwordHash) {
            return false;
        }
        this->currentUser = foundUser;
        return true;
    }
    /*
     * 退出登录
     */
    bool UserManager::logout() {
        this->currentUser = NULL;
        return true;
    }
    /*
     * 增加余额
     */
    bool UserManager::addFund(int amount) {
        if (!this->currentUser) {
            throw "Please login first";
        }
        this->currentUser->balance += amount;
        this->save();
        return true;
    }
    /*
     * 减少余额
     */
    bool UserManager::useFund(int amount) {
        if (this->currentUser->balance < amount) {
            return false;
        }
        this->currentUser->balance -= amount;
        this->save();
        return true;
    }
    /*
     * 设置折扣
     */
    bool UserManager::setDiscount(PRODUCTTYPE type, float discount) {
        if (this->currentUser->getUserType() != MERCHANT) {
            return false;
        }
        ((UserMerchant *)this->currentUser)->discount[(int)type] = discount;
        this->save();
        return true;
    }
    /*
     * 修改密码
     */
    bool UserManager::changePassword(string password) {
        this->currentUser->setPassword(password);
        this->save();
        return true;
    }
    /*
     * 加购物车
     */
    bool UserManager::addToCart(string id, int quantity) {
        bool found = false;
        int c = 0;
        for (auto i : this->currentUser->cart) {
            if (i.id == id) {
                found = true;
                i.quantity += quantity;
                if (i.quantity == 0) {
                    this->currentUser->cart.erase(begin(this->currentUser->cart) + c);
                }
            }
            c++;
        }
        if (!found && quantity > 0) {
            USERCART item;
            item.id = id;
            item.quantity = quantity;
            this->currentUser->cart.push_back(item);
        }
        return true;
    }
    bool UserManager::setToCart(string id, int quantity) {
        bool found = false;
        int c = 0;
        for (auto i : this->currentUser->cart) {
            if (i.id == id) {
                found = true;
                i.quantity = quantity;
                if (i.quantity == 0) {
                    this->currentUser->cart.erase(begin(this->currentUser->cart) + c);
                }
            }
            c++;
        }
        if (!found && quantity > 0) {
            USERCART item;
            item.id = id;
            item.quantity = quantity;
            this->currentUser->cart.push_back(item);
        }
        this->save();
        return true;
    }
};
