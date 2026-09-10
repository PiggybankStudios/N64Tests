/*
File:   sc64.c
Author: Taylor Robbins
Date:   09\09\2026
Description:
	** Contains #defines and a few helpers for interacting with SummerCart64 functionality through registers
	** See https://github.com/Polprzewodnikowy/SummerCart64/blob/main/docs/01_memory_map.md#sc64-registers
*/

#define SC64_REG_SCR 0x1FFF0000
#define SC64_REG_ID  0x1FFF000C
#define SC64_REG_KEY 0x1FFF0010
#define SC64_REG_IRQ 0x1FFF0014
#define SC64_REG_AUX 0x1FFF0018

#define SC64_REG_BIT_SCR_AUX_IRQ_PENDING (1<<23)

#define SC64_REG_BIT_AUX_CLEAR (1<<28)

#define SC64_AUX_MSG_PING   0xFF000000
#define SC64_AUX_MSG_HALT   0xFF000001
#define SC64_AUX_MSG_REBOOT 0xFF000002

// KSEG1 uncached=0xB0000000 and then IPL3 is offset +0x40 from that
#define KSEG1_UNCACHED_ADDRESS 0xB0000000u
#define KSEG1_RSP_DMEM_ADDRESS 0xA4000000u
#define IPL3_SIZE              0x1000u
#define IPL3_JUMP_OFFSET       0x40u //TODO: Is this correctly named? Is this the size of IPL1/IPL2?
#define IPL3_JUMP_ADDRESS      (KSEG1_RSP_DMEM_ADDRESS + IPL3_JUMP_OFFSET)

#define SC64_REGVAL_ASCII_UNL  0x5F554E4C // ASCII="_UNL" 
#define SC64_REGVAL_ASCII_OCK  0x4F434B5F // ASCII="OCK_"
#define SC64_REGVAL_ASCII_SCv2 0x53437632 // ASCII="SCv2"

// #define MI_MASK    ((volatile uint32_t*)0xA430000C)
// #define SP_STATUS  ((volatile uint32_t*)0xA4040010)
#define DPC_STATUS ((volatile uint32_t*)0xA410000C)
// #define PI_STATUS  ((volatile uint32_t*)0xA4600010)
#define AI_CTRL    ((volatile uint32_t*)0xA4500008)
#define SI_STATUS  ((volatile uint32_t*)0xA4800018)
#define VI_CURRENT ((volatile uint32_t*)0xA4400010)
#define AI_STATUS  ((volatile uint32_t*)0xA450000C)
// #define MI_MODE    ((volatile uint32_t*)0xA4300000)

__attribute__((noreturn, noinline))
static void JumpToIpl3(void)
{
	// caches must be coherent before executing freshly-copied code
	data_cache_hit_writeback_invalidate((void*)KSEG1_RSP_DMEM_ADDRESS, IPL3_SIZE);
	inst_cache_hit_invalidate((void*)IPL3_JUMP_ADDRESS, IPL3_SIZE - IPL3_JUMP_OFFSET);
	void (*entry)(void) = (void(*)(void))IPL3_JUMP_ADDRESS;
	entry();
	__builtin_unreachable();
}

__attribute__((noreturn))
void SoftRebootSC64(void)
{
	disable_interrupts();
	
	*SP_STATUS = 0x0A;
	while (!((*SP_STATUS) & (1<<0))) { }
	*DPC_STATUS = (1<<3)|(1<<2)|(1<<1);
	*AI_CTRL = 0;
	while ((*PI_STATUS) & 0x3) { }
	*MI_MASK = 0x555;
	*SI_STATUS  = 0;
	*VI_CURRENT = 0;
	*PI_STATUS  = 0x2;
	*AI_STATUS  = 0;
	*MI_MODE    = (1<<11);
	
	// CPU word copy (NOT PI DMA) of header+IPL3 into DMEM
	volatile uint32_t* src = (volatile uint32_t*)KSEG1_UNCACHED_ADDRESS;
	volatile uint32_t* dst = (volatile uint32_t*)KSEG1_RSP_DMEM_ADDRESS;
	for (int wordIndex = 0; wordIndex < IPL3_SIZE/sizeof(uint32_t); wordIndex++) { dst[wordIndex] = src[wordIndex]; }
	// ---- MISSING: set s3..s7 / sp / entropy per the target CIC's boot contract ----
	JumpToIpl3();
}

// Call this once during initialization, otherwise polling AUX will never return anything
void PerformUnlockSequenceSC64()
{
	io_write(SC64_REG_KEY, 0x00000000); // reset sequencer
	io_write(SC64_REG_KEY, SC64_REGVAL_ASCII_UNL);
	io_write(SC64_REG_KEY, SC64_REGVAL_ASCII_OCK);
}

// Poll this in the update loop
// NOTE: Keep polling it even when a HALT has been received (but avoid doing all other work)
bool PollAuxSC64()
{
	static bool haltReceived = false;
	if (io_read(SC64_REG_ID) != SC64_REGVAL_ASCII_SCv2) { return haltReceived; } // Check if this is a v2 SummerCart
	if (!(io_read(SC64_REG_SCR) & SC64_REG_BIT_SCR_AUX_IRQ_PENDING)) { return haltReceived; } // Check if AUX_IRQ_PENDING bit is set
	
	uint32_t auxMsg = io_read(SC64_REG_AUX);
	io_write(SC64_REG_IRQ, SC64_REG_BIT_AUX_CLEAR);
	io_write(SC64_REG_AUX, auxMsg); //echo the message back as an acknowledgement
	
	switch (auxMsg)
	{
		case SC64_AUX_MSG_PING: break;
		case SC64_AUX_MSG_HALT: haltReceived = true; break;
		case SC64_AUX_MSG_REBOOT: SoftRebootSC64(); break;
		default: /*Unknown message, do nohing*/ break;
	}
	return haltReceived;
}
