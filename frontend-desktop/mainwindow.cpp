#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // Setup window
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    initStyle();
    initFlags();

    // Setup Networking
    initHttpClient("127.0.0.1", 3080);

    // Connect remaining signal handlers
    connect(ui->login_button, SIGNAL(clicked()), this, SLOT(on_login_button_Clicked()));
    QList<QPushButton *> buttons = this->findChildren<QPushButton *>();
    for (QPushButton *button : buttons)
    {
        // qDebug() << "debug: " << button->text() << button->property("isBackButton").toBool();
        if (button->property("isBackButton").toBool())
        {
            // qDebug() << "debug: isbackbutton";
            connect(button, SIGNAL(clicked()), this, SLOT(on_back_button_Clicked()));
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initFlags()
{
    flags = windowFlags();
    // setWindowFlags(flags | Qt::SplashScreen);
}

void MainWindow::on_login_button_Clicked()
{
    QJsonObject jsonObj;
    jsonObj["email"] = ui->email_field->text();
    jsonObj["password"] = ui->password_field->text();
    QJsonDocument jsonDoc(jsonObj);
    QByteArray data = jsonDoc.toJson();
    sendHttpRequest(HTTP_METHOD::POST, "login", data);
}

void MainWindow::initStyle()
{
    // Load Stylesheet
    QString resourcePath = ":/resources/style/app.qss";
    if (QFile::exists(resourcePath))
    {
        qDebug() << "Resource exists:" << resourcePath;
    }
    else
    {
        qDebug() << "Resource NOT found:" << resourcePath;
    }
    QFile file(resourcePath);
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);
}

void MainWindow::initHttpClient(QString hostIP, int hostPort)
{
    this->hostIP = hostIP;
    this->hostPort = hostPort;
    networkManager = new QNetworkAccessManager();
    connect(
        networkManager,
        SIGNAL(finished(QNetworkReply *)),
        this,
        SLOT(on_networkManager_Finished(QNetworkReply *))
    );
}

void MainWindow::on_networkManager_Finished(QNetworkReply *reply)
{
    if (reply->error())
    {
        QString errorMessage = reply->readAll();
        qDebug() << "Error: " << errorMessage;
        QMessageBox messageBox;
        messageBox.critical(this, "Error", errorMessage);
        messageBox.setFixedSize(500, 200);
        return;
    }

    QString response = reply->readAll();
    qDebug() << "Success: " << response;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(response.toUtf8());
    if (networkRequest.url().path().endsWith("login")) 
    {
        ui->stackedWidget->setCurrentIndex(1);
        ui->welcome_page_heading->setText(ui->welcome_page_heading->text() + "user");
    }

    reply->deleteLater();
}

void MainWindow::sendHttpRequest(HTTP_METHOD method, QString endpoint, QByteArray jsonObj)
{
    // QString httpRequest = "http://" + hostIP + ":" + hostPort + "/" + endpoint;
    QUrl url;
    url.setScheme("http");
    url.setHost(hostIP);
    url.setPort(hostPort);
    url.setPath("/" + endpoint);
    qDebug() << "Http request: " << url.toString();
    networkRequest.setUrl(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    switch (method)
    {
    case HTTP_METHOD::GET:
        networkManager->get(networkRequest);
        break;
    case HTTP_METHOD::POST:
        networkManager->post(networkRequest, jsonObj);
        break;
    default:
        break;
    }
}

void MainWindow::on_back_button_Clicked()
{
    int currentIndex = ui->stackedWidget->currentIndex();
    if (currentIndex > 0)
    {
        ui->stackedWidget->setCurrentIndex(currentIndex-1);
    }
}