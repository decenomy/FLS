// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "activemasternodeman.h"
#include "db.h"
#include "init.h"
#include "main.h"
#include "masternode-payments.h"
#include "masternode-sync.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "netbase.h"
#include "rpc/server.h"
#include "spork.h"
#include "utilmoneystr.h"

#include <univalue.h>

#include <boost/tokenizer.hpp>

UniValue mnping(const JSONRPCRequest& request)
{
    if (request.fHelp || !request.params.empty()) {
        throw std::runtime_error(
            "mnping \n"
            "\nSend masternode ping.\n"

            "\nResult:\n"
            "[{\n"
            "  \"alias\": \"xxxx\",        (string) masternode alias\n"        
            "  \"sent\":  (string YES|NO) Whether the ping was sent and, if not, the error.\n"
            "}]\n"

            "\nExamples:\n" +
            HelpExampleCli("mnping", "") + HelpExampleRpc("mnping", ""));
    }

    if (!fMasterNode) {
        throw JSONRPCError(RPC_MISC_ERROR, "this is not a masternode");
    }

    UniValue resultsObj(UniValue::VARR);
        
    auto amns = amnodeman.GetActiveMasternodes();

    for (auto& amn : amns) {

        UniValue mnObj(UniValue::VOBJ);
        std::string strError;

        mnObj.push_back(Pair("alias", amn.strAlias));
        mnObj.push_back(Pair("sent", amn.SendMasternodePing(strError) ? "YES" : strprintf("NO (%s)", strError)));

        resultsObj.push_back(mnObj);
    }

    return resultsObj;
}

UniValue listmasternodes(const JSONRPCRequest& request)
{
    std::string strFilter = "";

    if (request.params.size() == 1) strFilter = request.params[0].get_str();

    if (request.fHelp || (request.params.size() > 1))
        throw std::runtime_error(
            "listmasternodes ( \"filter\" )\n"
            "\nGet a list of masternodes\n"

            "\nArguments:\n"
            "1. \"filter\"    (string, optional) Filter search text. Partial match by txhash, status, or addr.\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"txhash\": \"hash\",  (string) Collateral transaction hash\n"
            "    \"outidx\": n,         (numeric) Collateral transaction output index\n"
            "    \"pubkey\": \"key\",   (string) Masternode public key used for message broadcasting\n"
            "    \"status\": s,         (string) Status (ENABLED/EXPIRED/REMOVE/etc)\n"
            "    \"addr\": \"addr\",    (string) Masternode __DSW__ address\n"
            "    \"ip\": \"ip\",        (string) Masternode IP address\n"
            "    \"version\": v,        (numeric) Masternode protocol version\n"
            "    \"lastseen\": ttt,     (numeric) The time in seconds since epoch (Jan 1 1970 GMT) of the last seen\n"
            "    \"activetime\": ttt,   (numeric) The time in seconds since epoch (Jan 1 1970 GMT) masternode has been active\n"
            "    \"lastpaid\": ttt,     (numeric) The time in seconds since epoch (Jan 1 1970 GMT) masternode was last paid\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n" +
            HelpExampleCli("listmasternodes", "") + HelpExampleRpc("listmasternodes", ""));

    UniValue ret(UniValue::VARR);
    const auto pIndex = WITH_LOCK(cs_main, return chainActive.Tip());
    const auto nHeight = pIndex->nHeight;
    if (nHeight < 0) return "[]";

    for (auto& mn : mnodeman.GetFullMasternodeVector()) {
        UniValue obj(UniValue::VOBJ);
        std::string strVin = mn.vin.prevout.ToStringShort();
        std::string strTxHash = mn.vin.prevout.hash.ToString();
        uint32_t oIdx = mn.vin.prevout.n;

        if (strFilter != "" && 
            strTxHash.find(strFilter) == std::string::npos &&
            mn.Status().find(strFilter) == std::string::npos &&
            EncodeDestination(mn.pubKeyCollateralAddress.GetID()).find(strFilter) == std::string::npos) continue;

        std::string strStatus = mn.Status();
        std::string strHost;
        int port;
        SplitHostPort(mn.addr.ToString(), port, strHost);
        CNetAddr node;
        LookupHost(strHost.c_str(), node, false);
        std::string strNetwork = GetNetworkName(node.GetNetwork());

        obj.push_back(Pair("network", strNetwork));
        obj.push_back(Pair("txhash", strTxHash));
        obj.push_back(Pair("outidx", (uint64_t)oIdx));
        obj.push_back(Pair("pubkey", HexStr(mn.pubKeyMasternode)));
        obj.push_back(Pair("status", strStatus));
        obj.push_back(Pair("addr", EncodeDestination(mn.pubKeyCollateralAddress.GetID())));
        obj.push_back(Pair("ip", mn.addr.ToString()));
        obj.push_back(Pair("version", mn.protocolVersion));
        obj.push_back(Pair("lastseen", mn.lastPing.sigTime));
        obj.push_back(Pair("activetime", mn.lastPing.sigTime - mn.sigTime));
        obj.push_back(Pair("lastpaid", mn.GetLastPaid(pIndex)));

        ret.push_back(obj);
    }

    return ret;
}

UniValue getmasternodecount (const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() > 0))
        throw std::runtime_error(
            "getmasternodecount\n"
            "\nGet masternode count values\n"

            "\nResult:\n"
            "{\n"
            "  \"total\": n,        (numeric) Total masternodes\n"
            "  \"stable\": n,       (numeric) Stable count\n"
            "  \"enabled\": n,      (numeric) Enabled masternodes\n"
            "  \"inqueue\": n       (numeric) Masternodes in queue\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getmasternodecount", "") + HelpExampleRpc("getmasternodecount", ""));

    UniValue obj(UniValue::VOBJ);
    int ipv4 = 0, ipv6 = 0, onion = 0;

    const auto tipIndex = WITH_LOCK(cs_main, return chainActive.Tip());
    const auto nChainHeight = tipIndex->nHeight;
    if (nChainHeight < 0) return "unknown";

    mnodeman.CountNetworks(ipv4, ipv6, onion);

    obj.push_back(Pair("total", mnodeman.size()));

    if (masternodeSync.IsSynced()) {
        int nCount = mnodeman.GetNextMasternodeInQueueCount(tipIndex);

        obj.push_back(Pair("stable", mnodeman.stable_size()));
        obj.push_back(Pair("enabled", mnodeman.CountEnabled()));
        obj.push_back(Pair("inqueue", nCount));
    }

    obj.push_back(Pair("ipv4", ipv4));
    obj.push_back(Pair("ipv6", ipv6));
    obj.push_back(Pair("onion", onion));

    return obj;
}

UniValue masternodecurrent (const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() != 0))
        throw std::runtime_error(
            "masternodecurrent\n"
            "\nGet current masternode winner (scheduled to be paid next).\n"

            "\nResult:\n"
            "{\n"
            "  \"protocol\": xxxx,        (numeric) Protocol version\n"
            "  \"txhash\": \"xxxx\",      (string) Collateral transaction hash\n"
            "  \"pubkey\": \"xxxx\",      (string) MN Public key\n"
            "  \"lastseen\": xxx,         (numeric) Time since epoch of last seen\n"
            "  \"activeseconds\": xxx,    (numeric) Seconds MN has been active\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("masternodecurrent", "") + HelpExampleRpc("masternodecurrent", ""));

    const auto tipIndex = WITH_LOCK(cs_main, return chainActive.Tip());
    CMasternode* winner = mnodeman.GetNextMasternodeInQueueForPayment(tipIndex);
    if (winner) {
        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("protocol", (int64_t)winner->protocolVersion));
        obj.push_back(Pair("txhash", winner->vin.prevout.hash.ToString()));
        obj.push_back(Pair("pubkey", EncodeDestination(winner->pubKeyCollateralAddress.GetID())));
        obj.push_back(Pair("lastseen", winner->lastPing.IsNull() ? winner->sigTime : (int64_t)winner->lastPing.sigTime));
        obj.push_back(Pair("activeseconds", winner->lastPing.IsNull() ? 0 : (int64_t)(winner->lastPing.sigTime - winner->sigTime)));
        return obj;
    }

    throw std::runtime_error("unknown");
}

bool StartMasternodeEntry(UniValue& statusObjRet, CMasternodeBroadcast& mnbRet, bool& fSuccessRet, const CMasternodeConfig::CMasternodeEntry& mne, std::string& errorMessage, std::string strCommand = "", std::string privkey = "")
{
    int nIndex;
    if(!mne.castOutputIndex(nIndex)) {
        return false;
    }

    CTxIn vin = CTxIn(uint256S(mne.getTxHash()), uint32_t(nIndex));
    CMasternode* pmn = mnodeman.Find(vin);
    if (pmn != NULL) {
        if (strCommand == "missing") return false;
        if (strCommand == "disabled" && pmn->IsEnabled()) return false;
    }

    fSuccessRet = CMasternodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnbRet, false, privkey);

    if (strCommand != "external") statusObjRet.push_back(Pair("alias", mne.getAlias()));
    statusObjRet.push_back(Pair("result", fSuccessRet ? "success" : "failed"));
    statusObjRet.push_back(Pair("error", fSuccessRet ? "" : errorMessage));

    return true;
}

void RelayMNB(CMasternodeBroadcast& mnb, const bool fSuccess, int& successful, int& failed)
{
    if (fSuccess) {
        successful++;
        mnodeman.UpdateMasternodeList(mnb);
        mnb.Relay();
    } else {
        failed++;
    }
}

void RelayMNB(CMasternodeBroadcast& mnb, const bool fSucces)
{
    int successful = 0, failed = 0;
    return RelayMNB(mnb, fSucces, successful, failed);
}

void SerializeMNB(UniValue& statusObjRet, const CMasternodeBroadcast& mnb, const bool fSuccess, int& successful, int& failed)
{
    if(fSuccess) {
        successful++;
        CDataStream ssMnb(SER_NETWORK, PROTOCOL_VERSION);
        ssMnb << mnb;
        statusObjRet.push_back(Pair("hex", HexStr(ssMnb.begin(), ssMnb.end())));
    } else {
        failed++;
    }
}

void SerializeMNB(UniValue& statusObjRet, const CMasternodeBroadcast& mnb, const bool fSuccess)
{
    int successful = 0, failed = 0;
    return SerializeMNB(statusObjRet, mnb, fSuccess, successful, failed);
}

UniValue reloadmasternodeconfig (const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() != 0))
        throw std::runtime_error(
            "reloadmasternodeconfig\n"
            "\nHot-reloads the masternode.conf file, adding and/or removing masternodes from the wallet at runtime.\n"

            "\nResult:\n"
            "{\n"
            "  \"success\": true|false, (boolean) Success status.\n"
            "  \"message\": \"xxx\"   (string) result message.\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("reloadmasternodeconfig", "") + HelpExampleRpc("reloadmasternodeconfig", ""));

    UniValue retObj(UniValue::VOBJ);

    // Remember the previous MN count (for comparison)
    auto mnconflock = GetBoolArg("-mnconflock", DEFAULT_MNCONFLOCK);
    auto& entries = masternodeConfig.getEntries();
    int prevCount = entries.size();

    // Creates a set with the outputs to unlock at the end of the method, and
    // Save the old entries to restore them in case of an error on the file
    std::set<COutPoint> outpointsToUnlock;
    std::vector<CMasternodeConfig::CMasternodeEntry> oldEntries;
    for (auto mne : entries) {
        if (mnconflock) {
            uint256 mnTxHash;
            mnTxHash.SetHex(mne.getTxHash());
            COutPoint outpoint = COutPoint(mnTxHash, (unsigned int)std::stoul(mne.getOutputIndex().c_str()));
            outpointsToUnlock.insert(outpoint);
        }
        oldEntries.push_back(mne);
    }

    // Clear the loaded config
    masternodeConfig.clear();
    // Load from disk
    std::string error;
    if (!masternodeConfig.read(error)) {
        // Failed
        retObj.push_back(Pair("success", false));
        retObj.push_back(Pair("message", "Error reloading masternode.conf, please fix it, " + error));

        outpointsToUnlock.clear();
    } else {
        // Success
        int newCount = masternodeConfig.getCount() + 1; // legacy preservation without showing a strange counting
        retObj.push_back(Pair("success", true));
        retObj.push_back(Pair("message", "Successfully reloaded from the masternode.conf file (Prev nodes: " + std::to_string(prevCount) + ", New nodes: " + std::to_string(newCount) + ")"));

        if (mnconflock) {
            for (auto& mne : entries) {
                uint256 mnTxHash;
                mnTxHash.SetHex(mne.getTxHash());
                COutPoint outpoint = COutPoint(mnTxHash, (unsigned int)std::stoul(mne.getOutputIndex().c_str()));
                pwalletMain->LockCoin(outpoint);
                outpointsToUnlock.erase(outpoint);
            }
        }
    }

    if (mnconflock) {
        for (auto outpoint : outpointsToUnlock) {
            pwalletMain->UnlockCoin(outpoint);
        }
    }

    return retObj;
}

UniValue startmasternode (const JSONRPCRequest& request)
{
    std::string strCommand;
    if (request.params.size() >= 1) {
        strCommand = request.params[0].get_str();

        // Backwards compatibility with legacy 'masternode' super-command forwarder
        if (strCommand == "start") strCommand = "local";
        if (strCommand == "start-alias") strCommand = "alias";
        if (strCommand == "start-all") strCommand = "all";
        if (strCommand == "start-many") strCommand = "many";
        if (strCommand == "start-missing") strCommand = "missing";
        if (strCommand == "start-disabled") strCommand = "disabled";
    }

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 3 ||
        (request.params.size() == 2 && (strCommand != "local" && strCommand != "all" && strCommand != "many" && strCommand != "missing" && strCommand != "disabled")) ||
        (request.params.size() == 3 && strCommand != "alias"))
        throw std::runtime_error(
            "startmasternode \"local|all|many|missing|disabled|alias\" lockwallet ( \"alias\" )\n"
            "\nAttempts to start one or more masternode(s)\n"

            "\nArguments:\n"
            "1. set         (string, required) Specify which set of masternode(s) to start.\n"
            "2. lockwallet  (boolean, required) Lock wallet after completion.\n"
            "3. alias       (string) Masternode alias. Required if using 'alias' as the set.\n"

            "\nResult: (for 'local' set):\n"
            "\"status\"     (string) Masternode status message\n"

            "\nResult: (for other sets):\n"
            "{\n"
            "  \"overall\": \"xxxx\",     (string) Overall status message\n"
            "  \"detail\": [\n"
            "    {\n"
            "      \"node\": \"xxxx\",    (string) Node name or alias\n"
            "      \"result\": \"xxxx\",  (string) 'success' or 'failed'\n"
            "      \"error\": \"xxxx\"    (string) Error message, if failed\n"
            "    }\n"
            "    ,...\n"
            "  ]\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("startmasternode", "\"alias\" \"0\" \"my_mn\"") + HelpExampleRpc("startmasternode", "\"alias\" \"0\" \"my_mn\""));

    bool fLock = (request.params[1].get_str() == "true" ? true : false);

    EnsureWalletIsUnlocked();

    if (strCommand == "local") {
        if (!fMasterNode) throw std::runtime_error("you must set masternode=1 in the configuration\n");

        UniValue resultsObj(UniValue::VARR);
        
        auto amns = amnodeman.GetActiveMasternodes();
        auto legacy = amns.size() == 1 && amns[0].strAlias == "legacy";

        for (auto& amn : amns) {

            UniValue mnObj(UniValue::VOBJ);

            if (amn.GetStatus() != ACTIVE_MASTERNODE_STARTED) {
                amn.ResetStatus();
            }

            if (amn.vin == nullopt) {
                mnObj.push_back(Pair("alias", amn.strAlias));
                mnObj.push_back(Pair("txhash", "N/A"));
                mnObj.push_back(Pair("outputidx", -1));
                mnObj.push_back(Pair("netaddr", amn.service.ToString()));
                mnObj.push_back(Pair("addr", "N/A"));
                mnObj.push_back(Pair("status", amn.GetStatus()));
                mnObj.push_back(Pair("message", amn.GetStatusMessage()));
                resultsObj.push_back(mnObj);
                if(legacy) break;
                continue;
            }

            CMasternode* pmn = mnodeman.Find(*(amn.vin));

            mnObj.push_back(Pair("alias", amn.strAlias));
            mnObj.push_back(Pair("txhash", amn.vin->prevout.hash.ToString()));
            mnObj.push_back(Pair("outputidx", (uint64_t)amn.vin->prevout.n));
            mnObj.push_back(Pair("netaddr", amn.service.ToString()));
            mnObj.push_back(Pair("addr", pmn ? EncodeDestination(pmn->pubKeyCollateralAddress.GetID()) : "N/A"));
            mnObj.push_back(Pair("status", amn.GetStatus()));
            mnObj.push_back(Pair("message", amn.GetStatusMessage()));
            resultsObj.push_back(mnObj);
            if (legacy) break;
        }

        if (fLock) pwalletMain->Lock();
        if(legacy) amns[0].GetStatusMessage();

        return resultsObj;
    }

    if (strCommand == "all" || strCommand == "many" || strCommand == "missing" || strCommand == "disabled") {
        if ((strCommand == "missing" || strCommand == "disabled") &&
            (masternodeSync.RequestedMasternodeAssets <= MASTERNODE_SYNC_LIST ||
                masternodeSync.RequestedMasternodeAssets == MASTERNODE_SYNC_FAILED)) {
            throw std::runtime_error("You can't use this command until masternode list is synced\n");
        }

        std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
        mnEntries = masternodeConfig.getEntries();

        int successful = 0;
        int failed = 0;

        UniValue resultsObj(UniValue::VARR);

        for (CMasternodeConfig::CMasternodeEntry mne : masternodeConfig.getEntries()) {
            UniValue statusObj(UniValue::VOBJ);
            CMasternodeBroadcast mnb;
            std::string errorMessage;
            bool fSuccess = false;
            if (StartMasternodeEntry(statusObj, mnb, fSuccess, mne, errorMessage, strCommand))
                RelayMNB(mnb, fSuccess, successful, failed);
            resultsObj.push_back(statusObj);
        }
        if (fLock)
            pwalletMain->Lock();

        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", strprintf("Successfully started %d masternodes, failed to start %d, total %d", successful, failed, successful + failed)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }

    if (strCommand == "alias") {
        std::string alias = request.params[2].get_str();

        bool found = false;

        UniValue resultsObj(UniValue::VARR);
        UniValue statusObj(UniValue::VOBJ);

        for (CMasternodeConfig::CMasternodeEntry mne : masternodeConfig.getEntries()) {
            if (mne.getAlias() == alias) {
                CMasternodeBroadcast mnb;
                found = true;
                std::string errorMessage;
                bool fSuccess = false;
                if (!StartMasternodeEntry(statusObj, mnb, fSuccess, mne, errorMessage, strCommand))
                        continue;
                RelayMNB(mnb, fSuccess);
                break;
            }
        }

        if (fLock)
            pwalletMain->Lock();

        if(!found) {
            statusObj.push_back(Pair("success", false));
            statusObj.push_back(Pair("error_message", "Could not find alias in config. Verify with listmasternodeconf."));
        }

        return statusObj;
    }
    return NullUniValue;
}

UniValue createmasternodekey (const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() != 0))
        throw std::runtime_error(
            "createmasternodekey\n"
            "\nCreate a new masternode private key\n"

            "\nResult:\n"
            "\"key\"    (string) Masternode private key\n"

            "\nExamples:\n" +
            HelpExampleCli("createmasternodekey", "") + HelpExampleRpc("createmasternodekey", ""));

    CKey secret;
    secret.MakeNewKey(false);

    return EncodeSecret(secret);
}

UniValue getmasternodeoutputs (const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() != 0))
        throw std::runtime_error(
            "getmasternodeoutputs\n"
            "\nPrint all masternode transaction outputs\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"txhash\": \"xxxx\",    (string) output transaction hash\n"
            "    \"outputidx\": n       (numeric) output index number\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n" +
            HelpExampleCli("getmasternodeoutputs", "") + HelpExampleRpc("getmasternodeoutputs", ""));

    // Find possible candidates
    std::vector<COutput> possibleCoins;
    pwalletMain->AvailableCoins(&possibleCoins, nullptr, ONLY_10000);

    UniValue ret(UniValue::VARR);
    for (COutput& out : possibleCoins) {
        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("txhash", out.tx->GetHash().ToString()));
        obj.push_back(Pair("outputidx", out.i));
        ret.push_back(obj);
    }

    return ret;
}

UniValue listmasternodeconf (const JSONRPCRequest& request)
{
    std::string strFilter = "";

    if (request.params.size() == 1) strFilter = request.params[0].get_str();

    if (request.fHelp || (request.params.size() > 1))
        throw std::runtime_error(
            "listmasternodeconf ( \"filter\" )\n"
            "\nPrint masternode.conf in JSON format\n"

            "\nArguments:\n"
            "1. \"filter\"    (string, optional) Filter search text. Partial match on alias, address, txHash, or status.\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"alias\": \"xxxx\",        (string) masternode alias\n"
            "    \"address\": \"xxxx\",      (string) masternode IP address\n"
            "    \"privateKey\": \"xxxx\",   (string) masternode private key\n"
            "    \"txHash\": \"xxxx\",       (string) transaction hash\n"
            "    \"outputIndex\": n,       (numeric) transaction output index\n"
            "    \"status\": \"xxxx\"        (string) masternode status\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n" +
            HelpExampleCli("listmasternodeconf", "") + HelpExampleRpc("listmasternodeconf", ""));

    std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
    mnEntries = masternodeConfig.getEntries();

    UniValue ret(UniValue::VARR);

    for (CMasternodeConfig::CMasternodeEntry mne : masternodeConfig.getEntries()) {
        int nIndex;
        if(!mne.castOutputIndex(nIndex))
            continue;
        CTxIn vin = CTxIn(uint256S(mne.getTxHash()), uint32_t(nIndex));
        CMasternode* pmn = mnodeman.Find(vin);

        std::string strStatus = pmn ? pmn->Status() : "MISSING";

        if (strFilter != "" && mne.getAlias().find(strFilter) == std::string::npos &&
            mne.getIp().find(strFilter) == std::string::npos &&
            mne.getTxHash().find(strFilter) == std::string::npos &&
            strStatus.find(strFilter) == std::string::npos) continue;

        UniValue mnObj(UniValue::VOBJ);
        mnObj.push_back(Pair("alias", mne.getAlias()));
        mnObj.push_back(Pair("address", mne.getIp()));
        mnObj.push_back(Pair("privateKey", mne.getPrivKey()));
        mnObj.push_back(Pair("txHash", mne.getTxHash()));
        mnObj.push_back(Pair("outputIndex", mne.getOutputIndex()));
        mnObj.push_back(Pair("status", strStatus));
        ret.push_back(mnObj);
    }

    return ret;
}

UniValue getactivemasternodecount (const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() > 0))
        throw std::runtime_error(
            "getactivemasternodecount\n"
            "\nGet active masternode count values\n"

            "\nResult:\n"
            "{\n"
            "  \"total\": n,        (numeric) Total masternodes\n"
            "  \"initial\": n,      (numeric) Initial state masternodes\n"
            "  \"syncing\": n,      (numeric) Syncing masternodes\n"
            "  \"not_capable\": n,  (numeric) Not capable masternodes\n"
            "  \"started\": n,      (numeric) Started masternodes\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getactivemasternodecount", "") + HelpExampleRpc("getactivemasternodecount", ""));

    if (!fMasterNode)
        throw JSONRPCError(RPC_MISC_ERROR, _("This is not a masternode."));

    int total = 0;
    int initial = 0;
    int syncing = 0;
    int not_capable = 0;
    int started = 0;
    
    for (auto& amn : amnodeman.GetActiveMasternodes()) {
        switch(amn.GetStatus()) {
            case ACTIVE_MASTERNODE_INITIAL:
                initial++;
                break;
            case ACTIVE_MASTERNODE_SYNC_IN_PROCESS:
                syncing++;
                break;
            case ACTIVE_MASTERNODE_NOT_CAPABLE:
                not_capable++;
                break;
            case ACTIVE_MASTERNODE_STARTED:
                started++;
                break;
        }
        total++;
    }

    UniValue obj(UniValue::VOBJ);

    obj.push_back(Pair("total", total));
    obj.push_back(Pair("initial", initial));
    obj.push_back(Pair("syncing", syncing));
    obj.push_back(Pair("not_capable", not_capable));
    obj.push_back(Pair("started", started));

    return obj;
}

UniValue getmasternodestatus(const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() != 0))
        throw std::runtime_error(
            "getmasternodestatus\n"
            "\nPrint masternode status\n"

            "\nResult:\n"
            "{\n"
            "  \"txhash\": \"xxxx\",      (string) Collateral transaction hash\n"
            "  \"outputidx\": n,          (numeric) Collateral transaction output index number\n"
            "  \"netaddr\": \"xxxx\",     (string) Masternode network address\n"
            "  \"addr\": \"xxxx\",        (string) __DSW__ address for masternode payments\n"
            "  \"status\": \"xxxx\",      (string) Masternode status\n"
            "  \"message\": \"xxxx\"      (string) Masternode status message\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("getmasternodestatus", "") + HelpExampleRpc("getmasternodestatus", ""));

    if (!fMasterNode)
        throw JSONRPCError(RPC_MISC_ERROR, _("This is not a masternode."));

    UniValue resultsObj(UniValue::VARR);

    auto amns = amnodeman.GetActiveMasternodes();
    auto legacy = amns.size() == 1 && amns[0].strAlias == "legacy";

    for (auto& amn : amns) {

        if (amn.vin == nullopt) {
            UniValue mnObj(UniValue::VOBJ);
            mnObj.push_back(Pair("alias", amn.strAlias));
            mnObj.push_back(Pair("txhash", "N/A"));
            mnObj.push_back(Pair("outputidx", -1));
            mnObj.push_back(Pair("netaddr", amn.service.ToString()));
            mnObj.push_back(Pair("addr", "N/A"));
            mnObj.push_back(Pair("status", amn.GetStatus()));
            mnObj.push_back(Pair("message", amn.GetStatusMessage()));
            resultsObj.push_back(mnObj);
            if(legacy) return mnObj;
            continue;
        }

        CMasternode* pmn = mnodeman.Find(*(amn.vin));

        UniValue mnObj(UniValue::VOBJ);
        mnObj.push_back(Pair("alias", amn.strAlias));
        mnObj.push_back(Pair("txhash", amn.vin->prevout.hash.ToString()));
        mnObj.push_back(Pair("outputidx", (uint64_t)amn.vin->prevout.n));
        mnObj.push_back(Pair("netaddr", amn.service.ToString()));
        mnObj.push_back(Pair("addr", pmn ? EncodeDestination(pmn->pubKeyCollateralAddress.GetID()) : "N/A"));
        mnObj.push_back(Pair("status", amn.GetStatus()));
        mnObj.push_back(Pair("message", amn.GetStatusMessage()));
        if(legacy) return mnObj;
        resultsObj.push_back(mnObj);
    }

    return resultsObj;
}

bool DecodeHexMnb(CMasternodeBroadcast& mnb, std::string strHexMnb) {

    if (!IsHex(strHexMnb))
        return false;

    std::vector<unsigned char> mnbData(ParseHex(strHexMnb));
    CDataStream ssData(mnbData, SER_NETWORK, PROTOCOL_VERSION);
    try {
        ssData >> mnb;
    }
    catch (const std::exception&) {
        return false;
    }

    return true;
}

UniValue createmasternodebroadcast(const JSONRPCRequest& request)
{
    // wait for reindex and/or import to finish
    if (fImporting || fReindex)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Wait for reindex and/or import to finish");

    std::string strCommand;
    if (request.params.size() >= 1)
        strCommand = request.params[0].get_str();
    if (request.fHelp || 
        (strCommand != "alias" && strCommand != "all" && strCommand != "external") || 
        (strCommand == "alias" && request.params.size() < 2) ||
        (strCommand == "external" && request.params.size() < 6)
    ) {
        throw std::runtime_error(
            "createmasternodebroadcast \"command\" ( \"alias\")\n"
            "\nCreates a masternode broadcast message for one or all masternodes configured in masternode.conf or for an external one\n" +
            HelpRequiringPassphrase() + "\n"

            "\nArguments:\n"
            "1. \"command\"                 (string, required)\n" 
            "                                   \"alias\" for single masternode, \"all\" for all masternodes\n"
            "                                   \"external\" for starting an external masternode\n"
            "2. \"alias\"                   (string, required if command is \"alias\") Alias of the masternode\n"
            "   \"ip:port\"                 (string, required if command is \"external\") masternode's IP address and port (port:default)\n"
            "3. \"masternodeprivkey\"       (string, required if command is \"external\") operator masternode key\n"
            "4. \"collateral_output_txid\"  (string, required if command is \"external\") collateral's transaction id\n"
            "5. \"collateral_output_index\" (string, required if command is \"external\") collateral's output index\n"
            "6. \"collateral_privkey\"      (string, required if command is \"external\") collateral's private key\n"
            
            "\nResult (all):\n"
            "{\n"
            "  \"overall\": \"xxx\",        (string) Overall status message indicating number of successes.\n"
            "  \"detail\": [                (array) JSON array of broadcast objects.\n"
            "    {\n"
            "      \"alias\": \"xxx\",      (string) Alias of the masternode.\n"
            "      \"success\": true|false, (boolean) Success status.\n"
            "      \"hex\": \"xxx\"         (string, if success=true) Hex encoded broadcast message.\n"
            "      \"error_message\": \"xxx\"   (string, if success=false) Error message, if any.\n"
            "    }\n"
            "    ,...\n"
            "  ]\n"
            "}\n"

            "\nResult (alias):\n"
            "{\n"
            "  \"alias\": \"xxx\",      (string) Alias of the masternode.\n"
            "  \"success\": true|false, (boolean) Success status.\n"
            "  \"hex\": \"xxx\"         (string, if success=true) Hex encoded broadcast message.\n"
            "  \"error_message\": \"xxx\"   (string, if success=false) Error message, if any.\n"
            "}\n"

            "\nResult (external):\n"
            "{\n"
            "  \"success\": true|false, (boolean) Success status.\n"
            "  \"hex\": \"xxx\"         (string, if success=true) Hex encoded broadcast message.\n"
            "  \"error_message\": \"xxx\"   (string, if success=false) Error message, if any.\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("createmasternodebroadcast", "alias mymn1") + HelpExampleRpc("createmasternodebroadcast", "alias mymn1"));
    }

    if (strCommand == "alias" || strCommand == "all") EnsureWalletIsUnlocked();

    if (strCommand == "alias")
    {
        std::string alias = request.params[1].get_str();
        bool found = false;

        UniValue statusObj(UniValue::VOBJ);
        statusObj.push_back(Pair("alias", alias));

        for (CMasternodeConfig::CMasternodeEntry mne : masternodeConfig.getEntries()) {
            if(mne.getAlias() == alias) {
                CMasternodeBroadcast mnb;
                found = true;
                std::string errorMessage;
                bool fSuccess = false;
                if (!StartMasternodeEntry(statusObj, mnb, fSuccess, mne, errorMessage, strCommand))
                        continue;
                SerializeMNB(statusObj, mnb, fSuccess);
                break;
            }
        }

        if(!found) {
            statusObj.push_back(Pair("success", false));
            statusObj.push_back(Pair("error_message", "Could not find alias in config. Verify with listmasternodeconf."));
        }

        return statusObj;
    }

    if (strCommand == "all")
    {
        std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
        mnEntries = masternodeConfig.getEntries();

        int successful = 0;
        int failed = 0;

        UniValue resultsObj(UniValue::VARR);

        for (CMasternodeConfig::CMasternodeEntry mne : masternodeConfig.getEntries()) {
            UniValue statusObj(UniValue::VOBJ);
            CMasternodeBroadcast mnb;
            std::string errorMessage;
            bool fSuccess = false;
            if (!StartMasternodeEntry(statusObj, mnb, fSuccess, mne, errorMessage, strCommand))
                    continue;
            SerializeMNB(statusObj, mnb, fSuccess, successful, failed);
            resultsObj.push_back(statusObj);
        }

        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", strprintf("Successfully created broadcast messages for %d masternodes, failed to create %d, total %d", successful, failed, successful + failed)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }

    if (strCommand == "external")
    {
        auto ip                 = request.params[1].get_str();
        auto masternodeprivkey  = request.params[2].get_str();
        auto txid               = request.params[3].get_str();
        auto n                  = request.params[4].get_str();
        auto privkey            = request.params[5].get_str();

        UniValue statusObj(UniValue::VOBJ);

        CMasternodeConfig::CMasternodeEntry mne("external", ip, masternodeprivkey, txid, n);
        CMasternodeBroadcast mnb;
        std::string errorMessage;
        bool fSuccess = false;
        if (StartMasternodeEntry(statusObj, mnb, fSuccess, mne, errorMessage, strCommand, privkey)) {
            SerializeMNB(statusObj, mnb, fSuccess);
        }

        return statusObj;
    }

    return NullUniValue;
}

UniValue decodemasternodebroadcast(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "decodemasternodebroadcast \"hexstring\"\n"
            "\nCommand to decode masternode broadcast messages\n"

            "\nArgument:\n"
            "1. \"hexstring\"        (string) The hex encoded masternode broadcast message\n"

            "\nResult:\n"
            "{\n"
            "  \"vin\": \"xxxx\"                (string) The unspent output which is holding the masternode collateral\n"
            "  \"addr\": \"xxxx\"               (string) IP address of the masternode\n"
            "  \"pubkeycollateral\": \"xxxx\"   (string) Collateral address's public key\n"
            "  \"pubkeymasternode\": \"xxxx\"   (string) Masternode's public key\n"
            "  \"vchsig\": \"xxxx\"             (string) Base64-encoded signature of this message (verifiable via pubkeycollateral)\n"
            "  \"sigtime\": \"nnn\"             (numeric) Signature timestamp\n"
            "  \"sigvalid\": \"xxx\"            (string) \"true\"/\"false\" whether or not the mnb signature checks out.\n"
            "  \"protocolversion\": \"nnn\"     (numeric) Masternode's protocol version\n"
            "  \"nlastdsq\": \"nnn\"            (numeric) The last time the masternode sent a DSQ message (for mixing) (DEPRECATED)\n"
            "  \"nMessVersion\": \"nnn\"        (numeric) MNB Message version number\n"
            "  \"lastping\" : {                 (object) JSON object with information about the masternode's last ping\n"
            "      \"vin\": \"xxxx\"            (string) The unspent output of the masternode which is signing the message\n"
            "      \"blockhash\": \"xxxx\"      (string) Current chaintip blockhash minus 12\n"
            "      \"sigtime\": \"nnn\"         (numeric) Signature time for this ping\n"
            "      \"sigvalid\": \"xxx\"        (string) \"true\"/\"false\" whether or not the mnp signature checks out.\n"
            "      \"vchsig\": \"xxxx\"         (string) Base64-encoded signature of this ping (verifiable via pubkeymasternode)\n"
            "      \"nMessVersion\": \"nnn\"    (numeric) MNP Message version number\n"
            "  }\n"
            "}\n"

            "\nExamples:\n" +
            HelpExampleCli("decodemasternodebroadcast", "hexstring") + HelpExampleRpc("decodemasternodebroadcast", "hexstring"));

    CMasternodeBroadcast mnb;

    if (!DecodeHexMnb(mnb, request.params[0].get_str()))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Masternode broadcast message decode failed");

    UniValue resultObj(UniValue::VOBJ);

    resultObj.push_back(Pair("vin", mnb.vin.prevout.ToString()));
    resultObj.push_back(Pair("addr", mnb.addr.ToString()));
    resultObj.push_back(Pair("pubkeycollateral", EncodeDestination(mnb.pubKeyCollateralAddress.GetID())));
    resultObj.push_back(Pair("pubkeymasternode", EncodeDestination(mnb.pubKeyMasternode.GetID())));
    resultObj.push_back(Pair("vchsig", mnb.GetSignatureBase64()));
    resultObj.push_back(Pair("sigtime", mnb.sigTime));
    resultObj.push_back(Pair("sigvalid", mnb.CheckSignature() ? "true" : "false"));
    resultObj.push_back(Pair("protocolversion", mnb.protocolVersion));
    resultObj.push_back(Pair("nlastdsq", mnb.nLastDsq));
    resultObj.push_back(Pair("nMessVersion", mnb.nMessVersion));

    UniValue lastPingObj(UniValue::VOBJ);
    lastPingObj.push_back(Pair("vin", mnb.lastPing.vin.prevout.ToString()));
    lastPingObj.push_back(Pair("blockhash", mnb.lastPing.blockHash.ToString()));
    lastPingObj.push_back(Pair("sigtime", mnb.lastPing.sigTime));
    lastPingObj.push_back(Pair("sigvalid", mnb.lastPing.CheckSignature(mnb.pubKeyMasternode) ? "true" : "false"));
    lastPingObj.push_back(Pair("vchsig", mnb.lastPing.GetSignatureBase64()));
    lastPingObj.push_back(Pair("nMessVersion", mnb.lastPing.nMessVersion));

    resultObj.push_back(Pair("lastping", lastPingObj));

    return resultObj;
}

UniValue relaymasternodebroadcast(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "relaymasternodebroadcast \"hexstring\"\n"
            "\nCommand to relay masternode broadcast messages\n"

            "\nArguments:\n"
            "1. \"hexstring\"        (string) The hex encoded masternode broadcast message\n"

            "\nExamples:\n" +
            HelpExampleCli("relaymasternodebroadcast", "hexstring") + HelpExampleRpc("relaymasternodebroadcast", "hexstring"));


    CMasternodeBroadcast mnb;

    if (!DecodeHexMnb(mnb, request.params[0].get_str()))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Masternode broadcast message decode failed");

    if(!mnb.CheckSignature())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Masternode broadcast signature verification failed");

    mnodeman.UpdateMasternodeList(mnb);
    mnb.Relay();

    return strprintf("Masternode broadcast sent (service %s, vin %s)", mnb.addr.ToString(), mnb.vin.ToString());
}
