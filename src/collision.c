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
			if (carAltitude >= -queryThickness && carAltitude <= 0.1f)
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

CollisionFace* FindGroundFaceBelow(CollisionScene* scene, v3 queryPos, r32* altitudeOut)
{
	CollisionFace* resultFace = nullptr;
	r32 resultEdgeDistance = 0.0f;
	r32 resultAltitude = 0.0f;
	
	for (u32 fIndex = 0; fIndex < scene->numFaces; fIndex++)
	{
		CollisionFace* face = &scene->faces[fIndex];
		if (face->normal.y > 0) //perpendicular faces are "walls", upside-down faces are "roofs", neither are considered "ground"
		{
			v2 topDownTriVerts[3] = {
				MakeV2_Const(face->verts[0].x, face->verts[0].z),
				MakeV2_Const(face->verts[1].x, face->verts[1].z),
				MakeV2_Const(face->verts[2].x, face->verts[2].z),
			};
			v2 topDownQueryPos = MakeV2_Const(queryPos.x, queryPos.z);
			r32 topDownDistance = DistanceToTriangleEdgeV2(topDownTriVerts[0], topDownTriVerts[1], topDownTriVerts[2], topDownQueryPos);
			if (topDownDistance < EPSILON)
			{
				if (resultFace == nullptr ||
					topDownDistance < resultEdgeDistance ||
					(topDownDistance < EPSILON && resultEdgeDistance < EPSILON))
				{
					// We need to find the Y coordinate of the point directly below the queryPos that lives on the surface of the triangle
					// Any non-wall triangle has a "height function" like: y = ax + bz + c
					// Here we name them: a=`slopeX`, b=`slopeZ`, c=`heightAtOrigin`
					r32 slopeX = -(face->normal.x) / face->normal.y;
					r32 slopeZ = -(face->normal.z) / face->normal.y;
					r32 heightAtOrigin = face->verts[0].y - (slopeX * face->verts[0].x) - (slopeZ * face->verts[0].z);
					
					r32 triFaceY = (slopeX * queryPos.x) + (slopeZ * queryPos.z) + heightAtOrigin;
					r32 altitude = queryPos.y - triFaceY;
					if (resultFace == nullptr || AbsR32(altitude) < AbsR32(resultAltitude))
					{
						resultFace = face;
						resultEdgeDistance = topDownDistance;
						resultAltitude = altitude;
					}
				}
			}
		}
	}
	
	SetOptionalOutPntr(altitudeOut, resultAltitude);
	return resultFace;
}
