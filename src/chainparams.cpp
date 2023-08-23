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
    (3542, uint256S("cdb6273f7f9748910440c7bc458e03417c70c22e5cce8c6ae2166ea55ceb8acb"))
    (7085, uint256S("934d31797f9c97b6ef6ef0f482e6eb060ffd20e3ce7a661b57df7296f76f85e4"))
    (10628, uint256S("035ad9a29867e12dc9b7d865aac5995149b3fca38902160366d49e65cf1b0335"))
    (14171, uint256S("f3cd424d862061c9a51c97066cfd803c4161b12c1a86afc11407ebdc1f6cd22e"))
    (17714, uint256S("12bf4e66dfc7036bbf3b269bb52b3baa27674e1f45c5f69b965917126be942d0"))
    (21257, uint256S("03f72006b237c3eddc5f643edecdbee819bdcc4ca81228271d04c81ffda83a5d"))
    (24800, uint256S("9f47fef709064cf98b4e7b62cb12d26a9147df6c169d009b9706fb3f74416d48"))
    (28343, uint256S("87d990993f2ac1a2933a60a3b9f48b9bd9c909e1910964b6e5bf2fd5a7a7edfc"))
    (31886, uint256S("20285784b644a6c8bd4b8ad39090b01d72d12b730db4d2fb4f9a892247a857fc"))
    (35429, uint256S("d53980a69eda091f9d97c45cef3b3646b6b399defef1936137b8658ea2f11161"))
    (38972, uint256S("ce3dbbad7f2dfaebfcc8d3887b11ca7d9f6cf07078df960a246b8dd9f525d4cf"))
    (42515, uint256S("b15c00b614510ee647f1d0619cf2c86f0e625f53c0ff5ffe3196f03a63147923"))
    (46058, uint256S("8f4d95a4266a6b475175e1a91b84a96d88ad5b06a3cd265478e236916faa4c0a"))
    (49601, uint256S("9958433fb15a06eeff1024e78105d67db13785ad18058631723c5b48820e4c29"))
    (53144, uint256S("73d8c2746cbb41cfecda21a4bc94412778dddd5a6b0d749d66957669e6539b68"))
    (56687, uint256S("cfb195eeeb4036b8abb267aab1d2314cb455f1323d0004f62c587cb38e2d5364"))
    (60230, uint256S("4a006c79098bea9da48df4cde1a0a6c316177bf397e96cd3b8642fc8d67e9fbe"))
    (63773, uint256S("1de5ead450718d7000a399232f9b6f3e0d36494fd6576ae6f1c2d7d772139acf"))
    (67316, uint256S("302131695cb18dde1be08264f0bf2a7c9574302ddfffcab2ada25e3483404d42"))
    (70859, uint256S("8649f5ff3d8fad65f4f59b8b36591e1bc7ee0d49edfc46000f63e1d8befb15e8"))
    (74402, uint256S("f4d96a5d3baf1ec962637c0c0a1515b9109335f476f9bad5ba64a66e9215c080"))
    (77945, uint256S("090f82b5c9437b7342bfe35ce52efe1d025306738612515432ceaa01d9b4f60e"))
    (81488, uint256S("78bf41c3a9b878a72cba1d55550b1c2fe4bf8bf37be057f8e0d049a1551dc498"))
    (85031, uint256S("9b72e6b3f44fb89e55370ad9686bb14d2d1169861d1a1090a3e08853d01336ca"))
    (88574, uint256S("3a376fb6bfd4dd36d0e3a86b1eb31f20a5518d50d98ef3aef0420f5c246fa01d"))
    (92117, uint256S("2dddc3fd6c22fb7f57811a3e9467af9b37f5490d87336f6a86c30f1f3b4decc9"))
    (95660, uint256S("fbccc95106a876b8580966d6221d140dd86d06b18f0464ec7ee49483ca16cf23"))
    (99203, uint256S("26677d5624da680ee6bad34a8ff9353d02578ea2ba53b94353f0066979dc443d"))
    (102746, uint256S("528d08fc0ed04109874a015d92b0ae7628ab965acd7422d2abcf0f76658ab37e"))
    (106289, uint256S("adb36f8bf5fc3a2af73c7940ac6841490d702138755981a5d06dbaf2bce20ea3"))
    (109832, uint256S("968efe289b3d268541299d9892c8061e465a7bd59379daefd5bdc8d4e63c30f0"))
    (113375, uint256S("7c93bddcbfa470a5666d00deb7543faf78411b2928a18bcc5a3af3f3b6dbf83a"))
    (116918, uint256S("b1329853235d2fb40571516e756adb05a343fef0f4e7beb6d0e16ee0e0a22e91"))
    (120461, uint256S("29123c6fef665bf401873ce772ec991e88fec8ac41cf1b7d914844e768f5fd21"))
    (124004, uint256S("a28e5e1a32290e7913f3387a9ff90439221fcc01caf22c7612948b6f9682afe6"))
    (127547, uint256S("c0916c729ae7f357057a53f0ad01b6e9488bf7f5b5d78c9beb86819b78815c2d"))
    (131090, uint256S("779342941342c4a5f548869dc3c503be8b362d3bb6e22763f54a078f1d69ff9f"))
    (134633, uint256S("9debe2a55bba1564463cf9ddb3d4a4077775fea2af8afdc9ce89c1d1aaa47e72"))
    (138176, uint256S("84149d08c586b35285e1881a13ec2583d88e70e1cabbeb6f8c96d8f729ee6eda"))
    (141719, uint256S("a508fd4bedea9caaef1fe1f28329b7d41c50bd565f5d2f94764f19907aab238d"))
    (145262, uint256S("bc8b59227409e8ea987cf63a037fe489a43350ac3f14d2007b4de73210e8d3f2"))
    (148805, uint256S("43b59174333821ef88d8b0f4f91049769468e15ea71f4e1cc3cf5678520317d9"))
    (152348, uint256S("5b66d110d72d4d5a22286d08ede639aef5c051bfcae461b18b611bc5d81f66d8"))
    (155891, uint256S("41646457e853cef90b4a35a3e0f091d56a3928ed97b137390bc72e9c4265dbec"))
    (159434, uint256S("b736884bacb93ef289a862389603adcdc4bb2d34704cb7fa42479d520d2d43c7"))
    (162977, uint256S("44cfb97d78a13932fb64b73430219e18a8e5b4ccadf0f42aac0edfd463eb8322"))
    (166520, uint256S("285e85d39e48818049bc1a059bc133884e07959928aea23487964f402ab4feac"))
    (170063, uint256S("d164b02efa9bdfaf12085f4f49eaffcc09dac51046fb08190666e4ec24ba976c"))
    (173606, uint256S("f643614e6e76f34b26b0cd1891ce26a95c59642535faaa090bbb3daa1287c171"))
    (177149, uint256S("355a6b70c0e1d44ffdbedc0a07e2e2dc881de64c6ba1c9f20c6111612e997d9d"))
    (180692, uint256S("3cb9afb681bd63e0d1bb7bd57301ca0e6b0cf062de095dd4e6c075f0ca27751e"))
    (184235, uint256S("3ad6206f06d9331fd49fc70e92e4f56110dd567739b7dc2f675d1b3b694adf2a"))
    (187778, uint256S("b8b9b659db9f3c96acc2487f9315f1bee4302e1453509b6fafb4f1635148f3c5"))
    (191321, uint256S("22cf731668817a4790e33eaffe61c4b17ff8f8af49d0395801f3190240555d73"))
    (194864, uint256S("2d30af062b38bc3ef0dad427eeeb77a30a81260db8eb0eab38863b38256e3af3"))
    (198407, uint256S("b55b46474b82ce19a0ba2bd47588d2b5a6e634a80e965517ab4bb82de85dfe35"))
    (201950, uint256S("101671ee4dac9e78dae2b1c18dbfa9e8d87a117fb0f2c4f72cabdcca0e6f5b6d"))
    (205493, uint256S("9e762ac2dab27c492fad3edbd2e8a1d0e42fa791cc09662e5ed437c939685ca7"))
    (209036, uint256S("0c13818b9a30f4e5323722a6e296f5c18b197583c51188eac07b1c5818a4bd81"))
    (212579, uint256S("9d45892d1b7303bd2242411922549a5211794eabf91abb2e2f81346eff2209c7"))
    (216122, uint256S("18738f172b4150836edf4ccf952a94547d278b62a75012145d69017e72725ced"))
    (219665, uint256S("ee8f5dbbe7c074afa987ae6d32dd1e6f0bfb80346fbc2d33c1d897d1975b8048"))
    (223208, uint256S("09233829b15af22f70aea19b411158f96b47ecc4d66c2524fda4e31388719a6a"))
    (226751, uint256S("19d76220bb47ca721ba178ad7b92788c58feed76304aaa9e5f503e23a5090669"))
    (230294, uint256S("8728332031b1e74ffc4211632a8e8941ed69181fec6153a2accfa121bf18be5f"))
    (233837, uint256S("d3516e4cc73634fe8f183e2c711f2fd574e22b2ef390a8c781ad7e20173eb83a"))
    (237380, uint256S("a2ba399954ef19bf9b13ce79928228e8902ad61f2810c8886f7aba87681067ac"))
    (240923, uint256S("701f4428224012575c83f4cf192e3531707baac8fe705f58228a12602c35ef92"))
    (244466, uint256S("87a880a039c87eea1fcd5dae1816cca36a82e69eeeec33dfb41e78d117ca187a"))
    (248009, uint256S("8aff725868a1bac1e8184a867bb379a313fa4806c9d5111cb7d4c3c7fc30af65"))
    (251552, uint256S("8deed9903916901543718520980fe9baf99a2e094135c442d081f6351a6c4a23"))
    (255095, uint256S("94d7051db7d046793c4ee4ec0589e1efa6f8e3138ec59fbf1d0bc21de7c0da06"))
    (258638, uint256S("5ad86eb8b35354e0e07a96c548dfbdf4e7d0731acd4043bc0fb861c41ca7439c"))
    (262181, uint256S("c2574c46640abd30ea60abf08040ff3755eeeb04d98eebc2304d7a8ce1ca0ae0"))
    (265724, uint256S("44aaad3c90a0ab2df1428e58ca1e004a3551589f9442e21df37e1bd9a5727bef"))
    (269267, uint256S("fc8b1a2cc2194910ddb3696d96b099f91009e6db97d76a57f162d57b49c3401f"))
    (272810, uint256S("0882ad96078c724b4aa32b8ac8bb143fbc79c6a31f4e1c094649ce2cc43506f9"))
    (276353, uint256S("d63df12ea735d5368ac23bf8afddbbe3e6081a048183c4106303dff959c96d54"))
    (279896, uint256S("96a953374d312fd55bb06b77eb721e2765e325dd001ea6efd34bcaef6ff7764b"))
    (283439, uint256S("1234cf869a2319dbc9dcbef1b9dae48e721afbc482d8d7ba07ac1056aa1e0bbc"))
    (286982, uint256S("08b4074ec9cb435847b449d00cb6bb536085b48682cbf46c8376d0f3e5f848fe"))
    (290525, uint256S("9b3e33a596b8f40246ba248e0f9e8dd2c05bdd0d5350152a9179e992c930f76d"))
    (294068, uint256S("01b20a70d16805fdc5b26eac89c1b700f1c056d6efb91908156987a0a61b0b8a"))
    (297611, uint256S("b57122932d4fdd3f85161a29b308398e66f85b5bf4ffe820b7c02c93aa6146f6"))
    (301154, uint256S("f35cc8856e487256fde8add021a161235cea788ee6aa900f5e53e66121a6f010"))
    (304697, uint256S("438301da0f279fdcf086a893f47dcdd577a037b6e3541b38955a18b377182464"))
    (308240, uint256S("cdd32b58d4ca98ec1c46270025ab902371c6aa6ee1c192c1036e6aed0eb9a7dc"))
    (311783, uint256S("a22f053e6caa5be5ab82d2803ad753f83117735b1616717c27e89eed7d6642ab"))
    (315326, uint256S("f0b69be84bed1e8b45bcba177d30d043a2cb765000f7e3e3567045189b8d60c3"))
    (318869, uint256S("68f05cab255492c6ad2617d4a2e81e9e285165d4cb20c56c1d641f4600723a49"))
    (322412, uint256S("a142881b41babac5fb89900a272ef80949616ebc509187cc9e919968c169ec41"))
    (325955, uint256S("c3161990553ccadabda32624fb25ff56ecba11c261756f11f473268ef4aeae30"))
    (329498, uint256S("59c0dea1c5c36f48018e031e42c43beb15d726ee1fdb575bfd8107d89f057d4c"))
    (333041, uint256S("19101ec87e1e7f37e981f52c36f6e717f2dec913ebae9eff6cb21d1b9b22d07f"))
    (336584, uint256S("354807e8aa8516e585e08a9ac3562b54200d27dc5c1ee951fc0270a03fe68201"))
    (340127, uint256S("980aea359a98cb651b0dda6eebc6b71ede369898b3fdbd03079110205b01c11a"))
    (343670, uint256S("1a31909b7d3f619099075784a3aec89505df25459343f324d85078f0e13d2252"))
    (347213, uint256S("134e8de1f1ef4bcaa769b7ff3c2715763a3f12b43f3abb4cc834bfa3ced7dc03"))
    (350756, uint256S("6b1ef78e351bfa669675d7aa84041bc9760e3c477d4bc78730fb7b28fe5615d8"))
    (354299, uint256S("0dd3326eee4e52429e489dc0ba158670143b6f487541248490243335d2e002d9"))
;

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1692802425, // * UNIX timestamp of last checkpoint block
    757839,     // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the UpdateTip debug.log lines)
    2952        // * estimated number of transactions per day after checkpoint
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
