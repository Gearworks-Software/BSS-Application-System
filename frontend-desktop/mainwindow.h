#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private slots:
    void on_login_button_Clicked();

private:
    Ui::MainWindow *ui;
    Qt::WindowFlags flags;
};
#endif // MAINWINDOW_H
