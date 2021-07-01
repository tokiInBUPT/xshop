#include "Common/Protocol/http.h"
#include "Data/Client/httpclient.h"
#include "Data/DataManager.h"
#include "UI/CLI/cli.h"
#include "UI/GUI/mainwindow.h"
#include "UI/Server/Server.h"
#include "WinSock2.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <memory>

/* DataManager作为单例模式存在 */
std::shared_ptr<DataManager> DataManager::instance(new DataManager(), DataManager::Destory);

int main(int argc, char *argv[]) {
    /* Init CommandLine */
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("xShop");
    QCoreApplication::setApplicationVersion("stage-3");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription("\nSimple shop app by xyToki 2021.06");
    parser.addOptions({
        {{"s", "server"}, "Work as server."},
        {{"c", "client"}, "Work as client."},
        {{"l", "local"}, "Work standalone."},
        {{"p", "port"}, "Set port for server. Default 3010.", "port"},
        {{"r", "remote"}, "Set remote for client. Default http://127.0.0.1:3010/", "remote"},
    });
    parser.process(a);
    if (parser.isSet("s") || parser.isSet("l")) {
        /* Init DataManager */
        shared_ptr<DataManager> d = DataManager::getInstance();
        d->init("C:/Users/xytoki/Workspace/Coding/bupt/xshop/xshop/_data/_xshop_user.json",
                "C:/Users/xytoki/Workspace/Coding/bupt/xshop/xshop/_data/_xshop_product.json",
                "C:/Users/xytoki/Workspace/Coding/bupt/xshop/xshop/_data/_xshop_order.json");
    } else if (parser.isSet("c")) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        shared_ptr<DataManager> d = DataManager::getInstance();
        d->init(parser.isSet("r") ? parser.value("r").toStdString() : "http://127.0.0.1:3010/");
    }
    if (parser.isSet("s")) {
        int port = parser.isSet("p") ? parser.value("p").toInt() : 3010;
        ShopServer server;
        cout << "xShop stage-3 server listening on http://0.0.0.0:" << port << endl;
        server.listen(QHostAddress::Any, port);
        return a.exec();
    } else if (parser.isSet("l") || parser.isSet("c")) {
        new CLI;
        return 0;
    } else {
        parser.showHelp();
        return -1;
    }
}
