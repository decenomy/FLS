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
    ui->spinBoxAutoCombineThreshold->setEnabled(false);
    connect(ui->checkBoxAutoCombine, &QCheckBox::stateChanged, this, &SettingsWalletOptionsWidget::onAutoCombineCheckboxStateChanged);
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

void SettingsWalletOptionsWidget::onSpinBoxStakeSplitThresholdChanged(){
    if (ui->spinBoxStakeSplitThreshold->value() > 0) {
        // Autocombine threshold must be < 2*stake split threshold, and the smallest amount by which it can be smaller is 0.00000001.
        ui->spinBoxAutoCombineThreshold->setMaximum(ui->spinBoxStakeSplitThreshold->value() * 2 - 0.00000001);
    } else {
        ui->spinBoxAutoCombineThreshold->setMaximum(0);
    }
}

void SettingsWalletOptionsWidget::onAutoCombineCheckboxStateChanged(){
    ui->spinBoxAutoCombineThreshold->setEnabled(ui->checkBoxAutoCombine->isChecked());
}

SettingsWalletOptionsWidget::~SettingsWalletOptionsWidget(){
    delete ui;
}
