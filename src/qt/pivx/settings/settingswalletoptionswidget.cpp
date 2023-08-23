// Copyright (c) 2019 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/pivx/settings/settingswalletoptionswidget.h"
#include "qt/pivx/settings/forms/ui_settingswalletoptionswidget.h"
#include <QListView>
#include "optionsmodel.h"
#include "clientmodel.h"
#include "qt/pivx/qtutils.h"

SettingsWalletOptionsWidget::SettingsWalletOptionsWidget(PIVXGUI* _window, QWidget *parent) :
    PWidget(_window, parent),
    ui(new Ui::SettingsWalletOptionsWidget)
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    // Containers
    ui->left->setProperty("cssClass", "container");
    ui->left->setContentsMargins(10,10,10,10);
    ui->labelDivider->setProperty("cssClass", "container-divider");

    // Title
    setCssTitleScreen(ui->labelTitle);
    setCssSubtitleScreen(ui->labelSubtitle1);

    // Combobox
    ui->labelTitleStake->setProperty("cssClass", "text-main-settings");
    ui->spinBoxStakeSplitThreshold->setProperty("cssClass", "btn-spin-box");
    ui->spinBoxStakeSplitThreshold->setAttribute(Qt::WA_MacShowFocusRect, 0);
    connect(ui->spinBoxStakeSplitThreshold, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &SettingsWalletOptionsWidget::onSpinBoxStakeSplitThresholdChanged);
    setShadow(ui->spinBoxStakeSplitThreshold);

    // Checkbox+spinbox
    ui->spinBoxAutoCombineThreshold->setProperty("cssClass", "btn-spin-box");
    ui->spinBoxAutoCombineThreshold->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->spinBoxAutoCombineThreshold->setVisible(false);
    connect(ui->checkBoxAutoCombine, &QCheckBox::stateChanged, this, &SettingsWalletOptionsWidget::onAutoCombineCheckboxStateChanged);
    connect(ui->spinBoxAutoCombineThreshold, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &SettingsWalletOptionsWidget::onSpinBoxAutoCombineThresholdChanged);
    setShadow(ui->spinBoxAutoCombineThreshold);

    // Radio buttons

    // Title
    ui->labelTitleNetwork->setText(tr("Network"));
    setCssTitleScreen(ui->labelTitleNetwork);
    setCssSubtitleScreen(ui->labelSubtitleNetwork);

    // Proxy
    ui->labelSubtitleProxy->setProperty("cssClass", "text-main-settings");
    initCssEditLine(ui->lineEditProxy);

    // Port
    ui->labelSubtitlePort->setProperty("cssClass", "text-main-settings");
    initCssEditLine(ui->lineEditPort);

    // Buttons
    setCssBtnPrimary(ui->pushButtonSave);
    setCssBtnSecondary(ui->pushButtonReset);
    setCssBtnSecondary(ui->pushButtonClean);

    connect(ui->pushButtonSave, &QPushButton::clicked, [this] { Q_EMIT saveSettings(); });
    connect(ui->pushButtonReset, &QPushButton::clicked, this, &SettingsWalletOptionsWidget::onResetClicked);
    connect(ui->pushButtonClean, &QPushButton::clicked, [this] { Q_EMIT discardSettings(); });
}

void SettingsWalletOptionsWidget::onResetClicked(){
    if (clientModel) {
        OptionsModel *optionsModel = clientModel->getOptionsModel();
        QSettings settings;
        optionsModel->setWalletDefaultOptions(settings, true);
        optionsModel->setNetworkDefaultOptions(settings, true);
        inform(tr("Options reset succeed"));
    }
}

void SettingsWalletOptionsWidget::setMapper(QDataWidgetMapper *mapper){
    mapper->addMapping(ui->radioButtonSpend, OptionsModel::SpendZeroConfChange);
    mapper->addMapping(ui->spinBoxStakeSplitThreshold, OptionsModel::StakeSplitThreshold);
    mapper->addMapping(ui->checkBoxAutoCombine, OptionsModel::CombineDust);
    mapper->addMapping(ui->spinBoxAutoCombineThreshold, OptionsModel::AutoCombineThreshold);

    // Network
    mapper->addMapping(ui->checkBoxMap, OptionsModel::MapPortUPnP);
    mapper->addMapping(ui->checkBoxAllow, OptionsModel::Listen);
    mapper->addMapping(ui->checkBoxConnect, OptionsModel::ProxyUse);
    mapper->addMapping(ui->lineEditProxy, OptionsModel::ProxyIP);
    mapper->addMapping(ui->lineEditPort, OptionsModel::ProxyPort);
}

void SettingsWalletOptionsWidget::setSpinBoxStakeSplitThreshold(double val)
{
    ui->spinBoxStakeSplitThreshold->setValue(val);
}

void SettingsWalletOptionsWidget::onSpinBoxStakeSplitThresholdChanged()
{
    if (ui->spinBoxStakeSplitThreshold->value() > 1) {
        if (ui->spinBoxAutoCombineThreshold->value() >= ui->spinBoxStakeSplitThreshold->value() * 2 - 1) {
            ui->spinBoxAutoCombineThreshold->setValue(floor(ui->spinBoxStakeSplitThreshold->value() * 2 - 1));
        }
    } else {
        ui->spinBoxAutoCombineThreshold->setValue(1);
    }
}

void SettingsWalletOptionsWidget::onSpinBoxAutoCombineThresholdChanged()
{
    if (ui->spinBoxStakeSplitThreshold->value() > 1) {
        if (ui->spinBoxAutoCombineThreshold->value() >= ui->spinBoxStakeSplitThreshold->value() * 2 - 1) {
            ui->spinBoxAutoCombineThreshold->setValue(floor(ui->spinBoxStakeSplitThreshold->value() * 2 - 1));
        }
    } else {
        ui->spinBoxAutoCombineThreshold->setValue(1);
    }

    // Enforce only integers.
    ui->spinBoxAutoCombineThreshold->setValue(floor(ui->spinBoxAutoCombineThreshold->value()));
}

void SettingsWalletOptionsWidget::onAutoCombineCheckboxStateChanged()
{
    ui->spinBoxAutoCombineThreshold->setVisible(ui->checkBoxAutoCombine->isChecked());
}

SettingsWalletOptionsWidget::~SettingsWalletOptionsWidget()
{
    delete ui;
}
