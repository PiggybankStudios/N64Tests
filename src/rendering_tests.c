/*
File:   rendering_tests.c
Author: Taylor Robbins
Date:   09\01\2026
Description: 
	** Holds a bunch of random functions that we made while learning the N64 hardware
	** and libdragon API. These tests mostly have to do with rendering but may also
	** interact with joypad input, audio output, save data, and other capabilities.
*/

void Test_RenderGradientWithBoxes()
{
	rdpq_set_mode_fill(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
	Color32 topColor = MakeColor(0x25, 0x12, 0x14, 0xFF);
	Color32 bottomColor = MakeColor(0xFF, 0xD8, 0x8B, 0xFF);
	for (int y = 0; y < 480; y+=5)
	{
		Color32 barColor = Color32Lerp(topColor, bottomColor, (r32)(y+5) / 480.0f);
		// debugf("Line[%d] is r=0x%02X g=0x%02X b=0x%02X a=0x%02X\n", y, barColor.r, barColor.g, barColor.b, barColor.a);
		rdpq_set_fill_color(RGBA32(barColor.r, barColor.g, barColor.b, 0xFF));
		rdpq_fill_rectangle(0, y, 640, y+5);
	}
}

void Test_RenderTypeSizes()
{
	int yPos = 50;
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(int)       = %u", sizeof(int));       yPos += 15; //4 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(long)      = %u", sizeof(long));      yPos += 15; //4 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(long long) = %u", sizeof(long long)); yPos += 15; //8 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(float)     = %u", sizeof(float));     yPos += 15; //4 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(double)    = %u", sizeof(double));    yPos += 15; //8 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(char)      = %u", sizeof(char));      yPos += 15; //1 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(void*)     = %u", sizeof(void*));     yPos += 15; //4 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(int*)      = %u", sizeof(int*));      yPos += 15; //4 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(size_t)    = %u", sizeof(size_t));    yPos += 15; //4 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(intptr_t)  = %u", sizeof(intptr_t));  yPos += 15; //4 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(uintptr_t) = %u", sizeof(uintptr_t)); yPos += 15; //4 bytes
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 50, yPos, "sizeof(ptrdiff_t) = %u", sizeof(ptrdiff_t)); yPos += 15; //4 bytes
}

void Test_RenderDfsEntries()
{
	int yPos = 50;
	dir_t dir = {0};
	if (dir_findfirst(MODELS_FOLDER, &dir) == 0)
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
			rdpq_text_print(NULL, DEBUG_FONT_ID, 50, yPos, dir.d_name);
			yPos += 15;
		} while (dir_findnext(MODELS_FOLDER, &dir) == 0);
	}
}
