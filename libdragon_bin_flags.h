/*
File:   libdragon_bin_flags.h
Author: Taylor Robbins
Date:   08\25\2026
*/

#ifndef _LIBDRAGON_BIN_FLAGS_H
#define _LIBDRAGON_BIN_FLAGS_H

// mkasset.exe [flags] <input files...>
//   This tool can be used to compress/decompress arbitrary asset files in a format
//   that can be loaded by the libdragon library. To open the compressed
//   files, use asset_fopen() or asset_load().
//   Supported window sizes: 2, 4, 8, 16, 32, 64, 128, 256
//   The window size affects the memory used by asset_fopen() only.
//   If you only use asset_load(), use the biggest window (256 KiB) to improve ratio.
#define MKASSET_EXE "mkasset.exe"
#define MKASSET_OUTPUT   "--output <dir>"       //-o, Specify output directory (default: .)
#define MKASSET_COMPRESS "--compress <algo>"    //-c, Compression level 0-3 (default: 1)
#define MKASSET_WINDOW   "--window <window>"    //-w, Maximum size of the matching window in KiB. (default: 4)
#define MKASSET_VERBOSE  "--verbose"            //-v, Verbose output

// mkdfs.exe <File> <Directory>
//   where <File> is the resulting filesystem image
//   and <Directory> is the directory (including subdirectories) to include
#define MKDFS_EXE "mkdfs.exe"

// mkfont.exe [flags] <input files...>
#define MKFONT_EXE "mkfont.exe"
// Command-line flags:
//    -o/--output <dir>         Specify output directory (default: .)
//    -v/--verbose              Verbose output
//    --no-kerning              Do not export kerning information
//    --ellipsis <cp>,<reps>    Select glyph and repetitions to use for ellipsis (default: 2E,3)
//    -c/--compress <level>     Compress output files (default: 1)
//    -d/--debug                Dump also debug images
// TTF/OTF specific flags:
//    -s/--size <pt>            Point size of the font (default: whatever the font defaults to)
//    --monochrome              Force monochrome output, with no aliasing (default: off)
//    --outline <width>         Add outline to font, specifying its width in (fractional) pixels
//    --char-spacing <width>    Add extra spacing between characters (default: 0)
//    --display <WxH[,A:B]>     Specify target display resolution and optional aspect ratio
//                              (e.g., --display 320x240 or --display 320x240,16:9)
//                              Default assumes 4:3 display ratio
//    --format <format>         Specify the output texture format for color fonts.
//                              Valid options are: RGBA16, RGBA32, CI4, CI8 (default: autoselect)
//    --var-axis <tag=value>    Override axis value of variable font
//                              (e.g., --var-axis wght=800)
//                              Can be specified multiple times.
//    Glyph selection modes (choose one of the following):
//    --charset <file>          Create a font that covers all and only the glyphs used in the
//                              specified file (in UTF-8 format).
//    -r/--range <start-stop>   Range of unicode codepoints to convert, as hex values (default: 20-7F)
//                              Can be specified multiple times. Use "--range all" to extract all
//                              glyphs in the font.
// BMFont specific flags:
//    --format <format>         Specify the output texture format. Valid options are:
//                              RGBA16, RGBA32, CI4, CI8 (default: RGBA16)

// mkmaterial [flags] <file.mat>...
#define MKMATERIAL_EXE "mkmaterial.exe"
// Command-line flags:
//   -v, --verbose            verbose output
//   -h, --help               print this help message
//   -I, --include [path]     specify additional texture path
//   -o, --output [path]      specify output path (default: .)
//   -t, --texdb [path]       specify texture database path (default: {output}/texdb)
//   -c. --compress [level]   specify compression level for textures (default: 1)
//   --raw-material           generate a single raw headerless material instead of a database

// mkmodel.exe [flags] <input files...>
#define MKMODEL_EXE "mkmodel.exe"
#define   MKMODEL_OUTPUT            "--output \"[VAL]\""   //-o, Specify output directory (default: .)
#define   MKMODEL_NO_ANIM_STREAMING "--anim-no-stream"     //    Disable animation streaming
#define   MKMODEL_COMPRESS          "--compress \"[VAL]\"" //-c, Compress output files (default: 1)
#define   MKMODEL_VERBOSE           "--verbose"            //-v, Verbose output

// mksprite [flags] <input files...>
//   Supported formats: AUTO, RGBA32, RGBA16, YUV16, IA16, CI8, I8, IA8, CI4, I4, IA4, ZBUF, IHQ
//   Supported mipmap algorithms: NONE (disable), BOX
//   Supported dithering algorithms: NONE (disable), RANDOM, ORDERED.
#define MKSPRITE_EXE "mksprite.exe"
#define   MKSPRITE_VERBOSE           "--verbose"               //-v, Verbose output (can be specified multiple times)
#define   MKSPRITE_OUTPUT            "--output \"[VAL]\""      //-o, Specify output directory (default: .)
#define   MKSPRITE_FORMAT            "--format [VAL]"          //-f, Specify output RDP surface format (default: AUTO)
#define   MKSPRITE_GAMMA             "--gamma"                 //-g, Convert colors to linear scale (use with runtime VI gamma correction enabled)
#define   MKSPRITE_DEBUG             "--debug"                 //-d, Dump computed images as PNGs (eg: mipmaps)
#define   MKSPRITE_COMPRESS          "--compress [VAL]"        //-c, <level> Compress output files (default: level 1 for lossless, level 3 (H264) for lossy)
//Lossless Sprite (default mode):
#define   MKSPRITE_DITHER            "--dither [VAL]"          //-D, <dither> Dithering algorithm (default: NONE)
#define   MKSPRITE_MIPMAP            "--mipmap [VAL]"          //-m, <algo> Calculate mipmap levels using the specified algorithm (default: NONE)
#define   MKSPRITE_TEXPARMS          "--texparms [VAL]"        //    <x,s,r,m>/<x,x,s,s,r,r,m,m> Runtime sampling parameters: x=translation, s=scale, r=repetitions, m=mirror
#define   MKSPRITE_DETAIL            "--detail [VAL]"          //    [<image>[,<fmt>]][,<factor>] Use detail texture: <image> is the file to use as detail (default: reuse input image) <fmt> is the output format (default: AUTO) <factor> is the blend factor in range 0..1 (default: 0.5)
#define   MKSPRITE_DETAIL_TEXPARMS   "--detail-texparms [VAL]" //    <x,x,s,s,r,r,m,m> Runtime sampling parameters for the detail texture
//Lossy Sprite:
#define   MKSPRITE_LOSSY             "--lossy [VAL]"           //-L, <0..100> Activate lossy mode with the specified quality (default: 100 - lossless)

#endif //  _LIBDRAGON_BIN_FLAGS_H
