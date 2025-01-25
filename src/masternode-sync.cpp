// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// clang-format off
#include "main.h"
#include "activemasternode.h"
#include "activemasternodeman.h"
#include "masternode-sync.h"
#include "masternode-payments.h"
#include "masternode.h"
#include "masternodeman.h"
#include "netmessagemaker.h"
#include "spork.h"
#include "util.h"
#include "addrman.h"
// clang-format on

class CMasternodeSync;
CMasternodeSync masternodeSync;

CMasternodeSync::CMasternodeSync()
{
    Reset();
}

bool CMasternodeSync::IsSynced()
{
    return RequestedMasternodeAssets == MASTERNODE_SYNC_FINISHED;
}

bool CMasternodeSync::IsSporkListSynced()
{
    return RequestedMasternodeAssets > MASTERNODE_SYNC_SPORKS;
}

bool CMasternodeSync::IsMasternodeListSynced()
{
    return RequestedMasternodeAssets > MASTERNODE_SYNC_LIST;
}

bool CMasternodeSync::NotCompleted()
{
    return (!IsSynced() && (
            !IsSporkListSynced() ||
            sporkManager.IsSporkActive(SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT)));
}

bool CMasternodeSync::IsBlockchainSynced()
{
    int64_t now = GetTime();

    // if the last call to this function was more than 60 minutes ago (client was in sleep mode) reset the sync process
    if (now > lastProcess + HOUR_IN_SECONDS) {
        Reset();
        fBlockchainSynced = false;
    }
    lastProcess = now;

    if (fBlockchainSynced) return true;

    if (fImporting || fReindex) return false;

    int64_t blockTime = 0;
    {
        TRY_LOCK(cs_main, lockMain);
        if (!lockMain) return false;
        CBlockIndex *pindex = chainActive.Tip();
        if (pindex == nullptr) return false;
        blockTime = pindex->nTime;
    }

    auto maxBlockTime = sporkManager.GetSporkValue(SPORK_104_MAX_BLOCK_TIME);

    if (maxBlockTime > lastProcess && blockTime + HOUR_IN_SECONDS < lastProcess)
        return false;

    if (maxBlockTime <= lastProcess && blockTime + HOUR_IN_SECONDS < maxBlockTime)
        return false;

    fBlockchainSynced = true;

    return fBlockchainSynced;
}

void CMasternodeSync::Reset()
{
    fBlockchainSynced = false;
    lastProcess = 0;
    lastMasternodeList = 0;
    lastFailure = 0;
    nCountFailures = 0;
    sumMasternodeList = 0;
    countMasternodeList = 0;
    RequestedMasternodeAssets = MASTERNODE_SYNC_INITIAL;
    RequestedMasternodeAttempt = 0;
    nAssetSyncStarted = GetTime();
}

void CMasternodeSync::AddedMasternodeList(const uint256& hash)
{
    if (mnodeman.mapSeenMasternodeBroadcast.count(hash)) {
        if (mapSeenSyncMNB[hash] < MASTERNODE_SYNC_THRESHOLD) {
            lastMasternodeList = GetTime();
            mapSeenSyncMNB[hash]++;
        }
    } else {
        lastMasternodeList = GetTime();
        mapSeenSyncMNB.insert(std::make_pair(hash, 1));
    }
}

void CMasternodeSync::GetNextAsset()
{
    switch (RequestedMasternodeAssets) {
    case (MASTERNODE_SYNC_INITIAL):
    case (MASTERNODE_SYNC_FAILED): // should never be used here actually, use Reset() instead
        ClearFulfilledRequest();
        RequestedMasternodeAssets = MASTERNODE_SYNC_SPORKS;
        break;
    case (MASTERNODE_SYNC_SPORKS):
        RequestedMasternodeAssets = MASTERNODE_SYNC_LIST;
        break;
    case (MASTERNODE_SYNC_LIST):
        RequestedMasternodeAssets = MASTERNODE_SYNC_FINISHED;
        LogPrintf("CMasternodeSync::GetNextAsset - Sync has finished\n");
        break;
    }
    RequestedMasternodeAttempt = 0;
    nAssetSyncStarted = GetTime();

    // Notify the UI
    uiInterface.NotifyBlockTip(IsInitialBlockDownload(), chainActive.Tip());
}

std::string CMasternodeSync::GetSyncStatus()
{
    switch (masternodeSync.RequestedMasternodeAssets) {
    case MASTERNODE_SYNC_INITIAL:
        return _("MNs synchronization pending...");
    case MASTERNODE_SYNC_SPORKS:
        return _("Synchronizing sporks...");
    case MASTERNODE_SYNC_LIST:
        return _("Synchronizing masternodes...");
    case MASTERNODE_SYNC_FAILED:
        return _("Synchronization failed");
    case MASTERNODE_SYNC_FINISHED:
        return _("Synchronization finished");
    }
    return "";
}

void CMasternodeSync::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if (strCommand == NetMsgType::SYNCSTATUSCOUNT) { //Sync status count
        int nItemID;
        int nCount;
        vRecv >> nItemID >> nCount;

        if (RequestedMasternodeAssets >= MASTERNODE_SYNC_FINISHED) return;

        switch (nItemID) {
        case (MASTERNODE_SYNC_LIST):
            if (nItemID != RequestedMasternodeAssets) return;
            sumMasternodeList += nCount;
            countMasternodeList++;
            break;
        }

        LogPrint(BCLog::MASTERNODE, "CMasternodeSync:ProcessMessage - ssc - got inventory count %d %d\n", nItemID, nCount);
    }
}

void CMasternodeSync::ClearFulfilledRequest()
{
    g_connman->ForEachNode([](CNode* pnode) {
        pnode->ClearFulfilledRequest("getspork");
        pnode->ClearFulfilledRequest("mnsync");
    });
}

void CMasternodeSync::Process()
{
    static int tick = 0;
    const bool isRegTestNet = Params().IsRegTestNet();

    if (tick++ % MASTERNODE_SYNC_TIMEOUT != 0) return;

    if (IsSynced()) {
        /*
            Resync if we lose all masternodes from sleep/wake or failure to sync originally
        */
        if (mnodeman.CountEnabled() == 0) {
            Reset();
        } else
            return;
    }

    //try syncing again
    if (RequestedMasternodeAssets == MASTERNODE_SYNC_FAILED && lastFailure + (1 * 60) < GetTime()) {
        Reset();
    } else if (RequestedMasternodeAssets == MASTERNODE_SYNC_FAILED) {
        return;
    }

    LogPrint(
        BCLog::MASTERNODE, "%s - tick %d RequestedMasternodeAssets %d\n", 
        __func__, 
        tick, 
        RequestedMasternodeAssets
    );

    if (RequestedMasternodeAssets == MASTERNODE_SYNC_INITIAL) GetNextAsset();

    // sporks synced but blockchain is not, wait until we're almost at a recent block to continue
    if (!isRegTestNet && !IsBlockchainSynced() &&
        RequestedMasternodeAssets > MASTERNODE_SYNC_SPORKS) return;

    CMasternodeSync* sync = this;
    g_connman->ForEachNodeContinueIf([sync, isRegTestNet](CNode* pnode) {
      return sync->SyncWithNode(pnode, isRegTestNet);
    });
}

bool CMasternodeSync::SyncWithNode(CNode* pnode, bool isRegTestNet)
{
    CNetMsgMaker msgMaker(pnode->GetSendVersion());
    if (isRegTestNet) {
        if (RequestedMasternodeAttempt <= 2) {
            g_connman->PushMessage(pnode, msgMaker.Make(NetMsgType::GETSPORKS)); //get current network sporks
        } else if (RequestedMasternodeAttempt < 4) {
            mnodeman.DsegUpdate(pnode);
        } else if (RequestedMasternodeAttempt < 6) {
            int nMnCount = mnodeman.CountEnabled();

            g_connman->PushMessage(pnode, msgMaker.Make(NetMsgType::GETMNWINNERS, nMnCount)); //sync payees
        } else {
            RequestedMasternodeAssets = MASTERNODE_SYNC_FINISHED;
        }
        RequestedMasternodeAttempt++;
        return false;
    }

    //set to synced
    if (RequestedMasternodeAssets == MASTERNODE_SYNC_SPORKS) {
        if (pnode->HasFulfilledRequest("getspork")) return true;
        pnode->FulfilledRequest("getspork");

        g_connman->PushMessage(pnode, msgMaker.Make(NetMsgType::GETSPORKS)); //get current network sporks
        if (RequestedMasternodeAttempt >= 2) GetNextAsset();
        RequestedMasternodeAttempt++;
        return false;
    }

    if (pnode->nVersion >= ActiveProtocol()) {
        if (RequestedMasternodeAssets == MASTERNODE_SYNC_LIST) {
            LogPrint(
                BCLog::MASTERNODE, 
                "%s - lastMasternodeList %lld (GetTime() - MASTERNODE_SYNC_TIMEOUT) %lld\n", 
                __func__,
                lastMasternodeList, 
                GetTime() - MASTERNODE_SYNC_TIMEOUT
            );

            if (lastMasternodeList > 0 && countMasternodeList > 0 &&
                RequestedMasternodeAttempt >= MASTERNODE_SYNC_THRESHOLD &&
                mnodeman.CountEnabled() >= (sumMasternodeList * 60) / (countMasternodeList * 100) // only move on after getting a properly sized MN list
            ) { // we have a good enough mn list, so we'll move to the next step
                GetNextAsset();
                return false;
            }

            if (pnode->HasFulfilledRequest("mnsync")) return true;
            pnode->FulfilledRequest("mnsync");

            // timeout
            if (lastMasternodeList == 0 &&
                (RequestedMasternodeAttempt >= MASTERNODE_SYNC_THRESHOLD * 3 || GetTime() - nAssetSyncStarted > MASTERNODE_SYNC_TIMEOUT * 6)) {
                if (sporkManager.IsSporkActive(SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT)) {
                    LogPrintf("CMasternodeSync::Process - ERROR - Sync has failed on %s, will retry later\n", "MASTERNODE_SYNC_LIST");
                    RequestedMasternodeAssets = MASTERNODE_SYNC_FAILED;
                    RequestedMasternodeAttempt = 0;
                    lastFailure = GetTime();
                    nCountFailures++;
                } else {
                    GetNextAsset();
                }
                return false;
            }

            if (RequestedMasternodeAttempt >= MASTERNODE_SYNC_THRESHOLD * 5) return false;

            mnodeman.DsegUpdate(pnode);
            RequestedMasternodeAttempt++;
            return false;
        }
    }

    return true;
}
