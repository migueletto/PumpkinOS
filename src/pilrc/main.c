/*
 * @(#)main.c
 *
 * Copyright 1997-1999, Wes Cherry   (mailto:wesc@technosis.com)
 *           2000-2005, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;  either version 2, or (at your option)
 * any version.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT  ANY  WARRANTY;   without  even   the  implied  warranty  of 
 * MERCHANTABILITY  or FITNESS FOR A  PARTICULAR  PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You  should have  received a  copy of the GNU General Public License
 * along with this program;  if not,  please write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Revisions:
 * ==========
 *
 * pre 18-Jun-2000 <numerous developers>
 *                 creation
 *     18-Jun-2000 Aaron Ardiri
 *                 GNU GPL documentation additions
 *     20-Nov-2000 Renaud malaval (rmalaval@palm.com)
 *                 Add full support of PalmOS 3.5 (bitfields, structures, ...)
 *                 Add resource LAUNCHERCATEGORY (taic)
 *                 Add support for Little Endian 32 bits: LE32 (ARM and NT)
 *     Jan-2001    Regis Nicolas
 *                 Merged 68K and LE32 version into one binary
 *     12-Jan-2001 Renaud Malaval
 *                 Added 'wrdl' resource support
 *      4-Jun-2002 Ben Combee
 *                 Added more control over AUTOID numbers
 *      3-Oct-2002 Ben Combee
 *                 Changed include path mechanism to allow unlimited
 *                 numbers of include paths (constrained by memory)
 *     23-Oct-2003 Don't print copyright string when -q used
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef PALM_INTERNAL
#include <io.h>
#include <direct.h>
#endif
#include "pilrc.h"
#include "version.h"
#include "restype.h"

#if !defined(__GNUC__) && !defined(__MWERKS__)
#ifdef PALM_INTERNAL
#pragma message( "- Palm internal version" )
#else
#pragma message( "- Standard version" )
#endif
#endif

#define Str(x)  #x
#define XStr(x) Str(x)

static const char header[] =
"PilRC v" PILRC_VERSION_STR "\n"
"  Copyright 1997-1999 Wes Cherry   (wesc@ricochet.net)\n"
"  Copyright 2000-2005 Aaron Ardiri (aaron@ardiri.com)\n";

static const char disclaimer[] =
"This program is free software; you may redistribute it under the\n"
"terms of the GNU General Public License. This program has absolutely\n"
"no warranty, you use it AS IS at your own risk.\n";

static const char usage[] =
"Usage: pilrc [options...] infile [outfiledir]\n"
"\n"
"Options:\n"
"  -L <language>  Use the TRANSLATION section for the given language\n"
"                 Up to " XStr(MAXLANG) " -L options may be given\n"
"  -I <path>      Search for bitmap and include files in <path> as well as\n"
"                 the current directory (may be given more than once)\n"
#ifdef PALM_INTERNAL
"  -noIFIH        Parse includes files in header files\n"
#else
"  -noIFIH        Ignore includes files in header files\n"
#endif
"  -R <resfile>   Generate JUMP/PilA .res file\n"
"  -ro            Generate resource database file (.ro) instead of .bins\n"
"  -ts            put POSIX timestamp on .ro file generated\n"
"  -o <filedir>   Equivalent to [outfiledir]\n"
"  -H <incfile>   Autoassign IDs and write .h file with #defines\n"
"  -idStart <n>   Start autoID assignment at number <n>, counting up\n"
"  -D <macro>     Define a pre-processor macro symbol\n"
"  -F5            Use Big5 Chinese font widths\n"
"  -Fkt           Use Korean font widths (hantip font)\n"
"  -Fkm           Use Korean font widths (hanme font)\n"
"  -Fg            Use GB Chinese font widths\n"
"  -Fj            Use Japense font widths\n"
"  -Fh            Use Hebrew font widths\n"
"  -Fc            Use Cyrillic font widths\n"
"  -rtl           Right to left support\n"
"  -q             Less noisy output\n"
"  -V             Generate M$ (VS-type) error/warning output\n"
"  -allowEditID   Disable warnings about menu item IDs in the\n"
"                 edit (10000-10007) range\n"
"  -allowBadSize  Disable warnings about icon sizes being incorrect\n"
"  -allowLargeRes Disable warnings about resources that exceed the maximum\n"
"                 HotSync resource size\n"
"  -noEllipsis    Disable special handling of \"...\" and ellipsis char\n"
"  -PalmRez       Generate res with PalmRez option\n"
"  -LE32          Generate 32 bit little endian (ARM, NT) resources\n"
#ifdef PALM_INTERNAL
"  -AppIcon68K    Force AppIcon resources generation in 68K format\n"
"  -amdc <name>   Generate resource 'amdc' id=1 with <name>.dll as content\n"
#endif
"  -Loc <code>    Compile only res with the attribute LOCALE \"code\"\n"
"                 code samples: deDE, esES, enUS, frFR, itIT, jpJP\n"
"  -StripLoc      Don't compile 'non localisable resources'\n"
"  -type          Specify the type to use when generating a prc (-ro)\n"
"  -creator       Specify the creator to use when generating a prc (-ro)\n"
"  -name          Specify the database name when generating a prc (-ro)\n"
"  <outfiledir>   Directory where .bin files should be generated, or name of\n"
"                 the file to generate containing all the generated resources\n"
"  -M             Generate dependency list only; suppress all other output\n"
"  -MD            Generate dependency list\n";

static void
ArgError()
{
  fprintf(stderr, "%s", usage);
  exit(1);
}

/**
 * The main entry point of PilRC.
 *
 * @param cArg    the number of command line arguments.
 * @param rgszArg a reference to a list of command line argument strings.
 * @return program exit status, zero successful, non-zero otherwise.
 */
int
main(int cArg,
     char *rgszArg[])
{
  char *szOutputPath;
  char *szInputFile;
  char *szResFile;
  char *szMacro;
  char *szValue;
  char *szIncFile;
  char *szIDStart;
  int i;
  int fontType;
  int macroValue;

  if (cArg < 2)
  {
    printf("%s\n%s\n%s", header, disclaimer, usage);
    exit(1);
  }

  // initialize
  vfCheckDupes = 1;
  szResFile = NULL;
  szIncFile = NULL;
  fontType = fontDefault;

  szOutputPath = ".";

  vfPrcName = NULL;
  vfPrcCreator = NULL;
  vfPrcType = NULL;
  vfStripNoLocRes = fFalse;
  szLocaleP = NULL;
  szDllNameP = NULL;
  //  vfIFIH = fFalse;
#ifdef PALM_INTERNAL
  vfIFIH = fTrue;                                // RMa Default no Parse include in header file
#else
  vfIFIH = fFalse;
#endif
  vfLE32 = fFalse;
  vfAppIcon68K = fFalse;
  vfAutoAmdc = fFalse;

  // process as many command line arguments as possible
  for (i = 1; i < cArg; i++)
  {
    // no more options to process
    if (rgszArg[i][0] != '-')
      break;

    // RMa localisation
    if (FSzEqI(rgszArg[i], "-Loc"))
    {
      if (i++ == cArg)
        ArgError();
      szLocaleP = rgszArg[i];
      continue;
    }
    if (FSzEqI(rgszArg[i], "-StripLoc"))
    {
      vfStripNoLocRes = fTrue;
      continue;
    }

    // language
    if (FSzEqI(rgszArg[i], "-L"))
    {
      if (i++ == cArg || totalLanguages >= MAXLANG)
        ArgError();

      aszLanguage[totalLanguages++] = rgszArg[i];
      continue;
    }

    // include directory(s)
    if (FSzEqI(rgszArg[i], "-I"))
    {
      if (i++ == cArg)
        ArgError();

	  AddAccessPath(rgszArg[i]);
      continue;
    }

    // define macro(s) for #ifdef's
    if (FSzEqI(rgszArg[i], "-D"))
    {
      if (i++ == cArg)
        ArgError();

      szMacro = strdup(rgszArg[i]);

      // Check if there is a value defined for the macro, otherwise use '1'
      if ((szValue = strchr(szMacro, '=')) != NULL)
      {
        *szValue++ = '\0';
        macroValue = atoi(szValue);
      }
      else
      {
        macroValue = 1;
      }

      AddSym(szMacro, macroValue);
      free(szMacro);
      continue;
    }

    // resource file for PILA?
    if (FSzEqI(rgszArg[i], "-R"))
    {
      if (i++ == cArg)
        ArgError();

      szResFile = rgszArg[i];
      continue;
    }

    // header file generation (autoID for ALL)
    if (FSzEqI(rgszArg[i], "-H"))
    {
      if (i++ == cArg)
        ArgError();

      szIncFile = rgszArg[i];
      vfAutoId = fTrue;
      continue;
    }

    // ID value control -- Increasing IDs
    if (FSzEqI(rgszArg[i], "-idStart"))
    {
      if (i++ == cArg)
        ArgError();
        
      szIDStart = rgszArg[i];
      // reset ID start sequence to this number and start counting up.
      vfAutoStartID = atoi(szIDStart);
      idAutoDirection = +1;
      continue;
    }

    // be quiet?
    if (FSzEqI(rgszArg[i], "-q"))
    {
      vfQuiet = fTrue;
      continue;
    }

    // allow "edit" ID's?
    if (FSzEqI(rgszArg[i], "-allowEditID"))
    {
      vfAllowEditIDs = fTrue;
      continue;
    }

    // allow "edit" ID's?
    if (FSzEqI(rgszArg[i], "-allowBadSize"))
    {
      vfAllowBadIconSizes = fTrue;
      continue;
    }

    // allow "edit" ID's?
    if (FSzEqI(rgszArg[i], "-allowLargeRes"))
    {
      vfAllowLargeResources = fTrue;
      continue;
    }

    // disable ellipsis processing?
    if (FSzEqI(rgszArg[i], "-noEllipsis"))
    {
      vfNoEllipsis = fTrue;
      continue;
    }

    // No default flag for form and object form
    if (FSzEqI(rgszArg[i], "-PalmRez"))
    {
      vfPalmRez = fTrue;
      continue;
    }

    // M$ (VS-type) error/warning ouput (regis_nicolas@palm.com)
    if (FSzEqI(rgszArg[i], "-V"))
    {
      vfVSErrors = fTrue;
      continue;
    }

    // font hebrew
    if (FSzEqI(rgszArg[i], "-Fh"))
    {
      fontType = fontHebrew;
      continue;
    }

    // font japanese
    if (FSzEqI(rgszArg[i], "-Fj"))
    {
      fontType = fontJapanese;
      continue;
    }

    // font chinese big 5
    if (FSzEqI(rgszArg[i], "-F5"))
    {
      fontType = fontChineseBig5;
      continue;
    }

    // font chinese GB
    if (FSzEqI(rgszArg[i], "-Fg"))
    {
      fontType = fontChineseGB;
      continue;
    }

    // font korean (jmjeong@oopsla.snu.ac.kr)
    if (FSzEqI(rgszArg[i], "-Fkm"))
    {
      fontType = fontKoreanHanme;
      continue;
    }

    // font korean (jmjeong@oopsla.snu.ac.kr)
    if (FSzEqI(rgszArg[i], "-Fkt"))
    {
      fontType = fontKoreanHantip;
      continue;
    }

    // font cyrillic
    if (FSzEqI(rgszArg[i], "-Fc"))
    {
      fontType = fontCyrillic;
      continue;
    }

    // right to left?
    if (FSzEqI(rgszArg[i], "-rtl"))
    {
      vfRTL = fTrue;
      continue;
    }

    // LE32
    if (FSzEqI(rgszArg[i], "-LE32"))
    {
      vfLE32 = fTrue;
      continue;
    }

#ifdef PALM_INTERNAL
    // AppIcon68K
    if (FSzEqI(rgszArg[i], "-AppIcon68K"))
    {
      vfAppIcon68K = fTrue;
      continue;
    }

    if (FSzEqI(rgszArg[i], "-amdc"))
    {
      if (i++ == cArg)
        ArgError();
      szDllNameP = rgszArg[i];
      vfAutoAmdc = fTrue;
      continue;
    }
#endif

    // Output a 'ro' File
    if (FSzEqI(rgszArg[i], "-ro"))
    {
      vfPrc = fTrue;
      continue;
    }

    // place timestamp (POSIX standard) on .ro file
    if (FSzEqI(rgszArg[i], "-ts"))
    {
      vfPrcTimeStamp = fTrue;
      continue;
    }
    /*
     * LDu 31-8-2001 : Ignore Include File In Header Files
     */
    if (FSzEqI(rgszArg[i], "-noIFIH"))
    {
#ifdef PALM_INTERNAL
      vfIFIH = fFalse;
#else
      vfIFIH = fTrue;
#endif
      continue;
    }

    // Output filename
    if (FSzEqI(rgszArg[i], "-o"))
    {
      if (i++ == cArg)
        ArgError();

      szOutputPath = rgszArg[i];
      continue;
    }

    // name definition for prc output?
    if (FSzEqI(rgszArg[i], "-name"))
    {
      if (i++ == cArg)
        ArgError();

      vfPrcName = rgszArg[i];
      continue;
    }

    // creator definition for prc output?
    if (FSzEqI(rgszArg[i], "-creator"))
    {
      if (i++ == cArg)
        ArgError();

      vfPrcCreator = rgszArg[i];
      continue;
    }

    // type definition for prc output?
    if (FSzEqI(rgszArg[i], "-type"))
    {
      if (i++ == cArg)
        ArgError();

      vfPrcType = 0;

      vfPrcType = rgszArg[i];
      continue;
    }

    // dependency tracking, no output files
    if (FSzEqI(rgszArg[i], "-M"))
    {
        vfTrackDepends = fTrue;
        vfInhibitOutput = fTrue;
        continue;
    }
    
    // dependency tracking, compiler resources too
    if (FSzEqI(rgszArg[i], "-MD"))
    {
        vfTrackDepends = fTrue;
        continue;
    }

    if (FSzEqI(rgszArg[i], "--version"))
    {
        printf("%s\n%s", header, disclaimer);
        exit(0);
    }

    if (FSzEqI(rgszArg[i], "--help"))
    {
        printf("%s\n%s\n%s", header, disclaimer, usage);
        exit(0);
    }

    // unknown argument?
    ArgError();
  }

  // display the (c) string
  if (!vfQuiet)
    printf("%s\n", header);

  if ((cArg - i) < 1)
    ArgError();

  if ((!szLocaleP) && (vfStripNoLocRes))
  {
    ErrorLine
      ("no -Loc option and strip no localizable resource, PilRc have nothing to do.");
  }

  // determine the input path
  szInputFile = rgszArg[i++];

  // determine the ouput path
  if (cArg != i)
    szOutputPath = rgszArg[i++];
  //  else
  //    szOutputPath = ".";

#ifdef PALM_INTERNAL
  // if output folder not exist create it
  if (strstr(szOutputPath, ".prc") == NULL)
    if ((_access(szOutputPath, 0)) == -1)
    {
      _mkdir(szOutputPath);
    }
#endif

  // last minute check? (extra stuff?)
  if (cArg != i)
    ArgError();

  if (!vfQuiet)
  {
    if (vfLE32)
      printf("Generating LE32 resources (ARM/NT) from '%s'.\n", szInputFile);
    else
      printf("Generating 68K resources from '%s'.\n", szInputFile);
  }

  ResTypeInit();
  CbInit();
  FreeRcpfile(ParseFile(szInputFile, szOutputPath, szResFile, szIncFile, fontType));
  FreeAccessPathsList();
  
  return 0;
}
