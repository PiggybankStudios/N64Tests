/*
File:   rom.h
Author: Taylor Robbins
Date:   09\01\2026
Description:
	** This file holds the primary RomState structure which holds all of our
	** global state that lasts from power-on to power-off
*/

#ifndef _MAIN_H
#define _MAIN_H

typedef struct RomState RomState;
struct RomState
{
	bool initialized;
	bool halt;
	bool shutdown;
	
	bool rtcAvailable;
	bool debugOutputAvailable;
	
	u32 prevRomTime;
	u64 romTime;
	u32 elapsedMs;
	float timeScale;
	u32 frameIndex;
	u32 frameTimeWriteIndex;
	r32 frameTimes[5];
	
	joypad_inputs_t joy[4];
	joypad_inputs_t prevJoy[4];
	
	model64_t* unitBoxModel;
	model64_t* carModel;
	model64_t* planetModel;
	CollisionScene planetCollision;
	
	bool drawCollisionFace;
	CollisionFace* closestFace;
	r32 closestFaceDistance;
	r32 carAltitude;
	
	float carRotation;
	
	r32 cameraAngle;
	v3 cameraPos;
	v3 cameraTarget;
	v3 cameraForward; v3 cameraUp; v3 cameraRight;
	mat4 cameraViewMat;
	
	v3 origCarPos;
	v3 carPos;
	v3 carUpVec;
};

extern RomState rom;

#endif //  _MAIN_H
