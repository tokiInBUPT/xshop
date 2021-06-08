#include "cli.h"
#include "../../Data/DataManager.h"
#include <codecvt>
#include <locale>
#include <map>
#include <sstream>
using namespace std;
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
void CLI::printCart(Product *p, int quantity) {
    string cName = "？";
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
    cout << "[" << cName << "] ";
    cout << p->name << endl;
    cout << "数量：" << quantity;
    cout << "\t库存：" << p->stock;
    cout << "\t原价：" << (float)(p->pricing) / 100.0 << "\t折后：" << (float)p->getRealPrice() / 100.0 << endl;
}

void CLI::printOrder(Product *p, int quantity, int price) {
    string cName = "？";
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
    cout << "[" << cName << "] ";
    cout << p->name << endl;
    cout << "数量：" << quantity;
    cout << "\t库存：" << p->stock;
    cout << "\t原价：" << (float)(p->pricing) / 100.0 << "\t快照价格：" << (float)price / 100.0 << endl;
}