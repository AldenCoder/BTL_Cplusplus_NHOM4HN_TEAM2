# cd "f:\project\team2C"

# # Create directories
# mkdir -Force -Path obj/models
# mkdir -Force -Path obj/security
# mkdir -Force -Path obj/storage
# mkdir -Force -Path obj/system
# mkdir -Force -Path obj/ui
# mkdir -Force -Path bin

# # Compile all source files
# g++ -std=c++17 -c src/main.cpp -o obj/main.o
# g++ -std=c++17 -c src/models/User.cpp -o obj/models/User.o
# g++ -std=c++17 -c src/models/Wallet.cpp -o obj/models/Wallet.o
# g++ -std=c++17 -c src/security/OTPManager.cpp -o obj/security/OTPManager.o
# g++ -std=c++17 -c src/security/SecurityUtils.cpp -o obj/security/SecurityUtils.o
# g++ -std=c++17 -c src/storage/DataManager.cpp -o obj/storage/DataManager.o
# g++ -std=c++17 -c src/system/AuthSystem.cpp -o obj/system/AuthSystem.o
# g++ -std=c++17 -c src/system/WalletManager.cpp -o obj/system/WalletManager.o
# g++ -std=c++17 -c src/ui/UserInterface.cpp -o obj/ui/UserInterface.o

# # Link all object files
# g++ -std=c++17 -o wallet_system.exe obj/main.o obj/models/User.o obj/models/Wallet.o obj/security/OTPManager.o obj/security/SecurityUtils.o obj/storage/DataManager.o obj/system/AuthSystem.o obj/system/WalletManager.o obj/ui/UserInterface.o

# Write-Host "Build completed! Run with: .\wallet_system.exe"

########
#!/bin/bash

# Create directories
mkdir -p obj/models
mkdir -p obj/security
mkdir -p obj/storage
mkdir -p obj/system
mkdir -p obj/ui
mkdir -p bin

g++ -std=c++17 -c src/main.cpp -o obj/main.o
g++ -std=c++17 -c src/models/User.cpp -o obj/models/User.o
g++ -std=c++17 -c src/models/Wallet.cpp -o obj/models/Wallet.o
g++ -std=c++17 -c src/security/OTPManager.cpp -o obj/security/OTPManager.o
g++ -std=c++17 -c src/security/SecurityUtils.cpp -o obj/security/SecurityUtils.o
g++ -std=c++17 -c src/storage/DataManager.cpp -o obj/storage/DataManager.o
g++ -std=c++17 -c src/system/AuthSystem.cpp -o obj/system/AuthSystem.o
g++ -std=c++17 -c src/system/WalletManager.cpp -o obj/system/WalletManager.o
g++ -std=c++17 -c src/ui/UserInterface.cpp -o obj/ui/UserInterface.o

g++ -std=c++17 -o wallet_system obj/main.o obj/models/User.o obj/models/Wallet.o obj/security/OTPManager.o obj/security/SecurityUtils.o obj/storage/DataManager.o obj/system/AuthSystem.o obj/system/WalletManager.o obj/ui/UserInterface.o

echo "Build completed! Run with: ./wallet_system"