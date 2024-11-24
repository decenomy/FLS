// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "masternodeman.h"

#include "addrman.h"
#include "fs.h"
#include "masternode-payments.h"
#include "masternode-sync.h"
#include "masternode.h"
#include "messagesigner.h"
#include "netbase.h"
#include "netmessagemaker.h"
#include "spork.h"
#include "util.h"

#include <boost/thread/thread.hpp>

#define MN_WINNER_MINIMUM_AGE 8000    // Age in seconds. This should be > MASTERNODE_REMOVAL_SECONDS to avoid misconfigured new nodes in the list.

/** Masternode manager */
CMasternodeMan mnodeman;
/** Keep track of the active Masternode */
CActiveMasternodeMan amnodeman;

struct CompareLastPaid {
    bool operator()(const std::pair<int64_t, CTxIn>& t1,
        const std::pair<int64_t, CTxIn>& t2) const
    {
        return t1.first < t2.first;
    }
};

struct CompareScoreTxIn {
    bool operator()(const std::pair<int64_t, CTxIn>& t1,
        const std::pair<int64_t, CTxIn>& t2) const
    {
        return t1.first < t2.first;
    }
};

struct CompareScoreMN {
    bool operator()(const std::pair<int64_t, CMasternode>& t1,
        const std::pair<int64_t, CMasternode>& t2) const
    {
        return t1.first < t2.first;
    }
};

//
// CMasternodeDB
//

CMasternodeDB::CMasternodeDB()
{
    pathMN = GetDataDir() / "mncache.dat";
    strMagicMessage = "MasternodeCache";
}

bool CMasternodeDB::Write(const CMasternodeMan& mnodemanToSave)
{
    int64_t nStart = GetTimeMillis();

    // serialize, checksum data up to that point, then append checksum
    CDataStream ssMasternodes(SER_DISK, CLIENT_VERSION);
    ssMasternodes << strMagicMessage;                   // masternode cache file specific magic message
    ssMasternodes << FLATDATA(Params().MessageStart()); // network specific magic number
    ssMasternodes << mnodemanToSave;
    uint256 hash = Hash(ssMasternodes.begin(), ssMasternodes.end());
    ssMasternodes << hash;

    // open output file, and associate with CAutoFile
    FILE* file = fsbridge::fopen(pathMN, "wb");
    CAutoFile fileout(file, SER_DISK, CLIENT_VERSION);
    if (fileout.IsNull())
        return error("%s : Failed to open file %s", __func__, pathMN.string());

    // Write and commit header, data
    try {
        fileout << ssMasternodes;
    } catch (const std::exception& e) {
        return error("%s : Serialize or I/O error - %s", __func__, e.what());
    }
    //    FileCommit(fileout);
    fileout.fclose();

    LogPrint(BCLog::MASTERNODE,"Written info to mncache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrint(BCLog::MASTERNODE,"  %s\n", mnodemanToSave.ToString());

    return true;
}

CMasternodeDB::ReadResult CMasternodeDB::Read(CMasternodeMan& mnodemanToLoad, bool fDryRun)
{
    int64_t nStart = GetTimeMillis();
    // open input file, and associate with CAutoFile
    FILE* file = fsbridge::fopen(pathMN, "rb");
    CAutoFile filein(file, SER_DISK, CLIENT_VERSION);
    if (filein.IsNull()) {
        error("%s : Failed to open file %s", __func__, pathMN.string());
        return FileError;
    }

    // use file size to size memory buffer
    int fileSize = fs::file_size(pathMN);
    int dataSize = fileSize - sizeof(uint256);
    // Don't try to resize to a negative number if file is small
    if (dataSize < 0)
        dataSize = 0;
    std::vector<unsigned char> vchData;
    vchData.resize(dataSize);
    uint256 hashIn;

    // read data and checksum from file
    try {
        filein.read((char*)&vchData[0], dataSize);
        filein >> hashIn;
    } catch (const std::exception& e) {
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return HashReadError;
    }
    filein.fclose();

    CDataStream ssMasternodes(vchData, SER_DISK, CLIENT_VERSION);

    // verify stored checksum matches input data
    uint256 hashTmp = Hash(ssMasternodes.begin(), ssMasternodes.end());
    if (hashIn != hashTmp) {
        error("%s : Checksum mismatch, data corrupted", __func__);
        return IncorrectHash;
    }

    unsigned char pchMsgTmp[4];
    std::string strMagicMessageTmp;
    try {
        // de-serialize file header (masternode cache file specific magic message) and ..

        ssMasternodes >> strMagicMessageTmp;

        // ... verify the message matches predefined one
        if (strMagicMessage != strMagicMessageTmp) {
            error("%s : Invalid masternode cache magic message", __func__);
            return IncorrectMagicMessage;
        }

        // de-serialize file header (network specific magic number) and ..
        ssMasternodes >> FLATDATA(pchMsgTmp);

        // ... verify the network matches ours
        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp))) {
            error("%s : Invalid network magic number", __func__);
            return IncorrectMagicNumber;
        }
        // de-serialize data into CMasternodeMan object
        ssMasternodes >> mnodemanToLoad;
    } catch (const std::exception& e) {
        mnodemanToLoad.Clear();
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return IncorrectFormat;
    }

    LogPrint(BCLog::MASTERNODE,"Loaded info from mncache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrint(BCLog::MASTERNODE,"  %s\n", mnodemanToLoad.ToString());
    if (!fDryRun) {
        LogPrint(BCLog::MASTERNODE,"Masternode manager - cleaning....\n");
        mnodemanToLoad.CheckAndRemove(true);
        LogPrint(BCLog::MASTERNODE,"Masternode manager - result:\n");
        LogPrint(BCLog::MASTERNODE,"  %s\n", mnodemanToLoad.ToString());
    }

    return Ok;
}

void DumpMasternodes()
{
    int64_t nStart = GetTimeMillis();

    CMasternodeDB mndb;
    CMasternodeMan tempMnodeman;

    LogPrint(BCLog::MASTERNODE,"Verifying mncache.dat format...\n");
    CMasternodeDB::ReadResult readResult = mndb.Read(tempMnodeman, true);
    // there was an error and it was not an error on file opening => do not proceed
    if (readResult == CMasternodeDB::FileError)
        LogPrint(BCLog::MASTERNODE,"Missing masternode cache file - mncache.dat, will try to recreate\n");
    else if (readResult != CMasternodeDB::Ok) {
        LogPrint(BCLog::MASTERNODE,"Error reading mncache.dat: ");
        if (readResult == CMasternodeDB::IncorrectFormat)
            LogPrint(BCLog::MASTERNODE,"magic is ok but data has invalid format, will try to recreate\n");
        else {
            LogPrint(BCLog::MASTERNODE,"file format is unknown or invalid, please fix it manually\n");
            return;
        }
    }
    LogPrint(BCLog::MASTERNODE,"Writting info to mncache.dat...\n");
    mndb.Write(mnodeman);

    LogPrint(BCLog::MASTERNODE,"Masternode dump finished  %dms\n", GetTimeMillis() - nStart);
}

CMasternodeMan::CMasternodeMan()
{
    nDsqCount = 0;
}

bool CMasternodeMan::Add(CMasternode& mn)
{
    LOCK(cs);

    if (!mn.IsEnabled())
        return false;

    CMasternode* pmn = Find(mn.vin);
    CMasternode* pmnByAddr = Find(mn.addr);
    bool masternodeRankV2 = Params().GetConsensus().NetworkUpgradeActive(chainActive.Height(), Consensus::UPGRADE_MASTERNODE_RANK_V2);

    auto mnScript = Find(GetScriptForDestination(mn.pubKeyCollateralAddress.GetID()));
    if(mnScript) {
        auto it = std::find(vMasternodes.begin(), vMasternodes.end(), mnScript);
        if(it != vMasternodes.end()) vMasternodes.erase(it);

        return false;
    }

    if (pmn == NULL && 
        (sporkManager.IsSporkActive(SPORK_111_ALLOW_DUPLICATE_MN_IPS) || !masternodeRankV2 || pmnByAddr == NULL) 
    ) {
        LogPrint(BCLog::MASTERNODE, "CMasternodeMan: Adding new Masternode %s - count %i now\n", mn.vin.prevout.ToStringShort(), size() + 1);
        auto m = new CMasternode(mn);
        vMasternodes.push_back(m);
        {
            LOCK(cs_script);
            mapScriptMasternodes[GetScriptForDestination(m->pubKeyCollateralAddress.GetID())] = m;
        }
        {
            LOCK(cs_txin);
            mapTxInMasternodes[m->vin] = m;
        }
        {
            LOCK(cs_pubkey);
            mapPubKeyMasternodes[m->pubKeyMasternode] = m;
        }
        return true;
    }

    return false;
}

void CMasternodeMan::AskForMN(CNode* pnode, const CTxIn& vin)
{
    std::map<COutPoint, int64_t>::iterator i = mWeAskedForMasternodeListEntry.find(vin.prevout);
    if (i != mWeAskedForMasternodeListEntry.end()) {
        int64_t t = (*i).second;
        if (GetTime() < t) return; // we've asked recently
    }

    // ask for the mnb info once from the node that sent mnp

    LogPrint(BCLog::MASTERNODE, "CMasternodeMan::AskForMN - Asking node for missing entry, vin: %s\n", vin.prevout.ToStringShort());
    g_connman->PushMessage(pnode, CNetMsgMaker(pnode->GetSendVersion()).Make(NetMsgType::GETMNLIST, vin));
    int64_t askAgain = GetTime() + MASTERNODE_MIN_MNP_SECONDS;
    mWeAskedForMasternodeListEntry[vin.prevout] = askAgain;
}

void CMasternodeMan::Check(bool forceCheck)
{
    if(forceCheck) {
        LOCK2(cs_main, cs);

        for (auto mn : vMasternodes) {
            mn->Check(forceCheck);
        }
    } else {
        LOCK(cs);

        for (auto mn : vMasternodes) {
            mn->Check();
        }
    }
}

void CMasternodeMan::CheckAndRemove(bool forceExpiredRemoval)
{
    Check(true);

    LOCK(cs);

    //remove inactive and outdated
    auto it = vMasternodes.begin();
    while (it != vMasternodes.end()) {
        if ((**it).activeState == CMasternode::MASTERNODE_REMOVE ||
            (**it).activeState == CMasternode::MASTERNODE_VIN_SPENT ||
            (forceExpiredRemoval && (**it).activeState == CMasternode::MASTERNODE_EXPIRED)) {
            LogPrint(BCLog::MASTERNODE, "CMasternodeMan: Removing inactive Masternode %s - %i now\n", (**it).vin.prevout.ToStringShort(), size() - 1);

            //erase all of the broadcasts we've seen from this vin
            // -- if we missed a few pings and the node was removed, this will allow is to get it back without them
            //    sending a brand new mnb
            std::map<uint256, CMasternodeBroadcast>::iterator it3 = mapSeenMasternodeBroadcast.begin();
            while (it3 != mapSeenMasternodeBroadcast.end()) {
                if ((*it3).second.vin == (**it).vin) {
                    masternodeSync.mapSeenSyncMNB.erase((*it3).first);
                    mapSeenMasternodeBroadcast.erase(it3++);
                } else {
                    ++it3;
                }
            }

            // allow us to ask for this masternode again if we see another ping
            std::map<COutPoint, int64_t>::iterator it2 = mWeAskedForMasternodeListEntry.begin();
            while (it2 != mWeAskedForMasternodeListEntry.end()) {
                if ((*it2).first == (**it).vin.prevout) {
                    mWeAskedForMasternodeListEntry.erase(it2++);
                } else {
                    ++it2;
                }
            }

            {
                LOCK(cs_script);
                mapScriptMasternodes.erase(GetScriptForDestination((*it)->pubKeyCollateralAddress.GetID()));
            }
            {
                LOCK(cs_txin);
                mapTxInMasternodes.erase((*it)->vin);
            }
            {
                LOCK(cs_pubkey);
                mapPubKeyMasternodes.erase((*it)->pubKeyMasternode);
            }
            delete *it;
            it = vMasternodes.erase(it);
        } else {
            ++it;
        }
    }

    // check who's asked for the Masternode list
    std::map<CNetAddr, int64_t>::iterator it1 = mAskedUsForMasternodeList.begin();
    while (it1 != mAskedUsForMasternodeList.end()) {
        if ((*it1).second < GetTime()) {
            mAskedUsForMasternodeList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check who we asked for the Masternode list
    it1 = mWeAskedForMasternodeList.begin();
    while (it1 != mWeAskedForMasternodeList.end()) {
        if ((*it1).second < GetTime()) {
            mWeAskedForMasternodeList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check which Masternodes we've asked for
    std::map<COutPoint, int64_t>::iterator it2 = mWeAskedForMasternodeListEntry.begin();
    while (it2 != mWeAskedForMasternodeListEntry.end()) {
        if ((*it2).second < GetTime()) {
            mWeAskedForMasternodeListEntry.erase(it2++);
        } else {
            ++it2;
        }
    }

    // remove expired mapSeenMasternodeBroadcast
    std::map<uint256, CMasternodeBroadcast>::iterator it3 = mapSeenMasternodeBroadcast.begin();
    while (it3 != mapSeenMasternodeBroadcast.end()) {
        if ((*it3).second.lastPing.sigTime < GetTime() - (MASTERNODE_REMOVAL_SECONDS * 2)) {
            mapSeenMasternodeBroadcast.erase(it3++);
            masternodeSync.mapSeenSyncMNB.erase((*it3).second.GetHash());
        } else {
            ++it3;
        }
    }

    // remove expired mapSeenMasternodePing
    std::map<uint256, CMasternodePing>::iterator it4 = mapSeenMasternodePing.begin();
    while (it4 != mapSeenMasternodePing.end()) {
        if ((*it4).second.sigTime < GetTime() - (MASTERNODE_REMOVAL_SECONDS * 2)) {
            mapSeenMasternodePing.erase(it4++);
        } else {
            ++it4;
        }
    }
}

void CMasternodeMan::Clear()
{
    {
        LOCK(cs_script);
        mapScriptMasternodes.clear();
    }
    {
        LOCK(cs_txin);
        mapTxInMasternodes.clear();
    }
    {
        LOCK(cs_pubkey);
        mapPubKeyMasternodes.clear();
    }

    LOCK(cs);
    auto it = vMasternodes.begin();
    while (it != vMasternodes.end()) {
        delete *it;
        it = vMasternodes.erase(it);
    }
    mAskedUsForMasternodeList.clear();
    mWeAskedForMasternodeList.clear();
    mWeAskedForMasternodeListEntry.clear();
    mapSeenMasternodeBroadcast.clear();
    mapSeenMasternodePing.clear();
    nDsqCount = 0;
}

int CMasternodeMan::stable_size ()
{
    int nStable_size = 0;
    int64_t nMasternode_Min_Age = MN_WINNER_MINIMUM_AGE;
    int64_t nMasternode_Age = 0;

    LOCK2(cs_main, cs);

    for (auto mn : vMasternodes) {
        if (sporkManager.IsSporkActive(SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT) &&
            sporkManager.IsSporkActive(SPORK_108_FORCE_MASTERNODE_MIN_AGE)) 
        {
            nMasternode_Age = GetAdjustedTime() - mn->sigTime;
            if ((nMasternode_Age) < nMasternode_Min_Age) {
                continue; // Skip masternodes younger than (default) 8000 sec (MUST be > MASTERNODE_REMOVAL_SECONDS)
            }
        }
        mn->Check ();
        if (!mn->IsEnabled ())
            continue; // Skip not-enabled masternodes

        nStable_size++;
    }

    return nStable_size;
}

int CMasternodeMan::CountEnabled()
{
    int i = 0;

    LOCK(cs);

    for (auto mn : vMasternodes) {
        mn->Check();
        if (!mn->IsEnabled()) continue;
        i++;
    }

    return i;
}

void CMasternodeMan::CountNetworks(int& ipv4, int& ipv6, int& onion)
{
    LOCK(cs);

    for (auto mn : vMasternodes) {
        mn->Check();
        std::string strHost;
        int port;
        SplitHostPort(mn->addr.ToString(), port, strHost);
        CNetAddr node;
        LookupHost(strHost.c_str(), node, false);
        int nNetwork = node.GetNetwork();
        switch (nNetwork) {
            case 1 :
                ipv4++;
                break;
            case 2 :
                ipv6++;
                break;
            case 3 :
                onion++;
                break;
        }
    }
}

void CMasternodeMan::DsegUpdate(CNode* pnode)
{
    LOCK(cs);

    if (Params().NetworkID() == CBaseChainParams::MAIN) {
        if (!(pnode->addr.IsRFC1918() || pnode->addr.IsLocal())) {
            std::map<CNetAddr, int64_t>::iterator it = mWeAskedForMasternodeList.find(pnode->addr);
            if (it != mWeAskedForMasternodeList.end()) {
                if (GetTime() < (*it).second) {
                    LogPrint(BCLog::MASTERNODE, "dseg - we already asked peer %i for the list; skipping...\n", pnode->GetId());
                    return;
                }
            }
        }
    }

    g_connman->PushMessage(pnode, CNetMsgMaker(pnode->GetSendVersion()).Make(NetMsgType::GETMNLIST, CTxIn()));
    int64_t askAgain = GetTime() + MASTERNODES_DSEG_SECONDS;
    mWeAskedForMasternodeList[pnode->addr] = askAgain;
}

CMasternode* CMasternodeMan::Find(const CScript& payee)
{
    LOCK(cs_script);

    auto it = mapScriptMasternodes.find(payee);
    if (it != mapScriptMasternodes.end())
        return it->second;

    return NULL;
}

CMasternode* CMasternodeMan::Find(const CTxIn& vin)
{
    LOCK(cs_txin);

    auto it = mapTxInMasternodes.find(vin);
    if (it != mapTxInMasternodes.end())
        return it->second;

    return NULL;
}


CMasternode* CMasternodeMan::Find(const CPubKey& pubKeyMasternode)
{
    LOCK(cs_pubkey);

    auto it = mapPubKeyMasternodes.find(pubKeyMasternode);
    if (it != mapPubKeyMasternodes.end())
        return it->second;

    return NULL;
}

CMasternode* CMasternodeMan::Find(const CService &addr)
{
    LOCK(cs);

    for (auto mn : vMasternodes) {
        if (mn->addr.ToStringIP() == addr.ToStringIP())
            return mn;
    }
    return NULL;
}

//
// Deterministically select the oldest/best masternode to pay on the network
//
CMasternode* CMasternodeMan::GetNextMasternodeInQueueForPayment(int nBlockHeight, bool fFilterSigTime, int& nCount, std::vector<CTxIn>& vEligibleTxIns, bool fJustCount, bool fCleanLastPaid)
{

    CMasternode* pBestMasternode = nullptr;

    /*
        Make a vector with all of the last paid times
    */

    std::vector<std::pair<int64_t, CTxIn>> vecMasternodeLastPaid;
    vEligibleTxIns.clear();
    int nMnCount = 0;
    {
        LOCK(cs);

        nMnCount = CountEnabled();
        for (auto mn : vMasternodes) {
            mn->Check();
            if (!mn->IsEnabled()) continue;

            //it's in the list (up to 8 entries ahead of current block to allow propagation) -- so let's skip it
            if (masternodePayments.IsScheduled(*mn, nBlockHeight)) continue;

            //it's too new, wait for a cycle
            if (Params().GetConsensus().NetworkUpgradeActive(chainActive.Tip()->nHeight, Consensus::UPGRADE_STAKE_MODIFIER_V2)) {
                if (fFilterSigTime && mn->sigTime + (nMnCount * 60) > GetAdjustedTime()) continue;
            } else {
                if (fFilterSigTime && mn->sigTime + (nMnCount * 2.6 * 60) > GetAdjustedTime()) continue;
            }

            //make sure it has as many confirmations as there are masternodes
            if (!sporkManager.IsSporkActive(SPORK_107_IGNORE_COLLATERAL_CONFIRMATIONS)) {
                if (pcoinsTip->GetCoinDepthAtHeight(mn->vin.prevout, nBlockHeight) < nMnCount) continue;
            }

            vecMasternodeLastPaid.push_back(std::make_pair(mn->SecondsSincePayment(chainActive[nBlockHeight - 1]), mn->vin));
        }
    }

    nCount = (int)vecMasternodeLastPaid.size();

    //when the network is in the process of upgrading, don't penalize nodes that recently restarted
    if (fFilterSigTime && nCount < nMnCount / 3) return GetNextMasternodeInQueueForPayment(nBlockHeight, false, nCount, vEligibleTxIns, fJustCount, fCleanLastPaid);

    if(!fJustCount) {

        // Sort them high to low
        sort(vecMasternodeLastPaid.rbegin(), vecMasternodeLastPaid.rend(), CompareLastPaid());

        auto nEnabled = CountEnabled();
        int nEligibleNetwork = nEnabled / 10; 
        
        if(sporkManager.IsSporkActive(SPORK_114_MN_PAYMENT_V2)) {
            nEligibleNetwork = std::max(10, nEnabled * 5 / 100); // oldest 5% or the minimal of 10 MNs
        }

        int n = 0;
        // clean last paid and recalculate again
        if(fCleanLastPaid && sporkManager.IsSporkActive(SPORK_114_MN_PAYMENT_V2)) {
            for (const auto& s : vecMasternodeLastPaid) {
                CMasternode* pmn = Find(s.second);
                if (!pmn) continue;

                pmn->lastPaid = INT64_MAX;

                n++;
                if (n >= nEligibleNetwork / 3) break;
            }

            return GetNextMasternodeInQueueForPayment(nBlockHeight, fFilterSigTime, nCount, vEligibleTxIns, fJustCount, false);
        }

        uint256 nHigh;
        int nCountEligible = 0;
        for (const auto& s : vecMasternodeLastPaid) {
            CMasternode* pmn = Find(s.second);
            if (!pmn) continue;

            if (sporkManager.IsSporkActive(SPORK_114_MN_PAYMENT_V2)) {
                if (pBestMasternode == nullptr) {
                    pBestMasternode = pmn; // get the MN that was paid the last
                }
            } else {
                uint256 n = pmn->CalculateScore(1, nBlockHeight - 100);
                if (n > nHigh) {
                    nHigh = n;
                    pBestMasternode = pmn;
                }
            }

            vEligibleTxIns.push_back(s.second);
            if (sporkManager.IsSporkActive(SPORK_114_MN_PAYMENT_V2) && 
                pmn->GetLastPaid(chainActive[nBlockHeight - 1]) != 0
            ) {
                nCountEligible++;
            }
            if (nCountEligible >= nEligibleNetwork) break;
        }
    }

    return pBestMasternode;
}

CMasternode* CMasternodeMan::GetCurrentMasterNode(int mod, int64_t nBlockHeight)
{
    int64_t score = 0;
    CMasternode* winner = NULL;

    LOCK(cs);

    // scan for winner
    for (auto mn : vMasternodes) {
        mn->Check();
        if (!mn->IsEnabled()) continue;

        // calculate the score for each Masternode
        uint256 n = mn->CalculateScore(mod, nBlockHeight);
        int64_t n2 = n.GetCompact(false);

        // determine the winner
        if (n2 > score) {
            score = n2;
            winner = mn;
        }
    }

    return winner;
}

int CMasternodeMan::GetMasternodeRank(const CTxIn& vin, int64_t nBlockHeight)
{
    std::vector<std::pair<int64_t, CTxIn>> vecMasternodeScores;
    int64_t nMasternode_Min_Age = MN_WINNER_MINIMUM_AGE;
    int64_t nMasternode_Age = 0;
    bool masternodeRankV2 = Params().GetConsensus().NetworkUpgradeActive(chainActive.Height(), Consensus::UPGRADE_MASTERNODE_RANK_V2);
    int defaultValue = 
        masternodeRankV2 ?
        INT_MAX :
        -1;

    //make sure we know about this block
    uint256 hash;
    if (!GetBlockHash(hash, nBlockHeight)) return defaultValue;

    LOCK(cs);

    // scan for winner
    for (auto mn : vMasternodes) {
        if (sporkManager.IsSporkActive(SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT) &&
            sporkManager.IsSporkActive(SPORK_108_FORCE_MASTERNODE_MIN_AGE)) 
        {
            nMasternode_Age = GetAdjustedTime() - mn->sigTime;
            if ((nMasternode_Age) < nMasternode_Min_Age) {
                LogPrint(BCLog::MASTERNODE,"Skipping just activated Masternode. Age: %ld\n", nMasternode_Age);
                continue;                                                   // Skip masternodes younger than (default) 1 hour
            }
        }
        
        mn->Check();
        if (!mn->IsEnabled()) continue;

        uint256 n = mn->CalculateScore(1, nBlockHeight);
        int64_t n2 = n.GetCompact(false);

        vecMasternodeScores.push_back(std::make_pair(n2, mn->vin));
    }

    sort(vecMasternodeScores.rbegin(), vecMasternodeScores.rend(), CompareScoreTxIn());

    int rank = 0;
    for (PAIRTYPE(int64_t, CTxIn) & s : vecMasternodeScores) {
        rank++;
        if (s.second.prevout == vin.prevout) {
            return rank;
        }
    }

    return defaultValue;
}

std::vector<std::pair<int, CMasternode>> CMasternodeMan::GetMasternodeRanks(int64_t nBlockHeight)
{
    std::vector<std::pair<int64_t, CMasternode> > vecMasternodeScores;
    std::vector<std::pair<int, CMasternode> > vecMasternodeRanks;

    //make sure we know about this block
    uint256 hash;
    if (!GetBlockHash(hash, nBlockHeight)) return vecMasternodeRanks;

    {
        std::vector<CMasternode> vmn;
        {
            LOCK(cs);

            for(auto mn : vMasternodes) { // the below code takes too long, copy and go
                vmn.push_back(*mn);
            }
        }

        // scan for winner
        for (CMasternode& mn : vmn) {

            if (!mn.IsEnabled()) {
                vecMasternodeScores.push_back(std::make_pair(INT_MAX, mn));
                continue;
            }

            uint256 n = mn.CalculateScore(1, nBlockHeight);
            int64_t n2 = n.GetCompact(false);

            vecMasternodeScores.push_back(std::make_pair(n2, mn));
        }
    }

    sort(vecMasternodeScores.rbegin(), vecMasternodeScores.rend(), CompareScoreMN());

    int rank = 0;
    for (PAIRTYPE(int64_t, CMasternode) & s : vecMasternodeScores) {
        rank++;
        vecMasternodeRanks.push_back(std::make_pair(rank, s.second));
    }

    return vecMasternodeRanks;
}

void CMasternodeMan::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if (fLiteMode) return; //disable all Masternode related functionality
    if (!masternodeSync.IsBlockchainSynced()) return;

    LOCK(cs_process_message);

    if (strCommand == NetMsgType::MNBROADCAST) { //Masternode Broadcast
        CMasternodeBroadcast mnb;
        vRecv >> mnb;

        if (mapSeenMasternodeBroadcast.count(mnb.GetHash())) { //seen
            masternodeSync.AddedMasternodeList(mnb.GetHash());
            return;
        }
        mapSeenMasternodeBroadcast.insert(std::make_pair(mnb.GetHash(), mnb));

        int nDoS = 0;
        if (!mnb.CheckAndUpdate(nDoS)) {
            if (nDoS > 0) {
                LOCK(cs_main);
                Misbehaving(pfrom->GetId(), nDoS);
            }
            //failed
            return;
        }

        // make sure the vout that was signed is related to the transaction that spawned the Masternode
        //  - this is expensive, so it's only done once per Masternode
        if (!mnb.IsInputAssociatedWithPubkey()) {
            LogPrintf("CMasternodeMan::ProcessMessage() : mnb - Got mismatched pubkey and vin\n");
            LOCK(cs_main);
            Misbehaving(pfrom->GetId(), 33);
            return;
        }

        // make sure it's still unspent
        //  - this is checked later by .check() in many places and by ThreadCheckObfuScationPool()
        if (mnb.CheckInputsAndAdd(nDoS)) {
            // use this as a peer
            g_connman->AddNewAddress(CAddress(mnb.addr, NODE_NETWORK), pfrom->addr, 2 * 60 * 60);
            masternodeSync.AddedMasternodeList(mnb.GetHash());
        } else {
            LogPrint(BCLog::MASTERNODE,"mnb - Rejected Masternode entry %s\n", mnb.vin.prevout.ToStringShort());

            if (nDoS > 0) {
                LOCK(cs_main);
                Misbehaving(pfrom->GetId(), nDoS);
            }
        }
    }

    else if (strCommand == NetMsgType::MNPING) { //Masternode Ping
        CMasternodePing mnp;
        vRecv >> mnp;

        LogPrint(BCLog::MNPING, "mnp - Masternode ping, vin: %s\n", mnp.vin.prevout.ToStringShort());

        if (mapSeenMasternodePing.count(mnp.GetHash())) return; //seen
        mapSeenMasternodePing.insert(std::make_pair(mnp.GetHash(), mnp));

        int nDoS = 0;
        if (mnp.CheckAndUpdate(nDoS)) return;

        if (nDoS > 0) {
            // if anything significant failed, mark that node
            LOCK(cs_main);
            Misbehaving(pfrom->GetId(), nDoS);
        } else {
            // if nothing significant failed, search existing Masternode list
            CMasternode* pmn = Find(mnp.vin);
            // if it's known, don't ask for the mnb, just return
            if (pmn != NULL) return;
        }

        // something significant is broken or mn is unknown,
        // we might have to ask for a masternode entry once
        AskForMN(pfrom, mnp.vin);

    } else if (strCommand == NetMsgType::GETMNLIST) { //Get Masternode list or specific entry

        CTxIn vin;
        vRecv >> vin;

        if (vin == CTxIn()) { //only should ask for this once
            //local network
            bool isLocal = (pfrom->addr.IsRFC1918() || pfrom->addr.IsLocal());

            if (!isLocal && Params().NetworkID() == CBaseChainParams::MAIN) {
                std::map<CNetAddr, int64_t>::iterator i = mAskedUsForMasternodeList.find(pfrom->addr);
                if (i != mAskedUsForMasternodeList.end()) {
                    int64_t t = (*i).second;
                    if (GetTime() < t) {
                        LogPrintf("CMasternodeMan::ProcessMessage() : dseg - peer already asked me for the list\n");
                        return;
                    }
                }
                int64_t askAgain = GetTime() + MASTERNODES_DSEG_SECONDS;
                mAskedUsForMasternodeList[pfrom->addr] = askAgain;
            }
        } //else, asking for a specific node which is ok


        int nInvCount = 0;

        {
            if(vin == CTxIn()) { // send all
                LOCK(cs);

                for (auto mn : vMasternodes) {
                    if (mn->addr.IsRFC1918()) continue; // local network

                    if (mn->IsEnabled()) {
                        LogPrint(BCLog::MASTERNODE, "dseg - Sending Masternode entry - %s \n", mn->vin.prevout.ToStringShort());
                        
                        CMasternodeBroadcast mnb = CMasternodeBroadcast(*mn);
                        uint256 hash = mnb.GetHash();
                        pfrom->PushInventory(CInv(MSG_MASTERNODE_ANNOUNCE, hash));
                        nInvCount++;

                        if (!mapSeenMasternodeBroadcast.count(hash)) mapSeenMasternodeBroadcast.insert(std::make_pair(hash, mnb));

                        if (vin == mn->vin) {
                            LogPrint(BCLog::MASTERNODE, "dseg - Sent 1 Masternode entry to peer %i\n", pfrom->GetId());
                            return;
                        }
                    }
                }
            } else { // send specific one

                auto mn = Find(vin);

                if(mn && mn->IsEnabled() && !mn->addr.IsRFC1918()) {
                    LogPrint(BCLog::MASTERNODE, "dseg - Sending Masternode entry - %s \n", mn->vin.prevout.ToStringShort());

                    CMasternodeBroadcast mnb = CMasternodeBroadcast(*mn);
                    uint256 hash = mnb.GetHash();
                    pfrom->PushInventory(CInv(MSG_MASTERNODE_ANNOUNCE, hash));

                    if (!mapSeenMasternodeBroadcast.count(hash)) mapSeenMasternodeBroadcast.insert(std::make_pair(hash, mnb));

                    LogPrint(BCLog::MASTERNODE, "dseg - Sent 1 Masternode entry to peer %i\n", pfrom->GetId());
                }

                return;
            }
        }

        if (vin == CTxIn()) { // send the total count
            g_connman->PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::SYNCSTATUSCOUNT, MASTERNODE_SYNC_LIST, nInvCount));
            LogPrint(BCLog::MASTERNODE, "dseg - Sent %d Masternode entries to peer %i\n", nInvCount, pfrom->GetId());
        }
    }
}

void CMasternodeMan::Remove(CTxIn vin)
{
    LOCK(cs);

    auto it = vMasternodes.begin();
    while (it != vMasternodes.end()) {
        if ((**it).vin == vin) {
            LogPrint(BCLog::MASTERNODE, "CMasternodeMan: Removing Masternode %s - %i now\n", (**it).vin.prevout.ToStringShort(), size() - 1);
            {
                LOCK(cs_script);
                mapScriptMasternodes.erase(GetScriptForDestination((*it)->pubKeyCollateralAddress.GetID()));
            }
            {
                LOCK(cs_txin);
                mapTxInMasternodes.erase((*it)->vin);
            }
            {
                LOCK(cs_pubkey);
                mapPubKeyMasternodes.erase((*it)->pubKeyMasternode);
            }
            delete *it;
            vMasternodes.erase(it);
            break;
        }
        ++it;
    }
}

void CMasternodeMan::UpdateMasternodeList(CMasternodeBroadcast mnb)
{
    mapSeenMasternodePing.insert(std::make_pair(mnb.lastPing.GetHash(), mnb.lastPing));
    mapSeenMasternodeBroadcast.insert(std::make_pair(mnb.GetHash(), mnb));
    masternodeSync.AddedMasternodeList(mnb.GetHash());

    LogPrint(BCLog::MASTERNODE,"CMasternodeMan::UpdateMasternodeList() -- masternode=%s\n", mnb.vin.prevout.ToStringShort());

    CMasternode* pmn = Find(mnb.vin);
    if (pmn == NULL) {
        CMasternode mn(mnb);
        Add(mn);
    } else {
        pmn->UpdateFromNewBroadcast(mnb);
    }
}

std::string CMasternodeMan::ToString() const
{
    std::ostringstream info;

    info << "Masternodes: " << (int)vMasternodes.size() << ", peers who asked us for Masternode list: " << (int)mAskedUsForMasternodeList.size() << ", peers we asked for Masternode list: " << (int)mWeAskedForMasternodeList.size() << ", entries in Masternode list we asked for: " << (int)mWeAskedForMasternodeListEntry.size();

    return info.str();
}

void ThreadCheckMasternodes()
{
    if (fLiteMode) return; //disable all Masternode related functionality

    // Make this thread recognisable as the wallet flushing thread
    util::ThreadRename("pivx-masternodeman");
    LogPrintf("Masternodes thread started\n");

    unsigned int c = 0;

    try {
        while (true) {

            if (ShutdownRequested()) {
                break;
            }

            MilliSleep(1000);
            boost::this_thread::interruption_point();
            // try to sync from all available nodes, one step at a time
            masternodeSync.Process();

            if (masternodeSync.IsBlockchainSynced()) {
                c++;

                // check if we should activate or ping every few minutes,
                // start right after sync is considered to be done
                if (c % MASTERNODE_PING_SECONDS == 1) amnodeman.ManageStatus();

                if (c % 60 == 0) {
                    mnodeman.CheckAndRemove();
                    masternodePayments.CleanPaymentList();
                }
            }
        }
    } catch (boost::thread_interrupted&) {
        // nothing, thread interrupted.
    }
}
