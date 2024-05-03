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
    (6079, uint256S("d6199a7a2dbd48051fcda1a76f35831f5f7291dfa222c3ff99d4fc4a5252299f"))
    (12159, uint256S("1e13ff71feaefa093c5ddfa90460f64e49291a2bfceb438cc4d3e9ef0aba9db6"))
    (18239, uint256S("ed01f2eb09c4cb702d2b378a65b3aa6f251e2c2bfd4f105474206b040e2de18c"))
    (24319, uint256S("696402d5b0f988963059fb4a8f32c40680d31504ba19d87330b652533c51b862"))
    (30399, uint256S("a625e7a70ee70df389fab6b81767c4b7e50f8f67533accd80947196b0614be11"))
    (36479, uint256S("879ad6edbb1164f30118b68fbf6bbcaf3a9bd968db12d7fab623e22f5b0f0a0d"))
    (42559, uint256S("3a1ba2399a77598c5df13a96bdbb14294f3d7c4c13eca14cfb5489f2ca757ebe"))
    (48639, uint256S("dca6c66ad5d207db291c5e48e51ba9aea34fd65f2bdf0ae75787f8c9ad0f1875"))
    (54719, uint256S("d682d2b87cdb4c750566e17005c83678442462a332acb08405115dcfee3885d5"))
    (60799, uint256S("91199d0c50cfc58d39ef82680916f3758c1cf211fad47553675c5e7d133628df"))
    (66879, uint256S("ac47997f290382ee6368cc252bd250567ed6c31cda723763d41f0b5d9b3b2158"))
    (72959, uint256S("eacdbc13a6e94fe00a3101ad9384ae19e23b729bd59fd4a30e45f540d6ee5139"))
    (79039, uint256S("d6cf983084ec964ef4699981679333cc6c79e5930a5e2f3ec86dec8f2ceea038"))
    (85119, uint256S("91086b0d15c6c7efbc639aa143d68c4d67eb687673675f44dca86c61e4afc422"))
    (91199, uint256S("094dd0da7b02399c87b4b6f70287d3e5c67b57af37962bf5ad73456d01236f26"))
    (97279, uint256S("8b8cb96c469731f7b37180f6a7710323326614ef6e91ac44a0870c0f3b2fb365"))
    (103359, uint256S("2a6c6280da1608373c48cbde00ee3b5844deaedf41bb9657a8b324582abefc8a"))
    (109439, uint256S("6710bfb603243e16683909c40bbcd989d6e7263d73e87d2816ec5d18cfcbdd1d"))
    (115519, uint256S("1ac9bcb09dc3868c0d871d77737b15eca32f5cbb642fdd48870015d514c61db4"))
    (121599, uint256S("ff6760c552629f3f14f183da011cf4fa3987132c6cba3fec376a33c60148fd55"))
    (127679, uint256S("bf22b9cb2cd982fc2802de84635876f1f5c84bbc197e6ae88180cdc6b417ac9c"))
    (133759, uint256S("d3e4b967cbd6338b934333a8f13a09a94ba4344adb2ca688b119c30668185570"))
    (139839, uint256S("07e03de6564a70bc75d3dc9b1a8cb331f4150880c412ca1d82a8a33219e56fd1"))
    (145919, uint256S("444125106b03c04950be87d8711b3006359bb4587268063ffd184c1ac2be0eb5"))
    (151999, uint256S("cdbe0408690cae66c0ef2cba8f6d86f20b2eca8d0c6d93fa38d455ef7b6cf273"))
    (158079, uint256S("fb1412359f20af39d7ed5909f7fba6d04b457a99e457de7c9426c61a2428a097"))
    (164159, uint256S("88d76839855a364899698e965229424b21a943a95f0df9074a68a18d1a763463"))
    (170239, uint256S("522a679c32ab1ce888a4fe8583133a15ac181fac7191eba0940c9cac99127570"))
    (176319, uint256S("10fb44573f909e207a1d6e8d5265a56f3aba9f7fb32c89183933d2b70dd6a41d"))
    (182399, uint256S("570e37eef5618ded77155e703828f0be064ede0b0676ddee510f8af77e568faf"))
    (188479, uint256S("3af9e66acf44754b26d18b64bdf8607a752e7d61d70b557fc9e2e4b9fa95e92a"))
    (194559, uint256S("836141b3eb5f630eea5565972d7c9804f3a0e8e310d55a54e1f894084e42b491"))
    (200639, uint256S("5d2eca3236bfbf1e31c2e3f4fb9b29ef22a4500dd512e63914f8d725f2a838a0"))
    (206719, uint256S("4a5b38d7962e228294ed00c777f92511bc7e3e57f2e79d6b354dd28739c3fe0a"))
    (212799, uint256S("43ffe51fe2fa38b38378d599736670d1f1c4ee2e31ac1a96e0e68b5f1a3a91e2"))
    (218879, uint256S("4e9eb510e276c81f2977383aeabc741009b9cd7559386473f71b3c0e656a3524"))
    (224959, uint256S("ca8d7b14df2344f2218cdde3a6d91329af3515bf9c3a866914487a3a1b392bcc"))
    (231039, uint256S("3f341a5136bee04b990f6da1f3667c4c1ed3498ffa3477f32a5fb0f0de152b9a"))
    (237119, uint256S("5de1a6fcb7310310f4547569514d20b455cb47fcbfaada5c545d55556524c31e"))
    (243199, uint256S("eb2f0c2cba2d490a28d52fc0ab60838a6429de7b499b128fd7ef3f2ea38e3b25"))
    (249279, uint256S("3cc4056ea2b40f4dd91d4eb56d1cac09d29987b54e0ec57d8cc9de54ef62b6c4"))
    (255359, uint256S("56d5a67c980c947945cb050865dd2b9217a351c35e78d610abc6b37103d3d48c"))
    (261439, uint256S("b86fc9e5dd6c66cd5a4e5aa05f69987f333c9b112dd5375b01978f0d43b82816"))
    (267519, uint256S("8e03132483a617e6863805a72515845446bd20d1cddb33a507f304591ac4e20f"))
    (273599, uint256S("a60ce6a6ede71b767d066b5c01f57f6a376d89f78ad5a548f6c0b716cd6d43b8"))
    (279679, uint256S("8626e001810f3504967fe8547ce362f855cd8681d6adb151bdc02afff8c29394"))
    (285759, uint256S("9dbc88f21976d1a66cea108cbfe4035007e3662932821a242846023155bbf791"))
    (291839, uint256S("ee80c88bc58eeb04245989f36919589ecce26de25a56cbedb10fe95bc6e8767b"))
    (297919, uint256S("fad581fa8f2f065f8664ef8104fc68ec864c72fccd94daa03c62a8b7aea90f06"))
    (303999, uint256S("27ae731ed649e555f8870669e199142b470ed38eb1454625687ea9a5711cef58"))
    (310079, uint256S("78e57c5fce15b2d9375ea78668abf106a338e6beab7938244b784d40badd8e02"))
    (316159, uint256S("77ffcaae371d7c7c7dfc3cb30204a6dc5edd4b4071a8f6a527b94cd519c85903"))
    (322239, uint256S("e20bf58fed80dec2b3b2ccd1aa8a23ef34d3cadd7f62f8293bc6521d55985b03"))
    (328319, uint256S("b6712acfeb8e9e5a93662a6c34d95cddfd5c86b0d6de2d34ef359a32ea9ff290"))
    (334399, uint256S("781b7098ace3a35525285c8c3e08176eaa5de56ba65bc00404155e32ff734b54"))
    (340479, uint256S("bfd3126e5d79fdb88cf95f928908d358b104e62738fc4f8c60b20dd3c79478e3"))
    (346559, uint256S("e4c1be30d1f0ac248eebaacebd4761b65686271d8654c6ba2bfaeca174edc1a9"))
    (352639, uint256S("3f89c085a6cafe72eba14a5392e5346f2101773f9d0b7528625b4e3ef893f9aa"))
    (358719, uint256S("2c13df37da6048a6dea045e6b3ce1c00b45ea3ac2a11b74a0a84d8d1a34686db"))
    (364799, uint256S("0d3cb1abfd1c721c647762db253e6da6b7178b16d0035bc22751ae7035048a27"))
    (370879, uint256S("fa2af7165e86cf0cda4c9627d851d55f0a7a6ea280a80279b588a3a462ce53cf"))
    (376959, uint256S("f41a2c4b5a3971dd76ed0db553d05ee4104c1b10380813ba9dfa45069ae2432b"))
    (383039, uint256S("0d6bf9fab5a8dcf6109484f1268ba77a2d38faeca112594f1f0741dc15125272"))
    (389119, uint256S("9cabd02c35b11a8559ac0678c3b431781463591cb6696bc7462af52d9f84e8c1"))
    (395199, uint256S("43332ba238ef80dff320dcd9586ac12276e9d4503de6151470f5f28ec35779e9"))
    (401279, uint256S("ee4e98cd84e1c47c37dc2c9bfc45e04200dd2ad984c09c0f4bd8c26a1979e3e1"))
    (407359, uint256S("6a2d9ee0b4bee5fce84eb202899e4a333a272fd2c87f37f931a74f9b4b9e47c7"))
    (413439, uint256S("0517bbaf8de1adc54634ad0af03e0fd44dea20243f244628d5719a23ce5aaec5"))
    (419519, uint256S("b4fe856ee32a83eb6742b1209fd8f4077f589cbd82e1868b689cd9501640c45f"))
    (425599, uint256S("8a2d353093974013802261f2d255429725d5c4484d9e91533f2b9d472416a378"))
    (431679, uint256S("924d0e545e3e653c53a9dbb9f7f9f8a1048a6bbc6a7239cc3994774d9adfb515"))
    (437759, uint256S("4482901dee3d637da3340fb3c717964fb98e1f6318188253cc7237c612fbd05a"))
    (443839, uint256S("a05cb85ecf5dead03a3e93220bf7cb051e5ea05abac7a8611717c2a8b838caf1"))
    (449919, uint256S("2652298b3480084ed39faefa73a57a3b97ffb326cf01bc917730a9b88b685770"))
    (455999, uint256S("6f3754ff4a4f3481b90465fd05c2ef6b3ab3de2f1ab4cf86448021622396af7d"))
    (462079, uint256S("e4c224b70dc9bb3c0af3ffaabe91107e17842c96b8a8c621f26870e6772def90"))
    (468159, uint256S("a8d65f9557b965d2e016b245666ad9eb5b5464253c6623f48159d941f813228d"))
    (474239, uint256S("4086de934bccf86a44a6779033081ec81dda9005a7a6e37260423c8987523e20"))
    (480319, uint256S("61b013a2df809fa16cd99bf010e79b179968cd219670592d98b9a0ab4485eaf9"))
    (486399, uint256S("00bbde91c1b90a4fdf654459c5c6f4ab0b2c140aa9e61d152d7cce1c073e9ce7"))
    (492479, uint256S("3c9b5c2ce29d4212df47790a55c01444ab24dc136844374fe9bbe1d408ce91c2"))
    (498559, uint256S("8ba82543350cbb2deb8a168acaacc069934ec193117b3b13310240e3b447487f"))
    (504639, uint256S("41a73a7f984e372c2d63b048297057961fe3c4a704c96b592cdd00d1c8242a7e"))
    (510719, uint256S("8515db365e27f430e370143c4df4dbd1a177d4596a39655bb3f38ce535f7075c"))
    (516799, uint256S("56e7440f4b2dae17d738b3e0e1cc12bee87a028d47f5509b0ae65acf8b470d54"))
    (522879, uint256S("8805e20d7fdaf710d09409a2a19b78d84720c015a4d36289f9304b0999f95502"))
    (528959, uint256S("81a78294763016e75c5deed8071a775513b2c1797e1cdc4ec355ebe73e3644a1"))
    (535039, uint256S("8945466d95b4cd76d298695f1c0b016db9140e0f9728e4561ff2c0c1a3ed0604"))
    (541119, uint256S("5586d4903c3f3d10d649bc62f64d14c283a8bef960bf6bbcb9a8124ed8fff573"))
    (547199, uint256S("ae7b022f7926707d9cf6d455251ee32f862aea382d25c6fc581c961c8e55fe09"))
    (553279, uint256S("b30d0aa79794c24c1b4e49e9a16d7089dd39d60bebbdd77c8c5fd59b97221e8c"))
    (559359, uint256S("0758355f0a64360f91b08beaf364480add51c1b7106d46fe73bbb79d4b1bf534"))
    (565439, uint256S("3b9900828a10a2ddce2af0f11da86cb2174e65df813a1879a24c0e90cc794ddb"))
    (571519, uint256S("e4b9087ade95daf97cb9c6474bcbaef8c55386f6c6b320296312861cf5d08056"))
    (577599, uint256S("d7d5c87fca7a459d56a04a65f3104654df3961442f72011f36b72b9fc089d3e9"))
    (583679, uint256S("86bb088752ef59893ea6df663865dcc140fdc6891708ac83fa8e6b6f0720bfc1"))
    (589759, uint256S("21f4805db8ee753020da322ea72f1b4795a7b0cc482b7321b4f8dcfad39649da"))
    (595839, uint256S("ed6dae30dbc9c0ef128fcab537e3bfe34708e850f3abf1f5e1e67cdc18e1247f"))
    (601919, uint256S("7f1d5d1effc1675178961b06caaa73c63da49eb336ce83339d59ab7ae9fce278"))
 
;

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1707746655, // * UNIX timestamp of last checkpoint block
    1273096,     // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the UpdateTip debug.log lines)
    2894        // * estimated number of transactions per day after checkpoint
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
        consensus.nRewardAdjustmentInterval = 7 * 1440;

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
        consensus.vUpgrades[Consensus::UPGRADE_DYNAMIC_REWARDS].nActivationHeight        = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_DYNAMIC_COLLATERALS].nActivationHeight    = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.vUpgrades[Consensus::UPGRADE_POS].hashActivationBlock                    = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].hashActivationBlock                 = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].hashActivationBlock                  = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MODIFIER_V2].hashActivationBlock      = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_TIME_PROTOCOL_V2].hashActivationBlock       = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_P2PKH_BLOCK_SIGNATURES].hashActivationBlock = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MIN_DEPTH_V2].hashActivationBlock     = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_MASTERNODE_RANK_V2].hashActivationBlock     = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_DYNAMIC_REWARDS].hashActivationBlock        = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_DYNAMIC_COLLATERALS].hashActivationBlock    = uint256S("0x0");

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
        consensus.nRewardAdjustmentInterval = 60;

        // spork keys
        consensus.strSporkPubKey = "04E88BB455E2A04E65FCC41D88CD367E9CCE1F5A409BE94D8C2B4B35D223DED9C8E2F4E061349BA3A38839282508066B6DC4DB72DD432AC4067991E6BF20176127";
        consensus.strSporkPubKeyOld = "";
        consensus.nTime_EnforceNewSporkKey = 0;
        consensus.nTime_RejectOldSporkKey = 0;

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight                      = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight                 = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight                       = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight                    = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight                     = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MODIFIER_V2].nActivationHeight         = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TIME_PROTOCOL_V2].nActivationHeight          = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_P2PKH_BLOCK_SIGNATURES].nActivationHeight    = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MIN_DEPTH_V2].nActivationHeight        = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_MASTERNODE_RANK_V2].nActivationHeight        = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_DYNAMIC_REWARDS].nActivationHeight           = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_DYNAMIC_COLLATERALS].nActivationHeight       = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.vUpgrades[Consensus::UPGRADE_POS].hashActivationBlock                     = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].hashActivationBlock                  = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].hashActivationBlock                   = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MODIFIER_V2].hashActivationBlock       = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_TIME_PROTOCOL_V2].hashActivationBlock        = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_P2PKH_BLOCK_SIGNATURES].hashActivationBlock  = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MIN_DEPTH_V2].hashActivationBlock      = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_STAKE_MIN_DEPTH_V2].hashActivationBlock      = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_DYNAMIC_REWARDS].hashActivationBlock         = uint256S("0x0");
        consensus.vUpgrades[Consensus::UPGRADE_DYNAMIC_COLLATERALS].hashActivationBlock     = uint256S("0x0");
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
