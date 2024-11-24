// Copyright (c) 2024 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CURL_H
#define CURL_H

#include <curl/curl.h>
#include <string>

class CCurlWrapper
{
public:
    static bool DownloadFile(
        const std::string& url,
        const std::string& filename,
        curl_xferinfo_callback xferinfoCallback = nullptr);
};

#endif // CURL_H
