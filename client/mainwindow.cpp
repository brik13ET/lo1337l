#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QDebug>
#include <QCheckBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , serial(nullptr)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listViewMsg ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listViewComs->setEditTriggers(QAbstractItemView::NoEditTriggers);

    strComs = new QStringListModel(*(new QStringList ()));
    strMsg  = new QStringListModel(*(new QStringList ()));

    updateComs();

    ui->listViewComs->setModel(strComs);
    ui->listViewMsg ->setModel(strMsg );

    connect(
        ui->listViewComs, SIGNAL(activated()),
        this, SLOT(on_listViewComs_activated())
    );

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 8; ++j) {
            auto cb = new QCheckBox(QString("(%1 %2)").arg(i).arg(j));
            auto cbCall = [this](){
                Serial::Message msg (
                    this->getAddr(),
                    Serial::Message::setSettings().cmd,
                    Serial::Message::setSettings().op,
                    this->settings
                );
                this->serial->txd(msg);
            };
            ui->gridLayout->addWidget(cb);
            connect(cb, &QCheckBox::stateChanged, cbCall);
        }
    }
}

MainWindow::~MainWindow()
{
    disconnect(
        ui->listViewComs, SIGNAL(activated()),
        this, SLOT(on_listViewComs_activated())
    );
    delete ui;
}

void MainWindow::updateComs()
{
    auto l = Serial::getComs();
    strComs->setStringList(l);
}

uint8_t MainWindow::getAddr()
{
    return ui->addrBox->value();
}


void MainWindow::on_listViewComs_activated(const QModelIndex &index)
{
    qDebug() << __FUNCTION__;
    qDebug() << "this: " << this;
    if (this->serial != nullptr)
        static_cast<QSerialPort>(this->serial).close();

    auto port = new QSerialPort(index.data().toString());
    this->serial = new Serial(port);

    qDebug() << "this->serial: " << this->serial;
    qDebug() << "QSerialPort: " << static_cast<QSerialPort>(this->serial).isOpen();

    serial->txd(
                Serial::Message (
                    getAddr(),
                    Serial::Message::handshake().cmd,
                    Serial::Message::handshake().op,
                    QByteArray {}
                )); // Send Handshake

    connect(
         serial, &Serial::rxd,
         this,   &MainWindow::on_serial_rxd
     );
}

void MainWindow::on_serial_rxd(Serial::Message msg)
{
    auto msgStr = QString ("Addr: %1, CN: %2, MT: %3\n%4")
            .arg(msg.getAddress())
            .arg(msg.getCmdNo())
            .arg(msg.getMeta())
            .arg(QString(msg.getData().toHex()));
    if (strMsg->insertRow(0)) {
        QModelIndex index = strMsg->index(0, 0);
        strMsg->setData(index, msgStr);
    }
}

