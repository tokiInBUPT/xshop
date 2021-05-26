#include "../../Common/product.h"
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
    void setDiscount();
    void show(Product *p);
    void print(Product *p);
    void buy(Product *p);
    void alert(string s);
    void addOrEdit(Product *p);
    void editorInput(string *s);
    string to_string(const double val, const int n = 2);
};