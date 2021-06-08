#include "../../Common/product.h"
#include "../../Common/user.h"
#include <conio.h>
#include <iostream>
using namespace std;
class CLI {
public:
    CLI();
    string passwordInput();

private:
    bool mainMenu();
    void reg();
    void login();
    void passwd();
    void recharge();
    void list();
    void cart();
    void setDiscount();
    void show(Product *p);
    void print(Product *p);
    void printCart(Product *p, int quantity);
    void printOrder(Product *p, int quantity, int price);
    void buy(Product *p);
    void alert(string s);
    void addOrEdit(Product *p);
    void editCart(USERCART item);
    void showOrder(string id);
    void payOrCancel(string id);
    bool checkout();
    void listOrder();
    void editorInput(string *s);
    string to_string(const double val, const int n = 2);
};