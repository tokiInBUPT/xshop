#include "user.h"
#include <QByteArray>
#include <QCryptographicHash>

User::User() {
    this->balance = 0;
}
void User::setPassword(string password) {
    QByteArray hash = QCryptographicHash::hash(QByteArray::fromStdString(password), QCryptographicHash::Sha256);
    this->passwordHash = hash.toHex().toStdString();
}