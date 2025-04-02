#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QStack>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void on_networkManager_Finished(QNetworkReply *reply);
    void on_login_button_Clicked();
    void on_register_applicant_button_Clicked();
    void on_view_applications_button_Clicked();
    void on_app_submit_button_Clicked();
    void on_review_application_button_Clicked();
    void on_back_button_Clicked();

private:
    Ui::MainWindow *ui;
    Qt::WindowFlags flags;
    QStack<QWidget*> navigationStack;
    QNetworkAccessManager *networkManager;
    QNetworkRequest networkRequest;
    QString hostIP;
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
    void initStyle();
    void initHttpClient(QString hostIP, int hostPort);
    void sendHttpRequest(HTTP_METHOD METHOD, QString endpoint, QByteArray jsonObj);
    void navigateTo(QWidget* toPage);
    void navigateBack();
};
#endif // MAINWINDOW_H
