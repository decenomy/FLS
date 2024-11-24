// Copyright (c) 2024 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "curl.h"

#include "init.h"
#include "logging.h"

// Callback function to write downloaded data to a file
size_t writeCallback(void* data, size_t size, size_t nmemb, void* clientp)
{
    if(ShutdownRequested()) {
        LogPrintf(
            "CCurlWrapper::%s: Shutdown requested while downloading a file\n", 
            __func__);
        return 0;
    }

    size_t total_size = size * nmemb;
    std::ofstream* file = static_cast<std::ofstream*>(clientp);
    file->write(static_cast<const char*>(data), total_size);
    return total_size;
}

#ifndef WIN32

bool fileExists(const std::string &path) {
    std::ifstream file(path);
    return file.good();
}

std::string findCAPath() {
    std::vector<std::string> commonPaths = {
        // Linux common locations
        "/etc/ssl/certs/ca-certificates.crt",
        "/etc/pki/tls/certs/ca-bundle.crt",
        "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem",
        "/etc/pki/tls/cert.pem",
        // macOS common locations
        "/etc/ssl/cert.pem",
        "/usr/local/etc/openssl/cert.pem"
    };

    // Search through the common paths
    for (const auto &path : commonPaths) {
        if (fileExists(path)) {
            LogPrintf(
                "CCurlWrapper::%s: CA certificate found: %s\n", 
                __func__, path);
            return path;
        }
    }

    // If no file is found, return an empty string
    LogPrintf(
        "CCurlWrapper::%s: No CA certificate found!\n", 
        __func__);
    return "";
}

std::string caPath = "";

#endif

bool CCurlWrapper::DownloadFile(
    const std::string& url,
    const std::string& filename,
    curl_xferinfo_callback xferinfoCallback)
{
    try {
        // Initializes libcurl
        const auto curl = curl_easy_init();
        if (!curl) {
            LogPrintf(
                "CCurlWrapper::%s: Error initializing libcurl.\n", __func__);
            return false;
        }

        // Gets libcurl version information
        const auto info = curl_version_info(CURLVERSION_NOW);
        if (info) {
            LogPrintf(
                "CCurlWrapper::%s: libcurl version: %s\n", 
                __func__, info->version);
            LogPrintf(
                "CCurlWrapper::%s: libcurl SSL version: %s\n", 
                __func__, info->ssl_version);
            LogPrintf(
                "CCurlWrapper::%s: libcurl zlib version: %s\n", 
                __func__, info->libz_version);
        } else {
            LogPrintf(
                "CCurlWrapper::%s: Failed to retrieve libcurl version information.\n", 
                __func__);
            return false;
        }

        LogPrintf(
            "CCurlWrapper::%s: Downloading from %s\n", 
            __func__,
            url);

        // Creates and open the destination file
        std::ofstream outputFile(filename, std::ios::binary);
        if (!outputFile.is_open()) {
            LogPrintf(
                "CCurlWrapper::%s: Error opening output file %s\n", 
                __func__, filename);
            return false;
        }

        // Sets url parameter
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Sets HTTPS parameters
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);

#ifndef WIN32
        if (caPath.empty()) {
            caPath = findCAPath();
        }
        if (!caPath.empty()) {
            // Set the path to the CA bundle if found
            LogPrintf("CCurlWrapper::%s: ca path: %s\n", __func__, caPath);
            curl_easy_setopt(curl, CURLOPT_CAINFO, caPath.c_str());
        }
#endif

        // Sets file releated parameters
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputFile);

        // Sets progress function parameters
        if (xferinfoCallback) {
            // Enable the progress function
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
            // Set up the progress function
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfoCallback);
            // Necessary to pass the callback to CURLOPT_XFERINFODATA (not used)
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, NULL); 
        }

        // HTTP call execution
        const auto res = curl_easy_perform(curl);

        // Cleanup
        curl_easy_cleanup(curl);
        outputFile.close();

        // Evaluation and return
        if (res != CURLE_OK) {
            if(!ShutdownRequested()) {
                LogPrintf(
                    "CCurlWrapper::%s: Error downloading file: %s\n", 
                    __func__, curl_easy_strerror(res));
            }
            fs::remove_all(filename);
            return false;
        }
    } catch (const std::exception& e) {
        LogPrintf(
            "CCurlWrapper::%s: Error downloading file: %s\n", 
            __func__, e.what());
        fs::remove_all(filename);
        return false;
    }

    return true;
}
