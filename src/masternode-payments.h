// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MASTERNODE_PAYMENTS_H
#define MASTERNODE_PAYMENTS_H

#include "key.h"
#include "main.h"
#include "masternode.h"
#include "masternodeman.h"

bool IsBlockPayeeValid(const CBlock& block, CBlockIndex* pindexPrev);
bool IsBlockValueValid(int nHeight, CAmount nExpectedValue, CAmount nMinted);
void FillBlockPayee(CMutableTransaction& txNew, const CBlockIndex* pindexPrev, bool fProofOfStake);

//
// Masternode Payments Class
//
class CMasternodePayments
{
public:
    bool GetBlockPayee(const CBlockIndex* pindexPrev, CScript& payee);
    bool IsTransactionValid(const CBlock& block, const CBlockIndex* pindexPrev);
    
    void FillBlockPayee(CMutableTransaction& txNew, const CBlockIndex* pindexPrev, bool fProofOfStake);
};

extern CMasternodePayments masternodePayments;

#endif
