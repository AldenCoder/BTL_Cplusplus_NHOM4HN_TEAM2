/**
 * @file WalletManager.cpp
 * @brief Triển khai quản lý ví điểm thưởng và giao dịch
 * @author Team 2C
 */

#include "WalletManager.h"
#include "../security/OTPManager.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

const double WalletManager::INITIAL_USER_POINTS = 100.0; // 100 điểm ban đầu

WalletManager::WalletManager(std::shared_ptr<DatabaseManager> dataManager, 
                            std::shared_ptr<OTPManager> otpManager)
    : dataManager(dataManager), otpManager(otpManager) {
}

bool WalletManager::initialize() {
    try {
        std::cout << "[DEBUG] Starting WalletManager initialization..." << std::endl;
        
        // Initialize master wallet
        masterWallet = std::unique_ptr<MasterWallet>(new MasterWallet(10000000.0)); // 10 million initial points
        std::cout << "[DEBUG] Master wallet created successfully" << std::endl;
        
        // Load all wallets from storage into cache
        auto wallets = dataManager->loadAllWallets();
        std::cout << "[DEBUG] Loaded " << wallets.size() << " wallets from storage" << std::endl;
        
        for (auto& wallet : wallets) {
            if (wallet) {
                std::string walletId = wallet->getId();
                std::cout << "[DEBUG] Processing wallet " << walletId << std::endl;
                
                // Wallet is already shared_ptr from DatabaseManager
                walletCache[walletId] = wallet;
                
                std::cout << "[DEBUG] Added wallet " << walletId << " to cache" << std::endl;
            } else {
                std::cout << "[DEBUG] Warning: null wallet found in loadAllWallets result" << std::endl;
            }
        }

        std::cout << "[DEBUG] WalletManager initialization completed successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] WalletManager initialization failed: " << e.what() << std::endl;
        return false;
    }
}

bool WalletManager::createUserWallet(const std::string& userId, const std::string& walletId) {
    try {
        // Kiểm tra ví đã tồn tại chưa
        if (walletExists(walletId)) {
            return false;
        }        // Tạo ví mới với số điểm khởi tạo
        auto wallet = std::shared_ptr<Wallet>(new Wallet(walletId, userId, INITIAL_USER_POINTS));
        // ID is already set in constructor

        // Lưu vào storage
        if (dataManager->saveWallet(wallet)) {
            // Thêm vào cache
            walletCache[walletId] = wallet;
            
            // Ghi log giao dịch phát hành điểm ban đầu
            Transaction initTransaction(
                SecurityUtils::generateUUID(),
                "MASTER",
                walletId,
                INITIAL_USER_POINTS,
                TransactionType::TRANSFER,
                TransactionStatus::COMPLETED,
                "Điểm khởi tạo cho user mới"
            );
            wallet->addTransaction(initTransaction);
            
            return true;
        }
        
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi tạo ví: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<Wallet> WalletManager::getWallet(const std::string& walletId) {
    // Kiểm tra cache trước
    auto it = walletCache.find(walletId);
    if (it != walletCache.end()) {
        return it->second;
    }

    // Tải từ storage
    return loadWalletToCache(walletId);
}

std::shared_ptr<Wallet> WalletManager::getWalletByUserId(const std::string& userId) {
    try {
        // Tìm trong cache trước
        for (const auto& pair : walletCache) {
            if (pair.second->getOwnerId() == userId) {
                return pair.second;
            }
        }

        // Tải từ storage
        return dataManager->loadWalletByOwnerId(userId);
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi tìm ví theo user ID: " << e.what() << std::endl;
        return nullptr;
    }
}

TransferResult WalletManager::transferPoints(const TransferRequest& request) {
    // std::lock_guard<std::mutex> lock(transferMutex); // Thread-safe - disabled for MinGW
    
    TransferResult result;
    result.success = false;

    // Xác thực yêu cầu
    std::string validationError = validateTransferRequest(request);
    if (!validationError.empty()) {
        result.message = validationError;
        return result;
    }

    try {
        // Lấy ví gửi và nhận
        auto fromWallet = getWallet(request.fromWalletId);
        auto toWallet = getWallet(request.toWalletId);

        if (!fromWallet || !toWallet) {
            result.message = "Không tìm thấy ví!";
            return result;
        }        // Kiểm tra ví có bị khóa không
        if (fromWallet->getIsLocked() || toWallet->getIsLocked()) {
            result.message = "Ví đã bị khóa!";
            return result;
        }

        // Kiểm tra số dư
        if (fromWallet->getBalance() < request.amount) {
            result.message = "Số dư không đủ!";
            return result;
        }

        // Xác thực OTP
        if (!otpManager->verifyOTP(fromWallet->getOwnerId(), request.otpCode, OTPType::TRANSFER)) {
            result.message = "Mã OTP không đúng hoặc đã hết hạn!";
            return result;
        }

        // Thực hiện giao dịch atomic
        std::string transactionId = executeAtomicTransfer(
            fromWallet, toWallet, request.amount, request.description);

        if (!transactionId.empty()) {
            result.success = true;
            result.message = "Chuyển điểm thành công!";
            result.transactionId = transactionId;
            result.newBalance = fromWallet->getBalance();

            // Lưu vào storage
            dataManager->saveWallet(fromWallet);
            dataManager->saveWallet(toWallet);
        } else {
            result.message = "Lỗi thực hiện giao dịch!";
        }
    }
    catch (const std::exception& e) {
        result.message = "Lỗi hệ thống: " + std::string(e.what());
    }

    return result;
}

std::string WalletManager::generateTransferOTP(const std::string& fromUserId,
                                              const std::string& toWalletId,
                                              double amount) {
    try {
        // Tạo OTP với thông tin giao dịch
        std::string otpData = fromUserId + "|" + toWalletId + "|" + std::to_string(amount);
        return otpManager->generateOTP(fromUserId, OTPType::TRANSFER);
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi tạo OTP chuyển điểm: " << e.what() << std::endl;
        return "";
    }
}

double WalletManager::getBalance(const std::string& walletId) {
    auto wallet = getWallet(walletId);
    return wallet ? wallet->getBalance() : -1.0;
}

std::vector<Transaction> WalletManager::getTransactionHistory(const std::string& walletId, 
                                                             int limit) {
    std::vector<Transaction> transactions;
    
    auto wallet = getWallet(walletId);
    if (!wallet) {
        return transactions;
    }

    auto allTransactions = wallet->getTransactionHistory();
    
    // Sắp xếp theo thời gian mới nhất
    std::sort(allTransactions.begin(), allTransactions.end(),
              [](const Transaction& a, const Transaction& b) {
                  return a.getTimestamp() > b.getTimestamp();
              });

    // Giới hạn số lượng nếu cần
    if (limit > 0 && allTransactions.size() > static_cast<size_t>(limit)) {
        allTransactions.resize(limit);
    }

    return allTransactions;
}

std::vector<Transaction> WalletManager::getTransactionHistoryByDate(
    const std::string& walletId,
    const std::chrono::system_clock::time_point& fromDate,
    const std::chrono::system_clock::time_point& toDate) {
    
    std::vector<Transaction> filteredTransactions;
    auto allTransactions = getTransactionHistory(walletId);

    for (const auto& transaction : allTransactions) {
        auto transactionTime = transaction.getTimestamp();
        if (transactionTime >= fromDate && transactionTime <= toDate) {
            filteredTransactions.push_back(transaction);
        }
    }

    return filteredTransactions;
}

std::vector<std::string> WalletManager::findWalletsByOwner(const std::string& ownerId) {
    std::vector<std::string> walletIds;
    
    try {
        // Tìm trong cache
        for (const auto& pair : walletCache) {
            if (pair.second->getOwnerId() == ownerId) {
                walletIds.push_back(pair.first);
            }
        }

        // Nếu không tìm thấy trong cache, tìm trong storage
        if (walletIds.empty()) {
            auto wallet = dataManager->loadWalletByOwnerId(ownerId);
            if (wallet) {
                walletIds.push_back(wallet->getId());
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi tìm ví theo owner: " << e.what() << std::endl;
    }

    return walletIds;
}

bool WalletManager::walletExists(const std::string& walletId) {
    return getWallet(walletId) != nullptr;
}

bool WalletManager::setWalletLocked(const std::string& walletId, bool locked) {
    try {
        auto wallet = getWallet(walletId);
        if (!wallet) {
            return false;
        }

        wallet->setLocked(locked);
        return dataManager->saveWallet(wallet);
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi khóa/mở ví: " << e.what() << std::endl;
        return false;
    }
}

std::string WalletManager::issuePointsFromMaster(const std::string& toWalletId,
                                                double amount,
                                                const std::string& description) {
    // std::lock_guard<std::mutex> lock(transferMutex); // disabled for MinGW
    
    try {
        auto toWallet = getWallet(toWalletId);
        if (!toWallet || toWallet->getIsLocked()) {
            return "";
        }

        // Kiểm tra ví tổng có đủ điểm không
        if (!masterWallet->hasEnoughPoints(amount)) {
            return "";
        }

        // Tạo giao dịch
        std::string transactionId = SecurityUtils::generateUUID();
        Transaction transaction(
            transactionId,
            "MASTER",
            toWalletId,
            amount,
            TransactionType::TRANSFER,
            TransactionStatus::COMPLETED,
            description
        );

        // Thực hiện giao dịch
        if (masterWallet->transferOut(amount) && toWallet->deposit(amount)) {
            toWallet->addTransaction(transaction);
            dataManager->saveWallet(toWallet);
            
            logTransaction(transaction, "COMPLETED", "Admin issued points successfully");
            return transactionId;
        }

        return "";
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi phát hành điểm: " << e.what() << std::endl;
        return "";
    }
}

std::string WalletManager::getSystemStatistics() {
    try {
        std::ostringstream stats;
        
        // Thống kê tổng quan
        int totalWallets = walletCache.size();
        double totalPoints = 0.0;
        int activeWallets = 0;
        int lockedWallets = 0;

        for (const auto& pair : walletCache) {
            auto wallet = pair.second;
            totalPoints += wallet->getBalance();
            
            if (wallet->getIsLocked()) {
                lockedWallets++;
            } else {
                activeWallets++;
            }
        }

        stats << "===== THỐNG KÊ HỆ THỐNG VÍ =====\n";
        stats << "Tổng số ví: " << totalWallets << "\n";
        stats << "Ví đang hoạt động: " << activeWallets << "\n";
        stats << "Ví bị khóa: " << lockedWallets << "\n";
        stats << "Tổng điểm trong hệ thống: " << std::fixed << std::setprecision(2) << totalPoints << "\n";
        stats << "Điểm còn lại trong ví tổng: " << masterWallet->getTotalPoints() << "\n";
        
        return stats.str();
    }
    catch (const std::exception& e) {
        return "Lỗi lấy thống kê: " + std::string(e.what());
    }
}

bool WalletManager::confirmPendingTransaction(const std::string& transactionId,
                                            const std::string& otpCode) {
    // Tính năng này có thể được mở rộng cho giao dịch đang chờ xử lý
    // Hiện tại hệ thống xử lý giao dịch ngay lập tức
    return false;
}

bool WalletManager::cancelPendingTransaction(const std::string& transactionId,
                                           const std::string& reason) {
    // Tính năng này có thể được mở rộng cho giao dịch đang chờ xử lý
    return false;
}

bool WalletManager::saveAllWallets() {
    try {
        for (const auto& pair : walletCache) {
            if (!dataManager->saveWallet(pair.second)) {
                return false;
            }
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi lưu tất cả ví: " << e.what() << std::endl;
        return false;
    }
}

void WalletManager::clearWalletCache() {
    walletCache.clear();
}

// Private methods

std::shared_ptr<Wallet> WalletManager::loadWalletToCache(const std::string& walletId) {
    try {
        auto wallet = dataManager->loadWallet(walletId);
        if (wallet) {
            walletCache[walletId] = wallet;
        }
        return wallet;
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi tải ví vào cache: " << e.what() << std::endl;
        return nullptr;
    }
}

void WalletManager::removeWalletFromCache(const std::string& walletId) {
    walletCache.erase(walletId);
}

std::string WalletManager::validateTransferRequest(const TransferRequest& request) {
    // Kiểm tra số điểm
    if (request.amount <= 0) {
        return "Số điểm phải lớn hơn 0!";
    }

    if (request.amount > 1000000) { // Giới hạn 1 triệu điểm/giao dịch
        return "Số điểm vượt quá giới hạn cho phép!";
    }

    // Kiểm tra ví gửi và nhận khác nhau
    if (request.fromWalletId == request.toWalletId) {
        return "Không thể chuyển điểm cho chính mình!";
    }

    // Kiểm tra OTP
    if (request.otpCode.empty()) {
        return "Cần mã OTP để xác thực giao dịch!";
    }

    // Kiểm tra mô tả
    if (request.description.empty()) {
        return "Cần có mô tả cho giao dịch!";
    }

    return ""; // Hợp lệ
}

std::string WalletManager::executeAtomicTransfer(std::shared_ptr<Wallet> fromWallet,
                                               std::shared_ptr<Wallet> toWallet,
                                               double amount,
                                               const std::string& description) {
    try {
        // Tạo ID giao dịch
        std::string transactionId = SecurityUtils::generateUUID();

        // Backup số dư trước khi thực hiện
        double fromBalanceBackup = fromWallet->getBalance();
        double toBalanceBackup = toWallet->getBalance();

        // Thực hiện giao dịch
        bool withdrawSuccess = fromWallet->withdraw(amount);
        if (!withdrawSuccess) {
            return "";
        }

        bool depositSuccess = toWallet->deposit(amount);
        if (!depositSuccess) {
            // Rollback withdrawal
            fromWallet->deposit(amount);
            return "";
        }

        // Tạo giao dịch cho ví gửi (trừ tiền)
        Transaction fromTransaction(
            transactionId,
            fromWallet->getId(),
            toWallet->getId(),
            amount,
            TransactionType::TRANSFER,
            TransactionStatus::COMPLETED,
            description
        );

        // Tạo giao dịch cho ví nhận (cộng tiền)
        Transaction toTransaction(
            transactionId,
            fromWallet->getId(),
            toWallet->getId(),
            amount,
            TransactionType::TRANSFER,
            TransactionStatus::COMPLETED,
            description
        );

        // Thêm giao dịch vào lịch sử
        fromWallet->addTransaction(fromTransaction);
        toWallet->addTransaction(toTransaction);

        // Log giao dịch
        logTransaction(fromTransaction, "COMPLETED", "Transfer executed successfully");

        return transactionId;
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi thực hiện giao dịch atomic: " << e.what() << std::endl;
        return "";
    }
}

void WalletManager::rollbackTransfer(std::shared_ptr<Wallet> fromWallet,
                                   std::shared_ptr<Wallet> toWallet,
                                   double amount,
                                   const std::string& transactionId) {
    try {
        // Hoàn tác giao dịch
        toWallet->withdraw(amount);
        fromWallet->deposit(amount);

        // Tạo giao dịch rollback
        Transaction rollbackTransaction(
            SecurityUtils::generateUUID(),
            "SYSTEM",
            "SYSTEM",
            amount,
            TransactionType::ROLLBACK,
            TransactionStatus::COMPLETED,
            "Rollback transaction " + transactionId
        );

        fromWallet->addTransaction(rollbackTransaction);
        toWallet->addTransaction(rollbackTransaction);

        logTransaction(rollbackTransaction, "ROLLBACK", "Transaction rolled back");
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi rollback giao dịch: " << e.what() << std::endl;
    }
}

void WalletManager::logTransaction(const Transaction& transaction,
                                 const std::string& status,
                                 const std::string& message) {
    try {
        // Ghi log vào file hoặc database
        // Hiện tại chỉ ghi ra console
        std::cout << "[TRANSACTION LOG] " 
                  << "ID: " << transaction.getId()
                  << ", Status: " << status
                  << ", Amount: " << transaction.getAmount()
                  << ", Message: " << message << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Lỗi ghi log giao dịch: " << e.what() << std::endl;
    }
}
