#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStringListModel>
#include <QDebug>

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

    connect(
        ui->addrBox, &QSpinBox::textChanged,
        this,        &MainWindow::addressChanged
    );


    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 8; ++j) {
            auto cb = new QCheckBox(
                QString("(%1 %2)").arg(i).arg(j)
            );
            cb->setEnabled(false);
            ui->gridLayout->addWidget(cb, j, i);
        }

    }
}

void MainWindow::addressChanged(const QString &)
{
    if (this->actor == nullptr)
        return;
    this->actor->setAddress(ui->addrBox->value());
}

MainWindow::~MainWindow()
{
    disconnect(
        ui->comList, &QListWidget::activated,
        this,        &MainWindow::setCom
    );

    disconnect(
        actor, &ServerActor::recived,
        this , &MainWindow::showMsg
    );
    delete ui;
}

void MainWindow::setCom(const QModelIndex &index)
{
    if (actor != nullptr) {
        actor->quit();
        actor->setPort(index.data().toString());
    }
    else
    {
        auto serial = new Serial(index.data().toString());
        actor = new ServerActor(serial, ui->addrBox->value());
    }

    actor->start(QThread::Priority::NormalPriority);

    connect(
        actor, &ServerActor::recived,
        this , &MainWindow::showMsg
    );


    connect(
        actor, &ServerActor::recived,
        this , &MainWindow::attenUpdate
    );

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 8; ++j) {

            auto cbCall = [this, i, j](Serial::Message msg){
                auto settings = this->actor->getSettings();
                bool checked = (settings.output[i] & (1 << j)) != 0;
                Qt::CheckState checkedLookup[] = {
                    Qt::Unchecked,
                    Qt::Checked
                };


                auto cbw = this->ui->gridLayout->itemAtPosition(j, i)->widget();
                if (cbw == nullptr)
                {
                    qDebug() << "cbw == nullptr";
                    return;
                }
                auto cb = static_cast<QCheckBox*>(cbw);

                if (cb == nullptr)
                {
                    qDebug() << "cb == nullptr";
                    return;
                }

                cb->setCheckState(checkedLookup[checked]);
                this->ui->gridLayout->invalidate();
            };

            connect(
                this->actor, &ServerActor::recived,
                this       , cbCall);

        }
    }
}

void MainWindow::attenUpdate(Serial::Message)
{

    auto settings = this->actor->getSettings();
    ui->Atten1->setValue(settings.attenuator[0]);
    ui->Atten2->setValue(settings.attenuator[1]);
}

void MainWindow::showMsg(Serial::Message msg)
{
    if (msg.getAddress() != actor->getAddress())
        return;
    ui->msgList->insertItem(0, msg.toString());
}

