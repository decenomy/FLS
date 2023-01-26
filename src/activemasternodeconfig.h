// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2019 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SRC_ACTIVEMASTERNODECONFIG_H_
#define SRC_ACTIVEMASTERNODECONFIG_H_

#include "uint256.h"
#include <string>
#include <vector>

class CActiveMasternodeConfig;
extern CActiveMasternodeConfig activeMasternodeConfig;

class CActiveMasternodeConfig
{
public:
    class CActiveMasternodeEntry
    {
    public:
        std::string strAlias{""};
        std::string strMasterNodePrivKey{""};

        CActiveMasternodeEntry(std::string strAlias, std::string strMasterNodePrivKey)
        {
            this->strAlias = strAlias;
            this->strMasterNodePrivKey = strMasterNodePrivKey;
        }
    };

    bool Load(std::string& strErr);
    uint256 GetFileHash();

    std::vector<CActiveMasternodeEntry>& Entries()
    {
        return vEntries;
    }
private:
    std::vector<CActiveMasternodeEntry> vEntries;
    void add(std::string strAlias, std::string strMasterNodePrivKey);
};

#endif /* SRC_ACTIVEMASTERNODECONFIG_H_ */
