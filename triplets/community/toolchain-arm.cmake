# triplets/community/toolchain-arm.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# 使用环境变量确保路径正确
set(COMPILER_DIR "$ENV{VCINSTALLDIR}/bin/Hostx64/arm")
set(CMAKE_C_COMPILER "${COMPILER_DIR}/cl.exe")
set(CMAKE_CXX_COMPILER "${COMPILER_DIR}/cl.exe")
set(CMAKE_RC_COMPILER "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64/rc.exe")

# 强制SDK版本
set(CMAKE_SYSTEM_VERSION 10.0.22621.0)
message(STATUS "Using ARM toolchain: ${CMAKE_C_COMPILER}")