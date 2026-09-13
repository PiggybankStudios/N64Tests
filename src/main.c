/*
File:   main.c
Author: Taylor Robbins
Date:   09\01\2026
Description: 
	** This is the only file that is directly compiled by the build_script.c
	** All other source files are #included inside this one (aka a "unity build")
*/

#define PIG_CORE_IMPLEMENTATION 1
#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "std/std_includes.h"

#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "std/std_basic_math.h"
#include "struct/struct_string.h"
#include "struct/struct_vectors.h"
#include "struct/struct_matrices.h"
#include "struct/struct_quaternion.h"
#include "struct/struct_rectangles.h"

// #include "base/base_all.h"
// #include "struct/struct_all.h"

// +==============================+
// |           Headers            |
// +==============================+
#include "registers.h"
#include "defines.h"
#include "macros.h"
#include "model.h"
#include "collision.h"

#include "rom.h"

// +==============================+
// |         Source Files         |
// +==============================+
#include "reboot.c"
#include "sc64.c"
#include "model.c"
#include "debug_draw.c"
#include "collision.c"
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

