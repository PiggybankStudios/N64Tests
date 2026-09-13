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
	for (int y = 0; y < 480; y+=5)
	{
		rdpq_set_mode_fill(RGBA32(180, 180, 180+y/2, 0xFF));
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

// +--------------------------------------------------------------+
// |                        Car Rendering                         |
// +--------------------------------------------------------------+

static const GLfloat light_ambient[] = {0.12f, 0.12f, 0.12f, 1.0f};
static const GLfloat key_diffuse[] = {1.00f, 0.95f, 0.85f, 1.0f};
static const GLfloat fill_diffuse[] = {0.35f, 0.40f, 0.50f, 1.0f};
static const GLfloat rim_diffuse[] = {0.60f, 0.60f, 0.70f, 1.0f};

void Test_Init3dScene()
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
	
	rom.carModel = model64_load(CAR_MODEL_PATH);
	// rom.carModel = model64_load(ERROR_MODEL_PATH);
	// rom.carModel = model64_load(UNIT_BOX_MODEL_PATH);
	rom.planetModel = model64_load(PLANET_MODEL_PATH);
	rom.origPlanetOffset = MakeV3(1.5f, -1.7f, -11.0f);
	rom.planetOffset = rom.origPlanetOffset;
	rom.planetOffsetGoto = rom.planetOffset;
	
	rom.planetCollision = LoadCollisionSceneFromModel(StrLit(PLANET_MODEL_PATH));
	// rom.planetCollision = LoadCollisionSceneFromModel(StrLit(CAR_MODEL_PATH));
	debugf("collision has %lu faces\n", rom.planetCollision.numFaces);
	debugf("collision bounds=(%g,%g,%g, %g,%g,%g)\n",
		rom.planetCollision.bounds.x, rom.planetCollision.bounds.y, rom.planetCollision.bounds.z,
		rom.planetCollision.bounds.width, rom.planetCollision.bounds.height, rom.planetCollision.bounds.depth
	);
	
	rom.drawClosestFace = (false && DEBUG_BUILD);
	
	rom.carRotation = 0;
}

void Test_Update3dScene()
{
	rom.carRotation += rom.timeScale * 5.0f;
	if (rom.carRotation >= 360.0f) { rom.carRotation -= 360.0f; }
	
	if (rom.joy[0].btn.z && !rom.prevJoy[0].btn.z)
	{
		rom.planetOffset = rom.origPlanetOffset;
		rom.planetOffsetGoto = rom.planetOffset;
	}
	
	if (rom.joy[0].btn.b && !rom.prevJoy[0].btn.b)
	{
		rom.drawClosestFace = !rom.drawClosestFace;
		debugf("Closest face rendering %s!\n", rom.drawClosestFace ? "Enabled" : "Disabled");
	}
	
	if (rom.joy[0].btn.d_right) { rom.planetOffsetGoto.x -= PLANET_DBG_MOVE_HORI_SPEED; }
	if (rom.joy[0].btn.d_down)  { rom.planetOffsetGoto.z -= PLANET_DBG_MOVE_HORI_SPEED; }
	if (rom.joy[0].btn.d_left)  { rom.planetOffsetGoto.x += PLANET_DBG_MOVE_HORI_SPEED; }
	if (rom.joy[0].btn.d_up)    { rom.planetOffsetGoto.z += PLANET_DBG_MOVE_HORI_SPEED; }
	if (rom.joy[0].btn.r)       { rom.planetOffsetGoto.y -= PLANET_DBG_MOVE_VERT_SPEED; }
	if (rom.joy[0].btn.l)       { rom.planetOffsetGoto.y += PLANET_DBG_MOVE_VERT_SPEED; }
	
	v2 stickVec = MakeV2((r32)rom.joy[0].stick_x / 127.0f, (r32)rom.joy[0].stick_y / 127.0f);
	r32 stickLengthSquared = LengthSquaredV2(stickVec);
	bool stickNotInDeadzone = (stickLengthSquared > STICK_DEADZONE * STICK_DEADZONE);
	if (stickNotInDeadzone)
	{
		rom.planetOffsetGoto.x -= stickVec.x * PLANET_ANALOG_MOVE_SPEED;
		rom.planetOffsetGoto.z += stickVec.y * PLANET_ANALOG_MOVE_SPEED;
	}
	
	if (rom.drawClosestFace)
	{
		v3 carPos = SubV3(CAR_OFFSET, rom.planetOffsetGoto);
		// rom.closestFace = FindClosestFace(&rom.planetCollision, carPos, &rom.closestFaceDistance);
		
		rom.closestFace = FindCurrentCollisionFace(&rom.planetCollision, carPos, COLL_GROUND_THICKNESS, &rom.carAltitude);
		if (rom.closestFace != nullptr)
		{
			if (rom.carAltitude < 0)
			{
				rom.planetOffsetGoto = AddV3(rom.planetOffsetGoto, ScaleV3(rom.closestFace->normal, rom.carAltitude - EPSILON));
			}
		}
	}
	
	v3 planetOffsetDiff = SubV3(rom.planetOffsetGoto, rom.planetOffset);
	if (LengthSquaredV3(planetOffsetDiff) > 0.001f || stickNotInDeadzone)
	{
		rom.planetOffset = AddV3(rom.planetOffset, ShrinkV3(planetOffsetDiff, MOVEMENT_LAG_DIVISOR));
	}
	else if (!AreEqualV3(rom.planetOffset, rom.planetOffsetGoto))
	{
		rom.planetOffset = rom.planetOffsetGoto;
	}
	
}

void Test_Render3dScene()
{
	gl_context_begin();
	{
		// glClearColor(0.243f, 0.25f, 0.33f, 1.0f); // BG color
		glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);
		
		// update_light_positions();
		static const GLfloat key_pos[] = {2.5f, 2.0f, 2.5f, 1.0f};
		static const GLfloat fill_pos[] = {-2.5f, 1.0f, 2.0f, 1.0f};
		static const GLfloat rim_pos[] = {0.0f, 2.5f, -2.5f, 1.0f};
		glLightfv(GL_LIGHT0, GL_POSITION, key_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, fill_pos);
		glLightfv(GL_LIGHT2, GL_POSITION, rim_pos);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		v3 carOffset = CAR_OFFSET;
		glTranslatef(carOffset.x, carOffset.y, carOffset.z);
		glRotatef(rom.carRotation, 0.0f, 1.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		model64_draw(rom.carModel);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(rom.planetOffset.x, rom.planetOffset.y, rom.planetOffset.z);
		model64_draw(rom.planetModel);
		
		if (rom.closestFace != nullptr && rom.drawClosestFace)
		{
			v3 vert0 = AddV3(rom.planetOffset, rom.closestFace->verts[0]);
			v3 vert1 = AddV3(rom.planetOffset, rom.closestFace->verts[1]);
			v3 vert2 = AddV3(rom.planetOffset, rom.closestFace->verts[2]);
			v3 center = ShrinkV3(AddV3(AddV3(vert0, vert1), vert2), 3);
			v3 faceTangent1 = CrossV3(rom.closestFace->normal, V3_Right);
			v3 faceTangent2 = CrossV3(faceTangent1, rom.closestFace->normal);
			v3 normalScaled = ScaleV3(rom.closestFace->normal, 1.5f);
			v3 faceTangent1Scaled = ScaleV3(faceTangent1, 1.5f);
			v3 faceTangent2Scaled = ScaleV3(faceTangent2, 1.5f);
			
			DrawBoxAt(vert0, 0.2f);
			DrawBoxAt(vert1, 0.2f);
			DrawBoxAt(vert2, 0.2f);
			DrawBoxAt(AddV3(center, normalScaled), 0.2f);
			DrawBoxAt(AddV3(center, faceTangent1Scaled), 0.2f);
			DrawBoxAt(AddV3(center, faceTangent2Scaled), 0.2f);
			
			DrawLineBox(vert0, vert1, 0.15f);
			DrawLineBox(vert1, vert2, 0.15f);
			DrawLineBox(vert2, vert0, 0.15f);
			
			// DrawLineBox(center, AddV3(center, normalScaled), 0.15f);
		}
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	gl_context_end();
}
