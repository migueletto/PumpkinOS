/*
 * @(#)util.c
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
 */

#ifdef WINGUI
#include <windows.h>
#elif defined(CW_PLUGIN)
#include "Extra.h"                               // this file is part of the plugin sources, not standard
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include "pilrc.h"

#ifdef WIN32
#define DIRSEPARATOR  '\\'
#else
#define DIRSEPARATOR  '/'
#endif

/*
 * Globals 
 */
char rgbZero[16];
BOOL vfErr;

/*
 * Globals for output file handling
 */

static FILE *vfhOut;                             /* file receiving binary resource data */
static int ibOut;                                /* current output offset (within .bin being emitted) */
static int ibTotalOut;                           /* total output offset (after .bins so far emitted) */

static FILE *vfhRes = NULL;                      /* file receiving resource file ("-R") output */

static const char *szOutFileDir = NULL;          /* directory for *.bin files */

static char *szOutResDBFile = NULL;              /* filename for final .ro file */
static char *szTempFile = NULL;                  /* temporary filename */

static VOID WriteOutResourceDB(void);

DEFPL(PLEXRESOURCEDIR)
typedef struct RESOURCEDIRENTRY
{
  p_int type[4];
  p_int id;
  p_int offset;
} RESOURCEDIRENTRY;

#define szRESOURCEDIRENTRY "b4,w,l"

static PLEXRESOURCEDIR resdir;

/*
 * Includes 
 */
static const char **includePaths = NULL;
static int totalIncludePaths = 0;
static int allocatedIncludePaths = 0;

/**
 * Adds a new access path to the end of the access path array
 *
 * @param path access path string to add to list, must remain available
 */
void
AddAccessPath(const char *path)
{
  /* allocate memory for include paths in bundles of 32 */
  if (totalIncludePaths == allocatedIncludePaths)
  {
    allocatedIncludePaths += 32;
    includePaths = realloc((void *)includePaths, allocatedIncludePaths * sizeof(const char *));
    if (includePaths == NULL)
    {
      /* error: out of memory */
      Error("Out of memory allocating include path");
    }
  }
  includePaths[totalIncludePaths++] = path;
}

void FreeAccessPathsList(void)
{
  free((void *)includePaths);

  includePaths = NULL;
  totalIncludePaths = 0;
  allocatedIncludePaths = 0;
}


#ifndef CW_PLUGIN
/*-----------------------------------------------------------------------------
|	Diagnostic
|
|		Print error or warning, and perhaps exit. All error reporting
|		is funneled via this function -- it is the only diagnostic
|		function that things like the CW plugin need to override.
-------------------------------------------------------------JohnM-----------*/
VOID
Diagnostic(BOOL fError, const FILELINE *pos,
           const char *szFormat, va_list *args)
{
  if (pos && pos->szFilename)
    fprintf(stderr, vfVSErrors? "%s(%d): %s : " : "%s:%d: %s: ",
            pos->szFilename, pos->line, fError? "error" : "warning");
  else
  {
    fprintf(stderr, "pilrc: ");
    if (!fError)
      fprintf(stderr, vfVSErrors? "%s : " : "%s: ", "warning");
  }

  vfprintf(stderr, szFormat, *args);
  fprintf(stderr, "\n");

  if (fError)
    exit(1);
}
#endif

/*-----------------------------------------------------------------------------
|	Error, ErrorLine, WarningLine
|
|		Call Diagnostic() with appropriate arguments
------------------------------------------------------------WESC------------*/
VOID
Error(const char *szFormat, ...)
{
  va_list args;
  va_start(args, szFormat);
  Diagnostic(fTrue, NULL, szFormat, &args);
  va_end(args);
}

VOID
ErrorLine(const char *szFormat, ...)
{
  va_list args;
  va_start(args, szFormat);
  Diagnostic(fTrue, &vIn.file, szFormat, &args);
  va_end(args);
}

VOID
WarningLine(const char *szFormat, ...)
{
  va_list args;
  va_start(args, szFormat);
  Diagnostic(fFalse, &vIn.file, szFormat, &args);
  va_end(args);
}


/*-----------------------------------------------------------------------------
|	MakeFilename
|
|		Construct a filename and return it in newly malloc()ed memory
-------------------------------------------------------------JOHN------------*/
char *
MakeFilename(const char *szFormat, ...)
{
  const char *s;
  char *fname, *fn;
  va_list args;
  size_t len = 0;

  va_start(args, szFormat);

  for (s = szFormat; *s; s++)
    if (*s == '%')
    {
      char *arg = va_arg(args, char *);
      if (*++s == 's')
        len += strlen(arg);
    }
    else
      len++;

  va_end(args);

  fname = malloc(len + 1);
  if (fname == NULL)
    Error("Out of memory");

  va_start(args, szFormat);

  fn = fname;
  for (s = szFormat; *s; s++)
    switch (*s)
    {
      case '%':
        switch (*++s)
        {
          case 's':
            strcpy(fn, va_arg(args, char *));
            fn = strrchr(fn, '\0');
            break;

          case 'e':
            {
              char *ext = va_arg(args, char *);
              size_t extlen = strlen(ext);
              *fn = '\0';
              if (strlen(fname) >= extlen && FSzEqI(fn - extlen, ext))
                fn -= extlen;
            }
            break;
        }
        break;

      case '/':
        if (fn > fname && fn[-1] != '/' && fn[-1] != '\\')
          *fn++ = DIRSEPARATOR;
        break;

      default:
        *fn++ = *s;
        break;
    }

  *fn = '\0';

  va_end(args);
  return fname;
}

/*-----------------------------------------------------------------------------
|	MakeTempFilename
|
|		Return a temporary filename in newly malloc()ed memory
-------------------------------------------------------------JOHN------------*/
char *
MakeTempFilename(void)
{
  /* In some situations, the race condition inherent in the use of tmpnam()
     is a security risk.  This is not a significant issue for PilRC.  */
  char *fname = tmpnam(NULL);
  if (fname == NULL)
    Error("Can't make temporary filename -- tmpnam() failed");
  return MakeFilename("%s", fname);
}


/*-----------------------------------------------------------------------------
|	EmitB
|	
|		Emit a byte to the output
-------------------------------------------------------------WESC------------*/
VOID
EmitB(unsigned char b)
{
  DumpBytes(&b, 1);
}

/*-----------------------------------------------------------------------------
|	EmitW
|	
|		Emit a word
-------------------------------------------------------------WESC------------*/
VOID
EmitW(unsigned short w)
{
  if (vfLE32)
  {
    /*
     * RMa little indian 
     */
    EmitB((unsigned char)(w & 0xff));
    EmitB((unsigned char)((w & 0xff00) >> 8));
  }
  else
  {
    /*
     * big indian 
     */
    EmitB((unsigned char)((w & 0xff00) >> 8));
    EmitB((unsigned char)(w & 0xff));
  }
}

/*-----------------------------------------------------------------------------
|	EmitL
|	
|		emit a long
-------------------------------------------------------------WESC------------*/
VOID
EmitL(unsigned long l)
{
  if (vfLE32)
  {
    /*
     * RMa little indian 
     */
    EmitB((unsigned char)(l & 0xff));
    EmitB((unsigned char)((l & 0xff00) >> 8));
    EmitB((unsigned char)((l & 0xff0000) >> 16));
    EmitB((unsigned char)((l & 0xff000000) >> 24));
  }
  else
  {
    /*
     * big indian 
     */
    EmitB((unsigned char)((l & 0xff000000) >> 24));
    EmitB((unsigned char)((l & 0xff0000) >> 16));
    EmitB((unsigned char)((l & 0xff00) >> 8));
    EmitB((unsigned char)(l & 0xff));
  }
}

/*-----------------------------------------------------------------------------
|	IbOut
|
|		Return current output file file offset
-------------------------------------------------------------WESC------------*/
int
IbOut(void)
{
  return ibOut;
}

/*-----------------------------------------------------------------------------
|	DumpBytes
|	
|		Emit bytes to current output file
-------------------------------------------------------------WESC------------*/
VOID
DumpBytes(VOID * pv,
          int cb)
{
#ifdef CW_PLUGIN
  CWDumpBytes(pv, cb);
  ibOut += cb;
#else
#ifdef HEXOUT                                    /* RMa activate Hex dump in debug */
  BYTE *pb;
  int ib;
  static int nbrBytesOut;
  static int ibLine;
  static BYTE rgbLine[16];

  if (!pv)
  {                                              /* RMa little hack to display clean Hex output */
    if ((ibLine > 0) && (ibLine < 16))
    {
      for (ib = ibLine; ib < 16; ib++)
      {
        rgbLine[ib] = ' ';
        printf("   ");
        if (ibLine == 8)
          printf(" ");
      }
      printf(" ");
      for (ib = 0; ib < 16; ib++)
      {
        if (isprint(rgbLine[ib]))
          printf("%c", rgbLine[ib]);
        else
          printf(".");
      }
    }
    ibLine = 0;
    nbrBytesOut = 0;
    return;
  }
#endif
  if (vfInhibitOutput || vfWinGUI)
    return;

  /*
   * #ifdef BINOUT 
   */
  fwrite(pv, cb, 1, vfhOut);
  ibOut += cb;

  /*
   * #endif 
   */
#ifdef HEXOUT
  pb = (BYTE *) pv;
  while (cb--)
  {
    if (ibLine == 0)
      printf("\n%08x  ", nbrBytesOut);
    rgbLine[ibLine++] = *pb;
    printf("%02x ", *pb);
    pb++;
    if (ibLine == 8)
      printf(" ");
    if (ibLine == 16)
    {
      nbrBytesOut += ibLine;
      ibLine = 0;
      printf(" ");
      for (ib = 0; ib < 16; ib++)
      {
        if (isprint(rgbLine[ib]))
          printf("%c", rgbLine[ib]);
        else
          printf(".");
      }
    }
  }
#endif
#endif
}

/*-----------------------------------------------------------------------------
|	PadBoundary
|	
|		Pads output to a word or a long boundary by emitting a 0 if necessary
-------------------------------------------------------------WESC------------*/
VOID
PadBoundary(void)
{
  if (vfLE32)
  {
    if (ibOut & 3)
      DumpBytes(rgbZero, 4 - (ibOut & 3));
  }
  else if (ibOut & 1)
    DumpBytes(rgbZero, 1);
}

/*-----------------------------------------------------------------------------
|	PadWordBoundary
|	
|		Pads output to a word boundary by emitting a 0 if necessary
-------------------------------------------------------------WESC------------*/
VOID
PadWordBoundary(void)
{
  if (ibOut & 1)
    DumpBytes(rgbZero, 1);
}

/*-----------------------------------------------------------------------------
|	SetOutFileDir
|	
|		Set output file path -- no trailing / or \ 
-------------------------------------------------------------WESC------------*/
VOID
SetOutFileDir(const char *sz)
{
  if (sz && strcmp(sz, ".") == 0)
    szOutFileDir = "";
  else
    szOutFileDir = sz;
}

/*-----------------------------------------------------------------------------
|      OpenResDBFile
|
|              Set up to write a PRC formatted .ro file
-------------------------------------------------------------JOHN------------*/

static VOID
RemoveTempFile(VOID)
{
  if (szTempFile)
  {
    remove(szTempFile);
    free(szTempFile);
    szTempFile = NULL;
  }
}

VOID
OpenResDBFile(const char *sz)
{
  static BOOL registered = fFalse;

  FILE *f;

  szOutResDBFile = MakeFilename("%s", sz);
  szTempFile = MakeTempFilename();

  f = fopen(szTempFile, "wb");
  if (f)
    fclose(f);

  if (!registered)
  {
    atexit(RemoveTempFile);
    registered = fTrue;
  }

  PlexInit(&resdir, sizeof(RESOURCEDIRENTRY), 64, 64);
}

VOID
CloseResDBFile(void)
{
  if (!vfInhibitOutput)
  {
      if (!vfQuiet)
        printf("Collecting *.bin files into %s\n", szOutResDBFile);

      Assert(vfhOut == NULL);
      vfhOut = fopen(szOutResDBFile, "wb");
      if (vfhOut == NULL)
        Error("Unable to open output resource DB %s: %s", szOutResDBFile, strerror(errno));

      WriteOutResourceDB();
      fclose(vfhOut);
  }
  
  RemoveTempFile();
  free(szOutResDBFile);
  PlexFree(&resdir);
}

/*-----------------------------------------------------------------------------
|	OpenOutput
|	
|		Open output file of the form %outfiledir%\TTTTXXXX.bin ,
|	
|	where TTTT is the base 4 character mac/pilot res type and
|	XXXX is the resource id as a 4 digit hex number
-------------------------------------------------------------WESC------------*/
static char outputResStr[5];
static int outputResID;

VOID
OpenOutput(char *szBase,
           int id)
{
#ifdef CW_PLUGIN
  CWOpenOutput(szBase, id);
  ibOut = 0;
#else
  char szBinFileName[4 + 4 + 4 + 1];

  // save for possible errors
  memset(outputResStr, 0, 5);
  strncpy(outputResStr, szBase, 4);
  outputResID = id;

  if (vfInhibitOutput || vfWinGUI)
    return;

  Assert(vfhOut == NULL);

  sprintf(szBinFileName, "%.4s%04x.bin", szBase, id);

  if (vfPrc)
  {
    RESOURCEDIRENTRY entry;

    intstrncpy(entry.type, szBase, 4);
    entry.id = id;
    entry.offset = ibTotalOut;
    PlexAddElement(&resdir, &entry);

    vfhOut = fopen(szTempFile, "ab");
    if (vfhOut == NULL)
      Error("Unable to open binary file %s: %s", szTempFile, strerror(errno));

    if (!vfQuiet)
      printf("Writing temporary %s ", szBinFileName);

    if (vfhRes)
      fprintf(vfhRes, "\tres '%s', %d, \"%s(%s)\"\n", szBase, id,
              szOutResDBFile, szBinFileName);
  }
  else
  {
    char *szBinPath;
    if (szOutFileDir == NULL)
      Error("Can't happen (OutFileDir is not set)");
    szBinPath = MakeFilename("%s/%s", szOutFileDir, szBinFileName);

    vfhOut = fopen(szBinPath, "w+b");
    if (vfhOut == NULL)
      Error("Unable to open binary file %s: %s", szBinPath, strerror(errno));

    if (!vfQuiet)
      printf("Writing %s ", szBinPath);

    if (vfhRes)
      fprintf(vfhRes, "\tres '%s', %d, \"%s\"\n", szBase, id, szBinPath);

    free(szBinPath);
  }

  ibOut = 0;
#endif
}

/*-----------------------------------------------------------------------------
|	CloseOutput
-------------------------------------------------------------WESC------------*/
VOID
CloseOutput(void)
{
#ifdef CW_PLUGIN
  CWCloseOutput();
#else
#ifdef HEXOUT                                    /* RMA call little hack to display clean Hex output */
  DumpBytes(NULL, 0);
  putchar('\n');
#endif


  /*
   * #ifdef BINOUT 
   */
  if (vfInhibitOutput || vfWinGUI)
    return;

  if (!vfQuiet)
    printf("(%d bytes)\n", ibOut);
  
  if (!vfAllowLargeResources && ibOut > maxSafeResourceSize)
    WarningLine("Resource '%s' %d, %d bytes, exceeds safe "
  		"HotSync size limit of %d bytes",
  		outputResStr, outputResID, ibOut, maxSafeResourceSize);

  ibTotalOut += ibOut;
  if (vfhOut != NULL)
  {
    fclose(vfhOut);
    vfhOut = NULL;
  }

  /*
   * #endif 
   */
#endif
}

/*-----------------------------------------------------------------------------
|	getOpenedOutputFile
-------------------------------------------------------------RMa-------------*/
FILE *
getOpenedOutputFile(void)
{
  if (vfhOut == NULL)
    Error("No output file opened");
  return vfhOut;
}

VOID
OpenResFile(const char *sz)
{
  if (vfInhibitOutput || vfWinGUI)
    return;

  if (sz == NULL)
    return;

  vfhRes = fopen(sz, "wt");

  if (vfhRes == NULL)
    Error("Unable to open res file %s: %s", sz, strerror(errno));
  if (!vfQuiet)
    printf("Generating res file: %s\n", sz);
}

VOID
CloseResFile(void)
{
  if (vfInhibitOutput || vfWinGUI)
    return;

  if (vfhRes != NULL)
  {
    fclose(vfhRes);
    vfhRes = NULL;
  }
}

/*-----------------------------------------------------------------------------
|	FindAndOpenFile
-------------------------------------------------------------DAVE------------*/
char *
FindAndOpenFile(const char *szIn,
                const char *mode,
                FILE ** returnFile)
{
  char *szFullName = NULL;

  if ((*returnFile = fopen(szIn, mode)) != NULL)
  {
    szFullName = MakeFilename("%s", szIn);
  }
  else
  {
    int i;
    for (i = 0; i < totalIncludePaths; i++)
    {
      szFullName = MakeFilename("%s/%s", includePaths[i], szIn);
      if ((*returnFile = fopen(szFullName, mode)) != NULL)
        break;

      free(szFullName);
    }
  }

  if (*returnFile == NULL)
  {
    ErrorLine("Unable to find %s", szIn);
    return NULL;
  }

  if (vfTrackDepends)
    AddEntryToDependsList(szFullName);

  return szFullName;
}

/*
 * case insenstitive string comparison  
 */
BOOL
FSzEqI(const char *sz1,
       const char *sz2)
{
  while (tolower(*sz1) == tolower(*sz2))
  {
    if (*sz1 == 0)
      return fTrue;
    sz1++;
    sz2++;
  }
  return fFalse;
}

int
WMin(int w1,
     int w2)
{
  return w1 < w2 ? w1 : w2;
}

int
WMax(int w1,
     int w2)
{
  return w1 > w2 ? w1 : w2;
}

/*-----------------------------------------------------------------------------
|      intstrncpy
-------------------------------------------------------------JOHN------------*/
VOID
intstrncpy(p_int * dst,
           const char *src,
           int n)
{
  while (n > 0)
  {
    n--;
    if ((*dst++ = *src++) == 0)
      break;
  }

  while (n > 0)
  {
    n--;
    *dst++ = 0;
  }
}

/*-----------------------------------------------------------------------------
|      WriteOutResourceDB
-------------------------------------------------------------JOHN------------*/

typedef struct DBHEADER
{
  p_int name[32];                                /* b32 */
  p_int attr;                                    /* w */
  p_int version;                                 /* w */
  p_int created;                                 /* l */
  p_int modified;                                /* l */
  //p_int backup;      /* zl */
  //p_int modnum;      /* zl */
  //p_int appinfo;     /* zl */
  //p_int sortinfo;    /* zl */
  p_int type[4];                                 /* b4 */
  p_int creator[4];                              /* b4 */
  //p_int uidseed;     /* zl */
  //p_int nextlist;    /* zl */
  p_int nrecords;                                /* w */
} DBHEADER;

#define szDBHEADER "b32,w,w,l,l,zl4,b4,b4,zl2,w"

static VOID
WriteOutResourceDB(void)
{
  DBHEADER head;
  char buf[4096];
  int head_offset;
  FILE *f;
  int i;
  size_t n;
  BOOL saveLE32;

  /*
   * Even resources with LE32 contents go into a M68K-style PRC file.  
   */
  saveLE32 = vfLE32;
  vfLE32 = fFalse;

  /*
   * It was intended not to provide facilities to set the name, attributes,
   * type, creator, timestamps, etc because that's not really PilRC's job.
   * We're not generating fully flexible .prc files, we're really just using
   * the PRC format as an archive format.  But in the end we've provided
   * command line options for name, type, and creator.  Apparently it is
   * too difficult to use this:  :-)
   * pilrc -prc foo.rcp
   * build-prc -n NAME -t TYPE -c CRID foo.ro
   * 
   * The random number for the timestamps corresponds to 1996-05-16
   * 11:14:40, which is the same fixed number emitted by build-prc from
   * prc-tools 0.5.0.  A little bit of history lives on.  :-)
   * 
   * We output a constant time because it doesn't seem to be worth getting
   * this right for a temporary file which won't be distributed, because
   * it's non-trivial to output the correct time in a portable way, and
   * especially because variable timestamps embedded in object files are
   * the spawn of the devil: they make it harder to determine when anything
   * has really changed -- cmp always detects differences after a rebuild.
   * This is an issue in certain debugging scenarios that you never want
   * to encounter.  
   */

  intstrncpy(head.name, (vfPrcName) ? vfPrcName : szOutResDBFile, 31);
  head.name[31] = 0;
  head.attr = 1;                                 /* dmHdrAttrResDB */
  head.version = 1;

  // if we have requested to set a timestamp (POSIX only) do so
  if (!vfPrcTimeStamp)
    head.created = head.modified = 0xadc0bea0;
  else
    head.created = head.modified =
      time(0) + (unsigned long)(66L * (365.25252 * 24 * 60 * 60));

  intstrncpy(head.type, (vfPrcType) ? vfPrcType : "RESO", 4);
  intstrncpy(head.creator, (vfPrcCreator) ? vfPrcCreator : "pRES", 4);
  head.nrecords = PlexGetCount(&resdir);

  head_offset = CbEmitStruct(&head, szDBHEADER, NULL, fTrue);
  head_offset += head.nrecords * CbStruct(szRESOURCEDIRENTRY);
  head_offset += 2;                              /* Allow for that daft gap */

  for (i = 0; i < head.nrecords; i++)
  {
    RESOURCEDIRENTRY *e = PlexGetElementAt(&resdir, i);

    e->offset += head_offset;
    CbEmitStruct(e, szRESOURCEDIRENTRY, NULL, fTrue);
  }

  CbEmitStruct(NULL, "zb2", NULL, fTrue);        /* The dreaded gap */

  f = fopen(szTempFile, "rb");
  if (f == NULL)
    Error("Unable to open resource DB %s: %s", szTempFile, strerror(errno));

  while ((n = fread(buf, 1, sizeof buf, f)) > 0)
    DumpBytes(buf, n);

  fclose(f);

  vfLE32 = saveLE32;
}

struct DependsNode
{
    char *name;
    struct DependsNode *next;
};

static struct DependsNode *pdDependsRoot = NULL;

/*-----------------------------------------------------------------------------
|      InitDependsList
-------------------------------------------------------------BLC-------------*/

void InitDependsList(void)
{
    pdDependsRoot = NULL;
}

/*-----------------------------------------------------------------------------
|      AddEntryToDependsList
-------------------------------------------------------------BLC-------------*/

void AddEntryToDependsList(const char *filename)
{
    struct DependsNode *pdNode;
    
    if (pdDependsRoot != NULL)
    {
        // search list for filename
        pdNode = pdDependsRoot;
        while (1)
        {
            if (strcmp(pdNode->name, filename) == 0) return;
            if (pdNode->next == NULL) break;
            pdNode = pdNode->next;
        }
    
        // add node to the end
        pdNode->next = malloc(sizeof(*pdNode));
        pdNode = pdNode->next;
    }
    else
    {
        // start new list
        pdDependsRoot = malloc(sizeof(*pdNode));
        pdNode = pdDependsRoot;
    }

    // fill in new node
    pdNode->next = NULL;
    pdNode->name = malloc(strlen(filename) + 1);
    strcpy(pdNode->name, filename);
}

/*-----------------------------------------------------------------------------
|      EscapeChars
-------------------------------------------------------------BLC-------------*/

static char *EscapeChars(char *dest, const char *src)
{
    char *origDest = dest;
    
    for (; *src; ++src)
    {
        if (strchr(" \\\"\'", *src))
        {
            *dest++ = '\\';
        }
        *dest++ = *src;
    }
	/* NULL terminate destination */
	*dest = 0;

    return origDest;
}

/*-----------------------------------------------------------------------------
|      OutputDependsList
-------------------------------------------------------------BLC-------------*/

void OutputDependsList(FILE *dependsFile, const char *szDependsTarget)
{
    char buffer[512];
    struct DependsNode *pdNode;

    fprintf(dependsFile, "%s:", EscapeChars(buffer, szDependsTarget));

    pdNode = pdDependsRoot;
    while (pdNode)
    {
        fprintf(dependsFile, " \\\n %s", EscapeChars(buffer, pdNode->name));
        pdNode = pdNode->next;
    }
        
    // terminate line
    fprintf(dependsFile, "\n");
}

/*-----------------------------------------------------------------------------
|      FreeDependsList
-------------------------------------------------------------BLC-------------*/

void FreeDependsList(void)
{
    while (pdDependsRoot)
    {
        struct DependsNode *pdNode = pdDependsRoot;
        pdDependsRoot = pdDependsRoot->next;
        free(pdNode->name);
        free(pdNode);
    }
}
