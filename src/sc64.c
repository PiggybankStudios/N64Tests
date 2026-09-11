/*
File:   sc64.c
Author: Taylor Robbins
Date:   09\09\2026
Description:
	** Contains #defines and a few helpers for interacting with SummerCart64 functionality through registers
	** See https://github.com/Polprzewodnikowy/SummerCart64/blob/main/docs/01_memory_map.md#sc64-registers
	**
	** Mostly we just handle the "reboot" signal sent to us by sc64deployer.exe
	** through the AUX register when the "--reboot" option is passed.
	** Once we get that signal we call SoftRebootN64(...) from reboot.c
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

#define SC64_REGVAL_ASCII_UNL  0x5F554E4C // ASCII="_UNL" 
#define SC64_REGVAL_ASCII_OCK  0x4F434B5F // ASCII="OCK_"
#define SC64_REGVAL_ASCII_SCv2 0x53437632 // ASCII="SCv2"

// Call this once during initialization, otherwise polling AUX will never return anything
void SC64_PerformUnlockSequence()
{
	io_write(SC64_REG_KEY, 0x00000000); // reset sequencer
	io_write(SC64_REG_KEY, SC64_REGVAL_ASCII_UNL);
	io_write(SC64_REG_KEY, SC64_REGVAL_ASCII_OCK);
}

// Poll this in the update loop
// NOTE: Keep polling it even when a HALT has been received (but avoid doing all other work)
bool SC64_PollAUX()
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
		// case SC64_AUX_MSG_REBOOT: SoftRebootSC64(); break;
		case SC64_AUX_MSG_REBOOT: SoftRebootN64(/*cart=*/0, /*fast*/1, /*6102*/0x3F); break;
		default: /*Unknown message, do nohing*/ break;
	}
	return haltReceived;
}
