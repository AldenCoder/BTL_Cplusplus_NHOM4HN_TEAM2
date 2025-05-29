cmake_minimum_required(VERSION 3.10)
project(WalletSystem VERSION 1.0.0 LANGUAGES CXX)

# Thiết lập C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Include directories
include_directories(src)

# Tìm OpenSSL (cho hàm băm)
find_package(OpenSSL REQUIRED)

# Tìm tất cả source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

# Tạo executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    OpenSSL::SSL 
    OpenSSL::Crypto
)

# Tạo thư mục cần thiết
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/backup)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)

# Custom targets
add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running ${PROJECT_NAME}"
)

add_custom_target(setup-dirs
    COMMAND ${CMAKE_COMMAND} -E make_directory data backup logs
    COMMENT "Creating necessary directories"
)

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Package configuration
set(CPACK_PACKAGE_NAME "WalletSystem")
set(CPACK_PACKAGE_VENDOR "Team 2C")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Hệ thống đăng nhập và quản lý ví điểm thưởng")

include(CPack)
