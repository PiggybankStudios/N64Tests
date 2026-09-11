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

extern uint32_t reboot_start;
extern uint32_t reboot_end;

/* ------------------------------------------------------------------ *
 *  RCP register access (uncached KSEG1). RCP regs are on the system
 *  bus (no PI wait). Cart/ROM/SC64 regs are on the PI bus (busy-wait).
 * ------------------------------------------------------------------ */
#define REGISTER(address) (*(volatile uint32_t*)(uintptr_t)(address))

#define REG_ADDR_SP_SR       0xA4040010u
#define REG_ADDR_SP_DMA_BUSY 0xA4040018u
#define REG_ADDR_SP_SEM      0xA404001Cu
#define REG_ADDR_SP_PC       0xA4080000u
#define REG_ADDR_DPC_SR      0xA410000Cu
#define REG_ADDR_VI_V_INTR   0xA440000Cu
#define REG_ADDR_VI_CUR_LINE 0xA4400010u
#define REG_ADDR_VI_H_LIMITS 0xA4400024u
#define REG_ADDR_AI_MADDR    0xA4500000u
#define REG_ADDR_AI_LEN      0xA4500004u
#define REG_ADDR_PI_SR       0xA4600010u
#define REG_ADDR_PI_DOM0_LAT 0xA4600014u   /* LAT, PWD, PGS, RLS at +0,4,8,C */
#define REG_ADDR_PI_DOM0_PWD 0xA4600018u
#define REG_ADDR_PI_DOM0_PGS 0xA460001Cu
#define REG_ADDR_PI_DOM0_RLS 0xA4600020u
#define REG_ADDR_SI_SR       0xA4800018u

#define PI_BUSY     0x3u          /* DMA_BUSY | IO_BUSY */

static inline uint32_t BlockingPiRead(uint32_t a){ while (REGISTER(REG_ADDR_PI_SR)&PI_BUSY){} return REGISTER(a); }
static inline void     BlockingPiWrite(uint32_t a, uint32_t v){ while (REGISTER(REG_ADDR_PI_SR)&PI_BUSY){} REGISTER(a)=v; }

// clear SIG0..7 | INTR_BREAK | SSTEP | INTR | BROKE | SET_HALT
#define SP_HALT_WRITE 0x00AAAAAEu

// ------------------------------------------------------------------
// Re-runs the boot process from IPL3 on without powering off
//    device:     0 = cart (0xB0000000), 1 = 64DD IPL (0xB6000000)
//    fastReset:  0 = cold (reset RDRAM), 1 = NMI/fast (keep RDRAM)
//    seed:       CIC seed (6102/7101 = 0x3F). Must match the target ROM.
// ------------------------------------------------------------------
void SoftRebootN64(uint32_t device, uint32_t fastReset, uint32_t seed)
{
	disable_interrupts();
	uint32_t tvType = get_tv_type();
	
	__asm__ volatile ("li $t0,0x34000000\n mtc0 $t0,$12\n" ::: "t0"); // CU1|CU0|FR
	
	while (!(REGISTER(REG_ADDR_SP_SR) & (1u<<0))) {}          // wait SP HALT
	REGISTER(REG_ADDR_SP_SR) = SP_HALT_WRITE;
	REGISTER(REG_ADDR_SP_SEM) = 0;
	REGISTER(REG_ADDR_SP_PC)  = 0;
	while (REGISTER(REG_ADDR_SP_DMA_BUSY)) {}
	
	REGISTER(REG_ADDR_PI_SR) = (1u<<1)|(1u<<0);               // CLR_INTR | RESET
	REGISTER(REG_ADDR_SI_SR) = 0;
	while ((REGISTER(REG_ADDR_VI_CUR_LINE) & ~1u) != 0) {}    // mask VI_CURR_LINE_FIELD
	REGISTER(REG_ADDR_VI_V_INTR)   = 0x3FF;
	REGISTER(REG_ADDR_VI_H_LIMITS) = 0;
	REGISTER(REG_ADDR_VI_CUR_LINE) = 0;
	REGISTER(REG_ADDR_AI_MADDR)    = 0;
	REGISTER(REG_ADDR_AI_LEN)      = 0;
	while (REGISTER(REG_ADDR_SP_SR) & (1u<<2)) {}             // SP DMA_BUSY
	
	// copy trampoline into RSP IMEM
	volatile uint32_t* imem = (volatile uint32_t*)(uintptr_t)0xA4001000u;
	uint32_t *src   = &reboot_start;
	size_t    words = (size_t)(&reboot_end - &reboot_start);
	for (size_t i = 0; i < words; i++) imem[i] = src[i];
	
	// set PI DOM0 timing from the target ROM header, via safe defaults first
	uint32_t base = device ? 0xB6000000u : 0xB0000000u;
	BlockingPiWrite(REG_ADDR_PI_DOM0_LAT, 0xFF); BlockingPiWrite(REG_ADDR_PI_DOM0_PWD, 0xFF);
	BlockingPiWrite(REG_ADDR_PI_DOM0_PGS, 0x0F); BlockingPiWrite(REG_ADDR_PI_DOM0_RLS, 0x03);
	uint32_t cfg = BlockingPiRead(base);
	BlockingPiWrite(REG_ADDR_PI_DOM0_LAT, cfg & 0xFF);
	BlockingPiWrite(REG_ADDR_PI_DOM0_PWD, (cfg >> 8)  & 0xFF);
	BlockingPiWrite(REG_ADDR_PI_DOM0_PGS, (cfg >> 16) & 0xFF);
	BlockingPiWrite(REG_ADDR_PI_DOM0_RLS, (cfg >> 20) & 0xFF);
	
	if (REGISTER(REG_ADDR_DPC_SR) & (1u<<0))                  // XBUS_DMEM_DMA
	{
		while (REGISTER(REG_ADDR_DPC_SR) & (1u<<5)) {}        // PIPE_BUSY
	}
	
	// copy target IPL3 (words 16..1023) into RSP DMEM
	volatile uint32_t *dmem = (volatile uint32_t*)(uintptr_t)0xA4000000u;
	for (int i = 16; i < 1024; i++) dmem[i] = BlockingPiRead(base + (uint32_t)i*4);
	
	register uint32_t s3 __asm__("s3") = device & 1;
	register uint32_t s4 __asm__("s4") = tvType & 3;
	register uint32_t s5 __asm__("s5") = fastReset & 1;
	register uint32_t s6 __asm__("s6") = seed & 0xFF;
	register uint32_t s7 __asm__("s7") = (tvType==0?6u : tvType==1?1u : tvType==2?4u : 0u);
	
	__asm__ volatile (
		".set noreorder \n"
		"li $t3, reboot \n"       // absolute constant -> lui/ori, no reloc
		"jr $t3         \n"
		"nop            \n"
		".set reorder   \n"
		:: "r"(s3),"r"(s4),"r"(s5),"r"(s6),"r"(s7) : "t3");
	__builtin_unreachable();
}
