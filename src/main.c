/*
File:   main.c
Author: Taylor Robbins
Date:   08\22\2026
Description: 
	** This is the main file that gets compiled. All other source files are #included from here
*/
#include <libdragon.h>

static bool showSecondText = false;

static bool str_ends_with(const char *str, const char *suffix)
{
	size_t str_len = strlen(str);
	size_t suffix_len = strlen(suffix);
	if (suffix_len > str_len) { return false; }
	return strcmp(str + str_len - suffix_len, suffix) == 0;
}

int main(void)
{
	// console_init(); //Calls display_init(...) internally below
	dfs_init(DFS_DEFAULT_LOCATION);
	joypad_init();
	debug_init_emulog();
	debug_init_usblog();
	timer_init();
	rtc_init();
	
	// libdragon subsystems are typically initialized on demand by calling
	// *_init functions, like here we initialize the display and rdpq systems.
	display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
	rdpq_init();
	
	// Register a built-in font as id 1
	rdpq_text_register_font(1, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR));
	
	while (true)
	{
		joypad_poll();
		joypad_buttons_t pad1 = joypad_get_buttons_pressed(JOYPAD_PORT_1);
		if (pad1.a) { showSecondText = true;  /*debugf("Pressed A!");*/ }
		if (pad1.b) { showSecondText = false; /*debugf("Pressed B!");*/ }
		
		// Acquire a free framebuffer for rendering
		surface_t* disp = display_get();
		// Attach the framebuffer for use by rdpq
		rdpq_attach(disp, NULL);
		
		// Clear the framebuffer with black
		rdpq_clear((color_t){0, 0, 0, 0});
		
		// Print some text
		if (!showSecondText)
		{
			uint32_t tv_type = get_tv_type();
			if (tv_type == TV_PAL) { rdpq_text_print(NULL, 1, 50, 50,  "This console is PAL"); }
			else if (tv_type == TV_NTSC) { rdpq_text_print(NULL, 1, 50, 50,  "This console is NTSC"); }
			else if (tv_type == TV_MPAL) { rdpq_text_print(NULL, 1, 50, 50,  "This console is MPAL"); }
		}
		else
		{
			int yPos = 50;
			dir_t dir = {0};
			if (dir_findfirst("rom:/models", &dir) == 0)
			{
				do
				{
					if (dir.d_type != DT_REG) { continue; }
					// if (!str_ends_with(dir.d_name, ".model64")) { continue; }
					// char full_path[MAX_MODEL_PATH];
					// snprintf(full_path, sizeof(full_path), "%s/%s", base_path, dir.d_name);
					// char name_buf[MAX_MODEL_NAME];
					// strncpy(name_buf, dir.d_name, sizeof(name_buf) - 1);
					// name_buf[sizeof(name_buf) - 1] = '\0';
					// char *ext = strstr(name_buf, ".model64");
					// if (ext) { *ext = '\0'; }
					// model_list_add(list, full_path, name_buf);
					rdpq_text_print(NULL, 1, 50, yPos, dir.d_name);
					yPos += 15;
				} while (dir_findnext("rom:/models", &dir) == 0);
			}
			
			// rdpq_text_print(NULL, 1, 50, 50,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 51,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 53,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 56,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 60,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 65,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 71,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 78,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 86,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 95,  "This is a test program with lots of text to see how performance looks!");
			// rdpq_text_print(NULL, 1, 50, 105, "This is a test program with lots of text to see how performance looks!");
		}
		
		// Detach the framebuffer and show it on screen when it's ready
		// (when previous rendering operations have completed)
		rdpq_detach_show();
	}
}
