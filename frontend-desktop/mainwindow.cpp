#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Setup window
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    initStyle();
    initFlags();

    // Setup Networking
    initHttpClient("127.0.0.1", "3080");

    // Connect remaining signal handlers
    connect(ui->login_button, SIGNAL(clicked()), this, SLOT(on_login_button_Clicked()));
    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
    for (QPushButton button in buttons)
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
    qDebug() << "Login button clicked";
    ui->stackedWidget->setCurrentIndex(1);
    ui->welcome_page_heading->setText(ui->welcome_page_heading->text() + "user");
    sendHttpRequest("login");
}

void MainWindow::initStyle()
{
    // Load Stylesheet
    QString resourcePath = ":/resources/style/app.qss";
    if (QFile::exists(resourcePath)) {
        qDebug() << "Resource exists:" << resourcePath;
    } else {
        qDebug() << "Resource NOT found:" << resourcePath;
    }
    QFile file(resourcePath);
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);
}

void MainWindow::initHttpClient(QString hostIP, QString hostPort)
{
    this->hostIP = hostIP;
    this->hostPort = hostPort;
    networkManager = new QNetworkAccessManager();
    QObject::connect(
        networkManager, 
        SIGNAL(finished(QNetworkReply*)), 
        this,
        SLOT(on_networkManager_Finished(QNetworkReply*))
    );
}

void MainWindow::on_networkManager_Finished(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << reply->errorString();
        return;
    }

    QString answer = reply->readAll();

    qDebug() << "Network Manager:" << answer;
}

void MainWindow::sendHttpRequest(QString endpoint) {
    QString httpRequest = "http://"+hostIP+":"+hostPort+"/"+endpoint;
    qDebug() << "Http request: " << httpRequest;
    networkRequest.setUrl(QUrl(httpRequest));
    networkManager->get(networkRequest);
}