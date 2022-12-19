// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CRYPTO_GOOGLE_AUTHENTICATOR_H
#define CRYPTO_GOOGLE_AUTHENTICATOR_H

#include "utilstrencodings.h"
#include <string>

/** A Google Authenticator support class. */
class GoogleAuthenticator
{
private:
    static const int INTERVAL_LENGHT = 30;
    static const int PIN_LENGHT = 6;

    std::string seed;

public:
    GoogleAuthenticator(std::string base32_seed)
    {
        seed = DecodeBase32(base32_seed);
    }

    int GeneratePin();

    static std::string CreateNewSeed(int size = 20);
};

#endif // CRYPTO_GOOGLE_AUTHENTICATOR_H
