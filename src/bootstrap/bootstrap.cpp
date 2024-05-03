
#include "bootstrap.h"

namespace fs = boost::filesystem;
static bool log_flag = false;

bool Bootstrap::rmDirectory(const std::string& directory_path) {

    try {
        // Check if the directory exists
        if (fs::exists(directory_path)) {
            // Remove the directory and its contents
            fs::remove_all(directory_path);
            //LogPrintf("-bootstrap: Directory removed successfully.");
        } else {
            //LogPrintf("-bootstrap: Directory does not exist.");
        }
    } catch (const fs::filesystem_error& ex) {
        //LogPrintf("-bootstrap: Error removing directory: " << ex.what());
        return false;
    }

    return true;
}

bool Bootstrap::isDirectory(const std::string& directory_path) {

    if (fs::exists(directory_path)) return true;

    return false;
}

// Callback function to write downloaded data to a file
size_t Bootstrap::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::ofstream* file = static_cast<std::ofstream*>(userp);
    file->write(static_cast<const char*>(contents), total_size);
    return total_size;
}

// Define a function to handle progress updates
int Bootstrap::ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    // Calculate progress percentage
    double progress = (dlnow > 0) ? (dlnow / dltotal) * 100.0 : 0.0;

    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    if(!log_flag && duration.count() % 2 == 0){
        log_flag = true;
        LogPrintf("-bootstrap: Download: %d%%\n", (uint8_t)progress);
        //uiInterface.ShowProgress(_("Download: "), (uint8_t)progress);    
    }else if(duration.count() % 2 != 0) log_flag = false;
    
    return 0;
}

// Function to download a file using libcurl
bool Bootstrap::DownloadFile(const std::string& url, const std::string& outputFileName, curl_progress_callback func) {

    CURL* curl = curl_easy_init();
    if (!curl) {
        //LogPrintf("-bootstrap: Error initializing libcurl.");
        return false;
    }

    curl_version_info_data *info = curl_version_info(CURLVERSION_NOW);

    if (info) {
        // Print libcurl version information
        LogPrintf("-bootstrap: libcurl version: %s \n", info->version);
        LogPrintf("-bootstrap: libcurl SSL version: %s \n", info->ssl_version);
        //LogPrintf("-bootstrap: libcurl zlib version: %s \n", info->libz_version);
    } else {
        LogPrintf("-bootstrap: Failed to retrieve libcurl version information.\n");
    }

    std::ofstream outputFile(outputFileName, std::ios::binary);
    if (!outputFile.is_open()) {
        LogPrintf("-bootstrap: Error opening output file.");
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // Verify the peer's SSL certificate
    curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputFile);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    if(func == nullptr)
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
    else
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, func);

    #if defined(__APPLE__)
        LogPrintf("-bootstrap: apple ca path: %s \n",(const char*)APPLE_CA_PATH);
        curl_easy_setopt(curl, CURLOPT_CAINFO, (const char*)APPLE_CA_PATH);
    #elif defined(__linux__)
        //LogPrintf("-bootstrap: linux ca path: " << (const char*)LINUX_CA_PATH);
        //curl_easy_setopt(curl, CURLOPT_CAINFO, (const char*)LINUX_CA_PATH);
    #elif defined(_WIN32)
        ///LogPrintf("-bootstrap: windows ca path: " << (const char*)WIN_CA_PATH);
        //curl_easy_setopt(curl, CURLOPT_CAINFO, (const char*)WIN_CA_PATH);
    #else
        LogPrintf("-bootstrap: OS not recognized, CA Path not defined");
    #endif

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    outputFile.close();

    if (res != CURLE_OK) {
        LogPrintf("-bootstrap: Error downloading file: %s \n",curl_easy_strerror(res));
        return false;
    }

    return true;
}

bool Bootstrap::extractZip(const std::string& zipFilePath, const std::string& outputFolderPath) {

    // Open the zip file
    unzFile zipFile = unzOpen(zipFilePath.c_str());
    if (!zipFile) {
        LogPrintf("-bootstrap: Error opening zip file: %s \n",zipFilePath);
        return false;
    }

    // Create the output folder if it doesn't exist
    if (!ensureOutputFolder(outputFolderPath)) {
        LogPrintf("-bootstrap: Error creating output folder: %s \n",outputFolderPath);
        unzClose(zipFile);
        return false;
    }

    // Go through each file in the zip and extract it
    unz_global_info globalInfo;
    if (unzGetGlobalInfo(zipFile, &globalInfo) != UNZ_OK) {
        LogPrintf("-bootstrap: Error getting global info from zip file.");
        unzClose(zipFile);
        return false;
    }

    for (uLong i = 0; i < globalInfo.number_entry; ++i) {
        char fileName[256];
        unz_file_info fileInfo;

        if (unzGetCurrentFileInfo(zipFile, &fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0) != UNZ_OK) {
            LogPrintf("-bootstrap: Error getting file info from zip file.");
            unzClose(zipFile);
            return false;
        }

        if (unzOpenCurrentFile(zipFile) != UNZ_OK) {
            LogPrintf("-bootstrap: Error opening current file in zip.");
            unzClose(zipFile);
            return false;
        }

        std::string outputPath = std::string(outputFolderPath) + "/" + fileName;
        LogPrintf("-bootstrap: extract file: %s \n",fileName);

        if(endsWithSlash(outputPath))
            ensureOutputFolder(outputPath);
        else{
            std::ofstream outFile(outputPath, std::ios::binary);
            if (!outFile.is_open()) {
                LogPrintf("-bootstrap: Error creating output file: %s \n",outputPath);
                unzCloseCurrentFile(zipFile);
                unzClose(zipFile);
                return false;
            }

            // Read and write the file data
            char buffer[4096];
            int bytesRead;

            while ((bytesRead = unzReadCurrentFile(zipFile, buffer, sizeof(buffer))) > 0) {
                outFile.write(buffer, bytesRead);
            }
            outFile.close();
        }


        unzCloseCurrentFile(zipFile);

        if (unzGoToNextFile(zipFile) != UNZ_OK) {
            break;  // Reached the end of the zip file
        }
    }

    // Close the zip file
    unzClose(zipFile);

    LogPrintf("-bootstrap: Zip extraction successful.");

    return true;

}

bool Bootstrap::ensureOutputFolder(const std::string& outputPath) {
    try {
        if (!fs::exists(outputPath)) {
            // Create the directory if it doesn't exist
            fs::create_directories(outputPath);
        } else if (!fs::is_directory(outputPath)) {
            // If it exists but is not a directory, print an error
            LogPrintf("-bootstrap: Error: Output path %s is not a directory.", outputPath);
            return false;
        }
    } catch (const std::exception& e) {
        // Handle any exceptions that may occur during filesystem operations
        LogPrintf("-bootstrap: Error creating output folder: %s \n",e.what());
        return false;
    }

    return true;
}

bool Bootstrap::endsWithSlash(const std::string& str) {
    // Check if the string ends with '/'
    return !str.empty() && str.back() == '/';
}
