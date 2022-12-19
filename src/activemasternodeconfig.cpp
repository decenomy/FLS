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

CActiveMasternodeConfig::CActiveMasternodeEntry& CActiveMasternodeConfig::add(std::string strAlias, std::string strMasterNodePrivKey)
{
    CActiveMasternodeEntry cme(strAlias, strMasterNodePrivKey);
    vEntries.push_back(cme);
    return vEntries.back();
}

void CActiveMasternodeConfig::remove(std::string strAlias)
{
    int pos = -1;
    for (int i = 0; i < ((int)vEntries.size()); ++i) {
        CActiveMasternodeEntry e = vEntries[i];
        if (e.strAlias == strAlias) {
            pos = i;
            break;
        }
    }
    vEntries.erase(vEntries.begin() + pos);
}

bool CActiveMasternodeConfig::read(std::string& strErr)
{
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
                return false;
            }
        }

        if (strAlias.empty()) {
            strErr = _("alias cannot be empty in activemasternode.conf");
            streamConfig.close();
            return false;
        }

        add(strAlias, strMasterNodePrivKey);
    }

    streamConfig.close();
    return true;
}
