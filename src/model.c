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

#define RELATIVE_PNTR_TO_ABSOLUTE(pntrType, basePntr, relativePntr) (pntrType*)(((relativePntr) == 0) ? 0 : ((u8*)(basePntr) + (u32)(relativePntr)))

#define DO_MODEL_LOAD_DEBUG_OUTPUT 1

#define MAX_COLLISION_FACES 128

//Unaligned i16 is required to read the values packed with GL_HALF_FIXED_N64 and an odd stride value
typedef i16 ui16 __attribute__((aligned(1)));

r32 HalfFixedN64ToR32(i16 fixedPoint, int numFractionalBits)
{
	return (r32)fixedPoint / (r32)(1 << numFractionalBits);
}

PlanetCollision LoadPlanetCollision(Str8 path)
{
	AssertNullTerm(path); //TODO: Should we alloc on a scratch arena to ensure this instead?
	PlanetCollision result = ZEROED;
	
	int modelSize = 0;
	u8* modelData = (u8*)asset_load(path.chars, &modelSize);
	#if DO_MODEL_LOAD_DEBUG_OUTPUT
	debugf("Loaded %d model at %p\n", modelSize, modelData); //Loaded 7012 model at 0x800e6be0 - Loaded 37328 model at 0x800e6460
	#endif
	
	Assert(modelSize >= sizeof(ModelHeader));
	ModelHeader* header = (ModelHeader*)modelData;
	header->nodes        = RELATIVE_PNTR_TO_ABSOLUTE(ModelNode, header, header->nodes);
	header->meshes       = RELATIVE_PNTR_TO_ABSOLUTE(ModelMesh, header, header->meshes);
	header->skins        = RELATIVE_PNTR_TO_ABSOLUTE(ModelSkin, header, header->skins);
	header->anims        = RELATIVE_PNTR_TO_ABSOLUTE(ModelAnim, header, header->anims);
	header->texturePaths = RELATIVE_PNTR_TO_ABSOLUTE(char*,     header, header->texturePaths);
	
	#if DO_MODEL_LOAD_DEBUG_OUTPUT
	debugf("model[magic]          = 0x%08lX\n", header->magic);          //model[magic]          = 0x4D444C48 - 0x4D444C48 = "MDLH"
	debugf("model[refCount]       = 0x%08lX\n", header->refCount);       //model[refCount]       = 0x00000000 - 0x00000000
	debugf("model[version]        = 0x%08lX\n", header->version);        //model[version]        = 0x00000002 - 0x00000002
	debugf("model[headerSize]     = 0x%08lX\n", header->headerSize);     //model[headerSize]     = 0x00000058 - 0x00000058
	debugf("model[meshSize]       = 0x%08lX\n", header->meshSize);       //model[meshSize]       = 0x00000008 - 0x00000008
	debugf("model[primitiveSize]  = 0x%08lX\n", header->primitiveSize);  //model[primitiveSize]  = 0x00000074 - 0x00000074
	debugf("model[nodeSize]       = 0x%08lX\n", header->nodeSize);       //model[nodeSize]       = 0x00000080 - 0x00000080
	debugf("model[skinSize]       = 0x%08lX\n", header->skinSize);       //model[skinSize]       = 0x00000008 - 0x00000008
	debugf("model[animSize]       = 0x%08lX\n", header->animSize);       //model[animSize]       = 0x00000028 - 0x00000028
	debugf("model[numNodes]       = 0x%08lX\n", header->numNodes);       //model[numNodes]       = 0x00000001 - 0x0000000C
	debugf("model[nodes]          = %p\n",      header->nodes);          //model[nodes]          = 0x800e8690 - 0x8b10
	debugf("model[rootNode]       = 0x%08lX\n", header->rootNode);       //model[rootNode]       = 0x00000000 - 0x0000000B
	debugf("model[numSkins]       = 0x%08lX\n", header->numSkins);       //model[numSkins]       = 0x00000000 - 0x00000000
	debugf("model[skins]          = %p\n",      header->skins);          //model[skins]          = 0x800e8718 - 0x91ac
	debugf("model[numMeshes]      = 0x%08lX\n", header->numMeshes);      //model[numMeshes]      = 0x00000001 - 0x0000000A
	debugf("model[meshes]         = %p\n",      header->meshes);         //model[meshes]         = 0x800e6c38 - 0x58
	debugf("model[numAnims]       = 0x%08lX\n", header->numAnims);       //model[numAnims]       = 0x00000000 - 0x00000000
	debugf("model[anims]          = %p\n",      header->anims);          //model[anims]          = 0x800e8718 - 0x91ac
	debugf("model[maxTracks]      = 0x%08lX\n", header->maxTracks);      //model[maxTracks]      = 0x00000000 - 0x00000000
	debugf("model[animDataHandle] = %p\n",      header->animDataHandle); //model[animDataHandle] = 0x0        - 0x0
	debugf("model[numTextures]    = 0x%08lX\n", header->numTextures);    //model[numTextures]    = 0x00000002 - 0x00000002
	debugf("model[texturePaths]   = %p\n",      header->texturePaths);   //model[texturePaths]   = 0x800e8718 - 0x91ac
	#endif
	
	result.numFaces = 0;
	result.faces = (CollisionFace*)malloc(sizeof(CollisionFace) * MAX_COLLISION_FACES);
	NotNull(result.faces);
	bool boundsSet = false;
	
	for (u32 mIndex = 0; mIndex < header->numMeshes; mIndex++)
	{
		ModelMesh* mesh = &header->meshes[mIndex];
		mesh->primitives = RELATIVE_PNTR_TO_ABSOLUTE(ModelPrimitive, header, mesh->primitives);
		
		#if DO_MODEL_LOAD_DEBUG_OUTPUT
		debugf("mesh[%lu]: %lu primitives\n", mIndex, mesh->numPrimitives);
		#endif
		
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
			
			#if DO_MODEL_LOAD_DEBUG_OUTPUT
			debugf("\tprimitive[%lu]: mode=%lu %lu vertices %lu indices texture[%lu]\n", pIndex, primitive->mode, primitive->numVertices, primitive->numIndices, primitive->localTexture);
			debugf("\t\tposition: %p type=0x%04lX size=%lu stride=%lu\n", primitive->position.pointer, primitive->position.type, primitive->position.size, primitive->position.stride);
			debugf("\t\tcolor:    %p type=0x%04lX size=%lu stride=%lu\n", primitive->color.pointer,    primitive->color.type,    primitive->color.size,    primitive->color.stride);
			debugf("\t\ttexcoord: %p type=0x%04lX size=%lu stride=%lu\n", primitive->texcoord.pointer, primitive->texcoord.type, primitive->texcoord.size, primitive->texcoord.stride);
			debugf("\t\tnormal:   %p type=0x%04lX size=%lu stride=%lu\n", primitive->normal.pointer,   primitive->normal.type,   primitive->normal.size,   primitive->normal.stride);
			//primitive[0]: 112 vertices 156 indices
			//primitive[1]: 296 vertices 444 indices
			#endif
			
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
					default: debugf("We don't support index type %lu on mesh[%lu].primitive[%lu] in %s\n", primitive->indexType, mIndex, pIndex, path.chars); break;
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
						v3 vertPos[3] = ZEROED;
						if (primitive->position.type == GL_FLOAT)
						{
							u32 stride = ((primitive->position.stride == 0) ? sizeof(r32) : primitive->position.stride);
							vertPos[0] = *(v3*)((u8*)primitive->position.pointer + (stride * triIndices[0]));
							vertPos[1] = *(v3*)((u8*)primitive->position.pointer + (stride * triIndices[1]));
							vertPos[2] = *(v3*)((u8*)primitive->position.pointer + (stride * triIndices[2]));
						}
						else if (primitive->position.type == GL_HALF_FIXED_N64)
						{
							u32 stride = ((primitive->position.stride == 0) ? sizeof(u16) : primitive->position.stride);
							ui16* vertPntr0 = (ui16*)((u8*)primitive->position.pointer + (stride * triIndices[0]));
							ui16* vertPntr1 = (ui16*)((u8*)primitive->position.pointer + (stride * triIndices[1]));
							ui16* vertPntr2 = (ui16*)((u8*)primitive->position.pointer + (stride * triIndices[2]));
							
							vertPos[0].x = HalfFixedN64ToR32(vertPntr0[0], primitive->vertexPrecision);
							vertPos[0].y = HalfFixedN64ToR32(vertPntr0[1], primitive->vertexPrecision);
							vertPos[0].z = HalfFixedN64ToR32(vertPntr0[2], primitive->vertexPrecision);
							
							vertPos[1].x = HalfFixedN64ToR32(vertPntr1[0], primitive->vertexPrecision);
							vertPos[1].y = HalfFixedN64ToR32(vertPntr1[1], primitive->vertexPrecision);
							vertPos[1].z = HalfFixedN64ToR32(vertPntr1[2], primitive->vertexPrecision);
							
							vertPos[2].x = HalfFixedN64ToR32(vertPntr2[0], primitive->vertexPrecision);
							vertPos[2].y = HalfFixedN64ToR32(vertPntr2[1], primitive->vertexPrecision);
							vertPos[2].z = HalfFixedN64ToR32(vertPntr2[2], primitive->vertexPrecision);
						}
						else { AssertMsg(primitive->position.type == GL_FLOAT || primitive->position.type == GL_HALF_FIXED_N64, "Unsupported position attribute format!"); }
						
						#if DO_MODEL_LOAD_DEBUG_OUTPUT
						debugf("\t\ttri[%lu] [%lu->%lu->%lu]\n", iIndex/3, triIndices[0], triIndices[1], triIndices[2]);
						debugf("\t\ttri[%lu] [(%g,%g,%g)->(%g,%g,%g)->(%g,%g,%g)]\n",
							iIndex/3,
							vertPos[0].x, vertPos[0].y, vertPos[0].z,
							vertPos[1].x, vertPos[1].y, vertPos[1].z,
							vertPos[2].x, vertPos[2].y, vertPos[2].z
						);
						#endif
						
						if (result.numFaces < MAX_COLLISION_FACES)
						{
							CollisionFace* face = &result.faces[result.numFaces];
							result.numFaces++;
							face->verts[0] = vertPos[0];
							face->verts[1] = vertPos[1];
							face->verts[2] = vertPos[2];
							face->normal = CrossV3(NormalizeV3(SubV3(face->verts[1], face->verts[0])), NormalizeV3(SubV3(face->verts[2], face->verts[0])));
							face->planeDist = DotV3(face->verts[0], face->normal);
							
							if (!boundsSet) { result.bounds = MakeBoxV(face->verts[0], V3_Zero); }
							result.bounds = ExpandBoxToV3(result.bounds, face->verts[0]);
							result.bounds = ExpandBoxToV3(result.bounds, face->verts[1]);
							result.bounds = ExpandBoxToV3(result.bounds, face->verts[2]);
						}
						else { debugf("WARNING: Max collision faces hit!\n"); }
					}
				}
			}
			else { debugf("\tWARNING: We don't support primitive mode %lu with position type %lu on mesh[%lu].primitive[%lu] in \"%s\"\n", primitive->mode, primitive->position.type, mIndex, pIndex, path.chars); }
		}
	}
	
	for (u32 nIndex = 0; nIndex < header->numNodes; nIndex++)
	{
		ModelNode* node = &header->nodes[nIndex];
		node->name     = RELATIVE_PNTR_TO_ABSOLUTE(char,      header, node->name);
		node->mesh     = RELATIVE_PNTR_TO_ABSOLUTE(ModelMesh, header, node->mesh);
		node->skin     = RELATIVE_PNTR_TO_ABSOLUTE(ModelSkin, header, node->skin);
		node->children = RELATIVE_PNTR_TO_ABSOLUTE(u32,       header, node->children);
		
		#if DO_MODEL_LOAD_DEBUG_OUTPUT
		debugf("node[%lu]: \"%s\"\n", nIndex, node->name); //node[0]: "Grid"
		debugf("\tmesh        = %p\n",  node->mesh); //mesh        = 0x800e6f78
		debugf("\tskin        = %p\n",  node->skin); //skin        = 0x0
		debugf("\ttransform   = pos=(%g, %g, %g) rot=(%g, %g, %g, %g) scale=(%g, %g, %g)\n", //transform   = pos=(0, -3.1514, 0) rot=(0, 0, 0, 1) scale=(12.2019, 12.2019, 12.2019)
			node->transform.pos[0], node->transform.pos[1], node->transform.pos[2],
			node->transform.rot[0], node->transform.rot[1], node->transform.rot[2], node->transform.rot[3],
			node->transform.scale[0], node->transform.scale[1], node->transform.scale[2]
		);
		debugf("\t  [%g, %g, %g, %g]\n", node->transform.mtx[0],  node->transform.mtx[1],  node->transform.mtx[2],  node->transform.mtx[3]);  //[12.2019, 0, 0, 0]
		debugf("\t  [%g, %g, %g, %g]\n", node->transform.mtx[4],  node->transform.mtx[5],  node->transform.mtx[6],  node->transform.mtx[7]);  //[0, 12.2019, 0, 0]
		debugf("\t  [%g, %g, %g, %g]\n", node->transform.mtx[8],  node->transform.mtx[9],  node->transform.mtx[10], node->transform.mtx[11]); //[0, 0, 12.2019, 0]
		debugf("\t  [%g, %g, %g, %g]\n", node->transform.mtx[12], node->transform.mtx[13], node->transform.mtx[14], node->transform.mtx[15]); //[0, -3.1514, 0, 1]
		debugf("\tparent      = %lu\n", node->parent); //parent      = 1
		debugf("\tnumChildren = %lu\n", node->numChildren); //numChildren = 0
		for (u32 cIndex = 0; cIndex < node->numChildren; cIndex++) { debugf("\t\tchild[%lu]: %lu\n", cIndex, node->children[cIndex]); } //children    = 0x800e8a50
		#endif
	}
	
	for (u32 tIndex = 0; tIndex < header->numTextures; tIndex++)
	{
		header->texturePaths[tIndex] = RELATIVE_PNTR_TO_ABSOLUTE(char, header, header->texturePaths[tIndex]);
		
		#if DO_MODEL_LOAD_DEBUG_OUTPUT
		debugf("texture[%lu]: \"%s\"\n", tIndex, header->texturePaths[tIndex]); // texture[0]: "dirt_road_grass.sprite" - texture[1]: "grass.sprite"
		#endif
	}
	
	free(modelData);
	return result;
} 

