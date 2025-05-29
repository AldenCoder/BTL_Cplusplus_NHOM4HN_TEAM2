# Hệ thống Đăng nhập và Quản lý Ví Điểm Thưởng

## Giới thiệu dự án

Dự án **Hệ thống Đăng nhập và Quản lý Ví Điểm Thưởng** là một ứng dụng C++ được phát triển để quản lý tài khoản người dùng và thực hiện các giao dịch chuyển đổi điểm thưởng giữa các ví điện tử.

### Tính năng chính:
- 🔐 **Hệ thống xác thực an toàn**: Đăng ký, đăng nhập với mật khẩu được băm SHA256
- 👥 **Quản lý người dùng**: Phân quyền user thường và admin
- 💰 **Quản lý ví điểm thưởng**: Chuyển điểm, xem lịch sử giao dịch
- 🔒 **Xác thực 2 lớp**: Sử dụng OTP cho các giao dịch quan trọng
- 💾 **Lưu trữ dữ liệu**: File-based storage với hệ thống backup tự động
- 🔑 **Tạo mật khẩu tự động**: Admin có thể tạo tài khoản với mật khẩu tự động
- 📦 **Hệ thống backup nâng cao**: Backup thủ công/tự động, phục hồi dữ liệu
- ✅ **Validation nâng cao**: Kiểm tra email và số điện thoại Việt Nam
- 🎯 **Phát hành điểm từ ví tổng**: Admin có thể cấp điểm cho người dùng

## Thành viên nhóm và phân công công việc

| STT | Họ tên | MSSV | Công việc được giao |
|-----|--------|------|-------------------|
| 1 | [Tên thành viên 1] | [MSSV] | **System Architecture & Security**: Thiết kế kiến trúc hệ thống, implement SecurityUtils, OTP system |
| 2 | [Tên thành viên 2] | [MSSV] | **User Management**: Implement User class, AuthSystem, user authentication |
| 3 | [Tên thành viên 3] | [MSSV] | **Wallet System**: Implement Wallet class, WalletManager, transaction processing |
| 4 | [Tên thành viên 4] | [MSSV] | **Data Management & UI**: Implement DataManager, UserInterface, backup system |

## Tính năng mới được hoàn thiện (Phiên bản 2.0)

### 🔑 Tạo mật khẩu tự động
- **Chức năng**: Admin có thể tạo tài khoản cho user với mật khẩu được sinh tự động
- **Bảo mật**: Mật khẩu ngẫu nhiên 12 ký tự (chữ cái, số, ký tự đặc biệt)
- **UI Enhancement**: Hiển thị mật khẩu đã tạo với warning rõ ràng cho admin
- **Implementation**: Sử dụng `SecurityUtils::generateSecurePassword()`

### 📦 Hệ thống backup nâng cao
- **Backup thủ công**: Admin có thể tạo backup bất kỳ lúc nào với mô tả tùy chỉnh
- **Lịch sử backup**: Xem danh sách tất cả backup với thông tin chi tiết (ID, thời gian, kích thước)
- **Phục hồi dữ liệu**: Chọn từ danh sách backup và phục hồi với safety backup tự động
- **Dọn dẹp backup**: Tự động xóa backup cũ, chỉ giữ lại số lượng theo cấu hình
- **Metadata tracking**: Theo dõi loại backup (Manual/Auto/Emergency), checksum, file size

### ✅ Validation nâng cao
- **Email validation**: Kiểm tra format email theo chuẩn RFC với additional checks
  - Không cho phép dấu chấm liên tiếp (..)
  - Không cho phép dấu chấm ở đầu/cuối
  - Validate độ dài email tối đa 254 ký tự
- **Phone validation**: Hỗ trợ số điện thoại Việt Nam
  - Format: 84xxxxxxxxx, 0xxxxxxxxx, hoặc 10-11 chữ số
  - Tự động loại bỏ ký tự phân cách (space, dash, brackets)
  - Kiểm tra pattern phù hợp với tiêu chuẩn VN

### 🎯 Phát hành điểm từ ví tổng
- **Master Wallet Integration**: Kết nối với ví tổng để phát hành điểm
- **Admin Controls**: Chỉ admin có quyền phát hành điểm
- **Transaction Tracking**: Ghi nhận tất cả giao dịch phát hành với lý do
- **Balance Management**: Tự động cập nhật số dư ví người dùng

### 🔧 Cải thiện UI/UX
- **Warning System**: Thêm `showWarning()` method cho các cảnh báo quan trọng
- **Formatted Display**: Hiển thị file size, datetime theo định dạng dễ đọc
- **Menu Enhancement**: Backup management với submenu đầy đủ chức năng
- **Error Handling**: Xử lý lỗi tốt hơn với try-catch và thông báo rõ ràng

## Đặc tả chức năng

### A. Quản lý tài khoản người dùng

#### 1. Đăng ký tài khoản
- **Input**: Username, mật khẩu, họ tên, email, số điện thoại
- **Process**: 
  - Validate thông tin đầu vào
  - Kiểm tra username trùng lặp
  - Băm mật khẩu bằng SHA256 + salt
  - Tạo ví điểm thưởng tự động
- **Output**: Thông báo thành công/thất bại

#### 2. Đăng nhập
- **Input**: Username, mật khẩu
- **Process**: 
  - Tìm user trong database
  - Verify mật khẩu
  - Tạo session
- **Output**: Chuyển đến menu tương ứng với quyền user

#### 3. Quản lý thông tin cá nhân
- **Chức năng**: Xem, cập nhật thông tin (tên, email, SĐT)
- **Bảo mật**: Yêu cầu OTP khi thay đổi thông tin quan trọng

### B. Hệ thống phân quyền

#### 1. User thường
- Quản lý thông tin cá nhân
- Xem số dư ví
- Chuyển điểm cho user khác
- Xem lịch sử giao dịch

#### 2. Admin
- Tất cả chức năng của user thường
- Tạo tài khoản cho user khác
- Xem danh sách tất cả user
- Phát hành điểm từ ví tổng
- Quản lý backup dữ liệu

### C. Quản lý ví điểm thưởng

#### 1. Cấu trúc ví
```cpp
class Wallet {
    string walletId;        // ID duy nhất
    string ownerId;         // ID chủ sở hữu  
    double balance;         // Số dư hiện tại
    vector<Transaction> transactions; // Lịch sử giao dịch
};
```

#### 2. Giao dịch chuyển điểm
- **Quy trình ACID**: Đảm bảo tính toàn vẹn dữ liệu
- **Xác thực OTP**: Bắt buộc cho mọi giao dịch
- **Rollback**: Tự động khôi phục khi có lỗi

#### 3. Ví tổng (Master Wallet)
- Nguồn phát sinh điểm duy nhất trong hệ thống
- Chỉ admin có quyền phát hành điểm

### D. Hệ thống bảo mật

#### 1. Mã hóa mật khẩu
- **Thuật toán**: SHA256 với salt ngẫu nhiên
- **Format lưu trữ**: `salt$hash`

#### 2. OTP (One-Time Password)
- **Độ dài**: 6 chữ số
- **Thời gian hiệu lực**: 5 phút
- **Mục đích**: 
  - Xác thực giao dịch chuyển điểm
  - Xác thực thay đổi thông tin cá nhân

#### 3. Session Management
- Quản lý phiên đăng nhập
- Tự động đăng xuất khi không hoạt động

## Cài đặt và chạy chương trình

### Yêu cầu hệ thống
- **OS**: Linux/Unix, Windows (với MinGW), macOS
- **Compiler**: g++ hỗ trợ C++17 trở lên
- **Libraries**: OpenSSL (cho SHA256 hashing)

### Cài đặt dependencies

#### Windows (recommended):
```powershell
# Project đã được setup sẵn với build.ps1 script
# Không cần cài đặt thêm dependencies
```

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential libssl-dev
```

#### macOS:
```bash
brew install openssl
```

### Biên dịch và chạy chương trình

#### Windows (PowerShell):
```powershell
# Di chuyển vào thư mục project
cd f:\project\team2C

# Biên dịch sử dụng script có sẵn
.\build.ps1

# Chạy chương trình
.\wallet_system.exe
```

#### Linux/macOS (Makefile):
```bash
# Tạo thư mục cần thiết
make setup-dirs

# Biên dịch
make all

# Chạy
./wallet_system
```

#### Biên dịch thủ công:
```bash
g++ -std=c++17 -Wall -Wextra -O2 -Isrc \
    src/**/*.cpp src/*.cpp \
    -lssl -lcrypto -o wallet_system
```

### Chạy chương trình
```bash
# Nếu dùng Makefile
make run

# Hoặc chạy trực tiếp
./bin/wallet_system

# Windows
.\build.ps1
.\wallet_system.exe

# Mac
chmod +x build.sh
.\build.sh (đổi đuôi .sh)
.\wallet_system
```

## Hướng dẫn sử dụng

### 1. Khởi động ứng dụng
```
=================================================
    HỆ THỐNG ĐĂNG NHẬP VÀ QUẢN LÝ VÍ ĐIỂM THƯỞNG  
=================================================

MENU CHÍNH:
1. Đăng nhập
2. Đăng ký tài khoản mới  
3. Thoát

Chọn chức năng (1-3): 
```

### 2. Đăng ký tài khoản mới
- Nhập username (duy nhất, không thay đổi được)
- Nhập mật khẩu (tối thiểu 8 ký tự)
- Nhập thông tin cá nhân (họ tên, email, SĐT)

### 3. Đăng nhập
- Nhập username và mật khẩu
- Nếu là lần đăng nhập đầu tiên với mật khẩu tự sinh → bắt buộc đổi mật khẩu

### 4. Chức năng chính

#### User thường:
```
MENU NGƯỜI DÙNG:
1. Xem thông tin cá nhân
2. Thay đổi mật khẩu  
3. Cập nhật thông tin cá nhân
4. Xem số dư ví
5. Chuyển điểm
6. Xem lịch sử giao dịch
7. Đăng xuất
```

#### Admin:
```
MENU QUẢN TRỊ:
1. Quản lý thông tin cá nhân
2. Xem danh sách người dùng
3. Tạo tài khoản mới (với tùy chọn tạo mật khẩu tự động)
4. Phát hành điểm từ ví tổng
5. Xem thống kê hệ thống
6. Quản lý backup
   - Tạo backup thủ công
   - Xem lịch sử backup
   - Phục hồi từ backup
   - Dọn dẹp backup cũ
   - Quay lại menu chính
7. Đăng xuất
```

### 5. Quy trình tạo tài khoản mới (Admin)
1. Chọn "Tạo tài khoản mới" từ menu admin
2. Nhập thông tin user (username, tên, email, SĐT, role)
3. Chọn có tự động tạo mật khẩu hay không
4. Nếu chọn tự động: hệ thống sinh mật khẩu 12 ký tự ngẫu nhiên
5. Hiển thị mật khẩu đã tạo với cảnh báo bảo mật
6. Admin chịu trách nhiệm chuyển mật khẩu cho user một cách an toàn

### 6. Quy trình phát hành điểm (Admin)
1. Chọn "Phát hành điểm từ ví tổng"
2. Nhập username người nhận
3. Nhập số điểm muốn phát hành
4. Nhập lý do phát hành điểm
5. Xác nhận giao dịch
6. Hệ thống cập nhật số dư ví người dùng

### 7. Quy trình chuyển điểm
1. Chọn "Chuyển điểm" từ menu
2. Nhập ID ví đích
3. Nhập số điểm muốn chuyển
4. Nhập mô tả giao dịch (tùy chọn)
5. Hệ thống tạo và hiển thị mã OTP
6. Nhập mã OTP để xác nhận
7. Giao dịch được thực hiện

## Cấu trúc dữ liệu

### Lưu trữ User (JSON):
```json
{
  "userId": "uuid-string",
  "username": "user123", 
  "passwordHash": "salt$hash",
  "fullName": "Nguyễn Văn A",
  "email": "user@example.com",
  "phoneNumber": "0123456789",
  "role": 0,
  "isPasswordGenerated": false,
  "isFirstLogin": false,
  "walletId": "W_12345678",
  "createdAt": 1640995200,
  "lastLogin": 1640995200
}
```

### Lưu trữ Wallet (JSON):
```json
{
  "walletId": "W_12345678",
  "ownerId": "uuid-string",
  "balance": 1000.0,
  "transactions": [...],
  "createdAt": 1640995200,
  "isLocked": false
}
```

### Cấu trúc thư mục:
```
project/
├── src/                    # Source code
│   ├── models/            # Data models (User, Wallet)
│   ├── security/          # Security utilities (Hash, OTP)
│   ├── storage/           # Data management
│   ├── system/            # Core systems (Auth, WalletManager)
│   ├── ui/               # User interface
│   └── main.cpp          # Entry point
├── data/                  # User data storage
├── backup/               # Backup files  
├── logs/                 # Log files
├── Makefile              # Build configuration
├── CMakeLists.txt        # CMake configuration
└── README.md             # Documentation
```

## Hệ thống Backup và Recovery (Nâng cấp)

### Backup tự động:
- Backup hàng ngày vào lúc khởi động ứng dụng
- Backup emergency trước khi restore
- Giữ tối đa số lượng backup theo cấu hình (mặc định 10)
- Checksum để kiểm tra tính toàn vẹn dữ liệu
- Metadata tracking (loại backup, kích thước, thời gian)

### Backup thủ công (Admin):
```
1. Vào Menu Admin > Quản lý backup > Tạo backup thủ công
2. Nhập mô tả cho backup (tùy chọn)
3. Hệ thống tạo backup với thông tin chi tiết:
   - Backup ID: unique identifier
   - Filename: backup_YYYYMMDD_HHMMSS.zip
   - Size: kích thước file được format
   - Created: thời gian tạo
```

### Xem lịch sử backup:
```
Hiển thị table với thông tin:
- Backup ID (18 ký tự đầu)
- Filename (23 ký tự đầu)  
- Size (formatted: B, KB, MB, GB)
- Created (DD/MM/YYYY HH:MM:SS)
- Type (Manual/Auto/Emergency)
```

### Phục hồi dữ liệu:
```
1. Chọn "Phục hồi từ backup" từ menu
2. Hệ thống hiển thị danh sách backup (tối đa 10 gần nhất)
3. Chọn backup muốn restore với thông tin chi tiết
4. Hệ thống cảnh báo và tạo safety backup trước khi restore
5. Xác nhận để thực hiện restore
6. Thông báo kết quả và yêu cầu restart ứng dụng
```

### Dọn dẹp backup:
```
1. Chọn "Dọn dẹp backup cũ"
2. Nhập số lượng backup muốn giữ lại (1-20)
3. Hệ thống hiển thị thống kê:
   - Tổng số backup hiện tại
   - Số backup sẽ được giữ lại
   - Số backup sẽ bị xóa
4. Xác nhận để thực hiện cleanup
5. Báo cáo kết quả cleanup
```

### Backup thủ công qua file system:
```bash
# Từ menu admin chọn "Quản lý backup"
# Hoặc copy thư mục data/
cp -r data/ backup/manual_backup_$(date +%Y%m%d)
```

## Cải thiện Validation và Security

### Email Validation nâng cao:
```cpp
// Kiểm tra format email theo RFC standard
// Additional checks:
- Không cho phép dấu chấm liên tiếp (..)
- Không cho phép dấu chấm ở đầu/cuối  
- Validate độ dài email tối đa 254 ký tự
- Kiểm tra @ không ở đầu/cuối email
```

### Phone Number Validation (Vietnamese):
```cpp
// Hỗ trợ các format số điện thoại Việt Nam:
- 84xxxxxxxxx  (country code +84)
- 0xxxxxxxxx   (bắt đầu bằng 0)
- xxxxxxxxxx   (10-11 chữ số)

// Auto cleanup:
- Loại bỏ space, dash, brackets, plus
- Chỉ chấp nhận digits sau khi cleanup
```

### Password Security:
```cpp
// Auto-generated password:
- Độ dài: 12 ký tự
- Bao gồm: chữ hoa, chữ thường, số, ký tự đặc biệt
- Random generation với entropy cao
- Hiển thị một lần duy nhất cho admin
```

## Tài liệu tham khảo

1. **CPP_OTP**: [https://github.com/patzol768/cpp-otp](https://github.com/patzol768/cpp-otp) - Thư viện OTP cho C++
2. **COTP**: [https://github.com/tilkinsc/COTP](https://github.com/tilkinsc/COTP) - Thư viện OTP khác
3. **ACID Properties**: [https://200lab.io/blog/acid-la-gi/](https://200lab.io/blog/acid-la-gi/) - Tính chất ACID trong database
4. **OpenSSL Documentation**: [https://www.openssl.org/docs/](https://www.openssl.org/docs/) - Tài liệu OpenSSL
5. **C++17 Reference**: [https://en.cppreference.com/w/cpp/17](https://en.cppreference.com/w/cpp/17) - Tài liệu C++17

## Thông tin thêm

### Update Log (v2.0 - 29/05/2025):
- ✅ **Hoàn thiện tính năng tạo mật khẩu tự động** cho admin
- ✅ **Nâng cấp hệ thống backup** với UI đầy đủ và kết nối DataManager
- ✅ **Cải thiện validation** email và phone number cho người dùng Việt Nam  
- ✅ **Implement phát hành điểm từ ví tổng** với UI và xử lý giao dịch
- ✅ **Thêm showWarning() method** và các utility functions
- ✅ **Enhanced UI/UX** với formatted display và error handling
- ✅ **Cập nhật README.md** với tài liệu đầy đủ cho tất cả tính năng mới
- ✅ **Testing và compilation** thành công trên Windows

### Liên hệ hỗ trợ:
- Email nhóm: 
- Repository: [https://github.com/team2c/wallet-system](https://github.com/team2c/wallet-system)

### License:
MIT License - Xem file LICENSE để biết thêm chi tiết.
