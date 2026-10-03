/*
File:   kart.c
Author: Taylor Robbins
Date:   09\14\2026
Description: 
	** None
*/

void UpdateKart(KartState* kart, u32 kartIndex)
{
	if (kart->design == KartDesign_None) { return; }
	
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
	// kart->drivingFace = FindCurrentCollisionFace(&rom.planetCollision, kart->pos, COLL_GROUND_THICKNESS, &kart->altitude);
	kart->drivingFace = FindGroundFaceBelow(&rom.planetCollision, kart->pos, &kart->altitude);
	if (kartIndex == 0) { rom.closestFace = kart->drivingFace; }
	if (kart->drivingFace != nullptr)
	{
		kart->upVec = kart->drivingFace->normal;
		if (kart->altitude < 0 && !IsInfiniteOrNanR32(kart->altitude) && kart->altitude > -COLL_GROUND_THICKNESS)
		{
			// kart->pos = AddV3(kart->pos, ScaleV3(kart->drivingFace->normal, -kart->altitude + EPSILON));
			kart->pos.y += -(kart->altitude) + EPSILON;
			kart->altitude = EPSILON;
		}
	}
	else { kart->altitude = INFINITY; }
}