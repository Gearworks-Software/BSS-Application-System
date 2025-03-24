#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Load Stylesheet
    // Use QFile::exists to check if the resource is accessible.
    QString resourcePath = ":/resources/style/app.qss";
    if (QFile::exists(resourcePath)) {
        qDebug() << "Resource exists:" << resourcePath;
    } else {
        qDebug() << "Resource NOT found:" << resourcePath;
    }
    QFile file(resourcePath);
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qDebug() << styleSheet;
    this->setStyleSheet(styleSheet);

    connect(ui->login_button, SIGNAL(clicked()), this, SLOT(on_login_button_Clicked()));
    std::cout << "test";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_login_button_Clicked()
{
    qDebug() << "Login button clicked";
}
