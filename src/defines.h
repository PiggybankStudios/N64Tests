/*
File:   defines.h
Author: Taylor Robbins
Date:   09\01\2026
Description:
	** Holds all our ROM-wide defines
*/

#ifndef _DEFINES_H
#define _DEFINES_H

#define DEBUG_FONT_ID   10

#define PLANET_DBG_MOVE_HORI_SPEED 0.7f
#define PLANET_DBG_MOVE_VERT_SPEED 0.1f
#define PLANET_ANALOG_MOVE_SPEED 0.4f

#define STICK_DEADZONE 0.1f
#define CAR_OFFSET MakeV3(0.0f, -1.5f, -6.0f)
#define COLL_GROUND_THICKNESS 10.0f
#define COLL_EDGE_DIST 0.5f

#define MAX_PLAYERS  4 //same as JOYPAD_PORT_COUNT
#define MAX_KARTS    4

#define MODELS_FOLDER "rom:/models"

#define ERROR_MODEL_PATH    MODELS_FOLDER "/error.model64"

#define UNIT_BOX_MODEL_PATH MODELS_FOLDER "/unit_box.model64"

// #define CAR_MODEL_PATH      MODELS_FOLDER "/proto_kart2.model64"
#define CAR_MODEL_PATH      MODELS_FOLDER "/ash_kart.model64"

// #define PLANET_MODEL_PATH   MODELS_FOLDER "/proto_planet1.model64"
// #define PLANET_MODEL_PATH   MODELS_FOLDER "/proto_planet2.model64"
// #define PLANET_MODEL_PATH   MODELS_FOLDER "/planet_beach_test.model64"
#define PLANET_MODEL_PATH   MODELS_FOLDER "/planet_saturn1.model64"

#define ASSET_UNLOADED_INDEX 0xFE
#define ASSET_FAILED_INDEX   0xFF

#endif //  _DEFINES_H
