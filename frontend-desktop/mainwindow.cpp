#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    initFlags();
    ui->setupUi(this);
    
    ui->stackedWidget->setCurrentIndex(0);
    
    initStyle();

    // Connect signal handlers
    connect(ui->login_button, SIGNAL(clicked()), this, SLOT(on_login_button_Clicked()));
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