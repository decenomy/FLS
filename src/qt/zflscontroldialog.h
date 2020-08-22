// Copyright (c) 2017-2020 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZFLSCONTROLDIALOG_H
#define ZFLSCONTROLDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "zfls/zerocoin.h"

class CZerocoinMint;
class WalletModel;

namespace Ui {
class zFLSControlDialog;
}

class CzFLSControlWidgetItem : public QTreeWidgetItem
{
public:
    explicit CzFLSControlWidgetItem(QTreeWidget *parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    explicit CzFLSControlWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    explicit CzFLSControlWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent, type) {}

    bool operator<(const QTreeWidgetItem &other) const;
};

class zFLSControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit zFLSControlDialog(QWidget *parent);
    ~zFLSControlDialog();

    void setModel(WalletModel* model);

    static std::set<std::string> setSelectedMints;
    static std::set<CMintMeta> setMints;
    static std::vector<CMintMeta> GetSelectedMints();

private:
    Ui::zFLSControlDialog *ui;
    WalletModel* model;

    void updateList();
    void updateLabels();

    enum {
        COLUMN_CHECKBOX,
        COLUMN_DENOMINATION,
        COLUMN_PUBCOIN,
        COLUMN_VERSION,
        COLUMN_CONFIRMATIONS,
        COLUMN_ISSPENDABLE
    };
    friend class CzFLSControlWidgetItem;

private Q_SLOTS:
    void updateSelection(QTreeWidgetItem* item, int column);
    void ButtonAllClicked();
};

#endif // ZFLSCONTROLDIALOG_H
