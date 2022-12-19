// Copyright (c) 2012-2013 The Bitcoin Core developers
// Copyright (c) 2017-2019 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key.h"

#include "base58.h"
#include "key_io.h"
#include "script/script.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"
#include "test_pivx.h"

#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>


static const std::string strSecret1  = "7U39csztLTapBC1zsvbJuFekcVPRY4zGhVte5jp2VNBr9jvYQqEs";
static const std::string strSecret2  = "7NjnK7pkWodLpwH2dz7xbqiSGaKKoNDUPms5h1x4326NEXmbd7Ei";
static const std::string strSecret1C = "7U39csztLTapBC1zsvbJuFekcVPRY4zGhVte5jp2VNBr9jvYQqEs";
static const std::string strSecret2C = "7NjnK7pkWodLpwH2dz7xbqiSGaKKoNDUPms5h1x4326NEXmbd7Ei";
static const std::string addr1 = "7QKbVsprNSSpWE71cvvsSSPuC4YS5e6riu";
static const std::string addr2 = "78o7W4D6CwKdr64dKeSZpGCyNL2KKxRhWA";
static const std::string addr1C = "7QKbVsprNSSpWE71cvvsSSPuC4YS5e6riu";
static const std::string addr2C = "78o7W4D6CwKdr64dKeSZpGCyNL2KKxRhWA";


static const std::string strAddressBad ="Xta1praZQjyELweyMByXyiREw1ZRsjXzVP";


#ifdef KEY_TESTS_DUMPINFO
void dumpKeyInfo(uint256 privkey)
{
    CKey key;
    key.resize(32);
    memcpy(&secret[0], &privkey, 32);
    std::vector<unsigned char> sec;
    sec.resize(32);
    memcpy(&sec[0], &secret[0], 32);
    printf("  * secret (hex): %s\n", HexStr(sec).c_str());

    for (int nCompressed=0; nCompressed<2; nCompressed++)
    {
        printf("    * secret (base58): %s\n", EncodeSecret(secret));
        CKey key;
        key.SetSecret(secret, fCompressed);
        std::vector<unsigned char> vchPubKey = key.GetPubKey();
        printf("    * pubkey (hex): %s\n", HexStr(vchPubKey).c_str());
        printf("    * address (base58): %s\n", EncodeDestination(vchPubKey).c_str());
    }
}
#endif

BOOST_FIXTURE_TEST_SUITE(key_tests, TestingSetup)

BOOST_AUTO_TEST_CASE(key_test1)
{
    CKey key1  = KeyIO::DecodeSecret(strSecret1);
    BOOST_CHECK(key1.IsValid() && key1.IsCompressed());
    CKey key2  = KeyIO::DecodeSecret(strSecret2);
    BOOST_CHECK(key2.IsValid() && key2.IsCompressed());
    CKey key1C = KeyIO::DecodeSecret(strSecret1C);
    BOOST_CHECK(key1C.IsValid() && key1C.IsCompressed());
    CKey key2C = KeyIO::DecodeSecret(strSecret2C);
    BOOST_CHECK(key2C.IsValid() && key2C.IsCompressed());
    CKey bad_key = KeyIO::DecodeSecret(strAddressBad);
    BOOST_CHECK(!bad_key.IsValid());

    CPubKey pubkey1  = key1.GetPubKey();
    CPubKey pubkey2  = key2.GetPubKey();
    CPubKey pubkey1C = key1C.GetPubKey();
    CPubKey pubkey2C = key2C.GetPubKey();

    BOOST_CHECK(key1.VerifyPubKey(pubkey1));
    BOOST_CHECK(key1.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2C));

    BOOST_CHECK(key1C.VerifyPubKey(pubkey1));
    BOOST_CHECK(key1C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey1C));
    BOOST_CHECK(key2.VerifyPubKey(pubkey2));
    BOOST_CHECK(key2.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(key2C.VerifyPubKey(pubkey2));
    BOOST_CHECK(key2C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(DecodeDestination(addr1)  == CTxDestination(pubkey1.GetID()));
    BOOST_CHECK(DecodeDestination(addr2)  == CTxDestination(pubkey2.GetID()));
    BOOST_CHECK(DecodeDestination(addr1C) == CTxDestination(pubkey1C.GetID()));
    BOOST_CHECK(DecodeDestination(addr2C) == CTxDestination(pubkey2C.GetID()));

    for (int n=0; n<16; n++)
    {
        std::string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        std::vector<unsigned char> sign1, sign2, sign1C, sign2C;

        BOOST_CHECK(key1.Sign(hashMsg, sign1));
        BOOST_CHECK(key2.Sign(hashMsg, sign2));
        BOOST_CHECK(key1C.Sign(hashMsg, sign1C));
        BOOST_CHECK(key2C.Sign(hashMsg, sign2C));

        //TODO: Fix and enable commented checks
        // BOOST_CHECK( pubkey1.Verify(hashMsg, sign1));
        // BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2));
        // BOOST_CHECK(!pubkey1.Verify(hashMsg, sign1C));
        // BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2C));

        // BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1));
        // BOOST_CHECK( pubkey2.Verify(hashMsg, sign2));
        // BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1C));
        // BOOST_CHECK(!pubkey2.Verify(hashMsg, sign2C));

        // BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign1));
        // BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2));
        // BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1C));
        // BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2C));

        // BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1));
        // BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign2));
        // BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1C));
        // BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2C));

        // compact signatures (with key recovery)

        std::vector<unsigned char> csign1, csign2, csign1C, csign2C;

        BOOST_CHECK(key1.SignCompact(hashMsg, csign1));
        BOOST_CHECK(key2.SignCompact(hashMsg, csign2));
        BOOST_CHECK(key1C.SignCompact(hashMsg, csign1C));
        BOOST_CHECK(key2C.SignCompact(hashMsg, csign2C));

        CPubKey rkey1, rkey2, rkey1C, rkey2C;

        BOOST_CHECK(rkey1.RecoverCompact(hashMsg, csign1));
        BOOST_CHECK(rkey2.RecoverCompact(hashMsg, csign2));
        BOOST_CHECK(rkey1C.RecoverCompact(hashMsg, csign1C));
        BOOST_CHECK(rkey2C.RecoverCompact(hashMsg, csign2C));

        BOOST_CHECK(rkey1  == pubkey1);
        BOOST_CHECK(rkey2  == pubkey2);
        BOOST_CHECK(rkey1C == pubkey1C);
        BOOST_CHECK(rkey2C == pubkey2C);
    }

    // test deterministic signing

    std::vector<unsigned char> detsig, detsigc;
    std::string strMsg = "Very deterministic message";
    uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());
    BOOST_CHECK(key1.Sign(hashMsg, detsig));
    BOOST_CHECK(key1C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc);
    // BOOST_CHECK(detsig.size() == ParseHex("30450221009071d4fead181ea197d6a23106c48ee5de25e023b38afaf71c170e3088e5238a02200dcbc7f1aad626a5ee812e08ef047114642538e423a94b4bd6a272731cf500d0").size());
    BOOST_CHECK(key2.Sign(hashMsg, detsig));
    BOOST_CHECK(key2C.Sign(hashMsg, detsigc));
    BOOST_CHECK(detsig == detsigc);
    BOOST_CHECK(detsig.size() == ParseHex("304402204f304f1b05599f88bc517819f6d43c69503baea5f253c55ea2d791394f7ce0de02204f23c0d4c1f4d7a89bf130fed755201d22581911a8a44cf594014794231d325a").size());
    BOOST_CHECK(key1.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key1C.SignCompact(hashMsg, detsigc));
    BOOST_CHECK(detsig.size() == ParseHex("1b9071d4fead181ea197d6a23106c48ee5de25e023b38afaf71c170e3088e5238a0dcbc7f1aad626a5ee812e08ef047114642538e423a94b4bd6a272731cf500d0").size());
    BOOST_CHECK(detsigc.size() == ParseHex("1f9071d4fead181ea197d6a23106c48ee5de25e023b38afaf71c170e3088e5238a0dcbc7f1aad626a5ee812e08ef047114642538e423a94b4bd6a272731cf500d0").size());
    BOOST_CHECK(key2.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key2C.SignCompact(hashMsg, detsigc));
    BOOST_CHECK(detsig.size() == ParseHex("1b4f304f1b05599f88bc517819f6d43c69503baea5f253c55ea2d791394f7ce0de4f23c0d4c1f4d7a89bf130fed755201d22581911a8a44cf594014794231d325a").size());
    BOOST_CHECK(detsigc.size() == ParseHex("1f4f304f1b05599f88bc517819f6d43c69503baea5f253c55ea2d791394f7ce0de4f23c0d4c1f4d7a89bf130fed755201d22581911a8a44cf594014794231d325a").size());
}

BOOST_AUTO_TEST_SUITE_END()
