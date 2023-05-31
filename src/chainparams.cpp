// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "base58.h"
#include "chainparamsseeds.h"
#include "consensus/merkle.h"
#include "script/standard.h"
#include "util.h"
#include "utilstrencodings.h"

#include <boost/assign/list_of.hpp>

#include <assert.h>

#define DISABLED 0x7FFFFFFE;

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.nVersion = nVersion;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of the genesis coinbase cannot
 * be spent as it did not originally exist in the database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "COINTELEGRAPH 17/Dec/2022 SEC was “asleep at the wheel” about FTX - US Rep. Sessions";
    const CScript genesisOutputScript = CScript() << ParseHex("041bc0301bf765c0ba2047937f7c1c0386bbec764b2c023751dbbbb1c4bebffc74e405340552a6591325da57dc8a4906e0e544650530b895b734d7c0d9faac30bd") << OP_CHECKSIG;

    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
static Checkpoints::MapCheckpoints mapCheckpoints =
    boost::assign::map_list_of
    (0, uint256S("000001120699c31c261884ac30ac6036e00c4689057a129ff60ae332fa72208c"))
    (2341, uint256S("92057b3edaa0fc41d998e69d41d2af9f9e7e7be68eabc01ecf7ade832eee6bc3"))
    (4683, uint256S("5da6119ce297699cda96fa19c77aae46e8bd5a7e2e15f8fc2f6fb91453629923"))
    (7025, uint256S("371fddbbd639c741769457fdf1ca49aa4347e3d4d39e3b5bd11c271f20b53114"))
    (9367, uint256S("9d01c047334cf55bf36239f7c760d9beaadc85e4463682a2586db11849f3739e"))
    (11709, uint256S("46d0c55964ff0b9e1b8e7d454f41457449694bbb3ef7e5eccbdaac81e586a98d"))
    (14051, uint256S("e61297e3b0693cd09bf912f0f8832d39a44fa86d1c06d2e14625792ce210a21c"))
    (16393, uint256S("59ca1c437516eb5ac498acb95540aa1086fd4f13921c85dfa33e1ae4586bb24d"))
    (18735, uint256S("b2318df2392ec6ec6918ae04160b30e34a147e9e00ac02fbc3c31cbd874a2f77"))
    (21077, uint256S("70496c73667ca58bec29010204507d11ed7ebd828de643a7f433db4d4be06552"))
    (23419, uint256S("f3183066c5d0f84a965a4390a285be1571be77746e5eaaeaaad224946352e312"))
    (25761, uint256S("1c24ec46fd4bb4ed8107a9d0d7a677ec7d2f306015bd7451ace254caff106653"))
    (28103, uint256S("a720d1d32e4974ea29a140cfd3e0a8de4b45a493bb88a5ee930fa24337fc20a4"))
    (30445, uint256S("851239889bc08f678ce4302c3c363420661a77b8141f1976cc51ce81e1b44c9b"))
    (32787, uint256S("f8a1eb023af88aeb4eb228a93ddcf04d98032965bc8f0f3fa191395c38c6cb6e"))
    (35129, uint256S("5ae668320cd6070aea0a15ab7f65ac20932122e377e732f9d57be4b7a2981a04"))
    (37471, uint256S("7353069cbd474c6d4901094fed98358b9a578e8234439f7dcc083058f0ed9f46"))
    (39813, uint256S("74f55fe75edc0af9a38f85d3eabcdf75c270325f90bf73f4137a201fec7d44b4"))
    (42155, uint256S("6463425da85e2abaaf660091925548d6ef9892feb282c736e6e46ae330925255"))
    (44497, uint256S("79d43e7436950003b6babe2698ab0a799eeac59e24edf5a220005d376f8992cc"))
    (46839, uint256S("abf7770efca7b40e2f46fd1e14599f28875f970cdfcb5d41e360e4acfa629738"))
    (49181, uint256S("c050933cb6e71245171e3bcf09b9a41d533f9b6895da04a5dfc6f21c708e0daa"))
    (51523, uint256S("5cca4cd5545b3415e85658ddea95451b20637af3b188f8b4154629bc3778471f"))
    (53865, uint256S("e7cec2baafd5504020617604ea104e711bc259c74dd65861616e3f2fe98433e2"))
    (56207, uint256S("28c410f8b07abdfd7b399862fb37652ab991c18f50274617e37203c9ccac98cc"))
    (58549, uint256S("bd428011990ccef145b05768d301b425a5aa78bf28c53cc7e1be0b38abb07046"))
    (60891, uint256S("84892fc8514d1647658276e40d47a319ad508d796c639f0768c17dbfa4ee165d"))
    (63233, uint256S("14ae1d9f1b61e06171f096dac749935e6d42a0e0712316826d55821648714afd"))
    (65575, uint256S("92d4ceae3d2ab89c263d157740e9f78dd94697735f81c55a9c69022c07ef18ac"))
    (67917, uint256S("b1fb29caf0ac7d0deb9b8dea1322ad74dfd100cd4a9040c3c3f4adcab89b3b99"))
    (70259, uint256S("9e2c8c2a2c6213df2198211dace2d5738b1c786d9e82e2cc7c512f63956d8cf1"))
    (72601, uint256S("8e2efec829902d8076c3ff94c6edfb8d9c2956cbebf3b178a7f51793f968e5a7"))
    (74943, uint256S("30b030ab7c9cf9e0a5d03e3c98e3249462ec32dc39c4213449a352cbcbd62a61"))
    (77285, uint256S("fe6b5f0dc134d2f94603e9c6709eafa643e947d6116e15cf9c0b0fc782928fba"))
    (79627, uint256S("67f2859cdd184c2094d69bdaaf843604f122ad68df2627215b679db0a56d0830"))
    (81969, uint256S("2b25c11326e3c0ead7c87148c6d679eadaceec3ea87c3f9080d69a1e3bed579c"))
    (84311, uint256S("b56a7e30dd4a2029b434e930b22d509dd6717609601ac5a3fc765c65a5b3278d"))
    (86653, uint256S("c1128a816714d4fef569b10692c149aba939b787ace4103216a332049b4c57ce"))
    (88995, uint256S("e29a746bd36c1a9df19e78fa8d8a1b525a996a786ab55554b7a602cba2ed7fc2"))
    (91337, uint256S("ea3a413c437f519555a60ef6897976a74796486c8106836a5763f23e6440184b"))
    (93679, uint256S("630c5819f9626c490d503fb4df6e499911e3a458130761fb39169093cd96cf23"))
    (96021, uint256S("720c60119d96008c3b7be264dc3dc4ba51311aa9b574815349fa4e8413870175"))
    (98363, uint256S("022ab9cb4eee8e4b6b6c57f5f88e95b90e934b1a4763238e4ea7eff2bc128ad0"))
    (100705, uint256S("be5ce5acfcf93da122dcb457342ec5d468cb56f97d9f0603595159737152c5de"))
    (103047, uint256S("d901bb37ad7812ad3ed014c8bb5e4d42dd6b6fcd3b4f009d37bfbdef30f9036f"))
    (105389, uint256S("26f3e34b16e482d68d2c58c333c836c94c07aacc5c3895d817e4295bc58d3298"))
    (107731, uint256S("f2e3a45d0c6bb13086cfdfafb7bd3859052f9286ab407ea67165f7db6ffa507d"))
    (110073, uint256S("44db42ed5e41d1b25bc69a56300d10fdfe2a9f25ece6098bc61426ce0b880c8c"))
    (112415, uint256S("449994d02819392ec351f6645b34bee378e2a1ac7ad6db2eb02fbea3e74e4e9f"))
    (114757, uint256S("3019c5991f487aebe6dc230f4b028570cc9fa641a6ce749e4eec75179a679c42"))
    (117099, uint256S("4efdf6a8f771338f4402114c31a3d06f3720dce741cf45dd2c4295e5eaf177d6"))
    (119441, uint256S("8542a32811b8b366c7bae9dac72da9b251c0d42a5500cfbc01bbe1ffb5be4335"))
    (121783, uint256S("4be2bb828cc5a70bcc0e2ece1831d2cf4759db5c4dbdb3a0d378985cb0dbe747"))
    (124125, uint256S("019b21f9d1a6b2ecdc04098703289b30ab66cb3388a60c4725e5cd321b3311da"))
    (126467, uint256S("210692ce64abf7cc12892f0b72ae63c09652f59e4ce051a5d70708b4ac5160bd"))
    (128809, uint256S("4122590b3f3d2471dd92b311ef3776e03d6f6d523db662d7d67c12b38be26c4e"))
    (131151, uint256S("db49df1b3b51035b71356c4862d17349fde682115f5e047859530921371c91fa"))
    (133493, uint256S("c46f6bcd344461f93a4c9d7f53c6f03982c66fc97ec5b40419023c6d65c65bda"))
    (135835, uint256S("bb42149469cd031ef8377b6e425ba142b607a8e9d42861e0795af993e203d590"))
    (138177, uint256S("842bec83e95dcb7a7e413fe05ecf251b838ede331a615d1c2ce55d4ca284b842"))
    (140519, uint256S("29d998c5df9b8873e0a04d44ea69a75b3a2a6d5996e02c75f0c3c25b57811ad5"))
    (142861, uint256S("6111ab21e9ada5e061a218305105e61680843104bb1494c52243dba71e429f59"))
    (145203, uint256S("f3f8adee3b2d7a1bb22ff92492abe242d1052d1bb43024d81d4a1f1b53ebb22b"))
    (147545, uint256S("50894da3d1fe46c778e3ca825be838329bf7b2d3b2523ca767c31f547b42ee63"))
    (149887, uint256S("d02223b6eeb9bd24a41eb67d9834584b3478699b0ed5883cb5a95939c40f660d"))
    (152229, uint256S("c7b37282224aa1d0dcf94241deee1da04061d215ded4e0a43478b99c510e0785"))
    (154571, uint256S("deaf5607eb6f6c5fb3266edc590235878373b0fe5c38027ead6db61bc8c96e31"))
    (156913, uint256S("ddf2a1914f2f6b84a4d72c6f983356d75396c7ea8ec7789ed12574d01be43d72"))
    (159255, uint256S("2eafe469172fe0c5c5671ee22319f2cc108176b89e5e3fe23f7e2ccf1194bcf5"))
    (161597, uint256S("8174e8e76ce6e2b7bd3cf193df939cfd06617cdbedfc3390abb7a7fab0cad520"))
    (163939, uint256S("dffb5d9c9a93897142bb7b90bcea5589ac04f0da1e831ca2674ec4c056f650eb"))
    (166281, uint256S("3d77eb14cffcd2b9b38d0920318d94b5a709ea26bfdd42ef3502a63a0a46cf82"))
    (168623, uint256S("9f650b9f8d10d49a101179c4284aff0dc26d963bbcc88fd0c59ff7f9c2850ff0"))
    (170965, uint256S("d28deba7ad5f16f910695be3bb9b3406fa80c2f0ab5506b3f0992bdc2141a994"))
    (173307, uint256S("b0d20cfac65329ccdaae12c98febd357f93c2557801598cf8a35777e789696bf"))
    (175649, uint256S("279e9df9c3778b3ea46f20c8a7e1d446bdcc9b14957a8952adcfae32d467817a"))
    (177991, uint256S("4fd03f5f3ff39a868ba2fdb1888bdf7bfbb6d97b9c297727c629b0d393a6b7a2"))
    (180333, uint256S("64e9c38ccd8aa71bd539cbcc63133b38fbf2ad63fd87ad6a174eab3b6ab992f8"))
    (182675, uint256S("1dded556cd217160c3545d092b9ad92f3fea3b012dd353d26023595084acc40c"))
    (185017, uint256S("97ae5a8fd23f6be8c70a99a90d4db7e6d17682e380013a000c0e686ea18ae034"))
    (187359, uint256S("e5fa349d88b4d90c52e8d26b1eaef5f5b247d52651314078c7a9bb0cd415291e"))
    (189701, uint256S("ce21f938a71c4f4c62e1e66f97f8df69ff171e581b9d8dd6893749efa122b536"))
    (192043, uint256S("b21459915f1ff8807aac96d4d13c3001a9c12968b5765e45b34490442cb02042"))
    (194385, uint256S("5d938a46100ae0520a912e81d9594f0e9522a65481c076ffe665c70ccd088552"))
    (196727, uint256S("8c2bd9f0820e46147bbc20d8fa9eb7b43b2e8a17a7d17d2ccd20490618d7a0ee"))
    (199069, uint256S("9f3f06b4163aa9b3d825ca79d28e8969787e934a5f25e68ca3c12febb3c00b1d"))
    (201411, uint256S("e66e43ef4681e6b3f23f7c516826ba0d183c75be7df412f5c1c4eac716e70aaa"))
    (203753, uint256S("66f0b894aa318b8ac3ea98dadededc323852fc6bbeac4a8eee6508d268e9e0ed"))
    (206095, uint256S("bba147a76d4ae8d13885584b091c9954ce33fcd5aa3381f253c45c3920449859"))
    (208437, uint256S("db477f5fa77a629d403fcdc62bb33d5e0283d76cef0e8cc78efdc7077c5c8b29"))
    (210779, uint256S("d22a9ccaa1b1be825834671771d2de4063bfdc73c4a045a7e16391790c850565"))
    (213121, uint256S("31d6facb3a439ff6178f0be00ba4275893b306ae9b8c529cc0613fd5f901ea6e"))
    (215463, uint256S("880cbe714a4998d429f1e7b30c3919ee3eda8c5acc71169bf4b22afb52206b7b"))
    (217805, uint256S("9a44ada7b5c83f29617e77e96edfe316ad731ca02670d8e4324235444012d079"))
    (220147, uint256S("49694e63fa36bddf61543cef176276bcfc501d7f6a8ad169d6429c8edb2fc377"))
    (222489, uint256S("02335670adf576f6816342d6b7c0313a696467b95673647258291159c1009430"))
    (224831, uint256S("1cf70e7f1b3eb20b65853b8c0d570e96bac5993947c9b245c60a151e8a700f79"))
    (227173, uint256S("4ecfce645d5557f49710603f684ddc92c03726112cac14513b8804015a08a87b"))
    (229515, uint256S("d956c05b706f6ba822660f2664bd9f0dc60165cf09f24a7d30c1673303354fb3"))
    (231857, uint256S("4c2560e8d2398cec5f7aaa7a324020fe9f7ca40ce6e22e01d55946610e5f86bd"))
    (234199, uint256S("66682c69d8c30f4cadf34a3731b665893097676c7f533dfb217f1156ae33e354"))
;

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1685553210, // * UNIX timestamp of last checkpoint block
    507360,     // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the UpdateTip debug.log lines)
    2926        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of
    (0, uint256S("0x0"));

static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1740710,
    0,
    250};

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of(0, uint256S("0x0"));

static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1454124731,
    0,
    100};

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";

        // // This is used inorder to mine the genesis block. Once found, we can use the nonce and block hash found to create a valid genesis block
        // /////////////////////////////////////////////////////////////////

        // uint32_t nGenesisTime = 1671465720;

        // arith_uint256 test;
        // bool fNegative;
        // bool fOverflow;
        // test.SetCompact(0x1e0ffff0, &fNegative, &fOverflow);
        // std::cout << "Test threshold: " << test.GetHex() << "\n\n";

        // int genesisNonce = 0;
        // uint256 TempHashHolding = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        // uint256 BestBlockHash = uint256S("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        // for (int i=0;i<40000000;i++) {
        //     genesis = CreateGenesisBlock(nGenesisTime, i, 0x1e0ffff0, 1, 0 * COIN);
        //     //genesis.hashPrevBlock = TempHashHolding;
        //     consensus.hashGenesisBlock = genesis.GetHash();

        //     arith_uint256 BestBlockHashArith = UintToArith256(BestBlockHash);
        //     if (UintToArith256(consensus.hashGenesisBlock) < BestBlockHashArith) {
        //         BestBlockHash = consensus.hashGenesisBlock;
        //         std::cout << BestBlockHash.GetHex() << " Nonce: " << i << "\n";
        //         std::cout << "   PrevBlockHash: " << genesis.hashPrevBlock.GetHex() << "\n";
        //     }

        //     TempHashHolding = consensus.hashGenesisBlock;

        //     if (BestBlockHashArith < test) {
        //         genesisNonce = i - 1;
        //         break;
        //     }
        //     //std::cout << consensus.hashGenesisBlock.GetHex() << "\n";
        // }
        // std::cout << "\n";
        // std::cout << "\n";
        // std::cout << "\n";

        // std::cout << "nGenesisTime to " << nGenesisTime << std::endl;
        // std::cout << "hashGenesisBlock to 0x" << BestBlockHash.GetHex() << std::endl;
        // std::cout << "Genesis Nonce to " << genesisNonce << std::endl;
        // std::cout << "Genesis Merkle 0x" << genesis.hashMerkleRoot.GetHex() << std::endl;

        // exit(0);

        // /////////////////////////////////////////////////////////////////

        genesis = CreateGenesisBlock(1671465720, 1187616, 0x1e0ffff0, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000001120699c31c261884ac30ac6036e00c4689057a129ff60ae332fa72208c"));
        assert(genesis.hashMerkleRoot == uint256S("0x974c0a0e71ef29244b04ca29e2a308efde9cb295ad39beca871d208e79d2ca78"));

        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.powLimit   = ~UINT256_ZERO >> 20;
        consensus.posLimitV1 = ~UINT256_ZERO >> 24;
        consensus.posLimitV2 = ~UINT256_ZERO >> 20;
        consensus.nCoinbaseMaturity = 100;
        consensus.nFutureTimeDriftPoW = 120;
        consensus.nFutureTimeDriftPoS = 120;
        consensus.nMaxMoneyOut = 9999999999 * COIN;
        consensus.nPoolMaxTransactions = 3;
        consensus.nStakeMinAge = 60 * 60; // 1h
        consensus.nStakeMinDepth = 100;
        consensus.nStakeMinDepthV2 = 600;
        consensus.nTargetTimespan = 40 * 60;
        consensus.nTargetTimespanV2 = 2 * 60 * 60;
        consensus.nTargetSpacing = 60;
        consensus.nTimeSlotLength = 15;

        // spork keys
        consensus.strSporkPubKey = "036495ceb2a9c576177816d77152880e024ad4d0c6c276e02210ef1ac7559db173";
        consensus.strSporkPubKeyOld = "";
        consensus.nTime_EnforceNewSporkKey = 0;
        consensus.nTime_RejectOldSporkKey = 0;

        // burn addresses
        consensus.mBurnAddresses = {
            { "FXXXXXXXXXXXXXXXXXXXXXXXXXXXVqtnVL", 0 },
            { "FrCzb47ToX5hPwvNrssN3KH4g1rcd7RoYQ", 0 }
        };

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight                   = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight              = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight                    = 1001;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight                 = 1101;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight                  = 1201;
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MODIFIER_V2].nActivationHeight      = 1301;
        consensus.vUpgrades[Consensus::UPGRADE_TIME_PROTOCOL_V2].nActivationHeight       = 1401;
        consensus.vUpgrades[Consensus::UPGRADE_P2PKH_BLOCK_SIGNATURES].nActivationHeight = 1501;
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MIN_DEPTH_V2].nActivationHeight     = 1501;
        consensus.vUpgrades[Consensus::UPGRADE_MASTERNODE_RANK_V2].nActivationHeight     = 1501;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xe1;
        pchMessageStart[1] = 0xf4;
        pchMessageStart[2] = 0xc2;
        pchMessageStart[3] = 0xef;
        nDefaultPort = 32972;

        vSeeds.push_back(CDNSSeedData("seeder", "seeder.flitswallet.app"));
	    vSeeds.push_back(CDNSSeedData("seed1",   "seed1.flitswallet.app"));
        vSeeds.push_back(CDNSSeedData("seed2",   "seed2.flitswallet.app"));
        vSeeds.push_back(CDNSSeedData("seed3",   "seed3.flitswallet.app"));
        vSeeds.push_back(CDNSSeedData("seed4",   "seed4.flitswallet.app"));
	    vSeeds.push_back(CDNSSeedData("seed5",   "seed5.flitswallet.app"));
	    vSeeds.push_back(CDNSSeedData("seed6",   "seed6.flitswallet.app"));
	    vSeeds.push_back(CDNSSeedData("seed7",   "seed7.flitswallet.app"));
	    vSeeds.push_back(CDNSSeedData("seed8",   "seed8.flitswallet.app"));

        base58Prefixes[PUBKEY_ADDRESS]  = std::vector<unsigned char>(1, 36);
        base58Prefixes[SCRIPT_ADDRESS]  = std::vector<unsigned char>(1, 48);
        base58Prefixes[SECRET_KEY]      = std::vector<unsigned char>(1, 66);
        base58Prefixes[EXT_PUBLIC_KEY]  = boost::assign::list_of(0x02)(0x2D)(0x25)(0x33).convert_to_container<std::vector<unsigned char>>();
        base58Prefixes[EXT_SECRET_KEY]  = boost::assign::list_of(0x02)(0x21)(0x31)(0x2B).convert_to_container<std::vector<unsigned char>>();
        // BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x03)(0x52).convert_to_container<std::vector<unsigned char>>();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));
        //convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main)); // added
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }

};
static CMainParams mainParams;

/**
 * Testnet (v1)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";

        genesis = CreateGenesisBlock(1454124731, 2402015, 0x1e0ffff0, 1, 250 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x0000041e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"));
        //assert(genesis.hashMerkleRoot == uint256S("0x1b2ef6e2f28be914103a277377ae7729dcd125dfeb8bf97bd5964ba72b6dc39b"));

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.powLimit   = ~UINT256_ZERO >> 20;   // flits starting difficulty is 1 / 2^12
        consensus.posLimitV1 = ~UINT256_ZERO >> 24;
        consensus.posLimitV2 = ~UINT256_ZERO >> 20;
        consensus.nCoinbaseMaturity = 100;
        consensus.nFutureTimeDriftPoW = 7200;
        consensus.nFutureTimeDriftPoS = 180;
        consensus.nMaxMoneyOut = 9999999999 * COIN;
        consensus.nPoolMaxTransactions = 3;
        consensus.nStakeMinAge = 60 * 60;
        consensus.nStakeMinDepth = 100;
        consensus.nStakeMinDepthV2 = 600;
        consensus.nTargetTimespan = 40 * 60;
        consensus.nTargetTimespanV2 = 30 * 60;
        consensus.nTargetSpacing = 1 * 60;
        consensus.nTimeSlotLength = 15;

        // spork keys
        consensus.strSporkPubKey = "04E88BB455E2A04E65FCC41D88CD367E9CCE1F5A409BE94D8C2B4B35D223DED9C8E2F4E061349BA3A38839282508066B6DC4DB72DD432AC4067991E6BF20176127";
        consensus.strSporkPubKeyOld = "";
        consensus.nTime_EnforceNewSporkKey = 0;
        consensus.nTime_RejectOldSporkKey = 0;

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight                   = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight              = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight                    = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight                 = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight                  = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MODIFIER_V2].nActivationHeight      = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TIME_PROTOCOL_V2].nActivationHeight       = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_P2PKH_BLOCK_SIGNATURES].nActivationHeight = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MIN_DEPTH_V2].nActivationHeight     = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_MASTERNODE_RANK_V2].nActivationHeight     = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */

        pchMessageStart[0] = 0x45;
        pchMessageStart[1] = 0x76;
        pchMessageStart[2] = 0x65;
        pchMessageStart[3] = 0xba;
        nDefaultPort = 42972;

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.push_back(CDNSSeedData("tseeder", "tseeder.flitswallet.app", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 139); // Testnet flits addresses start with 'x' or 'y'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);  // Testnet flits script addresses start with '8' or '9'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        // Testnet flits BIP32 pubkeys start with 'DRKV'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x3a)(0x80)(0x61)(0xa0).convert_to_container<std::vector<unsigned char> >();
        // Testnet flits BIP32 prvkeys start with 'DRKP'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x3a)(0x80)(0x58)(0x37).convert_to_container<std::vector<unsigned char> >();
        // Testnet flits BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";

        genesis = CreateGenesisBlock(1454124731, 2402015, 0x1e0ffff0, 1, 250 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x0000041e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"));
        //assert(genesis.hashMerkleRoot == uint256S("0x1b2ef6e2f28be914103a277377ae7729dcd125dfeb8bf97bd5964ba72b6dc39b"));

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.powLimit   = ~UINT256_ZERO >> 20;   // flits starting difficulty is 1 / 2^12
        consensus.posLimitV1 = ~UINT256_ZERO >> 24;
        consensus.posLimitV2 = ~UINT256_ZERO >> 20;
        consensus.nCoinbaseMaturity = 100;
        consensus.nFutureTimeDriftPoW = 7200;
        consensus.nFutureTimeDriftPoS = 180;
        consensus.nMaxMoneyOut = 43199500 * COIN;
        consensus.nPoolMaxTransactions = 2;
        consensus.nStakeMinAge = 0;
        consensus.nStakeMinDepth = 2;
        consensus.nTargetTimespan = 40 * 60;
        consensus.nTargetTimespanV2 = 30 * 60;
        consensus.nTargetSpacing = 1 * 60;
        consensus.nTimeSlotLength = 15;

        /* Spork Key for RegTest:
        WIF private key: 932HEevBSujW2ud7RfB1YF91AFygbBRQj3de3LyaCRqNzKKgWXi
        private key hex: bd4960dcbd9e7f2223f24e7164ecb6f1fe96fc3a416f5d3a830ba5720c84b8ca
        Address: yCvUVd72w7xpimf981m114FSFbmAmne7j9
        */
        consensus.strSporkPubKey = "043969b1b0e6f327de37f297a015d37e2235eaaeeb3933deecd8162c075cee0207b13537618bde640879606001a8136091c62ec272dd0133424a178704e6e75bb7";
        consensus.strSporkPubKeyOld = "";
        consensus.nTime_EnforceNewSporkKey = 0;
        consensus.nTime_RejectOldSporkKey = 0;

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 251;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 251;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MODIFIER_V2].nActivationHeight          = 251;
        consensus.vUpgrades[Consensus::UPGRADE_TIME_PROTOCOL_V2].nActivationHeight          =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_P2PKH_BLOCK_SIGNATURES].nActivationHeight       = 300;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */

        pchMessageStart[0] = 0xa1;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0x7e;
        pchMessageStart[3] = 0xac;
        nDefaultPort = 52972;

        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }

    void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
    {
        assert(idx > Consensus::BASE_NETWORK && idx < Consensus::MAX_NETWORK_UPGRADES);
        consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
    }
};
static CRegTestParams regTestParams;

static CChainParams* pCurrentParams = 0;

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    regTestParams.UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}
