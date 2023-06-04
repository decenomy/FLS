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
    (2388, uint256S("b5a8b6b43151748e0701528f15e172d9f0fa9ade95eb0a1388b5eb1c706de799"))
    (4777, uint256S("3097f4f18526b55b1c2dd775ca6a01e2c43aef8494663e1370f683165b91fc40"))
    (7166, uint256S("30985c7bec61e41c1aeb67d11a61a259187985c0074a8d452c09a65fbe7b4e3e"))
    (9555, uint256S("555ef27731998c016e60107ebdc168c4d775c42f3234328e4cc1f6e4834ba7ca"))
    (11944, uint256S("c76d897176ce25a2e24fbfbd104ea42c5b6fa3ad699935f06edbaa3816f6731c"))
    (14333, uint256S("bdf5d5f48d9a543aa45c7c253f4e9f20ab3d73978ee10d4732715d7bebe5a6ae"))
    (16722, uint256S("c8e5980a861e4eb7f6bedfd2253999dcd0660829a502f2cdcf7193a87873534a"))
    (19111, uint256S("decabafc195b00b21b68886da857a689b0c8a3ede7dc04c39d1530a3b1815581"))
    (21500, uint256S("28253621064186eadf9dae39e2a344f928406d54438921a0be0ade60336b53ba"))
    (23889, uint256S("e9440fcc555afd2d8caf83caffd57c9d81ef8aa0ac9fd69fce27fc4b4f791d09"))
    (26278, uint256S("1a8d066a6001767c0f975dbcfbcd18eea20c9eb727674786490740403029cf4a"))
    (28667, uint256S("c01cae701961a68e49ab98670ecde5c5ff2f2afa49487c19439758c9ec8e324a"))
    (31056, uint256S("1efc55486aff0ff2bb556bc83707d0e7d5cb08232276f8410b226e7fcdc9bc7d"))
    (33445, uint256S("81888e5fe11feb3d9699d25a0b021e38ac10b37cf20e1ff8b6c37f7cfea721e5"))
    (35834, uint256S("3d1723294a48abd21cad3325806f080606499a151ec75d5681fa592d34be9522"))
    (38223, uint256S("cb43fb30f24160d9990ce28c5d8e48026e07734df245fa5b2e44c561fea26a37"))
    (40612, uint256S("54f7135421e1b0f11fb07cc23558c537ba9e19564bbcbcd7907ccff80a425899"))
    (43001, uint256S("28acff8c8dc839cc25d86f4ed46eaaa7edd489cf45c8bd00057ce757834a6a6b"))
    (45390, uint256S("607cfbac0b771638edd49f051fcd3459f89934a9327e22f706e5e98bc7947011"))
    (47779, uint256S("00f4b16de249c3bdc9e3a807b8ea7273996526ba2017bc622f831687f4c312eb"))
    (50168, uint256S("d58479530c9276da8f272881ed4e7c09e86c887ef8635cbbac6c9654098a9391"))
    (52557, uint256S("0034b5d068f8abab9c67a0341b4fdc37dec00298f1b3739e24821d21e57c6b9b"))
    (54946, uint256S("11068957d39d268e90eb0a48ab7c6561b2afedbae0441f235d6872bd258fb2f3"))
    (57335, uint256S("938ddb99b2442d971e01dff9af387c7694d1e3fb377f406fa66be1bdd9b67a41"))
    (59724, uint256S("32a691e005067e52d7692a34c4171f9b9106211af64d8df8a5afe6a1d606a3ac"))
    (62113, uint256S("d347b7aca327d2a52d95adc238bb6b156989623c6097fcdc33f4554f880632e3"))
    (64502, uint256S("c158bda8cb4535115e96c8000bb6db68b1748f7e0798c06f7539eeabf1a85aa9"))
    (66891, uint256S("34cca949063b599d1efd7e861f23eaac3d2144d1012905828bf6a233f86ea917"))
    (69280, uint256S("f569123b47e78e094fc818583f36f5777f93ba41fbcf779ef50c096d08070667"))
    (71669, uint256S("977e512a14dd1d88fc582091bfbaebc809e4c1c74c369b4fc62130ce986cf1c2"))
    (74058, uint256S("0243b8a8518b6252c9bbb548b6cdbc329211b21b88065cbf8d77f22ed38cf40d"))
    (76447, uint256S("ff30f97fb292b10dc5d9e53c25ddd0b50d0c07db34f9cbb6f9fe7c8dfc2d586c"))
    (78836, uint256S("829ce63cbef8a7981c384cb1c329d967dafe69ac4c4ed2d34235c39aacce1220"))
    (81225, uint256S("d1cc90cac297dbbeac05c29231e11bb1b3c369f0763829df27951df5c55917ed"))
    (83614, uint256S("88156f9c3fb9e045d249ed5b0b3757d2c23c9e88539445fa86c74e4948487fe6"))
    (86003, uint256S("9512aa2956c25af894df7cfd8c807a81426356e3ffe576b7a67833174c65d424"))
    (88392, uint256S("c4ca100fa5283230fa176735288c838cf830a2b3429917ea6aeea22fcc95180c"))
    (90781, uint256S("3fc1d9318637cfaf106c0a98d412f1d81c4bb54025882b54d2ba30959375bc08"))
    (93170, uint256S("2da2110c54921d0a5708678a3aee546ebc6298bff127c31489f38baa38e79e82"))
    (95559, uint256S("ef5a473e7cfd757ba66d3decf91388b1db14719d54cbf51aa72d423b2c5dfa46"))
    (97948, uint256S("55c212322a2512193429902523e25f2c1efa79de9ba6e4b7d74fc04d7e02db0f"))
    (100337, uint256S("7eb8885d9eea8ff6b1732569b98631801f09830aae3d487b39b626d11356ca7b"))
    (102726, uint256S("f547bcdb2ff5b654eaa21368d1ac1a4c8bb7e7c7ab8c8ffdaaceaa0dcc3f0d9f"))
    (105115, uint256S("721e82aafeb4ecbcb01467a4cde1a098a0c3cbdcf246266291b6ccdb43c65c04"))
    (107504, uint256S("3b06a8ab3dbbc6948ed1e0c850715a7ecc45c3c5310468bf01e92e6c29c0c386"))
    (109893, uint256S("865eea3a104b05799c4548b462ada89e34ce6c51047d233c5766084f6896881f"))
    (112282, uint256S("85206523977a6b81c15444ff03248a9a82a7ec89e4ba8d9dd522194ba4f2549f"))
    (114671, uint256S("8c10500ac9a2efe4e8c0a8d91d6b0c41d488238eb28df196c2d750bff4203c97"))
    (117060, uint256S("aec9699052578d8ce7ccf44ff25c676dbbcf17b9dd66ca2dae1e22edc62d89c9"))
    (119449, uint256S("783b0ab882a0132be58b284a0e1a0302ae266944b71fe9425baf2ecafa535afe"))
    (121838, uint256S("56c93b65abeef9c6cb6c4d6dd2abf10739a1502874e1693418532af48c2cd1c0"))
    (124227, uint256S("fabdde01508ae022ad4d1ea3e53abb739602b717c933641c3d0e600401d500d0"))
    (126616, uint256S("10d9232a06fc2b5c40311d2eb6d9f4c868587976e9c726e2a8d7d18ae46a9cb6"))
    (129005, uint256S("0abade3bbef880ef297723409bcf2cbfc00ff726aa52ce851d61da17b20d2401"))
    (131394, uint256S("119442ec29ecbea2768853780ef15f0ff40997e6c08cac0c9bdb415d074e1f34"))
    (133783, uint256S("633e0d227a5fc0853a1b2232b69140b9fb1aa467271dc05a399460f4aeabb94b"))
    (136172, uint256S("24424ad5f9d86ae65df1af9e1b2718736ed9fbfb57b87afeb11b83b323a975cc"))
    (138561, uint256S("4becf1da2c5296f9b92e2a23c0680dde7af25d19df9c5b5470f7cb11d5c711a7"))
    (140950, uint256S("55b77b015eff5a9eda569a98cbf03addd8e70934f78208b3f626d159ba0892e7"))
    (143339, uint256S("eb933b22862a7ffb96318198b8e17ad9a468f9a9fd4f6ae709e109f4c0f69d86"))
    (145728, uint256S("61f337c227446b4af882985993f5e0bbf54ec01674c604144165116b03efb587"))
    (148117, uint256S("0e05164eb6b91b1361d264e530ad44739d21f16b97450c0e56fce0c0f6db97da"))
    (150506, uint256S("b429077ba1b961b9121c28481f04225222078ef6bf889affc78a6ac98b1d501f"))
    (152895, uint256S("5ea436189fe2380d39b43a768cd4ad9d779e5e7a7b4be751388957056ee65149"))
    (155284, uint256S("0769c35b46c16a62f78e995a9a475280b7ff41226ed6786be653c32258cfd1a7"))
    (157673, uint256S("c6f1a98c2f83b524998ccef895dd71c8a916d7fd37ef153a904d122a8ed3aeed"))
    (160062, uint256S("9816c9cfa47ae357cb908437a405e79a77c50871cc72ca6de0b9c00b7b5d1a7f"))
    (162451, uint256S("f45303a5dd937cce938b2d88952af537c17d5e110b26bc330b2fcd4e9308f4ad"))
    (164840, uint256S("b6e2e18c6080a5ff280c8be35e835cb72131ae4058e6a0b395cfb59b0591754f"))
    (167229, uint256S("7d1845347978b2111139f5d1683e705f6966b9569ea43183e9a737d0c8d4dade"))
    (169618, uint256S("a34430ae7a775659872c7da75975608e05549a0f976b725f3efa5a4d21d15d65"))
    (172007, uint256S("a45f9c1b4186393b6463085941cc447da16becd2475d6c4d6d02628eb080b776"))
    (174396, uint256S("6f55ae3a3b07165b7d36a09e9fca688773cd451003bde51a773430b40759e2b0"))
    (176785, uint256S("9f23eb59150e0294d66e605dda6c8c65c0cbf6064d92912af1fb58bac82e3ae6"))
    (179174, uint256S("c8a50689fd1ee50f3aae36eb20900f2cdf412289a31fcd76db80661aee6ecca8"))
    (181563, uint256S("dbdefccbe985a85749540d4a36f429af4c6e8bc3b94b42748186ae7623bce38e"))
    (183952, uint256S("b05b165daaccaa247739c83a259a995f36a65c4a53db931837b99908fc04307f"))
    (186341, uint256S("c63682bbb16d851c5dc218ca3b90211cd967e59d744b8c37a3addd5f733e3d23"))
    (188730, uint256S("21d66fc04182198accc253b7ade93756c7adf23b9fd692bc5919115963355156"))
    (191119, uint256S("ab66deb8885ef01ceea72ed4f8acc4a623472aaae776be96867f4e8bd4dfdc84"))
    (193508, uint256S("fd456fac30b1215cbe7d450e51515876f0ac6b44049a3b96ad75b3b68dfd6b9c"))
    (195897, uint256S("d7c792f15d3a3b6865e03e0bf6645265091a65a498ec3d093c1701d18368ac8c"))
    (198286, uint256S("f86b80fdfbaee6018bfcb963dbb6a0bc849dd57ed0a3044b26bfe4c162033500"))
    (200675, uint256S("96ff7e7d6f7e98c7ff309e318d7967ae29ff272737f413cf3c1f407f45d3b2b2"))
    (203064, uint256S("e3bc150682df3a32b52c669f2dc05dbaf83f39f1dc5c6833a708abee720c0316"))
    (205453, uint256S("ecb3c4ad50cdf54aa9e8a400dc9f41c8ec0c1069d3f952612d7529e16f1fcd87"))
    (207842, uint256S("711b595956e17805bba84c7b8d492acad18c6da016aae5e2e7194894a7cfbb21"))
    (210231, uint256S("3be2e64edb49af2c146e6a93ebd75c455cf09b746e8c72bfc0a991826eb8d49b"))
    (212620, uint256S("7b5c2342fb743265af768c26be4bd6492e5b74f4619a43770aa4711780d16e04"))
    (215009, uint256S("532a8c9da130b488343dd4d4b13097ccfc67ad70cd9afb50e81bfe86917a61b7"))
    (217398, uint256S("9b2dfa00bd352f52d9e992ddbd7cac4de156edaa593d1ef1677193daf0f2cc8b"))
    (219787, uint256S("101f30e767bbe11cd87a4efe64dcad89a9e2ee21afe52916c82180fccdf73b6f"))
    (222176, uint256S("be5c7a9a02a72954878e58c315afa99495f8402db54fb9da7cb1e45fd8a934dd"))
    (224565, uint256S("22446d2d248bd2b551b181e56bdf11435a111f4fdba47d34f67e4c911a09f1cd"))
    (226954, uint256S("64fcbdba728e0e755bc24e6afe3b4000b2ffa19a8ebe5ca3db86282cae8e4017"))
    (229343, uint256S("3e71a0a7a6d8f394bc6bff1ee322e83396c47195cddb29f61b5ce3d1073401a3"))
    (231732, uint256S("e21a1f73e61bba3643acbb31650209282a96fcc15094a5c21a430493cd1a627c"))
    (234121, uint256S("90f92a88097173314d93df679e4d0bd9e68883dbe4710c4b62de31879e240c1e"))
    (236510, uint256S("e8b3b25f89cb8bf0cf57a2af1bf123f13173013a27c36daf9070d30aa3c153c5"))
    (238899, uint256S("6a2fda045d360436289a2aca2084e5112a51983cf958a5beddb2ffa5828b1341"))
;

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1685836785, // * UNIX timestamp of last checkpoint block
    516943,     // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the UpdateTip debug.log lines)
    2986        // * estimated number of transactions per day after checkpoint
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
