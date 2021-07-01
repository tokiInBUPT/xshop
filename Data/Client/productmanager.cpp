#include "productmanager.h"
#include "../DataManager.h"
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
#include <QUuid>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

namespace Client {
    ProductManager::ProductManager(xHttpClient *client) {
        this->client = client;
        this->load();
    }
    ProductManager::~ProductManager() {
        for (auto v : this->productList) {
            delete v.second;
        }
        this->productList.empty();
    }
    void ProductManager::load() {
        this->client->request.method = HTTP_GET;
        this->client->request.url = "/product";
        this->client->request.body = "";
        if (this->client->sendRequest() > 0) {
            cout << "GetProductList faild." << endl;
            exit(2);
        }
        if (this->client->response.code != 200) {
            cout << "GetProductList faild with code " << this->client->response.code << "." << endl;
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
            Product *product = NULL;
            switch ((PRODUCTTYPE)obj["type"].toInt()) {
            case BOOK:
                product = new ProductBook;
                break;
            case FOOD:
                product = new ProductFood;
                break;
            case CLOTH:
                product = new ProductCloth;
                break;
            }
            product->id = obj["id"].toString().toStdString();
            product->name = obj["name"].toString().toStdString();
            product->description = obj["description"].toString().toStdString();
            product->owner = obj["owner"].toString().toStdString();
            product->pricing = obj["pricing"].toInt();
            product->stock = obj["stock"].toInt();
            if (this->_getProductById(product->id)) {
                delete this->_getProductById(product->id);
            }
            this->productList.insert_or_assign(product->id, product);
        }
    }
    bool ProductManager::decAmount(Product *product, int amount) {
        // 客户端不允许直接减少库存
        return false;
    }
    map<string, Product *> ProductManager::listProduct() {
        this->load();
        return this->productList;
    }
    /*
     * 商家添加商品，由服务端设置商品所有人为自己
     */
    bool ProductManager::addProduct(Product *v) {
        User *currentUser = (DataManager::getInstance())->user->currentUser;
        if (currentUser == NULL || currentUser->getUserType() != MERCHANT) {
            return false;
        }
        QJsonObject obj;
        obj["name"] = QString::fromStdString(v->name);
        obj["description"] = QString::fromStdString(v->description);
        obj["pricing"] = v->pricing;
        obj["stock"] = v->stock;
        obj["type"] = (int)v->getProductType();
        this->client->request.body = QJsonDocument(obj).toJson();
        this->client->request.method = HTTP_POST;
        this->client->request.url = "/product";
        if (this->client->sendRequest() > 0) {
            cout << "HTTPError" << endl;
            return false;
        }
        if (this->client->response.code != 201) {
            return false;
        }
        this->load();
        return true;
    }

    /*
     * 商家修改自己的商品
     */
    bool ProductManager::replaceProduct(Product *product) {
        User *currentUser = (DataManager::getInstance())->user->currentUser;
        if (currentUser == NULL || currentUser->getUserType() != MERCHANT) {
            return false;
        }
        Product *origProduct = this->getProductById(product->id);
        /* 不能修改不存在的的商品 */
        if (origProduct == NULL) {
            return false;
        }
        /* 不能修改别人的商品 */
        if (origProduct->owner != currentUser->username) {
            delete origProduct;
            return false;
        }
        /* 覆盖owner */
        product->owner = origProduct->owner;
        delete origProduct;
        QJsonObject obj;
        obj["name"] = QString::fromStdString(product->name);
        obj["description"] = QString::fromStdString(product->description);
        obj["pricing"] = product->pricing;
        obj["stock"] = product->stock;
        obj["type"] = (int)product->getProductType();
        this->client->request.body = QJsonDocument(obj).toJson();
        this->client->request.method = HTTP_PUT;
        this->client->request.url = "/product/" + product->id;
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
     * 从列表中删除商品 - 这不会delete传入的商品对象
     */
    bool ProductManager::deleteProduct(Product *product) {
        User *currentUser = (DataManager::getInstance())->user->currentUser;
        if (currentUser == NULL || currentUser->getUserType() != MERCHANT) {
            return false;
        }
        Product *p = this->getProductById(product->id);
        if (p == NULL) {
            return false;
        }
        if (p->owner != currentUser->username) {
            delete p;
            return false;
        }
        delete p;
        this->client->request.body = "";
        this->client->request.method = HTTP_DELETE;
        this->client->request.url = "/product/" + product->id;
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
     * 计算商品价格
     */
    int ProductManager::getPrice(const Product *product) const {
        Product *p = this->_getProductById(product->id);
        if (p == NULL) {
            return product->pricing;
        }
        UserMerchant *ownerUser = (UserMerchant *)((DataManager::getInstance())->user->getUserByName(product->owner));
        if (ownerUser == NULL) {
            return product->pricing;
        }
        double pricing = (double)(ownerUser->discount[p->getProductType()] * p->pricing);
        return (int)pricing;
    }
    /*
     * 内部函数 - 通过ID获取单个商品 - 获取到的是原的对象
     */
    Product *ProductManager::_getProductById(string id) const {
        auto it = this->productList.find(id);
        if (it == this->productList.end()) {
            return NULL;
        } else {
            return it->second;
        }
    }
    /*
     * 通过ID获取单个商品 - 获取到的是复制的对象
     */
    Product *ProductManager::getProductById(string id) const {
        auto it = this->productList.find(id);
        if (it == this->productList.end()) {
            return NULL;
        } else {
            return this->copy(it->second);
        }
    }
    /*
     * 内部函数 - 复制一份
     */
    Product *ProductManager::copy(Product *product) const {
        Product *copyProduct = NULL;
        switch (product->getProductType()) {
        case BOOK:
            copyProduct = new ProductBook;
            break;
        case FOOD:
            copyProduct = new ProductFood;
            break;
        case CLOTH:
            copyProduct = new ProductCloth;
            break;
        }
        copyProduct->id = product->id;
        copyProduct->name = product->name;
        copyProduct->description = product->description;
        copyProduct->owner = product->owner;
        copyProduct->pricing = product->pricing;
        copyProduct->stock = product->stock;
        return copyProduct;
    }
}