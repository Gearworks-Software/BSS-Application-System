#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QMenuBar>
#include <QJsonArray>
#include <QDesktopServices>
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
    connect(ui->app_stack, SIGNAL(currentChanged(int)), this, SLOT(on_app_page_Changed()));

    connect(ui->register_applicant_button, SIGNAL(clicked()), this, SLOT(on_register_applicant_button_Clicked()));
    connect(ui->app_submit_button, SIGNAL(clicked()), this, SLOT(on_app_submit_button_Clicked()));
    connect(ui->scan_document_button, SIGNAL(clicked()), this, SLOT(on_scan_document_Clicked()));

    connect(ui->refresh_button, SIGNAL(clicked()), this, SLOT(updateApplicationsList()));
    connect(ui->review_accept_button, SIGNAL(clicked()), this, SLOT(on_accept_button_Clicked()));
    connect(ui->review_reject_button, SIGNAL(clicked()), this, SLOT(on_reject_button_Clicked()));
    connect(ui->view_applications_button, SIGNAL(clicked()), this, SLOT(on_view_applications_button_Clicked()));
    connect(ui->review_application_button, SIGNAL(clicked()), this, SLOT(on_review_application_button_Clicked()));
    connect(ui->refresh_camera, SIGNAL(clicked()), this, SLOT(on_refresh_camera_button_Clicked()));
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
    if (currentTheme == "dark")
        currentTheme = "light";
    else if (currentTheme == "light")
        currentTheme = "dark";

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
    auto reply = _networkManager->post("/login", data);
    if (!reply)
    {
        QMessageBox::critical(this, "Login Error", "Request timed out or failed.");
        return;
    }

    if (reply->error())
    {
        QMessageBox::critical(this, "Login Error", reply->readAll());
    }
    else
    {
        QString response = reply->readAll();
        qDebug() << "Login successful: " << response;
        navigateTo(ui->application_page);
    }
    reply->deleteLater();
}

void MainWindow::on_register_applicant_button_Clicked()
{
    navigateTo(ui->register_applicant_page);

    // Ensure the camera is properly initialized and managed
    if (!camera)
    {
        const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
        for (const QCameraDevice &cameraDevice : cameras)
        {
            qDebug() << cameraDevice.description();
            if (cameraDevice.description() == "Integrated Camera")
            {
                camera = new QCamera(cameraDevice, this);
                break;
            }
        }
    }

    if (camera)
    {
        camera->start(); // Start the camera
        ui->video_widget->show();
        // ui->video_widget->resize(640, 480);

        // Configure the media capture session
        mediaCaptureSession.setCamera(camera);
        mediaCaptureSession.setVideoOutput(ui->video_widget);
    }
    else
    {
        qDebug() << "No camera found or failed to initialize.";
        QMessageBox::warning(this, "Camera Error", "Unable to initialize the camera.");
    }
}
void MainWindow::on_scan_document_Clicked()
{
    // Ensure the camera is active
    if (!camera || !camera->isActive())
    {
        QMessageBox::warning(this, "Camera Error", "Camera is not active.");
        return;
    }

    // Initialize QImageCapture if not already done
    if (!imageCapture)
    {
        imageCapture = new QImageCapture(this);
        mediaCaptureSession.setImageCapture(imageCapture);
    }

    // Capture the image
    if (imageCapture->isReadyForCapture())
    {
        connect(imageCapture, &QImageCapture::imageCaptured, this, [this](int id, const QImage &image)
            {
                Q_UNUSED(id);

                // Pause the camera
                if (camera->isActive())
                {
                    camera->stop();
                }

                // Store the captured image
                QBuffer buffer(&currentImageData);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "JPEG"); // Save the image to the buffer in JPEG format

                // Display the captured image in the video widget
                QPixmap pixmap = QPixmap::fromImage(image);
                QLabel *imagePreview = new QLabel(ui->video_widget);
                imagePreview->setPixmap(pixmap.scaled(ui->video_widget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                imagePreview->setAlignment(Qt::AlignCenter);
                imagePreview->setStyleSheet("background-color: black;");
                imagePreview->show();

                // QMessageBox::information(this, "Image Captured", "Image has been captured and displayed.");
            });

        imageCapture->capture();
    }
    else
    {
        QMessageBox::warning(this, "Capture Error", "Image capture is not ready.");
    }
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
    currentLoadedApplications = json["data"].toArray();

    ui->applications_table->setRowCount(currentLoadedApplications.size());
    int i = 0;
    for (const QJsonValue &value : currentLoadedApplications)
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
    QJsonObject jsonObj;
    jsonObj["user_type"] = "external";
    jsonObj["fName"] = ui->app_fname_field->text();
    jsonObj["lName"] = ui->app_lname_field->text();
    jsonObj["email"] = ui->app_email_field->text();
    jsonObj["dateOfBirth"] = ui->app_dob_field->date().toString("yyyy-MM-dd");
    jsonObj["document"] = QString::fromLatin1(currentImageData.toBase64());
    QJsonDocument jsonDoc(jsonObj);
    QByteArray data = jsonDoc.toJson();

    auto reply = _networkManager->post("/application", data);
    if (!reply)
    {
        QMessageBox::critical(this, "Submission Error", "Request timed out or failed.");
        return;
    }

    if (reply->error())
    {
        QMessageBox::critical(this, "Submission Error", reply->readAll());
    }
    else
    {
        QString response = reply->readAll();
        qDebug() << "Application submitted successfully: " << response;
        QMessageBox::information(this, "Success!", response);
    }
    reply->deleteLater();
}

void MainWindow::on_review_application_button_Clicked()
{
    QList<QTableWidgetItem *> selectedItems = ui->applications_table->selectedItems();
    QJsonObject obj;
    for (const QJsonValue &value : currentLoadedApplications)
    {
        obj = value.toObject();
        if (obj["application_id"].toInt() == selectedItems[0]->text().toInt()) break;
    }

    if (!selectedItems.isEmpty())
    {
        navigateTo(ui->review_page);
        ui->review_fname_field->setText(obj["f_name"].toString());
        ui->review_lname_field->setText(obj["l_name"].toString());
        ui->review_email_field->setText(obj["email"].toString());
        ui->review_dob_field->setText(QDateTime::fromString(obj["date_of_birth"].toString(), Qt::ISODate).toTimeZone(QTimeZone("America/Belize")).toString("yyyy-MM-dd"));

        // Decode the document_data and display it in the review_document_widget
        QByteArray imageData = QByteArray::fromBase64(obj["document_data"].toString().toLatin1());/*.toBase64();*/
        QImage image;
        // qDebug() << imageData.to;
        if (image.loadFromData(imageData, "JPEG"))
        {
            QPixmap pixmap = QPixmap::fromImage(image);
            QLabel *imagePreview = new QLabel(ui->review_document_widget);
            imagePreview->setPixmap(pixmap.scaled(ui->review_document_widget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            imagePreview->setAlignment(Qt::AlignCenter);
            imagePreview->setStyleSheet("background-color: black;");
            imagePreview->show();
        }
        else
        {
            qDebug() << "Failed to load image from document_data.";
        }
    }
}

void MainWindow::on_accept_button_Clicked()
{
    // Post accept request
    QList<QTableWidgetItem *> selectedItems = ui->applications_table->selectedItems();
    QJsonObject obj;
    for (const QJsonValue &value : currentLoadedApplications)
    {
        obj = value.toObject();
        if (obj["application_id"].toInt() == selectedItems[0]->text().toInt()) break;
    }

    // Post accept request
    qDebug() << "Accepting application..." << obj["application_id"];
    QJsonObject jsonObj;
    jsonObj["application_id"] = obj["application_id"];
    jsonObj["status"] = "Accepted";
    QJsonDocument jsonDoc(jsonObj);
    QByteArray data = jsonDoc.toJson();

    auto reply = _networkManager->put("/application", data);
    if (!reply)
    {
        QMessageBox::critical(this, "Submission Error", "Request timed out or failed.");
        return;
    }

    if (reply->error())
    {
        QMessageBox::critical(this, "Submission Error", reply->readAll());
    }
    else
    {
        QString response = reply->readAll();
        qDebug() << "Application accepted successfully: " << response;
        QMessageBox::information(this, "", response);
        navigateBack();
    }
    reply->deleteLater();
}

void MainWindow::on_reject_button_Clicked()
{
    // Post accept request
    QList<QTableWidgetItem *> selectedItems = ui->applications_table->selectedItems();
    QJsonObject obj;
    for (const QJsonValue &value : currentLoadedApplications)
    {
        obj = value.toObject();
        if (obj["application_id"].toInt() == selectedItems[0]->text().toInt()) break;
    }

    // Post accept request
    qDebug() << "Rejecting application..." << obj["application_id"];
    QJsonObject jsonObj;
    jsonObj["application_id"] = obj["application_id"];
    jsonObj["status"] = "Rejected";
    QJsonDocument jsonDoc(jsonObj);
    QByteArray data = jsonDoc.toJson();

    auto reply = _networkManager->put("/application", data);
    if (!reply)
    {
        QMessageBox::critical(this, "Submission Error", "Request timed out or failed.");
        return;
    }

    if (reply->error())
    {
        QMessageBox::critical(this, "Submission Error", reply->readAll());
    }
    else
    {
        QString response = reply->readAll();
        qDebug() << "Application rejected successfully: " << response;
        QMessageBox::information(this, "Success!", response);
        navigateBack();
    }
    reply->deleteLater();
}

int MainWindow::getSelectedApplicationId()
{
    QList<QTableWidgetItem *> selectedItems = ui->applications_table->selectedItems();
    if (!selectedItems.isEmpty()) {
        return selectedItems[0]->text().toInt();
    }
    return -1; // Return an invalid ID if no application is selected
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

    if (currentPage == ui->register_applicant_page)
    {
        if (camera->isActive())
        {
            camera->stop();
            ui->video_widget->hide();
        }
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
void MainWindow::on_refresh_camera_button_Clicked()
{
    // Check if the camera exists and is not active
    if (camera && !camera->isActive())
    {
        camera->start();          // Restart the camera
        ui->video_widget->show(); // Ensure the video widget is visible
        qDebug() << "Camera restarted.";
    }
    // } else if (!camera) {
    //     QMessageBox::warning(this, "Camera Error", "No camera is initialized.");
    // } else {
    //     return;
    //     // QMessageBox::information(this, "Camera Active", "The camera is already running.");
    // }
}