#include "Handler.h"
#include "../../Data/DataManager.h"
#include <QByteArray>
#include <QMessageAuthenticationCode>
Handler::Handler(xHttp request) {
    this->request = request;
    std::regex rgx("/");
    std::sregex_token_iterator iter(request.url.begin(), request.url.end(), rgx, -1);
    std::sregex_token_iterator end;
    for (; iter != end; ++iter) {
        this->paths.push_back(*iter);
    }
}
xHttp Handler::getResponse() {
    QJsonDocument d;
    xHttp response;
    response.code = 404;
    if (this->paths.size() <= 1) {
        // /
        response.code = 200;
        response.body = "{\"msg\":\"Welcome to xShop API Server.\"}";
    } else if (this->paths[1] == "product" && this->request.method == HTTP_GET) {
        // GET /product
        response.code = 200;
        d = QJsonDocument(this->getProducts());
    } else if (this->paths[1] == "user" && this->request.method == HTTP_GET) {
        // GET /user
        response.code = 200;
        d = QJsonDocument(this->getUsers());
    } else if (this->paths[1] == "order" && this->request.method == HTTP_GET) {
        // GET /order
        response.code = 200;
        d = QJsonDocument(this->getOrders());
    } else if (this->paths[1] == "session" && this->request.method == HTTP_POST) {
        // POST /login
        QJsonObject obj = this->login();
        response.code = obj["token"].isString() ? 200 : 403;
        d = QJsonDocument(obj);
    } else if (this->paths[1] == "user" && this->request.method == HTTP_POST) {
        // POST /user
        if (this->reg()) {
            response.code = 200;
        } else {
            response.code = 400;
        }
    } else if (!this->checkSession()) {
        // Not login
        response.code = 401;
        response.body = "{\"msg\":\"Authencation required.\"}";
    } else if (this->paths[1] == "session" && this->request.method == HTTP_GET) {
        // GET /session
        response.code = 200;
        d = QJsonDocument(this->getCurrentUser());
    } else if (this->paths[1] == "session" && this->request.method == HTTP_PATCH) {
        // PATCH /session
        if (this->patchUser()) {
            response.code = 200;
            d = QJsonDocument(this->getCurrentUser());
        } else {
            response.code = 500;
        }
    } else if (this->paths[1] == "product" && this->request.method == HTTP_POST) {
        // POST /product
        if (DataManager::getInstance()->user->currentUser->getUserType() != MERCHANT) {
            response.code = 403;
        } else {
            response.code = this->addProduct();
        }
    } else if (this->paths[1] == "product" && this->paths.size() >= 3 && this->request.method == HTTP_PUT) {
        // PUT /product/:id
        if (DataManager::getInstance()->user->currentUser->getUserType() != MERCHANT) {
            response.code = 403;
        } else {
            response.code = this->editProduct(this->paths[2]);
        }
    } else if (this->paths[1] == "product" && this->paths.size() >= 3 && this->request.method == HTTP_DELETE) {
        // DELETE /product/:id
        if (DataManager::getInstance()->user->currentUser->getUserType() != MERCHANT) {
            response.code = 403;
        } else {
            response.code = this->deleteProduct(this->paths[2]);
        }
    } else if (this->paths[1] == "order" && this->paths.size() >= 3 && this->request.method == HTTP_POST) {
        // POST /order/:id
        response.code = this->payOrder(this->paths[2]);
    } else if (this->paths[1] == "order" && this->paths.size() >= 3 && this->request.method == HTTP_DELETE) {
        // DELETE /order/:id
        response.code = this->deleteOrder(this->paths[2]);
    } else if (this->paths[1] == "order" && this->request.method == HTTP_POST) {
        // POST /order
        QJsonObject obj = this->checkout();
        response.code = obj["code"].toInt();
        obj.remove("code");
        d = QJsonDocument(obj);
    }
    if (response.body.length() == 0) {
        response.body = d.toJson().toStdString();
    }
    response.headers.insert_or_assign("content-type", "application/json; charset=utf-8");
    response.headers.insert_or_assign("content-length", to_string(response.body.length()));
    response.headers.insert_or_assign("server", "xshop/stage-3 @xyToki");
    return response;
}
/*
 * 采用JWT进行身份验证
 */
bool Handler::checkSession() {
    auto it = this->request.headers.find("authorization");
    if (it == this->request.headers.end()) {
        return false;
    }
    string authStr = it->second;
    authStr = authStr.substr(7);
    vector<string> seg;
    std::regex rgx("\\.");
    std::sregex_token_iterator iter(authStr.begin(), authStr.end(), rgx, -1);
    std::sregex_token_iterator end;
    for (; iter != end; ++iter) {
        seg.push_back(*iter);
    }
    if (seg.size() != 3) {
        // Wrong JWT Segments
        return false;
    }
    string key = "xshop2KeyC@9Z^2ZEhkDz9Ycm";
    QByteArray sign = QMessageAuthenticationCode::hash(QByteArray::fromStdString(seg[0] + "." + seg[1]), QByteArray::fromStdString(key), QCryptographicHash::Sha256).toBase64();
    string signStr = sign.toStdString();
    replace(signStr.begin(), signStr.end(), '+', '-');
    replace(signStr.begin(), signStr.end(), '/', '_');
    signStr.erase(remove(signStr.begin(), signStr.end(), '='), signStr.end());
    if (signStr != seg[2]) {
        // Wrong signature
        return false;
    }
    QByteArray jsonStr = QByteArray::fromBase64(QByteArray::fromStdString(seg[1]));
    QJsonDocument json = QJsonDocument::fromJson(jsonStr);
    if (!json.isObject()) {
        // Wrong json
        return false;
    }
    QJsonObject obj = json.object();
    time_t now = time(NULL);
    time_t exp = obj["exp"].toInt();
    if (exp < now) {
        // Expired
        return false;
    }
    return ((Backend::UserManager *)DataManager::getInstance()->user)->setCurrentUser(obj["sub"].toString().toStdString());
}
QJsonArray Handler::getProducts() {
    auto m = DataManager::getInstance()->product->listProduct();
    QJsonArray jsonArray;
    for (auto kv : m) {
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
    return jsonArray;
}

QJsonArray Handler::getUsers() {
    auto m = ((Backend::UserManager *)DataManager::getInstance()->user)->list();
    QJsonArray jsonArray;
    for (auto kv : m) {
        User *v = kv.second;
        if (v->getUserType() != MERCHANT) {
            continue;
        }
        QJsonObject obj;
        obj["username"] = QJsonValue(QString::fromStdString(v->username));
        obj["balance"] = QJsonValue(v->balance);
        obj["type"] = QJsonValue(v->getUserType() == MERCHANT ? "MERCHANT" : "CONSUMER");
        // 折扣是商家的特殊参数
        QJsonArray discount = QJsonArray();
        for (int i = 0; i < 3; i++) {
            discount.push_back(QJsonValue(((UserMerchant *)v)->discount[i]));
        }
        obj["discount"] = discount;
        jsonArray.push_back(obj);
    }
    return jsonArray;
}
QJsonArray Handler::getOrders() {
    auto m = DataManager::getInstance()->order->list();
    QJsonArray jsonArray;
    for (auto kv : m) {
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
    return jsonArray;
}
QJsonObject Handler::getCurrentUser() {
    User *v = DataManager::getInstance()->user->currentUser;
    QJsonObject obj;
    obj["username"] = QJsonValue(QString::fromStdString(v->username));
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
    QJsonArray cart;
    for (auto c : v->cart) {
        QJsonObject cartObj;
        cartObj["id"] = QJsonValue(QString::fromStdString(c.id));
        cartObj["quantity"] = QJsonValue(c.quantity);
        cart.push_back(cartObj);
    }
    obj["cart"] = cart;
    return obj;
}
bool Handler::patchUser() {
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(this->request.body), &jsonError);
    if (QJsonParseError::NoError != jsonError.error || !jsonDoc.isObject()) {
        cout << jsonError.errorString().toStdString() << endl;
        return false;
    }
    QJsonObject obj = jsonDoc.object();
    if (obj["password"].isString()) {
        DataManager::getInstance()->user->changePassword(obj["password"].toString().toStdString());
    }
    if (obj["balance"].isDouble()) {
        DataManager::getInstance()->user->addFund(obj["balance"].toInt());
    }
    if (obj["discount"].isArray() && DataManager::getInstance()->user->currentUser->getUserType() == MERCHANT) {
        QJsonArray arr = obj["discount"].toArray();
        if (arr[0].toDouble() >= 0)
            DataManager::getInstance()->user->setDiscount(BOOK, arr[0].toDouble());
        if (arr[1].toDouble() >= 0)
            DataManager::getInstance()->user->setDiscount(FOOD, arr[1].toDouble());
        if (arr[2].toDouble() >= 0)
            DataManager::getInstance()->user->setDiscount(CLOTH, arr[2].toDouble());
    }
    if (obj["cart"].isArray()) {
        QJsonArray cart = obj["cart"].toArray();
        for (auto c : cart) {
            QJsonObject cartObj = c.toObject();
            DataManager::getInstance()->user->setToCart(cartObj["id"].toString().toStdString(), cartObj["quantity"].toInt());
        }
    }
    return true;
}
QJsonObject Handler::login() {
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(this->request.body), &jsonError);
    if (QJsonParseError::NoError != jsonError.error || !jsonDoc.isObject()) {
        cout << jsonError.errorString().toStdString() << endl;
        QJsonObject obj;
        return obj;
    }
    QJsonObject obj = jsonDoc.object();
    if (!obj["password"].isString() || !obj["username"].isString()) {
        QJsonObject obj;
        return obj;
    }
    if (DataManager::getInstance()->user->login(obj["username"].toString().toStdString(), obj["password"].toString().toStdString())) {
        // Success
        string jwtHead = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
        QJsonObject jwt;
        jwt["sub"] = QString::fromStdString(DataManager::getInstance()->user->currentUser->username);
        jwt["exp"] = time(NULL) + 86400;
        string jwtBody = QJsonDocument(jwt).toJson().toBase64().toStdString();
        replace(jwtBody.begin(), jwtBody.end(), '+', '-');
        replace(jwtBody.begin(), jwtBody.end(), '/', '_');
        jwtBody.erase(remove(jwtBody.begin(), jwtBody.end(), '='), jwtBody.end());
        string key = "xshop2KeyC@9Z^2ZEhkDz9Ycm";
        QByteArray sign = QMessageAuthenticationCode::hash(QByteArray::fromStdString(jwtHead + "." + jwtBody), QByteArray::fromStdString(key), QCryptographicHash::Sha256).toBase64();
        string signStr = sign.toStdString();
        replace(signStr.begin(), signStr.end(), '+', '-');
        replace(signStr.begin(), signStr.end(), '/', '_');
        signStr.erase(remove(signStr.begin(), signStr.end(), '='), signStr.end());
        QJsonObject obj;
        obj["token"] = QString::fromStdString(jwtHead + "." + jwtBody + "." + signStr);
        return obj;
    } else {
        QJsonObject obj;
        return obj;
    }
}
bool Handler::reg() {
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(this->request.body), &jsonError);
    if (QJsonParseError::NoError != jsonError.error || !jsonDoc.isObject()) {
        cout << jsonError.errorString().toStdString() << endl;
        return false;
    }
    QJsonObject obj = jsonDoc.object();
    if (!obj["username"].isString() || !obj["passwordHash"].isString() || !obj["type"].isString()) {
        return false;
    }
    User *user = NULL;
    if (obj["type"].toString() == "MERCHANT") {
        user = new UserMerchant;
    } else {
        user = new UserConsumer;
    }
    user->username = obj["username"].toString().toStdString();
    user->passwordHash = obj["passwordHash"].toString().toStdString();
    bool res = DataManager::getInstance()->user->reg(user);
    delete user;
    return res;
}
int Handler::addProduct() {
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(this->request.body), &jsonError);
    if (QJsonParseError::NoError != jsonError.error || !jsonDoc.isObject()) {
        cout << jsonError.errorString().toStdString() << endl;
        return 400;
    }
    QJsonObject obj = jsonDoc.object();
    if (!obj["type"].isDouble() || !obj["stock"].isDouble() || !obj["pricing"].isDouble()) {
        return 400;
    }
    if (!obj["description"].isString() || !obj["name"].isString()) {
        return 400;
    }
    Product *np = NULL;
    switch (obj["type"].toInt()) {
    case BOOK:
        np = new ProductBook;
        break;
    case FOOD:
        np = new ProductFood;
        break;
    case CLOTH:
        np = new ProductCloth;
        break;
    default:
        return 400;
    }
    np->name = obj["name"].toString().toStdString();
    np->description = obj["description"].toString().toStdString();
    np->stock = obj["stock"].toInt();
    np->pricing = obj["pricing"].toInt();
    bool res = DataManager::getInstance()->product->addProduct(np);
    delete np;
    return res ? 201 : 500;
}
int Handler::editProduct(string id) {
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(this->request.body), &jsonError);
    if (QJsonParseError::NoError != jsonError.error || !jsonDoc.isObject()) {
        cout << jsonError.errorString().toStdString() << endl;
        return 400;
    }
    QJsonObject obj = jsonDoc.object();
    if (!obj["type"].isDouble() || !obj["stock"].isDouble() || !obj["pricing"].isDouble()) {
        return 400;
    }
    if (!obj["description"].isString() || !obj["name"].isString()) {
        return 400;
    }
    Product *np = DataManager::getInstance()->product->getProductById(id);
    if (np->owner != DataManager::getInstance()->user->currentUser->username) {
        delete np;
        return 403;
    }
    switch (obj["type"].toInt()) {
    case BOOK:
        np = new ProductBook;
        break;
    case FOOD:
        np = new ProductFood;
        break;
    case CLOTH:
        np = new ProductCloth;
        break;
    default:
        delete np;
        return 400;
    }
    np->id = id;
    np->name = obj["name"].toString().toStdString();
    np->description = obj["description"].toString().toStdString();
    np->stock = obj["stock"].toInt();
    np->pricing = obj["pricing"].toInt();
    bool res = DataManager::getInstance()->product->replaceProduct(np);
    delete np;
    return res ? 200 : 500;
}
int Handler::deleteProduct(string id) {
    Product *np = DataManager::getInstance()->product->getProductById(id);
    if (np->owner != DataManager::getInstance()->user->currentUser->username) {
        delete np;
        return 403;
    }
    bool res = DataManager::getInstance()->product->deleteProduct(np);
    delete np;
    return res ? 200 : 500;
}
QJsonObject Handler::checkout() {
    QJsonObject d;
    string msg = "";
    string orderId = DataManager::getInstance()->order->checkout(&msg);
    if (orderId.length() <= 0) {
        d["msg"] = QString::fromStdString(msg);
        d["code"] = 400;
    } else {
        d["code"] = 200;
        d["id"] = QString::fromStdString(orderId);
    }
    return d;
}
int Handler::payOrder(string id) {
    if (DataManager::getInstance()->order->pay(id)) {
        return 200;
    } else {
        return 500;
    }
}
int Handler::deleteOrder(string id) {
    if (DataManager::getInstance()->order->cancel(id)) {
        return 200;
    } else {
        return 500;
    }
}