#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QMenuBar>
#include <QJsonArray>
#include <QDesktopServices>
#include <QTemporaryFile>
#include "NetworkManager.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), _networkManager(new NetworkManager(this, "127.0.0.1", 3080))
{
    // Setup window
    ui->setupUi(this);
    ui->init_stack->setCurrentIndex(0);
    ui->app_stack->setCurrentIndex(0);
    currentTheme = "dark";
    initStyle(currentTheme);
    initFlags();

    // Setup Networking
    initHttpClient("127.0.0.1", 3080);

    // Connect remaining signal handlers
    connect(ui->color_theme_toggle, SIGNAL(clicked()), this, SLOT(toggleColorTheme()));
    connect(ui->open_documentation_button, SIGNAL(clicked()), this, SLOT(openDocumentation()));
    connect(ui->login_button, SIGNAL(clicked()), this, SLOT(on_login_button_Clicked()));
    connect(ui->back_button, SIGNAL(clicked()), this, SLOT(on_back_button_Clicked()));
    connect(ui->register_applicant_button, SIGNAL(clicked()), this, SLOT(on_register_applicant_button_Clicked()));
    connect(ui->view_applications_button, SIGNAL(clicked()), this, SLOT(on_view_applications_button_Clicked()));
    connect(ui->review_application_button, SIGNAL(clicked()), this, SLOT(on_review_application_button_Clicked()));
    connect(ui->app_submit_button, SIGNAL(clicked()), this, SLOT(on_app_submit_button_Clicked()));
    connect(ui->app_stack, SIGNAL(currentChanged(int)), this, SLOT(on_app_page_Changed()));
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

void MainWindow::toggleColorTheme()
{
    if (currentTheme == "dark") currentTheme = "light";
    else if (currentTheme == "light") currentTheme = "dark";

    initStyle(currentTheme);
}

void MainWindow::openDocumentation()
{
    QString guideHtmlPath = ":/resources/web/user-guide.html";
    QFile resourceFile(":/resources/web/user-guide.html");
    if (resourceFile.open(QIODevice::ReadOnly))
    {
        QString filePath = "user-guide.html";
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(resourceFile.readAll());
            resourceFile.close();
            file.close();
            QUrl url = QUrl::fromLocalFile(filePath);
            if (QDesktopServices::openUrl(url))
            {
                return;
            }
        }
    }
    QMessageBox::warning(this, "Warning", "Unable to locate help file");
}

void MainWindow::on_login_button_Clicked()
{
    // Debugging shortcut: Skip login if "debug" is entered
    if (ui->email_field->text() == "debug")
    {
        navigateTo(ui->application_page);
        return;
    }
    QJsonObject jsonObj;
    jsonObj["user_type"] = "internal";
    jsonObj["email"] = ui->email_field->text();
    jsonObj["password"] = ui->password_field->text();
    QJsonDocument jsonDoc(jsonObj);
    QByteArray data = jsonDoc.toJson();
    qDebug() << sendHttpRequest(HTTP_METHOD::POST, "/login", data);
}

void MainWindow::on_register_applicant_button_Clicked()
{
    // Debugging shortcut: Skip login if "debug" is entered
    if (ui->email_field->text() == "debug")
    {
        navigateTo(ui->register_applicant_page);
        return;
    }
    navigateTo(ui->register_applicant_page);
    // TODO: Create logic to check if user is priviliged to enter certain tab
    // QJsonObject jsonObj;
    // jsonObj["email"] = ui->email_field->text();
    // jsonObj["password"] = ui->password_field->text();
    // QJsonDocument jsonDoc(jsonObj);
    // QByteArray data = jsonDoc.toJson();
    // sendHttpRequest(HTTP_METHOD::POST, "login", data);
}

void MainWindow::on_view_applications_button_Clicked()
{
    // Debugging shortcut: Skip login if "debug" is entered
    // if (ui->email_field->text() == "debug") {
    //     ui->app_stack->setCurrentIndex(1);
    //     return;
    // }
    // TODO: Create logic to check if user is priviliged to enter certain tab
    navigateTo(ui->view_applications_page);
    updateApplicationsList();
}

void MainWindow::updateApplicationsList()
{
    // sendHttpRequest(HTTP_METHOD::GET, "/application", nullptr);
    QString response = _networkManager->get("/application", nullptr)->readAll();
    qDebug() << "Success: " << response;
    QJsonObject json = QJsonDocument::fromJson(response.toUtf8()).object();
    QJsonArray jsonArray = json["data"].toArray();

    ui->applications_table->setRowCount(jsonArray.size());
    int i = 0;
    for (const QJsonValue &value : jsonArray)
    {
        QJsonObject obj = value.toObject();
        int applicationId = obj.contains("application_id") ? obj["application_id"].toInt() : -1;
        QString fName = obj.contains("f_name") ? obj["f_name"].toString() : "";
        QString lName = obj.contains("l_name") ? obj["l_name"].toString() : "";
        QString status = obj.contains("status") ? obj["status"].toString() : "";
        QDateTime submissionDate = obj.contains("submission_date") ? QDateTime::fromString(obj["submission_date"].toString(), Qt::ISODate).toTimeZone(QTimeZone("America/Belize")) : QDateTime();

        ui->applications_table->setItem(i, 0, new QTableWidgetItem(QString::number(applicationId)));
        ui->applications_table->setItem(i, 1, new QTableWidgetItem(fName));
        ui->applications_table->setItem(i, 2, new QTableWidgetItem(lName));
        ui->applications_table->setItem(i, 3, new QTableWidgetItem(submissionDate.toString("yyyy-MM-dd")));
        ui->applications_table->setItem(i, 4, new QTableWidgetItem(status));

        i++;
    }
    // ui->applications_table->resizeColumnsToContents();
}

void MainWindow::on_app_submit_button_Clicked()
{
    // TODO: Encapsulate debug skipping in compilation conditionals
    // Debugging shortcut: Skip login if "debug" is entered
    QJsonObject jsonObj;
    jsonObj["user_type"] = "external";
    jsonObj["fName"] = ui->app_fname_field->text();
    jsonObj["lName"] = ui->app_lname_field->text();
    jsonObj["email"] = ui->app_email_field->text();
    jsonObj["dateOfBirth"] = ui->app_dob_field->date().toString("yyyy-MM-dd");
    QJsonDocument jsonDoc(jsonObj);
    QByteArray data = jsonDoc.toJson();
    qDebug() << "DATA: " << data;

    QByteArray response = _networkManager->post("/application", data)->readAll();
    qDebug() << "REPLY FROM HTTP REQUEST: " << response;
}

void MainWindow::on_review_application_button_Clicked()
{
    auto selectedItems = ui->applications_table->selectedItems();
    if (!selectedItems.isEmpty())
    {
        for (auto item : ui->applications_table->selectedItems())
        {
            qDebug() << item->text();
        }
        navigateTo(ui->review_page);
        ui->review_fname_field->setText(selectedItems[1]->text());
        ui->review_lname_field->setText(selectedItems[2]->text());
    }
}

void MainWindow::initStyle(QString theme)
{
    // Load Stylesheet
    QString resourcePath = ":/resources/style/app-<theme>.qss";
    resourcePath.replace("<theme>", theme);

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

    // connect(
    //     networkManager,
    //     SIGNAL(finished(QNetworkReply *)),
    //     this,
    //     SLOT(on_networkManager_Finished(QNetworkReply *))
    // );
}

void MainWindow::on_networkManager_Finished(QNetworkReply *reply)
{
    networkReply = reply;
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
    QJsonObject json = QJsonDocument::fromJson(response.toUtf8()).object();
    QJsonArray jsonArray = json["data"].toArray();

    if (reply->url().path().endsWith("/login"))
    {
        navigateTo(ui->application_page);
        on_app_page_Changed();
    }
    if (reply->url().path().endsWith("/application"))
    {
        int http_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << http_code;
        if (http_code == 200)
        {
            // qDebug() << "APPLICATIONS: " << json;
            // ui->applications_table->setRowCount(jsonArray.size());
            // ui->applications_table->resizeColumnsToContents();
            // int i = 0;
            // for (const QJsonValue &value : jsonArray) {
            //     QJsonObject obj = value.toObject();
            //     int applicationId = obj.contains("application_id") ? obj["application_id"].toInt() : -1;
            //     QString fName = obj.contains("f_name") ? obj["f_name"].toString() : "";
            //     QString lName = obj.contains("l_name") ? obj["l_name"].toString() : "";
            //     QString status = obj.contains("status") ? obj["status"].toString() : "";
            //     QDateTime submissionDate = obj.contains("submission_date") ?
            //         QDateTime::fromString(obj["submission_date"].toString(), Qt::ISODate).toTimeZone(QTimeZone("America/Belize")): QDateTime();

            //     ui->applications_table->setItem(i, 0, new QTableWidgetItem(QString::number(applicationId)));
            //     ui->applications_table->setItem(i, 1, new QTableWidgetItem(fName));
            //     ui->applications_table->setItem(i, 2, new QTableWidgetItem(lName));
            //     ui->applications_table->setItem(i, 3, new QTableWidgetItem(submissionDate.toString("yyyy-MM-dd")));
            //     ui->applications_table->setItem(i, 4, new QTableWidgetItem(status));

            //     i++;
            // }
        }
        else
        {
            QMessageBox messageBox;
            messageBox.critical(this, "Error", response);
            messageBox.setFixedSize(500, 200);
        }
        // ui->welcome_page_heading->setText(ui->welcome_page_heading->text() + "user");
    }

    reply->deleteLater();
}

QNetworkReply *MainWindow::sendHttpRequest(HTTP_METHOD method, QString endpoint, QByteArray jsonObj)
{
    QNetworkRequest networkRequest;
    QUrl url;
    url.setScheme("http");
    url.setHost(hostIP);
    url.setPort(hostPort);
    url.setPath(endpoint);
    qDebug() << "Http request: " << url.toString();
    networkRequest.setUrl(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    switch (method)
    {
    case HTTP_METHOD::GET:
        return networkManager->get(networkRequest);
        break;
    case HTTP_METHOD::POST:
        return networkManager->post(networkRequest, jsonObj);
        break;
    default:
        return nullptr;
        break;
    }
}

void MainWindow::on_back_button_Clicked()
{
    QWidget *currentPage = navigationStack.top();
    // If user is on home page, use the initial QStackedWidget
    if (currentPage == ui->home_page)
    {
        ui->init_stack->setCurrentWidget(ui->login_page);
    }
    else
    {
        navigateBack();
    }
}

void MainWindow::navigateTo(QWidget *toPage)
{
    qDebug() << "Navigating to:" << toPage->objectName();

    QWidget *currentPage = navigationStack.isEmpty() ? nullptr : navigationStack.top();

    // Determine if the target page belongs to init_stack
    if (ui->init_stack->indexOf(toPage) != -1)
    {
        ui->init_stack->setCurrentWidget(toPage);
        navigationStack.push(ui->home_page);
        return; // Don't modify navigationStack for init_stack
    }

    // Only track navigation if inside app_stack
    if (ui->app_stack->indexOf(toPage) != -1)
    {
        if (currentPage != toPage)
        {
            navigationStack.push(toPage);
        }
        ui->app_stack->setCurrentWidget(toPage);
    }
}

void MainWindow::navigateBack()
{
    qDebug() << "STACK:" << navigationStack << "\n";
    qDebug() << "init_stack:" << ui->init_stack->children() << "\n";
    qDebug() << "app_stack:" << ui->app_stack->children() << "\n";
    if (navigationStack.size() > 1)
    {
        navigationStack.pop(); // Remove current page
        QWidget *lastPage = navigationStack.top();
        ui->app_stack->setCurrentWidget(lastPage);
        qDebug() << "Navigating to:" << lastPage->objectName();
    }
}

void MainWindow::on_app_page_Changed()
{
    QWidget *currentPage = navigationStack.isEmpty() ? nullptr : navigationStack.top();
    if (currentPage == ui->home_page)
    {
        ui->app_header->setText("Welcome Back, " + ui->email_field->text());
    }
    if (currentPage == ui->view_applications_page)
    {
        ui->app_header->setText("View Applications");
    }
    if (currentPage == ui->register_applicant_page)
    {
        ui->app_header->setText("Register Applicant");
    }
    if (currentPage == ui->review_page)
    {
        ui->app_header->setText("Review Application");
    }
}