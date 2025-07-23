#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "clientactor.h"

#include <QMainWindow>
#include <QStringListModel>
#include <serial.h>
#include <actor.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void on_listViewComs_activated(const QModelIndex &index);

private:
    ClientActor* actor;
    void on_anten1_value(int);
    void on_anten2_value(int);
    Ui::MainWindow *ui;
    void connectCom(QString);
    void msg(Serial::Message);
    void update(Actor::State);

};
#endif // MAINWINDOW_H
