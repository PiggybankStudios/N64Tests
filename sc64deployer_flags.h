/*
File:   sc64deployer_flags.h
Author: Taylor Robbins
Date:   08\24\2026
*/

#ifndef _SC_64DEPLOYER_FLAGS_H
#define _SC_64DEPLOYER_FLAGS_H

#define SC64_CMD_LIST     "list"     // List connected SC64 devices
#define SC64_CMD_DOWNLOAD "download" // Download specific memory region and write it to file
#define SC64_CMD_64DD     "64dd"     // Upload ROM (and save), 64DD IPL then run disk/debug server
#define SC64_CMD_DEBUG    "debug"    // Enter debug mode
#define SC64_CMD_DUMP     "dump"     // Dump data from arbitrary location in SC64 memory space
#define SC64_CMD_INFO     "info"     // Print information about connected SC64 device
#define SC64_CMD_RESET    "reset"    // Reset SC64 state (same as after power-up)
#define SC64_CMD_SET      "set"      // Update persistent settings on SC64 device
#define SC64_CMD_FIRMWARE "firmware" // Print firmware metadata / update or backup SC64 firmware
#define SC64_CMD_TEST     "test"     // Test SC64 hardware
#define SC64_CMD_SERVER   "server"   // Expose SC64 device over network
#define SC64_CMD_HELP     "help"     // Print this message or the help of the given subcommand(s)

#define SC64_CMD_UPLOAD    "upload"   // Upload ROM (and save) to the SC64
#define SC64_UPLOAD_OPTION_REBOOT    "--reboot"          // -a, Attempt to reboot the console (requires specific support in the running game)
#define SC64_UPLOAD_OPTION_SAVE      "--save [VAL]"      // -s, Path to the save file
#define SC64_UPLOAD_OPTION_SAVE_TYPE "--save-type [VAL]" // -t, Override autodetected save type [possible values: none, eeprom4k, eeprom16k, sram, sram-banked, sram1m, flashram]
#define SC64_UPLOAD_OPTION_DIRECT    "--direct"          // -d, Use direct boot mode (skip bootloader)
#define SC64_UPLOAD_OPTION_NO_SHADOW "--no-shadow"       // -n, Do not put last 128 kiB of ROM inside flash memory (can corrupt non EEPROM saves)
#define SC64_UPLOAD_OPTION_TV        "--tv [VAL]"        //     Force TV type [possible values: pal, ntsc, mpal]
#define SC64_UPLOAD_OPTION_CIC_SEED  "--cic-seed [VAL]"  //     Force CIC seed

#define SC64_CMD_SD "sd" // Perform operations on the SD card
#define   SC64_SD_SUBCMD_LS       "ls"       // List a directory on the SD card     - sd ls [PATH]
#define   SC64_SD_SUBCMD_STAT     "stat"     // Display a file or directory status  - sd stat <PATH>
#define   SC64_SD_SUBCMD_MV       "mv"       // Move or rename a file or directory  - sd mv <SRC> <DST>
#define   SC64_SD_SUBCMD_RM       "rm"       // Remove a file or empty directory    - sd rm <PATH>
#define   SC64_SD_SUBCMD_MKDIR    "mkdir"    // Create a new directory              - sd mkdir <PATH>
#define   SC64_SD_SUBCMD_DOWNLOAD "download" // Download a file to the PC           - sd download <SRC> [DST]
#define   SC64_SD_SUBCMD_UPLOAD   "upload"   // Upload a file to the SD card        - sd upload <SRC> [DST]
#define   SC64_SD_SUBCMD_MKFS     "mkfs"     // Format the SD card                  - sd mkfs

#endif //  _SC_64DEPLOYER_FLAGS_H
