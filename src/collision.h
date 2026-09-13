/*
File:   collision.h
Author: Taylor Robbins
Date:   09\12\2026
*/

#ifndef _COLLISION_H
#define _COLLISION_H

typedef struct CollisionFace CollisionFace;
struct CollisionFace
{
	v3 verts[3];
	v3 normal;
	r32 planeDist;
};

typedef struct CollisionScene CollisionScene;
struct CollisionScene
{
	box bounds;
	u32 numFaces;
	CollisionFace* faces;
};

#endif //  _COLLISION_H
