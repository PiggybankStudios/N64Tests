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
	u32 romTime;
	u32 elapsedMs;
	float timeScale;
	u32 frameIndex;
	
	joypad_inputs_t joy[4];
	joypad_inputs_t prevJoy[4];
	
	model64_t* unitBoxModel;
	model64_t* carModel;
	model64_t* planetModel;
	CollisionScene planetCollision;
	
	CollisionFace* closestFace;
	r32 closestFaceDistance;
	
	float carRotation;
	
	v3 origPlanetOffset;
	v3 planetOffset;
	v3 planetOffsetGoto;
};

extern RomState rom;

#endif //  _MAIN_H
