#include "NetworkManager.h"
#include <QJsonObject>
#include <QUrlQuery>
#include <QEventLoop>

NetworkManager::NetworkManager(QObject *parent, QString hostName, int hostPort)
    : QObject(parent),
      networkAccessManager(new QNetworkAccessManager(this)),
      hostUrl("http://" + hostName + ":" + QString::number(hostPort))
{}

QNetworkReply *NetworkManager::post(QString endpoint, QByteArray body, QJsonObject *query)
{
    QUrl targetUrl = hostUrl;
    targetUrl.setPath(endpoint);

    QNetworkRequest request(targetUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkAccessManager->post(request, body);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return reply;
}

QNetworkReply *NetworkManager::get(QString endpoint, QJsonObject *query)
{
    QUrl targetUrl = hostUrl;
    targetUrl.setPath(endpoint);

    if (query)
    {
        QUrlQuery urlQuery;
        for (auto q = query->begin(); q != query->end(); ++q)
        {
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