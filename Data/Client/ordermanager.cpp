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

namespace Client {
    OrderManager::OrderManager(xHttpClient *client) {
        this->client = client;
        this->load();
    }
    OrderManager::~OrderManager() {
    }
    void OrderManager::load() {
        this->client->request.method = HTTP_GET;
        this->client->request.url = "/order";
        this->client->request.body = "";
        if (this->client->sendRequest() > 0) {
            cout << "GetOrderList faild." << endl;
            exit(2);
        }
        if (this->client->response.code != 200) {
            cout << "GetOrderList faild with code " << this->client->response.code << "." << endl;
            exit(2);
        }
        QJsonParseError jsonError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(this->client->response.body), &jsonError);
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
        ((Client::UserManager *)DataManager::getInstance()->user)->loadCurrentUser();
    }
    bool OrderManager::buy(Product *product, int amount) {
        // 不再支持直接购买
        return false;
    }
    string OrderManager::checkout(string *message) {
        ((Client::ProductManager *)DataManager::getInstance()->product)->load();
        this->client->request.body = "";
        this->client->request.method = HTTP_POST;
        this->client->request.url = "/order";
        if (this->client->sendRequest() > 0) {
            cout << "HTTPError" << endl;
            return "网络错误";
        }
        QJsonParseError jsonError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(this->client->response.body), &jsonError);
        if (QJsonParseError::NoError != jsonError.error) {
            // 格式解析失败
            cout << jsonError.errorString().toStdString() << endl;
            return "服务器错误";
        }
        if (this->client->response.code != 200) {
            if (jsonDoc.object()["msg"].isString()) {
                return jsonDoc.object()["msg"].toString().toStdString();
            } else {
                return "服务器错误";
            }
        }
        if (jsonDoc.object()["id"].isString()) {
            this->load();
            return jsonDoc.object()["id"].toString().toStdString();
        } else {
            return "服务器错误";
        }
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
        this->client->request.body = "";
        this->client->request.method = HTTP_DELETE;
        this->client->request.url = "/order/" + id;
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
    bool OrderManager::pay(string id) {
        this->client->request.body = "";
        this->client->request.method = HTTP_POST;
        this->client->request.url = "/order/" + id;
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
    map<string, ORDER> OrderManager::list() {
        return this->orders;
    }
};