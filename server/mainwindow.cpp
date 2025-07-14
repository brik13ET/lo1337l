#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStringListModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , actor(nullptr)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comList->addItems(Serial::getComs());
    connect(
        ui->comList, &QListWidget::activated,
        this,        &MainWindow::setCom
    );
}

MainWindow::~MainWindow()
{
    disconnect(
        ui->comList, &QListWidget::activated,
        this,        &MainWindow::setCom
    );
    delete ui;
}

void MainWindow::setCom(const QModelIndex &index)
{
    if (actor != nullptr)
        actor->quit();
    auto port = new QSerialPort(index.data().toString());
    auto serial = new Serial(port);
    actor = new ServerActor(serial, 0);
    actor->start();
}

