#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NetworkManager : QObject
{
    Q_OBJECT

    public:
        NetworkManager(QObject *parent, QString host, int port);
        QNetworkReply* get(QString endpoint, QJsonObject *query = nullptr);
        QNetworkReply* post(QString endpoint, QByteArray body, QJsonObject *query = nullptr);
        void setHost(QString host);
        void setPort(int port);

    private:
        QNetworkAccessManager* networkAccessManager;
        QUrl hostUrl;
        
};
#endif // NETWORKMANAGER_H