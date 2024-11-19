// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2016-2020 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPORKID_H
#define SPORKID_H

/*
    Don't ever reuse these IDs for other sporks
    - This would result in old clients getting confused about which spork is for what
*/

enum SporkId : int32_t {
    SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT      = 10007,
    SPORK_14_MIN_PROTOCOL_ACCEPTED              = 10013,

    SPORK_101_SERVICES_ENFORCEMENT              = 10100,
    SPORK_102_FORCE_ENABLED_MASTERNODE          = 10101,
    SPORK_103_PING_MESSAGE_SALT                 = 10102,
    SPORK_104_MAX_BLOCK_TIME                    = 10103,
    SPORK_105_MAX_BLOCK_SIZE                    = 10104,
    SPORK_106_STAKING_SKIP_MN_SYNC              = 10105,
    SPORK_107_IGNORE_COLLATERAL_CONFIRMATIONS   = 10106,
    SPORK_108_FORCE_MASTERNODE_MIN_AGE          = 10107,
    SPORK_109_FORCE_ENABLED_VOTED_MASTERNODE    = 10108,
    SPORK_110_FORCE_ENABLED_MASTERNODE_PAYMENT  = 10109,
    SPORK_111_ALLOW_DUPLICATE_MN_IPS            = 10110,
    SPORK_112_MASTERNODE_LAST_PAID_V2           = 10111,
    SPORK_114_MN_PAYMENT_V2                     = 10113,
    SPORK_115_MN_COLLATERAL_WINDOW              = 10114,
    SPORK_118_MIN_BASE_FEE                      = 10117,
    SPORK_119_MIN_ECON_FEE                      = 10118,

    SPORK_2_NOOP                                = 10001,
    SPORK_3_NOOP                                = 10002,
    SPORK_5_NOOP                                = 10004,
    SPORK_7_NOOP                                = 10006,
    SPORK_9_NOOP                                = 10008,
    SPORK_10_NOOP                               = 10009,
    SPORK_11_NOOP                               = 10010,
    SPORK_12_NOOP                               = 10011,
    SPORK_13_NOOP                               = 10012,
    SPORK_15_NOOP                               = 10014,
    SPORK_16_NOOP                               = 10015,
    SPORK_17_NOOP                               = 10016,
    SPORK_18_NOOP                               = 10017,
    SPORK_19_NOOP                               = 10018,
    SPORK_113_NOOP                              = 10112,
    SPORK_116_NOOP                              = 10115,
    SPORK_117_NOOP                              = 10116,
    SPORK_INVALID                               = -1
};

// Default values
struct CSporkDef
{
    CSporkDef(): sporkId(SPORK_INVALID), defaultValue(0) {}
    CSporkDef(SporkId id, int64_t val, std::string n): sporkId(id), defaultValue(val), name(n) {}
    SporkId sporkId;
    int64_t defaultValue;
    std::string name;
};

#endif
