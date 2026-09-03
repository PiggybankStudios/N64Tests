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

