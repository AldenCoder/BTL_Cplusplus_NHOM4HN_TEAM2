#include <iostream>
#include "Wallet.h"
#include "../security/SecurityUtils.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

Transaction::Transaction(const std::string& fromId, const std::string& toId, 
                        double amt, TransactionType t, const std::string& desc)
    : fromWalletId(fromId), toWalletId(toId), amount(amt), type(t), 
      status(TransactionStatus::PENDING), description(desc),
      timestamp(std::chrono::system_clock::now()) {
    
    transactionId = SecurityUtils::generateUUID();
}

Transaction::Transaction(const std::string& id, const std::string& fromId, const std::string& toId,
                        double amt, TransactionType t, TransactionStatus stat, const std::string& desc)
    : transactionId(id), fromWalletId(fromId), toWalletId(toId), amount(amt), 
      type(t), status(stat), description(desc),
      timestamp(std::chrono::system_clock::now()) {
}

std::string Transaction::toJson() const {
    std::ostringstream json;
    
    json << "{\n";
    json << "  \"transactionId\": \"" << transactionId << "\",\n";
    json << "  \"fromWalletId\": \"" << fromWalletId << "\",\n";
    json << "  \"toWalletId\": \"" << toWalletId << "\",\n";
    json << "  \"amount\": " << amount << ",\n";
    json << "  \"type\": " << static_cast<int>(type) << ",\n";
    json << "  \"status\": " << static_cast<int>(status) << ",\n";
    json << "  \"description\": \"" << description << "\",\n";
    json << "  \"otpUsed\": \"" << otpUsed << "\",\n";
    
    auto timeT = std::chrono::duration_cast<std::chrono::seconds>(
        timestamp.time_since_epoch()).count();
    json << "  \"timestamp\": " << timeT << "\n";
    json << "}";
    
    return json.str();
}

Transaction Transaction::fromJson(const std::string& json) {
    (void)json;
    Transaction t("", "", 0.0, TransactionType::TRANSFER_OUT);
    return t;
}

Wallet::Wallet(const std::string& walletId, const std::string& ownerId, 
               double initialBalance)
    : walletId(walletId), ownerId(ownerId), balance(initialBalance),
      createdAt(std::chrono::system_clock::now()), isLocked(false) {
    
    if (initialBalance > 0) {
        Transaction initTransaction("SYSTEM", walletId, initialBalance, 
                                  TransactionType::INITIAL, 
                                  "Initial balance");
        initTransaction.status = TransactionStatus::COMPLETED;
        transactions.push_back(initTransaction);
    }
}

bool Wallet::hasSufficientBalance(double amount) const {
    return !isLocked && balance >= amount && amount > 0;
}

bool Wallet::deposit(double amount) {
    if (amount <= 0 || isLocked) {
        return false;
    }
    balance += amount;
    return true;
}

bool Wallet::withdraw(double amount) {
    if (!hasSufficientBalance(amount)) {
        return false;
    }
    balance -= amount;
    return true;
}

std::string Wallet::transferOut(double amount, const std::string& toWalletId, 
                               const std::string& description) {
    if (!hasSufficientBalance(amount)) {
        return "";
    }
    
    Transaction transaction(walletId, toWalletId, amount, 
                          TransactionType::TRANSFER_OUT, description);
    
    std::string transactionId = transaction.transactionId;
    
    balance -= amount;
    
    addTransaction(transaction);
    
    return transactionId;
}

void Wallet::receiveTransfer(double amount, const std::string& fromWalletId,
                           const std::string& transactionId, 
                           const std::string& description) {
    Transaction transaction(fromWalletId, walletId, amount, 
                          TransactionType::TRANSFER_IN, description);
    transaction.transactionId = transactionId;
    transaction.status = TransactionStatus::COMPLETED;
    
    balance += amount;
    
    addTransaction(transaction);
}

bool Wallet::cancelTransaction(const std::string& transactionId) {
    auto it = std::find_if(transactions.begin(), transactions.end(),
        [&transactionId](const Transaction& t) {
            return t.transactionId == transactionId;
        });
    
    if (it != transactions.end() && it->status == TransactionStatus::PENDING) {
        if (it->type == TransactionType::TRANSFER_OUT) {
            balance += it->amount;
        }
        
        it->status = TransactionStatus::CANCELLED;
        return true;
    }
    
    return false;
}

bool Wallet::confirmTransaction(const std::string& transactionId, 
                               const std::string& otpCode) {
    auto it = std::find_if(transactions.begin(), transactions.end(),
        [&transactionId](const Transaction& t) {
            return t.transactionId == transactionId;
        });
    
    if (it != transactions.end() && it->status == TransactionStatus::PENDING) {
        it->otpUsed = otpCode;
        it->status = TransactionStatus::COMPLETED;
        return true;
    }
    
    return false;
}

std::vector<Transaction> Wallet::getTransactionHistory(
    const std::chrono::system_clock::time_point& fromDate,
    const std::chrono::system_clock::time_point& toDate) const {
    
    std::vector<Transaction> filtered;
    
    for (const auto& transaction : transactions) {
        if (transaction.timestamp >= fromDate && transaction.timestamp <= toDate) {
            filtered.push_back(transaction);
        }
    }
    
    return filtered;
}

std::string Wallet::toJson() const {
    std::ostringstream json;
    
    json << "{\n";
    json << "  \"walletId\": \"" << walletId << "\",\n";
    json << "  \"ownerId\": \"" << ownerId << "\",\n";
    json << "  \"balance\": " << balance << ",\n";
    json << "  \"isLocked\": " << (isLocked ? "true" : "false") << ",\n";
    
    auto createdTime = std::chrono::duration_cast<std::chrono::seconds>(
        createdAt.time_since_epoch()).count();
    json << "  \"createdAt\": " << createdTime << ",\n";
    
    json << "  \"transactions\": [\n";
    for (size_t i = 0; i < transactions.size(); ++i) {
        json << "    " << transactions[i].toJson();
        if (i < transactions.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ]\n";
    json << "}";
    
    return json.str();
}

std::unique_ptr<Wallet> Wallet::fromJson(const std::string& json) {
    try {
        auto extractString = [&json](const std::string& key) -> std::string {
            std::string searchKey = "\"" + key + "\": \"";
            size_t start = json.find(searchKey);
            if (start == std::string::npos) return "";
            start += searchKey.length();
            size_t end = json.find("\"", start);
            if (end == std::string::npos) return "";
            return json.substr(start, end - start);
        };
        
        auto extractValue = [&json](const std::string& key) -> std::string {
            std::string searchKey = "\"" + key + "\": ";
            size_t start = json.find(searchKey);
            if (start == std::string::npos) return "";
            start += searchKey.length();
            size_t end = json.find_first_of(",}\n", start);
            if (end == std::string::npos) return "";
            return json.substr(start, end - start);
        };
        
        std::string walletId = extractString("walletId");
        std::string ownerId = extractString("ownerId");
        std::string balanceStr = extractValue("balance");
        
        if (walletId.empty() || ownerId.empty()) {
            return nullptr;
        }
        
        double balance = balanceStr.empty() ? 0.0 : std::stod(balanceStr);
        
        auto wallet = std::unique_ptr<Wallet>(new Wallet(walletId, ownerId, balance));
        
        std::string isLockedStr = extractValue("isLocked");
        if (!isLockedStr.empty()) {
            wallet->setLocked(isLockedStr == "true");
        }
        
        std::string createdAtStr = extractValue("createdAt");
        if (!createdAtStr.empty()) {
            auto createdTime = std::chrono::seconds(std::stoll(createdAtStr));
            wallet->createdAt = std::chrono::system_clock::time_point(createdTime);
        }
        
        wallet->transactions.clear();
        
        return wallet;
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing Wallet JSON: " << e.what() << std::endl;
        return nullptr;
    }
}

std::string Wallet::generateTransactionId() {
    return SecurityUtils::generateUUID();
}

void Wallet::addTransaction(const Transaction& transaction) {
    transactions.push_back(transaction);
    
    if (transactions.size() > 1000) {
        transactions.erase(transactions.begin());
    }
}

const std::string MasterWallet::MASTER_WALLET_ID = "MASTER_WALLET_00";
const std::string MasterWallet::MASTER_OWNER_ID = "SYSTEM";

MasterWallet::MasterWallet(double initialSupply) 
    : Wallet(MASTER_WALLET_ID, MASTER_OWNER_ID, initialSupply) {
    setLocked(false);
}

std::string MasterWallet::issuePoints(const std::string& toWalletId, double amount,
                                     const std::string& description) {
    if (amount <= 0) {
        return "";
    }
    
    Transaction transaction(MASTER_WALLET_ID, toWalletId, amount,
                          TransactionType::TRANSFER_OUT, description);
    transaction.status = TransactionStatus::COMPLETED;
    
    balance -= amount;
    
    addTransaction(transaction);
    
    return transaction.transactionId;
}

bool MasterWallet::hasEnoughPoints(double amount) const {
    return balance >= amount && amount > 0;
}

double MasterWallet::getTotalPoints() const {
    return balance;
}

bool MasterWallet::transferOut(double amount) {
    if (!hasEnoughPoints(amount)) {
        return false;
    }
    balance -= amount;
    return true;
}

MasterWallet& MasterWallet::getInstance() {
    static MasterWallet instance;
    return instance;
}
