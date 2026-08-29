/*
File:   build_script.c
Author: Taylor Robbins
Date:   08\22\2026
Description: 
	** None
*/

#include "pig_build.h"
#include "sc64deployer_flags.h"
#include "libdragon_bin_flags.h"

#define DEBUG_BUILD         1
#define MAKE_RESOURCES_DFS  1
#define UPLOAD_TO_SC64      1
#define INSTALL_TO_SC64     0 // Make sure the console is powered off

#if DEBUG_BUILD
#define IF_DEBUG(...)     __VA_ARGS__
#define IF_NOT_DEBUG(...) //nothing
#define IF_RELEASE(...)   //nothing
#else
#define IF_DEBUG(...)     //nothing
#define IF_NOT_DEBUG(...) __VA_ARGS__
#define IF_RELEASE(...)   __VA_ARGS__
#endif

#define TOOLCHAIN_PREFIX "mips64-elf"
#define ROM_NAME         "n64_tests"
#define ROM_TITLE        "N64 Tests"

#if !BUILDING_ON_WINDOWS
#error This project is only setup to build on Windows right now!
#endif

int main()
{
	PigBuildDebugMode = false;
	RecompileIfNeeded(MakeStrArrayVa("../build_script.c", "../sc64deployer_flags.h", "../libdragon_bin_flags.h"));
	
	Str libDragonSrcDir = StrLit("C:/gamedev/downloaded/libdragon-preview");
	Str toolchainDir = StrLit("F:/Programs/libdragon");
	Str toolchainBinDir = JoinPathsLit(toolchainDir, "/bin");
	Str gcc          = JoinPathsLit(toolchainBinDir, TOOLCHAIN_PREFIX "-gcc.exe");
	Str gpp          = JoinPathsLit(toolchainBinDir, TOOLCHAIN_PREFIX "-g++.exe");
	// Str ld           = JoinPathsLit(toolchainBinDir, TOOLCHAIN_PREFIX "-ld.exe");
	Str n64tool      = JoinPathsLit(toolchainBinDir, "n64tool.exe");
	Str sc64deployer = StrLit("F:/Programs/sc64deployer/sc64deployer.exe");
	Str mkmodel      = JoinPathsLit(toolchainBinDir, MKMODEL_EXE);
	Str mksprite     = JoinPathsLit(toolchainBinDir, MKSPRITE_EXE);
	Str mkdfs        = JoinPathsLit(toolchainBinDir, MKDFS_EXE);
	
	Str mainPath = StrLit("[ROOT]/src/main.c");
	// Str mainPath = JoinPathsLit(libDragonSrcDir, "/examples/ctest/ctest.c");
	// Str mainPath = JoinPathsLit(libDragonSrcDir, "/examples/helloworld/src/main.c");
	// Str mainPath = JoinPathsLit(libDragonSrcDir, "/examples/mixertest/mixertest.c");
	
	Str mainFilename = GetFileNamePart(mainPath, true);
	Str oFilename = ChangePathExtension(mainFilename, StrLit(".o"), true);
	Str mapFilename = ChangePathExtension(mainFilename, StrLit(".map"), true);
	Str elfFilename = ChangePathExtension(mainFilename, StrLit(".elf"), true);
	Str romFilename = StrLit(ROM_NAME ".z64");
	
	// +==============================+
	// |     Make Resources .dfs      |
	// +==============================+
	Str resourcesDfsFilename = StrLit("resources.dfs");
	if (MAKE_RESOURCES_DFS || !DoesFileExist(resourcesDfsFilename))
	{
		if (!DoesFileExist(resourcesDfsFilename) && !MAKE_RESOURCES_DFS) { PrintLine("Creating \"%.*s\" because it doesn't exist yet", StrPrint(resourcesDfsFilename)); }
		
		Str filesystemDir = StrLit("filesystem");
		if (DoesFolderExist(filesystemDir)) { MyRemoveDirectory(filesystemDir, true); }
		MyCreateFolder(filesystemDir, false);
		
		// +==============================+
		// |         Make Models          |
		// +==============================+
		{
			Str modelsOutputDir = JoinPathsLit(filesystemDir, "models");
			MyCreateFolder(modelsOutputDir, false);
			
			FileIter fileIter = StartFileIter(StrLit("../resources/models"));
			bool isFolder = false;
			Str path = Str_Empty;
			while (StepFileIter(&fileIter, &path, &isFolder))
			{
				Str fileExt = GetFileExtPart(path, false);
				if (StrAnyCaseEquals(fileExt, StrLit(".gltf")) ||
					StrAnyCaseEquals(fileExt, StrLit(".glb")))
				{
					Str filename = GetFileNamePart(path, true);
					Str outputFilename = ChangePathExtension(filename, StrLit(".model64"), false);
					Str outputFilePath = JoinPaths(modelsOutputDir, outputFilename);
					PrintLine("Converting %.*s to %.*s...", StrPrint(filename), StrPrint(outputFilename));
					CliArgs mkModelArgs = EMPTY;
					mkModelArgs.pathSepChar = '/';
					AddArgStr(&mkModelArgs, MKMODEL_OUTPUT, modelsOutputDir);
					AddArg(&mkModelArgs, MKMODEL_NO_ANIM_STREAMING);
					AddArgNt(&mkModelArgs, MKMODEL_COMPRESS, "0");
					IF_DEBUG(AddArg(&mkModelArgs, MKMODEL_VERBOSE));
					AddArgStr(&mkModelArgs, CLI_QUOTED_ARG, path);
					RunCliProgramAndExitOnFailure(mkmodel, &mkModelArgs, FormatStr("Failed to convert \"%.*s\"", StrPrint(filename)));
					AssertFileExist(outputFilePath, false);
					//TODO: Should we expect an anims file to get created?
				}
				else if (StrAnyCaseEquals(fileExt, StrLit(".png")))
				{
					Str filename = GetFileNamePart(path, true);
					Str outputFilename = ChangePathExtension(filename, StrLit(".sprite"), false);
					Str outputFilePath = JoinPaths(StrLit("filesystem/models"), outputFilename);
					PrintLine("Converting %.*s to %.*s...", StrPrint(filename), StrPrint(outputFilename));
					CliArgs mkSpriteArgs = EMPTY;
					mkSpriteArgs.pathSepChar = '/';
					AddArgNt(&mkSpriteArgs, MKSPRITE_OUTPUT, "filesystem/models");
					AddArgNt(&mkSpriteArgs, MKSPRITE_FORMAT, "RGBA16");
					IF_DEBUG(AddArg(&mkSpriteArgs, MKSPRITE_VERBOSE));
					AddArgStr(&mkSpriteArgs, CLI_QUOTED_ARG, path);
					RunCliProgramAndExitOnFailure(mksprite, &mkSpriteArgs, FormatStr("Failed to convert \"%.*s\"", StrPrint(filename)));
					AssertFileExist(outputFilePath, false);
				}
			}
		}
		
		CliArgs mkDfsArgs = EMPTY;
		AddArgStr(&mkDfsArgs, CLI_QUOTED_ARG, resourcesDfsFilename);
		AddArgStr(&mkDfsArgs, CLI_QUOTED_ARG, filesystemDir);
		RunCliProgramAndExitOnFailure(mkdfs, &mkDfsArgs, FormatStr("Failed to create filesystem to contain resources \"%.*s\"", StrPrint(resourcesDfsFilename)));
		AssertFileExist(resourcesDfsFilename, false);
	}
	
	// +==============================+
	// |           Compile            |
	// +==============================+
	{
		PrintLine("Compiling %.*s...", StrPrint(mainFilename));
		
		CliArgs compileArgs = EMPTY;
		AddArg(&compileArgs, GCC_COMPILE);
		AddArgStr(&compileArgs, CLI_QUOTED_ARG, mainPath);
		AddArgStr(&compileArgs, GCC_OUTPUT_FILE, oFilename);
		IF_NOT_DEBUG(AddDefineArgLit(&compileArgs, "NDEBUG"));
		AddDefineArgLit(&compileArgs, "LIBDRAGON_PREVIEW=2");
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
		AddArgNt(&compileArgs, DEBUG_BUILD ? GCC_DISABLE_WARNING : GCC_DISABLE_WARNING_AS_ERROR, "unused-variable");
		AddArgNt(&compileArgs, DEBUG_BUILD ? GCC_DISABLE_WARNING : GCC_DISABLE_WARNING_AS_ERROR, "unused-but-set-variable");
		AddArgNt(&compileArgs, DEBUG_BUILD ? GCC_DISABLE_WARNING : GCC_DISABLE_WARNING_AS_ERROR, "unused-function");
		AddArgNt(&compileArgs, DEBUG_BUILD ? GCC_DISABLE_WARNING : GCC_DISABLE_WARNING_AS_ERROR, "unused-parameter");
		AddArgNt(&compileArgs, DEBUG_BUILD ? GCC_DISABLE_WARNING : GCC_DISABLE_WARNING_AS_ERROR, "unused-but-set-parameter");
		AddArgNt(&compileArgs, DEBUG_BUILD ? GCC_DISABLE_WARNING : GCC_DISABLE_WARNING_AS_ERROR, "unused-label");
		AddArgNt(&compileArgs, DEBUG_BUILD ? GCC_DISABLE_WARNING : GCC_DISABLE_WARNING_AS_ERROR, "unused-local-typedefs");
		AddArgNt(&compileArgs, DEBUG_BUILD ? GCC_DISABLE_WARNING : GCC_DISABLE_WARNING_AS_ERROR, "unused-const-variable");
		AddArg(&compileArgs, "-ftrivial-auto-var-init=pattern");
		AddArgNt(&compileArgs, GCC_LANG_VERSION, "gnu17");
		
		StrArray tags = EMPTY;
		AddTag(&tags, T_GCC);
		AddTag(&tags, T_LANG_C);
		AddTag(&tags, T_OBJECT);
		
		RunCliProgramAndExitOnFailureTags(gcc, tags, &compileArgs, StrLit("Failed to compile"));
		AssertFileExist(oFilename, true);
	}
	
	// +==============================+
	// |             Link             |
	// +==============================+
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
		
		StrArray tags = EMPTY;
		AddTag(&tags, T_GCC);
		AddTag(&tags, T_LANG_C);
		AddTag(&tags, T_LIBRARY);
		
		RunCliProgramAndExitOnFailureTags(gpp, tags, &linkerArgs, StrLit("Failed to link"));
		AssertFileExist(elfFilename, true);
	}
	
	// +==============================+
	// |           Make ROM           |
	// +==============================+
	{
		PrintLine("Creating %.*s...", StrPrint(romFilename));
		
		//TODO: mips64-elf-strip.exe on the .elf
		//TODO: n64elfcompress.exe on the stripped .elf
		
		CliArgs toolArgs = EMPTY;
		AddArgNt(&toolArgs, "--title \"[VAL]\"", ROM_TITLE);
		// AddArgNt(&toolArgs, "--header \"[VAL]\"", ROM_HEADER);
		// AddArgNt(&toolArgs, "--category \"[VAL]\"", ROM_CATEGORY);
		// AddArgNt(&toolArgs, "--region \"[VAL]\"", ROM_REGION);
		AddArg(&toolArgs, "--toc");
		AddArgStr(&toolArgs, "--output \"[VAL]\"", romFilename);
		AddArgNt(&toolArgs, "--align [VAL]", "256");
		AddArgStr(&toolArgs, CLI_QUOTED_ARG, elfFilename);
		AddArgNt(&toolArgs, "--align [VAL]", "8");
		if (DoesFileExist(resourcesDfsFilename)) { AddArgStr(&toolArgs, CLI_QUOTED_ARG, resourcesDfsFilename); }
		
		RunCliProgramAndExitOnFailure(n64tool, &toolArgs, StrLit("n64tool.exe threw an error!"));
		AssertFileExist(romFilename, true);
		
		//TODO: ed64romconfig.exe on the .z64 [--savetype none/eeprom4k/eeprom16k/sram256k/sram768k/sram1m/flashram] [--rtc] [--regionfree] [--conroller1/2/3/4 n64/none/mouse/vru/gamecube/randnetkeyboard/gamecubekeyboard/n64,pak=rumble/controller/transfer]
	}
	
	// +==============================+
	// |            Upload            |
	// +==============================+
	if (UPLOAD_TO_SC64)
	{
		PrintLine("Uploading %.*s to SC64...", StrPrint(romFilename));
		CliArgs uploadArgs = EMPTY;
		AddArg(&uploadArgs, SC64_CMD_UPLOAD);
		AddArg(&uploadArgs, SC64_UPLOAD_OPTION_REBOOT); //TODO: What do we need to do in order to get this working? Warning says: no response for [Reboot] AUX message
		AddArgStr(&uploadArgs, CLI_QUOTED_ARG, romFilename);
		RunCliProgramAndExitOnFailure(sc64deployer, &uploadArgs, StrLit("Failed to upload ROM to SummerCart64 with sc64deployer.exe"));
	}
	
	// +==============================+
	// |           Install            |
	// +==============================+
	if (INSTALL_TO_SC64)
	{
		PrintLine("Installing %.*s on SC64...", StrPrint(romFilename));
		CliArgs uploadArgs = EMPTY;
		AddArg(&uploadArgs, SC64_CMD_SD);
		AddArg(&uploadArgs, SC64_SD_SUBCMD_UPLOAD);
		AddArgStr(&uploadArgs, CLI_QUOTED_ARG, romFilename);
		RunCliProgramAndExitOnFailure(sc64deployer, &uploadArgs, StrLit("Failed to upload ROM to SummerCart64 SD Card with sc64deployer.exe"));
	}
	
	WriteLine("DONE!");
	return 0;
}

