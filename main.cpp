#include "Data/DataManager.h"
#include "UI/GUI/mainwindow.h"
#include "UI/CLI/cli.h"
#include <QApplication>
#include <memory>

/* DataManager作为单例模式存在 */
std::shared_ptr<DataManager> DataManager::instance(new DataManager(), DataManager::Destory);

int main(int argc, char *argv[]) {
    /* Init DataManager */
    shared_ptr<DataManager> d = DataManager::getInstance();
    d->init("../../_data/_xshop_user.json", "../../_data/_xshop_product.json", "../../_data/_xshop_order.json");
    
    // CLI
    new CLI;
    return 0;

    // GUI没画好，暂不使用
    /*
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
    */
}
