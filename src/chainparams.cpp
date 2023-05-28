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
    (2299, uint256S("c53107bee7339d2b7f3218159f9da4e681922c63662545b28fd30d61b0e79d47"))
    (4599, uint256S("9096c5844e8981dc9525de3911bb64060ef9c40d5c988abf4ff991052367675e"))
    (6899, uint256S("6ca70e10d8562150d8cdee28263a83e312783a8ceba135f2b0887fa9934ff8c9"))
    (9199, uint256S("cbb543eb6afd4af985676b5db25f6fbf2d906d6f09d0ea33dbce46a9c98e478d"))
    (11499, uint256S("0018e42f477070aaed634c0703d1dca91be0663ac61dd1dc376f062572644f6b"))
    (13799, uint256S("9a024f42f5b781b5aabfbddb2b369dc05147cb619eb14a48b326310dde696447"))
    (16099, uint256S("c3577ab5a4f968f2208e741cf507d0f9bbfe3f0fd4d05dcaf087d8a700b0382e"))
    (18399, uint256S("55409cb89c0297bac683013845e62524e9f3ec3a1cb5843d5dad87b26f04f766"))
    (20699, uint256S("d12fb76e27c35100433fabb2581060540ab03448d3ed90ebac13cb54c5945689"))
    (22999, uint256S("81c1f58ae23be3ba33c665b0ac3e2b1e630a5ea48a63135eaccf97b69d37eb19"))
    (25299, uint256S("564585ab11fa069604ca90d9e4e81f96bee649261cbe782a6d1be1b4f2699409"))
    (27599, uint256S("3d687ea849e19ce92d09eecf3a390c516886de077a41acf836489831632c30d1"))
    (29899, uint256S("fb7b383668bfcc2edd7b5fad0c56d42b0782961f6531bc8f7cd1172b3138533f"))
    (32199, uint256S("3b698c90a1f011bc95aedc2b03589a76add529f117e988218f77cb49700b0b99"))
    (34499, uint256S("c67afcd869783156f77dc74c18cb1597fe2eea093e0a952c5b56c543236c44c9"))
    (36799, uint256S("f6356df15c7ef2094b33610e1f7c639c309f8095bbaca7431590055ecf050a3d"))
    (39099, uint256S("a64a17c65c032ce8ff1ee8a3147168635773a85e42b8f9e9b0b89e518be48b3a"))
    (41399, uint256S("c0bcf8dea4109a0f2bb47d591cf58ec428366d0d07a570084d148f5d9e1ca9a8"))
    (43699, uint256S("da4bfb24bf2b24567f4a3c65afe1b05867e72252dfa405239dc5acfc3df28305"))
    (45999, uint256S("b19f914dee81eb6680705aa40ace3351852be3b89393e0d1e73d05f55e94e8cc"))
    (48299, uint256S("e2076b0394a5c83e51f89b1d7f1e9083bf91b073ce28a514728d70930225c16c"))
    (50599, uint256S("839fc08350846365f53be951436302c99e3d5779b6495cc2b15d55124c8354c2"))
    (52899, uint256S("7633a2c669456368e9d1c7115d770718f2e877a02fad7301bd3d76a75363dbb0"))
    (55199, uint256S("19c7ae3d171f3fe26eb871db387383354ad7dae7ebaf35dc45f5430f603b2d87"))
    (57499, uint256S("cd0a47a247a236a43f7b7e94cb8005b8a342ca0ef527a36e7262d1876d0913e2"))
    (59799, uint256S("245b6f4d2c3a0e6869db226d0223bd7c2bf7773414cbf36ce68fb4f24dfa5add"))
    (62099, uint256S("e575711e05c007cb8b748953f327b663c0cb8157cfd294c71f70e4ad90739e60"))
    (64399, uint256S("0aa6b0af0735ce775bbbcaecce68c63ba362384abbdbceeb53f1d3ddac87345b"))
    (66699, uint256S("eb2a88475369da9af144191512efe1d899b67d6279712d785d246176d7608b05"))
    (68999, uint256S("5dad6f45ad57dee95dc1367f296a5528a1df09d1794169c8e8fd22cd2a28269c"))
    (71299, uint256S("1a64ed971b4f805f10c8b7008d1f5dece0dc0309ac430212d791526331bdd8ba"))
    (73599, uint256S("c83adea19a3903eff191edd4121b7a83d568edec50568082195c11f124ae0cfd"))
    (75899, uint256S("af783e32302ed01367602f3b18c230169fc5a2ebc57ebe80cafc35f6119c0943"))
    (78199, uint256S("931705d7bfc14f8690dcfa45e3ad9a98850fedd8cd5ddf0c3370abf2a538ab91"))
    (80499, uint256S("779021ba1693f101dd737d748b4e8dab822ddb72e738396383f9e4a8527fe1ef"))
    (82799, uint256S("61651198e035f84ce4f7b7204675d06de091c4613910992518401a66a89367f8"))
    (85099, uint256S("6f2152e1f07d54b02a5823e67b4308c17dd9a7ab7f0e46e3844d4922766eb24e"))
    (87399, uint256S("a4b6546ef4eeb25a38d7600ec704bdf5cb1285b874315a4a8bffb515524a46b5"))
    (89699, uint256S("97438d8ba63e64960708e0bbb34b0fab5cb201d15e6d8e68168378a60434296b"))
    (91999, uint256S("2d2e42649fab31245869a91e24751b0022220e6e92bed5fcd399581ada49152d"))
    (94299, uint256S("e849315ee97840911dd40c906b7efa0b74f23e4f70e858bf36d31c858e6a2fee"))
    (96599, uint256S("57c14055d804058dcaa7af07ff4d7e3411a9978367be411f04cc8463fde0332f"))
    (98899, uint256S("498110af9520e6b75df67fa1cd615a671d53719425745830564623e207f2c20d"))
    (101199, uint256S("1e9acb68f4789dbb1578adddd6faf516d24bfeebe133c36bcba3003937fe6778"))
    (103499, uint256S("d4f74a21a54bf072c3a7df2a092a983e7dc71c6b6045964b2a550d9b5cddb4f5"))
    (105799, uint256S("0286a588c3e02d8972ba05f86111c990ac34f1604db0185276910a3a9db22416"))
    (108099, uint256S("b3a93e77d168fb4f625d01331494715b9505f6864cf8f7e363e61bf39bb69ddd"))
    (110399, uint256S("0e1da5e47953ffe8c62f04cb63669d03c6f159d8636925b60de8b3f21b79386f"))
    (112699, uint256S("9e288a75c10ccff6ca44d75e7e6b00d7cae64d2f1287941fd16b3f56f6882882"))
    (114999, uint256S("9dd9a7eabcc11906bbc281d1b5e88678b56c3d2548e2c43ec8585fdcf9d5f0b4"))
    (117299, uint256S("c60c17d5d085434941030dcee283cf7f32be283f2d43a7b7785ed8718d4ef2e1"))
    (119599, uint256S("1e7c054fcab4c890f60cf7e7a8c18a958cc6028609378494cb2a19398853816b"))
    (121899, uint256S("fbdb3cd2f908aa388444ea5604a25398aabaf6e2ec53f805945411f021c893cf"))
    (124199, uint256S("b81717eee3d102620c4418d902a93870ffb3ea9b583156976dfc845b861919a7"))
    (126499, uint256S("e1d49e699d23b24675ffd49603d0825fba3fb71e791fa2b0ea76aef1514505bc"))
    (128799, uint256S("fa137c6c395e4dda6adfc6c189df9b939bef1689f03b963116f02a485f9dfd17"))
    (131099, uint256S("1fda48e702998b7f2288379b1b706ad4bb914c5fa873f249adb89e3b420639fb"))
    (133399, uint256S("dc4ec1a89c88b8975529815d9d5322bf82f614ad157e5fefac897aa205ace0c5"))
    (135699, uint256S("0cb82167e7f31ad70834e1f4bac4611d2c4dddc2668586df93a78a87dd1bb1c3"))
    (137999, uint256S("58add851039b7d7d8dc98bce67fee052168aed19c67ee816334a52f2cb0e6d27"))
    (140299, uint256S("eeb9233f119c3765dbf7735a0eba77a9dda10786f0f5d3a919c7ddee907a5d07"))
    (142599, uint256S("09cf48c80226754aa3cfd8fa285f33b06c462b822e406a077b99574007c6ca29"))
    (144899, uint256S("9c813f2f82554409b710ac93162dae3813b25f45067d4dfc7952dc1751d8e180"))
    (147199, uint256S("36bcb781989aedbeba2b2b2b3c837c56b27b3154f967d35a50a4cbd8c8658277"))
    (149499, uint256S("dc899c54cda481b3099c5b5c52f4f519fa8b911023151681153752a90d63a35b"))
    (151799, uint256S("a30bcc22e2f2709f8c89a150290bd037a8b8483fa2e3d5a3f5dece78f952553b"))
    (154099, uint256S("c29d2efa41b2d8213912bc8c18a2e6bf279a472443ea089a56b1a23b08843881"))
    (156399, uint256S("c48b18daf6819a033ebc2f665449fbf37b7416cfbfada849d7f515db4ee99fdf"))
    (158699, uint256S("c4f8b2d354b4b9e5674ce0bc9e47f18638ac1d2b409bc32da50ac6572f6aa975"))
    (160999, uint256S("16266f762c62e299b393259dd09417c651a324d8ffec292a450149689eb24926"))
    (163299, uint256S("6e9c7a04fb886a214129eef2387068856c77cbf9ad59886faeb6b2eaa4656857"))
    (165599, uint256S("e743f27d64370f37aaf9b338469a7e8f7371d47f12682d3f3590060af951a9fb"))
    (167899, uint256S("beca68f494f0f20b07bb3c30a6bd07e1ac5fcfc5f54174bad57fa8c5ad45509e"))
    (170199, uint256S("ec92318f8420a1a2505d35a5bb9b35af8236ced432159e44bf8636b6ce119378"))
    (172499, uint256S("96e7ff88fda60a6236d62aa80db0708225422fd083da3b679354ea8afcf0d40d"))
    (174799, uint256S("836a17edf4abcf61978b507e43c514f0db8f89aff614cd1fadfafad3e18004a2"))
    (177099, uint256S("aa6f94730cced4c9a4881fd6212b184d9322d215f48324f860cc335db0fc9110"))
    (179399, uint256S("2208df5048ad4f4214de2873b443aa7296a117b49f0ae78d9806f118c0e6dc3f"))
    (181699, uint256S("b6dc2e95e5e4652c2cfc7c4d21cd9bd88904085ba2a3aef98d54c6c0b2226808"))
    (183999, uint256S("4069a08e97584d7bc7a7f6879ab9a6f593d169336550320d0d2d69c4ccbe5b0c"))
    (186299, uint256S("5eb2b738fe24400d032928b1c14ffa881e0290dc8ac843daabe699784c7512ef"))
    (188599, uint256S("c426cd656d9a5ebe02d4417ff093d2b2404e3838558ec977b8497e93e2f99b91"))
    (190899, uint256S("452dbde1f573fd56a8fc601c13d172acfb568d002d980eb86f44234472928bb0"))
    (193199, uint256S("3cae616679e5bd59e87dd71f3ff9ce8d63541acab5bb17d1b5d1a3c740dce861"))
    (195499, uint256S("db712901996949fa33271a29c28ae61b38725493d8aea1754fac9a7e218b8e49"))
    (197799, uint256S("e599fd7227a2e6e7380fedfa30907ca1a8c892f63ec2a2c18338b1dc396a2b7a"))
    (200099, uint256S("98d2f64f59f6e10491a4380dcd4b97605a59ea1a474d0431096925f75d919e37"))
    (202399, uint256S("f030810ced8797281bbfa9527d5e251be06155dc465c4866ffa907a884413346"))
    (204699, uint256S("686a0f5addb02a888536e4a998961c902dc996623f5ec21e203d6681979c9eab"))
    (206999, uint256S("f9262048fa5b243071a2d983a83c0cac01e96af2620f35c641dfdcba2869c55b"))
    (209299, uint256S("f0654f2f0ebdd81b2a37278557c3c7829ae48716aa5e0fe35a5c6d1d91904952"))
    (211599, uint256S("ed1f1d1cd74a58501a763d2441e9718827adda2af3729272dc81338fb2154159"))
    (213899, uint256S("b3d58cf30716a1829f72cbd714326576c02ded8b02389efd98c70a2157d77eba"))
    (216199, uint256S("95ee937178da163b7503b6a277d23cbd68543fdd173a72a0b80dbdde77a2c0fb"))
    (218499, uint256S("9d29c06f1c15337d9ce93907c2b890568817291c94d250c357a537e46ed378b9"))
    (220799, uint256S("245948f350747ac39e0023f65c778553da343d6993aefc0e3b48bc829e723d06"))
    (223099, uint256S("70ba896e03c7d10699e8bb7264dd90ca944bc6d04976c18fb66923d2a542a1e8"))
    (225399, uint256S("e9297d3ef7521ebce51aaeb04bb46d176989a35fbe1911fdde708881d7785217"))
    (227699, uint256S("0674a4692d22a2b61bf7b8a47705c32bd9d62c2c67d1efbd40125747af9eaa14"))
    (229999, uint256S("9be8b927adf9ccfabfc4bb398a89b25df5073eace45e6af099c5607ac86d7baa"))
;

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1685298795, // * UNIX timestamp of last checkpoint block
    498775,     // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the UpdateTip debug.log lines)
    2874        // * estimated number of transactions per day after checkpoint
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
