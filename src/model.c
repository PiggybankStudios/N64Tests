/*
File:   model.c
Author: Taylor Robbins
Date:   09\12\2026
Description:
	** This is our own model parser because the libdragon model64.c has a bunch of
	** internal functionality that we can't access. This is annoying because we need
	** the model data for collision calculations and we want to specify how the model
	** gets rendered in a more granular way (esp. texture filtering settings)
*/

#define MAX_COLLISION_FACES 1024 //TODO: Get rid of the need for this

//Unaligned i16 is required to read the values packed with GL_HALF_FIXED_N64 and an odd stride value
typedef i16 ui16 __attribute__((aligned(1)));

r32 HalfFixedN64ToR32(i16 fixedPoint, int numFractionalBits)
{
	return (r32)fixedPoint / (r32)(1 << numFractionalBits);
}

CollisionScene LoadCollisionSceneFromModel(Str8 path)
{
	AssertNullTerm(path); //TODO: Should we alloc on a scratch arena to ensure this instead?
	CollisionScene result = ZEROED;
	
	int modelSize = 0;
	u8* modelData = (u8*)asset_load(path.chars, &modelSize);
	
	Assert(modelSize >= sizeof(ModelHeader));
	ModelHeader* header = (ModelHeader*)modelData;
	header->nodes        = RELATIVE_PNTR_TO_ABSOLUTE(ModelNode, header, header->nodes);
	header->meshes       = RELATIVE_PNTR_TO_ABSOLUTE(ModelMesh, header, header->meshes);
	header->skins        = RELATIVE_PNTR_TO_ABSOLUTE(ModelSkin, header, header->skins);
	header->anims        = RELATIVE_PNTR_TO_ABSOLUTE(ModelAnim, header, header->anims);
	header->texturePaths = RELATIVE_PNTR_TO_ABSOLUTE(char*,     header, header->texturePaths);
	
	// +==============================+
	// |   Fixup Relative Pointers    |
	// +==============================+
	for (u32 mIndex = 0; mIndex < header->numMeshes; mIndex++)
	{
		ModelMesh* mesh = &header->meshes[mIndex];
		mesh->primitives = RELATIVE_PNTR_TO_ABSOLUTE(ModelPrimitive, header, mesh->primitives);
		for (u32 pIndex = 0; pIndex < mesh->numPrimitives; pIndex++)
		{
			ModelPrimitive* primitive = &mesh->primitives[pIndex];
			primitive->position.pointer = RELATIVE_PNTR_TO_ABSOLUTE(void, header, primitive->position.pointer);
			primitive->color.pointer    = RELATIVE_PNTR_TO_ABSOLUTE(void, header, primitive->color.pointer);
			primitive->texcoord.pointer = RELATIVE_PNTR_TO_ABSOLUTE(void, header, primitive->texcoord.pointer);
			primitive->normal.pointer   = RELATIVE_PNTR_TO_ABSOLUTE(void, header, primitive->normal.pointer);
			primitive->mtxIndex.pointer = RELATIVE_PNTR_TO_ABSOLUTE(void, header, primitive->mtxIndex.pointer);
			primitive->indices          = RELATIVE_PNTR_TO_ABSOLUTE(void, header, primitive->indices);
			primitive->sharedTexture = primitive->localTexture; //NOTE: We don't load textures in this loader, so we just fill in the sharedTexture index to match localTexture
		}
	}
	
	for (u32 nIndex = 0; nIndex < header->numNodes; nIndex++)
	{
		ModelNode* node = &header->nodes[nIndex];
		node->name     = RELATIVE_PNTR_TO_ABSOLUTE(char,      header, node->name);
		node->mesh     = RELATIVE_PNTR_TO_ABSOLUTE(ModelMesh, header, node->mesh);
		node->skin     = RELATIVE_PNTR_TO_ABSOLUTE(ModelSkin, header, node->skin);
		node->children = RELATIVE_PNTR_TO_ABSOLUTE(u32,       header, node->children);
	}
		
	for (u32 tIndex = 0; tIndex < header->numTextures; tIndex++)
	{
		header->texturePaths[tIndex] = RELATIVE_PNTR_TO_ABSOLUTE(char, header, header->texturePaths[tIndex]);
	}
	
	// +============================================+
	// | Loop Over Nodes and Extract CollisionFaces |
	// +============================================+
	//TODO: We should switch away from a fixed MAX_COLLISION_FACES, or we should shrink our allocation after we have discovered the true number of faces
	result.numFaces = 0;
	result.faces = (CollisionFace*)malloc(sizeof(CollisionFace) * MAX_COLLISION_FACES);
	NotNull(result.faces);
	bool boundsSet = false;
	
	for (u32 nIndex = 0; nIndex < header->numNodes; nIndex++)
	{
		ModelNode* node = &header->nodes[nIndex];
		mat4 nodeTransformMat = MakeMat4(
			node->transform.mtx[0], node->transform.mtx[4], node->transform.mtx[8],  node->transform.mtx[12],
			node->transform.mtx[1], node->transform.mtx[5], node->transform.mtx[9],  node->transform.mtx[13],
			node->transform.mtx[2], node->transform.mtx[6], node->transform.mtx[10], node->transform.mtx[14],
			node->transform.mtx[3], node->transform.mtx[7], node->transform.mtx[11], node->transform.mtx[15]
		);
		
		if (node->mesh != nullptr)
		{
			for (u32 pIndex = 0; pIndex < node->mesh->numPrimitives; pIndex++)
			{
				ModelPrimitive* primitive = &node->mesh->primitives[pIndex];
				
				//TODO: Do we need to handle a mesh with no indices?
				
				// TODO: Support mode=5: GL_TRIANGLE_STRIP?
				// TODO: Support mode=6: GL_TRIANGLE_FAN?
				// TODO: Support mode=7: GL_QUADS?
				// TODO: Support mode=8: GL_QUAD_STRIP?
				// TODO: Support mode=9: GL_POLYGON?
				if (primitive->mode == GL_TRIANGLES && (primitive->position.type == GL_FLOAT || primitive->position.type == GL_HALF_FIXED_N64))
				{
					u8 indexSize = sizeof(u8);
					// bool isIndexSigned = false;
					switch (primitive->indexType)
					{
						// case GL_BYTE:           indexSize = sizeof(i8); isIndexSigned=true; break;
						case GL_UNSIGNED_BYTE:  indexSize = sizeof(u8); break;
						// case GL_SHORT:          indexSize = sizeof(i16); isIndexSigned=true; break;
						case GL_UNSIGNED_SHORT: indexSize = sizeof(u16); break;
						// case GL_INT:            indexSize = sizeof(i32); isIndexSigned=true; break;
						case GL_UNSIGNED_INT:   indexSize = sizeof(u32); break;
						// case GL_FLOAT:          indexSize = sizeof(r32); break;
						// case GL_DOUBLE:         indexSize = sizeof(r64); break;
						// case GL_HALF_FIXED_N64: indexSize = ?; break;
						default: debugf("We don't support index type %lu on node[%lu].primitive[%lu] in %s\n", primitive->indexType, nIndex, pIndex, path.chars); break;
					}
					
					// Make a temporary buffer for holding decoded and transformed vertex locations
					v3* vertexPositions = (v3*)malloc(primitive->numVertices * sizeof(v3));
					NotNull(vertexPositions);
					for (u32 vIndex = 0; vIndex < primitive->numVertices; vIndex++)
					{
						vertexPositions[vIndex] = V3_Zero;
						if (primitive->position.type == GL_FLOAT)
						{
							u32 stride = ((primitive->position.stride == 0) ? sizeof(r32) : primitive->position.stride);
							vertexPositions[vIndex] = *(v3*)((u8*)primitive->position.pointer + (stride * vIndex));
						}
						else if (primitive->position.type == GL_HALF_FIXED_N64)
						{
							u32 stride = ((primitive->position.stride == 0) ? sizeof(u16) : primitive->position.stride);
							ui16* vertPntr = (ui16*)((u8*)primitive->position.pointer + (stride * vIndex));
							vertexPositions[vIndex].x = HalfFixedN64ToR32(vertPntr[0], primitive->vertexPrecision);
							vertexPositions[vIndex].y = HalfFixedN64ToR32(vertPntr[1], primitive->vertexPrecision);
							vertexPositions[vIndex].z = HalfFixedN64ToR32(vertPntr[2], primitive->vertexPrecision);
						}
						else { AssertMsg(primitive->position.type == GL_FLOAT || primitive->position.type == GL_HALF_FIXED_N64, "Unsupported position attribute format!"); }
						
						vertexPositions[vIndex] = MulMat4AndV3(nodeTransformMat, vertexPositions[vIndex], /*includeTranslation*/true);
					}
					
					u32 triIndices[3];
					for (u32 iIndex = 0; iIndex < primitive->numIndices; iIndex++)
					{
						u8* indexPntr = &((u8*)primitive->indices)[iIndex*indexSize];
						u32 index = 0;
						switch (primitive->indexType)
						{
							// case GL_BYTE:           index =  (u32)(*(i8*)indexPntr); break;
							case GL_UNSIGNED_BYTE:  index =  (u32)(*(u8*)indexPntr); break;
							// case GL_SHORT:          index = (u32)(*(i16*)indexPntr); break;
							case GL_UNSIGNED_SHORT: index = (u32)(*(u16*)indexPntr); break;
							// case GL_INT:            index = (u32)(*(i32*)indexPntr); break;
							case GL_UNSIGNED_INT:   index = (u32)(*(u32*)indexPntr); break;
							default: /*Unreachable*/ break;
						}
						Assert(index < primitive->numVertices);
						
						triIndices[iIndex%3] = index;
						if (iIndex%3 == 2)
						{
							if (result.numFaces < MAX_COLLISION_FACES)
							{
								CollisionFace* face = &result.faces[result.numFaces];
								result.numFaces++;
								face->verts[0] = vertexPositions[triIndices[0]];
								face->verts[1] = vertexPositions[triIndices[1]];
								face->verts[2] = vertexPositions[triIndices[2]];
								face->normal = NormalizeV3(CrossV3(SubV3(face->verts[1], face->verts[0]), SubV3(face->verts[2], face->verts[0])));
								face->planeDist = DotV3(face->verts[0], face->normal);
								
								if (!boundsSet) { result.bounds = MakeBoxV(face->verts[0], V3_Zero); boundsSet = true; }
								result.bounds = ExpandBoxToV3(result.bounds, face->verts[0]);
								result.bounds = ExpandBoxToV3(result.bounds, face->verts[1]);
								result.bounds = ExpandBoxToV3(result.bounds, face->verts[2]);
							}
							else { debugf("WARNING: Max collision faces hit!\n"); }
						}
					}
					
					free(vertexPositions);
				}
				else { debugf("\tWARNING: We don't support primitive mode %lu with position type %lu on node[%lu].primitive[%lu] in \"%s\"\n", primitive->mode, primitive->position.type, nIndex, pIndex, path.chars); }
			}
		}
	}
	
	free(modelData);
	return result;
} 

