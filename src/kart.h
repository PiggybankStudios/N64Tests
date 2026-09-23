/*
File:   kart.h
Author: Taylor Robbins
Date:   09\14\2026
*/

#ifndef _KART_H
#define _KART_H

typedef enum KartModel KartModel;
enum KartModel
{
	KartModel_None = 0,
	#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
		KartModel_##CodeNameUppercase,
	#include "kart_models.h"
	KartModel_Count,
};

typedef struct KartState KartState;
struct KartState
{
	KartModel model;
	u32 modelAssetIndex;
	v3 pos;
	v3 upVec;
	r32 rotation;
	r32 speed; //Assumed to be in the rotation direction, along the current drivingFace (with has normal of upVec)
	r32 sideSpeed; //Assumed to be perpendicular "right" of heading direction
	CollisionFace* drivingFace;
	r32 altitude; //distance above/below drivingFace
};

inline const char* GetKartModelName(KartModel model)
{
	const char* values[KartModel_Count] = {
		"None",
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			#CodeNameUppercase,
		#include "kart_models.h"
	};
	return (model < KartModel_Count) ? values[model] : "Unknown";
}
inline const char* GetKartModelDisplayName(KartModel model)
{
	const char* values[KartModel_Count] = {
		"ERROR0",
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			#displayName,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline const char* GetKartModelAssetPath(KartModel model)
{
	const char* values[KartModel_Count] = {
		"",
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			assetPath,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline r32 GetKartModelAssetScale(KartModel model)
{
	r32 values[KartModel_Count] = {
		1.0f,
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			assetScale,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline r32 GetKartModelAssetRotation(KartModel model)
{
	r32 values[KartModel_Count] = {
		1.0f,
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			assetRotation,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline r32 GetKartModelClearance(KartModel model)
{
	const r32 values[KartModel_Count] = {
		0.0f, //KartModel_None
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			clearance,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline r32 GetKartModelWidth(KartModel model)
{
	const r32 values[KartModel_Count] = {
		0.7f, //KartModel_None
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			width,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline r32 GetKartModelForeLength(KartModel model)
{
	const r32 values[KartModel_Count] = {
		1.0f, //KartModel_None
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			foreLength,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline r32 GetKartModelBackLength(KartModel model)
{
	const r32 values[KartModel_Count] = {
		0.8f, //KartModel_None
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			backLength,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline r32 GetKartModelSpeed(KartModel model)
{
	const r32 values[KartModel_Count] = {
		1.0f, //KartModel_None
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			speed,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline r32 GetKartModelAcceleration(KartModel model)
{
	const r32 values[KartModel_Count] = {
		1.0f, //KartModel_None
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			acceleration,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}
inline r32 GetKartModelTurnRadius(KartModel model)
{
	const r32 values[KartModel_Count] = {
		1.0f, //KartModel_None
		#define KART_MODEL(codeNameLowercase, CodeNameUppercase, displayName, assetPath, assetScale, assetRotation, clearance, width, foreLength, backLength, speed, acceleration, turnRadius) \
			turnRadius,
		#include "kart_models.h"
	};
	return values[model % KartModel_Count];
}

#endif //  _KART_H
