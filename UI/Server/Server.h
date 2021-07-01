#ifndef SERVER_H
#define SERVER_H

#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>

class ShopServer : public QTcpServer {
    Q_OBJECT
public:
    ShopServer(QObject *parent = 0);
    ~ShopServer();
protected slots:
    void onNewConnection();
    void onReadyRead();

private:
    void processData(QTcpSocket *sock);
};

#endif