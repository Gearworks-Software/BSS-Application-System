#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initFlags();
    void initStyle();
    void initHttpClient(QString hostIP, QString hostPort);
    void sendHttpRequest(QString endpoint);

private slots:
    void on_networkManager_Finished(QNetworkReply *reply);
    void on_login_button_Clicked();

private:
    Ui::MainWindow *ui;
    Qt::WindowFlags flags;
    QNetworkAccessManager *networkManager;
    QNetworkRequest networkRequest;
    QString hostIP, hostPort;
};
#endif // MAINWINDOW_H
