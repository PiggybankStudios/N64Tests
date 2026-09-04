/*
File:   main.c
Author: Taylor Robbins
Date:   09\01\2026
Description: 
	** This is the only file that is directly compiled by the build_script.c
	** All other source files are #included inside this one (aka a "unity build")
*/

#include <GL/gl.h>
#include <GL/gl_integration.h>

#include <libdragon.h>
#include <model64.h>

#define PIG_CORE_IMPLEMENTATION 1
#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "std/std_includes.h"
// #include "base/base_assert.h"
// #include "struct/struct_vectors.h"

// +==============================+
// |           Headers            |
// +==============================+
#include "defines.h"
#include "rom.h"

// +==============================+
// |         Source Files         |
// +==============================+
#include "rendering_tests.c"
#include "rom.c"

// +--------------------------------------------------------------+
// |                       Main Entry Point                       |
// +--------------------------------------------------------------+
int main(void)
{
	InitRom();
	
	while (!rom.shutdown)
	{
		UpdateRom();
		RenderRom();
	}
	
	return 0;
}

