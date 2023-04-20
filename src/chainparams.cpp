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
    (1743, uint256S("7b974e841ea83b378aecf598fc4efa52ab2d846ce6598ee7865dd3042aea2813"))
    (3487, uint256S("b0859c4540568b5c23c5c5d526b2ade1850ab2d486504b76d3e27976a065833f"))
    (5231, uint256S("4b936340c31200459586376f1b19a119981e795ec86579eaf9516cdf8517e8c8"))
    (6975, uint256S("c65e9452254bec601b4910d5754d76dd2c8d09f4730f2f6ee1afeb0e22139a9d"))
    (8719, uint256S("40676c0aab4180a399a35ab1218f723bdd6e3cd8c6ceb459ff1844128bb9b405"))
    (10463, uint256S("be9de6e8ae6d7fc5443e1f80f8eb1c5a60f98020d161d32ce7c8763aac607949"))
    (12207, uint256S("c7bf4fe6f3e55889cf71241585f82d242849c11a5c8b011b9da23ab093edf106"))
    (13951, uint256S("8de151526b7d88e6225d837cee30a28c579ad0284e2e38fc04793f9e985a5b3c"))
    (15695, uint256S("f4cbee7fe5fe75dc735662e311113231ec03128339b9ff387f8b437157f7fa7d"))
    (17439, uint256S("80db629975e18c45e7d059d6e669a890e1054ab8311460f22efad55fbe652291"))
    (19183, uint256S("866004e56e7d7cd9865b08110171b8cebb7c2fcef69edef6257d5332d3f50f82"))
    (20927, uint256S("fd77276e17643853df255e7e0ce9762f53e988857e88aaebd264fd67b072fa70"))
    (22671, uint256S("8043fae798af658e735a8b4edbdaf01e0c2b8442dbe55d1462af266b1ea6f661"))
    (24415, uint256S("ccef2003ae837a25665fee82186867277ce3a07534a010c8b865d66eada708da"))
    (26159, uint256S("6a9dabc9f43b29689246e84bf47587488d5d08740714dc5f6f402753eb729a21"))
    (27903, uint256S("2bad1c8d25ed4e2a687fd8ba817a0b0a09b661d586400d8610aa5f2d6b2f91b3"))
    (29647, uint256S("cc0c4564a12ff893ad88397fe233d5af1039e843649d3063ad5bd17eb08a10ca"))
    (31391, uint256S("1ab4889b31147ee94d46c45f129006f796498058f8d7b8c684704baf1656d7c5"))
    (33135, uint256S("efb37a3e2b258dab0a1bb4c695b0c10d23924c2e8669feff9c9dae0ea79207db"))
    (34879, uint256S("ee9b3f9523171ab339505167fa21ef40f60a703f360119229869ca526d4a16f7"))
    (36623, uint256S("a4e81a66d985af88e20ad4269a1316bea2027dd73e5a46d1ae41793796a0610c"))
    (38367, uint256S("98c9baa2079ce7ccb0789f8e598a6d1f77fdd9b2f32cc3e5f0c135af9eda996e"))
    (40111, uint256S("2c975ace6b015d51be0d08c57d0f5357e6842e02bac1584325e3f0daaa4e50c7"))
    (41855, uint256S("4a21a020cfbbc6d3e407b013037666c86e3f832b7cf60cb9e3eeec57f10dcd34"))
    (43599, uint256S("99f3bb01e71147de39efff6961deb1f36755df04520344cbc47cbd8077b7a703"))
    (45343, uint256S("d66cbc22026aa71eddef6d62a13e4e3c02252971d8466ceaf2345ac4d1370f24"))
    (47087, uint256S("965721a45af8a903e4b7233c8ba295f21bb42c72a437910bc8a27d632e91c914"))
    (48831, uint256S("01b561b9d91b0ea3bf9374879da88e51667e29743e67c2bb4559d3c5054455af"))
    (50575, uint256S("c2042823cfe86c4b8ca7b38f89af4df7bfdb60c4ade212e4d351f09ee747e95a"))
    (52319, uint256S("e7ac4bd8724403e4c879992d423b3c8aa81b1a02e0f3b422d34b8018b5a94a73"))
    (54063, uint256S("f127bf0df28f994eeaf13233ac1da0719ac3bfdeadff069b223654d518e3ebe9"))
    (55807, uint256S("4701e043f5085b437f071f28468ba6f169901a7bbbdd87ce1727468c91995664"))
    (57551, uint256S("0d92d9f3a635a249c23b2b50fbcfb3157767094bacfc1a58fc45b0db2dfd0aeb"))
    (59295, uint256S("5516a9e663932be13c6c270427a9b3016f28398a0003b7631ae83873d21e7bd2"))
    (61039, uint256S("072f8c408c7a93db74c93abc416e267ae96d39ac936f77bb61bef920d2b2f8e5"))
    (62783, uint256S("22206a68fbf160792a298e91bfb45cf5795e5211953650ad7db803015e2acfba"))
    (64527, uint256S("b19808e1475e4db043b094a8fa0b12fb777a7952db242340d8d46eabf6aa0cf6"))
    (66271, uint256S("2713263ec06cd122e3e96d63a0be84966783e09ae798d213970f16572f15c41e"))
    (68015, uint256S("2c34b667192207a3569255f8aad7f67df6abef76e88edad5527f9e532b3f63af"))
    (69759, uint256S("1f9875f4d40f6ad00fd909f41acc590d08cefa08beb06074627f9b00c71bd593"))
    (71503, uint256S("fda2584a78a1760ef40a9984a01199e3cf8a6155b1a4484fec21f90738f78995"))
    (73247, uint256S("ba27fcf815013fd113c9f39fb73e0f3a83c772bddd940e7d602f2cdb471cc3e8"))
    (74991, uint256S("ab05b23a3c6ccf52e8f240e667c41512b4e7e8ee6fe190a1d8492867300469dd"))
    (76735, uint256S("609f881204b96216deb7f691e052a3e86b355a6ba35924a0954072d683f042ec"))
    (78479, uint256S("4839b22d82d04d02fb555f0e88430916c6bc4a92d4507dd2bf67b1cf1638d10c"))
    (80223, uint256S("dbbeecbecb3d849903a0e0359eb7e1debed307b2dd59df7d6ceded4770a628f8"))
    (81967, uint256S("1639490cdf41ee06a74eadedbf636152604dfa11109452eba0ec4163c0aeae2e"))
    (83711, uint256S("c3b116aad9c6366c77bae705d6f2f6c34db2a3d25f878a765c294c1c4bcca936"))
    (85455, uint256S("bbe2993010c70530f0612802974e786a41e248ad7542fd4382ea54129fde8c03"))
    (87199, uint256S("bfb4588b3644e0ff94ce12c2059b6ee1d63599a5680b342202f36cbde99afda8"))
    (88943, uint256S("d8a7efde4ce38da093dff412c5f14bf383895f4d6046f264ad24209612fd3d6e"))
    (90687, uint256S("ceafb9097912b6f66639d836ee61d743fcee4f324b6abd65783656bff460e8b4"))
    (92431, uint256S("176d3b294014c6061882fa0928651fb3d968956e8d587952f9af670d4f269499"))
    (94175, uint256S("ef420cd1a98db2fd7cd049d2a850d7cafde095f040293d0efed050db9b05893c"))
    (95919, uint256S("80cfe99c0c3678bc107dfa7189f5c9f50947e9c2ab7d1b49af8ff8c6b2f475fe"))
    (97663, uint256S("153dfb57486e113e154e7b2beb156b6b12c2a02fe8b39722c5c83ab2f74c1bb7"))
    (99407, uint256S("60996148acaff23017aea4d665722af9dc24af56442314a70f16b8ee8d8b498b"))
    (101151, uint256S("50b5602cde59a89a9773856e0f1ea4d390072aee882e6e0be1955415fae5b8dc"))
    (102895, uint256S("85b1de7627b3ff9950379c2054a94677fa0e786763e1019dce9abbd87ee71829"))
    (104639, uint256S("cf1373f6e8ca69b0e5de4da02dd1bc3fcd597975661f9a6273d941243f5458bb"))
    (106383, uint256S("53049dbc6d5b38313a18628d3a1e5c4c92793a315750180d6ec3c911caf90336"))
    (108127, uint256S("8a313eaf730690179c366586b88fa8ce4a5856985fee28ab86e39683e8b3abf0"))
    (109871, uint256S("4adfb8497b99eadb0bfcc79deca9f9c721e4f46a7e84892598c96afb99c8d6ae"))
    (111615, uint256S("d53888a934bc8f718f93c0a31b629fa04a5eec2e83df13c0c209594e29fa9341"))
    (113359, uint256S("06b022a8a96489e6ad18482d264f6b9dad8791beee18dbc9d859e0c466b9e782"))
    (115103, uint256S("183c4583974192e91be78de447289305f7324507d7f2d51084bd3d2de54105d0"))
    (116847, uint256S("4d2da8c5ec084f3ebcd8b0a63349ec02dfb464dc7a7bbddbf8518c0be5ffdf0a"))
    (118591, uint256S("ce4762ba034476bb900926ec829b702770ec17d9ccf7d7f384f1e9ed32e51ca4"))
    (120335, uint256S("2b170ca6ae959d09b1985d36a5cee52ecd8d1558eb4d2d07dcc12d902f8e95a3"))
    (122079, uint256S("fac4f42192b0b92cab6af30704625f71378b3d95e3a17b19c8184d1b624383fc"))
    (123823, uint256S("9f997250c699979954ccdc861a8bd25c4bec9736a20b94b1b14ef6b698dd3a74"))
    (125567, uint256S("c0b8e58824e9ab6a769436e01549d4daba46ac76ee6f63d0d8dcf692992b5db2"))
    (127311, uint256S("ed7df16ce70eb89eb6e1681361d39ea07518add5f9fb83f3efe6a3dca4878a73"))
    (129055, uint256S("aa31a04947cdba66a6033445e5acc85e9a05082d99333d2cfdd2cf8da130425d"))
    (130799, uint256S("a02dea5c13c99a5300e41344cb712849d18724057057c72ffc8bdbc91f486b20"))
    (132543, uint256S("0dd19aa812b75eb105534fe5f854ed0773558ffa048400bba56a8093a468bc45"))
    (134287, uint256S("8d4c148701411997e8d6a19c91e188447280ac09fc653e629f27984b181aeb2e"))
    (136031, uint256S("5036248d00699a55e9ace2175ae590c7ed46dc181aa899daeb28aad3b0e93787"))
    (137775, uint256S("2176f14e29050e4e72f7e5889a85f76407a60f7cc8983ee8bf4241e8aebd0d44"))
    (139519, uint256S("9e8d000904f517a77cdf8310237853e04f001410fdc3481cffc8239a342a59dd"))
    (141263, uint256S("e913b18c01a63b93c8a48090e596ffdbc90d64639ced824361943551bf8eee4a"))
    (143007, uint256S("0a226d4bb067136cc022189d2a6dd73c8dd15ed5e368a8545c60e437e933ad29"))
    (144751, uint256S("36d2ab7761b01ad8be6bfc010efbfcef618b51ac3c714f7e0fdb718058e2c886"))
    (146495, uint256S("4b4db256f187a94a8e38194ec1fcce982219017dd3dc48f89575588cbbf51966"))
    (148239, uint256S("627cdcd40d6d432777545fccb2f3da436db18e030f2536588cd372b459aaab5e"))
    (149983, uint256S("32d4f225c45f70071ad6785d73323cc3c0fd990925f198961402798959307df6"))
    (151727, uint256S("d80527822e23a2eeefc54a6302ad1af3f60012b9701ba2d822ccf3a0d9fef22e"))
    (153471, uint256S("486387c8370466b37dcd8b460f4ef5144927dc0e2fca0f85267447e1a2c40f3d"))
    (155215, uint256S("56b6f9619824b85bce43cc986de310420d39582498bd0e72c403771d58a44d14"))
    (156959, uint256S("f72a5e9d792045a79e92c006137b76f8a221ea3b76ad4bd3135123f3691fd1d5"))
    (158703, uint256S("7271547192a340e994f360d9144a02a7fb84a4874e7f1bb04f0c3055617111e6"))
    (160447, uint256S("84a955d199493437ff94d76cacb1ff77ef08396c15574f18177b1496de9dbb21"))
    (162191, uint256S("90866cc9407a6c17099d659908fbe9df99b8d018592a405c7ae5b1fff4c8c59f"))
    (163935, uint256S("189488c9c073112fb1ca09e80c83c2f10edafeb33c149af6f8241d85b4001954"))
    (165679, uint256S("46f7deef42aa5552f4f49334670bdebe20b5673a65122b242bbb2a3350cc8a13"))
    (167423, uint256S("8f8b3f24cf0af1cd97f4d1bcd3dc334799fd654189785895d8011951fed03454"))
    (169167, uint256S("8bcfff00ca20b4d19b3b3726111bad8520cb14027ff59c699e99553878127ac3"))
    (170911, uint256S("60a94b151a7e5a5ae22c2a1999f79d3733ce61d0ac04b49447c9725cc21bec4f"))
    (172655, uint256S("10983f56dd12fd3233d2f5a634a6ddf1b13965c1e5938d598f94716e9b186168"))
    (174399, uint256S("a618da156e128a927ac82d2dbbd38a35eb994336942011c2b251900488dd7e2d"))
; 

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1681943400, // * UNIX timestamp of last checkpoint block
    385353,     // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the UpdateTip debug.log lines)
    2906        // * estimated number of transactions per day after checkpoint
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
