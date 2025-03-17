#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->login_button, SIGNAL(clicked()),this,SLOT(on_login_button_Clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_login_button_Clicked()
{
    qDebug() << "Login button clicked";
}