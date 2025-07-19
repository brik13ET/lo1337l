#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serveractor.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    void stateChanged(QByteArray);
    void addressChanged(const QString &);
    void setCom(const QModelIndex &index);
    void attenUpdate(Serial::Message);
private:
    ServerActor* actor;
    Ui::MainWindow *ui;

    void showMsg(Serial::Message);
};
#endif // MAINWINDOW_H
