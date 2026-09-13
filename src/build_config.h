/*
File:   build_config.h
Author: Taylor Robbins
Date:   09\03\2026
Description:
	** This file is required by PigCore
*/

#ifndef _BUILD_CONFIG_H
#define /*Don't show in C-Switch*/ _BUILD_CONFIG_H

#define ROM_NAME         "n64_tests"
#define ROM_TITLE        "N64 Tests"

#define DEBUG_BUILD         1
#define MAKE_RESOURCES_DFS  0
#define BUILD_FOR_EMULATOR  0

// This can be done while the console is powered on, but a power cycle must be performed for the change to take effect
#define UPLOAD_TO_SC64       1
// Make sure the console is powered off. Flash access is unsafe while it's powered on
#define INSTALL_TO_SC64      0
// After installing/uploading to SC64 cart, attach to the debug output via "sc64deployer.exe debug"
#define ATTACH_TO_DEBUG_SC64 1
// Runs Ares emulator - https://ares-emu.net/
#define START_ARES_EMULATOR  0



#define BUILD_WITH_CLAY     0
#define BUILD_WITH_IMGUI    0
#define BUILD_WITH_METADESK 0
#define BUILD_WITH_PROTOBUF 0
#define BUILD_WITH_FREETYPE 0
#define BUILD_WITH_PIG_UI   0



#define TOOLCHAIN_PREFIX "mips64-elf"

//Desktop Paths
#define LIB_DRAGON_DIR         "C:/gamedev/downloaded/libdragon-preview"
#define MIPS_GCC_TOOLCHAIN_DIR "F:/Programs/libdragon"
#define LIB_DRAGON_TOOLS_DIR   "F:/Programs/libdragon/bin"
#define SC64DEPLOYER_PATH      "F:/Programs/sc64deployer/sc64deployer.exe"
#define ARES_PATH              "F:/Programs/ares-v148/ares.exe"

//Laptop Paths
// #define LIB_DRAGON_DIR         "D:/gamedev/downloaded/libdragon"
// #define MIPS_GCC_TOOLCHAIN_DIR "D:/Programs/gcc-toolchain-mips64-win64"
// #define LIB_DRAGON_TOOLS_DIR   "D:/Programs/gcc-toolchain-mips64-win64/bin"
// #define SC64DEPLOYER_PATH      "D:/Programs/sc64deployer/sc64deployer.exe"
// #define ARES_PATH              "TODO:"

//Macbook Paths
// #define LIB_DRAGON_DIR         "/Users/robbitay/my/repos/libdragon"
// #define MIPS_GCC_TOOLCHAIN_DIR "/opt/libdragon"
// #define LIB_DRAGON_TOOLS_DIR   "/opt/libdragon/bin"
// #define SC64DEPLOYER_PATH      "/Users/robbitay/my/bin/sc64deployer"
// #define ARES_PATH              "/Applications/ares.app/Contents/MacOS/ares"

#endif //  _BUILD_CONFIG_H
