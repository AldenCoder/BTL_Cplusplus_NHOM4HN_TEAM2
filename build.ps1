Write-Host "=== Building Wallet System for Windows ===" -ForegroundColor Green

Write-Host "Creating directories..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path "obj" -Force | Out-Null
New-Item -ItemType Directory -Path "bin" -Force | Out-Null
New-Item -ItemType Directory -Path "obj\models" -Force | Out-Null
New-Item -ItemType Directory -Path "obj\security" -Force | Out-Null
New-Item -ItemType Directory -Path "obj\storage" -Force | Out-Null
New-Item -ItemType Directory -Path "obj\system" -Force | Out-Null
New-Item -ItemType Directory -Path "obj\ui" -Force | Out-Null

$CXX = "g++"
$CC = "gcc"
$CXXFLAGS = "-std=c++17", "-Wall", "-Wextra", "-O2", "-g", "-Isrc", "-Isqlite/sqlite-amalgamation-3460100", "-DSQLITE_THREADSAFE=0"
$CFLAGS = "-O2", "-g", "-DSQLITE_THREADSAFE=0", "-DSQLITE_OMIT_LOAD_EXTENSION"
$LDFLAGS = "-static-libgcc", "-static-libstdc++"

$sources = @(
    "src\main.cpp",
    "src\models\User.cpp",
    "src\models\Wallet.cpp", 
    "src\security\OTPManager.cpp",
    "src\security\SecurityUtils.cpp",
    "src\storage\DatabaseManager.cpp",
    "src\storage\OTPStorage.cpp",
    "src\system\AuthSystem.cpp",
    "src\system\WalletManager.cpp",
    "src\ui\UserInterface.cpp",
    "src\ui\UserValidator.cpp",
    "sqlite\sqlite-amalgamation-3460100\sqlite3.c"
)

$objects = @()
foreach ($source in $sources) {
    if ($source -match "sqlite3\.c$") {
        $objFile = "obj\sqlite3.o"
        Write-Host "Compiling $source (as C)..." -ForegroundColor Cyan
        $compileCmd = @($CC) + $CFLAGS + @("-c", $source, "-o", $objFile)
    } else {
        $objFile = $source -replace "src\\", "obj\" -replace "\.cpp$", ".o"
        Write-Host "Compiling $source..." -ForegroundColor Cyan
        $compileCmd = @($CXX) + $CXXFLAGS + @("-c", $source, "-o", $objFile)
    }
    $objects += $objFile
    
    try {
        & $compileCmd[0] $compileCmd[1..($compileCmd.Length-1)]
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Compilation failed for $source" -ForegroundColor Red
            exit 1
        }
    } catch {
        Write-Host "Error compiling $source : $_" -ForegroundColor Red
        exit 1
    }
}

Write-Host "Linking executable..." -ForegroundColor Yellow
$linkCmd = @($CXX) + $objects + @("-o", "bin\wallet_system.exe") + $LDFLAGS

try {
    & $linkCmd[0] $linkCmd[1..($linkCmd.Length-1)]
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Linking failed" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "Error linking: $_" -ForegroundColor Red
    exit 1
}

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Executable: bin\wallet_system.exe" -ForegroundColor Cyan