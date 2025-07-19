#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

    void updateComs();
    uint8_t getAddr();
private slots:
    void on_listViewComs_activated(const QModelIndex &index);
    void on_serial_rxd(Serial::Message);
    void request_state(bool checked = false);

private:
    void on_anten1_value(int);
    void on_anten2_value(int);
    void set_settings_msg();
    Serial* serial;
    Ui::MainWindow *ui;
    QStringListModel* strComs;
    QStringListModel* strMsg;
    Actor::Settings settings;
};
#endif // MAINWINDOW_H
