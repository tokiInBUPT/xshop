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
namespace Client {
    UserManager::UserManager(xHttpClient *client) {
        this->client = client;
        this->currentUser = NULL;
        this->load();
    }
    UserManager::~UserManager() {
        for (auto v : this->userList) {
            delete v.second;
        }
        this->userList.empty();
        if (this->currentUser != NULL) {
            delete this->currentUser;
        }
    }
    void UserManager::load() {
        this->client->request.method = HTTP_GET;
        this->client->request.url = "/user";
        if (this->client->sendRequest() > 0) {
            cout << "GetUserList faild." << endl;
            exit(2);
        }
        if (this->client->response.code != 200) {
            cout << "GetUserList faild with code " << this->client->response.code << "." << endl;
            exit(2);
        }
        QJsonParseError jsonError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(this->client->response.body), &jsonError);
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
            if (this->_getUserByName(user->username)) {
                delete this->_getUserByName(user->username);
            }
            this->userList.insert_or_assign(user->username, user);
        }
    }
    /*
     * 注册
     */
    bool UserManager::reg(User *user) {
        QJsonObject obj;
        obj["username"] = QString::fromStdString(user->username);
        obj["passwordHash"] = QString::fromStdString(user->passwordHash);
        obj["type"] = user->getUserType() == MERCHANT ? "MERCHANT" : "CONSUMER";
        this->client->request.body = QJsonDocument(obj).toJson();
        this->client->request.method = HTTP_POST;
        this->client->request.url = "/user";
        if (this->client->sendRequest() > 0) {
            cout << "HTTPError" << endl;
            return false;
        }
        if (this->client->response.code != 200) {
            return false;
        }
        this->load();
        return true;
    }
    /*
     * 登录
     */
    bool UserManager::login(string username, string password) {
        QJsonObject obj;
        obj["username"] = QString::fromStdString(username);
        obj["password"] = QString::fromStdString(password);
        this->client->request.body = QJsonDocument(obj).toJson();
        this->client->request.method = HTTP_POST;
        this->client->request.url = "/session";
        if (this->client->sendRequest() > 0) {
            cout << "HTTPError" << endl;
            return false;
        }
        if (this->client->response.code != 200) {
            return false;
        }
        QJsonDocument v = QJsonDocument::fromJson(QByteArray::fromStdString(this->client->response.body));
        obj = v.object();
        this->client->request.headers.insert_or_assign("Authorization", "Bearer " + obj["token"].toString().toStdString());
        this->loadCurrentUser();
        return true;
    }
    void UserManager::loadCurrentUser() {
        if (this->currentUser != NULL) {
            delete this->currentUser;
            this->currentUser = NULL;
        }
        this->client->request.method = HTTP_GET;
        this->client->request.url = "/session";
        this->client->request.body = "";
        if (this->client->sendRequest() > 0) {
            cout << "HTTPError" << endl;
            return;
        }
        if (this->client->response.code != 200) {
            if (this->currentUser != NULL) {
                delete this->currentUser;
            }
            this->currentUser = NULL;
            return;
        }
        QJsonDocument v = QJsonDocument::fromJson(QByteArray::fromStdString(this->client->response.body));
        QJsonObject obj = v.object();
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
        QJsonArray cart = obj["cart"].toArray();
        for (auto c : cart) {
            QJsonObject cartObj = c.toObject();
            USERCART userCart;
            userCart.id = cartObj["id"].toString().toStdString();
            userCart.quantity = cartObj["quantity"].toInt();
            user->cart.push_back(userCart);
        }
        this->currentUser = user;
        return;
    }
    /*
     * 退出
     */
    bool UserManager::logout() {
        this->client->request.headers.erase("Authorization");
        return true;
    }
    /*
     * 加钱
     */
    bool UserManager::addFund(int amount) {
        QJsonObject obj;
        obj["balance"] = amount;
        this->client->request.body = QJsonDocument(obj).toJson();
        this->client->request.method = HTTP_PATCH;
        this->client->request.url = "/session";
        if (this->client->sendRequest() > 0) {
            cout << "HTTPError" << endl;
            return false;
        }
        if (this->client->response.code != 200) {
            return false;
        }
        this->loadCurrentUser();
        return true;
    }
    /*
     * 减钱
     */
    bool UserManager::useFund(int amount) {
        // 客户端不能直接减钱
        return false;
    }
    bool UserManager::changePassword(string password) {
        QJsonObject obj;
        obj["password"] = QString::fromStdString(password);
        this->client->request.body = QJsonDocument(obj).toJson();
        this->client->request.method = HTTP_PATCH;
        this->client->request.url = "/session";
        if (this->client->sendRequest() > 0) {
            cout << "HTTPError" << endl;
            return false;
        }
        if (this->client->response.code != 200) {
            return false;
        }
        return true;
    }
    bool UserManager::setDiscount(PRODUCTTYPE type, float discount) {
        QJsonObject obj;
        QJsonArray arr;
        arr.push_back(-1);
        arr.push_back(-1);
        arr.push_back(-1);
        arr[type] = discount;
        obj["discount"] = arr;
        this->client->request.body = QJsonDocument(obj).toJson();
        this->client->request.method = HTTP_PATCH;
        this->client->request.url = "/session";
        if (this->client->sendRequest() > 0) {
            cout << "HTTPError" << endl;
            return false;
        }
        if (this->client->response.code != 200) {
            return false;
        }
        return true;
    }
    /*
     * 在购物车查找商品
     */
    int UserManager::getInCart(string id) {
        for (auto i : this->currentUser->cart) {
            if (i.id == id) {
                return i.quantity;
            }
        }
        return 0;
    }
    /*
     * 加购物车
     */
    int UserManager::addToCart(string id, int quantity) {
        bool found = false;
        int c = 0;
        for (int i = 0; i < this->currentUser->cart.size(); i++) {
            if (this->currentUser->cart[i].id == id) {
                found = true;
                this->currentUser->cart[i].quantity += quantity;
                quantity = this->currentUser->cart[i].quantity;
                if (this->currentUser->cart[i].quantity == 0) {
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
        this->saveCart();
        return quantity;
    }
    int UserManager::setToCart(string id, int quantity) {
        bool found = false;
        int c = 0;
        for (int i = 0; i < this->currentUser->cart.size(); i++) {
            if (this->currentUser->cart[i].id == id) {
                found = true;
                this->currentUser->cart[i].quantity = quantity;
                if (this->currentUser->cart[i].quantity == 0) {
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
        this->saveCart();
        return quantity;
    }
    bool UserManager::emptyCart() {
        this->currentUser->cart.swap(vector<USERCART>());
        this->saveCart();
        return true;
    }
    void UserManager::saveCart() {
        QJsonObject obj;
        QJsonArray cart;
        for (auto c : this->currentUser->cart) {
            QJsonObject cartObj;
            cartObj["id"] = QJsonValue(QString::fromStdString(c.id));
            cartObj["quantity"] = QJsonValue(c.quantity);
            cart.push_back(cartObj);
        }
        obj["cart"] = cart;
        this->client->request.body = QJsonDocument(obj).toJson();
        this->client->request.method = HTTP_PATCH;
        this->client->request.url = "/session";
        if (this->client->sendRequest() > 0) {
            cout << "HTTPError" << endl;
            return;
        }
        if (this->client->response.code != 200) {
            return;
        }
        this->loadCurrentUser();
        return;
    }
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
     */
    User *UserManager::_getUserByName(string username) const {
        auto it = this->userList.find(username);
        if (it == this->userList.end()) {
            return NULL;
        } else {
            return it->second;
        }
    }
}