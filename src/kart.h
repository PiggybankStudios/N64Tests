/*
File:   kart.h
Author: Taylor Robbins
Date:   09\14\2026
*/

#ifndef _KART_H
#define _KART_H

typedef enum KartDesign KartDesign;
enum KartDesign
{
	KartDesign_None = 0,
	#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
		KartDesign_##CodeNameUppercase,
	#include "kart_designs.h"
	KartDesign_Count,
};

typedef struct KartState KartState;
struct KartState
{
	KartDesign design;
	u32 modelAssetIndex;
	v3 pos;
	v3 upVec;
	r32 rotation;
	r32 speed; //Assumed to be in the rotation direction, along the current drivingFace (with has normal of upVec)
	r32 sideSpeed; //Assumed to be perpendicular "right" of heading direction
	CollisionFace* drivingFace;
	r32 altitude; //distance above/below drivingFace
};

inline const char* GetKartDesignName(KartDesign design)
{
	const char* values[KartDesign_Count] = {
		"None",
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			#CodeNameUppercase,
		#include "kart_designs.h"
	};
	return (design < KartDesign_Count) ? values[design] : "Unknown";
}
inline const char* GetKartDesignDisplayName(KartDesign design)
{
	const char* values[KartDesign_Count] = {
		"ERROR0",
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			#displayName,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline const char* GetKartDesignAssetPath(KartDesign design)
{
	const char* values[KartDesign_Count] = {
		"",
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			assetPath,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline r32 GetKartDesignAssetScale(KartDesign design)
{
	r32 values[KartDesign_Count] = {
		1.0f,
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			assetScale,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline r32 GetKartDesignAssetRotation(KartDesign design)
{
	r32 values[KartDesign_Count] = {
		1.0f,
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			assetRotation,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline r32 GetKartDesignClearance(KartDesign design)
{
	const r32 values[KartDesign_Count] = {
		0.0f, //KartDesign_None
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			clearance,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline r32 GetKartDesignWidth(KartDesign design)
{
	const r32 values[KartDesign_Count] = {
		0.7f, //KartDesign_None
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			width,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline r32 GetKartDesignForeLength(KartDesign design)
{
	const r32 values[KartDesign_Count] = {
		1.0f, //KartDesign_None
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			foreLength,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline r32 GetKartDesignBackLength(KartDesign design)
{
	const r32 values[KartDesign_Count] = {
		0.8f, //KartDesign_None
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			backLength,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline r32 GetKartDesignSpeed(KartDesign design)
{
	const r32 values[KartDesign_Count] = {
		1.0f, //KartDesign_None
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			speed,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline r32 GetKartDesignAcceleration(KartDesign design)
{
	const r32 values[KartDesign_Count] = {
		1.0f, //KartDesign_None
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			acceleration,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}
inline r32 GetKartDesignTurnRadius(KartDesign design)
{
	const r32 values[KartDesign_Count] = {
		1.0f, //KartDesign_None
		#define KART_DESIGN(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			turnRadius,
		#include "kart_designs.h"
	};
	return values[design % KartDesign_Count];
}

#endif //  _KART_H
