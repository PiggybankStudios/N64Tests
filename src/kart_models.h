/*
File:   kart_models.h
Author: Taylor Robbins
Date:   09\14\2026
Description:
	** This file defines all attributes of each kart that the player can
	** select in the game. Each model has various attributes that affect how it
	** behaves and renders in the world. This file contains one entry for each kart
	** and each entry contains all attributes that need to be specified for that cart.
	** In kart.h and kart.c we use this file to generate lookup tables for each attribute
	** so we can get attributes of a certain kinds for all karts put into one LUT
	** but when authoring we can manage attributes for each kart separately.
	** 
	** This file is meant to be included multiple times in an "X-Macros" like fashion
	** KART_MODEL should be defined with a code-generating template before each #include
*/

// codeNameLowercase = Name used in code with the first letter lowercase
// CodeNameUppercase = Name used in code with the first letter uppercase (the enum name will be KartModel_CodeNameUppercase)
// displayName       = Name used for display to the player when selecting this kart
// assetPath         = Path to the model64 asset in the DFS
// assetScale        = fixed scale to apply when rendering the model asset in-world
// clearance         = distance from ground to model origin
// width             = distance from model origin to left/right wheels
// foreLength        = distance from model origin to front tires
// backLength        = distance from model origin to back tires
// speed             = scalar for top speed
// acceleration      = scalar for acceleration per frame
// turnRadius        = scalar for tightest turn radius

#ifndef KART_MODEL
#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) //nothing
#endif

//                                    lowercase,  uppercase,  displayName,   assetPath,                            scale, rotation,  clear,  width,   fore,   back, speed, accel, turn
/* KartModel_UnitBox    */ KART_MODEL(unitBox,    UnitBox,    "Box",        MODELS_FOLDER "/unit_box.model64",    0.750f,     0.0f, 0.375f, 0.375f, 0.375f, 0.375f, 2.0f,  2.0f, 0.5f)
/* KartModel_ProtoKart1 */ KART_MODEL(protoKart1, ProtoKart1, "Prototype1", MODELS_FOLDER "/proto_kart2.model64", 1.000f,   180.0f, 0.200f, 0.400f, 0.400f, 0.400f, 1.0f,  1.0f, 1.2f)
/* KartModel_ProtoKart2 */ KART_MODEL(protoKart2, ProtoKart2, "Prototype2", MODELS_FOLDER "/ash_kart.model64",    1.000f,     0.0f, 0.200f, 0.400f, 0.400f, 0.400f, 1.4f,  0.8f, 1.0f)
/* KartModel_ProtoKart3 */ KART_MODEL(protoKart3, ProtoKart3, "Prototype3", MODELS_FOLDER "/proto_kart3.model64", 1.000f,     0.0f, 0.200f, 0.400f, 0.400f, 0.400f, 1.4f,  0.8f, 1.0f)

#undef KART_MODEL