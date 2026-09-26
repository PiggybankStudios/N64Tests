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

static const GLfloat light_ambient[] = {0.12f, 0.12f, 0.12f, 1.0f};
static const GLfloat key_diffuse[] = {1.00f, 0.95f, 0.85f, 1.0f};
static const GLfloat fill_diffuse[] = {0.35f, 0.40f, 0.50f, 1.0f};
static const GLfloat rim_diffuse[] = {0.60f, 0.60f, 0.70f, 1.0f};

// +--------------------------------------------------------------+
// |                     Main ROM Initialize                      |
// +--------------------------------------------------------------+
void InitRom()
{
	// Initialize debug output (both for emulator and for USB serial bus from flash cart)
	#if BUILD_FOR_EMULATOR
	rom.debugOutputAvailable = debug_init_emulog();
	#else
	rom.debugOutputAvailable = debug_init_usblog();
	#endif
	
	SC64_PerformUnlockSequence();
	
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
	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
	
	// Initialize the RDP (Command) Queue so we can send commands to the Reality Display Processor
	rdpq_init();
	
	// Initialize the OpenGL-like API for rendering
	// gl_init();
	
	rdpq_text_register_font(DEBUG_FONT_ID, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR)); //FONT_BUILTIN_DEBUG_MONO
	
	rom.prevRomTime = get_ticks_ms();
	rom.romTime = rom.prevRomTime;
	rom.elapsedMs = 0;
	rom.timeScale = 1.0f;
	rom.frameIndex = 0;
	
	// +==============================+
	// |      Initialize OpenGL       |
	// +==============================+
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
	}
	
	MyMemSet(&rom.karts[0], 0x00, sizeof(KartState) * MAX_KARTS);
	for (u32 kartIndex = 0; kartIndex < MAX_KARTS; kartIndex++)
	{
		rom.karts[kartIndex].modelAssetIndex = ASSET_UNLOADED_INDEX;
		rom.karts[kartIndex].upVec = V3_Up;
	}
	
	// rom.karts[0].design = KartDesign_UnitBox;
	// rom.karts[0].design = KartDesign_ProtoKart1;
	rom.karts[0].design = KartDesign_ProtoKart2;
	rom.karts[0].pos = MakeV3(-1.5f, 1.7f, 11.0f);
	rom.karts[0].rotation = 0;
	rom.karts[0].upVec = V3_Up;
	rom.origKartPos[0] = rom.karts[0].pos;
	
	rom.numKartAssets = 0;
	
	rom.unitBoxModel = model64_load(UNIT_BOX_MODEL_PATH);
	rom.planetModel = model64_load(PLANET_MODEL_PATH);
	
	rom.planetCollision = LoadCollisionSceneFromModel(StrLit(PLANET_MODEL_PATH));
	// rom.planetCollision = LoadCollisionSceneFromModel(StrLit(CAR_MODEL_PATH));
	debugf("collision has %lu faces\n", rom.planetCollision.numFaces);
	debugf("collision bounds=(%g,%g,%g, %g,%g,%g)\n",
		rom.planetCollision.bounds.x, rom.planetCollision.bounds.y, rom.planetCollision.bounds.z,
		rom.planetCollision.bounds.width, rom.planetCollision.bounds.height, rom.planetCollision.bounds.depth
	);
	
	rom.drawCollisionFace = (true && DEBUG_BUILD);
	
	rom.shutdown = false;
	rom.initialized = true;
	debugf("Initialized!\n");
}

// +--------------------------------------------------------------+
// |                       Main ROM Update                        |
// +--------------------------------------------------------------+
void UpdateRom()
{
	if (SC64_PollAUX()) { rom.halt = true; return; } //return if HALT has been received
	joypad_poll();
	
	rom.prevJoy[0] = rom.joy[0];
	rom.prevJoy[1] = rom.joy[1];
	rom.prevJoy[2] = rom.joy[2];
	rom.prevJoy[3] = rom.joy[3];
	rom.joy[0] = joypad_get_inputs(JOYPAD_PORT_1);
	rom.joy[1] = joypad_get_inputs(JOYPAD_PORT_2);
	rom.joy[2] = joypad_get_inputs(JOYPAD_PORT_3);
	rom.joy[3] = joypad_get_inputs(JOYPAD_PORT_4);
	
	rom.prevRomTime = rom.romTime;
	rom.romTime = get_ticks_ms();
	rom.elapsedMs = (rom.romTime >= rom.prevRomTime) ? (u32)(rom.romTime - rom.prevRomTime) : 0;
	rom.timeScale = ((float)rom.elapsedMs / 33.0f);
	rom.frameTimes[rom.frameTimeWriteIndex] = rom.elapsedMs;
	rom.frameTimeWriteIndex = (rom.frameTimeWriteIndex+1) % ArrayCount(rom.frameTimes);
	
	if (rom.joy[0].btn.a       && !rom.prevJoy[0].btn.a)       { debugf("A Button was Pressed!\n");       }
	if (rom.joy[0].btn.b       && !rom.prevJoy[0].btn.b)       { debugf("B Button was Pressed!\n");       }
	if (rom.joy[0].btn.c_left  && !rom.prevJoy[0].btn.c_left)  { debugf("C-Left Button was Pressed!\n");  }
	if (rom.joy[0].btn.c_right && !rom.prevJoy[0].btn.c_right) { debugf("C-Right Button was Pressed!\n"); }
	if (rom.joy[0].btn.c_up    && !rom.prevJoy[0].btn.c_up)    { debugf("C-Up Button was Pressed!\n");    }
	if (rom.joy[0].btn.c_down  && !rom.prevJoy[0].btn.c_down)  { debugf("C-Down Button was Pressed!\n");  }
	if (rom.joy[0].btn.z       && !rom.prevJoy[0].btn.z)       { debugf("Z Button was Pressed!\n");       }
	if (rom.joy[0].btn.start   && !rom.prevJoy[0].btn.start)   { debugf("Start Button was Pressed!\n");   }
	
	// +==============================+
	// |  Press L+R+Start to Reboot   |
	// +==============================+
	if (rom.joy[0].btn.l && rom.joy[0].btn.r && rom.joy[0].btn.start) { debugf("Rebooting!\n"); SoftRebootN64(); }
	
	// +====================================+
	// | Press C-Left/Right to Change Kart  |
	// +====================================+
	if (rom.joy[0].btn.c_left && !rom.prevJoy[0].btn.c_left)
	{
		for (u32 kartIndex = 0; kartIndex < MAX_PLAYERS; kartIndex++)
		{
			KartState* kart = &rom.karts[kartIndex];
			if (kart->design != KartDesign_None)
			{
				kart->design = (KartDesign)((u32)kart->design - 1);
				if (kart->design == KartDesign_None) { kart->design = (KartDesign_Count-1); }
				kart->modelAssetIndex = ASSET_UNLOADED_INDEX;
			}
		}
	}
	if (rom.joy[0].btn.c_right && !rom.prevJoy[0].btn.c_right)
	{
		for (u32 kartIndex = 0; kartIndex < MAX_PLAYERS; kartIndex++)
		{
			KartState* kart = &rom.karts[kartIndex];
			if (kart->design != KartDesign_None)
			{
				kart->design = (KartDesign)(((u32)kart->design + 1) % KartDesign_Count);
				if (kart->design == KartDesign_None) { kart->design = (KartDesign)((u32)kart->design+1); }
				kart->modelAssetIndex = ASSET_UNLOADED_INDEX;
			}
		}
	}
	
	// +--------------------------------------------------------------+
	// |                         Update Karts                         |
	// +--------------------------------------------------------------+
	for (u32 kartIndex = 0; kartIndex < MAX_PLAYERS; kartIndex++)
	{
		KartState* kart = &rom.karts[kartIndex];
		if (kart->design != KartDesign_None)
		{
			r32 kartSpeed = GetKartDesignSpeed(kart->design);
			
			// +================================+
			// | Reset Kart Position with Start |
			// +================================+
			if (rom.joy[kartIndex].btn.start && !rom.prevJoy[kartIndex].btn.start)
			{
				debugf("Resetting kart[%lu]\n", kartIndex);
				kart->pos = rom.origKartPos[kartIndex];
			}
			
			// +====================================+
			// | Debug Move Kart with DPAD and L/R  |
			// +====================================+
			if (rom.joy[kartIndex].btn.d_right) { kart->pos.x += PLANET_DBG_MOVE_HORI_SPEED * kartSpeed; }
			if (rom.joy[kartIndex].btn.d_down)  { kart->pos.z += PLANET_DBG_MOVE_HORI_SPEED * kartSpeed; }
			if (rom.joy[kartIndex].btn.d_left)  { kart->pos.x -= PLANET_DBG_MOVE_HORI_SPEED * kartSpeed; }
			if (rom.joy[kartIndex].btn.d_up)    { kart->pos.z -= PLANET_DBG_MOVE_HORI_SPEED * kartSpeed; }
			if (rom.joy[kartIndex].btn.r)       { kart->pos.y += PLANET_DBG_MOVE_VERT_SPEED * kartSpeed; }
			if (rom.joy[kartIndex].btn.z)       { kart->pos.y -= PLANET_DBG_MOVE_VERT_SPEED * kartSpeed; }
			
			// +==============================+
			// |   Move Kart with Joystick    |
			// +==============================+
			v2 stickVec = MakeV2((r32)rom.joy[kartIndex].stick_x / 127.0f, (r32)rom.joy[kartIndex].stick_y / 127.0f);
			r32 stickLengthSquared = LengthSquaredV2(stickVec);
			bool stickNotInDeadzone = (stickLengthSquared > STICK_DEADZONE * STICK_DEADZONE);
			if (stickNotInDeadzone)
			{
				r32 stickLength = SqrtR32(stickLengthSquared);
				r32 stickAngle = AngleFixR32(-AtanR32(stickVec.y, stickVec.x) + HalfPi32 + rom.cameraAngle);
				stickVec = MakeV2(CosR32(stickAngle), SinR32(stickAngle));
				kart->pos.x -= stickVec.x * PLANET_ANALOG_MOVE_SPEED * stickLength * kartSpeed;
				kart->pos.z -= stickVec.y * PLANET_ANALOG_MOVE_SPEED * stickLength * kartSpeed;
				kart->rotation = stickAngle;
			}
			
			// +==================================+
			// | Do Collision With CollisionScene |
			// +==================================+
			kart->drivingFace = FindCurrentCollisionFace(&rom.planetCollision, kart->pos, COLL_GROUND_THICKNESS, &kart->altitude);
			if (kartIndex == 0) { rom.closestFace = kart->drivingFace; }
			if (kart->drivingFace != nullptr)
			{
				kart->upVec = kart->drivingFace->normal;
				if (kart->altitude < 0)
				{
					kart->pos = AddV3(kart->pos, ScaleV3(kart->drivingFace->normal, -kart->altitude + EPSILON));
					kart->altitude = EPSILON;
				}
			}
			else { kart->altitude = INFINITY; }
		}
	}
	
	// +====================================+
	// | Toggle Collision Rendering with B  |
	// +====================================+
	if (rom.joy[0].btn.b && !rom.prevJoy[0].btn.b)
	{
		rom.drawCollisionFace = !rom.drawCollisionFace;
		debugf("Collision face rendering %s!\n", rom.drawCollisionFace ? "Enabled" : "Disabled");
	}
	
	// +==============================+
	// |        Update Camera         |
	// +==============================+
	for (u32 kartIndex = 0; kartIndex < MAX_KARTS; kartIndex++)
	{
		KartState* kart = &rom.karts[kartIndex];
		if (kart->design != KartDesign_None)
		{
			r32 cameraKartAngleDiff = AngleDiffR32(kart->rotation, rom.cameraAngle);
			if (AbsR32(cameraKartAngleDiff) > 0.01f)
			{
				rom.cameraAngle = AngleFixR32(rom.cameraAngle + cameraKartAngleDiff/8.0f);
			}
			else { rom.cameraAngle = kart->rotation; }
			rom.cameraPos = AddV3(kart->pos, MakeV3(CosR32(rom.cameraAngle) * CAMERA_DIST_X, CAMERA_DIST_Y, SinR32(rom.cameraAngle) * CAMERA_DIST_X));
			rom.cameraTarget = kart->pos;
			rom.cameraTarget.y += CAMERA_TARGET_ABOVE_CAR_Y;
			break;
		}
	}
	if (AreEqualV3(rom.cameraTarget, rom.cameraPos)) { rom.cameraTarget = AddV3(rom.cameraPos, V3_Forward); } //prevent division by 0 for NormalizeV3 below
	rom.cameraForward = NormalizeV3(SubV3(rom.cameraTarget, rom.cameraPos));
	rom.cameraRight = CrossV3(rom.cameraForward, V3_Up);
	rom.cameraUp = CrossV3(rom.cameraRight, rom.cameraForward);
	rom.cameraViewMat = MakeLookAtMat4_RH(rom.cameraPos, rom.cameraTarget, rom.cameraUp);
	
	rom.frameIndex++;
}

// +--------------------------------------------------------------+
// |                       Main ROM Render                        |
// +--------------------------------------------------------------+
void RenderRom()
{
	if (rom.shutdown || rom.halt) { return; }
	
	// Acquire a free framebuffer for rendering
	surface_t* disp = display_get();
	surface_t* zbuf = display_get_zbuf();
	// Attach the framebuffer for use by rdpq
	rdpq_attach(disp, zbuf);
	// Clear the framebuffer with black
	rdpq_clear((color_t){59, 58, 50, 0}); // 0x3B3A32
	
	// +==============================+
	// |      Render Background       |
	// +==============================+
	Test_RenderGradientWithBoxes();
	
	// +==============================+
	// |       Render 3D Scene        |
	// +==============================+
	gl_context_begin();
	{
		// glClearColor(0.243f, 0.25f, 0.33f, 1.0f); // BG color
		glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);
		
		static const GLfloat key_pos[] = {2.5f, 2.0f, 2.5f, 1.0f};
		static const GLfloat fill_pos[] = {-2.5f, 1.0f, 2.0f, 1.0f};
		static const GLfloat rim_pos[] = {0.0f, 2.5f, -2.5f, 1.0f};
		glLightfv(GL_LIGHT0, GL_POSITION, key_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, fill_pos);
		glLightfv(GL_LIGHT2, GL_POSITION, rim_pos);
		
		// +==============================+
		// |         Draw Planet          |
		// +==============================+
		DrawModel(rom.planetModel, V3_Zero, V3_One, Quat_Identity);
		
		// +==============================+
		// |          Draw Karts          |
		// +==============================+
		for (u32 kartIndex = 0; kartIndex < MAX_KARTS; kartIndex++)
		{
			KartState* kart = &rom.karts[kartIndex];
			if (kart->design != KartDesign_None)
			{
				r32 assetScale = GetKartDesignAssetScale(kart->design);
				
				if (kart->modelAssetIndex == ASSET_UNLOADED_INDEX)
				{
					const char* assetPathNt = GetKartDesignAssetPath(kart->design);
					Str8 assetPath = MakeStr8Nt(assetPathNt);
					
					bool alreadyLoaded = false;
					for (u32 assetIndex = 0; assetIndex < ArrayCount(rom.kartAssets); assetIndex++)
					{
						if (rom.kartAssets[assetIndex] != nullptr && StrExactEquals(rom.kartAssetPaths[assetIndex], assetPath))
						{
							alreadyLoaded = true;
							kart->modelAssetIndex = assetIndex;
							break;
						}
					}
					
					if (!alreadyLoaded)
					{
						if (rom.numKartAssets < MAX_KARTS)
						{
							debugf("Loading asset for KartDesign_%s: \"%s\"...\n", GetKartDesignName(kart->design), assetPathNt);
							model64_t* model = model64_load(assetPathNt);
							if (model != nullptr)
							{
								debugf("Loaded into [%lu]\n", rom.numKartAssets);
								rom.kartAssets[rom.numKartAssets] = model;
								rom.kartAssetPaths[rom.numKartAssets] = assetPath;
								kart->modelAssetIndex = rom.numKartAssets;
								rom.numKartAssets++;
							}
							else
							{
								debugf("Failed to load model64 for KartDesign_%s from \"%s\"\n", GetKartDesignName(kart->design), assetPathNt);
								kart->modelAssetIndex = ASSET_FAILED_INDEX;
							}
						}
						else
						{
							debugf("Can't load model64 for KartDesign_%s because we've filled the asset array!\n", GetKartDesignName(kart->design));
							kart->modelAssetIndex = ASSET_FAILED_INDEX;
						}
					}
				}
				
				r32 assetRotation = ToRadians32(GetKartDesignAssetRotation(kart->design));
				v3 kartRightVec = CrossV3(kart->upVec, MakeV3(CosR32(kart->rotation + assetRotation), 0.0f, SinR32(kart->rotation + assetRotation)));
				v3 kartForwardVec = CrossV3(kart->upVec, kartRightVec);
				
				if (kart->modelAssetIndex < ArrayCount(rom.kartAssets) && rom.kartAssets[kart->modelAssetIndex] != nullptr)
				{
					mat3 kartRotationMat = MakeMat3_Const(
						kartForwardVec.x, kart->upVec.x, kartRightVec.x,
						kartForwardVec.y, kart->upVec.y, kartRightVec.y,
						kartForwardVec.z, kart->upVec.z, kartRightVec.z
					);
					
					DrawModel(
						rom.kartAssets[kart->modelAssetIndex],
						AddV3(kart->pos, ScaleV3(kart->upVec, GetKartDesignClearance(kart->design))),
						FillV3(assetScale),
						QuatFromMat3(kartRotationMat)
					);
				}
				
				#if DEBUG_BUILD
				if (rom.drawCollisionFace)
				{
					// DrawBoxAt(AddV3(kart->pos, ScaleV3(kartRightVec, 1.5f)), 0.2f);
					DrawBoxAt(AddV3(kart->pos, ScaleV3(kartForwardVec, 1.5f)), 0.2f);
					DrawBoxAt(AddV3(kart->pos, ScaleV3(kart->upVec, 1.5f)), 0.2f);
				}
				#endif
			}
		}
		
		// +==============================+
		// |   Debug Draw Closest Face    |
		// +==============================+
		if (rom.closestFace != nullptr && rom.drawCollisionFace)
		{
			v3 vert0 = rom.closestFace->verts[0];
			v3 vert1 = rom.closestFace->verts[1];
			v3 vert2 = rom.closestFace->verts[2];
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
	
	// +==============================+
	// |          Render HUD          |
	// +==============================+
	int textY = 15;
	// rdpq_text_print(NULL, DEBUG_FONT_ID, 15, 15, rom.rtcAvailable      ? "RTC: Available"           : "RTC: NOT AVAILABLE"          );
	// rdpq_text_print(NULL, DEBUG_FONT_ID, 15, 25, rom.usbDebugAvailable ? "USB Debugging: Available" : "USB Debugging: NOT AVAILABLE");
	r32 avgFrameTime = (r32)(rom.frameTimes[0] + rom.frameTimes[1] + rom.frameTimes[2] + rom.frameTimes[3] + rom.frameTimes[4]) / 5.0f;
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 15, textY, "FrameTime: %.1fms (%.1fFPS)", avgFrameTime, 1000.0f / avgFrameTime); textY += 15;
	rdpq_text_printf(NULL, DEBUG_FONT_ID, 15, textY, "romTime: %llu,%llums", (rom.romTime/1000), (rom.romTime%1000)); textY += 15;
	for (u32 kartIndex = 0; kartIndex < MAX_KARTS; kartIndex++)
	{
		KartState* kart = &rom.karts[kartIndex];
		if (kart->design != KartDesign_None)
		{
			rdpq_text_printf(NULL, DEBUG_FONT_ID, 15, textY, "Kart[%lu]: (%g, %g, %g) %.0f %s%.2f %s surface",
				kartIndex,
				kart->pos.x, kart->pos.y, kart->pos.z,
				ToDegrees32(kart->rotation),
				(kart->altitude >= 0.0f) ? "+" : "",
				kart->altitude,
				(kart->altitude >= 0.0f) ? "above" : "below"
			);
			textY += 15;
		}
	}
	
	if (rom.closestFace != nullptr && rom.drawCollisionFace)
	{
		rdpq_text_printf(NULL, DEBUG_FONT_ID, 15, textY, "closest: face[%lu]", (u32)(rom.closestFace - rom.planetCollision.faces)); textY += 15;
	}
	
	// Test_RenderDfsEntries();
	// Test_RenderTypeSizes();
	
	// Detach the framebuffer and show it on screen when it's ready
	// (when previous rendering operations have completed)
	rdpq_detach_show();
}
