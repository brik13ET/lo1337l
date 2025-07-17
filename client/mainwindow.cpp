#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QDebug>
#include <QCheckBox>
#include <rangeexception.h>
#include <nullexception.h>


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
        ui->listViewComs, &QListView::activated,
        this, &MainWindow::on_listViewComs_activated
    );

    connect(
        ui->atten1, &QSlider::valueChanged,
        this      , &MainWindow::on_anten1_value
    );
    connect(
        ui->atten2, &QSlider::valueChanged,
        this      , &MainWindow::on_anten2_value
    );
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 8; ++j) {
            auto cb = new QCheckBox(QString("(%1 %2)").arg(i).arg(j));
            auto cbCall = [this, i, j](int state){
                if (state == Qt::CheckState::Checked)
                    this->settings.output[i] |= (state == Qt::CheckState::Checked) << j;
                else
                    this->settings.output[i] = this->settings.output[i] & ~((state != Qt::CheckState::Checked) << j);
                this->send_state();
            };
            ui->gridLayout->addWidget(cb, j, i);
            connect(cb, &QCheckBox::stateChanged, this, cbCall);
        }
    }
}

MainWindow::~MainWindow()
{
    disconnect(
        ui->listViewComs, &QListView::activated,
        this, &MainWindow::on_listViewComs_activated
    );

    disconnect(
        ui->atten1, &QSlider::valueChanged,
        this      , &MainWindow::on_anten1_value
    );
    disconnect(
        ui->atten2, &QSlider::valueChanged,
        this      , &MainWindow::on_anten2_value
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
    if (this->serial != nullptr){
        disconnect(
             serial, &Serial::rxd,
             this,   &MainWindow::on_serial_rxd
         );


        disconnect(
             serial, &Serial::txd,
             this,   &MainWindow::on_serial_rxd
         );
        static_cast<QSerialPort>(this->serial).close();
        delete this->serial;
    }

    auto port = new QSerialPort(index.data().toString());
    this->serial = new Serial(port);

    qDebug() << "this->serial: " << this->serial;
    qDebug() << "QSerialPort->isOpen(): " << port->isOpen();

    serial->transmit(
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


    connect(
         serial, &Serial::txd,
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

void MainWindow::on_anten1_value(int value)
{
    this->settings.attenuator[0] = value;
    send_state();
}

void MainWindow::on_anten2_value(int value)
{
    this->settings.attenuator[1] = value;
    send_state();
}

void MainWindow::send_state()
{
    try {
        Serial::Message msg (
            this->getAddr(),
            Serial::Message::setSettings().cmd,
            Serial::Message::setSettings().op,
            this->settings
        );
        if (serial == nullptr)
            throw new NullException("`Serial` is null, probably port isnt open");
        serial->transmit(msg);
    } catch (RangeException e) {
        qDebug() << e.what();
        return ;
    } catch (NullException e) {
        qDebug() << e.what();
        return ;
    }

}

