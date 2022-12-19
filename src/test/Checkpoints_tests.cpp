// Copyright (c) 2011-2013 The Bitcoin Core developers
// Copyright (c) 2017-2020 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for block-chain checkpoints
//

#include "checkpoints.h"

#include "uint256.h"
#include "test_pivx.h"

#include <boost/test/unit_test.hpp>


BOOST_FIXTURE_TEST_SUITE(Checkpoints_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(sanity)
{
    uint256 p0 = uint256S("0x0000095e24c9de08faea91e7dcafda400edcd769c0a4201081966f10bdef7896");
    //TODO: The below commented section should be uncommented when the blockchain have a few blocks mined.
    // uint256 p623933 = uint256S("0xc7aafa648a0f1450157dc93bd4d7448913a85b7448f803b4ab970d91fc2a7da7");
    // BOOST_CHECK(Checkpoints::CheckBlock(259201, p259201));
    // BOOST_CHECK(Checkpoints::CheckBlock(623933, p623933));


    // // Wrong hashes at checkpoints should fail:
    // BOOST_CHECK(!Checkpoints::CheckBlock(259201, p623933));
    // BOOST_CHECK(!Checkpoints::CheckBlock(623933, p259201));

    // // ... but any hash not at a checkpoint should succeed:
    // BOOST_CHECK(Checkpoints::CheckBlock(259201+1, p623933));
    // BOOST_CHECK(Checkpoints::CheckBlock(623933+1, p259201));

    // BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 623933);
}

BOOST_AUTO_TEST_SUITE_END()
