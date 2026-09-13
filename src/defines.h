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

#define STICK_DEADZONE 0.1f
#define CAR_OFFSET MakeV3(0.0f, -1.5f, -3.0f)

#define MODELS_FOLDER "rom:/models"

#define ERROR_MODEL_PATH    MODELS_FOLDER "/error.model64"

#define UNIT_BOX_MODEL_PATH MODELS_FOLDER "/unit_box.model64"

// #define CAR_MODEL_PATH      MODELS_FOLDER "/proto_kart2.model64"
#define CAR_MODEL_PATH      MODELS_FOLDER "/ash_kart.model64"

// #define PLANET_MODEL_PATH   MODELS_FOLDER "/proto_planet1.model64"
#define PLANET_MODEL_PATH   MODELS_FOLDER "/planet_beach_test.model64"

#endif //  _DEFINES_H
