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

#if ENABLE_CAR_RENDER

static const GLfloat light_ambient[] = {0.12f, 0.12f, 0.12f, 1.0f};
static const GLfloat key_diffuse[] = {1.00f, 0.95f, 0.85f, 1.0f};
static const GLfloat fill_diffuse[] = {0.35f, 0.40f, 0.50f, 1.0f};
static const GLfloat rim_diffuse[] = {0.60f, 0.60f, 0.70f, 1.0f};

void Test_Init3dCar()
{
	float aspect = (float)display_get_width() / (float)display_get_height();
	float near_plane = 1.0f;
	float far_plane = 50.0f;
	
	gl_init();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-near_plane * aspect, near_plane * aspect, -near_plane,
	          near_plane, near_plane, far_plane);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	
	glLightfv(GL_LIGHT0, GL_DIFFUSE, key_diffuse);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, fill_diffuse);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, rim_diffuse);
	
	glEnable(GL_NORMALIZE);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	rom.carRotation1 = 0;
}

void Test_Render3dCar()
{
	if (rom.carModel == 0)
	{
		debugf("Loading Car model...\n");
		//TODO: model64_free(model);
		rom.carModel = model64_load(CAR_MODEL_PATH);
	}
	
	rom.carRotation1 += rom.timeScale * 3.0f;
	rom.carRotation2 += rom.timeScale * 1.73f;
	if (rom.carRotation1 >= 360.0f) { rom.carRotation1 -= 360.0f; }
	if (rom.carRotation2 >= 360.0f) { rom.carRotation2 -= 360.0f; }
	
	gl_context_begin();
	
	glClearColor(0.243f, 0.25f, 0.33f, 1.0f); // BG color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, -0.5f, -3.0f);
	
	// update_light_positions();
	static const GLfloat key_pos[] = {2.5f, 2.0f, 2.5f, 1.0f};
	static const GLfloat fill_pos[] = {-2.5f, 1.0f, 2.0f, 1.0f};
	static const GLfloat rim_pos[] = {0.0f, 2.5f, -2.5f, 1.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, key_pos);
	glLightfv(GL_LIGHT1, GL_POSITION, fill_pos);
	glLightfv(GL_LIGHT2, GL_POSITION, rim_pos);
    
	glRotatef(rom.carRotation1, rom.carRotation2, 1.0f, 0.0f);

	if (rom.carModel != 0)
	{
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		model64_draw(rom.carModel);
	}
	
	gl_context_end();
}

#endif //ENABLE_CAR_RENDER
