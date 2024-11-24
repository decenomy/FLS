// Copyright (c) 2024 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bootstrap.h"

#ifdef HAVE_CONFIG_H
#include "config/pivx-config.h"
#endif

#include "amount.h"
#include "chainparams.h"
#include "curl.h"
#include "guiinterface.h"
#include "util.h"
#include "zip.h"

#include <iostream>

namespace fs = boost::filesystem;

std::string extractFilenameFromURL(const std::string& url)
{
    // Find the last '/' in the URL, which is expected to precede the filename
    size_t last_slash_pos = url.find_last_of("/");

    // Check if the last slash position is valid and there are characters after the '/'
    if (last_slash_pos != std::string::npos && last_slash_pos + 1 < url.size()) {
        return url.substr(last_slash_pos + 1);
    }

    // If no '/' found, or there is nothing after the last '/', return an empty string
    return "";
}

int progressCallback(
    void* clientp, 
    curl_off_t dltotal,
    curl_off_t dlnow,
    curl_off_t ultotal,
    curl_off_t ulnow)
{
    static int progress = 0;
    const auto currentProgress = dltotal > 0 ? (int)((dlnow * 100) / dltotal) : 0;

    if (currentProgress > progress) {
        progress = currentProgress;
        LogPrintf("CBootstrap::%s: Download: %d%%\n", __func__, progress);
        uiInterface.ShowProgress(_("Download: "), progress);
    }

    return 0;
}

bool CBootstrap::DownloadAndApply()
{
    const auto url =
        std::string(BOOTSTRAP_URL) +
        (Params().IsTestNet() ? "T" : "") + std::string(CURRENCY_UNIT) +
        "/bootstrap.zip";
    const auto datadir = GetDataDir();
    const auto fileName = datadir / extractFilenameFromURL(url);
    const auto blocks = datadir / "blocks";
    const auto chainstate = datadir / "chainstate";
    const auto sporks = datadir / "sporks";
    const auto banlist = datadir / "banlist.dat";

    try {
        // Step 1: Download the bootstrap file
        if (!CCurlWrapper::DownloadFile(url, fileName.string(), progressCallback)) {
            LogPrintf(
                "CBootstrap::%s: Failed to download the bootstrap file: %s\n",
                __func__,
                fileName.string());
            return false;
        }

        // Step 2: Cleanup the existing folders and files
        try {
            fs::remove_all(blocks);
            fs::remove_all(chainstate);
            fs::remove_all(sporks);
            fs::remove_all(banlist);
        } catch (const boost::filesystem::filesystem_error& e) {
            LogPrintf("CBootstrap::%s: Error removing: %s\n", __func__, e.what());
            return false;
        }

        // Step 3: Unzip the downloaded file
        if (!CZipWrapper::ExtractZip(fileName.string(), datadir.string())) {
            LogPrintf(
                "CBootstrap::%s: Failed to unzip the bootstrap file: %s into: %s\n",
                __func__, fileName.string(), datadir.string());
            fs::remove_all(fileName);
            fs::remove_all(blocks);
            fs::remove_all(chainstate);
            fs::remove_all(sporks);
            fs::remove_all(banlist);

            return false;
        }

        // Step 4: Cleanup the bootstrap file
        fs::remove_all(fileName);

    } catch (const std::exception& e) {
        LogPrintf(
            "CBootstrap::%s: Error downloading bootstrap file: %s\n", 
            __func__, e.what());

        fs::remove_all(fileName);
        fs::remove_all(blocks);
        fs::remove_all(chainstate);
        fs::remove_all(sporks);
        fs::remove_all(banlist);
        
        return false;
    }

    return true;
}
