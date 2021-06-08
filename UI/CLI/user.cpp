#include "cli.h"
#include "../../Data/DataManager.h"
#include <codecvt>
#include <locale>
#include <map>
#include <sstream>
using namespace std;
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