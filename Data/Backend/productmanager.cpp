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

namespace Backend {
    ProductManager::ProductManager(string productFile) {
        this->productFile = productFile;
        this->load();
    }
    ProductManager::~ProductManager() {
        this->save();
        for (auto v : this->productList) {
            delete v.second;
        }
        this->productList.empty();
    }
    /*
     * 从文件加载所有商品
     */
    void ProductManager::load() {
        string strData;
        ifstream s(this->productFile, ios_base::binary);
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
            this->productList.insert(pair<string, Product *>(product->id, product));
        }
    }
    /*
     * 保存所有商品到文件
     */
    void ProductManager::save() {
        cout << "Backend::ProductManager::save()" << endl;
        QJsonArray jsonArray;
        for (auto kv : this->productList) {
            Product *v = kv.second;
            QJsonObject obj;
            obj["id"] = QJsonValue(QString::fromStdString(v->id));
            obj["name"] = QJsonValue(QString::fromStdString(v->name));
            obj["description"] = QJsonValue(QString::fromStdString(v->description));
            obj["owner"] = QJsonValue(QString::fromStdString(v->owner));
            obj["pricing"] = QJsonValue(v->pricing);
            obj["stock"] = QJsonValue(v->stock);
            obj["type"] = QJsonValue((int)v->getProductType());
            jsonArray.push_back(obj);
        }
        string strData = QJsonDocument(jsonArray).toJson().toStdString();
        ofstream s(this->productFile, ios::out);
        if (s.fail()) {
            // 文件保存失败，报错
            cout << "Error saving user data" << endl;
            throw "Error saving user data";
        } else {
            s << strData;
        }
        s.close();
    }
    map<string, Product *> ProductManager::listProduct() const {
        return this->productList;
    }
    /* 
     * 减少指定库存
     */
    bool ProductManager::decAmount(Product *product, int amount) {
        Product *p = this->_getProductById(product->id);
        if (p == NULL || (amount > 0 && p->stock < amount)) {
            return false;
        }
        p->stock -= amount;
        this->save();
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
        this->productList.erase(product->id);
        this->save();
        return true;
    }
    /*
     * 商家添加商品并设置商品所有人为自己
     */
    bool ProductManager::addProduct(Product *product) {
        User *currentUser = (DataManager::getInstance())->user->currentUser;
        if (currentUser == NULL || currentUser->getUserType() != MERCHANT) {
            return false;
        }
        product->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        product->owner = currentUser->username;
        return this->addOrReplace(product);
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
        return this->addOrReplace(product);
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
    /*
     * 内部函数 - 添加或覆盖，不校验操作人
     */
    bool ProductManager::addOrReplace(Product *product) {
        /* 复制product，实现不影响原数据 */
        Product *copyProduct = this->copy(product);
        Product *origProduct = this->_getProductById(copyProduct->id);
        /* 如果是替换，释放被替换的 */
        if (origProduct != NULL) {
            delete origProduct;
        }
        this->productList.insert_or_assign(copyProduct->id, copyProduct);
        this->save();
        return true;
    }
}