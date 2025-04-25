#include "NetworkManager.h"
#include <QJsonObject>
#include <QUrlQuery>
#include <QEventLoop>

NetworkManager::NetworkManager(QObject* parent, QString host, int port)
    : QObject(parent), networkAccessManager(new QNetworkAccessManager(this))
{
    setHost(host);
    setPort(port);
}

QNetworkReply* NetworkManager::post(QString endpoint, QByteArray body, QJsonObject *query)
{
    QUrl targetUrl = url;
    targetUrl.setScheme("http");
    url.setPath(endpoint);

    QNetworkRequest request(targetUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkAccessManager->post(request, body);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return reply;
}

QNetworkReply* NetworkManager::get(QString endpoint, QJsonObject *query)
{
    QUrl targetUrl = url;
    targetUrl.setScheme("http");
    url.setPath(endpoint);

    if (query) {
        QUrlQuery urlQuery;
        for (auto q = query->begin(); q != query->end(); ++q) {
            urlQuery.addQueryItem(q.key(), q.value().toString());
        }
        targetUrl.setQuery(urlQuery);
    }

    QNetworkRequest request(targetUrl);
    QNetworkReply *reply = networkAccessManager->get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    return reply;
}

void NetworkManager::setPort(int port)
{
    url.setPort(port);
}

void NetworkManager::setHost(QString host)
{
    url.setHost(host);
}