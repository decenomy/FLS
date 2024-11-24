// Copyright (c) 2024 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZIP_H
#define ZIP_H

#include <string>

class CZipWrapper
{
public:
    static bool ExtractZip(
        const std::string& zipfile, 
        const std::string& outputPath);
};

#endif // ZIP_H
