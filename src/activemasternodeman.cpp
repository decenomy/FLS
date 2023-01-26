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

    LOCK(cs);

    std::string strErr;
    if(!Reload(strErr)) {
        LogPrintf("CActiveMasternodeMan::ManageStatus() - %s\n", strErr);
    }

    for(auto& amn : vActiveMasternodes) {
        amn.ManageStatus();
    }
}

void CActiveMasternodeMan::ResetStatus() {
    
    LOCK(cs);

    std::string strErr;
    if(!Reload(strErr)) {
        LogPrintf("CActiveMasternodeMan::ManageStatus() - %s\n", strErr);
    }
    
    for(auto& amn : vActiveMasternodes) {
        amn.ResetStatus();
    }
}

bool CActiveMasternodeMan::Add(CActiveMasternodeConfig::CActiveMasternodeEntry ame, std::string& strErr) {
    
    LOCK(cs);

    auto strAlias = ame.strAlias;
    auto strMasterNodePrivKey = ame.strMasterNodePrivKey;

    if (strAlias.empty()) {
        strErr = _("alias cannot be empty in activemasternode.conf");
        return false;
    }

    CActiveMasternode amn;

    amn.strAlias = strAlias;
    amn.strMasterNodePrivKey = strMasterNodePrivKey;

    CKey key;
    CPubKey pubkey;

    if (!CMessageSigner::GetKeysFromSecret(amn.strMasterNodePrivKey, key, pubkey)) {
        strErr = _("Invalid masternodeprivkey. Please see documenation.");
        return false;
    }

    amn.pubKeyMasternode = pubkey;
    
    vActiveMasternodes.push_back(amn);

    return true;
}

bool CActiveMasternodeMan::Load(std::string& strErr) {
    
    LOCK(cs);
    
    if (!activeMasternodeConfig.Load(strErr)) return false;

    fileHash = activeMasternodeConfig.GetFileHash();

    for(auto& ame : activeMasternodeConfig.Entries()) {
        if(!Add(ame, strErr)) return false;
    }

    return true;
}

bool CActiveMasternodeMan::Reload(std::string& strErr) {

    LOCK(cs);

    auto hash = activeMasternodeConfig.GetFileHash();

    if(fileHash != hash) {
        if (!activeMasternodeConfig.Load(strErr)) return false;

        auto backup = vActiveMasternodes;

        vActiveMasternodes.clear();
        for(auto& ame : activeMasternodeConfig.Entries()) {
            if(!Add(ame, strErr)) {
                vActiveMasternodes.clear();
                vActiveMasternodes = backup;
                return false;
            }
        }

        fileHash = hash;

        ResetStatus();

        return true;
    }

    return true;
}