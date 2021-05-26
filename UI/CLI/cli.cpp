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
void CLI::reg() {
    cout << "是否要注册商家账户？如是请按Y，否请按N >> ";
    char ch;
    User *user = NULL;
    bool loop = true;
    while (loop && (ch = _getch())) {
        switch (ch) {
        case 3: // Ctrl+C
            exit(0);
            break;
        case 'y':
        case 'Y':
            user = new UserMerchant;
            loop = false;
            cout << "Y";
            break;
        case 'n':
        case 'N':
            user = new UserConsumer;
            loop = false;
            cout << "N";
        default:
            // Continue loop
            break;
        }
    }
    cout << endl;
    cout << "账号 >> ";
    cin >> user->username;
    std::cout << "密码 >> ";
    string password = this->passwordInput();
    user->setPassword(password);
    if (DataManager::getInstance()->user->reg(user)) {
        this->alert("注册成功");
    } else {
        this->alert("系统错误");
    }
    delete user;
}
void CLI::login() {
    string username;
    cout << "账号 >> ";
    cin >> username;
    std::cout << "密码 >> ";
    string password = this->passwordInput();
    if (DataManager::getInstance()->user->login(username, password)) {
        this->alert("登录成功");
    } else {
        this->alert("用户名或密码错误");
    }
}
void CLI::passwd() {
    std::cout << "请输入新密码 >> ";
    string password = this->passwordInput();
    if (DataManager::getInstance()->user->changePassword(password)) {
        this->alert("密码已修改");
    } else {
        this->alert("系统错误");
    }
}
void CLI::setDiscount() {
    UserMerchant *currentUser = (UserMerchant *)DataManager::getInstance()->user->currentUser;
    cout << "请设置 书籍 分类折扣(%) >>";
    string d = this->to_string(currentUser->discount[0] * 100, 2);
    this->editorInput(&d);
    try {
        if (!DataManager::getInstance()->user->setDiscount(BOOK, stof(d) / 100)) {
            this->alert("设置失败！");
            return;
        }
    } catch (...) {
        this->alert("输入有误");
        return;
    }

    cout << "请设置 食品 分类折扣(%) >>";
    d = this->to_string(currentUser->discount[1] * 100, 2);
    this->editorInput(&d);
    try {
        if (!DataManager::getInstance()->user->setDiscount(FOOD, stof(d) / 100)) {
            this->alert("设置失败！");
            return;
        }
    } catch (...) {
        this->alert("输入有误");
        return;
    }

    cout << "请设置 衣物 分类折扣(%) >>";
    d = this->to_string(currentUser->discount[2] * 100, 2);
    this->editorInput(&d);
    try {
        if (!DataManager::getInstance()->user->setDiscount(CLOTH, stof(d) / 100)) {
            this->alert("设置失败！");
            return;
        }
    } catch (...) {
        this->alert("输入有误");
        return;
    }
    this->alert("保存成功");
}
void CLI::recharge() {
    std::cout << "请输入充值金额，如：648.00 >> ";
    float f;
    cin >> f;
    int i = (int)(f * 100);
    if (DataManager::getInstance()->user->addFund(i)) {
        this->alert("充值成功");
    } else {
        this->alert("系统错误");
    }
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
void CLI::print(Product *p) {
    string cName = "未知";
    switch (p->getProductType()) {
    case BOOK:
        cName = "书籍";
        break;
    case FOOD:
        cName = "食品";
        break;
    case CLOTH:
        cName = "衣物";
        break;
    }
    cout << "商品详情："
         << "[" << cName << "] ";
    cout << p->name << endl;
    cout << "库存：" << p->stock;
    cout << "\t原价：" << (float)(p->pricing) / 100.0 << "\t折后：" << (float)p->getRealPrice() / 100.0 << endl;
    cout << p->description << endl;
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
            cout << "[b]购买 ";
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
            this->buy(p);
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
/*
 * 可编辑的输入框函数
 */
void CLI::editorInput(string *s) {
    bool isCtrl = false;
    // 先输出原内容
    cout << *s;
    // 为了处理中文的退格问题，这里直接使用wstring
    wstring ws = wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(*s);
    wchar_t ch;
    while ((ch = _getwch()) != 13) {
        switch (ch) {
        case '\b':
            isCtrl = false;
            if (ws.length() > 0) {
                wchar_t c = ws.at(ws.length() - 1);
                ws.pop_back();
                putchar('\b');
                putchar(' ');
                putchar('\b');
                if (c > 127) {
                    // 中文在是3byte，但是退格只需要退两格
                    putchar('\b');
                    putchar(' ');
                    putchar('\b');
                }
            }
            break;
        case 3: // Ctrl+C
            exit(0);
            break;
        case 0:
            isCtrl = true;
            break;
        default:
            if (isCtrl) {
                // 00 xx 一般是方向键，这里直接丢弃
                isCtrl = false;
                break;
            }
            isCtrl = false;
            ws += ch;
            // wcout可能不输出，所以转为string再输出
            string sch = wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().to_bytes(ch);
            cout << sch;
        }
    }
    // 返回string
    *s = wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().to_bytes(ws);
    cout << endl;
}

string CLI::passwordInput() {
    string password;
    char ch;
    while ((ch = _getch()) != 13) {
        switch (ch) {
        case '\b':
            if (password.length() > 0) {
                password.pop_back();
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
            break;
        case 3: // Ctrl+C
            exit(0);
            break;
        default:
            password += ch;
            std::cout << '*';
        }
    }
    cout << endl;
    return password;
}

void CLI::alert(string s) {
    cout << endl;
    cout << "[提示] " << s << "，按任意键继续" << endl;
    cout << endl;
    _getch();
}
string CLI::to_string(const double val, const int n) {
    ostringstream out;
    out.precision(n);
    out << std::fixed << val;
    return out.str();
}