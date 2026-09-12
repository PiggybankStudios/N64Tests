/*
File:   registers.h
Author: Taylor Robbins
Date:   09\11\2026
Description:
	** This is our own #defines for naming the various addresses
	** and bits of memory mapped registers that control or read
	** information from various parts of the N64 hardware.
	** 
	** Some of these things are defined in places in libdragon
	** but I wanted my own naming convention to enable specific
	** patterns that I find more readable and compile to direct
	** read or write instructions.
	** Also if we eventually move off libdragon we will need to
	** replicated whatever we use from it.
	** This file also acts as a good place to put notes and links
	** to useful documentation on n64brew.dev and other places.
*/

#ifndef _REGISTERS_H
#define _REGISTERS_H

//NOTE: See https://n64brew.dev/wiki/Memory_map
//NOTE: See https://github.com/Dillonb/n64-resources/blob/master/n64maps.txt

// User segment, TLB mapped
#define KUSEG_START_ADDR 0x00000000u
#define KUSEG_END_ADDR   0x7FFFFFFFu

// Kernel segment 0, directly mapped, cached
// #define KSEG0_START_ADDR 0x80000000u //Already defined in n64sys.h
#define KSEG0_END_ADDR   0x9FFFFFFFu

// Kernel segment 1, directly mapped, uncached
#define KSEG1_START_ADDR 0xA0000000u
#define KSEG1_END_ADDR   0xBFFFFFFFu

// Kernel supervisor segment, TLB mapped
#define KSSEG_START_ADDR 0xC0000000u
#define KSSEG_END_ADDR   0xDFFFFFFFu

// Kernel segment 3, TLB mapped
#define KSEG3_START_ADDR 0xE0000000u
#define KSEG3_END_ADDR   0xFFFFFFFFu

#define RSP_MEMORY_BASE    0x04000000u
#define RSP_DMEM_START     0x00000000u
#define RSP_DMEM_END       0x00000FFFu
#define RSP_IMEM_START     0x00001000u
#define RSP_IMEM_END       0x00001FFFu

// +--------------------------------------------------------------+
// |           Reality Signal Processor (RSP) Registers           |
// +--------------------------------------------------------------+
//NOTE: See https://n64brew.dev/wiki/Reality_Signal_Processor

#define RSP_REGISTERS_BASE 0x04040000u

// RSP Status Register (aka SP_SR) - Writing to bits often does something different than reading from the same bits!
#define REG_SP_STATUS (*(volatile u32*)(KSEG1_START_ADDR + RSP_REGISTERS_BASE + 0x00000010u))
//Readable bits
#define REG_SP_STATUS_BIT_HALT                (1u <<  0) //0x00000001
#define REG_SP_STATUS_BIT_BROKE               (1u <<  1) //0x00000002
#define REG_SP_STATUS_BIT_DMA_BUSY            (1u <<  2) //0x00000004
#define REG_SP_STATUS_BIT_DMA_FULL            (1u <<  3) //0x00000008
#define REG_SP_STATUS_BIT_IO_FULL             (1u <<  4) //0x00000010
#define REG_SP_STATUS_BIT_SINGLE_STEP         (1u <<  5) //0x00000020
#define REG_SP_STATUS_BIT_INTR_ON_BREAK       (1u <<  6) //0x00000040
#define REG_SP_STATUS_BIT_SIGNAL0             (1u <<  7) //0x00000080
#define REG_SP_STATUS_BIT_SIGNAL1             (1u <<  8) //0x00000100
#define REG_SP_STATUS_BIT_SIGNAL2             (1u <<  9) //0x00000200
#define REG_SP_STATUS_BIT_SIGNAL3             (1u << 10) //0x00000400
#define REG_SP_STATUS_BIT_SIGNAL4             (1u << 11) //0x00000800
#define REG_SP_STATUS_BIT_SIGNAL5             (1u << 12) //0x00001000
#define REG_SP_STATUS_BIT_SIGNAL6             (1u << 13) //0x00002000
#define REG_SP_STATUS_BIT_SIGNAL7             (1u << 14) //0x00004000
//Writable bits
#define REG_SP_STATUS_BIT_CLR_HALT            (1u <<  0) //0x00000001
#define REG_SP_STATUS_BIT_SET_HALT            (1u <<  1) //0x00000002
#define REG_SP_STATUS_BIT_CLEAR_BROKE         (1u <<  2) //0x00000004
#define REG_SP_STATUS_BIT_CLEAR_INTR          (1u <<  3) //0x00000008
#define REG_SP_STATUS_BIT_SET_INTR            (1u <<  4) //0x00000010
#define REG_SP_STATUS_BIT_CLEAR_SSTEP         (1u <<  5) //0x00000020
#define REG_SP_STATUS_BIT_SET_SSTEP           (1u <<  6) //0x00000040
#define REG_SP_STATUS_BIT_CLEAR_INTR_ON_BREAK (1u <<  7) //0x00000080
#define REG_SP_STATUS_BIT_SET_INTR_ON_BREAK   (1u <<  8) //0x00000100
#define REG_SP_STATUS_BIT_CLEAR_SIGNAL0       (1u <<  9) //0x00000200
#define REG_SP_STATUS_BIT_SET_SIGNAL0         (1u << 10) //0x00000400
#define REG_SP_STATUS_BIT_CLEAR_SIGNAL1       (1u << 11) //0x00000800
#define REG_SP_STATUS_BIT_SET_SIGNAL1         (1u << 12) //0x00001000
#define REG_SP_STATUS_BIT_CLEAR_SIGNAL2       (1u << 13) //0x00002000
#define REG_SP_STATUS_BIT_SET_SIGNAL2         (1u << 14) //0x00004000
#define REG_SP_STATUS_BIT_CLEAR_SIGNAL3       (1u << 15) //0x00008000
#define REG_SP_STATUS_BIT_SET_SIGNAL3         (1u << 16) //0x00010000
#define REG_SP_STATUS_BIT_CLEAR_SIGNAL4       (1u << 17) //0x00020000
#define REG_SP_STATUS_BIT_SET_SIGNAL4         (1u << 18) //0x00040000
#define REG_SP_STATUS_BIT_CLEAR_SIGNAL5       (1u << 19) //0x00080000
#define REG_SP_STATUS_BIT_SET_SIGNAL5         (1u << 20) //0x00100000
#define REG_SP_STATUS_BIT_CLEAR_SIGNAL6       (1u << 21) //0x00200000
#define REG_SP_STATUS_BIT_SET_SIGNAL6         (1u << 22) //0x00400000
#define REG_SP_STATUS_BIT_CLEAR_SIGNAL7       (1u << 23) //0x00800000
#define REG_SP_STATUS_BIT_SET_SIGNAL7         (1u << 24) //0x01000000

// RSP DMA busy (aka SP_DMA_BUSY_REG)
#define REG_SP_DMA_BUSY (*(volatile u32*)(KSEG1_START_ADDR + RSP_REGISTERS_BASE + 0x00000018u))
//Readable bits
#define REG_SP_DMA_BUSY_BIT_HALT  (1u <<  0) //0x00000001

//RSP Semaphore (aka SP_SEMAPHORE_REG) - Read to set, write to clear
#define REG_SP_SEMAPHORE (*(volatile u32*)(KSEG1_START_ADDR + RSP_REGISTERS_BASE + 0x0000001Cu))

// RSP Program Counter (aka SP_PC_REG)
#define REG_SP_PC (*(volatile u32*)(KSEG1_START_ADDR + RSP_REGISTERS_BASE + 0x00040000u)) // Address: A4080000
//Readable/Writable bits
#define REG_SP_PC_MASK (0x00000FFFu) //bits[0:11]

// +--------------------------------------------------------------+
// |       Reality Display Process (RDP) Command Registers        |
// +--------------------------------------------------------------+
//NOTE: See https://n64brew.dev/wiki/Reality_Display_Processor/Commands

#define DPC_REGISTERS_BASE 0x04100000u

// RDP Command Status Register (aka DPC_SR) - Writing to bits often does something different than reading from the same bits!
#define REG_DPC_STATUS (*(volatile u32*)(KSEG1_START_ADDR + DPC_REGISTERS_BASE + 0x000000Cu)) // Address: A410000C
//Readable bits
#define REG_DPC_STATUS_BIT_XBUS_DMEM_DMA     (1u <<   0) //0x00000001
#define REG_DPC_STATUS_BIT_FREEZE            (1u <<   1) //0x00000002
#define REG_DPC_STATUS_BIT_FLUSH             (1u <<   2) //0x00000004
#define REG_DPC_STATUS_BIT_START_GCLK        (1u <<   3) //0x00000008
#define REG_DPC_STATUS_BIT_TMEM_BUSY         (1u <<   4) //0x00000010
#define REG_DPC_STATUS_BIT_PIPE_BUSY         (1u <<   5) //0x00000020
#define REG_DPC_STATUS_BIT_CMD_BUSY          (1u <<   6) //0x00000040
#define REG_DPC_STATUS_BIT_CBUF_BUSY         (1u <<   7) //0x00000080
#define REG_DPC_STATUS_BIT_DMA_BUSY          (1u <<   8) //0x00000100
#define REG_DPC_STATUS_BIT_END_VALID         (1u <<   9) //0x00000200
#define REG_DPC_STATUS_BIT_START_VALID       (1u <<  10) //0x00000400
//Writable bits
#define REG_DPC_STATUS_BIT_CLR_XBUS_DMEM_DMA (1u <<   0) //0x00000001
#define REG_DPC_STATUS_BIT_SET_XBUX_DMEM_DMA (1u <<   1) //0x00000002
#define REG_DPC_STATUS_BIT_CLR_FREEZE        (1u <<   2) //0x00000004
#define REG_DPC_STATUS_BIT_SET_FREEZE        (1u <<   3) //0x00000008
#define REG_DPC_STATUS_BIT_CLR_FLUSH         (1u <<   4) //0x00000010
#define REG_DPC_STATUS_BIT_SET_FLUSH         (1u <<   5) //0x00000020
#define REG_DPC_STATUS_BIT_CLR_TMEM_CTR      (1u <<   6) //0x00000040
#define REG_DPC_STATUS_BIT_CLR_PIPE_CTR      (1u <<   7) //0x00000080
#define REG_DPC_STATUS_BIT_CLR_CMD_CTR       (1u <<   8) //0x00000100
#define REG_DPC_STATUS_BIT_CLR_CLOCK_CTR     (1u <<   9) //0x00000200

// +--------------------------------------------------------------+
// |                Video Interface (VI) Registers                |
// +--------------------------------------------------------------+
//NOTE: See https://n64brew.dev/wiki/Video_Interface

#define VI_REGISTERS_BASE 0x04400000u

// VI Vertical Interrupt Register (aka VI_INTR_REG or VI_V_INTR_REG)
#define REG_VI_V_INTR (*(volatile u32*)(KSEG1_START_ADDR + VI_REGISTERS_BASE + 0x000000Cu)) // Address: A440000C
//Readable/Writable bits
#define REG_VI_V_INTR_MASK (0x000003FFu) //bits[0:9]

// VI Current Vertical Line (aka VI_V_CURRENT or VI_CURRENT_REG or VI_V_CURRENT_LINE_REG)
#define REG_VI_CUR_LINE (*(volatile u32*)(KSEG1_START_ADDR + VI_REGISTERS_BASE + 0x0000010u)) // Address: A4400010
//Readable/Writable bits
#define REG_VI_CUR_LINE_BIT_FIELD      (1u << 0) //0x00000001 - In interlaced mode this oscillates between 0 and 1 every frame
#define REG_VI_CUR_LINE_CURRENT_OFFSET (1u << 1)
#define REG_VI_CUR_LINE_CURRENT_MASK   (0x000003FEu) //bits[1:9]

// VI Horizontal Video (aka VI_H_START_REG or VI_H_VIDEO_REG)
#define REG_VI_H_LIMITS (*(volatile u32*)(KSEG1_START_ADDR + VI_REGISTERS_BASE + 0x0000024u)) // Address: A4400024
//Readable/Writable bits
#define REG_VI_H_LIMITS_END_OFFSET   (1u << 0)
#define REG_VI_H_LIMITS_END_MASK     (0x000003FFu) //bits[0:9]
#define REG_VI_H_LIMITS_START_OFFSET (1u << 16)
#define REG_VI_H_LIMITS_START_MASK   (0x0003FF00u) //bits[16:25]

// +--------------------------------------------------------------+
// |                Audio Interface (AI) Registers                |
// +--------------------------------------------------------------+
//NOTE: See https://n64brew.dev/wiki/Audio_Interface

#define AI_REGISTERS_BASE 0x04500000u

// AI DMA Address Register (aka AI_MADDR or AI_DRAM_ADDR_REG) - Determines address for next DMA transfer
#define REG_AI_DRAM_ADDR (*(volatile u32*)(KSEG1_START_ADDR + AI_REGISTERS_BASE + 0x0000000u)) // Address: A4500000
//Writable bits
#define REG_AI_DRAM_ADDR_MASK (0x00FFFFFFu) //bits[0:23] (bottom 3 bits should always be 0)

// AI DMA Length Register (aka AI_LEN or AI_LEN_REG) - Determines length of next DMA transfer
#define REG_AI_DRAM_LEN (*(volatile u32*)(KSEG1_START_ADDR + AI_REGISTERS_BASE + 0x0000004u)) // Address: A4500004
//Writable bits
#define REG_AI_DRAM_LEN_MASK (0x00FFFFFFu) //bits[0:17] (bottom 3 bits should always be 0)

// +--------------------------------------------------------------+
// |             Peripheral Interface (PI) Registers              |
// +--------------------------------------------------------------+
//NOTE: See https://n64brew.dev/wiki/Parallel_Interface

#define PI_REGISTERS_BASE 0x04600000u

// PI Status Register (aka PI_SR) - Writing to bits often does something different than reading from the same bits!
#define REG_PI_STATUS (*(volatile u32*)(KSEG1_START_ADDR + PI_REGISTERS_BASE + 0x00000010u)) // Address: A4600010
//Readable bits
#define REG_PI_STATUS_BIT_DMA_BUSY              (1u <<  0) //0x00000001
#define REG_PI_STATUS_BIT_IO_BUSY               (1u <<  1) //0x00000002
#define REG_PI_STATUS_BIT_DMA_ERROR             (1u <<  2) //0x00000004
#define REG_PI_STATUS_BIT_DMA_INTR_DMA_COMPLETE (1u <<  3) //0x00000008
//Writable bits
#define REG_PI_STATUS_BIT_RESET_DMA             (1u <<  0) //0x00000001
#define REG_PI_STATUS_BIT_CLR_INTR              (1u <<  1) //0x00000002

// Domain 0's LAT(ch) (aka PI_BSD_DOM1_LAT) - Number of RCP cycles (minus 1) after address sent before first read or write may start.
// NOTE: During IPL2 the ROM's header is read and used to initialize this register (all ROMs set this to 64?)
#define REG_PI_DOM0_LAT (*(volatile u32*)(KSEG1_START_ADDR + PI_REGISTERS_BASE + 0x00000014u)) // Address: A4600014
//Readable/Writable bits
#define REG_PI_DOM0_LAT_MASK (0x000000FFu) //bits[0:7]

// Domain 0's Pulse Width (aka PI_BSD_DOM1_PWD) - The number of RCP cycles (minus 1) the read/write signals are held low.
// NOTE: During IPL2 the ROM's header is read and used to initialize this register (all ROMs set this to 18?)
#define REG_PI_DOM0_PWD (*(volatile u32*)(KSEG1_START_ADDR + PI_REGISTERS_BASE + 0x00000018u)) // Address: A4600018
//Readable/Writable bits
#define REG_PI_DOM0_PWD_MASK (0x000000FFu) //bits[0:7]

// Domain 0's Page Size (aka PI_BSD_DOM1_PGS) - How many bytes can be sequentially read/written on the bus before sending the next base address (Size = 2^(PGS+2) bytes)
// NOTE: During IPL2 the ROM's header is read and used to initialize this register (all ROMs set this to 7?)
#define REG_PI_DOM0_PGS (*(volatile u32*)(KSEG1_START_ADDR + PI_REGISTERS_BASE + 0x0000001Cu)) // Address: A460001C
//Readable/Writable bits
#define REG_PI_DOM0_PGS_MASK (0x0000000Fu) //bits[0:3]

// #define REG_ADDR_PI_DOM0_RLS 0xA4600020u
// Domain 0's Release Time (aka PI_BSD_DOM1_RLS) - The number of RCP cycles (minus 1) that the read/write signals are held high between each 16-bits of data
// NOTE: During IPL2 the ROM's header is read and used to initialize this register (all ROMs set this to 3?)
#define REG_PI_DOM0_RLS (*(volatile u32*)(KSEG1_START_ADDR + PI_REGISTERS_BASE + 0x00000020u)) // Address: A4600020
//Readable/Writable bits
#define REG_PI_DOM0_RLS_MASK (0x00000003u) //bits[0:1]


// +--------------------------------------------------------------+
// |               Serial Interface (SI) Registers                |
// +--------------------------------------------------------------+
//NOTE: See https://n64brew.dev/wiki/Serial_Interface

#define SI_REGISTERS_BASE 0x04800000u

// SI Status Register (aka SI_SR) - Writing to bits often does something different than reading from the same bits!
#define REG_SI_STATUS (*(volatile u32*)(KSEG1_START_ADDR + SI_REGISTERS_BASE + 0x00000018u)) // Address: A4800018

#endif //  _REGISTERS_H
