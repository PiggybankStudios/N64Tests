/*
File:   collision.c
Author: Taylor Robbins
Date:   09\12\2026
Description: 
	** Holds the logic that allows us to perform collision calculations with the
	** planet geometry in an efficient manner
*/

CollisionFace* FindClosestFace(CollisionScene* scene, v3 queryPos, r32* distanceOut)
{
	CollisionFace* closestFace = nullptr;
	u32 closestFaceIndex = 0;
	r32 closestFaceDistSquared = 0.0f;
	
	for (u32 fIndex = 0; fIndex < scene->numFaces; fIndex++)
	{
		CollisionFace* face = &scene->faces[fIndex];
		v3 triCenter = ShrinkV3(AddV3(AddV3(face->verts[0], face->verts[1]), face->verts[2]), 3);
		r32 distanceSquared = LengthSquaredV3(SubV3(triCenter, queryPos));
		if (closestFace == nullptr || distanceSquared < closestFaceDistSquared)
		{
			closestFace = face;
			closestFaceIndex = fIndex;
			closestFaceDistSquared = distanceSquared;
		}
	}
	
	SetOptionalOutPntr(distanceOut, SqrtR32(closestFaceDistSquared));
	return closestFace;
}
