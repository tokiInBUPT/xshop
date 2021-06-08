#include "ordermanager.h"
#include "../DataManager.h"
#include "productmanager.h"
#include "usermanager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QUuid>
#include <QVariantMap>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Backend {
    OrderManager::OrderManager(string orderFile) {
        this->orderFile = orderFile;
        this->load();
    }
    OrderManager::~OrderManager() {
        this->save();
    }
    void OrderManager::load() {
        string strData;
        ifstream s(this->orderFile, ios_base::binary);
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
            // 错误时直接return list就会是空的
            cout << jsonError.errorString().toStdString() << endl;
            return;
        }
        QJsonArray jsonArray = jsonDoc.array();
        for (auto v : jsonArray) {
            QJsonObject obj = v.toObject();
            ORDER order;
            order.id = obj["id"].toString().toStdString();
            order.user = obj["user"].toString().toStdString();
            order.status = (ORDER_STATUS)obj["status"].toInt();
            order.pricing = obj["pricing"].toInt();
            for (auto v : obj["cart"].toArray()) {
                QJsonObject jitem = v.toObject();
                USERCART item;
                item.id = jitem["id"].toString().toStdString();
                item.quantity = jitem["quantity"].toInt();
                order.cart.push_back(item);
            }
            QJsonObject itemPriceObj = obj["itemPrice"].toObject();
            QJsonObject::const_iterator it = itemPriceObj.constBegin();
            QJsonObject::const_iterator end = itemPriceObj.constEnd();
            while (it != end) {
                order.itemPrice.insert_or_assign(it.key().toStdString(), it.value().toInt());
                it++;
            }
            this->orders.insert_or_assign(order.id, order);
        }
    }
    void OrderManager::save() {
        cout << "Backend::OrderManager::save()" << endl;
        QJsonArray jsonArray;
        for (auto kv : this->orders) {
            ORDER order = kv.second;
            QJsonObject obj;
            obj["id"] = QJsonValue(QString::fromStdString(order.id));
            obj["user"] = QJsonValue(QString::fromStdString(order.user));
            obj["status"] = QJsonValue(order.status);
            obj["pricing"] = QJsonValue(order.pricing);
            QJsonArray cart;
            for (auto c : order.cart) {
                QJsonObject cartObj;
                cartObj["id"] = QJsonValue(QString::fromStdString(c.id));
                cartObj["quantity"] = QJsonValue(c.quantity);
                cart.push_back(cartObj);
            }
            obj["cart"] = cart;
            QJsonObject itemPrice;
            for (auto kv2 : order.itemPrice) {
                itemPrice.insert(QString::fromStdString(kv2.first), QJsonValue(kv2.second));
            }
            obj["itemPrice"] = itemPrice;
            jsonArray.push_back(obj);
        }
        string strData = QJsonDocument(jsonArray).toJson().toStdString();
        ofstream s(this->orderFile, ios::out);
        if (s.fail()) {
            // 文件保存失败，报错
            throw "Error saving order data";
        } else {
            s << strData;
        }
        s.close();
    }
    bool OrderManager::buy(Product *product, int amount) {
        User *currentUser = DataManager::getInstance()->user->currentUser;
        int realPrice = product->getRealPrice() * amount;
        if (currentUser == NULL || currentUser->balance < realPrice || product->stock < amount) {
            return false;
        }
        DataManager::getInstance()->user->useFund(realPrice);
        ((Backend::ProductManager *)DataManager::getInstance()->product)->decAmount(product, amount);
        return true;
    }
    string OrderManager::checkout(string *message) {
        User *currentUser = DataManager::getInstance()->user->currentUser;
        ORDER order;
        order.status = ORDER_PENDING;
        order.user = currentUser->username;
        order.pricing = 0;
        for (auto v : currentUser->cart) {
            Product *p = DataManager::getInstance()->product->getProductById(v.id);
            if (p == NULL) {
                continue;
            }
            // 把购物车复制一份
            order.cart.push_back(v);
            int realPrice = p->getRealPrice() * v.quantity;
            order.itemPrice.insert_or_assign(p->id, p->getRealPrice());
            order.pricing += realPrice;
            if (p->stock < v.quantity) {
                *message = p->name + " 库存不足";
                delete p;
                return "";
            }
            ((Backend::ProductManager *)DataManager::getInstance()->product)->decAmount(p, v.quantity);
            delete p;
        }
        order.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        this->orders.insert_or_assign(order.id, order);
        DataManager::getInstance()->user->emptyCart();
        this->save();
        return order.id;
    }
    ORDER OrderManager::getById(string id) {
        auto it = this->orders.find(id);
        if (it == this->orders.end()) {
            ORDER empty;
            empty.id = "";
            return empty;
        } else {
            return it->second;
        }
    }
    bool OrderManager::cancel(string id) {
        User *currentUser = DataManager::getInstance()->user->currentUser;
        ORDER order = this->getById(id);
        if (order.id.length() <= 0) {
            return false;
        }
        if (order.status != ORDER_PENDING) {
            return false;
        }
        if (order.user != currentUser->username) {
            return false;
        }
        for (auto v : order.cart) {
            Product *p = DataManager::getInstance()->product->getProductById(v.id);
            ((Backend::ProductManager *)DataManager::getInstance()->product)->decAmount(p, 0 - v.quantity);
            delete p;
        }
        order.status = ORDER_CANCELED;
        this->orders.insert_or_assign(order.id, order);
        this->save();
        return true;
    }
    bool OrderManager::pay(string id) {
        User *currentUser = DataManager::getInstance()->user->currentUser;
        ORDER order = this->getById(id);
        if (order.id.length() <= 0) {
            return false;
        }
        if (order.status != ORDER_PENDING) {
            return false;
        }
        if (currentUser == NULL || order.user != currentUser->username) {
            return false;
        }
        if (currentUser->balance < order.pricing) {
            return false;
        }
        for (auto v : order.cart) {
            Product *p = DataManager::getInstance()->product->getProductById(v.id);
            auto it = order.itemPrice.find(v.id);
            int price = 0;
            if (it == order.itemPrice.end()) {
                continue;
            } else {
                price = it->second * v.quantity;
            }
            ((Backend::UserManager *)DataManager::getInstance()->user)->addFundToUser(p->owner, price);
            DataManager::getInstance()->user->useFund(price);
            delete p;
        }
        order.status = ORDER_PAID;
        this->orders.insert_or_assign(order.id, order);
        this->save();
        return true;
    }
    map<string, ORDER> OrderManager::list() {
        return this->orders;
    }
};