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
    , actor(nullptr)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listViewMsg ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listViewComs->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QStringListModel* strComs = new QStringListModel(Serial::getComs());
    QStringListModel* strMsg  = new QStringListModel(QStringList ());

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
    connect(
        ui->pushButton, &QPushButton::clicked,
        this,           [this](int){this->actor->getState();;}
    );

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 8; ++j) {
            auto cb = new QCheckBox(QString("(%1 %2)").arg(i).arg(j));
            ui->gridLayout->addWidget(cb, j, i);
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

void MainWindow::on_listViewComs_activated(const QModelIndex &index)
{
    try {
        connectCom(index.data().toString());
    } catch (NullException e)
    {
        auto mod = static_cast<QStringListModel*>(ui->listViewComs->model());
        mod->removeRow(index.row(), index);
    }

}

void MainWindow::on_anten1_value(int v)
{
    Actor::Settings& set = this->actor->getSettings();
    set.attenuator[0] = v;
    this->actor->setSettings();
}

void MainWindow::on_anten2_value(int v)
{
    Actor::Settings& set = this->actor->getSettings();
    set.attenuator[1] = v;
    this->actor->setSettings();
}


void MainWindow::connectCom(QString name)
{
    if (actor == nullptr)
        actor = new ClientActor(name, ui->addrBox->value());
    actor ->setPort(name);
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 8; ++j) {
            QCheckBox* cb = static_cast<QCheckBox*>(
                ui->gridLayout->itemAtPosition(j, i)->widget()
            );
            auto cbCall = [this, i, j](int state){
                Actor::Settings& sett = this->actor->getSettings();
                if (state == Qt::CheckState::Checked)
                    sett.output[i] = sett.output[i] | 1 << j;
                else
                    sett.output[i] = sett.output[i] & ~(1 << j);

                this->actor->setSettings();
            };
            connect(
                cb, &QCheckBox::stateChanged,
                this, cbCall
            );
        }
    }
    actor->handshake();

    connect(
        actor, &ClientActor::state,
        this , &MainWindow::update
    );

    connect(
        actor, &ClientActor::msg,
        this,  &MainWindow::msg
    );
}

void MainWindow::msg(Serial::Message msg)
{
    auto m = static_cast<QStringListModel*>(ui->listViewMsg->model());
    if (m->insertRow(0))
        m->setData(m->index(0), QString(msg.dir == Serial::Message::Direction::RX ? "> " : "< ") + msg.toString());
}

void MainWindow::update(Actor::State st)
{
    Qt::CheckState checkedLookup[] = {
        Qt::Unchecked,
        Qt::Checked
    };

    ui->TempStat->setText(
        QString("Temp %1 C")
            .arg((float)(st.temp / 2.f) - 110.f, 7)
        );

    qDebug() << "st.temp: " << st.temp;
    qDebug() << "st.voltage: " << st.voltage;

    ui->VoltStat->setText(
        QString("Volt %1 V")
            .arg((float)(st.voltage / 10.f), 7)
    );


    ui->Module_OK->setCheckState(checkedLookup[st.parts.Module  ]);
    ui->PSU_OK   ->setCheckState(checkedLookup[st.parts.PSU     ]);
    ui->ROM_OK   ->setCheckState(checkedLookup[st.parts.ROM     ]);
    ui->Temp_OK  ->setCheckState(checkedLookup[st.parts.TempSens]);
    ui->Volt_OK  ->setCheckState(checkedLookup[st.parts.VSens   ]);
    ui->Atten1_OK->setCheckState(checkedLookup[st.parts.Atten1  ]);
    ui->Atten2_OK->setCheckState(checkedLookup[st.parts.Atten2  ]);
    ui->Res      ->setCheckState(checkedLookup[st.parts.Res     ]);
    ui->MajorV   ->setValue(st.ver.major);
    ui->MinorV   ->setValue(st.ver.minor);

    switch (st.mode) {
        case Actor::State::Mode::Loader:
            ui->mode_label->setText(QString("Loader"));
            break;
        case Actor::State::Mode::Control:
        ui->mode_label->setText(QString("Control"));
            break;
        case Actor::State::Mode::Standard:
        ui->mode_label->setText(QString("Standard"));
            break;
        case Actor::State::Mode::Calibration:
        ui->mode_label->setText(QString("Calibration"));
            break;

    }
}

