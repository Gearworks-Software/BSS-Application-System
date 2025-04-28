#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QStack>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtMultimedia>
#include <QPushButton>
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
	class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

private slots:
	void on_networkManager_Finished(QNetworkReply *reply);
	void toggleColorTheme();
	void openDocumentation();
	void on_login_button_Clicked();
	void on_register_applicant_button_Clicked();
	void on_view_applications_button_Clicked();
	void updateApplicationsList();
	void on_app_submit_button_Clicked();
	void on_scan_document_Clicked();
	void on_refresh_camera_button_Clicked();
	void on_review_application_button_Clicked();
	void on_accept_button_Clicked();
	void on_reject_button_Clicked();
	void on_back_button_Clicked();
	void on_app_page_Changed();

private:
	Ui::MainWindow *ui;
	Qt::WindowFlags flags;
	QStack<QWidget *> navigationStack;
	NetworkManager *_networkManager;
	QNetworkAccessManager *networkManager;
	QNetworkReply *networkReply;
	QString hostIP;
	QString currentTheme;
    QCamera *camera = nullptr;
    QMediaCaptureSession mediaCaptureSession;
	QImageCapture *imageCapture = nullptr;
	QByteArray currentImageData;
	QJsonArray currentLoadedApplications;
	int hostPort;
	enum class HTTP_METHOD
	{
		GET,
		POST,
		DELETE
	};

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();  
	void initFlags();
	void initStyle(QString theme);
	void navigateTo(QWidget *toPage);
	void navigateBack();
	void initHttpClient(QString hostIP, int hostPort);
	QNetworkReply *sendHttpRequest(HTTP_METHOD METHOD, QString endpoint, QByteArray jsonObj);
	int getSelectedApplicationId();
};

#endif // MAINWINDOW_H