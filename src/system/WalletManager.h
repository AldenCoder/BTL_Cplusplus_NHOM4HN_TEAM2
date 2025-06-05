#ifndef WALLET_MANAGER_H
#define WALLET_MANAGER_H

#include "../models/Wallet.h"
#include "../security/SecurityUtils.h"
#include "../security/OTPManager.h"
#include "../storage/DatabaseManager.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include "../thread_compat.h"

struct TransferRequest {
    std::string fromWalletId;   // ID ví gửi
    std::string toWalletId;     // ID ví nhận
    double amount;              // Số điểm
    std::string description;    // Mô tả giao dịch
    std::string otpCode;        // Mã OTP xác thực
};

/**
 * @struct TransferResult
 * @brief Kết quả chuyển điểm
 */
struct TransferResult {
    bool success;               // Thành công hay không
    std::string message;        // Thông báo kết quả
    std::string transactionId;  // ID giao dịch (nếu thành công)
    double newBalance;          // Số dư mới của ví gửi
};

/**
 * @class WalletManager
 * @brief Quản lý tất cả các ví trong hệ thống
 * 
 * Chức năng:
 * - Tạo ví mới cho user
 * - Thực hiện giao dịch chuyển điểm
 * - Xác thực giao dịch bằng OTP
 * - Quản lý ví tổng
 * - Theo dõi lịch sử giao dịch
 */
class WalletManager {
private:
    std::shared_ptr<DatabaseManager> dataManager;    // Database manager
    std::shared_ptr<OTPManager> otpManager;           // Quản lý OTP
    std::unique_ptr<MasterWallet> masterWallet;       // Ví tổng
    
    std::unordered_map<std::string, std::shared_ptr<Wallet>> walletCache; // Cache ví
    // std::mutex transferMutex;                     // Mutex cho giao dịch thread-safe - temporarily disabled for MinGW compatibility
    
    static const double INITIAL_USER_POINTS;          // Điểm khởi tạo cho user mới

public:
    /**
     * @brief Constructor
     * @param dataManager Database manager
     * @param otpManager Quản lý OTP
     */
    WalletManager(std::shared_ptr<DatabaseManager> dataManager, 
                  std::shared_ptr<OTPManager> otpManager);

    /**
     * @brief Khởi tạo hệ thống ví
     * @return true nếu thành công
     */
    bool initialize();

    /**
     * @brief Tạo ví mới cho user
     * @param userId ID người dùng
     * @param walletId ID ví (đã được tạo trong User)
     * @return true nếu tạo thành công
     */
    bool createUserWallet(const std::string& userId, const std::string& walletId);

    /**
     * @brief Lấy ví theo ID
     * @param walletId ID ví
     * @return Shared pointer đến ví (nullptr nếu không tìm thấy)
     */
    std::shared_ptr<Wallet> getWallet(const std::string& walletId);

    /**
     * @brief Lấy ví theo user ID
     * @param userId ID người dùng
     * @return Shared pointer đến ví
     */
    std::shared_ptr<Wallet> getWalletByUserId(const std::string& userId);

    /**
     * @brief Thực hiện giao dịch chuyển điểm
     * @param request Yêu cầu chuyển điểm
     * @return Kết quả giao dịch
     */
    TransferResult transferPoints(const TransferRequest& request);

    /**
     * @brief Tạo OTP cho giao dịch chuyển điểm
     * @param fromUserId ID user gửi
     * @param toWalletId ID ví nhận
     * @param amount Số điểm
     * @return Mã OTP
     */
    std::string generateTransferOTP(const std::string& fromUserId,
                                   const std::string& toWalletId,
                                   double amount);

    /**
     * @brief Lấy số dư ví
     * @param walletId ID ví
     * @return Số dư (hoặc -1 nếu không tìm thấy ví)
     */
    double getBalance(const std::string& walletId);

    /**
     * @brief Lấy lịch sử giao dịch của ví
     * @param walletId ID ví
     * @param limit Số lượng giao dịch tối đa (-1 = tất cả)
     * @return Vector các giao dịch
     */
    std::vector<Transaction> getTransactionHistory(const std::string& walletId, 
                                                  int limit = -1);

    /**
     * @brief Lấy lịch sử giao dịch trong khoảng thời gian
     * @param walletId ID ví
     * @param fromDate Từ ngày
     * @param toDate Đến ngày
     * @return Vector các giao dịch
     */
    std::vector<Transaction> getTransactionHistoryByDate(
        const std::string& walletId,
        const std::chrono::system_clock::time_point& fromDate,
        const std::chrono::system_clock::time_point& toDate);

    /**
     * @brief Tìm ví theo owner ID
     * @param ownerId ID chủ sở hữu
     * @return Vector ID các ví
     */
    std::vector<std::string> findWalletsByOwner(const std::string& ownerId);

    /**
     * @brief Kiểm tra ví có tồn tại không
     * @param walletId ID ví
     * @return true nếu tồn tại
     */
    bool walletExists(const std::string& walletId);

    /**
     * @brief Khóa/mở khóa ví (chỉ admin)
     * @param walletId ID ví
     * @param locked true để khóa, false để mở
     * @return true nếu thành công
     */
    bool setWalletLocked(const std::string& walletId, bool locked);

    /**
     * @brief Phát hành điểm từ ví tổng (chỉ admin)
     * @param toWalletId ID ví nhận
     * @param amount Số điểm
     * @param description Mô tả
     * @return ID giao dịch nếu thành công
     */
    std::string issuePointsFromMaster(const std::string& toWalletId,
                                     double amount,
                                     const std::string& description = "Admin issued points");

    /**
     * @brief Lấy thống kê tổng quan hệ thống ví
     * @return String chứa thống kê
     */
    std::string getSystemStatistics();

    /**
     * @brief Xác thực giao dịch đang chờ
     * @param transactionId ID giao dịch
     * @param otpCode Mã OTP
     * @return true nếu xác thực thành công
     */
    bool confirmPendingTransaction(const std::string& transactionId,
                                  const std::string& otpCode);

    /**
     * @brief Hủy giao dịch đang chờ
     * @param transactionId ID giao dịch
     * @param reason Lý do hủy
     * @return true nếu hủy thành công
     */
    bool cancelPendingTransaction(const std::string& transactionId,
                                 const std::string& reason = "User cancelled");

    /**
     * @brief Lưu tất cả ví trong cache
     * @return true nếu lưu thành công
     */
    bool saveAllWallets();

    /**
     * @brief Dọn dẹp cache ví
     */
    void clearWalletCache();

private:
    /**
     * @brief Tải ví vào cache
     * @param walletId ID ví
     * @return Shared pointer đến ví
     */
    std::shared_ptr<Wallet> loadWalletToCache(const std::string& walletId);

    /**
     * @brief Xóa ví khỏi cache
     * @param walletId ID ví
     */
    void removeWalletFromCache(const std::string& walletId);

    /**
     * @brief Xác thực yêu cầu chuyển điểm
     * @param request Yêu cầu chuyển điểm
     * @return Chuỗi lỗi (rỗng nếu hợp lệ)
     */
    std::string validateTransferRequest(const TransferRequest& request);

    /**
     * @brief Thực hiện giao dịch atomic (không thể tách rời)
     * @param fromWallet Ví gửi
     * @param toWallet Ví nhận
     * @param amount Số điểm
     * @param description Mô tả
     * @return ID giao dịch nếu thành công
     */
    std::string executeAtomicTransfer(std::shared_ptr<Wallet> fromWallet,
                                     std::shared_ptr<Wallet> toWallet,
                                     double amount,
                                     const std::string& description);

    /**
     * @brief Rollback giao dịch khi có lỗi
     * @param fromWallet Ví gửi
     * @param toWallet Ví nhận
     * @param amount Số điểm
     * @param transactionId ID giao dịch cần hủy
     */
    void rollbackTransfer(std::shared_ptr<Wallet> fromWallet,
                         std::shared_ptr<Wallet> toWallet,
                         double amount,
                         const std::string& transactionId);

    /**
     * @brief Ghi log giao dịch
     * @param transaction Thông tin giao dịch
     * @param status Trạng thái
     * @param message Thông báo
     */
    void logTransaction(const Transaction& transaction,
                       const std::string& status,
                       const std::string& message);
};

#endif // WALLET_MANAGER_H

