// Copyright (c) 2024 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "zip.h"

#include "init.h"
#include "logging.h"
#include "minizip/unzip.h"
#include "util.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

bool CZipWrapper::ExtractZip(
    const std::string& zipfile,
    const std::string& destinationFolder)
{
    try {
        // Open the zip file
        unzFile unzip = unzOpen(zipfile.c_str());
        if (!unzip) {
            LogPrintf(
                "CZipWrapper::%s: Error opening zip file: %s\n", 
                __func__, 
                zipfile);
            return false;
        }

        // Create the output folder if it doesn't exist
        if (!fs::exists(destinationFolder) &&
            !fs::create_directory(destinationFolder)) 
        {
            LogPrintf(
                "CZipWrapper::%s: Error creating output folder: %s\n", 
                __func__, 
                destinationFolder);
            unzClose(unzip);
            return false;
        }

        // Get info from the zip file
        unz_global_info globalInfo;
        if (unzGetGlobalInfo(unzip, &globalInfo) != UNZ_OK) {
            LogPrintf(
                "CZipWrapper::%s: Error getting global info from zip file: %s\n", 
                __func__, 
                zipfile);
            unzClose(unzip);
            return false;
        }

        // Extract each file one by one
        for (uLong i = 0; i < globalInfo.number_entry; ++i) {

            if(ShutdownRequested()) {
                LogPrintf(
                    "CZipWrapper::%s: Shutdown requested while unzipping the zip file: %s\n", 
                    __func__, 
                    zipfile);
                unzClose(unzip);
                return false;
            }

            char filename[256];
            unz_file_info fileInfo;

            // Get info about the current file being extracted
            if (unzGetCurrentFileInfo(
                    unzip, &fileInfo, filename, sizeof(filename),
                    nullptr, 0, nullptr, 0) != UNZ_OK) 
            {
                LogPrintf(
                    "CZipWrapper::%s: Error getting file info from zip file: %s\n", 
                    __func__, 
                    zipfile);
                unzClose(unzip);
                return false;
            }

            // Open the current file inside the zip file
            if (unzOpenCurrentFile(unzip) != UNZ_OK) {
                LogPrintf(
                    "CZipWrapper::%s: Error opening current file in zip file: %s\n", 
                    __func__, 
                    zipfile);
                unzClose(unzip);
                return false;
            }

            const auto fullpath = destinationFolder + "/" + filename;
            LogPrintf("CZipWrapper::%s: Extracting file: %s\n", __func__, fullpath);

            // Is it a directory?
            if (IsDirectory(fullpath)) {
                fs::create_directory(fullpath);
            } else { // It is a file!
                std::ofstream output(fullpath, std::ios::binary);
                if (!output.is_open()) {
                    LogPrintf(
                        "CZipWrapper::%s: Error creating output file: %s\n", 
                        __func__, fullpath);
                    unzCloseCurrentFile(unzip);
                    unzClose(unzip);
                    return false;
                }

                // Read and write the file data
                char buffer[4096];
                int bytesRead;

                while ((bytesRead = unzReadCurrentFile(unzip, buffer, sizeof(buffer))) > 0) {
                    output.write(buffer, bytesRead);
                }
                output.close();
            }

            // Close the current file in the zip, and move to the next
            unzCloseCurrentFile(unzip);

            if (unzGoToNextFile(unzip) != UNZ_OK) {
                break; // Reached the end of the zip file
            }
        }

        // Close the zip file
        if (unzClose(unzip) != UNZ_OK) {
            LogPrintf(
                "CZipWrapper::%s: Error closing zip file: %s\n", 
                __func__, zipfile);
            return false;
        }

    } catch (const std::exception& e) {
        LogPrintf(
            "CZipWrapper::%s: Error extracting zip file: %s\n", 
            __func__, e.what());
        return false;
    }

    return true;
}
