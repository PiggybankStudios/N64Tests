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
	bool shutdown;
	
	bool rtcAvailable;
	bool debugOutputAvailable;
	
	uint32_t prevRomTime;
	uint32_t romTime;
	uint32_t elapsedMs;
	float timeScale;
	
	joypad_buttons_t prevPadStates[4];
	
	model64_t* carModel;
	model64_t* planetModel;
	float carRotation;
};

extern RomState rom;

#endif //  _MAIN_H
