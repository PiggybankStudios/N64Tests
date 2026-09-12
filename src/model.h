/*
File:   model.h
Author: Taylor Robbins
Date:   09\12\2026
*/

#ifndef _MODEL_H
#define _MODEL_H

// +--------------------------------------------------------------+
// |                       libdragon Types                        |
// +--------------------------------------------------------------+
// @brief Parameters for a single vertex attribute (part of #primitive_t)
// Copy of libdragon's attribute_t
typedef struct ModelPrimAttrib ModelPrimAttrib;
struct ModelPrimAttrib
{
	u32 size;                  ///< Number of components per vertex. If 0, this attribute is not defined
	u32 type;                  ///< The data type of each component (for example GL_FLOAT)
	u32 stride;                ///< The byte offset between consecutive vertices. If 0, the values are tightly packed
	void* pointer;             ///< Pointer to the first value
};

// @brief A single draw call that makes up part of a mesh (part of #mesh_t)
// Copy of libdragon's primitive_t
typedef struct ModelPrimitive ModelPrimitive;
struct ModelPrimitive
{
	u32 mode;                 ///< ModelPrimitive assembly mode (for example GL_TRIANGLES)
	ModelPrimAttrib position; ///< Vertex position attribute, if defined
	ModelPrimAttrib color;    ///< Vertex color attribyte, if defined
	ModelPrimAttrib texcoord; ///< Texture coordinate attribute, if defined
	ModelPrimAttrib normal;   ///< Vertex normals, if defined
	ModelPrimAttrib mtxIndex; ///< Matrix indices (aka bones), if defined
	u32 vertexPrecision;      ///< If the vertex positions use fixed point values, this defines the precision
	u32 texcoordPrecision;    ///< If the texture coordinates use fixed point values, this defines the precision
	u32 indexType;            ///< Data type of indices (for example GL_UNSIGNED_SHORT)
	u32 numVertices;          ///< Number of vertices
	u32 numIndices;           ///< Number of indices
	u32 localTexture;         ///< Texture index in this model
	u32 sharedTexture;        ///< A shared texture index between other models
	void* indices;            ///< Pointer to the first index value. If NULL, indices are not used
};

// @brief Transform of a node of a model
// Copy of libdragon's node_transform_t
typedef struct ModelNodeTransform ModelNodeTransform;
struct ModelNodeTransform
{
	r32 pos[3];   ///< Position of a node
	r32 rot[4];   ///< Rotation of a node (quaternion)
	r32 scale[3]; ///< Scale of a node
	r32 mtx[16];  ///< Matrix of a node
};

// @brief A mesh of the model
// Copy of libdragon's mesh_t
typedef struct ModelMesh ModelMesh;
struct ModelMesh
{
	u32 numPrimitives;          ///< Number of primitives
	ModelPrimitive* primitives; ///< Pointer to the first primitive
};

// @brief A joint of the model
// Copy of libdragon's model64_joint_t
typedef struct ModelJoint ModelJoint;
struct ModelJoint
{
	u32 nodeIdx;            ///< Index of node joint is attached to
	r32 inverseBindMat[16]; ///< Inverse bind matrix of joint
};

// @brief A skin of the model
// Copy of libdragon's model64_skin_t
typedef struct ModelSkin ModelSkin;
struct ModelSkin
{
	u32 numJoints;      ///< Number of joints
	ModelJoint* joints; ///< Pointer to the first joint
};

// Copy of libdragon's model64_node_t
typedef struct ModelNode ModelNode;
struct ModelNode
{
	char* name;                   ///< Name of a node
	ModelMesh* mesh;              ///< Mesh a node refers to
	ModelSkin* skin;              ///< Skin a node refers to
	ModelNodeTransform transform; ///< Initial transform of a node
	u32 parent;                   ///< Index of parent node
	u32 numChildren;              ///< Number of children nodes
	u32* children;                ///< List of children node indices
};

// @brief A keyframe of an animation
// Copy of libdragon's model64_keyframe_t
typedef struct ModelKeyframe ModelKeyframe;
struct ModelKeyframe
{
	r32 time;    ///< Time of keyframe
	r32 timeReq; ///< Time keyframe was requested
	u16 track;   ///< Track keyframe applies to
	u16 data[3]; ///< Data for keyframe
};

// @brief An animation of a model
// Copy of libdragon's model64_anim_t
typedef struct ModelAnim ModelAnim;
struct ModelAnim
{
	char* name;               ///< Name of the animation
	r32 posF1;                ///< Scale of position components of animation
	r32 posF2;                ///< Minimum position of animation
	r32 scaleF1;              ///< Scale of scale components of animation
	r32 scaleF2;              ///< Minimum scale of animation
	r32 duration;             ///< Duration of animation
	u32 numKeyframes;         ///< Number of keyframes in animation
	ModelKeyframe* keyframes; ///< Pointer to animation keyframes
	u32 numTracks;            ///< Number of tracks targeted by animation
	u16 *tracks;              ///< Top 2 bits: target component; lowest 14 bits: target node
};

// Copy of libdragon's model64_data_t
typedef struct ModelHeader ModelHeader;
struct ModelHeader
{
	u32 magic;            ///< Magic header (MODEL64_MAGIC)
	u32 refCount;         ///< Number of times model data is used
	u32 version;          ///< Version of this file
	u32 headerSize;       ///< Size of the header in bytes
	u32 meshSize;         ///< Size of a mesh header in bytes
	u32 primitiveSize;    ///< Size of a primitive header in bytes
	u32 nodeSize;         ///< Size of a node in bytes
	u32 skinSize;         ///< Size of a skin in bytes
	u32 animSize;         ///< Size of an animation in bytes
	u32 numNodes;         ///< Number of nodes
	ModelNode* nodes;     ///< Pointer to the first node
	u32 rootNode;         ///< Root node of the model
	u32 numSkins;         ///< Number of skins
	ModelSkin* skins;     ///< Pointer to the first skin
	u32 numMeshes;        ///< Number of meshes
	ModelMesh* meshes;    ///< Pointer to the first mesh
	u32 numAnims;         ///< Number of animations
	ModelAnim* anims;     ///< Pointer to first animation
	u32 maxTracks;        ///< Maximum number of tracks for animation
	void* animDataHandle; ///< Handle for animation data (0 means animations are not streamed)
	u32 numTextures;      ///< Number of texture paths
	char** texturePaths;  ///< Pointer to first texture path
};

// +--------------------------------------------------------------+
// |                          Our Types                           |
// +--------------------------------------------------------------+
typedef struct CollisionFace CollisionFace;
struct CollisionFace
{
	v3 verts[3];
	v3 normal;
	r32 planeDist;
};

typedef struct PlanetCollision PlanetCollision;
struct PlanetCollision
{
	box bounds;
	u32 numFaces;
	CollisionFace* faces;
};

#endif //  _MODEL_H
