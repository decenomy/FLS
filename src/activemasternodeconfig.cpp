// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2019 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternodeconfig.h"
#include "fs.h"
#include "guiinterface.h"
#include "netbase.h"
#include "util.h"
#include <algorithm>
#include <base58.h>

CActiveMasternodeConfig activeMasternodeConfig;

void CActiveMasternodeConfig::add(std::string strAlias, std::string strMasterNodePrivKey)
{
    CActiveMasternodeEntry cme(strAlias, strMasterNodePrivKey);
    vEntries.push_back(cme);
}

bool CActiveMasternodeConfig::Load(std::string& strErr)
{
    auto backup = vEntries;

    vEntries.clear();

    int linenumber = 1;
    fs::path pathActiveMasternodeConfigFile = GetActiveMasternodeConfigFile();
    fs::ifstream streamConfig(pathActiveMasternodeConfigFile);

    if (!streamConfig.good()) {
        FILE* configFile = fsbridge::fopen(pathActiveMasternodeConfigFile, "a");
        if (configFile != NULL) {
            std::string strHeader = "# Activemasternode config file\n"
                                    "# Format: alias activemasternodeprivkey\n"
                                    "#\n"
                                    "# Example: mn1 93HaYBVUCYjEMeeH1Y4sBGLALQZE1Yc1K64xiqgX37tGBDQL8Xg\n"
                                    "#\n";
            fwrite(strHeader.c_str(), std::strlen(strHeader.c_str()), 1, configFile);
            fclose(configFile);
        }
        return true; // Nothing to read, so just return
    }

    for (std::string line; std::getline(streamConfig, line); linenumber++) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string comment, strAlias, strMasterNodePrivKey;

        if (iss >> comment) {
            if (comment.at(0) == '#') continue;
            iss.str(line);
            iss.clear();
        }

        if (!(iss >> strAlias >> strMasterNodePrivKey)) {
            iss.str(line);
            iss.clear();
            if (!(iss >> strAlias >> strMasterNodePrivKey)) {
                strErr = _("Could not parse activemasternode.conf") + "\n" +
                         strprintf(_("Line: %d"), linenumber) + "\n\"" + line + "\"";
                streamConfig.close();
                vEntries = backup;
                return false;
            }
        }

        if (strAlias.empty()) {
            strErr = _("alias cannot be empty in activemasternode.conf");
            streamConfig.close();
            vEntries = backup;
            return false;
        }

        add(strAlias, strMasterNodePrivKey);
    }

    streamConfig.close();
    return true;
}

uint256 CActiveMasternodeConfig::GetFileHash() {
    fs::path pathActiveMasternodeConfigFile = GetActiveMasternodeConfigFile();
    fs::ifstream streamConfig(pathActiveMasternodeConfigFile);

    if (!streamConfig.good()) {
        return UINT256_ZERO;
    }

    //get length of file
    streamConfig.seekg(0, streamConfig.end);
    size_t length = streamConfig.tellg();
    streamConfig.seekg(0, streamConfig.beg);

    //read file
    if (length > 0) {
        std::vector<char> buffer;
        buffer.resize(length);    
        streamConfig.read(&buffer[0], length);

        return Hash(&buffer[0], &buffer[0] + length);
    }

    return UINT256_ZERO;
}
