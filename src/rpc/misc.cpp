// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "clientversion.h"
#include "httpserver.h"
#include "init.h"
#include "main.h"
#include "masternode.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "masternode-sync.h"
#include "net.h"
#include "netbase.h"
#include "rewards.h"
#include "rpc/server.h"
#include "spork.h"
#include "timedata.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#endif

#include <stdint.h>

#include <boost/assign/list_of.hpp>

#include <univalue.h>

extern std::vector<CSporkDef> sporkDefs;


/**
 * @note Do not add or change anything in the information returned by this
 * method. `getinfo` exists for backwards-compatibility only. It combines
 * information from wildly different sources in the program, which is a mess,
 * and is thus planned to be deprecated eventually.
 *
 * Based on the source of the information, new information should be added to:
 * - `getblockchaininfo`,
 * - `getnetworkinfo` or
 * - `getwalletinfo`
 *
 * Or alternatively, create a specific query method for the information.
 **/
UniValue getinfo(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "getinfo\n"
            "\nReturns an object containing various state info.\n"

            "\nResult:\n"
            "{\n"
            "  \"version\": xxxxx,             (numeric) the server version\n"
            "  \"protocolversion\": xxxxx,     (numeric) the protocol version\n"
            "  \"walletversion\": xxxxx,       (numeric) the wallet version\n"
            "  \"balance\": xxxxxxx,           (numeric) the total FLS balance of the wallet\n"
            "  \"staking status\": true|false, (boolean) if the wallet is staking or not\n"
            "  \"blocks\": xxxxxx,             (numeric) the current number of blocks processed in the server\n"
            "  \"timeoffset\": xxxxx,          (numeric) the time offset\n"
            "  \"connections\": xxxxx,         (numeric) the number of connections\n"
            "  \"proxy\": \"host:port\",       (string, optional) the proxy used by the server\n"
            "  \"difficulty\": xxxxxx,         (numeric) the current difficulty\n"
            "  \"testnet\": true|false,        (boolean) if the server is using testnet or not\n"
            "  \"moneysupply\" : \"supply\"    (numeric) The money supply when this block was added to the blockchain\n"
            "  \"keypoololdest\": xxxxxx,      (numeric) the timestamp (seconds since GMT epoch) of the oldest pre-generated key in the key pool\n"
            "  \"keypoolsize\": xxxx,          (numeric) how many new keys are pre-generated\n"
            "  \"unlocked_until\": ttt,        (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked\n"
            "  \"paytxfee\": x.xxxx,           (numeric) the transaction fee set in FLS/kb\n"
            "  \"relayfee\": x.xxxx,           (numeric) minimum relay fee for non-free transactions in FLS/kb\n"
            "  \"errors\": \"...\"             (string) any error messages\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getinfo", "") + HelpExampleRpc("getinfo", ""));

#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
#else
    LOCK(cs_main);
#endif

    std::string services;
    for (int i = 0; i < 8; i++) {
        uint64_t check = 1 << i;
        if (g_connman->GetLocalServices() & check) {
            switch (check) {
                case NODE_NETWORK:
                    services+= "NETWORK/";
                    break;
                case NODE_BLOOM:
                case NODE_BLOOM_WITHOUT_MN:
                    services+= "BLOOM/";
                    break;
                default:
                    services+= "UNKNOWN/";
            }
        }
    }

    proxyType proxy;
    GetProxy(NET_IPV4, proxy);

    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("version", CLIENT_VERSION));
    obj.push_back(Pair("protocolversion", PROTOCOL_VERSION));
    obj.push_back(Pair("services", services));
#ifdef ENABLE_WALLET
    if (pwalletMain) {
        obj.push_back(Pair("walletversion", pwalletMain->GetVersion()));
        obj.push_back(Pair("balance", ValueFromAmount(pwalletMain->GetAvailableBalance())));
        obj.push_back(Pair("staking status", (pwalletMain->pStakerStatus->IsActive() ?
                                                "Staking Active" :
                                                "Staking Inactive")));
    }
#endif
    obj.push_back(Pair("blocks", (int)chainActive.Height()));
    obj.push_back(Pair("timeoffset", GetTimeOffset()));
    if(g_connman)
        obj.push_back(Pair("connections", (int)g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL)));
    obj.push_back(Pair("proxy", (proxy.IsValid() ? proxy.proxy.ToStringIPPort() : std::string())));
    obj.push_back(Pair("difficulty", (double)GetDifficulty()));
    obj.push_back(Pair("testnet", Params().NetworkID() == CBaseChainParams::TESTNET));

    // During inital block verification chainActive.Tip() might be not yet initialized
    if (chainActive.Tip() == NULL) {
        obj.push_back(Pair("status", "Blockchain information not yet available"));
        return obj;
    }

    obj.push_back(Pair("moneysupply", ValueFromAmount(chainActive.Tip()->nMoneySupply.get())));

#ifdef ENABLE_WALLET
    if (pwalletMain) {
        obj.push_back(Pair("keypoololdest", pwalletMain->GetOldestKeyPoolTime()));
        size_t kpExternalSize = pwalletMain->KeypoolCountExternalKeys();
        obj.push_back(Pair("keypoolsize", (int64_t)kpExternalSize));
    }
    if (pwalletMain && pwalletMain->IsCrypted())
        obj.push_back(Pair("unlocked_until", nWalletUnlockTime));
    obj.push_back(Pair("paytxfee", ValueFromAmount(payTxFee.GetFeePerK())));
#endif
    obj.push_back(Pair("relayfee", ValueFromAmount(::minRelayTxFee.GetFeePerK())));
    obj.push_back(Pair("errors", GetWarnings("statusbar")));
    return obj;
}

UniValue mnsync(const JSONRPCRequest& request)
{
    std::string strMode;
    if (request.params.size() == 1)
        strMode = request.params[0].get_str();

    if (request.fHelp || request.params.size() != 1 || (strMode != "status" && strMode != "reset")) {
        throw std::runtime_error(
            "mnsync \"status|reset\"\n"
            "\nReturns the sync status or resets sync.\n"

            "\nArguments:\n"
            "1. \"mode\"    (string, required) either 'status' or 'reset'\n"

            "\nResult ('status' mode):\n"
            "{\n"
            "  \"IsBlockchainSynced\": true|false,    (boolean) 'true' if blockchain is synced\n"
            "  \"lastMasternodeList\": xxxx,        (numeric) Timestamp of last MN list message\n"
            "  \"lastMasternodeWinner\": xxxx,      (numeric) Timestamp of last MN winner message\n"
            "  \"lastFailure\": xxxx,           (numeric) Timestamp of last failed sync\n"
            "  \"nCountFailures\": n,           (numeric) Number of failed syncs (total)\n"
            "  \"sumMasternodeList\": n,        (numeric) Number of MN list messages (total)\n"
            "  \"sumMasternodeWinner\": n,      (numeric) Number of MN winner messages (total)\n"
            "  \"countMasternodeList\": n,      (numeric) Number of MN list messages (local)\n"
            "  \"countMasternodeWinner\": n,    (numeric) Number of MN winner messages (local)\n"
            "  \"RequestedMasternodeAssets\": n, (numeric) Status code of last sync phase\n"
            "  \"RequestedMasternodeAttempt\": n, (numeric) Status code of last sync attempt\n"
            "}\n"

            "\nResult ('reset' mode):\n"
            "\"status\"     (string) 'success'\n"

            "\nExamples:\n" +
            HelpExampleCli("mnsync", "\"status\"") + HelpExampleRpc("mnsync", "\"status\""));
    }

    if (strMode == "status") {
        UniValue obj(UniValue::VOBJ);

        obj.push_back(Pair("IsBlockchainSynced", masternodeSync.IsBlockchainSynced()));
        obj.push_back(Pair("lastMasternodeList", masternodeSync.lastMasternodeList));
        obj.push_back(Pair("lastMasternodeWinner", masternodeSync.lastMasternodeWinner));
        obj.push_back(Pair("lastFailure", masternodeSync.lastFailure));
        obj.push_back(Pair("nCountFailures", masternodeSync.nCountFailures));
        obj.push_back(Pair("sumMasternodeList", masternodeSync.sumMasternodeList));
        obj.push_back(Pair("sumMasternodeWinner", masternodeSync.sumMasternodeWinner));
        obj.push_back(Pair("countMasternodeList", masternodeSync.countMasternodeList));
        obj.push_back(Pair("countMasternodeWinner", masternodeSync.countMasternodeWinner));
        obj.push_back(Pair("RequestedMasternodeAssets", masternodeSync.RequestedMasternodeAssets));
        obj.push_back(Pair("RequestedMasternodeAttempt", masternodeSync.RequestedMasternodeAttempt));

        return obj;
    }

    if (strMode == "reset") {
        LOCK(cs_main);
        
        g_connman->DisconnectAll();
        g_connman->ClearBanned();
        masternodeSync.Reset();
        return "success";
    }
    return "failure";
}

#ifdef ENABLE_WALLET
class DescribeAddressVisitor : public boost::static_visitor<UniValue>
{
private:
    isminetype mine;

public:
    DescribeAddressVisitor(isminetype mineIn) : mine(mineIn) {}

    UniValue operator()(const CNoDestination &dest) const { return UniValue(UniValue::VOBJ); }

    UniValue operator()(const CKeyID &keyID) const {
        UniValue obj(UniValue::VOBJ);
        CPubKey vchPubKey;
        obj.push_back(Pair("isscript", false));
        if (bool(mine & ISMINE_ALL)) {
            pwalletMain->GetPubKey(keyID, vchPubKey);
            obj.push_back(Pair("pubkey", HexStr(vchPubKey)));
            obj.push_back(Pair("iscompressed", vchPubKey.IsCompressed()));
            if(vchPubKey.IsCompressed()) {
                vchPubKey.Decompress();
                obj.push_back(Pair("pubkey", HexStr(vchPubKey)));
            }
        }
        return obj;
    }

    UniValue operator()(const CScriptID &scriptID) const {
        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("isscript", true));
        CScript subscript;
        pwalletMain->GetCScript(scriptID, subscript);
        std::vector<CTxDestination> addresses;
        txnouttype whichType;
        int nRequired;
        ExtractDestinations(subscript, whichType, addresses, nRequired);
        obj.push_back(Pair("script", GetTxnOutputType(whichType)));
        obj.push_back(Pair("hex", HexStr(subscript.begin(), subscript.end())));
        UniValue a(UniValue::VARR);
        for (const CTxDestination& addr : addresses)
            a.push_back(EncodeDestination(addr));
        obj.push_back(Pair("addresses", a));
        if (whichType == TX_MULTISIG)
            obj.push_back(Pair("sigsrequired", nRequired));
        return obj;
    }
};
#endif

/*
    Used for updating/reading spork settings on the network
*/
UniValue spork(const JSONRPCRequest& request)
{
    if (request.params.size() == 1 && request.params[0].get_str() == "show") {
        UniValue ret(UniValue::VOBJ);
        for (const auto& sporkDef : sporkDefs) {
            if (sporkDef.name.find("NOOP") == std::string::npos) { // show only useful sporks
                ret.push_back(Pair(sporkDef.name, sporkManager.GetSporkValue(sporkDef.sporkId)));
            }
        }
        return ret;
    } else if (request.params.size() == 1 && request.params[0].get_str() == "active") {
        UniValue ret(UniValue::VOBJ);
        for (const auto& sporkDef : sporkDefs) {
            ret.push_back(Pair(sporkDef.name, sporkManager.IsSporkActive(sporkDef.sporkId)));
        }
        return ret;
    } else if (request.params.size() >= 2) {
        // advanced mode, update spork values
        SporkId nSporkID = sporkManager.GetSporkIDByName(request.params[0].get_str());
        if (nSporkID == SPORK_INVALID) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid spork name");
        }

        // SPORK VALUE
        int64_t nValue = request.params[1].get_int64();

        // get private key
        std::string strSporkPrivateKey = "";

        if(request.params.size() > 2) {
            strSporkPrivateKey = request.params[2].get_str();
        }

        //broadcast new spork
        if (sporkManager.UpdateSpork(nSporkID, nValue, strSporkPrivateKey)) {
            return "success";
        } else {
            return "failure";
        }
    }

    throw std::runtime_error(
        "spork \"name\" ( value )\n"
        "\nReturn spork values or their active state.\n"

        "\nArguments:\n"
        "1. \"name\"        (string, required)  \"show\" to show values, \"active\" to show active state.\n"
        "                       When set up as a spork signer, the name of the spork can be used to update it's value.\n"
        "2. value           (numeric, required when updating a spork) The new value for the spork.\n"

        "\nResult (show):\n"
        "{\n"
        "  \"spork_name\": nnn      (key/value) Key is the spork name, value is it's current value.\n"
        "  ,...\n"
        "}\n"

        "\nResult (active):\n"
        "{\n"
        "  \"spork_name\": true|false      (key/value) Key is the spork name, value is a boolean for it's active state.\n"
        "  ,...\n"
        "}\n"

        "\nResult (name):\n"
        " \"success|failure\"       (string) Whether or not the update succeeded.\n"

        "\nExamples:\n" +
        HelpExampleCli("spork", "show") + HelpExampleRpc("spork", "show"));
}

class DescribePaymentAddressVisitor : public boost::static_visitor<UniValue>
{
public:
    explicit DescribePaymentAddressVisitor() {}
    
    UniValue operator()(const CTxDestination &dest) const {
        UniValue ret(UniValue::VOBJ);
        std::string currentAddress = EncodeDestination(dest);
        ret.push_back(Pair("address", currentAddress));
        CScript scriptPubKey = GetScriptForDestination(dest);
        ret.push_back(Pair("scriptPubKey", HexStr(scriptPubKey.begin(), scriptPubKey.end())));

#ifdef ENABLE_WALLET
        isminetype mine = pwalletMain ? IsMine(*pwalletMain, dest) : ISMINE_NO;
        ret.push_back(Pair("ismine", bool(mine & (ISMINE_SPENDABLE_ALL))));
        ret.push_back(Pair("iswatchonly", bool(mine & ISMINE_WATCH_ONLY)));
        UniValue detail = boost::apply_visitor(DescribeAddressVisitor(mine), dest);
        ret.pushKVs(detail);
        if (pwalletMain && pwalletMain->mapAddressBook.count(dest))
            ret.push_back(Pair("account", pwalletMain->mapAddressBook[dest].name));
#endif
        return ret;
    }

private:
};

UniValue validateaddress(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "validateaddress \"FLSaddress\"\n"
            "\nReturn information about the given FLS address.\n"

            "\nArguments:\n"
            "1. \"FLSaddress\"     (string, required) The FLS address to validate\n"

            "\nResult:\n"
            "{\n"
            "  \"isvalid\" : true|false,         (boolean) If the address is valid or not. If not, this is the only property returned.\n"
            "  \"type\" : \"xxxx\",              (string) \"standard\"\n"
            "  \"address\" : \"FLS address\",    (string) The FLS address validated\n"
            "  \"scriptPubKey\" : \"hex\",       (string) The hex encoded scriptPubKey generated by the address -only if is standard address-\n"
            "  \"ismine\" : true|false,          (boolean) If the address is yours or not\n"
            "  \"iswatchonly\" : true|false,     (boolean) If the address is watchonly -only if standard address-\n"
            "  \"isscript\" : true|false,        (boolean) If the key is a script -only if standard address-\n"
            "  \"hex\" : \"hex\",                (string, optional) The redeemscript for the P2SH address -only if standard address-\n"
            "  \"pubkey\" : \"publickey hex\",    (string) The hex value of the raw public key -only if standard address-\n"
            "  \"iscompressed\" : true|false,    (boolean) If the address is compressed -only if standard address-\n"
            "  (\"pubkey\" : \"decompressed publickey hex\",    (string) The hex value of the decompressed raw public key -only if standard address)-\n"
            "  \"account\" : \"account\"         (string) DEPRECATED. The account associated with the address, \"\" is the default account\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("validateaddress", "\"1PSSGeFHDnKNxiEyFrD1wcEaHr9hrQDDWc\""));

#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : nullptr);
#else
    LOCK(cs_main);
#endif

    std::string currentAddress = request.params[0].get_str();
    CTxDestination dest = DecodeDestination(currentAddress);
    bool isValid = IsValidDestination(dest);

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("isvalid", isValid));
    if (isValid) {
        ret.push_back(Pair("address", currentAddress));
        CScript scriptPubKey = GetScriptForDestination(dest);
        ret.push_back(Pair("scriptPubKey", HexStr(scriptPubKey.begin(), scriptPubKey.end())));

#ifdef ENABLE_WALLET
        isminetype mine = pwalletMain ? IsMine(*pwalletMain, dest) : ISMINE_NO;
        ret.push_back(Pair("ismine", bool(mine & (ISMINE_SPENDABLE_ALL))));
        ret.push_back(Pair("iswatchonly", bool(mine & ISMINE_WATCH_ONLY)));
        UniValue detail = boost::apply_visitor(DescribeAddressVisitor(mine), dest);
        ret.pushKVs(detail);
        if (pwalletMain && pwalletMain->mapAddressBook.count(dest))
            ret.push_back(Pair("account", pwalletMain->mapAddressBook[dest].name));
#endif
    }
    return ret;
}

/**
 * Used by addmultisigaddress / createmultisig:
 */
CScript _createmultisig_redeemScript(const UniValue& params)
{
    int nRequired = params[0].get_int();
    const UniValue& keys = params[1].get_array();

    // Gather public keys
    if (nRequired < 1)
        throw std::runtime_error("a multisignature address must require at least one key to redeem");
    if ((int)keys.size() < nRequired)
        throw std::runtime_error(
            strprintf("not enough keys supplied "
                      "(got %u keys, but need at least %d to redeem)",
                keys.size(), nRequired));
    if (keys.size() > 16)
        throw std::runtime_error("Number of addresses involved in the multisignature address creation > 16\nReduce the number");
    std::vector<CPubKey> pubkeys;
    pubkeys.resize(keys.size());
    for (unsigned int i = 0; i < keys.size(); i++) {
        const std::string& ks = keys[i].get_str();
#ifdef ENABLE_WALLET
        // Case 1: FLS address and we have full public key:
        CTxDestination dest = DecodeDestination(ks);
        if (pwalletMain && IsValidDestination(dest)) {
            const CKeyID* keyID = boost::get<CKeyID>(&dest);
            if (!keyID) {
                throw std::runtime_error(
                        strprintf("%s does not refer to a key", ks));
            }
            CPubKey vchPubKey;
            if (!pwalletMain->GetPubKey(*keyID, vchPubKey))
                throw std::runtime_error(
                    strprintf("no full public key for address %s", ks));
            if (!vchPubKey.IsFullyValid())
                throw std::runtime_error(" Invalid public key: " + ks);
            pubkeys[i] = vchPubKey;
        }

        // Case 2: hex public key
        else
#endif
            if (IsHex(ks)) {
            CPubKey vchPubKey(ParseHex(ks));
            if (!vchPubKey.IsFullyValid())
                throw std::runtime_error(" Invalid public key: " + ks);
            pubkeys[i] = vchPubKey;
        } else {
            throw std::runtime_error(" Invalid public key: " + ks);
        }
    }
    CScript result = GetScriptForMultisig(nRequired, pubkeys);

    if (result.size() > MAX_SCRIPT_ELEMENT_SIZE)
        throw std::runtime_error(
            strprintf("redeemScript exceeds size limit: %d > %d", result.size(), MAX_SCRIPT_ELEMENT_SIZE));

    return result;
}

UniValue createmultisig(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
        throw std::runtime_error(
            "createmultisig nrequired [\"key\",...]\n"
            "\nCreates a multi-signature address with n signature of m keys required.\n"
            "It returns a json object with the address and redeemScript.\n"

            "\nArguments:\n"
            "1. nrequired      (numeric, required) The number of required signatures out of the n keys or addresses.\n"
            "2. \"keys\"       (string, required) A json array of keys which are FLS addresses or hex-encoded public keys\n"
            "     [\n"
            "       \"key\"    (string) FLS address or hex-encoded public key\n"
            "       ,...\n"
            "     ]\n"

            "\nResult:\n"
            "{\n"
            "  \"address\":\"multisigaddress\",  (string) The value of the new multisig address.\n"
            "  \"redeemScript\":\"script\"       (string) The string value of the hex-encoded redemption script.\n"
            "}\n"

            "\nExamples:\n"
            "\nCreate a multisig address from 2 addresses\n" +
            HelpExampleCli("createmultisig", "2 \"[\\\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\"") +
            "\nAs a json rpc call\n" +
            HelpExampleRpc("createmultisig", "2, \"[\\\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\""));

    // Construct using pay-to-script-hash:
    CScript inner = _createmultisig_redeemScript(request.params);
    CScriptID innerID(inner);

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("address", EncodeDestination(innerID)));
    result.push_back(Pair("redeemScript", HexStr(inner.begin(), inner.end())));

    return result;
}

UniValue verifymessage(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 3)
        throw std::runtime_error(
            "verifymessage \"FLSaddress\" \"signature\" \"message\"\n"
            "\nVerify a signed message\n"

            "\nArguments:\n"
            "1. \"FLSaddress\"  (string, required) The FLS address to use for the signature.\n"
            "2. \"signature\"       (string, required) The signature provided by the signer in base 64 encoding (see signmessage).\n"
            "3. \"message\"         (string, required) The message that was signed.\n"

            "\nResult:\n"
            "true|false   (boolean) If the signature is verified or not.\n"

            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n" +
            HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n" +
            HelpExampleCli("signmessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" \"my message\"") +
            "\nVerify the signature\n" +
            HelpExampleCli("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" \"signature\" \"my message\"") +
            "\nAs json rpc\n" +
            HelpExampleRpc("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\", \"signature\", \"my message\""));

    LOCK(cs_main);

    std::string strAddress = request.params[0].get_str();
    std::string strSign = request.params[1].get_str();
    std::string strMessage = request.params[2].get_str();

    CTxDestination destination = DecodeDestination(strAddress);
    if (!IsValidDestination(destination))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    const CKeyID* keyID = boost::get<CKeyID>(&destination);
    if (!keyID) {
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");
    }

    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig))
        return false;

    return (pubkey.GetID() == *keyID);
}

UniValue setmocktime(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "setmocktime timestamp\n"
            "\nSet the local time to given timestamp (-regtest only)\n"

            "\nArguments:\n"
            "1. timestamp  (integer, required) Unix seconds-since-epoch timestamp\n"
            "   Pass 0 to go back to using the system time.");

    if (!Params().IsRegTestNet())
        throw std::runtime_error("setmocktime for regression testing (-regtest mode) only");

    LOCK(cs_main);

    RPCTypeCheck(request.params, boost::assign::list_of(UniValue::VNUM));
    SetMockTime(request.params[0].get_int64());

    uint64_t t = GetTime();
    if(g_connman) {
        g_connman->ForEachNode([t](CNode* pnode) {
            pnode->nLastSend = pnode->nLastRecv = t;
        });
    }

    return NullUniValue;
}

void EnableOrDisableLogCategories(UniValue cats, bool enable) {
    cats = cats.get_array();
    for (unsigned int i = 0; i < cats.size(); ++i) {
        std::string cat = cats[i].get_str();

        bool success;
        if (enable) {
            success = g_logger->EnableCategory(cat);
        } else {
            success = g_logger->DisableCategory(cat);
        }

        if (!success)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "unknown logging category " + cat);
    }
}

UniValue logging(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() > 2) {
        throw std::runtime_error(
            "logging [include,...] <exclude>\n"
            "Gets and sets the logging configuration.\n"
            "When called without an argument, returns the list of categories that are currently being debug logged.\n"
            "When called with arguments, adds or removes categories from debug logging.\n"
            "The valid logging categories are: " + ListLogCategories() + "\n"
            "libevent logging is configured on startup and cannot be modified by this RPC during runtime."
            "Arguments:\n"
            "1. \"include\" (array of strings) add debug logging for these categories.\n"
            "2. \"exclude\" (array of strings) remove debug logging for these categories.\n"
            "\nResult: <categories>  (string): a list of the logging categories that are active.\n"
            "\nExamples:\n"
            + HelpExampleCli("logging", "\"[\\\"all\\\"]\" \"[\\\"http\\\"]\"")
            + HelpExampleRpc("logging", "[\"all\"], \"[libevent]\"")
        );
    }

    uint32_t original_log_categories = g_logger->GetCategoryMask();
    if (request.params.size() > 0 && request.params[0].isArray()) {
        EnableOrDisableLogCategories(request.params[0], true);
    }

    if (request.params.size() > 1 && request.params[1].isArray()) {
        EnableOrDisableLogCategories(request.params[1], false);
    }
    uint32_t updated_log_categories = g_logger->GetCategoryMask();
    uint32_t changed_log_categories = original_log_categories ^ updated_log_categories;

    // Update libevent logging if BCLog::LIBEVENT has changed.
    // If the library version doesn't allow it, UpdateHTTPServerLogging() returns false,
    // in which case we should clear the BCLog::LIBEVENT flag.
    // Throw an error if the user has explicitly asked to change only the libevent
    // flag and it failed.
    if (changed_log_categories & BCLog::LIBEVENT) {
        if (!UpdateHTTPServerLogging(g_logger->WillLogCategory(BCLog::LIBEVENT))) {
            g_logger->DisableCategory(BCLog::LIBEVENT);
            if (changed_log_categories == BCLog::LIBEVENT) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "libevent logging cannot be updated when using libevent before v2.1.1.");
            }
        }
    }

    UniValue result(UniValue::VOBJ);
    std::vector<CLogCategoryActive> vLogCatActive = ListActiveLogCategories();
    for (const auto& logCatActive : vLogCatActive) {
        result.pushKV(logCatActive.category, logCatActive.active);
    }

    return result;
}

#ifdef ENABLE_WALLET
UniValue getstakingstatus(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0)
        throw std::runtime_error(
            "getstakingstatus\n"
            "\nReturns an object containing various staking information.\n"

            "\nResult:\n"
            "{\n"
            "  \"staking_status\": true|false,      (boolean) whether the wallet is staking or not\n"
            "  \"staking_enabled\": true|false,     (boolean) whether staking is enabled/disabled in flits.conf\n"
            "  \"haveconnections\": true|false,     (boolean) whether network connections are present\n"
            "  \"mnsync\": true|false,              (boolean) whether the required masternode/spork data is synced\n"
            "  \"walletunlocked\": true|false,      (boolean) whether the wallet is unlocked\n"
            "  \"stakeablecoins\": n                (numeric) number of stakeable UTXOs\n"
            "  \"stakingbalance\": d                (numeric) FLS value of the stakeable coins (minus reserve balance, if any)\n"
            "  \"stakesplitthreshold\": d           (numeric) value of the current threshold for stake split\n"
            "  \"autocombinethreshold\": d          (numeric) value of the current threshold for auto combine\n"
            "  \"lastattempt_age\": n               (numeric) seconds since last stake attempt\n"
            "  \"lastattempt_depth\": n             (numeric) depth of the block on top of which the last stake attempt was made\n"
            "  \"lastattempt_hash\": xxx            (hex string) hash of the block on top of which the last stake attempt was made\n"
            "  \"lastattempt_coins\": n             (numeric) number of stakeable coins available during last stake attempt\n"
            "  \"lastattempt_tries\": n             (numeric) number of stakeable coins checked during last stake attempt\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getstakingstatus", "") + HelpExampleRpc("getstakingstatus", ""));


    if (!pwalletMain)
        throw JSONRPCError(RPC_IN_WARMUP, "Try again after active chain is loaded");
    {
        LOCK2(cs_main, &pwalletMain->cs_wallet);
        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("staking_status", pwalletMain->pStakerStatus->IsActive()));
        obj.push_back(Pair("staking_active", fStakingActive));
        obj.push_back(Pair("staking_enabled", GetBoolArg("-staking", DEFAULT_STAKING)));
        obj.push_back(Pair("haveconnections", (g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL) > 0)));
        obj.push_back(Pair("mnsync", !masternodeSync.NotCompleted()));
        obj.push_back(Pair("walletunlocked", !pwalletMain->IsLocked()));
        std::vector<COutput> vCoins;
        pwalletMain->StakeableCoins(&vCoins);
        obj.push_back(Pair("stakeablecoins", (int)vCoins.size()));
        obj.push_back(Pair("stakingbalance", ValueFromAmount(pwalletMain->GetStakingBalance())));
        obj.push_back(Pair("stakesplitthreshold", ValueFromAmount(pwalletMain->nStakeSplitThreshold)));
        obj.push_back(Pair("autocombinethreshold", ValueFromAmount(pwalletMain->fCombineDust ? pwalletMain->nAutoCombineThreshold : 0)));
        CStakerStatus* ss = pwalletMain->pStakerStatus;
        if (ss) {
            obj.push_back(Pair("lastattempt_age", (int)(GetTime() - ss->GetLastTime())));
            obj.push_back(Pair("lastattempt_depth", (chainActive.Height() - ss->GetLastHeight())));
            obj.push_back(Pair("lastattempt_hash", ss->GetLastHash().GetHex()));
            obj.push_back(Pair("lastattempt_coins", ss->GetLastCoins()));
            obj.push_back(Pair("lastattempt_tries", ss->GetLastTries()));
        }
        return obj;
    }
}

UniValue getrewardsinfo(const JSONRPCRequest& request)
{
    std::string info_type = "all"; // Default value

    if (request.fHelp || (request.params.size() > 1 || (request.params.size() == 1 && !request.params[0].isStr()))) {
        throw std::runtime_error(
            "getrewardsinfo ( \"info_type\" )\n"
            "\nReturns an object containing ROI information.\n"

            "\nArguments:\n"
            "1. \"info_type\"   (string, optional, default=\"all\") The type of info to display (\"network\", \"wallet\", \"simple\", or \"all\")\n"

            "\nResult:\n"
            "{\n"
            "  \"network\": {\n"
            "    \"reward\": d,                     (numeric) reward per block\n"
            "    \"allocated_coins\": d,            (numeric) total number of allocated coins\n"
            "    \"allocation\": \"xx.xx%\",        (string) percentage of total allocation\n"
            "    \"blocks\": {\n"
            "      \"day\": n,                      (numeric) blocks generated per day\n"
            "      \"week\": n,                     (numeric) blocks generated per week\n"
            "      \"month\": n,                    (numeric) blocks generated per month\n"
            "      \"year\": n                      (numeric) blocks generated per year\n"
            "    },\n"
            "    \"staking\": {\n"
            "      \"reward\": d,                  (numeric) staking reward per block\n"
            "      \"hashrate\": {\n"
            "        \"networkhashps\": \"xxxxx\", (string) network hash rate per second\n"
            "        \"allocated_coins\": d,       (numeric) number of allocated coins for staking\n"
            "        \"roi\": \"xx.xx%\"           (string) return on investment for staking\n"
            "      },\n"
            "      \"smooth_hashrate\": {\n"
            "        \"networkhashps\": \"xxxxx\", (string) smoothed network hash rate per second\n"
            "        \"allocated_coins\": d,       (numeric) number of allocated coins for smoothed staking\n"
            "        \"roi\": \"xx.xx%\"           (string) smoothed return on investment for staking\n"
            "      },\n"
            "      \"allocation\": \"xx.xx%\"      (string) percentage of staking allocation\n"
            "    },\n"
            "    \"masternoding\": {\n"
            "      \"reward\": d,                  (numeric) masternode reward per block\n"
            "      \"collateral\": d,              (numeric) required collateral for a masternode\n"
            "      \"nextweek_collateral\": d,     (numeric) next week's required collateral for a masternode\n"
            "      \"enabled_masternodes\": d,     (numeric) number of enabled masternodes\n"
            "      \"allocated_coins\": d,         (numeric) number of allocated coins for masternodes\n"
            "      \"allocation\": \"xx.xx%\",     (string) percentage of masternode allocation\n"
            "      \"daily\": d,                   (numeric) daily masternode reward\n"
            "      \"daily_roi\": \"xx.xx%\",      (string) daily return on investment for masternodes\n"
            "      \"weekly\": d,                  (numeric) weekly masternode reward\n"
            "      \"weekly_roi\": \"xx.xx%\",     (string) weekly return on investment for masternodes\n"
            "      \"monthly\": d,                 (numeric) monthly masternode reward\n"
            "      \"monthly_roi\": \"xx.xx%\",    (string) monthly return on investment for masternodes\n"
            "      \"yearly\": d,                  (numeric) yearly masternode reward\n"
            "      \"yearly_roi\": \"xx.xx%\"      (string) yearly return on investment for masternodes\n"
            "    }\n"
            "  },\n"
            "  \"wallet\": {\n"
            "    \"staking\": {\n"
            "      \"status\": true|false,         (boolean) whether the wallet is staking or not\n"
            "      \"tries\": n,                   (numeric) number of tries for staking\n"
            "      \"allocated_coins\": d,         (numeric) number of allocated coins in the wallet for staking\n"
            "      \"share\": \"xx.xx%\",          (string) wallet's share of the staking allocation\n"
            "      \"stakesplitthreshold\": d,     (numeric) value of the current threshold for stake split\n"
            "      \"autocombinethreshold\": d,    (numeric) value of the current threshold for auto combine\n"
            "      \"regular\": {\n"
            "        \"daily\": d,                 (numeric) daily staking reward\n"
            "        \"daily_roi\": \"xx.xx%\",    (string) daily return on investment for staking\n"
            "        \"weekly\": d,                (numeric) weekly staking reward\n"
            "        \"weekly_roi\": \"xx.xx%\",   (string) weekly return on investment for staking\n"
            "        \"monthly\": d,               (numeric) monthly staking reward\n"
            "        \"monthly_roi\": \"xx.xx%\",  (string) monthly return on investment for staking\n"
            "        \"yearly\": d,                (numeric) yearly staking reward\n"
            "        \"yearly_roi\": \"xx.xx%\"    (string) yearly return on investment for staking\n"
            "      },\n"
            "      \"compound\": {\n"
            "        \"daily\": d,                 (numeric) compounded daily staking reward\n"
            "        \"daily_roi\": \"xx.xx%\",    (string) compounded daily return on investment for staking\n"
            "        \"weekly\": d,                (numeric) compounded weekly staking reward\n"
            "        \"weekly_roi\": \"xx.xx%\",   (string) compounded weekly return on investment for staking\n"
            "        \"monthly\": d,               (numeric) compounded monthly staking reward\n"
            "        \"monthly_roi\": \"xx.xx%\",  (string) compounded monthly return on investment for staking\n"
            "        \"yearly\": d,                (numeric) compounded yearly staking reward\n"
            "        \"yearly_roi\": \"xx.xx%\"    (string) compounded yearly return on investment for staking\n"
            "      }\n"
            "    },\n"
            "    \"masternoding\": {\n"
            "      \"masternodes\": n,             (numeric) number of masternodes owned by the wallet\n"
            "      \"allocated_coins\": d,         (numeric) number of allocated coins in the wallet for masternoding\n"
            "      \"share\": \"xx.xx%\",          (string) wallet's share of the masternode allocation\n"
            "      \"daily\": d,                   (numeric) daily masternode reward\n"
            "      \"daily_roi\": \"xx.xx%\",      (string) daily return on investment for masternodes\n"
            "      \"weekly\": d,                  (numeric) weekly masternode reward\n"
            "      \"weekly_roi\": \"xx.xx%\",     (string) weekly return on investment for masternodes\n"
            "      \"monthly\": d,                 (numeric) monthly masternode reward\n"
            "      \"monthly_roi\": \"xx.xx%\",    (string) monthly return on investment for masternodes\n"
            "      \"yearly\": d,                  (numeric) yearly masternode reward\n"
            "      \"yearly_roi\": \"xx.xx%\"      (string) yearly return on investment for masternodes\n"
            "    }\n"
            "  }\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getrewardsinfo", "") + HelpExampleRpc("getrewardsinfo", ""));
    }

    if (request.params.size() == 1) {
        info_type = request.params[0].get_str();
        if (info_type != "network" && info_type != "wallet" && info_type != "simple" && info_type != "all") {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid info_type parameter, must be \"network\", \"wallet\", \"simple\", or \"all\"");
        }
    }

    CBlockchainStatus cbs;
    int cbserror = cbs.getblockchainstatus();
    if (cbserror > 0) {
        throw JSONRPCError(RPC_IN_WARMUP, "Try again after the active chain is loaded");
    } else
    if (cbserror < 0) {
        throw JSONRPCError(RPC_IN_WARMUP, "Try again after the chain is fully synced");
    }
    int64_t nBlocksPerWeek = cbs.nBlocksPerDay * 7;
    int64_t nBlocksPerMonth = cbs.nBlocksPerDay * 30;
    int64_t nBlocksPerYear = cbs.nBlocksPerDay * 365;

    const auto nStakingAllocation = static_cast<double>(cbs.nSmoothStakedCoins) / cbs.nMoneySupplyThisBlock;
    const auto nYearlyStakingRewards = cbs.nStakeReward * nBlocksPerYear;
    const auto nMNAllocation = static_cast<double>(cbs.nMNCoins) / cbs.nMoneySupplyThisBlock;

    const auto nTotalAllocationCoins = cbs.nSmoothStakedCoins + cbs.nMNCoins;
    const auto nTotalAllocation = nStakingAllocation + nMNAllocation;

    const auto nYearlyMNRewards = cbs.nMNReward * nBlocksPerYear;
    const auto nMasternodingRewards = static_cast<double>((cbs.nMNReward * nBlocksPerYear) / cbs.nMNEnabled);
    const auto nDailyMasternodingRewards = nMasternodingRewards / 365;

    const auto fStakingActive = pwalletMain->pStakerStatus->IsActive();
    const auto nStakedBalance =
        pwalletMain->mapWallet.empty() ? 
            0 :
            fStakingActive ?
                pwalletMain->pStakerStatus->GetLastValue() :
                pwalletMain->GetStakingBalance();
    const auto nStakeShare = static_cast<double>(nStakedBalance) / cbs.nSmoothStakedCoins;

    UniValue obj(UniValue::VOBJ);

    if (info_type == "simple") {

        const auto nMasternodingYearlyRewardsROI = (cbs.nMNReward * nBlocksPerYear) / cbs.nMNCoins;

        obj.push_back(Pair("staking_roi", strprintf("%4.2f%%", cbs.nStakingROI * 100)));
        obj.push_back(Pair("smooth_staking_roi", strprintf("%4.2f%%", cbs.nSmoothStakingROI * 100)));
        obj.push_back(Pair("staked_coins", ValueFromAmount(cbs.nSmoothStakedCoins)));
        obj.push_back(Pair("masternoding_roi", strprintf("%4.2f%%", nMasternodingYearlyRewardsROI * 100)));
        obj.push_back(Pair("masternoded_coins", ValueFromAmount(cbs.nMNCoins)));
        obj.push_back(Pair("enabled_masternodes", cbs.nMNEnabled));
        obj.push_back(Pair("blocks_per_day", cbs.nBlocksPerDay));

    } else if (info_type == "network" || info_type == "all") {
            
        UniValue networkObj(UniValue::VOBJ);

        networkObj.push_back(Pair("reward", ValueFromAmount(cbs.nBlockValue)));
        networkObj.push_back(Pair("allocated_coins", ValueFromAmount(nTotalAllocationCoins)));
        networkObj.push_back(Pair("allocation", strprintf("%4.2f%%", nTotalAllocation * 100)));

        UniValue blocksObj(UniValue::VOBJ);
        blocksObj.push_back(Pair("day", cbs.nBlocksPerDay));
        blocksObj.push_back(Pair("week", nBlocksPerWeek));
        blocksObj.push_back(Pair("month", nBlocksPerMonth));
        blocksObj.push_back(Pair("year", nBlocksPerYear));
        networkObj.push_back(Pair("blocks", blocksObj));

        UniValue stakingObj(UniValue::VOBJ);

        stakingObj.push_back(Pair("reward", ValueFromAmount(cbs.nStakeReward)));

        UniValue hashRateObj(UniValue::VOBJ);
        hashRateObj.push_back(Pair("networkhashps", GetReadableHashRate(cbs.nNetworkHashPS)));
        hashRateObj.push_back(Pair("allocated_coins", ValueFromAmount(cbs.nStakedCoins)));
        hashRateObj.push_back(Pair("roi", strprintf("%4.2f%%", cbs.nStakingROI * 100)));
        stakingObj.push_back(Pair("hashrate", hashRateObj));

        UniValue smoothHashRateObj(UniValue::VOBJ);
        smoothHashRateObj.push_back(Pair("networkhashps", GetReadableHashRate(cbs.nSmoothNetworkHashPS)));
        smoothHashRateObj.push_back(Pair("allocated_coins", ValueFromAmount(cbs.nSmoothStakedCoins)));
        smoothHashRateObj.push_back(Pair("roi", strprintf("%4.2f%%", cbs.nSmoothStakingROI * 100)));
        stakingObj.push_back(Pair("smooth_hashrate", smoothHashRateObj));

        stakingObj.push_back(Pair("allocation", strprintf("%4.2f%%", nStakingAllocation * 100)));

        {
            // Calculate all data for the wallet's staking ROI
            UniValue stakingRegularObj(UniValue::VOBJ);

            const auto nStakingRewards = nYearlyStakingRewards;
            const auto nStakingDailyRewards = nStakingRewards / 365;
            const auto nStakingWeeklyRewards = nStakingDailyRewards * 7;
            const auto nStakingMonthlyRewards = nStakingDailyRewards * 30;
            const auto nStakingYearlyRewards = nStakingRewards;

            const auto nStakingDailyRewardsROI = cbs.nSmoothStakedCoins == 0 ? 0 : nStakingDailyRewards / cbs.nSmoothStakedCoins;
            const auto nStakingWeeklyRewardsROI = cbs.nSmoothStakedCoins == 0 ? 0 : nStakingWeeklyRewards / cbs.nSmoothStakedCoins;
            const auto nStakingMonthlyRewardsROI = cbs.nSmoothStakedCoins == 0 ? 0 : nStakingMonthlyRewards / cbs.nSmoothStakedCoins;
            const auto nStakingYearlyRewardsROI = cbs.nSmoothStakedCoins == 0 ? 0 : nStakingYearlyRewards / cbs.nSmoothStakedCoins;

            stakingRegularObj.push_back(Pair("daily", ValueFromAmount(nStakingDailyRewards)));
            stakingRegularObj.push_back(Pair("daily_roi", strprintf("%4.2f%%", nStakingDailyRewardsROI * 100)));
            stakingRegularObj.push_back(Pair("weekly", ValueFromAmount(nStakingWeeklyRewards)));
            stakingRegularObj.push_back(Pair("weekly_roi", strprintf("%4.2f%%", nStakingWeeklyRewardsROI * 100)));
            stakingRegularObj.push_back(Pair("monthly", ValueFromAmount(nStakingMonthlyRewards)));
            stakingRegularObj.push_back(Pair("monthly_roi", strprintf("%4.2f%%", nStakingMonthlyRewardsROI * 100)));
            stakingRegularObj.push_back(Pair("yearly", ValueFromAmount(nStakingYearlyRewards)));
            stakingRegularObj.push_back(Pair("yearly_roi", strprintf("%4.2f%%", nStakingYearlyRewardsROI * 100)));

            stakingObj.push_back(Pair("regular", stakingRegularObj));

            {
                // Compounding calculations
                UniValue stakingCompoundObj(UniValue::VOBJ);

                auto nStakingBalanceAccumulator = static_cast<double>(cbs.nSmoothStakedCoins);

                for (int i = 1; i <= 365; i++) {
                    nStakingBalanceAccumulator += ((nStakingBalanceAccumulator * cbs.nSmoothStakingROI) / 365);

                    if (i == 1) { // After a day
                        const auto nCompoundStakingDailyRewards = nStakingBalanceAccumulator - cbs.nSmoothStakedCoins;
                        const auto nCompoundStakingDailyRewardsROI = cbs.nSmoothStakedCoins == 0 ? 0 : nCompoundStakingDailyRewards / cbs.nSmoothStakedCoins;

                        stakingCompoundObj.push_back(Pair("daily", ValueFromAmount(nCompoundStakingDailyRewards)));
                        stakingCompoundObj.push_back(Pair("daily_roi", strprintf("%4.2f%%", nCompoundStakingDailyRewardsROI * 100)));
                    }

                    if (i == 7) { // After a week
                        const auto nCompoundStakingWeeklyRewards = nStakingBalanceAccumulator - cbs.nSmoothStakedCoins;
                        const auto nCompoundStakingWeeklyRewardsROI = cbs.nSmoothStakedCoins == 0 ? 0 : nCompoundStakingWeeklyRewards / cbs.nSmoothStakedCoins;

                        stakingCompoundObj.push_back(Pair("weekly", ValueFromAmount(nCompoundStakingWeeklyRewards)));
                        stakingCompoundObj.push_back(Pair("weekly_roi", strprintf("%4.2f%%", nCompoundStakingWeeklyRewardsROI * 100)));
                    }

                    if (i == 30) { // After a month
                        const auto nCompoundStakingMonthlyRewards = nStakingBalanceAccumulator - cbs.nSmoothStakedCoins;
                        const auto nCompoundStakingMonthlyRewardsROI = cbs.nSmoothStakedCoins == 0 ? 0 : nCompoundStakingMonthlyRewards / cbs.nSmoothStakedCoins;

                        stakingCompoundObj.push_back(Pair("monthly", ValueFromAmount(nCompoundStakingMonthlyRewards)));
                        stakingCompoundObj.push_back(Pair("monthly_roi", strprintf("%4.2f%%", nCompoundStakingMonthlyRewardsROI * 100)));
                    }

                    if (i == 365) { // After a year
                        const auto nCompoundStakingYearlyRewards = nStakingBalanceAccumulator - cbs.nSmoothStakedCoins;
                        const auto nCompoundStakingYearlyRewardsROI = cbs.nSmoothStakedCoins == 0 ? 0 : nCompoundStakingYearlyRewards / cbs.nSmoothStakedCoins;

                        stakingCompoundObj.push_back(Pair("yearly", ValueFromAmount(nCompoundStakingYearlyRewards)));
                        stakingCompoundObj.push_back(Pair("yearly_roi", strprintf("%4.2f%%", nCompoundStakingYearlyRewardsROI * 100)));
                    }
                }

                stakingObj.push_back(Pair("compound", stakingCompoundObj));
            }
        }

        networkObj.push_back(Pair("staking", stakingObj));

        {
            UniValue masternodingObj(UniValue::VOBJ);

            masternodingObj.push_back(Pair("reward", ValueFromAmount(cbs.nMNReward)));
            masternodingObj.push_back(Pair("collateral", ValueFromAmount(cbs.nMNCollateral)));
            masternodingObj.push_back(Pair("nextweek_collateral", ValueFromAmount(cbs.nMNNextWeekCollateral)));
            masternodingObj.push_back(Pair("enabled_masternodes", cbs.nMNEnabled));
            masternodingObj.push_back(Pair("allocated_coins", ValueFromAmount(cbs.nMNCoins)));
            masternodingObj.push_back(Pair("allocation", strprintf("%4.2f%%", nMNAllocation * 100)));

            const auto nMasternodingDailyRewards = nDailyMasternodingRewards * cbs.nMNEnabled;
            const auto nMasternodingWeeklyRewards = nMasternodingDailyRewards * 7;
            const auto nMasternodingMonthlyRewards = nMasternodingDailyRewards * 30;
            const auto nMasternodingYearlyRewards = nMasternodingRewards * cbs.nMNEnabled;

            const auto nMasternodingDailyRewardsROI = nMasternodingDailyRewards / cbs.nMNCoins;
            const auto nMasternodingWeeklyRewardsROI = nMasternodingWeeklyRewards / cbs.nMNCoins;
            const auto nMasternodingMonthlyRewardsROI = nMasternodingMonthlyRewards / cbs.nMNCoins;
            const auto nMasternodingYearlyRewardsROI = nMasternodingYearlyRewards / cbs.nMNCoins;

            masternodingObj.push_back(Pair("daily", ValueFromAmount(nMasternodingDailyRewards)));
            masternodingObj.push_back(Pair("daily_roi", strprintf("%4.2f%%", nMasternodingDailyRewardsROI * 100)));
            masternodingObj.push_back(Pair("weekly", ValueFromAmount(nMasternodingWeeklyRewards)));
            masternodingObj.push_back(Pair("weekly_roi", strprintf("%4.2f%%", nMasternodingWeeklyRewardsROI * 100)));
            masternodingObj.push_back(Pair("monthly", ValueFromAmount(nMasternodingMonthlyRewards)));
            masternodingObj.push_back(Pair("monthly_roi", strprintf("%4.2f%%", nMasternodingMonthlyRewardsROI * 100)));
            masternodingObj.push_back(Pair("yearly", ValueFromAmount(nMasternodingYearlyRewards)));
            masternodingObj.push_back(Pair("yearly_roi", strprintf("%4.2f%%", nMasternodingYearlyRewardsROI * 100)));

            networkObj.push_back(Pair("masternoding", masternodingObj));
        }

        obj.push_back(Pair("network", networkObj));
    }

    if (info_type == "wallet" || info_type == "all") {
        UniValue walletObj(UniValue::VOBJ);

        UniValue walletStakingObj(UniValue::VOBJ);

        walletStakingObj.push_back(Pair("status", fStakingActive));
        walletStakingObj.push_back(Pair("tries", pwalletMain->pStakerStatus->GetLastTries()));
        walletStakingObj.push_back(Pair("allocated_coins", ValueFromAmount(nStakedBalance)));
        walletStakingObj.push_back(Pair("share", strprintf("%4.2f%%", nStakeShare * 100)));
        walletStakingObj.push_back(Pair("stakesplitthreshold", ValueFromAmount(pwalletMain->nStakeSplitThreshold)));
        walletStakingObj.push_back(Pair("autocombinethreshold", ValueFromAmount(pwalletMain->fCombineDust ? pwalletMain->nAutoCombineThreshold : 0)));

        {
            // Calculate all data for the wallet's staking ROI
            UniValue stakingRegularObj(UniValue::VOBJ);
            const auto nStakingRewards = nStakedBalance * cbs.nSmoothStakingROI;
            const auto nStakingDailyRewards = nStakingRewards / 365;
            const auto nStakingWeeklyRewards = nStakingDailyRewards * 7;
            const auto nStakingMonthlyRewards = nStakingDailyRewards * 30;
            const auto nStakingYearlyRewards = nStakingRewards;

            const auto nStakingDailyRewardsROI = nStakedBalance == 0 ? 0 : nStakingDailyRewards / nStakedBalance;
            const auto nStakingWeeklyRewardsROI = nStakedBalance == 0 ? 0 : nStakingWeeklyRewards / nStakedBalance;
            const auto nStakingMonthlyRewardsROI = nStakedBalance == 0 ? 0 : nStakingMonthlyRewards / nStakedBalance;
            const auto nStakingYearlyRewardsROI = nStakedBalance == 0 ? 0 : nStakingYearlyRewards / nStakedBalance;

            stakingRegularObj.push_back(Pair("daily", ValueFromAmount(nStakingDailyRewards)));
            stakingRegularObj.push_back(Pair("daily_roi", strprintf("%4.2f%%", nStakingDailyRewardsROI * 100)));
            stakingRegularObj.push_back(Pair("weekly", ValueFromAmount(nStakingWeeklyRewards)));
            stakingRegularObj.push_back(Pair("weekly_roi", strprintf("%4.2f%%", nStakingWeeklyRewardsROI * 100)));
            stakingRegularObj.push_back(Pair("monthly", ValueFromAmount(nStakingMonthlyRewards)));
            stakingRegularObj.push_back(Pair("monthly_roi", strprintf("%4.2f%%", nStakingMonthlyRewardsROI * 100)));
            stakingRegularObj.push_back(Pair("yearly", ValueFromAmount(nStakingYearlyRewards)));
            stakingRegularObj.push_back(Pair("yearly_roi", strprintf("%4.2f%%", nStakingYearlyRewardsROI * 100)));

            walletStakingObj.push_back(Pair("regular", stakingRegularObj));

            {
                // Compounding calculations
                UniValue stakingCompoundObj(UniValue::VOBJ);

                auto nStakingBalanceAccumulator = static_cast<double>(nStakedBalance);

                for (int i = 1; i <= 365; i++) {
                    nStakingBalanceAccumulator += ((nStakingBalanceAccumulator * cbs.nSmoothStakingROI) / 365);

                    if (i == 1) { // After a day
                        const auto nCompoundStakingDailyRewards = nStakingBalanceAccumulator - nStakedBalance;
                        const auto nCompoundStakingDailyRewardsROI = nStakedBalance == 0 ? 0 : nCompoundStakingDailyRewards / nStakedBalance;

                        stakingCompoundObj.push_back(Pair("daily", ValueFromAmount(nCompoundStakingDailyRewards)));
                        stakingCompoundObj.push_back(Pair("daily_roi", strprintf("%4.2f%%", nCompoundStakingDailyRewardsROI * 100)));
                    }

                    if (i == 7) { // After a week
                        const auto nCompoundStakingWeeklyRewards = nStakingBalanceAccumulator - nStakedBalance;
                        const auto nCompoundStakingWeeklyRewardsROI = nStakedBalance == 0 ? 0 : nCompoundStakingWeeklyRewards / nStakedBalance;

                        stakingCompoundObj.push_back(Pair("weekly", ValueFromAmount(nCompoundStakingWeeklyRewards)));
                        stakingCompoundObj.push_back(Pair("weekly_roi", strprintf("%4.2f%%", nCompoundStakingWeeklyRewardsROI * 100)));
                    }

                    if (i == 30) { // After a month
                        const auto nCompoundStakingMonthlyRewards = nStakingBalanceAccumulator - nStakedBalance;
                        const auto nCompoundStakingMonthlyRewardsROI = nStakedBalance == 0 ? 0 : nCompoundStakingMonthlyRewards / nStakedBalance;

                        stakingCompoundObj.push_back(Pair("monthly", ValueFromAmount(nCompoundStakingMonthlyRewards)));
                        stakingCompoundObj.push_back(Pair("monthly_roi", strprintf("%4.2f%%", nCompoundStakingMonthlyRewardsROI * 100)));
                    }

                    if (i == 365) { // After a year
                        const auto nCompoundStakingYearlyRewards = nStakingBalanceAccumulator - nStakedBalance;
                        const auto nCompoundStakingYearlyRewardsROI = nStakedBalance == 0 ? 0 : nCompoundStakingYearlyRewards / nStakedBalance;

                        stakingCompoundObj.push_back(Pair("yearly", ValueFromAmount(nCompoundStakingYearlyRewards)));
                        stakingCompoundObj.push_back(Pair("yearly_roi", strprintf("%4.2f%%", nCompoundStakingYearlyRewardsROI * 100)));
                    }
                }

                walletStakingObj.push_back(Pair("compound", stakingCompoundObj));
            }
        }

        walletObj.push_back(Pair("staking", walletStakingObj));

        {
            // Calculate all data for the wallet's masternoding ROI
            UniValue walletMasternodingObj(UniValue::VOBJ);

            const auto nMNs = masternodeConfig.getCount() + 1; // legacy preservation without showing a strange counting
            const auto nMNBalance = nMNs * cbs.nMNCollateral;
            const auto nMNShare = static_cast<double>(nMNBalance) / cbs.nMNCoins;

            walletMasternodingObj.push_back(Pair("masternodes", nMNs));
            walletMasternodingObj.push_back(Pair("allocated_coins", ValueFromAmount(nMNBalance)));
            walletMasternodingObj.push_back(Pair("share", strprintf("%4.2f%%", nMNShare * 100)));
        
            const auto nMasternodingDailyRewards = nDailyMasternodingRewards * nMNs;
            const auto nMasternodingWeeklyRewards = nMasternodingDailyRewards * 7;
            const auto nMasternodingMonthlyRewards = nMasternodingDailyRewards * 30;
            const auto nMasternodingYearlyRewards = nMasternodingRewards * nMNs;

            const auto nMasternodingDailyRewardsROI = nMNBalance == 0 ? 0 : nMasternodingDailyRewards / nMNBalance;
            const auto nMasternodingWeeklyRewardsROI = nMNBalance == 0 ? 0 : nMasternodingWeeklyRewards / nMNBalance;
            const auto nMasternodingMonthlyRewardsROI = nMNBalance == 0 ? 0 : nMasternodingMonthlyRewards / nMNBalance;
            const auto nMasternodingYearlyRewardsROI = nMNBalance == 0 ? 0 : nMasternodingYearlyRewards / nMNBalance;

            walletMasternodingObj.push_back(Pair("daily", ValueFromAmount(nMasternodingDailyRewards)));
            walletMasternodingObj.push_back(Pair("daily_roi", strprintf("%4.2f%%", nMasternodingDailyRewardsROI * 100)));
            walletMasternodingObj.push_back(Pair("weekly", ValueFromAmount(nMasternodingWeeklyRewards)));
            walletMasternodingObj.push_back(Pair("weekly_roi", strprintf("%4.2f%%", nMasternodingWeeklyRewardsROI * 100)));
            walletMasternodingObj.push_back(Pair("monthly", ValueFromAmount(nMasternodingMonthlyRewards)));
            walletMasternodingObj.push_back(Pair("monthly_roi", strprintf("%4.2f%%", nMasternodingMonthlyRewardsROI * 100)));
            walletMasternodingObj.push_back(Pair("yearly", ValueFromAmount(nMasternodingYearlyRewards)));
            walletMasternodingObj.push_back(Pair("yearly_roi", strprintf("%4.2f%%", nMasternodingYearlyRewardsROI * 100)));

            walletObj.push_back(Pair("masternoding", walletMasternodingObj));
        }

        obj.push_back(Pair("wallet", walletObj));
    }
    return obj;
}

#endif // ENABLE_WALLET
