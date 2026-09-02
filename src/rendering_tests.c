/*
File:   rendering_tests.c
Author: Taylor Robbins
Date:   09\01\2026
Description: 
	** Holds a bunch of random functions that we made while learning the N64 hardware
	** and libdragon API. These tests mostly have to do with rendering but may also
	** interact with joypad input, audio output, save data, and other capabilities.
*/

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

// void Test_Render3dCar()
// {
// 	if (rom.carModel.data == nullptr)
// 	{
// 		//TODO: model64_free(model);
// 		rom.carModel = model64_load(CAR_MODEL_PATH);
// 	}
// }
