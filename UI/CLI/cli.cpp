#include "cli.h"
#include "../../Data/DataManager.h"
#include <codecvt>
#include <locale>
#include <map>
#include <sstream>
using namespace std;
/*
 * 命令行界面类
 */
CLI::CLI() {
    while (this->mainMenu()) {
        // Do Nothing
    }
}
bool CLI::mainMenu() {
    system("cls");
    cout << "xShop CLI by xyToki 2021.5" << endl;
    User *currentUser = DataManager::getInstance()->user->currentUser;
    if (currentUser == NULL) {
        cout << "您未登录";
    } else {
        cout << "当前用户：";
        cout << (currentUser->getUserType() == MERCHANT ? "[商家]" : "[买家]");
        cout << currentUser->username;
        cout << " 余额：" << ((float)currentUser->balance) / 100 << "元";
    }
    cout << endl;
    cout << "1. 浏览商品" << endl;
    if (currentUser == NULL) {
        cout << "2. 登录" << endl;
        cout << "3. 注册" << endl;
    } else {
        cout << "2. 余额充值" << endl;
        if (currentUser->getUserType() == MERCHANT) {
            cout << "3. 新商品" << endl;
            cout << "4. 设置折扣" << endl;
        }
        cout << "6. 购物车" << endl;
        cout << "7. 我的订单" << endl;
        cout << "8. 修改密码" << endl;
        cout << "9. 退出登录" << endl;
    }
    cout << "0. 退出程序" << endl;
    cout << "请选择操作并回车确认 >> ";
    string strOp;
    cin >> strOp;
    if (strOp.find_first_not_of("1234567890") != string::npos) {
        this->alert("错误输入");
        return true;
    }
    int op = -1;
    try {
        op = stoi(strOp);
    } catch (...) {
        // 非数字输入
    }
    if (currentUser == NULL && op > 0) {
        op += 10;
    }
    switch (op) {
    case 1:  // User--1: browse
    case 11: // Guest-1: browse
        this->list();
        break;
    case 2: // User-2: recharge
        this->recharge();
        break;
    case 6: // User-6: cart
        this->cart();
        break;
    case 7: // User-7: order
        this->listOrder();
        break;
    case 8: // User-8: passwd
        this->passwd();
        break;
    case 9: // User-9: logout
        if (DataManager::getInstance()->user->logout()) {
            this->alert("您已退出登录");
        } else {
            this->alert("系统错误");
        }
        break;
    case 12: // Guest-2: login
        this->login();
        break;
    case 13: // Guest-3: register
        this->reg();
        break;
    case 0:
        return false;
    case 3: // Merchant-3: new product
        if (currentUser->getUserType() == MERCHANT) {
            this->addOrEdit(NULL);
            break;
        }
    case 4: // Merchant-4: set discount
        if (currentUser->getUserType() == MERCHANT) {
            this->setDiscount();
            break;
        }
    default:
        this->alert("错误输入");
    }
    return true;
}
void CLI::list() {
    bool loop = true;
    bool filter = false;
    PRODUCTTYPE filterType = BOOK;
    string search = "";
    while (loop) {
        system("cls");

        int i = 1;
        map<int, Product *> displayProductMap;
        auto m = DataManager::getInstance()->product->listProduct();
        for (auto v : m) {
            Product *p = v.second;
            if (filter && filterType != p->getProductType()) {
                continue;
            }
            if (search.length() > 0 && p->name.find(search) == std::string::npos) {
                continue;
            }
            displayProductMap.insert_or_assign(i, p);
            string cName = "未知";
            switch (p->getProductType()) {
            case BOOK:
                cName = "书";
                break;
            case FOOD:
                cName = "食";
                break;
            case CLOTH:
                cName = "衣";
                break;
            }
            cout << i << ".\t"
                 << "[" << cName << "]" << p->name << endl;
            cout << "\t库存：" << p->stock;
            cout << "\t原价：" << (float)(p->pricing) / 100.0 << "\t折后：" << (float)p->getRealPrice() / 100.0 << endl;
            cout << endl;
            i++;
        }
        if (i == 1) {
            cout << "当前条件下暂无商品，您可以[q]返回上级，[f]筛选，[s]搜索，并回车确认 >> ";
        } else {
            cout << "请输入编号选择商品，或[q]返回上级，[f]筛选，[s]搜索，并回车确认 >> ";
        }
        string op;
        cin >> op;
        try {
            int seq = stoi(op);
            map<int, Product *>::iterator it = displayProductMap.find(seq);
            if (it == displayProductMap.end()) {
                this->alert("错误输入");
            } else {
                this->show(it->second);
            }
        } catch (...) {
            if (op == "q") {
                return;
            } else if (op == "f") {
                cout << "请选择筛选的产品分类：[b]图书 [f]食品 [c]衣物 [其他]取消筛选 >> ";
                string op;
                cin >> op;
                filter = true;
                if (op == "b") {
                    filterType = BOOK;
                } else if (op == "f") {
                    filterType = FOOD;
                } else if (op == "c") {
                    filterType = CLOTH;
                } else {
                    filter = false;
                }
            } else if (op == "s") {
                cout << "请输入搜索词，并回车确认 >> ";
                this->editorInput(&search);
            } else {
                this->alert("错误输入");
            }
        }
    }
}
void CLI::cart() {
    bool loop = true;
    while (loop) {
        system("cls");
        if (DataManager::getInstance()->user->currentUser->cart.size() <= 0) {
            this->alert("购物车空");
            return;
        } else {
            for (int i = 0; i < DataManager::getInstance()->user->currentUser->cart.size(); i++) {
                USERCART item = DataManager::getInstance()->user->currentUser->cart[i];
                Product *p = DataManager::getInstance()->product->getProductById(item.id);
                if (p == NULL) {
                    // 购物车商品不存在
                    continue;
                }
                this->printCart(p, item.quantity);
                delete p;
            }
            cout << "请输入编号选择需要修改的商品，或[q]返回上级，[b]结算，并回车确认 >> ";

            string op;
            cin >> op;
            try {
                int seq = stoi(op);
                if (seq > DataManager::getInstance()->user->currentUser->cart.size()) {
                    this->alert("错误输入");
                } else {
                    this->editCart(DataManager::getInstance()->user->currentUser->cart[seq - 1]);
                }
            } catch (...) {
                if (op == "q") {
                    return;
                } else if (op == "b") {
                    if (this->checkout()) {
                        return;
                    }
                } else {
                    this->alert("错误输入");
                }
            }
        }
    }
}
void CLI::editCart(USERCART item) {
    system("cls");
    Product *p = DataManager::getInstance()->product->getProductById(item.id);
    this->printCart(p, item.quantity);
    int stock = p->stock;
    delete p;
    cout << "请在此修改商品数量（0为删除）>> ";
    string input = this->to_string(item.quantity, 0);
    this->editorInput(&input);
    try {
        int quantity = stoi(input);
        if (stock < quantity) {
            return this->alert("该商品库存不足");
        }
        quantity = DataManager::getInstance()->user->setToCart(item.id, quantity);
        if (quantity > 0) {
            this->alert("设置成功");
        } else {
            this->alert("删除成功");
        }
    } catch (...) {
        this->alert("输入有误，数量未改变");
    }
}
bool CLI::checkout() {
    string msg = "";
    string orderId = DataManager::getInstance()->order->checkout(&msg);
    if (orderId.length() <= 0) {
        this->alert(msg);
        return false;
    }
    this->payOrCancel(orderId);
    return true;
}
void CLI::listOrder() {
    while (1) {
        map<string, ORDER> orders = DataManager::getInstance()->order->list();
        User *currentUser = DataManager::getInstance()->user->currentUser;
        map<int, string> displayOrdersMap;
        int i = 0;
        for (auto kv : orders) {
            if (kv.second.user != currentUser->username || kv.second.status != ORDER_PENDING) {
                continue;
            }
            i++;
            displayOrdersMap.insert_or_assign(i, kv.second.id);
            cout << i << ". [未支付]" << ((float)kv.second.pricing) / 100 << "元" << endl;
        }
        if (i == 0) {
            this->alert("没有未支付订单");
            return;
        }
        cout << "请输入编号选择订单，或 [q]返回上级，并回车确认 >> ";
        string op;
        cin >> op;
        try {
            int seq = stoi(op);
            auto it = displayOrdersMap.find(seq);
            if (it == displayOrdersMap.end()) {
                this->alert("错误输入");
            } else {
                this->showOrder(it->second);
            }
        } catch (...) {
            if (op == "q") {
                return;
            } else {
                this->alert("错误输入");
            }
        }
    }
}
void CLI::showOrder(string orderId) {
    system("cls");
    ORDER order = DataManager::getInstance()->order->getById(orderId);
    for (int i = 0; i < order.cart.size(); i++) {
        USERCART item = order.cart[i];
        Product *p = DataManager::getInstance()->product->getProductById(item.id);
        if (p == NULL) {
            // 购物车商品不存在
            this->alert("有商品已被删除，该订单将将自动取消");
            DataManager::getInstance()->order->cancel(orderId);
            return;
        }
        auto it = order.itemPrice.find(item.id);
        int price = 0;
        if (it == order.itemPrice.end()) {
            continue;
        } else {
            price = it->second;
        }
        this->printOrder(p, item.quantity, price);
        delete p;
    }
    this->payOrCancel(orderId);
}
void CLI::payOrCancel(string orderId) {
    ORDER order = DataManager::getInstance()->order->getById(orderId);
    User *currentUser = DataManager::getInstance()->user->currentUser;
    while (1) {
        cout << "结算：" << endl
             << "您当前余额为 ";
        cout << ((float)currentUser->balance) / 100 << "元" << endl;
        cout << "需要支付：" << ((float)order.pricing) / 100 << "元" << endl;
        cout << endl;
        if (order.pricing > currentUser->balance) {
            cout << "余额不足，您可以 [r] 充值 [c]取消 [q]返回 >> ";
        } else {
            cout << "您可以 [b] 支付 [c]取消 [q]返回 >> ";
        }
        char ch = _getch();
        switch (ch) {
        case 'b':
            if (order.pricing <= currentUser->balance) {
                DataManager::getInstance()->order->pay(orderId);
                this->alert("订单支付成功");
                return;
            }
            break;
        case 'r':
            cout << endl;
            this->recharge();
            break;
        case 'c':
            DataManager::getInstance()->order->cancel(orderId);
            this->alert("订单取消成功");
            return;
        case 'q':
            return;
        }
        system("cls");
    }
}
void CLI::show(Product *pn) {
    Product *p = DataManager::getInstance()->product->getProductById(pn->id);
    User *currentUser = DataManager::getInstance()->user->currentUser;
    while (1) {
        system("cls");
        // 复制一份
        bool isowner = false;
        cout << endl;
        this->print(p);
        cout << endl;
        if (currentUser != NULL) {
            cout << "[b]加入购物车 ";
            if (currentUser->getUserType() == MERCHANT && p->owner == currentUser->username) {
                isowner = true;
                cout << "[e]编辑 ";
                cout << "[d]删除 ";
            }
        }
        cout << "[q]返回" << endl;
        char op = '\n';
        while (op == '\n' || op == ' ' || op == '\b') {
            op = _getch();
        }
        if (op == 'q') {
            delete p;
            return;
        } else if (currentUser != NULL && op == 'b') {
            int originalCount = DataManager::getInstance()->user->getInCart(p->id);
            if (p->stock >= originalCount + 1) {
                DataManager::getInstance()->user->addToCart(p->id, 1);
                this->alert("添加购物车成功");
            } else {
                this->alert("库存不足");
            }
            delete p;
            return;
        } else if (isowner && op == 'e') {
            this->addOrEdit(p);
            delete p;
            return;
        } else if (isowner && op == 'd') {
            if (DataManager::getInstance()->product->deleteProduct(p)) {
                this->alert("删除成功");
            } else {
                this->alert("删除失败");
            }
            delete p;
            return;
        } else {
            this->alert("错误输入");
        }
    }
    delete p;
}
void CLI::buy(Product *p) {
    User *currentUser = DataManager::getInstance()->user->currentUser;
    if (currentUser->balance < p->getRealPrice()) {
        this->alert("余额不足");
    } else if (p->stock <= 0) {
        this->alert("库存不足");
    } else {
        bool ret = DataManager::getInstance()->order->buy(p, 1);
        if (ret) {
            this->alert("购买成功");
        } else {
            this->alert("购买失败");
        }
    }
}
void CLI::addOrEdit(Product *p) {
    Product *np = NULL;
    if (p == NULL) {
        cout << "创建商品";
    } else {
        cout << "编辑商品" << p->id;
    }
    cout << "：以下所有输入后都需要回车确认" << endl;

    /* 分类 */
    cout << "请选择产品分类：[b]图书 [f]食品 [c]衣物 ";
    if (p != NULL) {
        cout << "[其他]保持不变 ";
    } else {
        cout << "[其他]图书 ";
    }
    cout << ">> ";
    string op;
    cin >> op;
    PRODUCTTYPE ptype = BOOK;
    if (op == "b") {
        ptype = BOOK;
    } else if (op == "f") {
        ptype = FOOD;
    } else if (op == "c") {
        ptype = CLOTH;
    } else if (p != NULL) {
        ptype = p->getProductType();
    }
    switch (ptype) {
    case BOOK:
        np = new ProductBook;
        break;
    case FOOD:
        np = new ProductFood;
        break;
    case CLOTH:
        np = new ProductCloth;
        break;
    }

    /* 品名 */
    cout << "商品名称 >> ";
    if (p != NULL) {
        np->name = p->name;
        np->id = p->id;
    } else {
        np->name = "";
    }
    this->editorInput(&np->name);

    /* 描述 */
    cout << "商品描述 >> ";
    if (p != NULL) {
        np->description = p->description;
    } else {
        np->description = "";
    }
    this->editorInput(&np->description);

    /* 价格 */
    cout << "商品价格 >> ";
    string sprice = "";
    if (p != NULL) {
        sprice = this->to_string((float)p->pricing / 100);
    }
    this->editorInput(&sprice);
    try {
        np->pricing = (int)(stof(sprice) * 100);
    } catch (...) {
        cout << "输入有误，当前值已设置为0";
        np->pricing = 0;
    }

    /* 库存 */
    cout << "商品库存 >> ";
    string sstock = "";
    if (p != NULL) {
        sstock = this->to_string(p->stock, 0);
    }
    this->editorInput(&sstock);
    try {
        np->stock = stoi(sstock);
    } catch (...) {
        cout << "输入有误，当前值已设置为0";
        np->stock = 0;
    }

    cout << endl;
    this->print(np);
    cout << "以上为修改结果，[y]保存 [n]取消";
    while (1) {
        char op = _getch();
        if (op == 'y') {
            if (p != NULL) {
                if (DataManager::getInstance()->product->replaceProduct(np)) {
                    this->alert("修改成功");
                } else {
                    this->alert("系统错误");
                }
            } else {
                if (DataManager::getInstance()->product->addProduct(np)) {
                    this->alert("添加成功");
                } else {
                    this->alert("系统错误");
                }
            }
            delete np;
            return;
        } else if (op == 'n') {
            cout << endl;
            delete np;
            return;
        } else {
            this->alert("错误输入");
        }
    }
}