/*
File:   build_script.c
Author: Taylor Robbins
Date:   08\22\2026
Description: 
	** None
*/

#include "pig_build.h"

#define TOOLCHAIN_PREFIX "mips64-elf"
#define ROM_NAME         "n64_tests"
#define ROM_TITLE        "N64 Tests"

#if !BUILDING_ON_WINDOWS
#error This project is only setup to build on Windows right now!
#endif

int main()
{
	PigBuildDebugMode = false;
	RecompileIfNeeded(StrArray_Empty);
	
	Str libDragonSrcDir = StrLit("C:/gamedev/downloaded/libdragon");
	Str toolchainDir = StrLit("F:/Programs/libdragon");
	Str toolchainBinDir = JoinPathsLit(toolchainDir, "/bin");
	Str gcc     = JoinPathsLit(toolchainBinDir, TOOLCHAIN_PREFIX "-gcc.exe");
	Str gpp     = JoinPathsLit(toolchainBinDir, TOOLCHAIN_PREFIX "-g++.exe");
	// Str ld      = JoinPathsLit(toolchainBinDir, TOOLCHAIN_PREFIX "-ld.exe");
	Str n64tool = JoinPathsLit(toolchainBinDir, "n64tool.exe");
	
	Str mainPath = StrLit("[ROOT]/src/main.c");
	// Str mainPath = JoinPathsLit(libDragonSrcDir, "/examples/ctest/ctest.c");
	// Str mainPath = JoinPathsLit(libDragonSrcDir, "/examples/helloworld/src/main.c");
	// Str mainPath = JoinPathsLit(libDragonSrcDir, "/examples/mixertest/mixertest.c");
	
	Str mainFilename = GetFileNamePart(mainPath, true);
	Str oFilename = ChangePathExtension(mainFilename, StrLit(".o"), true);
	Str mapFilename = ChangePathExtension(mainFilename, StrLit(".map"), true);
	Str elfFilename = ChangePathExtension(mainFilename, StrLit(".elf"), true);
	Str romFilename = StrLit(ROM_NAME ".z64");
	
	{
		PrintLine("Compiling %.*s...", StrPrint(mainFilename));
		
		CliArgs compileArgs = EMPTY;
		AddArg(&compileArgs, GCC_COMPILE);
		AddArgStr(&compileArgs, CLI_QUOTED_ARG, mainPath);
		AddArgStr(&compileArgs, GCC_OUTPUT_FILE, oFilename);
		AddArg(&compileArgs, "-march=vr4300");
		AddArg(&compileArgs, "-mtune=vr4300");
		AddArg(&compileArgs, "-mabi=o64");
		AddArgStr(&compileArgs, GCC_INCLUDE_DIR, JoinPaths(libDragonSrcDir, StrLit("/include")));
		AddArgNt(&compileArgs, GCC_ALIGN_FUNCS_TO, "32");
		AddArg(&compileArgs, GCC_SEP_FUNC_SECTIONS);
		AddArg(&compileArgs, GCC_SEP_DATA_SECTIONS);
		AddArg(&compileArgs, GCC_DEBUG_INFO_DEFAULT);
		// AddArg(&compileArgs, "-ffile-prefix-map="$(CURDIR)"=$(N64_BACKTRACE_FILE_PREFIX)"); // NOTE: if you change this, also change backtrace() in backtrace.c
		AddArg(&compileArgs, "-ffast-math");
		AddArg(&compileArgs, "-ftrapping-math");
		AddArg(&compileArgs, "-fno-associative-math");
		AddArgNt(&compileArgs, GCC_DEFINE, "N64");
		// AddArg(&compileArgs, "-fdiagnostics-color=always"); //TODO: Do we want this?
		AddArgNt(&compileArgs, GCC_OPTIMIZATION_LEVEL, "2");
		AddArgNt(&compileArgs, GCC_WARNING_LEVEL, "all");
		AddArg(&compileArgs, GCC_WARNINGS_AS_ERRORS);
		AddArgNt(&compileArgs, GCC_DISABLE_WARNING_AS_ERROR, "deprecated-declarations");
		AddArgNt(&compileArgs, GCC_DISABLE_WARNING_AS_ERROR, "unused-variable");
		AddArgNt(&compileArgs, GCC_DISABLE_WARNING_AS_ERROR, "unused-but-set-variable");
		AddArgNt(&compileArgs, GCC_DISABLE_WARNING_AS_ERROR, "unused-function");
		AddArgNt(&compileArgs, GCC_DISABLE_WARNING_AS_ERROR, "unused-parameter");
		AddArgNt(&compileArgs, GCC_DISABLE_WARNING_AS_ERROR, "unused-but-set-parameter");
		AddArgNt(&compileArgs, GCC_DISABLE_WARNING_AS_ERROR, "unused-label");
		AddArgNt(&compileArgs, GCC_DISABLE_WARNING_AS_ERROR, "unused-local-typedefs");
		AddArgNt(&compileArgs, GCC_DISABLE_WARNING_AS_ERROR, "unused-const-variable");
		AddArg(&compileArgs, "-ftrivial-auto-var-init=pattern");
		AddArgNt(&compileArgs, GCC_LANG_VERSION, "gnu17");
		
		RunCliProgramAndExitOnFailure(gcc, &compileArgs, StrLit("Failed to compile"));
		AssertFileExist(oFilename, true);
	}
	
	{
		PrintLine("Linking into %.*s...", StrPrint(elfFilename));
		
		CliArgs linkerArgs = EMPTY;
		AddArgStr(&linkerArgs, CLI_QUOTED_ARG, oFilename);
		AddArgStr(&linkerArgs, GCC_OUTPUT_FILE, elfFilename);
		AddArg(&linkerArgs, "-mabi=o64");
		AddArgStr(&linkerArgs, "-Wl," GCC_LIBRARY_DIR, JoinPathsLit(toolchainDir, "/" TOOLCHAIN_PREFIX "/lib"));
		AddArgNt(&linkerArgs, GCC_SYSTEM_LIBRARY, "c");
		AddArgNt(&linkerArgs, GCC_SYSTEM_LIBRARY, "dragon");
		AddArgNt(&linkerArgs, GCC_SYSTEM_LIBRARY, "m");
		AddArgNt(&linkerArgs, GCC_SYSTEM_LIBRARY, "dragonsys");
		AddArgStr(&linkerArgs, "-Wl," GCC_LINKER_SCRIPT, JoinPathsLit(libDragonSrcDir, "/n64.ld"));
		AddArg(&linkerArgs, GCC_GC_SECTIONS);
		AddArg(&linkerArgs, "-Wl,--wrap __do_global_ctors");
		AddArgStr(&linkerArgs, GCC_MAP_FILE, mapFilename);
		//TODO: Add support for .externs file with -Wl,-T"file.externs"
		
		RunCliProgramAndExitOnFailure(gpp, &linkerArgs, StrLit("Failed to link"));
		AssertFileExist(elfFilename, true);
	}
	
	{
		PrintLine("Creating %.*s...", StrPrint(romFilename));
		
		CliArgs toolArgs = EMPTY;
		AddArgNt(&toolArgs, "--title \"[VAL]\"", ROM_TITLE);
		// AddArgNt(&toolArgs, "--header \"[VAL]\"", ROM_HEADER);
		// AddArgNt(&toolArgs, "--category \"[VAL]\"", ROM_CATEGORY);
		// AddArgNt(&toolArgs, "--region \"[VAL]\"", ROM_REGION);
		AddArg(&toolArgs, "--toc");
		AddArgStr(&toolArgs, "--output \"[VAL]\"", romFilename);
		AddArgNt(&toolArgs, "--align [VAL]", "256");
		// TODO: $<.stripped --align 8
		// TODO: $<.sym --align 8
		AddArgStr(&toolArgs, CLI_QUOTED_ARG, elfFilename);
		
		RunCliProgramAndExitOnFailure(n64tool, &toolArgs, StrLit("n64tool.exe threw an error!"));
		AssertFileExist(romFilename, true);
	}
	
	WriteLine("DONE!");
	return 0;
}

