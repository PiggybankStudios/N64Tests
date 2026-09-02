/*
File:   rom.c
Author: Taylor Robbins
Date:   09\01\2026
Description: 
	** This holds the majority of the top-level logic that drives the entire cart
	** We use the name "ROM" to refer to Read-Only Memory, which is the portion
	** of the cart that is normally backed up and is fed into emulators. In a way
	** the term "rom" is synonomous with an N64 "game". However, it is useful to have
	** a different name for this top-level logic since it often has to do more with
	** hardware interactions and less to do with gameplay logic. That way we can
	** use the word "game" to refer to things that actual affect the gameplay directly
*/

RomState rom = {0};

// +--------------------------------------------------------------+
// |                     Main ROM Initialize                      |
// +--------------------------------------------------------------+
void InitRom()
{
	// Initialize debug output (both for emulator and for USB serial bus from flash cart)
	debug_init_emulog();
	rom.usbDebugAvailable = debug_init_usblog();
	
	// Initialize the D File System, let the system find the location using TOC in the rompak
	int dfsResult = dfs_init(DFS_DEFAULT_LOCATION);
	debugf("dfsResult = %d\n", dfsResult);
	
	// Initialize joypad system so we can get input from the player
	joypad_init();
	
	// Initialize timer (TODO: So we can do TICKS_READ() or get_ticks_ms() or something later?)
	timer_init();
	
	// Initialize the Real-Time Clock (if it exists in the cart, and is powered)
	rom.rtcAvailable = rtc_init();
	debugf("rtcAvailable = %s\n", rom.rtcAvailable ? "YES" : "NO");
	
	// Initialize the display to 640x480 resolution - 16-bit - 2 buffers
	display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
	
	// Initialize the RDP (Command) Queue so we can send commands to the Reality Display Processor
	rdpq_init();
	
	// Initialize the OpenGL-like API for rendering
	// gl_init();
	
	rdpq_text_register_font(DEBUG_FONT_ID, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR)); //FONT_BUILTIN_DEBUG_MONO
	
	rom.prevRomTime = get_ticks_ms();
	rom.romTime = rom.prevRomTime;
	rom.elapsedMs = 0;
	
	rom.shutdown = false;
	rom.initialized = true;
	debugf("Initialized!\n");
}

// +--------------------------------------------------------------+
// |                       Main ROM Update                        |
// +--------------------------------------------------------------+
void UpdateRom()
{
	joypad_poll();
	joypad_buttons_t pads[4];
	pads[0] = joypad_get_buttons_pressed(JOYPAD_PORT_1);
	pads[1] = joypad_get_buttons_pressed(JOYPAD_PORT_2);
	pads[2] = joypad_get_buttons_pressed(JOYPAD_PORT_3);
	pads[3] = joypad_get_buttons_pressed(JOYPAD_PORT_4);
	
	rom.prevRomTime = rom.romTime;
	rom.romTime = get_ticks_ms();
	rom.elapsedMs = (rom.romTime >= rom.prevRomTime) ? (rom.romTime - rom.prevRomTime) : 0;
	
	if (pads[0].a && !rom.prevPadStates[0].a) { debugf("A Button was Pressed!\n"); }
	
	rom.prevPadStates[0] = pads[0];
	rom.prevPadStates[1] = pads[1];
	rom.prevPadStates[2] = pads[2];
	rom.prevPadStates[3] = pads[3];
}

// +--------------------------------------------------------------+
// |                       Main ROM Render                        |
// +--------------------------------------------------------------+
void RenderRom()
{
	// Acquire a free framebuffer for rendering
	surface_t* disp = display_get();
	// Attach the framebuffer for use by rdpq
	rdpq_attach(disp, NULL);
	// Clear the framebuffer with black
	rdpq_clear((color_t){59, 58, 50, 0}); // 0x3B3A32
	
	for (int y = 0; y < 480; y+=5)
	{
		rdpq_set_mode_fill(RGBA32(100, 32+y/4, 100, 0xFF));
		rdpq_fill_rectangle(0, y, 640, y+5);
	}
	
	// rdpq_text_print(NULL, DEBUG_FONT_ID, 15, 15, rom.rtcAvailable      ? "RTC: Available"           : "RTC: NOT AVAILABLE"          );
	// rdpq_text_print(NULL, DEBUG_FONT_ID, 15, 25, rom.usbDebugAvailable ? "USB Debugging: Available" : "USB Debugging: NOT AVAILABLE");
	
	char printBuffer[32];
	snprintf(&printBuffer[0], sizeof(printBuffer), "romTime: %lu", rom.romTime);
	rdpq_text_print(NULL, DEBUG_FONT_ID, 15, 15, &printBuffer[0]);
	
	Test_RenderDfsEntries();
	
	// Detach the framebuffer and show it on screen when it's ready
	// (when previous rendering operations have completed)
	rdpq_detach_show();
}
