/*
File:   debug_draw.c
Author: Taylor Robbins
Date:   09\12\2026
Description: 
	** Holds a bunch of useful functions that help us quickly draw simple geometry for debugging purposes
*/

void SetModelViewMatrix(const mat4* mat)
{
	GLfloat transposedMat[16] = {
		mat->r0c0, mat->r1c0, mat->r2c0, mat->r3c0,
		mat->r0c1, mat->r1c1, mat->r2c1, mat->r3c1,
		mat->r0c2, mat->r1c2, mat->r2c2, mat->r3c2,
		mat->r0c3, mat->r1c3, mat->r2c3, mat->r3c3
	};
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&transposedMat[0]);
}

void DrawBoxAt(v3 pos, r32 size)
{
	mat4 center = MakeTranslateXYZMat4_Const(-0.5f, -0.5f, -0.5f);
	mat4 scale = MakeScaleXYZMat4_Const(size, size, size);
	mat4 translate = MakeTranslateMat4_Const(pos);
	mat4 transformMat = Mat4_Identity_Const;
	TransformMat4(&transformMat, center);
	TransformMat4(&transformMat, scale);
	TransformMat4(&transformMat, translate);
	SetModelViewMatrix(&transformMat);
	
	model64_draw(rom.unitBoxModel);
}

void DrawLineBox(v3 start, v3 end, r32 thickness)
{
	v3 lineVec = SubV3(end, start);
	r32 lineLength = LengthV3(lineVec);
	v3 forwardBasis = ShrinkV3(lineVec, lineLength);
	v3 rightBasis = V3_Zero_Const;
	if (start.x != end.x || start.z != end.z) { rightBasis = CrossV3(forwardBasis, V3_Up); }
	else { rightBasis = CrossV3(forwardBasis, V3_Forward); }
	v3 upBasis = CrossV3(rightBasis, forwardBasis);
	mat4 transformMat = Mat4_Identity_Const;
	mat4 center = MakeTranslateXYZMat4_Const(-0.5f, -0.5f, 0.0f);
	mat4 scale = MakeScaleXYZMat4_Const(thickness, thickness, lineLength);
	mat4 rotation = MakeMat4_Const(
		rightBasis.x/* * thickness*/, upBasis.x,             forwardBasis.x,              0.0f, //start.x,
		rightBasis.y,             upBasis.y/* * thickness*/, forwardBasis.y,              0.0f, //start.y,
		rightBasis.z,             upBasis.z,             forwardBasis.z/* * lineLength*/, 0.0f, //start.z,
		        0.0f,                  0.0f,                       0.0f,                  1.0f
	);
	mat4 translate = MakeTranslateMat4_Const(start);
	TransformMat4(&transformMat, center);
	TransformMat4(&transformMat, scale);
	TransformMat4(&transformMat, rotation);
	TransformMat4(&transformMat, translate);
	SetModelViewMatrix(&transformMat);
	
	model64_draw(rom.unitBoxModel);
}


