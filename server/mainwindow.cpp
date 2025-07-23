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
        actor, &ServerActor::messaged,
        this , &MainWindow::showMsg
    );
    delete ui;
}

void MainWindow::stateChanged()
{
    Actor::State st = actor->getState();

    ui->Temp->setValue(st.temp + ui->Temp->minimum());
    ui->Volt->setValue(st.voltage + ui->Volt->minimum());

    ui->temp_label->setText(
        QString("Temp %1 C")
            .arg(ui->Temp->value() * 0.5f, 7, 'f', 5));


    ui->voltage_label->setText(
        QString("Volt %1 V")
            .arg(ui->Volt->value() * 0.1f, 7, 'f', 5));

    Qt::CheckState checkedLookup[] = {
        Qt::Unchecked,
        Qt::Checked
    };

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
            ui->loaderRadio->setChecked(true);
            break;
        case Actor::State::Mode::Control:
            ui->controlRadio->setChecked(true);
            break;
        case Actor::State::Mode::Standard:
            ui->standartRadio->setChecked(true);
            break;
        case Actor::State::Mode::Calibration:
            ui->calibrRadio->setChecked(true);
            break;
    }
}

void MainWindow::setCom(const QModelIndex &index)
{
    if (actor != nullptr) {
        actor->setPort(index.data().toString());
    }
    else
    {
        auto serial = new Serial(index.data().toString());
        actor = new ServerActor(serial, ui->addrBox->value());


        connect(
            actor, &ServerActor::messaged,
            this , &MainWindow::showMsg
        );


        connect(
            actor, &ServerActor::messaged,
            this , &MainWindow::attenUpdate
        );

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 8; ++j) {

                auto cbCall = [this, i, j](Serial::Message msg){
                    Q_UNUSED(msg);
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
                    this->actor, &ServerActor::messaged,
                    this       , cbCall
                );
            }
        }
        stateChanged();

        QCheckBox* stateParts[] {
            ui->PSU_OK   ,
            ui->ROM_OK   ,
            ui->Res      ,
            ui->Volt_OK  ,
            ui->Atten1_OK,
            ui->Atten2_OK,
            ui->Module_OK,
            ui->Temp_OK
        };

        QSpinBox* stateVers[] {
            ui->MajorV,
            ui->MinorV
        };

        QSlider* stateVals[] {
            ui->Temp  ,
            ui->Volt
        };

        QRadioButton* stateMode[] {
            ui->loaderRadio  ,
            ui->standartRadio,
            ui->controlRadio ,
            ui->calibrRadio
        };

        for (auto e : stateParts) {
            connect(
                e, &QCheckBox::stateChanged,
                this, [this](int){
                    this->setModelState();
            });
        }


        for (auto e : stateVers) {
            connect(
                e,    &QSpinBox::textChanged,
                this, [this](const QString &){
                    this->setModelState();
            });
        }

        QLabel* stateValsLabel[] {
            ui->temp_label,
            ui->voltage_label
        };


        QString stateValsTempl[] {
            "Temp %1 C",
            "Volt %1 V"

        };
        int stateValsDiv[] {
            2,
            10

        };
        for (size_t i = 0; i < (sizeof(stateVals) / sizeof(*stateVals)); i ++) {
            auto e = stateVals[i];
            auto l = stateValsLabel[i];
            auto t = stateValsTempl[i];
            auto d = stateValsDiv[i];
            connect(
                e,    &QSlider::valueChanged,
                this, [this](int){
                    this->setModelState();
            });
            connect(
                e,    &QSlider::valueChanged,
                this, [l, t, d](int v){
                    l->setText(
                        QString (t).arg(v * 1.f / d, 7, 'f', 5)
                    );
            });
        }
        for (auto e : stateMode) {
            connect(
                e,    &QRadioButton::clicked,
                this, [this](bool){
                    this->setModelState();
            });
        }
    }
}

void MainWindow::attenUpdate(Serial::Message)
{

    auto settings = this->actor->getSettings();
    ui->Atten1->setValue(settings.attenuator[0]);
    ui->Atten2->setValue(settings.attenuator[1]);
}

void MainWindow::setModelState()
{
    Actor::State& st = actor->getState();
    st.ver.major = ui->MajorV->value();
    st.ver.minor = ui->MinorV->value();
    st.temp      = ui->Temp  ->value() - ui->Temp->minimum();
    st.voltage   = ui->Volt  ->value() - ui->Volt->minimum();

    qDebug() << "ui->Temp  ->value(): " << ui->Temp  ->value()
             << "ui->Temp->minimum(): " << ui->Temp->minimum()
             << "ui->Temp  ->value() - ui->Temp->minimum(): " << ui->Temp  ->value() - ui->Temp->minimum();

    Actor::State::Mode lookup[] = {
        Actor::State::Mode::Loader,
        Actor::State::Mode::Control,
        Actor::State::Mode::Standard,
        Actor::State::Mode::Calibration
    };
    int index = ui->loaderRadio  ->isChecked() ? 0 :
                ui->controlRadio ->isChecked() ? 1 :
                ui->standartRadio->isChecked() ? 2 :
                /*ui->calibrRadio  ->isChecked()*/   3;
    st.mode = lookup[index];

    st.parts.PSU      = ui->PSU_OK   ->checkState() == Qt::CheckState::Checked;
    st.parts.ROM      = ui->ROM_OK   ->checkState() == Qt::CheckState::Checked;
    st.parts.Res      = ui->Res      ->checkState() == Qt::CheckState::Checked;
    st.parts.VSens    = ui->Volt_OK  ->checkState() == Qt::CheckState::Checked;
    st.parts.Atten1   = ui->Atten1_OK->checkState() == Qt::CheckState::Checked;
    st.parts.Atten2   = ui->Atten2_OK->checkState() == Qt::CheckState::Checked;
    st.parts.Module   = ui->Module_OK->checkState() == Qt::CheckState::Checked;
    st.parts.TempSens = ui->Temp_OK  ->checkState() == Qt::CheckState::Checked;

}

void MainWindow::showMsg(Serial::Message msg)
{
    if (msg.getAddress() != actor->getAddress())
        return;
    ui->msgList->insertItem(0, QString(msg.dir == Serial::Message::Direction::RX ? "> " : "< ") + msg.toString());
}

