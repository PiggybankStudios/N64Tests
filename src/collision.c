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

#define EPSILON 0.001f

#define ToFaceSpace(worldPos, faceBasisVecs) MakeV2_Const(DotV3((worldPos), (faceBasisVecs)[0]), DotV3((worldPos), (faceBasisVecs)[1]));

CollisionFace* FindCurrentCollisionFace(CollisionScene* scene, v3 queryPos, r32 queryThickness, r32* altitudeOut)
{
	SetOptionalOutPntr(altitudeOut, 0.0f);
	CollisionFace* collidingFace = nullptr;
	r32 collidingEdgeDist = 0.0f;
	bool printDebug = (rom.joy[0].btn.c_down && rom.prevJoy[0].btn.c_down);
	#define PRINT_DEBUG(...) if (printDebug) { debugf(__VA_ARGS__); }
	
	for (u32 fIndex = 0; fIndex < scene->numFaces; fIndex++)
	{
		CollisionFace* face = &scene->faces[fIndex];
		if (face->normal.y > 0) //perpendicular faces are "walls", upside-down faces are "roofs", neither are drivable
		{
			r32 carAltitude = DotV3(queryPos, face->normal) - face->planeDist;
			if (carAltitude >= -queryThickness && carAltitude <= -EPSILON)
			{
				v3 faceBasisVecs[2];
				faceBasisVecs[0] = CrossV3(face->normal, V3_Right);
				faceBasisVecs[1] = CrossV3(faceBasisVecs[0], face->normal);
				v2 queryFacePos = ToFaceSpace(queryPos, faceBasisVecs);
				v2 vertFacePos0 = ToFaceSpace(face->verts[0], faceBasisVecs);
				v2 vertFacePos1 = ToFaceSpace(face->verts[1], faceBasisVecs);
				v2 vertFacePos2 = ToFaceSpace(face->verts[2], faceBasisVecs);
				// bool insideTriangle = IsInsideTriangleV2(vertFacePos0, vertFacePos1, vertFacePos2, queryFacePos);
				r32 triangleDistance = DistanceToTriangleEdgeV2(vertFacePos0, vertFacePos1, vertFacePos2, queryFacePos);
				if (triangleDistance == 0.0f)
				{
					collidingFace = face;
					SetOptionalOutPntr(altitudeOut, carAltitude);
					PRINT_DEBUG("Inside and below face[%lu] by %g\n", fIndex, carAltitude);
					break;
				}
				else if (triangleDistance < COLL_EDGE_DIST && (collidingFace == nullptr || triangleDistance < collidingEdgeDist))
				{
					collidingFace = face;
					collidingEdgeDist = triangleDistance;
					SetOptionalOutPntr(altitudeOut, carAltitude);
					PRINT_DEBUG("Near by %g and below face[%lu] by %g\n", collidingEdgeDist, fIndex, carAltitude);
				}
				else if (triangleDistance < 1.0f) { PRINT_DEBUG("Outside face[%lu] by %g\n", fIndex, triangleDistance); }
			}
			else if (carAltitude < 1.0f) { PRINT_DEBUG("Above face[%lu] by %g\n", fIndex, carAltitude); }
		}
		else if (printDebug) { PRINT_DEBUG("Skipping wall or ceiling face[%lu]\n", fIndex); }
	}
	#undef PRINT_DEBUG
	return collidingFace;
}
