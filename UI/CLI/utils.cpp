#include "cli.h"
#include "../../Data/DataManager.h"
#include <codecvt>
#include <locale>
#include <map>
#include <sstream>
using namespace std;
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