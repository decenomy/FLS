// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "activemasternodeman.h"
#include "activemasternodeconfig.h"

#include <vector>

void CActiveMasternodeMan::ManageStatus() {
    for(auto& amn : vActiveMasternodes) {
        amn.ManageStatus();
    }
}

void CActiveMasternodeMan::ResetStatus() {
    for(auto& amn : vActiveMasternodes) {
        amn.ResetStatus();
    }
}

void CActiveMasternodeMan::Add(CActiveMasternode activeMasternode) {
    vActiveMasternodes.push_back(activeMasternode);
}
