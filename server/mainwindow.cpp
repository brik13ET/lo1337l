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

void MainWindow::stateChanged(QByteArray st_)
{
    Actor::State st(st_);

    float tempf = 0.f, voltf = 0.f;

    tempf = (st.temp + 0.f) / UINT16_MAX;
    tempf = (ui->Temp->minimum()) + tempf * ((ui->Temp->maximum())-(ui->Temp->minimum()));

    voltf = (st.voltage + 0.f) / UINT16_MAX;
    voltf = (ui->Volt->minimum()) + voltf * ((ui->Volt->maximum())-(ui->Volt->minimum()));

    ui->Temp->setValue(tempf / 0.5f);
    ui->Volt->setValue(voltf / 0.1f);

    ui->temp_label->setText(
        QString("Temp %1 C")
            .arg(
                tempf,
                7
    ));


    ui->voltage_label->setText(
        QString("Volt %1 V")
            .arg(
                voltf,
                7
    ));

    Qt::CheckState checkedLookup[] = {
        Qt::Unchecked,
        Qt::Checked
    };

    ui->Module_OK->setCheckState(checkedLookup[st.parts.Module  ]);
    ui->PSU_OK   ->setCheckState(checkedLookup[st.parts.PSU     ]);
    ui->Temp_OK  ->setCheckState(checkedLookup[st.parts.TempSens]);
    ui->Volt_OK  ->setCheckState(checkedLookup[st.parts.VSens   ]);
    ui->Atten1_OK->setCheckState(checkedLookup[st.parts.Atten1  ]);
    ui->Atten2_OK->setCheckState(checkedLookup[st.parts.Atten2  ]);
    ui->Res      ->setCheckState(checkedLookup[st.parts.Res     ]);
    ui->MajorV   ->setValue(st.ver.major);
    ui->MinorV   ->setValue(st.ver.minor);
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


    connect(
        this->actor, &ServerActor::newState,
        this       , &MainWindow  ::stateChanged
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
                this       , cbCall
            );
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

