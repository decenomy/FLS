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
#include "core_io.h"

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

    if (!mn.IsEnabled()) return false;

    CMasternode* pmn = Find(mn.vin);

    auto mnScript = Find(GetScriptForDestination(mn.pubKeyCollateralAddress.GetID()));
    if(mnScript) {
        auto it = std::find(vMasternodes.begin(), vMasternodes.end(), mnScript);
        if(it != vMasternodes.end()) vMasternodes.erase(it);

        return false;
    }

    if (pmn == nullptr) {
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

void CMasternodeMan::Check()
{
    LOCK(cs);

    for (auto mn : vMasternodes) {
        mn->Check();
    }
}

void CMasternodeMan::CheckAndRemove(bool forceExpiredRemoval)
{
    Check();

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

    {
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

    {
        LOCK(cs_collaterals);
        initiatedAt = -1;
    }
}

int CMasternodeMan::stable_size ()
{
    int nStable_size = 0;
    int64_t nMasternode_Age = 0;

    LOCK(cs);

    for (auto mn : vMasternodes) {
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

bool CMasternodeMan::HasCollateral(const CScript& payee) {
    LOCK(cs_collaterals);
    return mapScriptCollaterals.find(payee) != mnodeman.mapScriptCollaterals.end();
}

Coin CMasternodeMan::GetCollateral(const CScript& payee) {
    LOCK(cs_collaterals);

    const auto& it = mapScriptCollaterals.find(payee);
    if(it != mnodeman.mapScriptCollaterals.end()) {
        return (*it).second;
    } else {
        return Coin();
    }
}

//
// Deterministically select the oldest/best masternode to pay on the network
//
CMasternode* CMasternodeMan::GetNextMasternodeInQueueForPayment(const CBlockIndex* pindexPrev, bool fFilterSigTime, int& nCount, std::vector<CTxIn>& vEligibleTxIns, bool fJustCount)
{
    const auto& params = Params();
    const auto& consensus = params.GetConsensus();
    const auto nBlockHeight = pindexPrev->nHeight + 1;
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

            //it's too new, wait for a cycle
            if (fFilterSigTime && mn->sigTime + (nMnCount * 60) > GetAdjustedTime()) continue;

            //make sure it has as many confirmations as there are masternodes
            if (pcoinsTip->GetCoinDepthAtHeight(mn->vin.prevout, nBlockHeight) < nMnCount) continue;

            vecMasternodeLastPaid.push_back(std::make_pair(mn->SecondsSincePayment(pindexPrev), mn->vin));
        }
    }

    nCount = (int)vecMasternodeLastPaid.size();

    //when the network is in the process of upgrading, don't penalize nodes that recently restarted
    if (fFilterSigTime && nCount < nMnCount / 3) return GetNextMasternodeInQueueForPayment(pindexPrev, false, nCount, vEligibleTxIns, fJustCount);

    if(!fJustCount) {

        // Sort them high to low
        sort(vecMasternodeLastPaid.rbegin(), vecMasternodeLastPaid.rend(), CompareLastPaid());

        auto nEnabled = CountEnabled();
        auto nEligibleNetwork = std::max(10, nEnabled * 5 / 100); // oldest 5% or the minimal of 10 MNs

        uint256 nHigh;
        int nCountEligible = 0;
        for (const auto& s : vecMasternodeLastPaid) {
            auto pmn = Find(s.second);
            if (!pmn) continue;

            if (!pBestMasternode) {
                pBestMasternode = pmn; // get the MN that was paid the last
            }

            vEligibleTxIns.push_back(s.second);
            nCountEligible++;

            if (nCountEligible >= nEligibleNetwork) break;
        }
    }

    return pBestMasternode;
}

const CBlockIndex* CMasternodeMan::GetLastPaidBlockSlow(const CScript& script, const CBlockIndex* pindexPrev) 
{
    auto pindex = pindexPrev;
    CBlock block;

    {
        LOCK(cs_main);

        for(int i = 0; i < DEFAULT_MAX_REORG_DEPTH; i++) 
        {
            if(chainActive[pindex->nHeight]->GetBlockHash() == pindex->GetBlockHash()) {
                return GetLastPaidBlock(script, pindex); // onchain, use a faster alternative
            }

            if(!ReadBlockFromDisk(block, pindex)) {
                return nullptr; // should not happen
            }

            auto amount = CMasternode::GetMasternodePayment(pindex->nHeight);
            auto paidPayee = block.GetPaidPayee(amount);

            if(paidPayee == script) {
                return pindex;
            }

            if(block.hashPrevBlock.IsNull()) return nullptr; // should not happen or we reached the genesis block

            pindex = mapBlockIndex[block.hashPrevBlock];
        }
    }

    // we reached the limit of reorg lets continue with the faster algorithm
    return GetLastPaidBlock(script, pindex);
}

const CBlockIndex* CMasternodeMan::GetLastPaidBlock(const CScript& script, const CBlockIndex* pindex) 
{
    LOCK(cs_collaterals);

    if(mapPaidPayeesBlocks.find(script) != mapPaidPayeesBlocks.end() &&
       !mapPaidPayeesBlocks[script].empty()
    ) {
        const auto& vblocks = mapPaidPayeesBlocks[script];
        for (auto it = vblocks.rbegin(); it != vblocks.rend(); ++it)
        {
            if((*it)->nHeight <= pindex->nHeight) {
                return *it;
            }
        }
    }
    return nullptr;
}

int CMasternodeMan::BlocksSincePayment(const CScript& script, const CBlockIndex* pindex) {
    const auto pLastPaidBlock = GetLastPaidBlockSlow(script, pindex);

    if(pLastPaidBlock) {
        return pindex->nHeight - pLastPaidBlock->nHeight;
    }

    const auto collateral = mnodeman.GetCollateral(script);

    if (collateral.nHeight != 0) {
        return pindex->nHeight - collateral.nHeight;
    }

    return -1;
}

int64_t CMasternodeMan::GetLastPaid(const CScript& script, const CBlockIndex* pindex) {
    const auto pIndex = GetLastPaidBlock(script, pindex);
    if(pIndex) {
        return pIndex->GetBlockTime();
    }
    return 0;
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

        if(vin == CTxIn()) { // send all
            LOCK(cs);
            int nInvCount = 0;
            for (const auto& mn : vMasternodes) {
                if (mn->IsEnabled() && !mn->addr.IsRFC1918()) {
                    LogPrint(BCLog::MASTERNODE, "dseg - Sending Masternode entry - %s \n", mn->vin.prevout.ToStringShort());
                    
                    CMasternodeBroadcast mnb = CMasternodeBroadcast(*mn);
                    uint256 hash = mnb.GetHash();
                    pfrom->PushInventory(CInv(MSG_MASTERNODE_ANNOUNCE, hash));
                    nInvCount++;

                    if (!mapSeenMasternodeBroadcast.count(hash)) mapSeenMasternodeBroadcast.insert(std::make_pair(hash, mnb));
                }
            }

            g_connman->PushMessage(pfrom, CNetMsgMaker(pfrom->GetSendVersion()).Make(NetMsgType::SYNCSTATUSCOUNT, MASTERNODE_SYNC_LIST, nInvCount));
            LogPrint(BCLog::MASTERNODE, "dseg - Sent %d Masternode entries to peer %i\n", nInvCount, pfrom->GetId());
        } else { // send specific one

            const auto mn = Find(vin);

            if(mn && mn->IsEnabled() && !mn->addr.IsRFC1918()) {
                LogPrint(BCLog::MASTERNODE, "dseg - Sending Masternode entry - %s \n", mn->vin.prevout.ToStringShort());

                CMasternodeBroadcast mnb = CMasternodeBroadcast(*mn);
                uint256 hash = mnb.GetHash();
                pfrom->PushInventory(CInv(MSG_MASTERNODE_ANNOUNCE, hash));

                if (!mapSeenMasternodeBroadcast.count(hash)) mapSeenMasternodeBroadcast.insert(std::make_pair(hash, mnb));

                LogPrint(BCLog::MASTERNODE, "dseg - Sent 1 Masternode entry to peer %i\n", pfrom->GetId());
            }
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

bool CMasternodeMan::Init()
{
    if(initiatedAt > 0) return true;

    FlushStateToDisk();

    LOCK(cs_collaterals);

    // cleans up all collections
    mapScriptCollaterals.clear();
    mapCOutPointCollaterals.clear();
    mapCAmountCollaterals.clear();
    mapRemovedCollaterals.clear();
    mapPaidPayeesBlocks.clear();
    mapPaidPayeesHeight.clear();

    const auto nHeight = chainActive.Height();
    const auto& params = Params();
    const auto& consensus = params.GetConsensus();
    const auto nBlocksPerWeek = WEEK_IN_SECONDS / consensus.nTargetSpacing;

    // get the current masternode collateral, and the next week collateral
    auto nCollateralAmount = CMasternode::GetMasternodeNodeCollateral(nHeight);
    auto nNextWeekCollateralAmount = CMasternode::GetMasternodeNodeCollateral(nHeight + nBlocksPerWeek);

    if (nCollateralAmount > 0 || nNextWeekCollateralAmount > 0) {
        std::unique_ptr<CCoinsViewCursor> pcursor(pcoinsTip->Cursor());

        while (pcursor->Valid()) {
            boost::this_thread::interruption_point();
            COutPoint key;
            Coin coin;
            if (pcursor->GetKey(key) && pcursor->GetValue(coin)) {
                if (!coin.IsSpent() && (coin.out.nValue == nCollateralAmount || coin.out.nValue == nNextWeekCollateralAmount)) {
                    const auto& out = coin.out;
                    const auto& nCollateral = out.nValue;
                    // this is a possible collateral UTXO
                    mapScriptCollaterals[coin.out.scriptPubKey] = coin;
                    mapCOutPointCollaterals[key] = coin;
                    // check if there is no entry for this collateral
                    if(mapCAmountCollaterals.find(nCollateral) == mapCAmountCollaterals.end()) {
                        mapCAmountCollaterals[nCollateral] = boost::unordered_set<COutPoint, COutPointCheapHasher>(); // add an empty set
                    }
                    mapCAmountCollaterals[nCollateral].insert(key);
                }
            }
            pcursor->Next();
        }
    }

    // scan the blockchain for paid payees
    const auto nCollaterals = mapScriptCollaterals.size();
    const auto nMaxDepth = nCollaterals * 2;

    for(int h = nHeight - nMaxDepth; h <= nHeight; h++) {
        const auto pBlockIndex = chainActive[h];
        const auto paidPayee = pBlockIndex->GetPaidPayee();

        if(mapPaidPayeesBlocks.find(paidPayee) == mapPaidPayeesBlocks.end()) {
            mapPaidPayeesBlocks[paidPayee] = std::vector<const CBlockIndex*>();
        }

        mapPaidPayeesBlocks[paidPayee].push_back(pBlockIndex);
        mapPaidPayeesHeight[h] = paidPayee;
    }

    initiatedAt = nHeight;
    lastProcess = GetTime();

    return true;
}

void CMasternodeMan::Shutdown()
{
}

bool CMasternodeMan::ConnectBlock(const CBlockIndex* pindex, const CBlock& block)
{
    LOCK(cs_collaterals);

    int64_t now = GetTime();
    // if the last call to this function was more than 60 minutes ago (client was in sleep mode) reset data
    if (now > lastProcess + HOUR_IN_SECONDS) {
        initiatedAt = -1;
    }
    lastProcess = now;

    if (initiatedAt < 0 && !Init()) return false;

    const auto nHeight = pindex->nHeight;
    const auto& params = Params();
    const auto& consensus = params.GetConsensus();
    const auto nBlocksPerWeek = WEEK_IN_SECONDS / consensus.nTargetSpacing;

    // removes old data
    const auto nRemovalHeight = nHeight - DEFAULT_MAX_REORG_DEPTH;
    mapRemovedCollaterals.erase(nRemovalHeight);
    
    initiatedAt = std::max(initiatedAt, nRemovalHeight);

    // get the current masternode collateral, and the next week collateral
    auto nCollateralAmount = CMasternode::GetMasternodeNodeCollateral(nHeight);
    auto nNextWeekCollateralAmount = CMasternode::GetMasternodeNodeCollateral(nHeight + nBlocksPerWeek);

    // remove all UTXOs with old collaterals
    for(const auto& kv : mapCAmountCollaterals) {
        const auto& nCollateral = kv.first;
        if(nCollateral != nCollateralAmount && nCollateralAmount != nNextWeekCollateralAmount) {
            auto& toRemove = mapCAmountCollaterals[nCollateral];
            for (const auto& outPoint : toRemove) {
                const auto& coin = mapCOutPointCollaterals[outPoint];

                if(mapRemovedCollaterals.find(nHeight) == mapRemovedCollaterals.end()) {
                    mapRemovedCollaterals[nHeight] = boost::unordered_map<COutPoint, Coin, COutPointCheapHasher>();
                }
                mapRemovedCollaterals[nHeight][outPoint] = coin;

                mapCOutPointCollaterals.erase(outPoint);

                const auto& script = coin.out.scriptPubKey;
                mapScriptCollaterals.erase(script);

                // mark the masternode as vin spent
                const auto pmn = Find(script);
                if(pmn) {
                    pmn->activeState = CMasternode::MASTERNODE_VIN_SPENT;
                }
            }
            mapCAmountCollaterals.erase(nCollateral);
        }
    }

    for (const auto& tx : block.vtx) {
        // remove the collaterals that were spent
        for (const auto& in : tx.vin) {
            if(mapCOutPointCollaterals.find(in.prevout) != mapCOutPointCollaterals.end()) 
            {
                const auto& outPoint = in.prevout;
                const auto& coin = mapCOutPointCollaterals[outPoint];
                const auto& nCollateral = coin.out.nValue;
                const auto& scriptPubKey = coin.out.scriptPubKey;

                if(mapRemovedCollaterals.find(nHeight) == mapRemovedCollaterals.end()) {
                    mapRemovedCollaterals[nHeight] = boost::unordered_map<COutPoint, Coin, COutPointCheapHasher>();
                }
                mapRemovedCollaterals[nHeight][outPoint] = coin;

                mapCOutPointCollaterals.erase(outPoint);
                mapScriptCollaterals.erase(scriptPubKey);

                // check if there is a entry for this collateral
                if(mapCAmountCollaterals.find(nCollateral) != mapCAmountCollaterals.end()) {
                    mapCAmountCollaterals[nCollateral].erase(outPoint);    
                }

                // mark the masternode as vin spent
                const auto pmn = Find(scriptPubKey);
                if(pmn) {
                    pmn->activeState = CMasternode::MASTERNODE_VIN_SPENT;
                }
            }
        }

        // add the collaterals that were created
        auto n = 0;
        for (const auto& out : tx.vout) {
            if (out.nValue == nCollateralAmount || out.nValue == nNextWeekCollateralAmount) {
                const auto& nCollateral = out.nValue;
                const auto& outPoint = COutPoint(tx.GetHash(), n);
                const auto coin = Coin(out, nHeight, n == 0, n == 1);
                
                mapScriptCollaterals[out.scriptPubKey] = coin;
                mapCOutPointCollaterals[outPoint] = coin;
                
                if(mapCAmountCollaterals.find(nCollateral) == mapCAmountCollaterals.end()) {
                    mapCAmountCollaterals[nCollateral] = boost::unordered_set<COutPoint, COutPointCheapHasher>(); // add an empty set
                }
                mapCAmountCollaterals[nCollateral].insert(outPoint);
            }
            n++;
        }
    }

    // register the paid payee for this block
    const auto amount = CMasternode::GetMasternodePayment(nHeight);
    const auto paidPayee = block.GetPaidPayee(amount);

    if(!paidPayee.empty()) {
        if(mapPaidPayeesBlocks.find(paidPayee) == mapPaidPayeesBlocks.end()) {
            mapPaidPayeesBlocks[paidPayee] = std::vector<const CBlockIndex*>();
        }

        mapPaidPayeesBlocks[paidPayee].push_back(pindex);
        mapPaidPayeesHeight[nHeight] = paidPayee;
    }

    return true;
}

bool CMasternodeMan::DisconnectBlock(const CBlockIndex* pindex, const CBlock& block)
{
    LOCK(cs_collaterals);

    int64_t now = GetTime();
    // if the last call to this function was more than 60 minutes ago (client was in sleep mode) reset data
    if (now > lastProcess + HOUR_IN_SECONDS) {
        initiatedAt = -1;
    }
    lastProcess = now;

    const auto nHeight = pindex->nHeight;
    const auto& params = Params();
    const auto& consensus = params.GetConsensus();
    const auto nBlocksPerWeek = WEEK_IN_SECONDS / consensus.nTargetSpacing;

    if(nHeight < initiatedAt) {
        initiatedAt = -1; // redo all the mappings at next connect block
        return true;
    }

    // get the current masternode collateral, and the next week collateral
    auto nCollateralAmount = CMasternode::GetMasternodeNodeCollateral(nHeight);
    auto nNextWeekCollateralAmount = CMasternode::GetMasternodeNodeCollateral(nHeight + nBlocksPerWeek);

    for (const auto& tx : block.vtx) {
        // remove the collaterals that were created
        auto n = 0;
        for (const auto& out : tx.vout) {
            if (out.nValue == nCollateralAmount || out.nValue == nNextWeekCollateralAmount) {
                const auto& nCollateral = out.nValue;
                const auto& outPoint = COutPoint(tx.GetHash(), n);

                mapScriptCollaterals.erase(out.scriptPubKey);
                mapCOutPointCollaterals.erase(outPoint);

                if (mapCAmountCollaterals.find(nCollateral) != mapCAmountCollaterals.end()) {
                    mapCAmountCollaterals[nCollateral].erase(outPoint);
                }
            }
            n++;
        }
    }

    // restore the collaterals that were remove at this height
    if(mapRemovedCollaterals.find(nHeight) != mapRemovedCollaterals.end()) {
        for (const auto& kv : mapRemovedCollaterals[nHeight]) {
            const auto& outPoint = kv.first;
            const auto& coin = kv.second;

            mapScriptCollaterals[coin.out.scriptPubKey] = coin;
            mapCOutPointCollaterals[outPoint] = coin;

            const auto& nCollateral = coin.out.nValue;
            if (mapCAmountCollaterals.find(nCollateral) == mapCAmountCollaterals.end()) {
                mapCAmountCollaterals[nCollateral] = boost::unordered_set<COutPoint, COutPointCheapHasher>(); // add an empty set
            }
            mapCAmountCollaterals[nCollateral].insert(outPoint);
        }
        mapRemovedCollaterals.erase(nHeight);
    }

    // remove the paidpayees that were registered
    if(mapPaidPayeesHeight.find(nHeight) != mapPaidPayeesHeight.end()) {
        const auto& script = mapPaidPayeesHeight[nHeight];

        mapPaidPayeesBlocks[script].pop_back();

        if(mapPaidPayeesBlocks[script].empty()) {
            mapPaidPayeesBlocks.erase(script);
        }

        mapPaidPayeesHeight.erase(nHeight);
    }

    return true;
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
                }
            }
        }
    } catch (boost::thread_interrupted&) {
        // nothing, thread interrupted.
    }
}
