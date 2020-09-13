// Copyright (c) 2018 The Flits-Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_GOVERNANCEPAGE_H
#define BITCOIN_QT_GOVERNANCEPAGE_H

#include "qt/fls/pwidget.h"
#include "masternode.h"
#include "sync.h"
#include "util.h"

#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QTimer>
#include <QWidget>

class FLSGUI;
class CBudgetProposal;

namespace Ui
{
    class GovernancePage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class GovernancePage : public PWidget
{
    Q_OBJECT

public:
    explicit GovernancePage(FLSGUI* parent = nullptr);
    ~GovernancePage();

    void setExtendedProposal(CBudgetProposal* proposal);
    void lockUpdating(bool lock);

private:
    QMenu* contextMenu;
    int64_t nTimeFilterUpdated;
    bool fFilterUpdated;
    bool fLockUpdating;

public Q_SLOTS:
    void updateProposalList();

private Q_SLOTS:
    void onCreateProposalButtonclicked();
    void onUpdateButtonclicked();

private:
    QTimer* timer;
    Ui::GovernancePage *ui;
    QString strCurrentFilter;
    CBudgetProposal* extendedProposal;

};

#endif // BITCOIN_QT_GOVERNANCEPAGE_H
