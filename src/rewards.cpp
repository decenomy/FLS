// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021-2024 The DECENOMY Core Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "fs.h"
#include "logging.h"
#include "masternode.h"
#include "masternode-sync.h"
#include "rewards.h"
#include "sqlite3/sqlite3.h"
#include "timedata.h"
#include "utilmoneystr.h"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <unordered_map>

std::unordered_map<int, CAmount> mDynamicRewards;

sqlite3* db = nullptr;
sqlite3_stmt* insertStmt = nullptr;
sqlite3_stmt* deleteStmt = nullptr;

bool CRewards::Init(bool fReindex)
{
    std::ostringstream oss;
    auto ok = true;

    try
    {
        const std::string dirname = (GetDataDir() / "chainstate").string();
        const std::string filename = (GetDataDir() / "chainstate" / "rewards.db").string();

        // Create the chainstate directory if it doesn't exist
        if (!fs::exists(dirname.c_str())) {
            // Directory doesn't exist, create it
            if (fs::create_directory(dirname.c_str())) {
                oss << "CRewards::" << __func__ << " Created directory: " << dirname << std::endl;
            } else {
                oss << "CRewards::" << __func__ << " Failed to create directory: " << dirname << std::endl;
                ok = false;
            }
        }

        // Delete the database file if it exists when reindexing
        if(ok && fReindex) 
        {
            if (auto file = std::fopen(filename.c_str(), "r")) {
                std::fclose(file);
                // File exists, delete it
                if (std::remove(filename.c_str()) == 0) {
                    oss << "CRewards::" << __func__ << " Deleted existing database file: " << filename << std::endl;
                } else {
                    oss << "CRewards::" << __func__ << " Failed to delete existing database file: " << filename << std::endl;
                    ok = false;
                }
            }
        }

        if(ok) { // Create and/or open the database
            oss << "CRewards::" << __func__ << " Opening database: " << filename << std::endl;
            auto rc = sqlite3_open(filename.c_str(), &db);

            if (rc) 
            {
                oss << "CRewards::" << __func__ << " Can't open database: " << sqlite3_errmsg(db) << std::endl;
                ok = false;
            }
        }

        if(ok) { // database is open and working
            // Create the rewards table if not exists
            const auto create_table_query = "CREATE TABLE IF NOT EXISTS rewards (height INT PRIMARY KEY, amount INTEGER)";
            auto rc = sqlite3_exec(db, create_table_query, NULL, NULL, NULL);

            if (rc != SQLITE_OK) {
                oss << "CRewards::" << __func__ << " SQL error: " << sqlite3_errmsg(db) << std::endl;
                ok = false;
            }
        }

        if(ok) { // Create insert statement
            const std::string insertSql = "INSERT OR REPLACE INTO rewards (height, amount) VALUES (?, ?)";
            auto rc = sqlite3_prepare_v2(db, insertSql.c_str(), insertSql.length(), &insertStmt, nullptr);
            if (rc != SQLITE_OK) {
                oss << "CRewards::" << __func__ << " SQL error: " << sqlite3_errmsg(db) << std::endl;
                ok = false;
            }
        }

        if(ok) { // Create delete statement
            const std::string deleteSql = "DELETE FROM rewards WHERE height >= ?";
            auto rc = sqlite3_prepare_v2(db, deleteSql.c_str(), deleteSql.length(), &deleteStmt, nullptr);
            if (rc != SQLITE_OK) {
                oss << "CRewards::" << __func__ << " SQL error: " << sqlite3_errmsg(db) << std::endl;
                ok = false;
            }
        }

        if(ok) { // Loads the database into the in-memory map
            const char* sql = "SELECT height, amount FROM rewards";
            auto rc = sqlite3_exec(db, sql, [](void* data, int argc, char** argv, char** /* azColName */) -> int {
                int height = std::stoi(argv[0]);
                CAmount amount = std::stoll(argv[1]);
                mDynamicRewards[height] = amount;
                return 0;
            }, nullptr, nullptr);

            if (rc != SQLITE_OK) {
                oss << "CRewards::" << __func__ <<  " SQL error: " << sqlite3_errmsg(db) << std::endl;
                ok = false;
            }
        }

        if(ok && mDynamicRewards.size() > 0) { // Printing the map
            oss << "CRewards::" << __func__ <<  " Dynamic Rewards:" << std::endl;
            for (const auto& pair : mDynamicRewards) {
                oss << "CRewards::" << __func__ <<  " Height: " << pair.first << ", Amount: " << FormatMoney(pair.second) << std::endl;
            }
        }
    }
    catch(const std::exception& e)
    {
        oss << "CRewards::" << __func__ << " An exception was thrown: " << e.what() << std::endl;
        ok = false;
    }

    if (!oss.str().empty()) LogPrintf("%s: %s", __func__, oss.str());
    
    return ok;
}

void CRewards::Shutdown()
{
    if(insertStmt != nullptr) sqlite3_finalize(insertStmt);
    if(deleteStmt != nullptr) sqlite3_finalize(deleteStmt);
    if(db != nullptr) sqlite3_close(db);

    return;
}

int CRewards::GetDynamicRewardsEpoch(int nHeight)
{
    auto& consensus = Params().GetConsensus();
    const auto nRewardAdjustmentInterval = consensus.nRewardAdjustmentInterval;
    return nHeight / nRewardAdjustmentInterval;
}

int CRewards::GetDynamicRewardsEpochHeight(int nHeight)
{
    auto& consensus = Params().GetConsensus();
    const auto nRewardAdjustmentInterval = consensus.nRewardAdjustmentInterval;
    return GetDynamicRewardsEpoch(nHeight) * nRewardAdjustmentInterval;
}

bool CRewards::IsDynamicRewardsEpochHeight(int nHeight)
{
    return GetDynamicRewardsEpochHeight(nHeight) == nHeight;
}

bool CRewards::ConnectBlock(CBlockIndex* pindex, CAmount nSubsidy, CCoinsViewCache& coins)
{
    auto& consensus = Params().GetConsensus();
    const auto nHeight = pindex->nHeight;
    const auto nEpochHeight = GetDynamicRewardsEpochHeight(nHeight);
    std::ostringstream oss;
    auto ok = true;

    if (consensus.NetworkUpgradeActive(nHeight, Consensus::UPGRADE_DYNAMIC_REWARDS))
    {
        CAmount nNewSubsidy = 0;

        if (
            masternodeSync.IsSynced() &&
            IsDynamicRewardsEpochHeight(nHeight)  
        ) {
            auto nBlocksPerDay = DAY_IN_SECONDS / consensus.nTargetSpacing;
            auto nBlocksPerWeek = WEEK_IN_SECONDS / consensus.nTargetSpacing;
            auto nBlocksPerMonth = MONTH_IN_SECONDS / consensus.nTargetSpacing;

            // get total money supply
            const auto nMoneySupply = pindex->nMoneySupply.get();
            oss << "CRewards::" << __func__ << " nMoneySupply: " << FormatMoney(nMoneySupply) << std::endl;

            // get the current masternode collateral, and the next week collateral
            auto nCollateralAmount = CMasternode::GetMasternodeNodeCollateral(nHeight);
            auto nNextWeekCollateralAmount = CMasternode::GetMasternodeNodeCollateral(nHeight + nBlocksPerWeek);

            // calculate the current circulating supply
            CAmount nCirculatingSupply = 0;
            std::unique_ptr<CCoinsViewCursor> pcursor(coins.Cursor());

            while (pcursor->Valid()) {
                COutPoint key;
                Coin coin;
                if (pcursor->GetKey(key) && pcursor->GetValue(coin)) {
                    // ----------- burn address scanning -----------
                    CTxDestination source;
                    if (ExtractDestination(coin.out.scriptPubKey, source)) {
                        const std::string addr = EncodeDestination(source);
                        if (consensus.mBurnAddresses.find(addr) != consensus.mBurnAddresses.end() &&
                            consensus.mBurnAddresses.at(addr) < nHeight
                        ) {
                            pcursor->Next(); // Skip
                            continue;
                        }
                    }

                    // ----------- masternode collaterals scanning ----------- 
                    if(
                        coin.out.nValue == nCollateralAmount || 
                        coin.out.nValue == nNextWeekCollateralAmount
                    ) {
                        pcursor->Next(); // Skip
                        continue;
                    }

                    // ----------- UTXOs age related scanning -----------
                    auto nBlocksDiff = static_cast<int64_t>(nHeight - coin.nHeight);
                    const auto nMultiplier = 100000000L;

                    // y = mx + b 
                    // 3 months old or less => 100%
                    // 12 months old or greater => 0%
                    const auto nSupplyWeightRatio = 
                        std::min(
                            std::max(
                                (100LL * nMultiplier - (((100LL * nMultiplier)/(9LL * nBlocksPerMonth)) * (nBlocksDiff - 3LL * nBlocksPerMonth))) / nMultiplier, 
                            0LL), 
                        100LL);

                    nCirculatingSupply += coin.out.nValue * nSupplyWeightRatio / 100L;
                }

                pcursor->Next();
            }
            oss << "CRewards::" << __func__ << " nCirculatingSupply: " << FormatMoney(nCirculatingSupply) << std::endl;

            // calculate target emissions
            const auto nRewardAdjustmentInterval = consensus.nRewardAdjustmentInterval;
            oss << "CRewards::" << __func__ << " nRewardAdjustmentInterval: " << nRewardAdjustmentInterval << std::endl;
            const auto nTotalEmissionRate = sporkManager.GetSporkValue(SPORK_116_TOT_SPLY_TRGT_EMISSION);
            oss << "CRewards::" << __func__ << " nTotalEmissionRate: " << nTotalEmissionRate << std::endl;
            const auto nCirculatingEmissionRate = sporkManager.GetSporkValue(SPORK_117_CIRC_SPLY_TRGT_EMISSION);
            oss << "CRewards::" << __func__ << " nCirculatingEmissionRate: " << nCirculatingEmissionRate << std::endl;
            const auto nActualEmission = nSubsidy * nRewardAdjustmentInterval;
            oss << "CRewards::" << __func__ << " nActualEmission: " << FormatMoney(nActualEmission) << std::endl;
            const auto nSupplyTargetEmission = ((nMoneySupply / (365L * nBlocksPerDay)) / 1000000) * nTotalEmissionRate * nRewardAdjustmentInterval;
            oss << "CRewards::" << __func__ << " nSupplyTargetEmission: " << FormatMoney(nSupplyTargetEmission) << std::endl;
            const auto nCirculatingTargetEmission = ((nCirculatingSupply / (365L * nBlocksPerDay)) / 1000000) * nCirculatingEmissionRate * nRewardAdjustmentInterval;
            oss << "CRewards::" << __func__ << " nCirculatingTargetEmission: " << FormatMoney(nCirculatingTargetEmission) << std::endl;

            // calculate required delta values
            const auto nDelta = (nActualEmission - std::max(nSupplyTargetEmission, nCirculatingTargetEmission)) / nRewardAdjustmentInterval;
            oss << "CRewards::" << __func__ << " nDelta: " << FormatMoney(nDelta) << std::endl;
            
            // y = mx + b
            // <= 0% |ratio| => 1%
            // >= 100% |ratio| => 10%
            
            const auto nRatio = std::llabs((nDelta * 100) / nSubsidy); // percentage of the difference on emissions and the current reward 
            oss << "CRewards::" << __func__ << " nRatio: " << nRatio << std::endl;

            const auto nWeightRatio = ((std::min(nRatio, 100LL) * 9LL) / 100LL) + 1LL;

            const auto nDampedDelta = nDelta * nWeightRatio / 100LL;
            oss << "CRewards::" << __func__ << " nDampedDelta: " << FormatMoney(nDampedDelta) << std::endl;

            // adjust the reward for this epoch
            nNewSubsidy = nSubsidy - nDampedDelta;

            oss << "CRewards::" << __func__ << " Adjustment at height " << nHeight << ": " << FormatMoney(nSubsidy) << " => " << FormatMoney(nNewSubsidy) << std::endl;
        }

        if ( // if the wallet is syncing get the reward value from the blocks of the epoch
            !masternodeSync.IsSynced() &&
            nHeight != nEpochHeight && 
            mDynamicRewards.find(nEpochHeight) == mDynamicRewards.end()
        ) {
            nNewSubsidy = nSubsidy;
        }

        if(ok && nNewSubsidy > 0) { // store it
            mDynamicRewards[nEpochHeight] = nNewSubsidy; // on the in-memory map

            sqlite3_bind_int(insertStmt, 1, nEpochHeight); // on the file database
            sqlite3_bind_int64(insertStmt, 2, nNewSubsidy);
            auto rc = sqlite3_step(insertStmt);
            if (rc != SQLITE_DONE) {
                oss << "CRewards::" << __func__ <<  " SQL error: " << sqlite3_errmsg(db) << std::endl;
                ok = false;
            }
            sqlite3_reset(insertStmt);
        }
    }

    if (!oss.str().empty()) LogPrintf("%s: %s", __func__, oss.str());

    return ok;
}

bool CRewards::DisconnectBlock(CBlockIndex* pindex)
{
    auto& consensus = Params().GetConsensus();
    const auto nHeight = pindex->nHeight;
    std::ostringstream oss;
    auto ok = true;
    
    try
    {
        if (consensus.NetworkUpgradeActive(nHeight, Consensus::UPGRADE_DYNAMIC_REWARDS) &&
            IsDynamicRewardsEpochHeight(nHeight)
        ) {
            auto it = mDynamicRewards.find(nHeight);
            if (it != mDynamicRewards.end()) {
                // delete it
                mDynamicRewards.erase(it); // on the in-memory map

                sqlite3_bind_int(deleteStmt, 1, nHeight); // on the file database
                auto rc = sqlite3_step(deleteStmt);
                if (rc != SQLITE_DONE) {
                    oss << "CRewards::" << __func__ <<  " SQL error: " << sqlite3_errmsg(db) << std::endl;
                    ok = false;
                }
                sqlite3_reset(deleteStmt);
            }
        }
    } 
    catch(const std::exception& e)
    {
        oss << "CRewards::" << __func__ << " An exception was thrown: " << e.what() << std::endl;
        ok = false;
    }

    if (!oss.str().empty()) LogPrintf("%s: %s", __func__, oss.str());
    
    return ok;
}

CAmount CRewards::GetBlockValue(int nHeight)
{
    auto& consensus = Params().GetConsensus();

    CAmount nSubsidy = 0;

    if (nHeight >= 1 && nHeight < 100001)
        nSubsidy = 45 * COIN;
    else if (nHeight >= 100001 && nHeight < 200001)
        nSubsidy = 40 * COIN;
    else if (nHeight >= 200001 && nHeight < 300001)
        nSubsidy = 40 * COIN;
    else if (nHeight >= 300001 && nHeight < 400001)
        nSubsidy = 40 * COIN;
    else if (nHeight >= 400001 && nHeight < 500001)
        nSubsidy = 40 * COIN;
    else if (nHeight >= 500001 && nHeight < 600001)
        nSubsidy = 35 * COIN;
    else if (nHeight >= 600001 && nHeight < 700001)
        nSubsidy = 35 * COIN;
    else if (nHeight >= 700001 && nHeight < 800001)
        nSubsidy = 35 * COIN;
    else if (nHeight >= 800001 && nHeight < 900001)
        nSubsidy = 30 * COIN;
    else if (nHeight >= 900001 && nHeight < 1000001)
        nSubsidy = 30 * COIN;
    else if (nHeight >= 1000001 && nHeight < 1100001)
        nSubsidy = 25 * COIN;
    else if (nHeight >= 1100001 && nHeight < 1200001)
        nSubsidy = 25 * COIN;
    else if (nHeight >= 1200001 && nHeight < 1300001)
        nSubsidy = 25 * COIN;
    else if (nHeight >= 1300001 && nHeight < 1400001)
        nSubsidy = 20 * COIN;
    else if (nHeight >= 1400001 && nHeight < 1500001)
        nSubsidy = 20 * COIN;
    else if (nHeight >= 1500001 && nHeight < 1600001)
        nSubsidy = 20 * COIN;
    else if (nHeight >= 1600001 && nHeight < 1700001)
        nSubsidy = 15 * COIN;
    else if (nHeight >= 1700001 && nHeight < 1800001)
        nSubsidy = 15 * COIN;
    else if (nHeight >= 1800001 && nHeight < 1900001)
        nSubsidy = 15 * COIN;
    else if (nHeight >= 1900001 && nHeight < 2000001)
        nSubsidy = 15 * COIN;
    else if (nHeight >= 2000001 && nHeight < 2100001)
        nSubsidy = 10 * COIN;
    else if (nHeight >= 2100001 && nHeight < 2200001)
        nSubsidy = 10 * COIN;
    else if (nHeight >= 2200001 && nHeight < 2300001)
        nSubsidy = 10 * COIN;
    else if (nHeight >= 2300001 && nHeight < 2400001)
        nSubsidy = 8 * COIN;
    else if (nHeight >= 2400001 && nHeight < 2500001)
        nSubsidy = 8 * COIN;
    else if (nHeight >= 2500001 && nHeight < 2600001)
        nSubsidy = 8 * COIN;
    else if (nHeight >= 2600001 && nHeight < 2700001)
        nSubsidy = 6 * COIN;
    else if (nHeight >= 2700001 && nHeight < 2800001)
        nSubsidy = 6 * COIN;
    else if (nHeight >= 2800001 && nHeight < 2900001)
        nSubsidy = 6 * COIN;
    else if (nHeight >= 2900001 && nHeight < 3000001)
        nSubsidy = 4 * COIN;
    else if (nHeight >= 3000001 && nHeight < 3100001)
        nSubsidy = 4 * COIN;
    else if (nHeight >= 3100001 && nHeight < 3200001)
        nSubsidy = 4 * COIN;
    else if (nHeight >= 3200001 && nHeight < 3300001)
        nSubsidy = 4 * COIN;
    else if (nHeight >= 3300001 && nHeight < 3400001)
        nSubsidy = 3 * COIN;
    else if (nHeight >= 3400001 && nHeight < 3500001)
        nSubsidy = 3 * COIN;
    else if (nHeight >= 3500001 && nHeight < 3600001)
        nSubsidy = 3 * COIN;
    else
        nSubsidy = 2 * COIN;

    // loose the verification to allow the initial swap emission
    if (nHeight <= 15) nSubsidy = 13000000 * COIN;

    // ---- Static reward table ----

    if (masternodeSync.IsSynced() &&
        consensus.NetworkUpgradeActive(nHeight, Consensus::UPGRADE_DYNAMIC_REWARDS)
    ) {
        // if this is the block where calculations are made on ConnectBlock
        // return the reward value from the previous block
        if(IsDynamicRewardsEpochHeight(nHeight)) 
            return GetBlockValue(nHeight - 1);

        // find and return the dynamic reward
        const auto nEpochHeight = GetDynamicRewardsEpochHeight(nHeight);
        auto it = mDynamicRewards.find(nEpochHeight);
        if (it != mDynamicRewards.end()) {
            return std::min(nSubsidy, it->second);
        }
    }

    // fallback non-dynamic reward return
    return nSubsidy;
}