#define COIN "777"

#include "bootstrap.h"
namespace fs = boost::filesystem;

// Define the progress callback function
static int downloadProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {

    // Calculate progress percentage
    double progress = (dlnow > 0) ? (dlnow / dltotal) * 100.0 : 0.0;

    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    static bool log_flag = false; // Declare log_flag as static
    if (!log_flag && duration.count() % 2 == 0) {
        log_flag = true;
        std::printf("-bootstrap: Download: %d%%\n", (uint8_t)progress);
        // LogPrintf("-Bootstrap: Download: %d%%\n", (uint8_t)progress);
        // uiInterface.ShowProgress(_("Download: "), (uint8_t)progress);    
    } else if (duration.count() % 2 != 0) {
        log_flag = false;
    }

    return 0;
}

int main() {

    /* File src/init.cpp
    line 1206
    if (GetBoolArg("-bootstrap", false)) {
        uiInterface.InitMessage(_("Preparing for bootstrap..."));
        // Delete the local blockchain folders to force a bootstrap from scratch to get a consitent blockchain-state
        fs::path blocksDir = GetDataDir() / "blocks";
        fs::path chainstateDir = GetDataDir() / "chainstate";
        fs::path sporksDir = GetDataDir() / "sporks";

        LogPrintf("Deleting blockchain folders blocks, chainstate, and sporks\n");
        // We delete in 4 individual steps in case one of the folder is missing already
        try {
            if (fs::exists(blocksDir)){
                fs::remove_all(blocksDir);
                LogPrintf("-bootstrap: folder deleted: %s\n", blocksDir.string().c_str());
            }

            if (fs::exists(chainstateDir)){
                fs::remove_all(chainstateDir);
                LogPrintf("-bootstrap: folder deleted: %s\n", chainstateDir.string().c_str());
            }

            if (fs::exists(sporksDir)){
                fs::remove_all(sporksDir);
                LogPrintf("-bootstrap: folder deleted: %s\n", sporksDir.string().c_str());
            }
        } catch (const fs::filesystem_error& error) {
            LogPrintf("Failed to delete blockchain folders %s\n", error.what());
        }
    }
    */

    const std::string url = "https://bootstraps.decenomy.net/"+std::string(COIN)+"/bootstrap.zip";
    //const std::string url = std::string(BOOTSTRAP_URL)+std::string(TICKER)+"/bootstrap.zip";
    const std::string outputFileName = "bootstrap.zip";
    const std::string extractPath = "bootstrap_";

    std::cout << "Download: " << url << std::endl;

    if(Bootstrap::isDirectory(extractPath))
        Bootstrap::rmDirectory(extractPath);

    if (Bootstrap::DownloadFile(url, outputFileName, downloadProgressCallback)) {
        std::cout << "File downloaded successfully." << std::endl;

        if (Bootstrap::extractZip(outputFileName, extractPath)) {
            std::cout << "Zip file extracted successfully." << std::endl;
        } else {
            std::cerr << "Error extracting zip file." << std::endl;
        }

        if(Bootstrap::isDirectory(extractPath))
            Bootstrap::rmDirectory(extractPath);
    
    } else {
        std::cerr << "Error downloading file." << std::endl;
    }

    fs::remove(outputFileName);

    return 0;
}
