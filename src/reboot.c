/*
File:   reboot.c
Author: Taylor Robbins
Date:   09\10\2026
Description: 
	** Holds code that helps us perform a software reboot by replicating the state
	** we would expect after running IPL1+IPL2, copying IPL3 into DMEM and then
	** jumping to IPL3.
	** NOTE: Most of this code is based on the SummerCart64 repository:
	**   reboot.S - https://github.com/Polprzewodnikowy/SummerCart64/blob/main/sw/bootloader/src/reboot.S
	**   boot.c   - https://github.com/Polprzewodnikowy/SummerCart64/blob/main/sw/bootloader/src/boot.c
*/

// ------------------------------------------------------------------
//  Reboot trampoline (mostly a copy of reboot.S).
//  Copied to RSP IMEM at run time, then executed there. It re-runs the
//  console boot by jumping to IPL3 at 0xA4000040.
// ------------------------------------------------------------------
__asm__ (
	".set noat                                                    \n"
	".set noreorder                                               \n"
	".section .text.reboot,\"ax\",%progbits                       \n"
	".global reboot_start                                         \n"
"reboot_start:                                                    \n"

	// --- CIC x105 handshake stub. Remove this block if you never  ---
	// --- need x105 compatibility.                                 ---
"ipl2:                                                            \n"
	"  lui   $t5, 0xBFC0                                          \n"
"1:                                                               \n"
    "  lw    $t0, 0x7FC($t5)                                      \n"
    "  addiu $t5, $t5, 0x7C0                                      \n"
    "  andi  $t0, $t0, 0x80                                       \n"
    "  bnel  $t0, $zero, 1b                                       \n"
    "  lui   $t5, 0xBFC0                                          \n"   // delay slot
    "  lw    $t0, 0x24($t5)                                       \n"
    "  lui   $t3, 0xB000                                          \n"
    ".set reorder                                                 \n"

    ".global reboot                                               \n"
"reboot_entry:                                                    \n"
    ".set reboot, 0xA4001000 + (reboot_entry - reboot_start)      \n"
    "  li    $sp, 0xA4001FF0                                      \n"

    // $s5 (fastReset): 1 = keep RDRAM (fast/NMI), 0 = reset RDRAM refresh
    "  bnez  $s5, reset_rdram_skip                                \n"
    "  li    $t0, 0xA4700000                                      \n"   // RI base
    "  sw    $zero, 0x10($t0)                                     \n"   // RI_REFRESH
    "  sw    $zero, 0x0C($t0)                                     \n"   // RI_SELECT
"reset_rdram_skip:                                                \n"

    // $s4 (tvType): pick IPL3 return address per region
"detect_console_region:                                            \n"
    "  li    $t0, 1                                               \n"
    "  beq   $s4, $zero, pal_console                              \n"
    "  beq   $s4, $t0,  ntsc_console                              \n"
    "  b     mpal_console                                         \n"
"pal_console:                                                     \n"
    "  li    $ra, 0xA4001554                                      \n"
    "  b     prepare_registers                                    \n"
"ntsc_console:                                                    \n"
    "  li    $ra, 0xA4001550                                      \n"
    "  b     prepare_registers                                    \n"
"mpal_console:                                                    \n"
    "  li    $ra, 0xA4001554                                      \n"

"prepare_registers:                                               \n"
    "  move  $at, $zero                                           \n"
    "  move  $v0, $zero                                           \n"
    "  move  $v1, $zero                                           \n"
    "  move  $a0, $zero                                           \n"
    "  move  $a1, $zero                                           \n"
    "  move  $a2, $zero                                           \n"
    "  move  $a3, $zero                                           \n"
    "  move  $t0, $zero                                           \n"
    "  move  $t1, $zero                                           \n"
    "  li    $t2, 0x40                                            \n"
    "  move  $t4, $zero                                           \n"
    "  move  $t5, $zero                                           \n"
    "  move  $t6, $zero                                           \n"
    "  move  $t7, $zero                                           \n"
    "  move  $s0, $zero                                           \n"
    "  move  $s1, $zero                                           \n"
    "  move  $s2, $zero                                           \n"
    "  move  $t8, $zero                                           \n"
    "  move  $t9, $zero                                           \n"
    "  move  $k0, $zero                                           \n"
    "  move  $k1, $zero                                           \n"
    "  move  $gp, $zero                                           \n"
    "  move  $fp, $zero                                           \n"
    "  li    $t3, 0xA4000040                                      \n"   // IPL3 entry
    "  jr    $t3                                                  \n"
    "  nop                                                        \n"
    ".global reboot_end                                           \n"
"reboot_end:                                                      \n"

    // restore assembler state so compiler-emitted code isn't corrupted
    ".set at                                                      \n"
    ".set reorder                                                 \n"
    ".text                                                        \n"
);

extern u32 reboot_start;
extern u32 reboot_end;

/* ------------------------------------------------------------------ *
 *  RCP register access (uncached KSEG1). RCP regs are on the system
 *  bus (no PI wait). Cart/ROM/SC64 regs are on the PI bus (busy-wait).
 * ------------------------------------------------------------------ */
static inline u32 BlockingPiRead(volatile u32* regPntr)
{
	while (REG_PI_STATUS & (REG_PI_STATUS_BIT_DMA_BUSY | REG_PI_STATUS_BIT_IO_BUSY)) { }
	return *regPntr;
}
static inline void BlockingPiWrite(volatile u32* regPntr, u32 regValue)
{
	while (REG_PI_STATUS & (REG_PI_STATUS_BIT_DMA_BUSY | REG_PI_STATUS_BIT_IO_BUSY)) { }
	*regPntr = regValue;
}

// Re-runs the boot process from IPL3 on without powering off
void SoftRebootN64()
{
	disable_interrupts();
	u32 tvType = get_tv_type();
	// CIC seed (6102/7101 = 0x3F). Must match the target ROM. TODO: Why do we use this seed? Do we ever need to choose a different one?
	const u32 cicSeed = 0x3F;
	
	// CU1|CU0|FR
	__asm__ volatile (
		"li $t0,0x34000000\n"
		"mtc0 $t0,$12\n"
		::: "t0"
	); 
	
	// Stop the RSP
	while (!(REG_SP_STATUS & REG_SP_STATUS_BIT_HALT)) { } // wait for HALT signal
	REG_SP_STATUS = (
		REG_SP_STATUS_BIT_SET_HALT |
		REG_SP_STATUS_BIT_CLEAR_BROKE |
		REG_SP_STATUS_BIT_CLEAR_INTR |
		REG_SP_STATUS_BIT_CLEAR_SSTEP |
		REG_SP_STATUS_BIT_CLEAR_INTR_ON_BREAK |
		REG_SP_STATUS_BIT_CLEAR_SIGNAL0 | REG_SP_STATUS_BIT_CLEAR_SIGNAL1 | REG_SP_STATUS_BIT_CLEAR_SIGNAL2 | REG_SP_STATUS_BIT_CLEAR_SIGNAL3 |
		REG_SP_STATUS_BIT_CLEAR_SIGNAL4 | REG_SP_STATUS_BIT_CLEAR_SIGNAL5 | REG_SP_STATUS_BIT_CLEAR_SIGNAL6 | REG_SP_STATUS_BIT_CLEAR_SIGNAL7
	);
	REG_SP_SEMAPHORE = 0; //Set semaphore by writing
	REG_SP_PC = 0; //Set the program counter to 0
	while (REG_SP_DMA_BUSY & REG_SP_DMA_BUSY_BIT_HALT) { }
	
	REG_PI_STATUS = (REG_PI_STATUS_BIT_RESET_DMA | REG_PI_STATUS_BIT_CLR_INTR);
	
	REG_SI_STATUS = 0;
	while ((REG_VI_CUR_LINE & REG_VI_CUR_LINE_CURRENT_MASK) != 0) { }
	REG_VI_V_INTR = REG_VI_V_INTR_MASK; //Set the VI interrupt threshold to the default value (libdragon will set it back to '2' during initialization)
	REG_VI_H_LIMITS = 0;
	REG_VI_CUR_LINE = 0;
	REG_AI_DRAM_ADDR = 0; REG_AI_DRAM_LEN = 0; //Clear DMA address/size on Audio Interface (AI)
	while (REG_SP_STATUS & REG_SP_STATUS_BIT_DMA_BUSY) { } // wait for DMA_BUSY to go false
	
	// Copy trampoline into RSP IMEM
	volatile u32* imem = (volatile u32*)(uintptr_t)(KSEG1_START_ADDR + RSP_MEMORY_BASE + RSP_IMEM_START);
	u32 *src = &reboot_start;
	size_t words = (size_t)(&reboot_end - &reboot_start);
	for (size_t i = 0; i < words; i++) { imem[i] = src[i]; }
	
	// Set PI DOM0 timing from the target ROM header, via safe defaults first
	u32 romHeaderBase = (KSEG1_START_ADDR + 0x10000000u);
	BlockingPiWrite(&REG_PI_DOM0_LAT, 0xFF);
	BlockingPiWrite(&REG_PI_DOM0_PWD, 0xFF);
	BlockingPiWrite(&REG_PI_DOM0_PGS, 0x0F);
	BlockingPiWrite(&REG_PI_DOM0_RLS, 0x03);
	u32 romHeaderConfig = BlockingPiRead((volatile u32*)romHeaderBase);
	BlockingPiWrite(&REG_PI_DOM0_LAT, (romHeaderConfig >>  0) & 0xFF);
	BlockingPiWrite(&REG_PI_DOM0_PWD, (romHeaderConfig >>  8) & 0xFF);
	BlockingPiWrite(&REG_PI_DOM0_PGS, (romHeaderConfig >> 16) & 0xFF);
	BlockingPiWrite(&REG_PI_DOM0_RLS, (romHeaderConfig >> 20) & 0xFF);
	
	// Check RDP Command Status Register XBUS_DMEM_DMA flag and wait for PIPE_BUSY to clear
	//TODO: Is this basically waiting for the commands to flush? Aka the pipe is no longer busy?
	if (REG_DPC_STATUS & REG_DPC_STATUS_BIT_XBUS_DMEM_DMA)
	{
		while (REG_DPC_STATUS & REG_DPC_STATUS_BIT_PIPE_BUSY) { }
	}
	
	// Copy target IPL3 (words 16..1023) into RSP DMEM
	volatile u32* dmem = (volatile u32*)(uintptr_t)(KSEG1_START_ADDR + RSP_MEMORY_BASE + RSP_DMEM_START);
	for (int i = 16; i < 1024; i++) { dmem[i] = BlockingPiRead((volatile u32*)(romHeaderBase + (u32)i*4)); }
	
	register u32 s3 __asm__("s3") = 0; //0=cart, 1=device (TODO: For 64DD support maybe? Is it even used in the assembly code above?) 
	register u32 s4 __asm__("s4") = (tvType & 3);
	register u32 s5 __asm__("s5") = 1; //0=Full reset 1=Fast reset (Keep RDRAM mapping)
	register u32 s6 __asm__("s6") = cicSeed;
	register u32 s7 __asm__("s7") = (tvType==0 ? 6u : (tvType==1 ? 1u : (tvType==2 ? 4u : 0u)));
	__asm__ volatile (
		".set noreorder \n"
		"li $t3, reboot \n" // We use "li" here because the assembly is in this file, same compilation unit, absolute address
		"jr $t3         \n"
		"nop            \n"
		".set reorder   \n"
		:: "r"(s3),"r"(s4),"r"(s5),"r"(s6),"r"(s7) : "t3"
	);
	
	__builtin_unreachable();
}
