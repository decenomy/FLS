// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "masternode-payments.h"
#include "addrman.h"
#include "chainparams.h"
#include "fs.h"
#include "masternode-sync.h"
#include "masternodeman.h"
#include "netmessagemaker.h"
#include "rewards.h"
#include "spork.h"
#include "sync.h"
#include "util.h"
#include "utilmoneystr.h"
#include "core_io.h"

/** Object for who's going to get paid on which blocks */
CMasternodePayments masternodePayments;

bool IsBlockValueValid(int nHeight, CAmount nExpectedValue, CAmount nMinted)
{
    // No superblock, regular check
    return nMinted <= nExpectedValue;
}

bool IsBlockPayeeValid(const CBlock& block, CBlockIndex* pindexPrev)
{
    if (!masternodeSync.IsSynced()) { //there is no MN data to use to check anything -- find the longest chain
        LogPrint(BCLog::MASTERNODE, "Client not synced, skipping block payee checks\n");
        return true;
    }

    const auto& params = Params();
    const auto& consensus = params.GetConsensus();

    //check for masternode payee
    if (masternodePayments.IsTransactionValid(block, pindexPrev))
        return true;

    // fails if spork 8 is enabled
    if (sporkManager.IsSporkActive(SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT)) {
        return false;
    } else {
        LogPrint(BCLog::MASTERNODE,"Masternode payment enforcement is disabled, accepting block\n");
        return true;
    }
}

void FillBlockPayee(CMutableTransaction& txNew, const CBlockIndex* pindexPrev, bool fProofOfStake)
{
    masternodePayments.FillBlockPayee(txNew, pindexPrev, fProofOfStake);
}

void CMasternodePayments::FillBlockPayee(CMutableTransaction& txNew, const CBlockIndex* pindexPrev, bool fProofOfStake)
{
    if (!pindexPrev) return;

    CScript payee;

    const auto hasPayment = masternodePayments.GetBlockPayee(pindexPrev, payee);

    if (hasPayment) {
        CAmount masternodePayment = CMasternode::GetMasternodePayment(pindexPrev->nHeight + 1);
        if (fProofOfStake) {
            /**For Proof Of Stake vout[0] must be null
             * Stake reward can be split into many different outputs, so we must
             * use vout.size() to align with several different cases.
             * An additional output is appended as the masternode payment
             */
            unsigned int i = txNew.vout.size();
            txNew.vout.resize(i + 1);
            txNew.vout[i].scriptPubKey = payee;
            txNew.vout[i].nValue = masternodePayment;

            //subtract mn payment from the stake reward
            if (i == 2) {
                // Majority of cases; do it quick and move on
                txNew.vout[i - 1].nValue -= masternodePayment;
            } else if (i > 2) {
                // special case, stake is split between (i-1) outputs
                unsigned int outputs = i-1;
                CAmount mnPaymentSplit = masternodePayment / outputs;
                CAmount mnPaymentRemainder = masternodePayment - (mnPaymentSplit * outputs);
                for (unsigned int j=1; j<=outputs; j++) {
                    txNew.vout[j].nValue -= mnPaymentSplit;
                }
                // in case it's not an even division, take the last bit of dust from the last one
                txNew.vout[outputs].nValue -= mnPaymentRemainder;
            }
        } else {
            txNew.vout.resize(2);
            txNew.vout[1].scriptPubKey = payee;
            txNew.vout[1].nValue = masternodePayment;
            txNew.vout[0].nValue = CRewards::GetBlockValue(pindexPrev->nHeight + 1) - masternodePayment;
        }

        CTxDestination address1;
        ExtractDestination(payee, address1);

        LogPrint(BCLog::MASTERNODE,"Masternode payment of %s to %s\n", FormatMoney(masternodePayment).c_str(), EncodeDestination(address1).c_str());
    }
}

bool CMasternodePayments::GetBlockPayee(const CBlockIndex* pindexPrev, CScript& payee)
{
    const auto nBlockHeight = pindexPrev->nHeight + 1;
    LogPrint(BCLog::MASTERNODE, "%s : nHeight %d. \n", __func__, nBlockHeight);

    // pay to the oldest MN that still had no payment but its input is old enough and it was active long enough
    auto pmn = mnodeman.GetNextMasternodeInQueueForPayment(pindexPrev);

    if (pmn) {
        LogPrint(BCLog::MASTERNODE,"%s : Found by GetNextMasternodeInQueueForPayment \n", __func__);

        payee = GetScriptForDestination(pmn->pubKeyCollateralAddress.GetID());

        CTxDestination address1;
        ExtractDestination(payee, address1);

        LogPrint(BCLog::MASTERNODE,"%s : Winner payee %s nHeight %d. \n", __func__, EncodeDestination(address1).c_str(), nBlockHeight);

        return true;
    } 

    LogPrint(BCLog::MASTERNODE,"%s : Failed to find masternode to pay\n", __func__);

    return false;
}

bool CMasternodePayments::IsTransactionValid(const CBlock& block, const CBlockIndex* pindexPrev)
{
    // if the blockchain is not synced, then there is no enough data to perform verification
    if (!masternodeSync.IsBlockchainSynced()) {
        LogPrint(BCLog::MASTERNODE, "%s - !masternodeSync.IsBlockchainSynced()\n", __func__);
        return true;
    }

    assert(block.hashPrevBlock == pindexPrev->GetBlockHash());

    const auto nBlockHeight = pindexPrev->nHeight + 1;
    const auto& params = Params();
    const auto& consensus = params.GetConsensus();
    const auto& txNew = block.vtx[block.IsProofOfStake() ? 1 : 0];

    auto requiredMasternodePayment = CMasternode::GetMasternodePayment(nBlockHeight);
    auto found = false;
    CScript paidPayee;

    for (CTxOut out : txNew.vout) {
        if (out.nValue == requiredMasternodePayment) {
            found = true;
            paidPayee = out.scriptPubKey;
        }
    }

    if (found) {
        if (!mnodeman.HasCollateral(paidPayee)) {
            return false;
        }

        // if there is no MNs, then there is no enough data to perform further verifications
        if (mnodeman.CountEnabled() == 0) {
            LogPrint(BCLog::MASTERNODE, "%s - mnodeman.CountEnabled() == 0\n", __func__);
            return true;
        }

        // if the masternode list is not synced, then there is no enough data to perform further verifications
        if (!masternodeSync.IsSynced()) {
            LogPrint(BCLog::MASTERNODE, "%s - !masternodeSync.IsSynced()\n", __func__);
            return true;
        }

        // get collateral outpoint
        const auto collateral = mnodeman.GetCollateral(paidPayee);

        if (collateral.nHeight == 0) {
            return false; // should not happen
        }

        // get when this masternode was last paid
        int n = 0;
        const auto pLastPaidBlock = mnodeman.GetLastPaidBlockSlow(paidPayee, pindexPrev);
        const auto lastPaid = std::max(
            static_cast<uint32_t>(pLastPaidBlock ? pLastPaidBlock->nHeight : 0),
            collateral.nHeight
        );

        auto lastPaidDepth = mnodeman.BlocksSincePayment(paidPayee, pindexPrev);
        if(lastPaidDepth < 0) {
            lastPaidDepth = pindexPrev->nHeight - collateral.nHeight;
        }

        // get the masternodes choosen on this block from our point of view
        const auto eligible = mnodeman.GetNextMasternodeInQueueEligible(pindexPrev);

        // if there is no eligible masternode, then there is no enough data to perform further verifications
        if (eligible.first == nullptr) {
            return true;
        }

        const auto eligibleDepth = eligible.first->BlocksSincePayment(pindexPrev);

        auto minDepth = INT32_MAX;
        auto maxDepth = 0;

        for(const auto& txin : eligible.second) {
            CMasternode* pmn = mnodeman.Find(txin);
            if (!pmn) continue;

            const auto nDepth = pmn->BlocksSincePayment(pindexPrev);
            minDepth = std::min(minDepth, nDepth);
            maxDepth = std::max(maxDepth, nDepth);
        }

        if(LogAcceptCategory(BCLog::MASTERNODE)) {
            if(pLastPaidBlock) {

                CTxDestination destination;
                ExtractDestination(paidPayee, destination);

                LogPrint(
                    BCLog::MASTERNODE, "%s - Paid Payee %s Block %d : %s\n", 
                    __func__, 
                    EncodeDestination(destination), 
                    pLastPaidBlock->nHeight, 
                    pLastPaidBlock->GetBlockHash().ToString()
                );
            } // nBlockHeight
            LogPrint(BCLog::MASTERNODE, "%s - Block tested/tip %d/%d\n", __func__, nBlockHeight, chainActive.Height());
            LogPrint(BCLog::MASTERNODE, "%s - Eligible min/max depth %d/%d\n", __func__, minDepth, maxDepth);
            LogPrint(BCLog::MASTERNODE, "%s - Eligible and paid depth %d/%d\n", __func__, maxDepth, lastPaidDepth);
        }

        // reject it, if it is being paid faster than the shortest depth elegible MN
        if (lastPaidDepth < minDepth) {

            if(LogAcceptCategory(BCLog::MASTERNODE)) {
                auto p = pindexPrev;
                for(int i = 0; i < 5; i++) {
                    CTxDestination addr;
                    ExtractDestination(p->GetPaidPayee(), addr);
                    LogPrint(BCLog::MASTERNODE, "%s - %d %s %s\n", __func__, p->nHeight, p->GetBlockHash().ToString(), EncodeDestination(addr));
                    p = p->pprev;
                }
                LogPrint(BCLog::MASTERNODE,"Invalid mn payment detected %s\n", txNew.ToString().c_str());
            }

            return false;
        }

        return true;
    } else {
        LogPrint(BCLog::MASTERNODE, "%s - Missing required payment of %s\n", __func__, FormatMoney(requiredMasternodePayment));

        return false;
    }

    return false;
}
