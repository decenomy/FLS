// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTIVEMASTERNODES_H
#define ACTIVEMASTERNODES_H

#include "activemasternode.h"
#include "activemasternodeconfig.h"

#include <vector>

// Responsible for activating the Masternode and pinging the network
class CActiveMasternodeMan
{
private:
    std::vector<CActiveMasternode> vActiveMasternodes;
    uint256 fileHash = UINT256_ZERO;
    // critical section to protect the inner data structures
    mutable RecursiveMutex cs;
public:
    /// Manage status of all Masternodes
    void ManageStatus();
    void ResetStatus();
    std::vector<CActiveMasternode>& GetActiveMasternodes() { return vActiveMasternodes; }
    bool Add(CActiveMasternodeConfig::CActiveMasternodeEntry ame, std::string& strErr);
    bool Add(std::string strAlias, std::string strMasterNodePrivKey, std::string& strErr) {
        return Add(CActiveMasternodeConfig::CActiveMasternodeEntry(strAlias, strMasterNodePrivKey), strErr);
    }
    bool Load(std::string& strErr);
    bool Reload(std::string& strErr);
    std::size_t Count() { return vActiveMasternodes.size(); }
};

#endif //ACTIVEMASTERNODES_H
