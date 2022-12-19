// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTIVEMASTERNODES_H
#define ACTIVEMASTERNODES_H

#include "activemasternode.h"

#include <vector>

// Responsible for activating the Masternode and pinging the network
class CActiveMasternodeMan
{
private:
    std::vector<CActiveMasternode> vActiveMasternodes;
public:
    /// Manage status of all Masternodes
    void ManageStatus();
    void ResetStatus();
    std::vector<CActiveMasternode>& GetActiveMasternodes() { return vActiveMasternodes; }
    void Add(CActiveMasternode activeMasternode);
};

#endif //ACTIVEMASTERNODES_H
