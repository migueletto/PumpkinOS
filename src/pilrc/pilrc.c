/*
 * @(#)pilrc.c
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
 *     23-Jun-2000 Mark Garlanger
 *                 Additions to support #ifdef/#ifndef/#else/#endif in 
 *                 both *.rcp files and *.h files. 
 *     24-Jun-2000 Kurt Spaugh
 *                 Additions for tSTL resource support
 *     27-Jul-2000 Michal Kouril
 *                 Additions for slider control resource support
 *     20-Nov-2000 Renaud Malaval
 *                 Additions for PalmOS 3.5 support
 *     30-Nov-2000 Renaud Malaval
 *                 Debug tSTL resource support in LE32 generation
 *     05-Dec-2000 Renaud Malaval
 *                 Remove padding in ParseDumpApplicationIconName() (taic)
 *                 Force padding to 16bpp in ParseDumpLauncherCategory() (tAIN)
 *     30-Dec-2000 Ben Combee
 *                 Added 'tint' resource support
 *     Jan-2001    Regis Nicolas
 *                 Merged 68K and LE32 version into one binary
 *     12-Jan-2001 Renaud Malaval
 *                 Added 'wrdl' resource support
 *     19-Jan-2001 Renaud Malaval
 *                 Added 'tclt' resource support
 *     02-Fev-2001 Renaud Malaval
 *                 Added 'silk' resource support
 *     14-Fev-2001 Renaud Malaval
 *                 Added 'cnty' resource support
 *     16-Fev-2001 Renaud Malaval
 *                 Added 'feat' resource support
 *     23-Fev-2001 Renaud Malaval
 *                 Added 'tkbd' resource support
 *     02-Mar-2001 Renaud Malaval
 *                 Added 'DLST' & 'BLST' resource support
 *     09-Mar-2001 Renaud Malaval
 *                 Added 'MIDI' resource support
 *     12-Mar-2001 Renaud Malaval
 *                 Added 'Tbsb' resource support
 *     20-Jun-2001 Renaud Malaval
 *                 Added 'locs' resource support
 *     26-Jun-2001 Renaud Malaval
 *                 Added 'hsbd' resource support
 *     13-Jui-2001 Renaud Malaval
 *                 Added 'pref' resource support
 *     23-Jui-2001 Renaud Malaval
 *                 Added localisation support
 *     31-Aug-2001 Laurent Dutour
 *                 Re-Added recursive include support
 *     08-Sep-2001 Renaud Malaval
 *                 Added 'tSCH' resource support
 *     04-Aug-2002 Michael McLagen
 *                 Fixed BITMAPID/SELECTEDBITMAPID warnings
 *     06-Aug-2002 Renaud Malaval
 *                 Added 'nfnt' resource support
 *     22-Sep-2002 Ben Combee
 *                 Allow zero-item lists in forms
 *                 Fix transparency handling for high density bitmaps
 *                 Allow LABELs to have strings that span lines
 *      1-Oct-2002 Ben Combee
 *                 Patch from Joshua Buergel to not warn when using
 *                 horizonal scroll bars at recommended height.
 *      5-Nov-2002 Ben Combee
 *                 Altered MENU SEPARATOR to allow specifying ID or AUTOID
 *     22-Mar-2003 Bob Withers
 *                 Added support for #define strings which can be used in
 *                 place of string constants.
 *                 Added support for preprocessor directives within MENU,
 *                 FORM, and ALERT defintions.
 *     25-Sep-2003 John Marshall
 *                 Added support for Handspring's BITMAP syntax to the
 *                 unified bitmap parser.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifndef strdup
char *strdup(const char *s);
/*lint -sem( strdup, @p == (1p ? malloc(1p) : 0) )*/
#endif

#define EMITRWT
#include "pilrc.h"
#undef EMITRWT
#include "bitmap.h"
#include "restype.h"                             // RMa

#ifdef PALM_INTERNAL
#include "PalmRC.h"
#endif

#define idDefaultAutoInit 9999
#define idDefaultDirection -1
#define idPalmOSReservedMin 10000
#define idPalmOSReservedMinWithEditIDs 10008

#define szMultipleLineMaxLength 65536


static void ParseDirectives(RCPFILE * prcpfile);


/*-----------------------------------------------------------------------------
|	 Globals
-------------------------------------------------------------WESC------------*/

/*
 * Are we PilRCUI -- this was an attempt to merge pilrc and pilrcui 
 */
BOOL vfWinGUI;

/*
 * Variables to support #ifdef/#else/#endif 
 */
int ifdefSkipping = 0;
int ifdefLevel = 0;

/*
 * No default flag for form and object form 
 */
BOOL vfPalmRez;

/*
 * Allow edit ID's (10000-10007 inclusive)
 */
BOOL vfAllowEditIDs;

/*
 * Allow icons that aren't properly sized
 */
BOOL vfAllowBadIconSizes;

/*
 * Allow resources to exceed 64000 bytes
 */
BOOL vfAllowLargeResources;

/*
 * Disables ellipsis processing
 */
BOOL vfNoEllipsis;

/*
 * Quiet output 
 */
BOOL vfQuiet;

/*
 * M$ (VS-type) error/warning output 
 */
BOOL vfVSErrors;

/*
 * Translations 
 */
int totalLanguages = 0;
const char *aszLanguage[MAXLANG] = { NULL };
TE *pteFirst;

/*
 * RMa Localisation management
 */
char *szLocaleP;

/*
 * RMa Localisation management
 *  Don't compile non localisable resources
 */
BOOL vfStripNoLocRes;

/*
 * RTL For Hebrew
 */
BOOL vfRTL = fFalse;

/*
 * next auto id 
 */
BOOL vfAutoId = fFalse;
int vfAutoStartID = idDefaultAutoInit;
int idAutoMac = idDefaultAutoInit;
int idAutoDirection = idDefaultDirection;

/*
 * output header file support
 */
static char *szOutputHeaderFile = NULL;

/*
 * Warning about duplicates in forms and menus 
 */

/*
 * if == 2 then we warn about duplicate labels too 
 */
BOOL vfCheckDupes;

/*
 * Form globals 
 */
FRM *vpfrm;

//RCFORM form;  /* current form being parsed */
//#define cobjMax 256
//RCFORMOBJLIST rglt[cobjMax];

/*
 * RMa Little or Big indian support 
 */
BOOL vfLE32 = fFalse;

BOOL vfAppIcon68K = fFalse;

BOOL vfAutoAmdc = fFalse;
char *szDllNameP;

/*
 * LDu Output a Prc File
 */
BOOL vfPrc = fFalse;
const char *vfPrcName;
const char *vfPrcCreator;
const char *vfPrcType;
BOOL vfPrcTimeStamp = fFalse;

/*
 * LDu Ignore Include File In Header Files
 */
BOOL vfIFIH = fFalse;

/*
 * Track dependencies
 */
BOOL vfTrackDepends = fFalse;
BOOL vfInhibitOutput = fFalse;

/*
 * Menu globals 
 */
RCMENUBAR menu;

#define imiMax 128
RCMENUITEM rgmi[imiMax];

#define impdMax 32
RCMENUPULLDOWN rgmpd[impdMax];
int imiMac;
int idMenu;

#define iidMenuMax 128
int iidMenuMac;
int rgidMenu[iidMenuMax];

#define iidStringMax 512
int iidStringMac;
int rgidString[iidStringMax];

#define iidStringTableMax 384                    // RMa add for make difference between tSTR and tSTL string
int iidStringTableMac;                           // RMa add for make difference between tSTR and tSTL string
int rgidStringTable[iidStringTableMax];          // RMa add for make difference between tSTR and tSTL string

#define iidAlertMax 512
int iidAlertMac;
int rgidAlert[iidAlertMax];

#define iidAISMax 128
int iidAISMac;
int rgidAIS[iidAISMax];

/*
 * Symbol table 
 */
SYM *psymFirst;

/*
 * Parse globals 
 */
TOK tok;
INPUTCONTEXT vIn;

/*
 * Rect for Prev* keywords 
 */
RCRECT rcPrev;

/*-----------------------------------------------------------------------------
|	AddSym
|	
|		Add symbol with value wVal to symbol table
|
|		Note! no check is made for previous existence
-------------------------------------------------------------WESC------------*/
VOID
AddSym(const char *sz,
       int wVal)
{
  SYM *psym;

  psym = calloc(1, sizeof(SYM));
  if (psym == NULL) Error("out of memory");
  psym->sz = strdup(sz);
  psym->wVal = wVal;
  psym->sVal = NULL;
  psym->psymNext = psymFirst;
  psymFirst = psym;
}

VOID AddSymString(const char* sz, const char* val)
{
    auto     SYM*       psym;
    
    psym = calloc(1, sizeof(SYM));
    if (psym == NULL) Error("out of memory");
    psym->sz = strdup(sz);
    psym->wVal = 0;
    psym->sVal = strdup(val);
    psym->psymNext = psymFirst;
    psymFirst = psym;
}

static VOID FreeSym(SYM *psym)
{
  free(psym->sz);
  free(psym->sVal);
  free(psym);
}

VOID RemoveSym(const char *sz)
{
  SYM *psym, *next;
  SYM **prevNext = &psymFirst;

  for (psym = psymFirst; psym != NULL; psym = next)
  {
    next = psym->psymNext;

    if (strcmp(psym->sz, sz) == 0)
    {
      *prevNext = next;
      FreeSym(psym);
    }
    else
      prevNext = &(psym->psymNext);
  }
}

/*-----------------------------------------------------------------------------
|	PsymLookup
|	
|		Lookup symbol based on sz -- case sensitive
-------------------------------------------------------------WESC------------*/
static SYM *
PsymLookup(const char *sz)
{
  SYM *psym;

  for (psym = psymFirst; psym != NULL; psym = psym->psymNext)
    if (strcmp(psym->sz, sz) == 0)
      return psym;
  return NULL;
}

static VOID
CheckNumericSymbol(const SYM* psym)
{
  if (NULL != psym->sVal)
    ErrorLine("Symbol %s should be numeric ID but is defined as string %s",
              psym->sz, psym->sVal);
}

static int
IdGetAutoId()
{
  idAutoMac += idAutoDirection;
  return idAutoMac;
}

static SYM *
PsymAddSymAutoId(const char *sz)
{
  SYM *psym;

  AddSym(sz, IdGetAutoId());
  psym = PsymLookup(tok.lex.szId);
  psym->fAutoId = fTrue;
  return psym;
}

/*-----------------------------------------------------------------------------
|	FreeSymTable
|	
|	Free up all memory allocated for the symbol table
-------------------------------------------------------------WESC------------*/
static VOID
FreeSymTable()
{
    SYM* psym;
    SYM* psymNext;
 
    for (psym = psymFirst; psym != NULL; psym = psymNext)
    {
        psymNext = psym->psymNext;
        FreeSym(psym);
    }
     
    psymFirst = NULL;
}

/*-----------------------------------------------------------------------------
|	PchFromRw
|	
|		Maps a reserved word into a string
-------------------------------------------------------------WESC------------*/
static const char *
PchFromRw(RW rw,
          BOOL fTrySecondName)
{
  RWT *prwt;

  prwt = rgrwt;
  while (prwt->sz1 != NULL)
  {
    if (prwt->rw == rw)
    {
      if (fTrySecondName && prwt->sz2 != NULL)
        return prwt->sz2;
      return prwt->sz1;
    }
    prwt++;
  }
  return NULL;
}

/*-----------------------------------------------------------------------------
|	RwFromLex
|	
|		Looks up lex.id in reserved word table.  returns rwNil if not found
-------------------------------------------------------------WESC------------*/
static RW
RwFromLex(const LEX * plex)
{
  RWT *prwt;

  if (plex->lt != ltId)
    return rwNil;
  prwt = rgrwt;
  while (prwt->sz1 != NULL)
  {
    if (FSzEqI(plex->szId, prwt->sz1)
        || (prwt->sz2 != NULL && FSzEqI(plex->szId, prwt->sz2)))
      return prwt->rw;
    prwt++;
  }
  return rwNil;
}

/*-----------------------------------------------------------------------------
|	PteFromSz
|	
|		Look up a language tranlation entry. All strings in the .rcp file
|	potentially get translated
-------------------------------------------------------------WESC------------*/
static TE *
PteFromSz(const char *sz)
{
  TE *pte;

  for (pte = pteFirst; pte != NULL; pte = pte->pteNext)
    if (strcmp(pte->szOrig, sz) == 0)
      return pte;
  return NULL;
}

/*-----------------------------------------------------------------------------
|	FreeTranslations
|	
|		Free up the translation table
-------------------------------------------------------------WESC------------*/
static VOID
FreeTranslations()
{
  TE *pte;
  TE *pteNext;

  for (pte = pteFirst; pte != NULL; pte = pteNext)
  {
    pteNext = pte->pteNext;
    free(pte->szOrig);
    free(pte->szTrans);
    free(pte);
  }
  pteFirst = NULL;
}

/*-----------------------------------------------------------------------------
|	NextLine
|
|		Skip lexer ahead to the next line
-------------------------------------------------------------WESC------------*/
static BOOL
NextLine(void)
{
  BOOL fEof;
  char *pchNext;
  char *eoln = strpbrk(vIn.pch, "\n\x0a\x0d");

  if (eoln == NULL)
  {
    int buflen = strlen(vIn.pch);
    memmove(vIn.buffer, vIn.pch, buflen + 1);
    vIn.pch = vIn.buffer;
    fgets(&vIn.buffer[buflen], sizeof vIn.buffer - buflen, vIn.file.fh);

    eoln = strpbrk(vIn.pch, "\n\x0a\x0d");
    if (eoln == NULL)
      eoln = strrchr(vIn.pch, '\0');
  }

  if (eoln[0] == '\x0d' && eoln[1] == '\x0a')
    pchNext = eoln + 2;
  else if (*eoln == '\0')
    pchNext = eoln;
  else
    pchNext = eoln + 1;

  fEof = (*vIn.pch == '\0');
  *eoln = '\0';

  if (!fEof)
    vIn.file.line++;

  FInitLexer(vIn.pch, fTrue);
  vIn.pch = pchNext;
  return !fEof;
}

/*-----------------------------------------------------------------------------
|	FGetTok
|
|		Get the next token.
|		Returns fFalse, .lex.lt=ltNil, and .rw=rwNil on EOF.
|
|	Consistency issue -- takes a ptok, but some other other routines don't.
|	only one global tok...
-------------------------------------------------------------WESC------------*/
BOOL
FGetTok(TOK *ptok)
{
  BOOL fInComment;

  if (vIn.fPendingTok)
  {
    *ptok = vIn.pendingTok;
    vIn.fPendingTok = fFalse;
    return ptok->lex.lt != ltNil;
  }

  ptok->rw = rwNil;
  fInComment = fFalse;
  for (;;)
  {
    while (!FGetLex(&ptok->lex, fInComment))
    {
      if (!NextLine())
      {
        if (fInComment)
          ErrorLine("unexpected end of file during C-style comment");
        ptok->lex.lt = ltNil;
        vIn.pendingTok = *ptok;
        return fFalse;
      }
    }

    if (ptok->lex.lt == ltCComment)
      fInComment = fTrue;
    else if (ptok->lex.lt == ltEndCComment)
      fInComment = fFalse;
    else if (ptok->lex.lt == ltDoubleSlash)
      NextLine();
    else
      break;
  }

  if (ptok->lex.lt == ltId)
  {
    /*
     * check if it is a reserved word 
     */
    ptok->rw = RwFromLex(&ptok->lex);
  }
  else if (ptok->lex.lt == ltStr)
  {
    TE *pte;

    /*
     * attempt translation 
     */
    pte = PteFromSz(ptok->lex.szId);
    if (pte != NULL)
      strcpy(ptok->lex.szId, pte->szTrans);
  }

  vIn.pendingTok = *ptok;
  return fTrue;
}

/*-----------------------------------------------------------------------------
|	UngetTok
|	
|		Pushback one token.  Note! that this is 1 level only!
-------------------------------------------------------------WESC------------*/
VOID
UngetTok(void)
{
  vIn.fPendingTok = fTrue;
}

/*-----------------------------------------------------------------------------
|	PeekTok
-------------------------------------------------------------JohnM-----------*/
const TOK *
PeekTok(void)
{
  FGetTok(&tok);
  UngetTok();
  return &tok;
}

/*-----------------------------------------------------------------------------
|	GetExpectLt
|	
|		Get a token and expect a particular lex type.
|		Emit szErr if it isn't what's expected.
|		DO NOT use with lt==ltStr; use PchGetString() instead.
-------------------------------------------------------------WESC------------*/
static VOID
GetExpectLt(TOK * ptok,
            LT lt,
            char *szErr)
{
  FGetTok(ptok);
  if (ptok->lex.lt != lt)
  {
    if (szErr == NULL)
    {
      if (lt == ltId)
        ErrorLine("Syntax error : expecting identifier, got %s",ptok->lex.szId);
      else if (lt == ltConst)
        ErrorLine("Syntax error : expecting constant, got %s", ptok->lex.szId);
      else
        ErrorLine("syntax error: %s", ptok->lex.szId);
    }
    else
    {
      ErrorLine("expecting: %s, got %s", szErr, ptok->lex.szId);
    }
  }
}

/*-----------------------------------------------------------------------------
|	GetExpectRw
|	
|		Get and expect a particular  reserved word.  Emit "Expecting..." if next
|	token isn't rw
-------------------------------------------------------------WESC------------*/
VOID
GetExpectRw(RW rw)
{
  TOK tok;

  FGetTok(&tok);
  if (tok.rw != rw)
    ErrorLine("%s expected, got %s", PchFromRw(rw, fTrue), tok.lex.szId);
}

/*-----------------------------------------------------------------------------
|	FIsString
|
|		Returns true if the token could be the first token of
|		a (possibly concatenated) string.
-------------------------------------------------------------JohnM-----------*/
BOOL
FIsString(const TOK *ptok)
{
  if (ptok->lex.lt == ltStr)
    return fTrue;
  else if (ptok->lex.lt == ltId && ptok->rw == rwNil)
  {
    const SYM *psym = PsymLookup(ptok->lex.szId);
    return psym && psym->sVal;
  }
  else
    return fFalse;
}

/*-----------------------------------------------------------------------------
|	PchGetString
|
|		Get a string (which should be free()ed by the caller),
|		which may include concatenation and/or string #defines.
-------------------------------------------------------------JohnM-----------*/
char *
PchGetString(const char *szErr)
{
  char *szOut = NULL;
  size_t outlen = 0;

  for (;;)
  {
    const char *szSingle = NULL;
    const TOK *ptok;

    if (!FGetTok(&tok))
      ErrorLine("end of file %s %s", szOut? "in" : "when expecting", szErr);
    else if (tok.lex.lt == ltStr)
      szSingle = tok.lex.szId;
    else if (tok.lex.lt == ltId && tok.rw == rwNil)
    {
      const SYM *psym = PsymLookup(tok.lex.szId);
      if (psym == NULL)
        ErrorLine("undefined symbol '%s' in %s", tok.lex.szId, szErr);
      else if (psym->sVal == NULL)
        ErrorLine("numeric symbol '%s' in %s", tok.lex.szId, szErr);
      else
        szSingle = psym->sVal;
    }
    else
    {
      if (szOut)
        ErrorLine("cannot concatenate '%s' onto %s", tok.lex.szId, szErr);
      else
        ErrorLine("expecting %s, got '%s'", szErr, tok.lex.szId);
    }

    if (szSingle)
    {
      size_t singlelen = strlen(szSingle);
      char *sz = realloc(szOut, outlen + singlelen + 1);
      if (sz == NULL)
        Error("String is larger than available memory");
      else
        szOut = sz;

      strcpy(szOut + outlen, szSingle);
      outlen += singlelen;
    }

    ptok = PeekTok();
    if (ptok->lex.lt == ltPlus || ptok->lex.lt == ltBSlash)
      FGetTok(&tok);
    else
      break;
  }

  return szOut;
}

/*-----------------------------------------------------------------------------
|	WGetConst
|	
|		Get an integer constant or one of the Prev* reserved words, returning
|	valu	e.
|		
-------------------------------------------------------------WESC------------*/
int
WGetConst(char *szErr)
{
  if (!FGetTok(&tok))
    ErrorLine("unexpected end of file");
  /*lint -e{788}*/
  switch (tok.rw)
  {
    default:
      if (tok.lex.lt == ltId)
      {
        SYM *psym;

        psym = PsymLookup(tok.lex.szId);
        if (psym)
        {
          CheckNumericSymbol(psym);
        }
        else
        {
          if (vfAutoId)
          {
            psym = PsymAddSymAutoId(tok.lex.szId);
          }
          else
          {
            ErrorLine("Expecting %s, got unknown symbol: %s",
                      szErr, tok.lex.szId);
          }
        }
        return psym->wVal;
      }
      if (tok.lex.lt != ltConst)
      {
        ErrorLine("%s expected, got %s", szErr, tok.lex.szId);
      }
      return tok.lex.val;
    case rwPrevLeft:
      return rcPrev.topLeft.x;
    case rwPrevRight:
      return rcPrev.topLeft.x + rcPrev.extent.x;
    case rwPrevWidth:
      return rcPrev.extent.x;
    case rwPrevTop:
      return rcPrev.topLeft.y;
    case rwPrevBottom:
      return rcPrev.topLeft.y + rcPrev.extent.y;
    case rwPrevHeight:
      return rcPrev.extent.y;
  }
}

/*-----------------------------------------------------------------------------
| WGetConstExFactor
| 
| Get a constant expression -- parens allowed left to right associativity
-------------------------------------------------------------WESC------------*/
static int
WGetConstExFactor(char *szErr)
{
  int wVal;

  if (FGetTok(&tok))
  {
    if (tok.lex.lt == ltLParen)
    {
      wVal = WGetConstEx(szErr);
      if (!FGetTok(&tok) || tok.lex.lt != ltRParen)
        ErrorLine("Expected but didn't get ')'!");
    }
    else if (tok.lex.lt == ltMinus)              /* RMa add support for negative value */
    {
      wVal = 0 - WGetConstEx(szErr);
    }                                            /* RMa add support for negative value */
    else
    {
      UngetTok();
      wVal = WGetConst(szErr);
    }
  }
  else
  {
    wVal = WGetConst(szErr);                     /* aka hack */
  }

  return wVal;
}

/*-----------------------------------------------------------------------------
| WGetConstEx
| 
| Get a constant expression -- parens allowed left to right associativity
-------------------------------------------------------------WESC------------*/
int
WGetConstEx(char *szErr)
{
  int wValT;
  int wVal;
  LT ltOp;

  wVal = WGetConstExFactor(szErr);

  for (;;)
  {
    if (!FGetTok(&tok))
      return wVal;

    switch (tok.lex.lt)
    {
      default:
        UngetTok();
        return wVal;

      case ltPlus:
      case ltMinus:
      case ltMult:
      case ltDiv:
      case ltPipe:
        ltOp = tok.lex.lt;
        wValT = WGetConstExFactor(szErr);
        switch (ltOp)
        {
          case ltPlus:
            wVal += wValT;
            break;
          case ltMinus:
            wVal -= wValT;
            break;
          case ltMult:
            wVal *= wValT;
            break;
          case ltDiv:
            if (wValT == 0)
              ErrorLine("Divide By Zero!");
            wVal /= wValT;
            break;
          case ltPipe:
            if (vfRTL)
              wVal = wValT;
            break;
        }
    }
  }
}

VOID
AddDefineSymbol(void)
{
    int        wIdVal;
    char       szId[256];
    int        current_line;
    
    GetExpectLt(&tok, ltId, "identifier");
    strcpy(szId, tok.lex.szId);
    if (NULL != PsymLookup(szId))
        ErrorLine("Symbol %s is defined multiple times", szId);
    
    // save current line and check to see if we parse into another
    // line for the value -- if so, reject token and define as 0
    current_line = vIn.file.line;
    
    if (!FGetTok(&tok))
        ErrorLine("unexpected end of file");
    
    if (current_line != vIn.file.line)
    {
    	UngetTok();
    	AddSym(szId, 0);
    }
    else
    {
	    if (FIsString(&tok))
	    {
			// support a string define that spans multiple lines
			char *szText;
			UngetTok();
			szText = PchGetString("#define string");
	        AddSymString(szId, szText);
	        free(szText);
	    }
	    else
	    {
	        UngetTok();
	        wIdVal = WGetConstEx("Constant");
	        AddSym(szId, wIdVal);
	    }
    }
}


/*-----------------------------------------------------------------------------
|	KtGetK
|	
|		Get a Konstant, returning the type.
-------------------------------------------------------------WESC------------*/
static KT
KtGetK(K * pk,
       char *szErr)
{
  if (!FGetTok(&tok))
    ErrorLine("Unexpected end of file");
  /*lint -e{788}*/
  switch (tok.rw)
  {
    default:
      UngetTok();
      pk->kt = ktConst;
      pk->wVal = WGetConstEx(szErr);
      break;
    case rwCenter:
      pk->kt = ktCenter;
      if (FGetTok(&tok))
      {
        if (tok.lex.lt == ltAt)
        {
          pk->wVal = WGetConstEx("CENTER@ position") * 2;       /* we store extent */
          pk->kt = ktCenterAt;
        }
        else
          UngetTok();
      }

      break;
    case rwAuto:
      pk->kt = ktAuto;
      if (FGetTok(&tok))
      {
        if (tok.lex.lt == ltGT)
        {
          pk->wVal = WGetConstEx("AUTO> size");  /* we store min size */
          pk->kt = ktAutoGreaterThan;
        }
        else if (tok.lex.lt == ltGTE)
        {
          pk->wVal = WGetConstEx("AUTO>= size"); /* we store min size */
          pk->kt = ktAutoGreaterThan;
        }
        else
          UngetTok();
      }

      break;
    case rwRight:
      pk->kt = ktRightAt;
      GetExpectLt(&tok, ltAt, "@");
      pk->wVal = WGetConstEx("RIGHT@ position");
      break;
    case rwBottom:
      pk->kt = ktBottomAt;
      GetExpectLt(&tok, ltAt, "@");
      pk->wVal = WGetConstEx("BOTTOM@ position");
      break;
  }
  return pk->kt;
}

/*-----------------------------------------------------------------------------
|	ParseKpt
|	
|		Parse a point (not the AT and () parts).
-------------------------------------------------------------WESC------------*/
static VOID
ParseKpt(KPT * pkpt)
{
  KtGetK(&pkpt->kX, "x pos");
  KtGetK(&pkpt->kY, "y pos");
}

/*-----------------------------------------------------------------------------
|	ParseKrc
|	
|		Parse a rect (not the AT and () parts).
-------------------------------------------------------------WESC------------*/
static VOID
ParseKrc(KRC * pkrc)
{
  KtGetK(&pkrc->kptUpperLeft.kX, "rect left");
  KtGetK(&pkrc->kptUpperLeft.kY, "rect top");
  KtGetK(&pkrc->kptExtent.kX, "rect width");
  KtGetK(&pkrc->kptExtent.kY, "rect height");
}

/*---------------------------------------------------------------------------
| ParsePaletteFile
|
|
------------------------------------------------------------CD----------------*/
static void
ParsePaletteFile(char *pchFileName,
                 int p[256][3],
                 int *nColors)
{
  FILE *fh;
  int i = 0;
  int r, g, b;

  free(FindAndOpenFile(pchFileName, "rb", &fh));
  if (fh == NULL)
    ErrorLine("Unable to open palette file %s", pchFileName);

  // Start parse of file

  while (EOF != fscanf(fh, "%d %d %d", &r, &g, &b))
  {
	if (r > 255)
	{
		ErrorLine("Red color component must be BYTE (0..255)");
	}
	if (g > 255)
	{
		ErrorLine("Green color component must be BYTE (0..255)");
	}
	if (b > 255)
	{
		ErrorLine("Blue color component must be BYTE (0..255)");
	}

	p[i][0] = r;
	p[i][1] = g;
	p[i][2] = b;
	i++;

	if (i == 257)
	{
		break;
	}
  }

  fclose(fh);
  *nColors = i;
}

 /*
  * Semi-arbitrary margins 
  */
#define dxObjSmallMargin 3
#define dxObjBigMargin 6

/*-----------------------------------------------------------------------------
|	WResolveK
|	
|		Resolve a Konstant to it's real value, returning it.
-------------------------------------------------------------WESC------------*/
static int
WResolveK(const K * pk,
          const ITM * pitm,
          int dxyExtent,
          BOOL fHoriz)
{
  int wVal;
  int dxyCenterAcross;

  switch (pk->kt)
  {
    default:
      Assert(fFalse);
      return 0;
    case ktConst:
      return pk->wVal;
    case ktAuto:
    case ktAutoGreaterThan:
      
      if (fHoriz)
      {
        if (pitm->grif2 & if2GSI)
        {
        	wVal = kGsiWidth;
        }
        /*
         * added 2.8p8 - "auto" should give longest width, not first 
         */
        else if (pitm->grif & ifMultText)
        {
          char *text = pitm->text;

          wVal = 0;
          while (text < pitm->text + pitm->cbText)
          {
            int extlen = DxCalcExtent((unsigned char *)text, pitm->font);

            text += strlen(text) + 1;
            if (extlen > wVal)
              wVal = extlen;
          }
        }
        else
        {
          wVal = DxCalcExtent((unsigned char *)pitm->text, pitm->font);
        }

        if (pitm->grif & ifSmallMargin)
          wVal += 2 * dxObjSmallMargin;
        if (pitm->grif & ifBigMargin)
          wVal += 2 * dxObjBigMargin;
      }
      else
      {
        if (pitm->grif2 & if2GSI)
          wVal = kGsiHeight;
        else
          wVal = DyFont(pitm->font) + 1;
      }
      
      if (pk->kt == ktAuto)
        return wVal;
      else /* pk->kt == ktAutoGreaterThan */
        return (pk->wVal > wVal) ? pk->wVal : wVal;

    case ktCenter:
      if (fHoriz)
        dxyCenterAcross = PBAFIELD(vpfrm, form.window.windowBounds.extent.x);
      else
        dxyCenterAcross = PBAFIELD(vpfrm, form.window.windowBounds.extent.y);
      wVal = (dxyCenterAcross - dxyExtent) / 2;
      return wVal;
    case ktCenterAt:
      dxyCenterAcross = pk->wVal;
      wVal = (dxyCenterAcross - dxyExtent) / 2;
      return wVal;
    case ktRightAt:
    case ktBottomAt:
      wVal = pk->wVal - dxyExtent;
      return wVal;
  }
  Assert(fFalse);
}

/*-----------------------------------------------------------------------------
|	ResolveKrcKpt
|	
|		Resolve a Krc and/or Kpt, setting rcPrev
-------------------------------------------------------------WESC------------*/
static VOID
ResolveKrcKpt(ITM * pitm)
{
  if (pitm->grif & ifRc)
  {
    pitm->rc.extent.x = WResolveK(&pitm->krc.kptExtent.kX, pitm, 0, fTrue);
    pitm->rc.extent.y = WResolveK(&pitm->krc.kptExtent.kY, pitm, 0, fFalse);

    pitm->rc.topLeft.x =
      WResolveK(&pitm->krc.kptUpperLeft.kX, pitm, pitm->rc.extent.x, fTrue);
    pitm->rc.topLeft.y =
      WResolveK(&pitm->krc.kptUpperLeft.kY, pitm, pitm->rc.extent.y, fFalse);
    rcPrev = pitm->rc;
  }

  if (pitm->grif & ifPt)
  {
    K kT;
    int xT;
    int yT;

    kT.kt = ktAuto;
    xT = WResolveK(&kT, pitm, 0, fTrue);
    yT = WResolveK(&kT, pitm, 0, fFalse);

    pitm->pt.x = WResolveK(&pitm->kpt.kX, pitm, xT, fTrue);
    pitm->pt.y = WResolveK(&pitm->kpt.kY, pitm, yT, fFalse);
    rcPrev.topLeft = pitm->pt;
    rcPrev.extent.x = xT;
    rcPrev.extent.y = yT;
  }
}

/*-----------------------------------------------------------------------------
|	DoCheckGrif
|	
| Check if ifP is set in grif -- error if not
-------------------------------------------------------------WESC------------*/
static VOID
DoCheckGrif(int grif,
            int ifP)
{
  if (!(grif & ifP))
  {
    ErrorLine("%s unexpected", tok.lex.szId);
  }
}

/*-----------------------------------------------------------------------------
|	WGetId
|	
|		Parse ID <const>
-------------------------------------------------------------WESC------------*/
int
WGetId(char *szErr)
{
  int w;

  if (!FGetTok(&tok))
    return 0;
  if (tok.rw == rwAutoId)
    w = IdGetAutoId();
  else
  {
    if (tok.rw != rwId)
      UngetTok();
    w = WGetConstEx(szErr);
  }

  if (vfAllowEditIDs)
  {
    if (w >= idPalmOSReservedMinWithEditIDs)
    {
	  WarningLine("ID conflicts with reserved ID range (0-10007 only)");
    }
  }
  else if (w >= idPalmOSReservedMin)
  {
    WarningLine("ID conflicts with reserved ID range (0-9999 only)");
  }

  return w;
}

#define CheckGrif(ifP)   /*lint -e{717}*/ do {DoCheckGrif(pitm->grif, ifP); pitm->grifOut |= ifP;} while (0);  
#define CheckGrif2(ifP)  /*lint -e{717}*/ do {DoCheckGrif(pitm->grif2, ifP); pitm->grif2Out |= ifP;} while (0);
#define CheckGrif3(ifP)  /*lint -e{717}*/ do {DoCheckGrif(pitm->grif3, ifP); pitm->grif3Out |= ifP;} while (0);
#define CheckGrif4(ifP)  /*lint -e{717}*/ do {DoCheckGrif(pitm->grif4, ifP); pitm->grif4Out |= ifP;} while (0);

/*-----------------------------------------------------------------------------
|	ParseItm
|	
|		Parse an item in a form
|	Arguments:
|		ITM *pitm
|		int grif : valid attributes for this item
|		int grif2: more valid attributes for this item
-------------------------------------------------------------WESC------------*/
void
ParseItm(ITM * pitm,
         int grif,
         int grif2,
         int grif3,
         int grif4)
{
  BOOL fAt;
  int icol;

  memset(pitm, 0, sizeof(ITM));

  pitm->grif = grif;
  pitm->grif2 = grif2;
  pitm->grif3 = grif3;
  pitm->grif4 = grif4;
  /*
   * defaults 
   */
  if (!vfPalmRez)
  {
    pitm->usable = 1;
    pitm->frame = 1;
    pitm->justification = leftAlign;
    pitm->singleLine = 1;
    pitm->editable = 1;
    pitm->saveBehind = 1;
    pitm->leftAnchor = 1;
  }
  pitm->cvis = -1;                               // RNi
  pitm->enabled = 1;                             // RMa Must be a default value no tag in PilRC do do that

  if (grif & ifText)
  {
    pitm->grifOut |= ifText;
    pitm->text = PchGetString("item string");
    pitm->cbText = strlen(pitm->text) + 1;
  }
  if (grif & ifMultText)
  {
    pitm->grifOut |= ifMultText;

    pitm->text = NULL;
    pitm->cbText = 0;

    while (FIsString(PeekTok()))
    {
      char *pchSingle = PchGetString("item string");
      size_t singleSize = strlen(pchSingle) + 1;
      char *pch = realloc(pitm->text, pitm->cbText + singleSize);
      if (pch == NULL)
        ErrorLine("Strings are larger than available memory");
      else
        pitm->text = pch;

      memcpy(pitm->text + pitm->cbText, pchSingle, singleSize);
      pitm->cbText += singleSize;
      pitm->numItems++;

      free(pchSingle);
    }
  }

  if (grif & ifId)
  {
    pitm->grifOut |= ifId;
    pitm->id = WGetId("ItemId");
  }
  if (grif & ifListId)
  {
    pitm->grifOut |= ifListId;
    pitm->listid = WGetConstEx("ListId");
  }

  fAt = fFalse;
  if (grif & (ifRc | ifPt))
  {
    if (!FGetTok(&tok))
      return;
    if (tok.rw == rwAt)
    {
      fAt = fTrue;
      GetExpectLt(&tok, ltLParen, "(");
    }
    else
      UngetTok();
  }
  if (grif & ifRc)
  {
    pitm->grifOut |= ifRc;
    ParseKrc(&pitm->krc);
  }
  if (grif & ifPt)
  {
    pitm->grifOut |= ifPt;
    ParseKpt(&pitm->kpt);
  }
  if (fAt)
    GetExpectLt(&tok, ltRParen, ")");

  while (FGetTok(&tok))
  {
    /*lint -e{788}*/
    switch (tok.rw)
    {
      default:
        ResolveKrcKpt(pitm);
        UngetTok();
        return;
      case rwUsable:
      case rwNonUsable:
        CheckGrif(ifUsable);
        pitm->usable = tok.rw == rwUsable;
        break;
      case rwExtended:                          /* MBr add: starts here */
      case rwNonExtended:
        CheckGrif(ifExtended);
        pitm->extended = tok.rw == rwExtended;
        break;                                   /* MBr add: ends here */
      case rwLeftAnchor:                        /* default? */
      case rwRightAnchor:
        CheckGrif(ifAnchor);
        pitm->leftAnchor = tok.rw == rwLeftAnchor;
        break;
      case rwGroup:
        CheckGrif(ifGroup);
        pitm->group = WGetConstEx("GroupId");
        break;
      case rwFont:
        CheckGrif(ifFont);
        pitm->font = WGetConstEx("FontId");
        break;
      case rwFrame:
        CheckGrif(ifFrame);
        pitm->frame = standardButtonFrame;
        break;
      case rwNoFrame:
        CheckGrif(ifFrame);
        pitm->frame = noButtonFrame;
        break;
      case rwBoldFrame:
        CheckGrif(ifFrame);
        pitm->frame = boldButtonFrame;
        break;
      case rwRectFrame:
        CheckGrif(ifFrame);
        pitm->frame = rectangleButtonFrame;
        break;
      case rwDisabled:
      case rwEnabled:
        CheckGrif(ifEnabled);
        pitm->enabled = tok.rw == rwEnabled;
        break;
      case rwChecked:
        CheckGrif(ifOn);
        pitm->on = fTrue;
        break;
      case rwSearch:
        CheckGrif(ifSearch);
        pitm->search = fTrue;
        break;
      case rwEditable:
      case rwNonEditable:
        CheckGrif(ifEditable);
        pitm->editable = tok.rw == rwEditable;
        break;
      case rwSingleLine:
      case rwMultipleLines:
        CheckGrif(ifSingleLine);
        pitm->singleLine = tok.rw == rwSingleLine;
        break;
      case rwVertical:                          /* RMa add */
        CheckGrif2(if2Vertical);
        pitm->vertical = tok.rw == rwVertical;
        break;
      case rwGraphical:                         /* RMa add */
        CheckGrif2(if2Graphical);
        pitm->graphical = tok.rw == rwGraphical;
        break;
      case rwDynamicSize:
        CheckGrif(ifDynamicSize);
        pitm->dynamicSize = 1;
        break;
      case rwMaxChars:
        CheckGrif(ifMaxChars);
        pitm->maxChars = WGetConstEx("MaxChars");
        break;
      case rwLeftAlign:                         /* default? */
        CheckGrif(ifAlign);
        pitm->justification = leftAlign;
        break;
      case rwRightAlign:
        CheckGrif(ifAlign);
        pitm->justification = rightAlign;
        break;
      case rwUnderlined:
        CheckGrif(ifUnderlined);
        pitm->underlined = 1;                    /* WES! try 2 & 3! */
        break;
      case rwVisibleItems:
        CheckGrif(ifCvis);
        pitm->cvis = WGetConstEx("VisibleItems");
        break;
      case rwValue:
        CheckGrif2(if2Value);
        pitm->value = WGetConstEx("Value");
        break;
      case rwMinValue:
        CheckGrif2(if2MinValue);
        pitm->minValue = WGetConstEx("MinValue");
        break;
      case rwMaxValue:
        CheckGrif2(if2MaxValue);
        pitm->maxValue = WGetConstEx("MaxValue");
        break;
      case rwPageSize:
        CheckGrif2(if2PageSize);
        pitm->pageSize = WGetConstEx("PageSize");
        break;
      case rwFeedback:                          /* RMa add */
        CheckGrif2(if2Feedback);
        pitm->feedback = tok.rw == rwFeedback;
        break;
      case rwThumbID:                           /* RMa add */
        CheckGrif2(if2ThumbID);
        pitm->thumbid = WGetConstEx("ThumbID");
        break;
      case rwBackgroundID:                      /* RMa add */
        CheckGrif2(if2BackgroundID);
        pitm->backgroundid = WGetConstEx("BackgroundID");
        break;
      case rwBitmapID:                          /* RMa add */
        CheckGrif2(if2BitmapID);
        pitm->bitmapid = WGetConstEx("BitmapID");
        break;
      case rwSelectedBitmapID:                  /* RMa add */
        CheckGrif2(if2SelectedBitmapID);
        pitm->selectedbitmapid = WGetConstEx("SelectedBitmapID");
        break;

      case rwBitmap:
        CheckGrif(ifBitmap);
        pitm->rscID = WGetConstEx("BitmapId");
        break;
      case rwModal:
        CheckGrif(ifModal);
        pitm->modal = 1;
        break;
      case rwSaveBehind:
        CheckGrif(ifSaveBehind);
        pitm->saveBehind = 1;
        break;
      case rwNoSaveBehind:
        CheckGrif(ifSaveBehind);
        pitm->saveBehind = 0;
        break;
      case rwHelpId:
        CheckGrif(ifHelpId);
        pitm->helpId = WGetConstEx("HelpId");
        break;
      case rwDefaultBtnId:
        CheckGrif(ifDefaultBtnId);
        pitm->defaultBtnId = WGetConstEx("DefaultButtonId");
        break;
      case rwMenuId:
        CheckGrif(ifDefaultBtnId);
        pitm->menuId = WGetConstEx("MenuId");
        break;
      case rwNumColumns:
        CheckGrif2(if2NumColumns);
        pitm->numColumns = WGetConstEx("NumColumns");
        break;
      case rwNumRows:
        CheckGrif2(if2NumRows);
        pitm->numRows = WGetConstEx("NumRows");
        break;
      case rwColumnWidths:
        CheckGrif2(if2ColumnWidths);
        for (icol = 0; icol < pitm->numColumns; icol++)
          pitm->rgdxcol[icol] = WGetConstEx("ColumnWidth");
        break;
      case rwAutoShift:
        CheckGrif2(if2AutoShift);
        pitm->autoShift = 1;
        break;
      case rwHasScrollBar:
        CheckGrif2(if2Scrollbar);
        pitm->hasScrollBar = 1;
        break;
      case rwNumeric:
        CheckGrif2(if2Numeric);
        pitm->numeric = 1;
        break;
      case rwVersion:                           /* RMa addition */
        CheckGrif3(if3Vers);
        pitm->version = WGetConstEx("version");
        break;
      case rwInformation:                       /* RMa alert */
      case rwConfirmation:
      case rwWarning:
      case rwError:
        CheckGrif3(if3AlertType);
        pitm->AlertType = tok.rw - rwInformation;
        break;
      case rwBitmapDensity:                     /* RMa addition */
        CheckGrif4(if4BitmapExtDensity);
        pitm->density = WGetConstEx("density");
        break;
      case rwInitialState:
        CheckGrif4(if4InitialState);
        pitm->initialState = WGetConstEx("initialstate");
        break;
      case rwInitialObjectID:
        CheckGrif4(if4InitialObjectID);
        pitm->initialObjectID = WGetConstEx("initialobjectid");
        break;
      case rwJumpObjectID:
        CheckGrif4(if4JumpObjectID);
        pitm->jumpObjectID = WGetConstEx("jumpobjectid");
        break;
      case rwBottomLeftObjectID:
        CheckGrif4(if4BottomLeftObjectID);
        pitm->bottomLeftObjectID = WGetConstEx("bottomleftobjectid");
        break;
      case rwAbove:
        CheckGrif4(if4Above);
        pitm->aboveID = WGetConstEx("above");
        break;
      case rwBelow:
        CheckGrif4(if4Below);
        pitm->belowID = WGetConstEx("below");
        break;
      case rwSkip:
        CheckGrif4(if4Skip);
        pitm->skip = fTrue;
        break;
      case rwForceInteraction:
        CheckGrif2(if2ForceInteraction);
        pitm->forceInteraction = fTrue;
        break;
      case rwBigButton:
        CheckGrif4(if4BigButton);
        pitm->bigButton = fTrue;
        break;

#ifdef PALM_INTERNAL
      case rwCreator:                           /* RMa addition */
        CheckGrif3(if3Creator);
        pitm->creator = PchGetString("creator");
        break;
      case rwLanguage:                          /* RMa addition */
        CheckGrif3(if3Language);
        pitm->language = PchGetString("language");
        break;
      case rwCountry:                           /* RMa addition */
        CheckGrif3(if3Country);
        pitm->country = PchGetString("country");
        break;
      case rwScreen:                            /* RMa addition */
      case rwGraffiti:                          /* BUG! other types */
        CheckGrif3(if3areaType);
        pitm->areaType = tok.rw == rwGraffiti;
        break;
      case rwAreaIndex:                         /* RMa addition */
        CheckGrif3(if3areaIndex);
        pitm->areaIndex = WGetConstEx("index");
        break;
      case rwKeyDownChr:                        /* RMa addition */
        CheckGrif3(if3keyDownChr);
        pitm->keyDownChr = WGetConstEx("keydownchr");
        break;
      case rwKeyDownKeyCode:                    /* RMa addition */
        CheckGrif3(if3keyDownKeyCode);
        pitm->keyDownKeyCode = WGetConstEx("keydownkeycode");
        break;
      case rwKeyDownModifiers:                  /* RMa addition */
        CheckGrif3(if3keyDownModifiers);
        pitm->keyDownModifiers = WGetConstEx("keydownmodifiers");
        break;
      case rwNumber:                            /* RMa addition */
        CheckGrif3(if3Number);
        pitm->Number = WGetConstEx("number");
        break;
      case rwLanguages:                         /* RMa addition */
        CheckGrif3(if3Languages);
        pitm->Languages = WGetConstEx("languages");
        break;
      case rwCountrys:                          /* RMa addition */
        CheckGrif3(if3Countrys);
        pitm->Countrys = WGetConstEx("countrys");
        break;
      case rwDefaultItem:
        CheckGrif3(if3DefaultItm);
        pitm->DefaultItem = WGetConstEx("defaultitem");
        break;
      case rwTimeZone:                          /* RMa addition */
        CheckGrif3(if3TimeZone);
        pitm->TimeZone = WGetConstEx("timezone");
        break;
      case rwCountryName:                       /* RMa addition */
        CheckGrif3(if3CountryName);
        pitm->CountryName = PchGetString("countryname");
        break;
      case rwName:                              /* RMa addition */
        CheckGrif3(if3Name);
        pitm->Name = PchGetString("name");
        break;
      case rwDateFormat:                        /* RMa addition */
        CheckGrif3(if3DateFormat);
        pitm->DateFormat = WGetConstEx("dateformat");
        break;
      case rwLongDateFormat:                    /* RMa addition */
        CheckGrif3(if3LongDateFormat);
        pitm->LongDateFormat = WGetConstEx("longdateformat");
        break;
      case rwWeekStartDay:                      /* RMa addition */
        CheckGrif3(if3WeekStartDay);
        pitm->WeekStartDay = WGetConstEx("weekstartday");
        break;
      case rwTimeFormat:                        /* RMa addition */
        CheckGrif3(if3TimeFormat);
        pitm->TimeFormat = WGetConstEx("timeformat");
        break;
      case rwNumberFormat:                      /* RMa addition */
        CheckGrif3(if3NumberFormat);
        pitm->NumberFormat = WGetConstEx("numberformat");
        break;
      case rwCurrencyName:                      /* RMa addition */
        CheckGrif3(if3CurrencyName);
        pitm->CurrencyName = PchGetString("currencyname");
        break;
      case rwCurrencySymbol:                    /* RMa addition */
        CheckGrif3(if3CurrencySymbol);
        pitm->CurrencySymbol = PchGetString("currencysymbol");
        break;
      case rwCurrencyUniqueSymbol:              /* RMa addition */
        CheckGrif3(if3CurrencyUniqueSymbol);
        pitm->CurrencyUniqueSymbol = PchGetString("currencyuniquesymbol");
        break;
      case rwCurrencyDecimalPlaces:             /* RMa addition */
        CheckGrif3(if3CurrencyDecimalPlaces);
        pitm->CurrencyDecimalPlaces = WGetConstEx("currencydecimalplaces");
        break;
      case rwDaylightSavings:                   /* RMa addition */
        CheckGrif3(if3DayLightSaving);
        pitm->DaylightSavings = WGetConstEx("daylightsavings");
        break;
      case rwMinutesWestOfGmt:                  /* RMa addition */
        CheckGrif3(if3MinutesWestOfGmt);
        pitm->MinutesWestOfGmt = WGetConstEx("minuteswestofgmt");
        break;
      case rwMeasurementSystem:                 /* RMa addition */
        CheckGrif3(if3MeasurementSystem);
        pitm->MeasurementSystem = WGetConstEx("measurementsystem");
        break;
      case rwPriority:                          /* RMa addition */
        CheckGrif2(if2Priority);
        pitm->Priority = WGetConstEx("priority");
        break;
      case rwStackSize:                         /* RMa addition */
        CheckGrif3(if3StackSize);
        pitm->StackSize = WGetConstEx("stacksize");
        break;
      case rwMinHeapSpace:                      /* RMa addition */
        CheckGrif3(if3MinHeapSpace);
        pitm->MinHeapSpace = WGetConstEx("minheapspace");
        break;
#endif
      case rwLocale:                            /* RMa addition localisation management */
        CheckGrif3(if3Locale);
        pitm->Locale = PchGetString("locale");
        break;
#ifdef PALM_INTERNAL
      case rwFontType:                          /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4FontType);
        pitm->FontType = WGetConstEx("fonttype");
        break;
      case rwFirstChar:                         /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4firstChar);
        pitm->FirstChar = WGetConstEx("firstchar");
        break;
      case rwLastChar:                          /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4lastChar);
        pitm->LastChar = WGetConstEx("lastchar");
        break;
      case rwmaxWidth:                          /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4maxWidth);
        pitm->maxWidth = WGetConstEx("maxwidth");
        break;
      case rwkernMax:                           /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4kernMax);
        pitm->kernMax = WGetConstEx("kernmax");
        break;
      case rwnDescent:                          /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4nDescent);
        pitm->nDescent = WGetConstEx("ndescent");
        break;
      case rwfRectWidth:                        /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4fRectWidth);
        pitm->fRectWidth = WGetConstEx("frectwidth");
        break;
      case rwfRectHeight:                       /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4fRectHeight);
        pitm->fRectHeight = WGetConstEx("frectheight");
        break;
      case rwOwTLoc:                            /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4owTLoc);
        pitm->owTLoc = WGetConstEx("owtloc");
        break;
      case rwAscent:                            /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4Ascent);
        pitm->Ascent = WGetConstEx("ascent");
        break;
      case rwDescent:                           /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4Descent);
        pitm->Descent = WGetConstEx("descent");
        break;
      case rwLeading:                           /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4Leading);
        pitm->Leading = WGetConstEx("leading");
        break;
      case rwRowWords:                          /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4rowWords);
        pitm->rowWords = WGetConstEx("rowwords");
        break;
      case rwFlag:                              /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4flag);
        pitm->flag = WGetConstEx("flag");
        break;
      case rwState:                             /* RMa 'NFNT' & 'fntm' */
        CheckGrif4(if4state);
        pitm->state = WGetConstEx("state");
        break;
      case rwAutoCompress:                      /* RMa 'ttbl' */
        CheckGrif4(if4compressed);
        pitm->compress = tok.rw == rwAutoCompress;
        break;
      case rwTableType:                         /* RMa 'ttbl' */
        CheckGrif4(if4tableType);
        pitm->tableType = WGetConstEx("tabletype");
        break;
      case rwDefaultOutput:                     /* RMa 'ttbl' */
        CheckGrif4(if4defaultOutput);
        pitm->defaultOutput = WGetConstEx("defaultoutput");
        break;
      case rwNumElementBits:                    /* RMa 'ttbl' */
        CheckGrif4(if4numElementBits);
        pitm->numElementBits = WGetConstEx("numelementbits");
        break;
      case rwNumIndexedDataLenBits:             /* RMa 'ttbl' */
        CheckGrif4(if4numIndexedDataLenBits);
        pitm->numIndexedDataLenBits = WGetConstEx("numindexeddatalenbits");
        break;
      case rwNumResultBits:                     /* RMa 'ttbl' */
        CheckGrif4(if4numResultBits);
        pitm->numResultBits = WGetConstEx("numresultbits");
        break;
      case rwIndexDataOffset:                   /* RMa 'ttbl' */
        CheckGrif4(if4indexDataOffset);
        pitm->indexDataOffset = WGetConstEx("indexdataoffset");
        break;
#endif
    }
  }
}

/*
 * RMA function use to parse a complete tag structure
 *  it is use in Localisation
 */
void
ParseToFinalEnd(void)
{
  int encapsulation = 0;

  while (FGetTok(&tok))
  {
    if (tok.rw == rwBegin)
      encapsulation++;

    if (tok.rw == rwEnd)
    {
      if (encapsulation == 0)
        break;
      else
        encapsulation--;
    }
  }
}

/*-----------------------------------------------------------------------------
|	DesirableLocale
|
|		An object with a non-NULL locale is desired if
|		that particular locale has been selected with -Loc;
|		one without is desired depending only on -StripLoc.
-------------------------------------------------------------JohnM-----------*/
static BOOL
DesirableLocale(const char *objlocale)
{
  if (objlocale)
    return szLocaleP && strcmp(objlocale, szLocaleP) == 0;
  else
    return ! vfStripNoLocRes;
}

#define CondEmitB(b)  /*lint -e{717}*/ do {if (fEmit) EmitB(b);} while (0)
#define CondEmitW(w)  /*lint -e{717}*/ do {if (fEmit) EmitW(w);} while (0)
#define CondEmitL(l)  /*lint -e{717}*/ do {if (fEmit) EmitL(l);} while (0)

/*-----------------------------------------------------------------------------
|	CbEmitStruct
|	
|		Emit a struct (RC*).  It is assumed that the members of the struct
|	are integers or pointers only.  Further it is assumed that pointers and
|	integers are the same size.
|	
|	Arguments:
|		void *pv : struct to emit
|		char *szPic: format picture -- see pilrc.h for format
|		char **ppchText: if szPic contains a 'p' then return that pointer here
|		BOOL fEmit: Actually emit the bytes
|	
|	Returns:
|		size of the structure
-------------------------------------------------------------WESC------------*/
int
CbEmitStruct(void *pv,
             char *szPic,
             char **ppchText,
             BOOL fEmit)
{
  char *pch;
  p_int *pi;
  int ibit;
  unsigned char byte;
  unsigned short word;                           /* RMa add */
  int cb;

  if (ppchText != NULL)
    *ppchText = NULL;
  cb = 0;
  ibit = 0;
  byte = 0;
  word = 0;                                      /* RMa add */
  Assert(sizeof(char *) == sizeof(p_int));
  pi = (p_int *) pv;
  for (pch = szPic; *pch != 0;)
  {
    int ch;
    int c;
    BOOL fZero;

    if (*pch == ',')
    {
      pch++;
      continue;
    }
    fZero = *pch == 'z';
    if (fZero)
      pch++;
    ch = *pch++;
    c = 0;
    while (isdigit((p_int) * pch))
    {
      c = 10 * c + (*pch - '0');
      pch++;
    }
    if (c == 0)
      c = 1;
    switch (ch)
    {
      default:
        Assert(fFalse);
        break;
      case 'b':                                 /* byte (8 bit) */
        while (c--)
        {
          if (fZero)
            CondEmitB(0);
          else
          {
            if (pi) {
              CondEmitB((unsigned char)*pi);
              pi++;
            }
          }
          cb += 1;
        }
        break;
      case 'w':                                 /* word (16 bit) */
        while (c--)
        {
          if (fZero)
            CondEmitW(0);
          else
          {
            if (pi) {
              CondEmitW((unsigned short)*pi);
              pi++;
            }
          }
          cb += 2;
        }
        break;
      case 'p':                                 /* text (pointer) */
        Assert(c == 1);
        if (ppchText != NULL)
          *ppchText = *(char **)pi;
        if (!fZero)
          pi++;
        fZero = fTrue;
        /*lint -fallthrough */
      case 'l':                                 /* long (32bit) */
        while (c--)
        {
          if (fZero)
            CondEmitL(0);
          else
          {
            if (pi) {
              CondEmitL(*pi);
              pi++;
            }
          }
          cb += 4;
        }
        break;
      case 's':                                        /* skip */
        pi += c;
        break;
      case 't':                                 /* 8 bits field to byte */
        ibit += c;
        if (vfLE32)
        {
          byte = (unsigned char)(byte >> c);
          if (pv != NULL && pi != NULL && !fZero)
          {
            byte |= (*pi << (8 - c));
            pi++;
          }
        }
        else
        {
          byte = (unsigned char)(byte << c);
          if (pv != NULL && pi != NULL && !fZero)       // RMa : Bug correction test Pi != Null
            byte |= *pi++;
        }
        if (ibit == 8)
        {
          CondEmitB(byte);
          cb++;
          byte = 0;
          ibit = 0;
        }
        break;
      case 'u':                                 /* RMa add: 16 bits field to word */
        ibit += c;
        if (vfLE32)
        {                                        /* RMa add */
          word = (unsigned short)(word >> c);
          if (pv != NULL && pi != NULL && !fZero)
          {
            word |= (*pi << (16 - c));
            pi++;
          }
        }
        else
        {
          word = (unsigned short)(word << c);
          if (pv != NULL && pi != NULL && !fZero)
            word |= *pi++;
        }
        if (ibit == 16)
        {
          CondEmitW(word);
          cb += 2;
          word = 0;
          ibit = 0;
        }
        break;
    }
  }
  Assert(ibit == 0);
  return cb;
}

/*-----------------------------------------------------------------------------
|	CbStruct
|		
|		returns the size of a pilot struct given it's format pic
-------------------------------------------------------------WESC------------*/
int
CbStruct(char *szPic)
{
  return CbEmitStruct(NULL, szPic, NULL, fFalse);
}

/*
 * mapping of form object types to resource ids 
 */
static char *mpotszEmit[16];

void
CbInit(void)
{
  mpotszEmit[5] = NULL;                          /* 'tLIN'       bogus */
  mpotszEmit[6] = NULL;                          /* 'tFRA'       bogus */
  mpotszEmit[7] = NULL;                          /* 'tREC'       bogus */
  mpotszEmit[9] = szRCFORMTITLE;                 /* 'tTTL' */
  mpotszEmit[10] = szRCFORMPOPUP;                /* 'tPUL' */
  mpotszEmit[11] = szRCFORMGRAFFITISTATE;        /* 'tGSI' */
  mpotszEmit[12] = szRCFORMGADGET;               /* 'tGDT' */
  mpotszEmit[13] = szRCSCROLLBAR;                /* 'tSCL' */

  if (vfLE32)
  {
    mpotszEmit[0] = szRCFieldBA32;               /* 'tFLD' */
    mpotszEmit[1] = szRCControlBA32;             /* 'tCTL'    need to special case this one! */
    mpotszEmit[2] = szRCListBA32;                /* 'tLST' */
    mpotszEmit[3] = szRCTableBA32;               /*  szRCTBL'tTBL', */
    mpotszEmit[4] = szRCFormBitMapBA32;          /* 'tFBM' */
    mpotszEmit[8] = szRCFormLabelBA32;           /* 'tLBL' */
    mpotszEmit[14] = szRCSliderControlBA32;
    mpotszEmit[15] = szRCGraphicalControlBA32;
  }
  else
  {
    mpotszEmit[0] = szRCFieldBA16;
    mpotszEmit[1] = szRCControlBA16;
    mpotszEmit[2] = szRCListBA16;
    mpotszEmit[3] = szRCTableBA16;
    mpotszEmit[4] = szRCFormBitMapBA16;
    mpotszEmit[8] = szRCFormLabelBA16;
    mpotszEmit[14] = szRCSliderControlBA16;
    mpotszEmit[15] = szRCGraphicalControlBA16;
  }
}

/*-----------------------------------------------------------------------------
|	CbFromLt
|	
|		Return emitted size of the form item.  if fText then include the
|	length of the associated item text
-------------------------------------------------------------WESC------------*/
static int
CbFromLt(RCFORMOBJLIST * plt,
         int fText)
{
  int cb;
  RCFORMOBJECT *pobj;
  char *pchText;

  cb = 0;
  pobj = (vfLE32 ? &PBAFIELD32(plt, u.object) : &PBAFIELD16(plt, u.object));

  pchText = NULL;

  cb =
    CbEmitStruct(pobj->ptr, mpotszEmit[PBAFIELD(plt, objectType)],
                 &pchText, fFalse);
  switch (PBAFIELD(plt, objectType))
  {
    case frmListObj:
      pchText = NULL;
      if (PBAFIELD(pobj->list, numItems))        // RMa add Test for case : List with NO item
        cb +=
          PBAFIELD(pobj->list, numItems) * 4 + PBAFIELD(pobj->list,
                                                        cbListItems);
      break;
    case frmTableObj:
      pchText = NULL;
      cb += PBAFIELD(pobj->table, numColumns) *
        CbStruct(szRCTABLECOLUMNATTR) + PBAFIELD(pobj->table, numRows) *
        CbStruct(szRCTABLEROWATTR) + PBAFIELD(pobj->table, numColumns) *
        PBAFIELD(pobj->table, numRows) * CbStruct(szRCTABLEPADDING);
      ;
      break;
  }

  if (fText && pchText != NULL)
    cb += strlen(pchText) + 1;
  /*
   * padding 
   */
  if (vfLE32)
    cb = (cb + 3) & ~3;
  else
  {
    if (cb & 1)
      cb++;
  }

  return cb;
}

/*-----------------------------------------------------------------------------
|	DumpForm
-------------------------------------------------------------WESC------------*/
static void
DumpForm(FRM * pfrm)
{
  int cbDirectory;
  int clt;
  int ilt;
  RCFORMOBJLIST lt;
  int ib;
  int il;

  OpenOutput(kPalmResType[kFormRscType], PBAFIELD(pfrm, form.formId));  /* RMa "tFRM" */
  clt = PBAFIELD(pfrm, form.numObjects);
  if (vfLE32)
    Assert(PlexGetCount(&PBAFIELD32(pfrm, pllt)) == clt);
  else
    Assert(PlexGetCount(&PBAFIELD16(pfrm, pllt)) == clt);

  if (vfLE32)
    CbEmitStruct(&PBAFIELD32(pfrm, form), szRCFORM, NULL, fTrue);
  else
    CbEmitStruct(&PBAFIELD16(pfrm, form), szRCFORM, NULL, fTrue);

  cbDirectory = CbEmitStruct(&lt, szRCFORMOBJLIST, NULL, fFalse) * clt;
  ib = IbOut() + cbDirectory;
  for (ilt = 0; ilt < clt; ilt++)
  {
    int cb;

    if (vfLE32)
      lt = *(RCFORMOBJLIST *) PlexGetElementAt(&PBAFIELD32(pfrm, pllt), ilt);
    else
      lt = *(RCFORMOBJLIST *) PlexGetElementAt(&PBAFIELD16(pfrm, pllt), ilt);
    cb = CbFromLt(&lt, 1);
    SETBAFIELD(lt, u.ibobj, ib);

    if (BAFIELD(lt, objectType) == frmSliderObj)
      SETBAFIELD(lt, objectType, frmControlObj);
    if (BAFIELD(lt, objectType) == frmGraphicalControlObj)
      SETBAFIELD(lt, objectType, frmControlObj);
    ib += cb;
    CbEmitStruct(&lt, szRCFORMOBJLIST, NULL, fTrue);
  }
  /*
   * now dump the controls themselves              
   */
  for (ilt = 0; ilt < clt; ilt++)
  {
    //int cbLt;
    char *pchText;
    RCFORMOBJECT *pobj;
    RCFORMOBJLIST *plt;

    PadBoundary();
    if (vfLE32)
      plt = PlexGetElementAt(&PBAFIELD32(pfrm, pllt), ilt);
    else
      plt = PlexGetElementAt(&PBAFIELD16(pfrm, pllt), ilt);

    //              pobj = &plt->u.object;
    pobj = (vfLE32 ? &PBAFIELD32(plt, u.object) : &PBAFIELD16(plt, u.object));
    /*cbLt =*/ CbFromLt(plt, 0);
    pchText = NULL;
    CbEmitStruct(pobj->ptr, mpotszEmit[PBAFIELD(plt, objectType)],
                 &pchText, fTrue);
    switch (PBAFIELD(plt, objectType))
    {
      case frmListObj:
        pchText = NULL;                          /* we dump it ourselves */
        if (PBAFIELD(pobj->list, numItems))      // RMa add Test for case : List with NO item
        {
          for (il = 0; il < PBAFIELD(pobj->list, numItems); il++)
            DumpBytes(rgbZero, 4);
          DumpBytes(PBAFIELD(pobj->list, itemsText),
                    PBAFIELD(pobj->list, cbListItems));
        }
        break;
      case frmTableObj:
        {
          RCTABLECOLUMNATTR colattr;
          RCTABLEROWATTR rwattr;
          int irw;
          int icol;

          memset(&colattr, 0, sizeof(colattr));
          SETBAFIELD(colattr, spacing, 1);       /* ?? */
          for (icol = 0; icol < PBAFIELD(pobj->table, numColumns); icol++)
          {
            SETBAFIELD(colattr, width, PBAFIELD(pobj->table, rgdxcol[icol]));
            CbEmitStruct(&colattr, szRCTABLECOLUMNATTR, NULL, fTrue);
          }
          memset(&rwattr, 0, sizeof(rwattr));
          rwattr.height = 0x0b;                  /* ?? */
          rwattr.usable = 1;
          rwattr.selectable = 1;
          for (irw = 0; irw < PBAFIELD(pobj->table, numRows); irw++)
          {
            CbEmitStruct(&rwattr, szRCTABLEROWATTR, NULL, fTrue);
          }
          /*
           * emit bogus padding 
           */
          for (irw = 0;
               irw < PBAFIELD(pobj->table, numRows) * PBAFIELD(pobj->table,
                                                               numColumns);
               irw++)
            CbEmitStruct(NULL, szRCTABLEPADDING, NULL, fTrue);

          break;
        }
    }
    if (pchText != NULL)
    {
      PadBoundary();
      DumpBytes(pchText, strlen(pchText) + 1);
    }
  }
  PadBoundary();
  CloseOutput();
}

static BOOL
FIdFormObject(FormObjectKind kind,
              BOOL fIncludeLabels)
{
  switch (kind)
  {
    case frmFieldObj:
    case frmControlObj:
    case frmGraphicalControlObj:
    case frmListObj:
    case frmTableObj:

    case frmScrollBarObj:
    case frmSliderObj:
    case frmGadgetObj:
      return fTrue;
    case frmLabelObj:
      return fIncludeLabels;
    case frmTitleObj:
    case frmPopupObj:
    case frmLineObj:
    case frmFrameObj:
    case frmRectangleObj:
    case frmGraffitiStateObj:
    case frmBitmapObj:
      return fFalse;
  }
  Assert(fFalse);
  return fFalse;
}

/*
 * Grrr, the PalmOS developers could have been intelligent created a common header
 * for all form objects 
 */
static int
IdFromObj(const RCFORMOBJECT * pobj,
          FormObjectKind kind)
{
  switch (kind)
  {
    case frmFieldObj:
      //              return pobj->field->id;
      return PBAFIELD(pobj->field, id);

    case frmControlObj:
    case frmGraphicalControlObj:
      return PBAFIELD(pobj->control, id);
    case frmListObj:
      return PBAFIELD(pobj->list, id);
    case frmTableObj:
      return PBAFIELD(pobj->table, id);
    case frmScrollBarObj:
      return pobj->scrollbar->id;
    case frmGadgetObj:
      return pobj->gadget->id;
    case frmLabelObj:
      return PBAFIELD(pobj->label, id);
    case frmSliderObj:
      return PBAFIELD(pobj->slider, id);

    case frmTitleObj:
    case frmPopupObj:
    case frmLineObj:
    case frmFrameObj:
    case frmRectangleObj:
    case frmGraffitiStateObj:
    case frmBitmapObj:
      Assert(fFalse);
      return -1;
  }
  Assert(fFalse);
  return fFalse;
}

/*-----------------------------------------------------------------------------
|	AddObject
|	
|		Add an object (item) to the current form
-------------------------------------------------------------WESC------------*/
static VOID
AddObject(RCFORMOBJECT * pobj,
          FormObjectKind kind)
{
  RCFORMOBJLIST lt;
  int iobj;

  //      if (vpfrm->form.numObjects > cobjMax)
  //ErrorLine("Too many objects in current form");
  if (vfCheckDupes && FIdFormObject(kind, fTrue))
  {
    for (iobj = 0; iobj < PBAFIELD(vpfrm, form.numObjects); iobj++)
    {
      RCFORMOBJLIST *plt;

      if (vfLE32)
        plt = PlexGetElementAt(&PBAFIELD32(vpfrm, pllt), iobj);
      else
        plt = PlexGetElementAt(&PBAFIELD16(vpfrm, pllt), iobj);
      if (FIdFormObject((FormObjectKind)PBAFIELD(plt, objectType), fTrue) &&
          IdFromObj(pobj,
                    kind) ==
          IdFromObj((vfLE32 ? 
          	&PBAFIELD32(plt, u.object) :
            &PBAFIELD16(plt, u.object)),
            (FormObjectKind)PBAFIELD(plt, objectType)))
      {
        /*
         * dupe labels are ok 
         */
        if (!
            (kind == frmLabelObj && PBAFIELD(plt, objectType) == frmLabelObj))
          WarningLine
            ("Duplicate form object id (error is on the previous line)");
        break;
      }
    }
  }
  SETBAFIELD(lt, objectType, kind);
  SETBAFIELD(lt, u.object, *pobj);
  if (vfLE32)
  {
    PlexAddElement(&PBAFIELD32(vpfrm, pllt), &lt);
    PBAFIELD32(vpfrm, form.numObjects)++;
  }
  else
  {
    PlexAddElement(&PBAFIELD16(vpfrm, pllt), &lt);
    PBAFIELD16(vpfrm, form.numObjects)++;
  }
}

/*-----------------------------------------------------------------------------
|	FreeLt
|	
|		Frees stuff pointed to by lt (Doesn't free lt itself)
-------------------------------------------------------------WESC------------*/
static VOID
FreeLt(const RCFORMOBJLIST * plt)
{
  char *pchText;

  CbEmitStruct(PBAFIELD(plt, u.object.ptr),
               mpotszEmit[PBAFIELD(plt, objectType)], &pchText, fFalse);
  if (pchText != NULL)
    free(pchText);
  if (PBAFIELD(plt, objectType) == frmTableObj)
    //              free(plt->u.object.table->rgdxcol);
    free(PBAFIELD(PBAFIELD(plt, u.object.table), rgdxcol));
  free(PBAFIELD(plt, u.object.ptr));
}

/*-----------------------------------------------------------------------------
|	FreeFrm
-------------------------------------------------------------WESC------------*/
static void
FreeFrm(FRM * pfrm)
{
  int ilt, count;

  if (vfLE32)
  {
    count = PlexGetCount(&PBAFIELD32(pfrm, pllt));
    for (ilt = 0; ilt < count; ilt++)
      FreeLt(PlexGetElementAt(&PBAFIELD32(pfrm, pllt), ilt));
    PlexFree(&PBAFIELD32(pfrm, pllt));
  }
  else
  {
    count = PlexGetCount(&PBAFIELD16(pfrm, pllt));
    for (ilt = 0; ilt < count; ilt++)
      FreeLt(PlexGetElementAt(&PBAFIELD16(pfrm, pllt), ilt));
    PlexFree(&PBAFIELD16(pfrm, pllt));
  }
}

void
FreeRcpfile(RCPFILE * prcpf)
{
  int ifrm;

  if (prcpf != NULL)
  {
    for (ifrm = 0; ifrm < PlexGetCount(&prcpf->plfrm); ifrm++)
      FreeFrm((FRM *) PlexGetElementAt(&prcpf->plfrm, ifrm));
    PlexFree(&prcpf->plfrm);
  }

  iidMenuMac = 0;
  iidAlertMac = 0;
  iidStringMac = 0;
  iidStringTableMac = 0;
  iidAISMac = 0;
  idAutoMac = vfAutoStartID;

  FreeFontMem();
}

/*-----------------------------------------------------------------------------
|	FParseObjects
|	
|		Parse the objects (items) in a form
-------------------------------------------------------------WESC------------*/
static BOOL
FParseObjects(RCPFILE * prcpfile)
{
  RCFORMOBJECT obj;
  RW rwSav;
  ITM itm;
  int icol;
  FormObjectKind fok;
  BOOL bSeenEditableField = fFalse;
  BOOL bSeenGSI = fFalse;

  /*
   * add objects to object table until we see a rwEnd 
   */
  while (FGetTok(&tok))
  {
    fok = (FormObjectKind) - 1;

    if (ifdefSkipping && tok.lex.lt != ltPound)
      continue;

    // memset(&itm, 0, sizeof(ITM));                // Already done in ParseItm
    /*lint -e{788}*/
    switch (rwSav = tok.rw)
    {
      case rwEnd:
        if (bSeenEditableField && !bSeenGSI)
          WarningLine("Form has editable field(s) without Graffiti State Indicator");
        return fTrue;

      case rwTTL:                               /* Yuck, should just do this in FORM line! */
        obj.title = calloc(1, sizeof(RCFORMTITLE));
        ParseItm(&itm, ifText, if2Null, if3Null, if4Null);
        obj.title->text = itm.text;
        fok = frmTitleObj;
        break;

      case rwBTN:                               /* button */
      case rwREP:                               /* repeating control */
        ParseItm(&itm,
                 ifText | ifId | ifRc | ifUsable | ifEnabled | ifFont |
                 ifAnchor | ifFrame | ifBigMargin,
                 if2Graphical | if2BitmapID | if2SelectedBitmapID, 
                 if3Null, if4Null);
        goto Control;
      case rwPBN:                               /* pushbutton */
        ParseItm(&itm,
                 ifText | ifId | ifRc | ifUsable | ifEnabled | ifFont |
                 ifAnchor | ifGroup | ifSmallMargin,
                 if2Graphical | if2BitmapID | if2SelectedBitmapID, 
                 if3Null, if4Null);
        itm.frame = standardButtonFrame; 
        goto Control;
      case rwCBX:                               /* check box */
        ParseItm(&itm, ifText | ifId | ifRc | ifUsable | ifEnabled | ifFont | 
            ifAnchor | ifGroup | ifOn | ifSmallMargin | ifBigMargin, 
            if2Null, if3Null, if4Null);
        itm.frame = noButtonFrame;
        goto Control;
      case rwPUT:                               /* popuptrigger */
        ParseItm(&itm, ifText | ifId | ifRc | ifUsable | ifEnabled | ifFont | 
            ifAnchor | ifSmallMargin | ifBigMargin, 
            if2Graphical | if2BitmapID | if2SelectedBitmapID, 
            if3Null, if4Null);
        itm.frame = noButtonFrame;
        goto Control;
      case rwSLT:                               /* selectortrigger */
        ParseItm(&itm,
                 ifText | ifId | ifRc | ifUsable | ifEnabled | ifFont |
                 ifAnchor | ifSmallMargin,
                 if2Graphical | if2BitmapID | if2SelectedBitmapID, 
                 if3Null, if4Null);
        itm.frame = standardButtonFrame;
        goto Control;

      Control:
        obj.control = calloc(1, sizeof(RCControlType));
        SETPBAFIELD(obj.control, style, rwSav - rwBTN);
        SETPBAFIELD(obj.control, text, itm.text);
        SETPBAFIELD(obj.control, bounds, itm.rc);
        SETPBAFIELD(obj.control, id, itm.id);
        SETPBAFIELD(obj.control, attr.usable, itm.usable);
        SETPBAFIELD(obj.control, attr.enabled, itm.enabled);
        SETPBAFIELD(obj.control, attr.visible, fFalse);
        SETPBAFIELD(obj.control, attr.on, itm.on);
        SETPBAFIELD(obj.control, attr.leftAnchor, itm.leftAnchor);
        SETPBAFIELD(obj.control, attr.frame, itm.frame);
        SETPBAFIELD(obj.control, group, itm.group);
        SETPBAFIELD(obj.control, font, itm.font);
        if (itm.graphical)
        {                                        /* Graphic control */
          SETPBAFIELD(obj.control, attr.graphical, itm.graphical);
          SETPBAFIELD(obj.control, bitmapid, itm.bitmapid);
          SETPBAFIELD(obj.control, selectedbitmapid, itm.selectedbitmapid);
          fok = frmGraphicalControlObj;
          if (itm.bitmapid == 0)
          {
             ErrorLine
                ("Graphic control BITMAPID is missing or zero");
          }
        }
        else
        {
          SETPBAFIELD(obj.control, attr.graphical, fFalse);
          fok = frmControlObj;
        }
        break;

      case rwLBL:                               /* label */
        ParseItm(&itm, ifText | ifId | ifPt | ifFont | ifUsable, if2Null,
                 if3Null, if4Null);
        obj.label = calloc(1, sizeof(RCFormLabelType));
        SETPBAFIELD(obj.label, text, itm.text);
        SETPBAFIELD(obj.label, id, itm.id);
        SETPBAFIELD(obj.label, pos, itm.pt);
        SETPBAFIELD(obj.label, attr.usable, itm.usable);
        SETPBAFIELD(obj.label, fontID, itm.font);
        fok = frmLabelObj;
        break;

      case rwFLD:                               /* field */
        ParseItm(&itm,
                 ifId | ifRc | ifUsable | ifAlign | ifFont | ifEnabled |
                 ifUnderlined | ifSingleLine | ifEditable | ifDynamicSize |
                 ifMaxChars, if2AutoShift | if2Scrollbar | if2Numeric,
                 if3Null, if4Null);
        obj.field = calloc(1, sizeof(RCFieldType));
        SETPBAFIELD(obj.field, id, itm.id);
        SETPBAFIELD(obj.field, rect, itm.rc);
        SETPBAFIELD(obj.field, attr.usable, itm.usable);
        SETPBAFIELD(obj.field, attr.editable, itm.editable);
        SETPBAFIELD(obj.field, attr.singleLine, itm.singleLine);
        SETPBAFIELD(obj.field, attr.dynamicSize, itm.dynamicSize);
        SETPBAFIELD(obj.field, attr.insPtVisible, 1);   // RMa Must be set
        SETPBAFIELD(obj.field, attr.underlined, itm.underlined);
        SETPBAFIELD(obj.field, attr.justification, itm.justification);
        SETPBAFIELD(obj.field, attr.autoShift, itm.autoShift);
        SETPBAFIELD(obj.field, attr.hasScrollBar, itm.hasScrollBar);
        SETPBAFIELD(obj.field, attr.numeric, itm.numeric);
        SETPBAFIELD(obj.field, maxChars, itm.maxChars);
        SETPBAFIELD(obj.field, fontID, itm.font);
        fok = frmFieldObj;
		if (itm.editable) bSeenEditableField = fTrue;
        break;

      case rwPUL:                               /* popuplist */
        ParseItm(&itm, ifId | ifListId, if2Null, if3Null, if4Null);
        obj.popup = calloc(1, sizeof(RCFORMPOPUP));
        obj.popup->controlID = itm.id;
        obj.popup->listID = itm.listid;
        fok = frmPopupObj;
        break;

      case rwLST:                               /* list */
        ParseItm(&itm,
                 ifMultText | ifId | ifRc | ifUsable | ifFont | ifEnabled |
                 ifCvis | ifSmallMargin | ifSearch, if2Null, if3Null,
                 if4Null);

        obj.list = calloc(1, sizeof(RCListType));
        SETPBAFIELD(obj.list, itemsText, itm.text);
        SETPBAFIELD(obj.list, cbListItems, itm.cbText);
        SETPBAFIELD(obj.list, numItems, itm.numItems);
        SETPBAFIELD(obj.list, id, itm.id);
        SETPBAFIELD(obj.list, bounds, itm.rc);
        SETPBAFIELD(obj.list, attr.usable, itm.usable);
        if (vfPalmRez)                           // RMa (PalmRez compliant)
          SETPBAFIELD(obj.list, attr.enabled, fFalse);
        else
          SETPBAFIELD(obj.list, attr.enabled, itm.enabled);
        SETPBAFIELD(obj.list, attr.search, itm.search);

        SETPBAFIELD(obj.list, font, itm.font);
        if ( /*vfPalmRez && */ itm.rc.extent.y == 0 && itm.cvis == -1)  // RNi: PalmRez automatically compute height of lists set with a height of 0
          //          itm.cvis = WMin(itm.numItems, 10);
          //                      RNi: limiting to 10 is arbitrary and does not work. I limited to 156 pixels height total. Is 156 the best ?
          itm.cvis =
            WMin(itm.numItems,
                 156 / (PBAFIELD(obj.list, font) == largeFont ? 14 : 11));
        if (itm.cvis != -1)
        {

          /*
           * SETPBAFIELD(obj.list, bounds.extent.y, (PBAFIELD(obj.list, font) == largeFont ? 14 : 11)*itm.cvis);
           * rcPrev.extent.y = PBAFIELD(obj.list, bounds.extent.y);
           */
          /*
           * RNi 
           */
          int height =
            (PBAFIELD(obj.list, font) == largeFont ? 14 : 11) * itm.cvis;
          if (height >= 160)
            height = 160;
          /*
           * removed: 12-Nov-2001
           *
           * see doc/2.8p8emails/helge for explanation
           *
           if (itm.rc.topLeft.y + height >= 160)
           height = 160 - itm.rc.topLeft.y;
           */
          SETPBAFIELD(obj.list, bounds.extent.y, height);
          rcPrev.extent.y = height;
        }
        fok = frmListObj;
        break;

      case rwGSI:                               /* graffitistateindicator */
        ParseItm(&itm, ifPt, if2GSI, if3Null, if4Null);
		if (bSeenGSI)
		  WarningLine("Form contains multiple Graffiti State Indicators");
        obj.grfState = calloc(1, sizeof(RCFORMGRAFFITISTATE));
        obj.grfState->pos = itm.pt;
        fok = frmGraffitiStateObj;
		bSeenGSI = fTrue;
        break;

      case rwNoGSI:            /* indicate no graffitistateindicator expliticly */
		bSeenGSI = fTrue;
        break;

      case rwGDT:                               /* gadget */
        ParseItm(&itm, ifId | ifRc | ifUsable | ifExtended, if2Null, if3Null,
                 if4Null);
        obj.gadget = calloc(1, sizeof(RCFORMGADGET));
        obj.gadget->id = itm.id;
        obj.gadget->rect = itm.rc;
        obj.gadget->attr.usable = itm.usable;
        obj.gadget->attr.extended = fTrue;
        fok = frmGadgetObj;
        break;

      case rwFBM:                               /* formbitmap */
        ParseItm(&itm, ifPt | ifUsable | ifBitmap, if2Null, if3Null, if4Null);
        obj.bitmap = calloc(1, sizeof(RCFormBitMapType));
        /*
         * obj.bitmap->id = itm.id; 
         */
        SETPBAFIELD(obj.bitmap, pos, itm.pt);
        SETPBAFIELD(obj.bitmap, attr.usable, itm.usable);
        SETPBAFIELD(obj.bitmap, rscID, itm.rscID);
        fok = frmBitmapObj;
        break;

      case rwTBL:                               /* table */
        ParseItm(&itm, ifId | ifRc | ifEditable,
                 if2NumColumns | if2NumRows | if2ColumnWidths, if3Null,
                 if4Null);
        obj.table = calloc(1, sizeof(RCTableType));
        SETPBAFIELD(obj.table, id, itm.id);
        SETPBAFIELD(obj.table, bounds, itm.rc);
        if (vfPalmRez)                           // RMa (PalmRez compliant)
          SETPBAFIELD(obj.table, attr.editable, fTrue);
        else
          SETPBAFIELD(obj.table, attr.editable, itm.editable);
        SETPBAFIELD(obj.table, numColumns, itm.numColumns);
        SETPBAFIELD(obj.table, numRows, itm.numRows);
        SETPBAFIELD(obj.table, rgdxcol,
                    calloc(PBAFIELD(obj.table, numColumns), sizeof(p_int)));
        for (icol = 0; icol < PBAFIELD(obj.table, numColumns); icol++) {
          SETPBAFIELD(obj.table, rgdxcol[icol], itm.rgdxcol[icol]);
        }
        fok = frmTableObj;
        break;

      case rwSCL:                               /* scrollbar */
        ParseItm(&itm, ifId | ifRc | ifUsable,
                 if2Value | if2MinValue | if2MaxValue | if2PageSize, if3Null,
                 if4Null);
        obj.scrollbar = calloc(1, sizeof(RCSCROLLBAR));
        obj.scrollbar->id = itm.id;
        obj.scrollbar->bounds = itm.rc;
        if ((itm.rc.extent.x != 7) && (itm.rc.extent.y != 7))
          WarningLine("Scrollbar width or height not the recommended 7 pixels");
        obj.scrollbar->attr.usable = itm.usable;
        obj.scrollbar->value = itm.value;
        obj.scrollbar->minValue = itm.minValue;
        obj.scrollbar->maxValue = itm.maxValue;
        obj.scrollbar->pageSize = itm.pageSize;
        fok = frmScrollBarObj;
        break;

      case rwSLD:                               /* Slider or Slider feedback */
        ParseItm(&itm, ifId | ifRc | ifUsable | ifEnabled,
                 if2Value | if2MinValue | if2MaxValue | if2PageSize |
                 if2ThumbID | if2BackgroundID | if2Vertical | if2Feedback,
                 if3Null, if4Null);
        obj.slider = calloc(1, sizeof(RCSliderControlType));
        SETPBAFIELD(obj.slider, id, itm.id);
        if (itm.feedback)
          SETPBAFIELD(obj.slider, style, feedbackSliderCtl);    /* 7 */
        else
          SETPBAFIELD(obj.slider, style, sliderCtl);    /* 6 */
        SETPBAFIELD(obj.slider, bounds, itm.rc);
        SETPBAFIELD(obj.slider, attr.usable, itm.usable);
        SETPBAFIELD(obj.slider, attr.enabled, itm.enabled);
        SETPBAFIELD(obj.slider, attr.graphical, 1);
        SETPBAFIELD(obj.slider, attr.vertical, itm.vertical);
        SETPBAFIELD(obj.slider, value, itm.value);
        SETPBAFIELD(obj.slider, minValue, itm.minValue);
        SETPBAFIELD(obj.slider, maxValue, itm.maxValue);
        SETPBAFIELD(obj.slider, pageSize, itm.pageSize);
        SETPBAFIELD(obj.slider, thumbid, itm.thumbid);
        SETPBAFIELD(obj.slider, backgroundid, itm.backgroundid);
        fok = frmSliderObj;

        if ((itm.thumbid != 0) && (itm.backgroundid != 0))
        {
          if (itm.thumbid == itm.backgroundid)
            WarningLine
              ("graphic control custom thumbid & backgroundid bitmap are the same");
        }
        else if (itm.thumbid != 0)
          WarningLine
            ("Slider control custom backggroundid bitmap is missing");
        else if (itm.backgroundid != 0)
          WarningLine("Slider control custom thumbid bitmap is missing");
        else
        {
          /*
           * using default bitmaps for thumb abd background 
           */
          if ((itm.rc.extent.x != 72) &&
          	  (itm.rc.extent.x != 93) &&
          	  (itm.rc.extent.x != 114) &&
          	  (itm.rc.extent.x != 135) &&
          	  (itm.rc.extent.x != 156))
            WarningLine("Slider background bitmap looks best at width 72, 93, 114, 135, or 156");
          if (itm.rc.extent.y != 15)
            WarningLine("Slider background bitmap looks best at heigth 15");
        }

        if ((itm.minValue >= itm.maxValue) ||
            (itm.value < itm.minValue) || (itm.value > itm.maxValue))
        {
          WarningLine
            ("Assigning default values to VALUE, MIN, MAX and PAGESIZE");
          itm.minValue = 0;
          itm.maxValue = 100;
          itm.value = 50;
          itm.pageSize = 10;
        }

        break;

      default:
	    if (tok.lex.lt == ltPound)
	      ParseDirectives(prcpfile);
	    else
	      ErrorLine("Unknown token: %s", tok.lex.szId);
        break;
    }
    if (fok != (FormObjectKind) - 1)
      AddObject(&obj, fok);
  }

  return fFalse;
}

static void
InitFrm(FRM * pfrm)
{
  if (vfLE32)
    PlexInit(&PBAFIELD32(pfrm, pllt), sizeof(RCFORMOBJLIST), 10, 10);
  else
    PlexInit(&PBAFIELD16(pfrm, pllt), sizeof(RCFORMOBJLIST), 10, 10);
}

/*-----------------------------------------------------------------------------
|	FParseForm
-------------------------------------------------------------WESC------------*/
static BOOL
FParseForm(RCPFILE * prcpf)
{
  ITM itm;
  BOOL f;
  FRM frm;

  memset(&frm, 0, sizeof(FRM));
  InitFrm(&frm);
  vpfrm = &frm;
  //      memset(&form, 0, sizeof(RCFORM));
  //      memset(rglt, 0, cobjMax*sizeof(RCFORMOBJLIST));

  SETPBAFIELD(vpfrm, form.window.windowFlags.focusable, 1);
  ParseItm(&itm,
           ifId | ifRc | ifEnabled | ifUsable | ifFrame | ifModal |
           ifSaveBehind | ifHelpId | ifDefaultBtnId | ifMenuId, if2Null,
           if3Locale, if4Null);
  GetExpectRw(rwBegin);
  /*
   * RMa localisation 
   */
  if (!DesirableLocale(itm.Locale))
  {
    ParseToFinalEnd();
    return fFalse;
  }

  SETPBAFIELD(vpfrm, form.formId, itm.id);
  SETPBAFIELD(vpfrm, form.window.windowBounds, itm.rc);
  SETPBAFIELD(vpfrm, form.window.frameType.width, itm.frame);
  SETPBAFIELD(vpfrm, form.window.windowFlags.dialog, fTrue);    // RMa on my understanding is it better here 
  if (itm.modal)
  {
    SETPBAFIELD(vpfrm, form.window.windowFlags.modal, fTrue);
    //              SETPBAFIELD(vpfrm, form.window.windowFlags.dialog, fTrue);                      // RMa moved up 
    /*
     * SETPBAFIELD(vpfrm, form.window.frameType.word, dialogFrame);          Need to parse this!!! 
     */
    SETPBAFIELD(vpfrm, form.window.frameType.cornerDiam, 3);
    SETPBAFIELD(vpfrm, form.window.frameType.width, 2);
  }
  SETPBAFIELD(vpfrm, form.attr.saveBehind, itm.saveBehind);
  /*
   * SETPBAFIELD(vpfrm, form.attr.saveBehind, 1);
   */
  SETPBAFIELD(vpfrm, form.attr.usable, itm.usable);
  //      SETPBAFIELD(vpfrm, form.attr.enabled, itm.enabled);                                             // RMa remove. In doc this flag is said Not used
  SETPBAFIELD(vpfrm, form.helpRscId, itm.helpId);
  SETPBAFIELD(vpfrm, form.defaultButton, itm.defaultBtnId);
  SETPBAFIELD(vpfrm, form.menuRscId, itm.menuId);

  f = FParseObjects(prcpf);
  //      if (ifrmMac > ifrmMax)
  //              Error("Too many forms!");
  if (vfCheckDupes)
  {
    int ifrm;

    for (ifrm = 0; ifrm < PlexGetCount(&prcpf->plfrm); ifrm++)
    {
      FRM *pfrm;

      pfrm = (FRM *) PlexGetElementAt(&prcpf->plfrm, ifrm);
      if (PBAFIELD(pfrm, form.formId) == PBAFIELD(vpfrm, form.formId))
      {
        ErrorLine("Duplicate Form Resource IDs");
        break;
      }
    }
  }
  //      frm.rglt = calloc(sizeof(RCFORMOBJLIST), vpfrm->form.numObjects);
  //memcpy(frm.rglt, rglt, sizeof(RCFORMOBJLIST) * vpfrm->form.numObjects);

  return PlexAddElement(&prcpf->plfrm, &frm);
  //      rgfrm[ifrmMac].form = form;
  //      rgfrm[ifrmMac].rglt = calloc(sizeof(RCFORMOBJLIST), vpfrm->form.numObjects);
  //      memcpy(rgfrm[ifrmMac].rglt, rglt, sizeof(RCFORMOBJLIST) * vpfrm->form.numObjects);
  /*
   * clone pointers?  yuck 
   */
  //      ifrmMac++;
  return f;
}

/*-----------------------------------------------------------------------------
|	DumpMenu
|	
| This could be done more efficiently by emitting the strings differently, but I want to make the
| emitted bytes the same as the USR tools.
-------------------------------------------------------------WESC------------*/
static VOID
DumpMenu()
{
  int cmpd;
  int impd;
  int imi;
  p_int ibCommands;
  p_int ibStrings;

  OpenOutput(kPalmResType[kMenuRscType], idMenu);       /* RMa "MBAR" */
  cmpd = BAFIELD(menu, numMenus);
  CbEmitStruct(&menu, szRCMENUBAR, NULL, fTrue);
  imi = 0;
  ibCommands = CbStruct(szRCMENUBAR) + cmpd * CbStruct(szRCMENUPULLDOWN);
  Assert(!(ibCommands & 1));
  ibStrings = ibCommands + imiMac * CbStruct(szRCMENUITEM);
  Assert(!(ibStrings & 1));
  for (impd = 0; impd < cmpd; impd++)
  {
    int imiT;
    RCMENUPULLDOWN mpd;

    mpd = rgmpd[impd];
    SETBAFIELD(mpd, title, (char *)ibStrings);
    SETBAFIELD(mpd, items, (RCMENUITEM *) ibCommands);
    CbEmitStruct(&mpd, szRCMENUPULLDOWN, NULL, fTrue);
    ibCommands += CbStruct(szRCMENUITEM) * BAFIELD(rgmpd[impd], numItems);
    ibStrings += strlen(BAFIELD(rgmpd[impd], title)) + 1;
    /*
     * word align? 
     */
    PadBoundary();
    for (imiT = 0; imiT < BAFIELD(rgmpd[impd], numItems); imiT++)
    {
      ibStrings += strlen(rgmi[imi].itemStr) + 1;
      imi++;
    }
  }
  Assert(imi == imiMac);
  /*
   * emit menuitems 
   */
  ibCommands = CbStruct(szRCMENUBAR) + cmpd * CbStruct(szRCMENUPULLDOWN);
  Assert(!(ibCommands & 1));
  ibStrings = ibCommands + imiMac * CbStruct(szRCMENUITEM);
  Assert(!(ibStrings & 1));
  imi = 0;
  for (impd = 0; impd < cmpd; impd++)
  {
    int imiT;

    ibStrings += strlen(BAFIELD(rgmpd[impd], title)) + 1;
    /*
     * word align? 
     */
    for (imiT = 0; imiT < BAFIELD(rgmpd[impd], numItems); imiT++)
    {
      RCMENUITEM mi;

      mi = rgmi[imi];
      mi.itemStr = (char *)ibStrings;
      CbEmitStruct(&mi, szRCMENUITEM, NULL, fTrue);
      ibStrings += strlen(rgmi[imi].itemStr) + 1;
      imi++;
    }
  }
  /*
   * now finally emit the dang strings             
   */
  imi = 0;
  for (impd = 0; impd < cmpd; impd++)
  {
    int imiT;

    DumpBytes(BAFIELD(rgmpd[impd], title),
              strlen(BAFIELD(rgmpd[impd], title)) + 1);
    /*
     * word align? 
     */
    for (imiT = 0; imiT < BAFIELD(rgmpd[impd], numItems); imiT++)
    {
      DumpBytes(rgmi[imi].itemStr, strlen(rgmi[imi].itemStr) + 1);
      imi++;
    }
  }
  CloseOutput();

}

/*-----------------------------------------------------------------------------
|	AssignMenuRects
|	
|		Compute the menu screen rect
-------------------------------------------------------------WESC------------*/
static VOID
AssignMenuRects()
{
  int cmpd;
  int impd;
  int imi;
  int xTitle;

  cmpd = BAFIELD(menu, numMenus);
  imi = 0;
  xTitle = 4;
  for (impd = 0; impd < cmpd; impd++)
  {
    int imiT;
    int dxMac;
    int dx;
    RCMENUPULLDOWN *pmpd;

    pmpd = &rgmpd[impd];
    dx = DxCalcExtent((unsigned char *)PBAFIELD(pmpd, title), fTrue) + 7;
    SETPBAFIELD(pmpd, titleBounds.topLeft.x, xTitle);
    SETPBAFIELD(pmpd, titleBounds.topLeft.y, 0);
    SETPBAFIELD(pmpd, titleBounds.extent.y, 12);
    SETPBAFIELD(pmpd, titleBounds.extent.x, dx);
    xTitle += PBAFIELD(pmpd, titleBounds.extent.x);

    SETPBAFIELD(pmpd, bounds.topLeft.y, 14);
    SETPBAFIELD(pmpd, bounds.extent.y, 0);
    dxMac = 0;
    for (imiT = 0; imiT < BAFIELD(rgmpd[impd], numItems); imiT++)
    {
      int dxT;

      dxT = DxCalcExtent((unsigned char *)rgmi[imi].itemStr, fTrue) + 6;
      if (rgmi[imi].command != 0)
      {
        char szT[2];

        dxT += 6 + 12;
        szT[0] = (char)rgmi[imi].command;
        szT[1] = 0;
        dxT += DxCalcExtent((unsigned char *)szT, fTrue);
      }
      dxMac = WMax(dxMac, dxT);
      if (strcmp(rgmi[imi].itemStr, "-") == 0)
        SETPBAFIELD(pmpd, bounds.extent.y,
                    5 + PBAFIELD(pmpd, bounds.extent.y));
      else
        SETPBAFIELD(pmpd, bounds.extent.y,
                    11 + PBAFIELD(pmpd, bounds.extent.y));
      imi++;
    }
    SETPBAFIELD(pmpd, bounds.extent.x, dxMac);
    SETPBAFIELD(pmpd, bounds.topLeft.x,
                WMin(PBAFIELD(pmpd, titleBounds.topLeft.x) + 2,
                     dxScreen - dxMac - 2));
  }

}

/*-----------------------------------------------------------------------------
|	FParsePullDown
-------------------------------------------------------------WESC------------*/
static BOOL
FParsePullDown(RCPFILE * prcpfile)
{
  RCMENUPULLDOWN mpd;
  RCMENUITEM mi;
  int previousID = 0;

  memset(&mpd, 0, sizeof(RCMENUPULLDOWN));
  SETBAFIELD(mpd, title, PchGetString("Popup title"));
  GetExpectRw(rwBegin);
  while (FGetTok(&tok))
  {
    if (ifdefSkipping && tok.lex.lt != ltPound)
      continue;

    /*lint -e{788}*/
    switch (tok.rw)
    {
      default:
        if (tok.lex.lt == ltPound)
          ParseDirectives(prcpfile);
        else
          ErrorLine("END or MENUITEM expected");
        break;
      case rwMenuItem:
        memset(&mi, 0, sizeof(RCMENUITEM));
        if (!FGetTok(&tok))
          return fFalse;
        if (tok.rw == rwSeparator)
        {
          mi.itemStr = strdup("-");
          // BLC: eat an auto ID so menus specified using AutoIDs and a start number
          // will correctly skip ID numbers when used with the PilRC plugin
          (void) IdGetAutoId();
          // RMa add support of id for separator to be compliant with palmRez
          // File generated by Rsrc2Rcp generaly dont have id comflict
          if (vfPalmRez)
            mi.id = previousID;

		  // BLC: allow a manually specified ID for the separator
          FGetTok(&tok);
          if (tok.rw == rwId || tok.rw == rwAutoId)
          {
            UngetTok();
            mi.id = WGetId("CommandId");
          }
          else
          {
            UngetTok();
          }

        }
        else
        {
          int cch;

          UngetTok();
          mi.itemStr = PchGetString("Item Text");
          cch = strlen(mi.itemStr);

          // only apply special treatment if not requested
          if (!vfNoEllipsis)
          {
            // pilot has a special ... character
            // the ASCII value can be 0x18 or 0x85
            if (cch >= 3 && strcmp(&mi.itemStr[cch - 3], "...") == 0)
            {
              if (vfPalmRez)
                mi.itemStr[cch - 3] = (char)0x18;
              else
                mi.itemStr[cch - 3] = (char)0x85;
              mi.itemStr[cch - 2] = 0;
            }
            else                                 // RMa
            if ((vfPalmRez) && (mi.itemStr[cch - 1] == (char)0x85))
            {
              mi.itemStr[cch - 1] = (char)0x18;
            }
          }
          mi.id = WGetId("CommandId");
          if (vfPalmRez)
            previousID = mi.id + 1;              // RMa 
          if (FIsString(PeekTok()))
          {
            char *key = PchGetString("CommandKey string");
            if (strlen(key) != 1)
              ErrorLine("CommandKey must be 1 character");
            mi.command = toupper(key[0]);
            free(key);
          }
          if (vfCheckDupes)
          {
            int imi;

            for (imi = 0; imi < imiMac; imi++)
            {
              if (strcmp(rgmi[imi].itemStr, "-") != 0)
              {
                if (mi.id == rgmi[imi].id)
                {
                  WarningLine("Duplicate menu item id");
                  break;
                }
              }
            }
          }
        }

        if (imiMac > imiMax)
          ErrorLine("Too many menu items");
        rgmi[imiMac++] = mi;
        if (vfLE32)
          BAFIELD32(mpd, numItems)++;
        else
          BAFIELD16(mpd, numItems)++;
        break;
      case rwEnd:
        if (BAFIELD(menu, numMenus) > impdMax)
          ErrorLine("Too many menus");
        rgmpd[BAFIELD(menu, numMenus)] = mpd;
        if (vfLE32)
          BAFIELD32(menu, numMenus)++;
        else
          BAFIELD16(menu, numMenus)++;
        return fTrue;
    }
  }
  return fTrue;
}

/*-----------------------------------------------------------------------------
|	FParseMenu
-------------------------------------------------------------WESC------------*/
static BOOL
FParseMenu(RCPFILE * prcpfile)
{
  ITM itm;

  /*
   * init menu globals 
   */
  memset(&menu, 0, sizeof(RCMENUBAR));
  imiMac = 0;

  ParseItm(&itm, ifId, if2Null, if3Locale, if4Null);
  GetExpectRw(rwBegin);
  /*
   * RMa localisation 
   */
  if (!DesirableLocale(itm.Locale))
  {
    ParseToFinalEnd();
    return fFalse;
  }

  idMenu = itm.id;

  if (vfCheckDupes)
  {
    int iid;

    for (iid = 0; iid < iidMenuMac; iid++)
    {
      if (rgidMenu[iid] == idMenu)
      {
        ErrorLine("Duplicate Menu Resource ID");
      }
    }
  }

  rgidMenu[iidMenuMac++] = idMenu;

  SETBAFIELD(menu, curItem, -1);                 /* all resource I've seen have this set */
  while (FGetTok(&tok))
  {
    /*lint -e{788}*/
    switch (tok.rw)
    {
      default:
        ErrorLine("Unknown identifier: %s", tok.lex.szId);
        break;
      case rwPullDown:
        FParsePullDown(prcpfile);
        break;
      case rwEnd:
        return fTrue;
        break;
    }
  }
  return fFalse;
}

/*-----------------------------------------------------------------------------
|	FreeMenu
-------------------------------------------------------------WESC------------*/
static void
FreeMenu()
{
  int impd;
  int imi;

  for (impd = 0; impd < BAFIELD(menu, numMenus); impd++)
  {
    RCMENUPULLDOWN *pmpd;

    pmpd = &rgmpd[impd];
    free(PBAFIELD(pmpd, title));
  }
  for (imi = 0; imi < imiMac; imi++)
  {
    free(rgmi[imi].itemStr);
  }
  memset(&menu, 0, sizeof(RCMENUBAR));
  imiMac = 0;
}

/*-----------------------------------------------------------------------------
|	ParseDumpAlert
-------------------------------------------------------------WESC------------*/
static void
ParseDumpAlert(RCPFILE * prcpfile)
{
  ITM itm;
  RCALERTTEMPLATE at;
  int idAlert;
  char *pchTitle;
  char *pchMessage;

  memset(&at, 0, sizeof(at));
  memset(&itm, 0, sizeof(itm));
  pchTitle = rgbZero;
  pchMessage = rgbZero;

  //      idAlert = WGetId("AlertId", fFalse);
  ParseItm(&itm, ifId | ifHelpId | ifDefaultBtnId, if2Null,
           if3Locale | if3AlertType, if4Null);
  GetExpectRw(rwBegin);

  idAlert = itm.id;
  at.alertType = itm.AlertType;
  at.helpRscID = itm.helpId;
  at.defaultButton = itm.defaultBtnId;
  /*
   * RMa localisation 
   */
  if (!DesirableLocale(itm.Locale))
  {
    ParseToFinalEnd();
    return;
  }

  if (vfCheckDupes)
  {
    int iid;

    for (iid = 0; iid < iidAlertMac; iid++)
    {
      if (rgidAlert[iid] == idAlert)
      {
        ErrorLine("Duplicate Alert Resource ID");
      }
    }
  }
  if (iidAlertMac < iidAlertMax)
    rgidAlert[iidAlertMac++] = idAlert;

  while (FGetTok(&tok))
  {
    if (ifdefSkipping && tok.lex.lt != ltPound)
      continue;

    /*lint -e{788}*/
    switch (tok.rw)
    {
      default:
	if (tok.lex.lt == ltPound)
	  ParseDirectives(prcpfile);
	else
	  ErrorLine("END expected");
	break;
      case rwEnd:
        goto WriteAlert;
      case rwTTL:
        pchTitle = PchGetString("Title Text");
        break;
      case rwMessage:
        pchMessage = PchGetString("Message Text");
        break;
      case rwBTN:
      case rwButtons:                           /* button */
        ParseItm(&itm, ifMultText, if2Null, if3Null, if4Null);
        break;
    }
  }

WriteAlert:
  at.numButtons = itm.numItems;
  // BLC: since ifMultText now allows zero items, need to enforce
  // rule that alerts must have at least one button, or you get an alert
  // that cannot be dismissed.  Constructor enforces this for its alerts.
  if (at.numButtons == 0)
    ErrorLine("Alerts must have at least one button");
  if (at.defaultButton >= at.numButtons)
    ErrorLine("Invalid default button number");
  /*
   * removed 2.8p8, 12-Nov-2001
   *
   * see doc/2.8p8emails/neil for explanation
   
   if ((!at.defaultButton) && (at.numButtons == 1))
   at.defaultButton = 1;
   */

  OpenOutput(kPalmResType[kAlertRscType], idAlert);     /* RMa "Talt" */
  CbEmitStruct(&at, szRCALERTTEMPLATE, NULL, fTrue);
  DumpBytes(pchTitle, strlen(pchTitle) + 1);
  DumpBytes(pchMessage, strlen(pchMessage) + 1);
  if (itm.text != NULL)
    DumpBytes(itm.text, itm.cbText);
  CloseOutput();
  if (pchTitle != rgbZero)
    free(pchTitle);
  if (pchMessage != rgbZero)
    free(pchMessage);
}

/*-----------------------------------------------------------------------------
|	ParseDumpVersion
-------------------------------------------------------------WESC------------*/
static void
ParseDumpVersion()
{
  // default version ID is 1 (it dont work otherwise) :P
  int id = (FIsString(PeekTok()))? 1 : WGetId("Version ResourceId");
  char *pchVersion = PchGetString("Version Text");

  if (DesirableLocale(NULL))
  {
    OpenOutput(kPalmResType[kVerRscType], id);     /* RMa "tver" */
    DumpBytes(pchVersion, strlen(pchVersion) + 1);
    if (vfLE32)
      PadWordBoundary();
    CloseOutput();
  }

  free(pchVersion);
}

/*-----------------------------------------------------------------------------
|	ParseDumpStringTable
|
| RMa	30-Nov-00	bug correction
|							for make difference between tSTR and tSTL string
|							when we compare the Id.
|
-------------------------------------------------------------KAS-------------*/
static void
ParseDumpStringTable()
{
  int id;
  int tot = 0;
  int numStrings = 0;
  char *buf;
  char *prefixString;
  ITM itm;

  buf = malloc(32768);
  if (buf == NULL) Error("out of memory");

  ParseItm(&itm, ifId, if2Null, if3Locale, if4Null);
  id = itm.id;
  //  id = WGetId("StringTable ResourceId", fFalse);

  if (DesirableLocale(itm.Locale))
  {
    if (vfCheckDupes)
    {
      int iid;

      for (iid = 0; iid < iidStringTableMac; iid++)       // RMa add for make difference between tSTR and tSTL string
        if (rgidStringTable[iid] == id)            // RMa add for make difference between tSTR and tSTL string
          ErrorLine("Duplicate StringTable Resource ID");
    }
    if (iidStringTableMac < iidStringTableMax)     // RMa add for make difference between tSTR and tSTL string
      rgidStringTable[iidStringTableMac++] = id;   // RMa add for make difference between tSTR and tSTL string
  }

  prefixString = PchGetString("Prefix string");

  while (FIsString(PeekTok()))
  {
    char *pch = PchGetString("String text");
    int l = strlen(pch) + 1;
    if (tot + l > 32768)
      ErrorLine("Sum of string lengths must be less than 32768");

    strcpy(buf + tot, pch);
    tot += l;
    numStrings++;
    if (numStrings >= 384)
      ErrorLine("Number of strings in table must be less than 384");

    free(pch);
  }

  if (DesirableLocale(itm.Locale))
  {
    OpenOutput(kPalmResType[kStrListRscType], id); /* RMa "tSTL" */
    DumpBytes(prefixString, strlen(prefixString) + 1);
    if (vfLE32)
    {
      // RMa add  little hack for generate 68k format stringTable on LE32
      EmitB((char)(((unsigned short)numStrings & 0xff00) >> 8));
      EmitB((char)((unsigned short)numStrings & 0x00ff));
    }
    else
      EmitW((unsigned short)numStrings);

    DumpBytes(buf, tot);
    CloseOutput();
  }

  free(prefixString);
  free(buf);
}

/*-----------------------------------------------------------------------------
|	ParseDumpString
-------------------------------------------------------------WESC------------*/
static void
ParseDumpString()
{
  int id;
  char *pchString;
  char *pString;
  int cch;
  ITM itm;

  ParseItm(&itm, ifId, if2Null, if3Locale, if4Null);
  id = itm.id;

  pchString = NULL;
  //  id = WGetId("String ResourceId", fFalse);

  if (DesirableLocale(itm.Locale))
  {
    if (vfCheckDupes)
    {
      int iid;

      for (iid = 0; iid < iidStringMac; iid++)
      {
        if (rgidString[iid] == id)
        {
          ErrorLine("Duplicate String Resource ID");
        }
      }
    }
    if (iidStringMac < iidStringMax)
      rgidString[iidStringMac++] = id;
  }

  if (!FGetTok(&tok))
  {
    ErrorLine("String Text or FILE Expected");
  }

  if (tok.rw == rwFile)
  {
    char *pchFilename = PchGetString("String filename");
    FILE *fh;
    int cch;

    pchString = malloc(szMultipleLineMaxLength);

    free(FindAndOpenFile(pchFilename, "rt", &fh));
    if (fh == NULL)
      ErrorLine("Unable to open String file %s", pchFilename);

    cch = fread(pchString, 1, szMultipleLineMaxLength, fh);
    if (cch == szMultipleLineMaxLength)
      ErrorLine("String too long!");
    pchString[cch] = 0;
    fclose(fh);
    free(pchFilename);
  }
  else
  {
    /*
     * skip backslash, allow string to start on next line 
     */
    if (tok.lex.lt != ltBSlash)
      UngetTok();
    pchString = PchGetString("String Text");
  }

  cch = strlen(pchString);                       // RMa add 

  // only apply special treatment if not requested
  if (!vfNoEllipsis)
  {
    // pilot has a special ... character
    // the ASCII value can be 0x18 or 0x85
    if (cch >= 3 && strcmp((pchString + cch - 3), "...") == 0)
    {
      if (vfPalmRez)
        *(pchString + cch - 3) = (char)0x18;
      else
        *(pchString + cch - 3) = (char)0x85;
      *(pchString + cch - 2) = 0;
    }
    else                                         // RMa
    if ((vfPalmRez) && (*(pchString + cch - 1) == (char)0x85))
    {
      *(pchString + cch - 1) = (char)0x18;
    }
  }

  // character filtering for '\r'=0x0D to '\n'=0x0A
  pString = pchString;
  while (*pString)
  {
    if (*pString == 0x0D)
      *pString = 0x0A;
    pString++;
  }

  if (DesirableLocale(itm.Locale))
  {
    OpenOutput(kPalmResType[kStrRscType], id);     /* RMa "tSTR" */
    DumpBytes(pchString, strlen(pchString) + 1);
    CloseOutput();
  }

  free(pchString);
}

/*-----------------------------------------------------------------------------
|       ParseDumpCategories
-------------------------------------------------------------JOHNL-----------*/
static void
ParseDumpCategories()
{
  int id, len, count;
  char *string;
  ITM itm;

  ParseItm(&itm, ifId, if2Null, if3Locale, if4Null);
  id = itm.id;

  if (DesirableLocale(itm.Locale))
  {
    if (vfCheckDupes)
    {
      int iid;

      for (iid = 0; iid < iidAISMac; iid++)
      {
        if (rgidAIS[iid] == id)
        {
          ErrorLine("Duplicate Categories Resource ID");
        }
      }
    }
    if (iidAISMac < iidAISMax)
      //  rgidString[iidAISMac++] = id;
      rgidAIS[iidAISMac++] = id;

    OpenOutput(kPalmResType[kAppInfoStringsRscType], id); /* RMa "tAIS" */
  }

  count = 0;

  while (FIsString(PeekTok()))
  {
    string = PchGetString("String Text");
    len = strlen(string);
    /*
     * Check the size of the string and only write 15 character max 
     */
    if (len >= categoryLength)
      len = categoryLength - 1;
    if (DesirableLocale(itm.Locale))
    {
      DumpBytes(string, len);
      EmitB(0);
    }
    if (count == maxCategories)
      WarningLine
        ("More than 16 strings in a Categories. Check it to be sure");
    count++;

    /*
     * RMa  remove  maxCategories limitation.
     * For Categories the max is maxCategories (16 strings)
     * But we can use this type of resource to store other string list (I see that)
     * 
     * // Only read up to maxCategories strings
     * if (count < maxCategories) 
     * {
     * string = tok.lex.szId;
     * len = strlen(tok.lex.szId);
     * // Check the size of the string and only write 15 character max
     * if (len >= categoryLength)
     * len = categoryLength - 1;
     * DumpBytes(string, len);
     * EmitB(0);
     * }
     * else
     * {
     * if (count == maxCategories)
     * WarningLine("Too many strings in Categories. Extra strings will be ignored");
     * }
     * count++;
     */

    free(string);
  }

  if (DesirableLocale(itm.Locale))
  {
    /*
     * The AppInfo category structure expects exactly maxCategories 
     * * strings, so write a null byte for any unspecified strings 
     */
    for (; count < maxCategories; count++)
      EmitB(0);

    CloseOutput();
  }
}

/*-----------------------------------------------------------------------------
|       Parsing and dumping bitmaps and icons
-------------------------------------------------------------JohnM-----------*/

typedef struct BMPDEF
{
  char *pchFileName;
  RW compress;
  int compressMethod;
  int transparencyData[4];
  int density;
  int bpp;
  int bitmapType;
  int nColors;
  int palette[256][3];
  BOOL haspalette;
  BOOL colortable;
} BMPDEF;

static void
SetUserPalette(BMPDEF *bm)
{
  if (bm->bitmapType == rwBitmapColor16 && bm->nColors <= 16)
  {
    if (bm->haspalette)
      SetUserPalette4bpp(bm->palette, bm->nColors);
    else
      SetUserPalette4bppToDefault4bpp();
  }

  if (bm->bitmapType == rwBitmapColor256 && bm->nColors <= 256)
  {
    if (bm->haspalette)
      SetUserPalette8bpp(bm->palette, bm->nColors);
    else
      SetUserPalette8bppToDefault8bpp();
  }
}

static void
DumpBMPDEFs(int nfiles, BMPDEF *bm, int iconid, BOOL isbootscreen)
{
  int i, flag;

  if (iconid != 0 && bm[0].pchFileName == NULL)
    ErrorLine("ICONFAMILY/SMALLICONFAMILY must have a 1bpp bitmap");

  // which bitmaps do we have? :)
  flag = 0x00;
  for (i = 0; i < nfiles; i++)
    if (bm[i].pchFileName)
      flag |= 1 << i;

  for (i = 0; flag != 0x00; i++, flag >>= 1)
    if (flag & 0x01)
    {
      SetUserPalette(&bm[i]);
      DumpBitmap(bm[i].pchFileName, iconid, bm[i].compress,
		 bm[i].bitmapType, bm[i].colortable, bm[i].transparencyData,
		 (flag & 0xfe) != 0, isbootscreen, kSingleDensity);
    }
}

#define MAXDEPTH  24

static void
DumpMultidensityBMPDEFs(int flag, BMPDEF *bm, int iconid, BOOL isbootscreen)
{
  int i, nbrRemaining, nbrOfBmpDensityOne = 0, nbrOfBmpDensityTwoOrMore = 0;
  BOOL writeFakeDblHdr = fFalse;

  for (i = 0; i < MAXDEPTH; i++)
    if (flag & (1L << i))
    {
      if (bm[i].density == kSingleDensity)
	nbrOfBmpDensityOne++;
      else
	nbrOfBmpDensityTwoOrMore++;
    }

  for (i = 0; flag != 0x00; i++, flag >>= 1)
    if (flag & 0x01)
    {
      SetUserPalette(&bm[i]);

      // fake header: begin of bmp part with density != kSingleDensity
      if (bm[i].density != kSingleDensity && !writeFakeDblHdr)
      {
	RCBITMAP emptyBmpHeader;
	memset(&emptyBmpHeader, 0, sizeof emptyBmpHeader);
	emptyBmpHeader.pixelsize = 255;
	emptyBmpHeader.version = vfLE32? 0x81 : 0x01;
	CbEmitStruct(&emptyBmpHeader, szRCBITMAP, NULL, fTrue);
	writeFakeDblHdr = fTrue;
      }

      if (bm[i].density == kSingleDensity)
	nbrRemaining = --nbrOfBmpDensityOne + nbrOfBmpDensityTwoOrMore;
      else
	nbrRemaining = --nbrOfBmpDensityTwoOrMore;

      DumpBitmap(bm[i].pchFileName, iconid, bm[i].compress, bm[i].bitmapType,
		 bm[i].colortable, bm[i].transparencyData,
		 (nbrRemaining > 0), isbootscreen, bm[i].density);
    }
}

static char *
nullify(char *s)
{
  if (s != NULL && strlen(s) == 0)
    free (s), s = NULL;
  return s;
}

static int
DecodeDepthRW (RW rw)
{
  /*lint -e{788}*/
  switch (rw)
  {
    case rwDepth2:   return 2;
    case rwDepth4:   return 4;
    case rwDepth8:   return 8;
    case rwDepth16:  return 16;
    default:         return 0;
  }
}

static int
DecodeBitmapType(RW type)
{
  /*lint -e{788}*/
  switch (type)
  {
    case rwBitmap:          return 1;
    case rwBitmapGrey:      return 2;
    case rwBitmapGrey16: case rwBitmapColor16:  return 4;
    case rwBitmapColor256:  return 8;
    case rwBitmapColor16k:  return 16;
    case rwBitmapColor24k:  return 24;
    case rwBitmapColor32k:  return 32;
    default:                return 0;
  }
}

typedef struct
{
  RW type, compress;
} FamilyItemAttr;

static int
FindDepth(const FamilyItemAttr *eachAttr, int depth)
{
  int i;
  for (i = 0; eachAttr[i].type != rwNil; i++)
    if (DecodeBitmapType(eachAttr[i].type) == depth)
      return i;
  return i;
}

static int
ParseBitmapAttrs(BMPDEF *attr, FamilyItemAttr *eachAttr)
{
  char *fname;
  int inattrs = fTrue;
  int ndx = -1;

  while (inattrs && FGetTok(&tok))
    /*lint -e{788}*/
    switch (tok.rw)
    {
      case rwNoCompress:
      case rwAutoCompress:
      case rwForceCompress:
        attr->compress = tok.rw;

	if (eachAttr)
	{
	  if (PeekTok()->lex.lt == ltConst)
	    while (PeekTok()->lex.lt == ltConst)
	      eachAttr[FindDepth(eachAttr, WGetConst("bitmap depth"))].compress
		  = attr->compress;
	  else
	  {
	    int i;
	    for (i = 0; eachAttr[i].type != rwNil; i++)
	      eachAttr[i].compress = attr->compress;
	  }
	}
        break;

      case rwCompressRLE:
      case rwCompressPackBits:
	WarningLine("RLE and PackBits compression methods not yet supported");
	/*lint -fallthrough */
      case rwCompressScanLine:
      case rwCompressBest:
        attr->compressMethod = tok.rw;
        break;

      case rwBitmapBpp:
	attr->bpp = WGetConst("bits per pixel");
	switch (attr->bpp)
	{
	  case  1:  ndx = 0;  attr->bitmapType = rwBitmap;          break;
	  case  2:  ndx = 1;  attr->bitmapType = rwBitmapGrey;      break;
	  case  4:  ndx = 2;  attr->bitmapType = rwBitmapGrey16;    break;
	  case  5:  ndx = 2;  attr->bitmapType = rwBitmapColor16;   break;
	  case  8:  ndx = 3;  attr->bitmapType = rwBitmapColor256;  break;
	  case 16:  ndx = 4;  attr->bitmapType = rwBitmapColor16k;  break;
	  case 24:  ndx = 5;  attr->bitmapType = rwBitmapColor24k;  break;
	  case 32:  ndx = 6;  attr->bitmapType = rwBitmapColor32k;  break;
	  default:
	    ErrorLine("Bitmap BPP not supported");
	    break;
	}
	break;

      case rwNoColorTable:
	attr->colortable = fFalse;
	break;

      case rwColorTable:
	attr->colortable = fTrue;
	break;

      case rwBitmapPalette:
	fname = nullify(PchGetString("Palette filename"));
	if (fname)
	{
	  attr->haspalette = fTrue;
	  ParsePaletteFile(fname, attr->palette, &attr->nColors);
	  free(fname);
	}
	else
	  ErrorLine("Empty or no file name provided");
	break;

      case rwTransparency:
	attr->transparencyData[0] = rwTransparency;
	attr->transparencyData[1] = WGetConst("red value");
	attr->transparencyData[2] = WGetConst("green value");
	attr->transparencyData[3] = WGetConst("blue value");
	break;

      case rwTransparencyIndex:
	attr->transparencyData[0] = rwTransparencyIndex;
	attr->transparencyData[1] = WGetConst("transparency index");
	break;

      case rwBitmapDensity:
	attr->density = WGetConst("bitmap density");
	switch (attr->density)
	{
	  case 1:
	    attr->density = kSingleDensity;
	    break;
	  case 2:
	    attr->density = kDoubleDensity;
	    break;
	  case kSingleDensity:
	  case kOneAndOneHalfDensity:
	  case kDoubleDensity:
	    break;
	  default:
	    ErrorLine ("Unsupported bitmap density (use 72, 108, or 144)");
	    break;
	}
	break;

      default:
	UngetTok();
	inattrs = fFalse;
	break;
    }

  if (ndx >= 4 && attr->transparencyData[0] == rwTransparencyIndex)
    ErrorLine("16k, 24k, and 32k color bitmaps "
	      "don't support transparency index.");

  switch (attr->density)
  {
    case kOneAndOneHalfDensity:
      ndx += 8;
      break;
    case kDoubleDensity:
      ndx += 16;
      break;
  }

  return ndx;
}

static void
ParseDumpBitmap(RW kind, BOOL begin_allowed)
{
  char *restype = kPalmResType[kBitmapRscType];
  char *restype_freeme = NULL;
  int resid = -1;
  BOOL inpreamble, isicon, isbootscreen;
  char *locale = NULL;
  BMPDEF defattr, bm[MAXDEPTH];
  FamilyItemAttr eachAttr[MAXDEPTH+1];
  void (*dumper)(int, BMPDEF *, int, BOOL) = NULL;
  int i, dumperdata = 0;

  isicon = (kind == rwIcon || kind == rwIconSmall);

  if (! isicon && FIsString(PeekTok()))
    restype = restype_freeme = PchGetString("Resource Type");

  if (isicon)
    resid = (kind == rwIcon)? 1000 : 1001;

  isicon = (isicon || strcmp(restype, "tAIB") == 0);
  if (isicon)
  {
    /* RMa vfAppIcon68K indicates a forced "tAIB" resource */
    restype = (vfAppIcon68K)? "tAIB" : kPalmResType[kIconType];
  }

  isbootscreen = (kind == rwBootScreenFamily || strcmp(restype, "Tbsb") == 0);
  if (isbootscreen)
  {
    if (vfPrc)
      WarningLine("PilRC option -ro not compatible with 'Tbsb' dump.");
    restype = kPalmResType[kBsBitmapRscType]; // convert if ARM
  }

  memset (&defattr, 0, sizeof defattr);
  defattr.pchFileName = NULL;
  defattr.haspalette = fFalse;
  defattr.compress = rwNoCompress;
  defattr.compressMethod = rwCompressScanLine;
  defattr.colortable = fFalse;
  defattr.transparencyData[0] = 0;
  defattr.density = kSingleDensity;
  defattr.bpp = 0;

  for (i = 0; i < MAXDEPTH; i++)
    bm[i].pchFileName = NULL;

  inpreamble = fTrue;

  while (inpreamble && FGetTok(&tok))
    if (tok.rw == rwId || tok.lex.lt == ltConst || tok.lex.lt == ltLParen
	|| (tok.lex.lt == ltId && tok.rw == rwNil))
    {
      UngetTok();
      resid = WGetId("Bitmap/Icon resource ID");
    }
    else
      /*lint -e{788}*/
      switch (tok.rw)
      {
	case rwLocale:
	  locale = PchGetString("locale");
	  break;

	case rwNoCompress:
	case rwAutoCompress:
	case rwForceCompress:
	  defattr.compress = tok.rw;
	  break;

	case rwCompressScanLine:
	case rwCompressRLE:
	case rwCompressPackBits:
	case rwCompressBest:
	  defattr.compressMethod = tok.rw;
	  break;

	case rwRscType:
	  restype = restype_freeme = PchGetString("Resource Type");
	  break;

	case rwIncludeClut:
	  defattr.colortable = fTrue;
	  break;

	default:
	  inpreamble = fFalse;
	  break;
      }

  if (resid == -1)
    ErrorLine ("Bitmap resource ID required");

  if (FIsString(&tok))
  {
    static const RW normalTypes[] = {
      rwBitmap, rwBitmapGrey, rwBitmapGrey16,
      rwBitmapColor256, rwBitmapColor16k, rwBitmapColor24k, rwBitmapColor32k
      };
    static const RW specialTypes[] = {
      rwBitmap, rwBitmapGrey, rwBitmapColor16,
      rwBitmapColor256, rwBitmapColor16k, rwBitmapColor24k, rwBitmapColor32k
      };

    BMPDEF attr = defattr;
    char *pchFileName[MAXDEPTH];
    int maxfiles, ndx;
    const RW *bitmapTypes;

    /*lint -e{788}*/
    switch (kind)
    {
      case rwBitmapGrey:
      case rwBitmapGrey16:
      case rwBitmapColor16:
      case rwBitmapColor256:
      case rwBitmapColor16k:
      case rwBitmapColor24k:
      case rwBitmapColor32k:
	bitmapTypes = &kind;
	maxfiles = 1;
	break;

      case rwBitmapFamily_special:
	bitmapTypes = specialTypes;
	maxfiles = 7;
	break;

      default:
	bitmapTypes = normalTypes;
	maxfiles = 7;
	break;
    }

    for (i = 0; i < maxfiles; i++)
    {
      pchFileName[i] = NULL;
      eachAttr[i].type = bitmapTypes[i];
      eachAttr[i].compress = attr.compress;
    }
    eachAttr[maxfiles].type = rwNil;

    UngetTok();
    ndx = 0;
    while (FIsString(PeekTok()) || DecodeDepthRW(PeekTok()->rw) != 0)
    {
      if (! FIsString(PeekTok()))
      {
	FGetTok(&tok);
	ndx = FindDepth(eachAttr, DecodeDepthRW(tok.rw));
      }
	
      if (ndx < maxfiles)
	pchFileName[ndx++] = nullify(PchGetString("Bitmap filename"));
      else
      {
	free(PchGetString("Bitmap filename"));
	WarningLine("Excess bitmap filenames ignored.");
      }
    }

    ParseBitmapAttrs(&attr, eachAttr);

    for (i = 0; i < maxfiles; i++)
    {
      bm[i] = attr;
      bm[i].pchFileName = pchFileName[i];
      bm[i].bitmapType = eachAttr[i].type;
      bm[i].compress = eachAttr[i].compress;
    }

    dumper = DumpBMPDEFs;
    dumperdata = maxfiles;
  }
  else if (tok.rw == rwBegin && begin_allowed)
  {
    int flag = 0;

    for (i = 0; i < MAXDEPTH; i++)
      bm[i] = defattr;

    while (FGetTok(&tok) && tok.rw == rwBitmap)
    {
      int ndx;
      BMPDEF cur = defattr;
      cur.pchFileName = nullify(PchGetString("Bitmap filename"));
      ndx = ParseBitmapAttrs(&cur, NULL);

      if (ndx == -1)
	ErrorLine("BPP expected.");
      if (ndx == 2 && (flag & 0x04))
	ErrorLine("Bitmap can't have both 4 bit gray and 4 bit color data.");
      else if (flag & (1 << ndx))
	ErrorLine("Multiple bitmaps for one depth/density.");
      else
	flag |= 1 << ndx;

      bm[ndx] = cur;
    }

    if (tok.rw != rwEnd)
      ErrorLine("BITMAP or END expected");

    dumper = DumpMultidensityBMPDEFs;
    dumperdata = flag;
  }
  else
    ErrorLine(begin_allowed? "Bitmap filename or BEGIN expected."
			   : "Bitmap filename expected.");

  if (dumper && DesirableLocale(locale))
  {
    BOOL saveLE32 = vfLE32;

    if (isicon && vfAppIcon68K)
      vfLE32 = fFalse;  /* force app icon to be in 68K format */

    OpenOutput(restype, resid);
    dumper(dumperdata, bm, isicon? resid : 0, isbootscreen);
    CloseOutput();

    vfLE32 = saveLE32;
  }

  for (i = 0; i < MAXDEPTH; i++)
    free (bm[i].pchFileName);

  free (restype_freeme);
  free (locale);
}

/*---------------------------------------------------------------------------
|	ParseDumpLauncherCategory
-------------------------------------------------------------RMa------------*/
static void
ParseDumpLauncherCategory(void)
{
  int id = 0;                                    // ??? MJM: Added initialization to 0
  char *pString = NULL;
  char *pLocale = NULL;

  while (FGetTok(&tok))
  {
    if ((tok.rw == rwId) || (tok.rw == rwAutoId))
    {
      UngetTok();
      id = WGetId("LauncherCategory ResourceId");
      if (id != 1000)
        WarningLine
          ("Default Launcher Category ID is 1000 (it doesn't work otherwise)");
    }
    else if (tok.rw == rwLocale)
    {
      pLocale = PchGetString("locale");
    }
    else if (FIsString(&tok))
      {
        UngetTok();
        pString = PchGetString("taic");
        break;
      }
    else
      ErrorLine("Unexpected token: %s", tok.lex.szId);
  }

  if (id == 0)
    id = 1000;

  if (DesirableLocale(pLocale))
  {
    OpenOutput(kPalmResType[kDefaultCategoryRscType], id);      /* RMa "taic" */
    DumpBytes(pString, strlen(pString) + 1);
    //      RMa Remove padding is it no necessary padding. it done by prcbuild if needded
    //      PadBoundary();
    CloseOutput();
  }

  free(pString);
  free(pLocale);
}

/*-----------------------------------------------------------------------------
|	ParseDumpApplicationIconName
-------------------------------------------------------------WESC------------*/
static void
ParseDumpApplicationIconName()
{
  char *pchString;
  ITM itm;

  ParseItm(&itm, ifId, if2Null, if3Locale, if4Null);
  pchString = PchGetString("Icon Name Text");

  if (DesirableLocale(itm.Locale))
  {
    OpenOutput(kPalmResType[kAinRscType], itm.id);     /* RMa "tAIN" */
    DumpBytes(pchString, strlen(pchString) + 1);

    // RMa this resource is align on 16 bits in the two world
    //      Force 16 bit padding
    PadWordBoundary();

    CloseOutput();
  }

  free(pchString);
}

/*-----------------------------------------------------------------------------
|	ParseDumpApplication
-------------------------------------------------------------WESC------------*/
static void
ParseDumpApplication()
{
  char *pchString;
  ITM itm;

  ParseItm(&itm, ifId, if2Null, if3Locale, if4Null);
  pchString = PchGetString("APPL");
  if (strlen(pchString) != 4)
    ErrorLine("APPL resource must be 4 chars");

  if (DesirableLocale(itm.Locale))
  {
    OpenOutput(kPalmResType[kApplicationType], itm.id);       /* RMa "APPL" */
    DumpBytes(pchString, 4);
    CloseOutput();
  }

  free(pchString);
}

/*-----------------------------------------------------------------------------
|	ParseDumpTrap
-------------------------------------------------------------WESC------------*/
static void
ParseDumpTrap()
{
  int id;
  int wTrap;
  TOK tok;                                       // RMa validate if needed

  id = WGetId("TRAP ResourceId");
  /*
   * Skip "value" if it's present.  
   */
  FGetTok(&tok);
  if (tok.rw != rwValue)
    UngetTok();
  wTrap = WGetConstEx("Trap number");
  if ((wTrap & 0xA000) == 0)
  {
    wTrap |= 0xA000;
  }
  if (id < 1000)
    ErrorLine
      ("TRAP resource id must be >= 1000, see HackMaster documentation");

  if (DesirableLocale(NULL))
  {
    OpenOutput(kPalmResType[kTrapType], id);       /* RMa "TRAP" */
    EmitW((unsigned short)wTrap);
    CloseOutput();
  }
}

/*-----------------------------------------------------------------------------
|     ParseDumpFont
-------------------------------------------------------------DPT-------------*/
static void
ParseDumpFont()
{
  int id;
  int fontid;
  char *pchFileName;
  ITM itm;

  ParseItm(&itm, ifId | ifFont, if2Null, if3Locale, if4Null);
  id = itm.id;
  fontid = itm.font;

  if (fontid < 128 || fontid > 255)
    ErrorLine("FontID invalid.  valid values: 128<=FontID<=255");
  pchFileName = PchGetString("Font Filename");

  if (DesirableLocale(itm.Locale))
  {
    OpenOutput(kPalmResType[kFontRscType], id);  /* RMa "NFNT" */
    DumpFont(pchFileName, fontid);
    CloseOutput();
  }
  if (pchFileName)
    free(pchFileName);
}

/*-----------------------------------------------------------------------------
|     ParseDumpFontFamily
-------------------------------------------------------------RMa-------------*/
static void
ParseDumpFontFamily()
{
  ITM itm;
  int id;
  int densityCount = 0;
  int i;

#define MAXDENSITY 2
  FNTFAMDEF aFontFamilyEntries[MAXDENSITY];

  ParseItm(&itm, ifId | ifFont, if2Null, if3Locale, if4Null);
  id = itm.id;

  if (itm.font < 128 || itm.font > 255)
    ErrorLine("FontID invalid.  valid values: 128<=FontID<=255");

  GetExpectRw(rwBegin);

  while (FGetTok(&tok) && (tok.rw != rwEnd))
  {
    if (tok.rw == rwBitmapDensity)
    {
      aFontFamilyEntries[densityCount].density = WGetConstEx("Density");
      aFontFamilyEntries[densityCount].pchFileName = PchGetString("Font Filename");
    }
    else if (tok.rw == rwFont)
    {
      aFontFamilyEntries[densityCount].pchFileName = PchGetString("Font Filename");
      GetExpectRw(rwBitmapDensity);
      aFontFamilyEntries[densityCount].density = WGetConstEx("Density");
    }
    else
      ErrorLine("Unknown token in Font Family.");

    densityCount++;
    if (densityCount > MAXDENSITY)
    {
      ErrorLine("Font Family can have only have 2 density (72, 144).");
      break;
    }
  }

  if (DesirableLocale(itm.Locale))
  {
    OpenOutput(kPalmResType[kFontFamilyRscType], id);    /* RMa "nfnt" */
    DumpFontFamily(itm.font, 1, densityCount, aFontFamilyEntries);
    CloseOutput();
  }

  for (i = 0; i < densityCount; i++)                 /* RMa dump each value */
    free(aFontFamilyEntries[i].pchFileName);
}

/*-----------------------------------------------------------------------------
|     ParseDumpHex
--------------------------------------------------------------AA-------------*/
static void
ParseDumpHex()
{
  char *pchResType = PchGetString("Resource Type");
  ITM itm;

  ParseItm(&itm, ifId, if2Null, if3Locale, if4Null);

  if (DesirableLocale(itm.Locale))
    OpenOutput(pchResType, itm.id);

  for (;;)
  {
    const TOK *ptok = PeekTok();
    if (FIsString(ptok))
    {
      char *pch = PchGetString("String Text");
      if (DesirableLocale(itm.Locale))
        DumpBytes(pch, strlen(pch));
      free(pch);
    }
    else if (ptok->lex.lt == ltConst ||
             (ptok->lex.lt == ltId && ptok->rw == rwNil))
    {
      /* We can't really use WGetConst() here, because we need the token
         explicitly so that we can look at its size field.  */
      FGetTok(&tok);
      if (tok.lex.lt == ltId)
      {
        const SYM *psym = PsymLookup(tok.lex.szId);
        if (psym)
        {
          CheckNumericSymbol(psym);
          tok.lex.val = psym->wVal;
        }
        else
          ErrorLine("Symbol %s is not defined", tok.lex.szId);
      }

      if (DesirableLocale(itm.Locale))
         switch (tok.lex.size)
         {
           case lsLong:
                if ((unsigned long) tok.lex.val > 0xffffffffUL)
                  ErrorLine("HEX data must be LONG (0..4294967295)");
                EmitL((unsigned long)tok.lex.val);
                break;
 
           case lsWord:
                if (tok.lex.val > 0xffff)
                  ErrorLine("HEX data must be WORD (0..65535)");
                EmitW((unsigned short)tok.lex.val);
                break;
 
           case lsUnspecified:
           case lsByte:
                if (tok.lex.val > 0xff)
                  ErrorLine("HEX data must be BYTE (0..255)");
                EmitB((unsigned char)tok.lex.val);
                break;
         }
    }
    else  // Something else (including EOF) -- end of resource
      break;
  }

  if (DesirableLocale(itm.Locale))
    CloseOutput();

  free(pchResType);
}

/*-----------------------------------------------------------------------------
|     ParseDumpData
--------------------------------------------------------------AA-------------*/
static void
ParseDumpData()
{
  char *pchResType, *pchFileName;
  ITM itm;

  // get the information from the .rcp entry
  pchResType = PchGetString("Resource Type");
  ParseItm(&itm, ifId, if2Null, if3Locale, if4Null);
  pchFileName = PchGetString("Data Filename");

  // file name available?
  if ((pchFileName == NULL) || (strcmp(pchFileName, "") == 0))        /* RMa bug correction invert test */
    ErrorLine("Empty or no file name provided");
  else
  {
    if (DesirableLocale(itm.Locale))
    {
      // write the data to file
      OpenOutput(pchResType, itm.id);
      {
        int cch;
        char *data;
        FILE *fh;

        free(FindAndOpenFile(pchFileName, "rb", &fh));
        if (fh == NULL)
          ErrorLine("Unable to open Data file %s", pchFileName);

        data = malloc(4096);
        if (data == NULL) Error("out of memory");
        cch = fread(data, 1, 4096, fh);
        while (cch != 0)
        {
          DumpBytes(data, cch);
          cch = fread(data, 1, 4096, fh);
        }
        fclose(fh);
        free(data);
      }
      CloseOutput();
    }
  }

  free(pchResType);
  free(pchFileName);
}

/*-----------------------------------------------------------------------------
|	ParseDumpInteger
-------------------------------------------------------------BLC-------------*/
static void
ParseDumpInteger()
{
  ITM itm;

  ParseItm(&itm, ifId, if2Value, if3Locale, if4Null);

  if (!(itm.grif2Out & if2Value))
    itm.value = WGetConstEx("Integer Value");

  if (DesirableLocale(itm.Locale))
  {
    long nInteger = itm.value;
    OpenOutput(kPalmResType[kConstantRscType], itm.id);       /* "tint" */
    EmitL(nInteger);
    CloseOutput();
  }
}

/*-----------------------------------------------------------------------------
|	ParseNumbers
-------------------------------------------------------------JohnM-----------*/
static unsigned int
ParseNumbers(unsigned long **bufferp)
{
  unsigned long *buffer = NULL;
  unsigned int bufsize = 0;
  unsigned int n = 0;
  unsigned long w;

  while (FGetTok(&tok) && tok.rw != rwEnd)
  {
	UngetTok();
	w = WGetConstEx("number");

    if (n >= bufsize)
    {
      bufsize = (bufsize == 0)? 64 : 2 * bufsize;
      buffer = realloc (buffer, bufsize * sizeof (unsigned long));
    }

    buffer[n++] = w;
  }

  *bufferp = buffer;
  return n;
}

/*-----------------------------------------------------------------------------
|	ParseDumpWordList
-------------------------------------------------------------RMa-------------*/
static void
ParseDumpWordList(void)
{
  int id;
  unsigned int i, n;
  unsigned long *buffer;
  ITM itm;

  ParseItm(&itm, ifId, if2Null, if3Locale, if4Null);
  GetExpectRw(rwBegin);
  /*
   * RMa localisation 
   */
  if (!DesirableLocale(itm.Locale))
  {
    ParseToFinalEnd();
    return;
  }

  id = itm.id;

  n = ParseNumbers (&buffer);

  OpenOutput(kPalmResType[kWrdListRscType], id); /* RMa "wrdl" */
  EmitW(n);                                /* RMa element in list */
  for (i = 0; i < n; i++)                 /* RMa dump each value */
    EmitW(buffer[i]);
  CloseOutput();
  free(buffer);
}

/*-----------------------------------------------------------------------------
|	ParseDumpLongWordList
-------------------------------------------------------------RMa-------------*/
static void
ParseDumpLongWordList(void)
{
  int id;
  int defaultItem;
  unsigned int i, n;
  unsigned long *buffer;
  ITM itm;

  memset(&itm, 0, sizeof(itm));
  ParseItm(&itm, ifId, if2Null, if3Locale | if3DefaultItm, if4Null);
  GetExpectRw(rwBegin);
  /*
   * RMa localisation 
   */
  if (!DesirableLocale(itm.Locale))
  {
    ParseToFinalEnd();
    return;
  }

  id = itm.id;
  defaultItem = itm.DefaultItem;

  n = ParseNumbers (&buffer);

  OpenOutput(kPalmResType[kLongWrdListRscType], id);    /* RMa "DLST" */
  EmitW((unsigned short)defaultItem);            /* RMa default item */
  EmitW((unsigned short)n);                      /* RMa element in list */
  for (i = 0; i < n; i++)                 /* RMa dump each value */
    EmitL(buffer[i]);
  CloseOutput();
  free(buffer);
}

/*-----------------------------------------------------------------------------
|	ParseDumpByteList
-------------------------------------------------------------RMa-------------*/
static void
ParseDumpByteList(void)
{
  int id;
  int defaultItem;
  unsigned int i, n;
  unsigned long *buffer;
  ITM itm;

  memset(&itm, 0, sizeof(itm));
  ParseItm(&itm, ifId, if2Null, if3Locale | if3DefaultItm, if4Null);
  GetExpectRw(rwBegin);
  /*
   * RMa localisation 
   */
  if (!DesirableLocale(itm.Locale))
  {
    ParseToFinalEnd();
    return;
  }

  id = itm.id;
  defaultItem = itm.DefaultItem;

  n = ParseNumbers (&buffer);

  OpenOutput(kPalmResType[kByteListRscType], id);       /* RMa "BLST" */
  EmitW((unsigned short)defaultItem);            /* RMa default item */
  EmitW((unsigned short)n);                /* RMa element in list */
  for (i = 0; i < n; i++)                 /* RMa dump each value */
    EmitB(buffer[i]);
  CloseOutput();
  free(buffer);
}

/*-----------------------------------------------------------------------------
|	ParseDumpPaletteTable
-------------------------------------------------------------RMa-------------*/
static void
ParseDumpPaletteTable(void)
{
  int id;
  int counter = 0;                               /* RMa number of color in a palette. (a color is 4 bytes) */
  int component = 0;
  char *pData;
  char *pRunning;
  int bufferSize = 256 * 4;

  id = WGetId("Integer ResourceId");
  GetExpectRw(rwBegin);
  pData = malloc(bufferSize);
  pRunning = pData;

  while (FGetTok(&tok))
  {
    // we have a constant? 
    if (tok.lex.lt == ltConst)
    {
      if (tok.lex.val > 0xff)
        ErrorLine("Color conponent must be BYTE (0..255)");

      *pRunning = (unsigned char)tok.lex.val;
      pRunning++;
      component++;
      if (component >= 4)
      {
        component = 0;
        counter++;
        if ((counter * 4) >= bufferSize)
        {
          int offset = bufferSize;

          bufferSize *= 2;
          pData = realloc(pData, bufferSize);
          pRunning = pData + offset;
        }
      }
    }
    // we dunno, assume "end" of resource
    else if (tok.rw == rwEnd)
      break;
  }

  if (component)
    ErrorLine("Palette table : incomplete color definitions");

  /*
   * We write the datas to disk 
   */
  if (DesirableLocale(NULL))
  {
    OpenOutput(kPalmResType[kColorTableRscType], id);     /* RMa "tclt" */
    if (vfLE32)
      EmitL(counter);
    else
      EmitW((unsigned short)counter);
    DumpBytes(pData, (counter * 4));
    CloseOutput();
  }

  free(pData);
}

/*-----------------------------------------------------------------------------
|      ParseDumpPalette
-------------------------------------------------------------RMa-------------*/
static void
ParseDumpPalette()
{
  char *pchFileName;
  int id;

  id = WGetId("Data ResourceId");

  // get the information from the .rcp entry
  pchFileName = PchGetString("Palette Filename");

  // file name available?
  if ((pchFileName == NULL) || (strcmp(pchFileName, "") == 0))
    ErrorLine("Empty or no file name provided");
  else
  {
    int palette[256][3];
    int nColors;
    int c;

    OpenOutput("PALT", id);
    ParsePaletteFile(pchFileName, palette, &nColors);

    for (c = 0; c < nColors; c++)
    {
      DumpBytes(&c, 1);
      DumpBytes(&palette[c][0], 1);
      DumpBytes(&palette[c][1], 1);
      DumpBytes(&palette[c][2], 1);
    }

    CloseOutput();
  }

  if (pchFileName != NULL)
  {
    free(pchFileName);
  }
}

/*-----------------------------------------------------------------------------
|	ParseDumpMidi 'MIDI'
-------------------------------------------------------------RMa------------*/
static void
ParseDumpMidi(void)
{
  int resId;
  char *pFileName;

  // get the information from the .rcp entry
  resId = WGetId("Midi ResourceId");
  pFileName = PchGetString("Data Filename");

  if (!DesirableLocale(NULL))
  {
    if (pFileName != NULL)
      free(pFileName);
    return;
  }

  // file name available?
  if ((pFileName == NULL) || (strcmp(pFileName, "") == 0))      /* RMa bug correction invert test */
  {
    ErrorLine("Empty or no file name provided");
  }
  else
  {
    FILE *pFh;
    char *pData;
    int fileSize;

    free(FindAndOpenFile(pFileName, "rb", &pFh));      /* open file in read and binary */
    if (pFh == NULL)
      ErrorLine("Unable to open midi file %s", pFileName);

    fileSize = ftell(pFh);                       /* compute the file size */
    fseek(pFh, 0, SEEK_END);                     /* jump to the end of file */
    fileSize = ftell(pFh) - fileSize;            /* compute the file size */
    fseek(pFh, 0, SEEK_SET);                     /* jump to the begin of file */

    if (fileSize)
    {
      pData = malloc(fileSize);
      if (pData)
      {
        int cch;

        // write the data to file
        OpenOutput(kPalmResType[kMidiRscType], resId);  /* RMa "MIDI" */

        cch = fread(pData, 1, fileSize, pFh);
        while (cch != 0)
        {
          DumpBytes(pData, cch);
          cch = fread(pData, 1, fileSize, pFh);
        }
        fclose(pFh);
        CloseOutput();
        free(pData);
      }
    }
    else
    {
      ErrorLine("Unable to read empty midi file %s", pFileName);
    }
  }
  if (pFileName != NULL)
    free(pFileName);
}

#ifdef PALM_INTERNAL
/*-----------------------------------------------------------------------------
|	ParseApplicationPreferences : 'pref'
-------------------------------------------------------------RMa-------------*/
static void
ParseApplicationPreferences(void)
{
  RCSYSAPPPREFS sysAppPrefs;
  ITM itm;
  int resID = 0;

  ParseItm(&itm, ifId, if2Priority,
           if3StackSize | if3MinHeapSpace | if3Locale, if4Null);
  /*
   * RMa localisation 
   */
  if (!DesirableLocale(itm.Locale))
  {
    return;
  }

  resID = itm.id;

  SETBAFIELD(sysAppPrefs, priority, itm.Priority);
  SETBAFIELD(sysAppPrefs, stackSize, itm.StackSize);
  SETBAFIELD(sysAppPrefs, minHeapSpace, itm.MinHeapSpace);

  // We write the datas to disk
  OpenOutput(kPalmResType[kAppPrefsType], resID);       /* RMa "pref" */
  if (vfLE32)
    CbEmitStruct(&(sysAppPrefs.s32), szRCSYSAPPPREFS, NULL, fTrue);
  else
    CbEmitStruct(&(sysAppPrefs.s16), szRCSYSAPPPREFS, NULL, fTrue);
  CloseOutput();
}
#endif

/*-----------------------------------------------------------------------------
|	ParseTranslation
-------------------------------------------------------------WESC------------*/
static void
ParseTranslation()
{
  BOOL fAddTranslation;
  char *pch;
  TE *pte = NULL;
  int i;

  pch = PchGetString("Language");

  fAddTranslation = fFalse;
  for (i = 0; i < totalLanguages; i++)
    if (FSzEqI(pch, aszLanguage[i]))
      fAddTranslation = fTrue;

  free(pch);

  GetExpectRw(rwBegin);
  while (FGetTok(&tok))
  {
    /*lint -e{788}*/
    switch (tok.rw)
    {
      case rwEnd:
        return;
      case rwNil:
        if (! FIsString(&tok))
          ErrorLine("String Expected");
        if (fAddTranslation)
        {
          pte = malloc(sizeof(TE));
          UngetTok();
          pte->szOrig = PchGetString("Untranslated string");
        }
        GetExpectLt(&tok, ltAssign, "=");
        pch = PchGetString("Translation");
        if (fAddTranslation)
        {
          pte->szTrans = pch;
          pte->pteNext = pteFirst;
          pteFirst = pte;
        }
        else
          free(pch);
       	break;

      default:
        break;
    }
  }
}

/*-----------------------------------------------------------------------------
|	ParseResetAutoID
-------------------------------------------------------------BLC-------------*/

static void
ParseResetAutoID()
{
    int newAutoID = WGetConstEx("reset value for AutoID");
    idAutoMac = newAutoID;
    idAutoDirection = +1;
   
}

/*-----------------------------------------------------------------------------
|	ParseGenerateHeader
-------------------------------------------------------------BLC-------------*/

static void
ParseGenerateHeader()
{
    free(szOutputHeaderFile);
    szOutputHeaderFile = PchGetString("Output header name");
    vfAutoId = fTrue;
}


/*-----------------------------------------------------------------------------
|	ParseNavigation: helper to parse OBJECT entries in a NAVIGATION res
-------------------------------------------------------------BLC-------------*/
static void
ParseNavigationList(RCNAVIGATIONITEM **navigationItems, p_int *numItems)
{
  ITM itm;
  *numItems = 0;
  *navigationItems = NULL;

  while(1)
  {
    ParseItm(&itm, 
	  ifId, if2ForceInteraction, if3Null,
	  if4Above | if4Below | if4Skip | if4BigButton);

    ++(*numItems);
    
    *navigationItems = realloc(
    	*navigationItems, sizeof(RCNAVIGATIONITEM) * *numItems);
    (*navigationItems)[*numItems - 1].objectID = itm.id;
    (*navigationItems)[*numItems - 1].aboveObjectID = itm.aboveID;
    (*navigationItems)[*numItems - 1].belowObjectID = itm.belowID;
    (*navigationItems)[*numItems - 1].objectFlags =
      (itm.skip ? 0x0001 : 0) |
      (itm.forceInteraction ? 0x0002 : 0) |
      (itm.bigButton ? 0x8000 : 0);

    /* check for END at end of list */
    FGetTok(&tok);
    if (tok.rw == rwEnd) break;
    UngetTok();
  }
}

/*-----------------------------------------------------------------------------
|	ParseNavigationMap: helper to parse NAVIGATIONMAP entries
-------------------------------------------------------------BLC-------------*/
static void
ParseNavigationMap(RCNAVIGATIONITEM **navigationItems, p_int *numItems)
{
  ITM itm;
  const TOK *pTok;
  int currentRowStart = 0;
  int lastRowStart = 0;
  int lastRowEnd = 0;
  int lastRowFirstID = 0;
  int i;
  
  *numItems = 0;
  *navigationItems = NULL;

  /* check for END at end of list */
  while (1)
  {
    FGetTok(&tok);
    if (tok.rw == rwEnd) break;
    else if (tok.rw == rwRow)
    {
	  currentRowStart = *numItems;
	  
	  /* parse a row of data, which will be in the form:
	   * <ID.n> [SKIP] [FORCEINTERACTION] [BIGBUTTON] */
  	
  	  while (1)
  	  {
    	  int id;
    	  id = WGetConstEx("form object id");
    	  
    	  /* check for modifier keywords */
    	  ParseItm(&itm, ifNull, if2ForceInteraction, 
    	    if3Null, if4Skip | if4BigButton);

          ++(*numItems);
		  *navigationItems = realloc(
		    *navigationItems, sizeof(RCNAVIGATIONITEM) * *numItems);

		  (*navigationItems)[*numItems - 1].objectID = id;
	      (*navigationItems)[*numItems - 1].aboveObjectID = lastRowFirstID;
	      (*navigationItems)[*numItems - 1].objectFlags =
	        (itm.skip ? 0x0001 : 0) |
	        (itm.forceInteraction ? 0x0002 : 0) |
	        (itm.bigButton ? 0x8000 : 0);
	        
	      /* break if next token is "END" or "ROW" */
	      pTok = PeekTok();
	      if (pTok && (pTok->rw == rwEnd || pTok->rw == rwRow))
	        break;
  	  }
  	  
	  /* store ID of start of current row to use for "above" of next row */
	  lastRowFirstID = (*navigationItems)[currentRowStart].objectID;

      /* fix previous row to point to start of current row */
      for (i = lastRowStart; i < lastRowEnd; ++i)
      {
        (*navigationItems)[i].belowObjectID = lastRowFirstID;
      }

	  /* store start/end of current row to fixup after next row */
      lastRowStart = currentRowStart;
  	  lastRowEnd = *numItems;
    }
    else
      Error("Expected END or ROW");
  }

  /* fix last row to point to zero for "below" pointers */
  for (i = lastRowStart; i < lastRowEnd; ++i)
  {
    (*navigationItems)[i].belowObjectID = 0;
  }
}

/*-----------------------------------------------------------------------------
|	ParseNavigation: 'fnav'
-------------------------------------------------------------BLC-------------*/

static void
ParseNavigation(void)
{
  RCNAVIGATION navigation;
  RCNAVIGATIONITEM *navigationItems = NULL;
  ITM itm;
  int i;
  
  ParseItm(&itm, 
      ifId, if2Null, if3Vers | if3Locale,
      if4InitialState | if4InitialObjectID |
      if4JumpObjectID | if4BottomLeftObjectID);

  if (itm.grif4Out & if3Vers && itm.version != 1)
  {
  	Error("Only NAVIGATION version 1 supported");
  }
  if (!(itm.grif4Out & if4InitialState))
  {
  	Error("INITIALSTATE required in NAVIGATION resource");
  }
  
  // this is all version one of the resource
  // (rely on ParseItm to zero out fields of the itm)
  navigation.headerSizeInBytes = 20;
  navigation.navElementSizeInBytes = 8;
  navigation.version = 1;
  navigation.navFlags = itm.initialState;
  navigation.initialObjectIDHint = itm.initialObjectID;
  navigation.jumpObjectIDHint = itm.jumpObjectID;
  navigation.bottomLeftObjectIDHint = itm.bottomLeftObjectID;


  // FIXME: check for what kind of nav list we've got
  FGetTok(&tok);
  if (tok.rw == rwBegin)
      ParseNavigationList(&navigationItems, &navigation.numOfObjects);
  else if (tok.rw == rwNavigationMap)
      ParseNavigationMap(&navigationItems, &navigation.numOfObjects);
  else
      Error("Expected BEGIN or NAVIGATIONMAP");
 
  if (DesirableLocale(itm.Locale))
  {
    OpenOutput(kPalmResType[kNavigationType], itm.id);
    CbEmitStruct(&navigation, szRCNAVIGATION, NULL, fTrue);
    for (i = 0; i < navigation.numOfObjects; ++i)
    {
      CbEmitStruct(navigationItems + i, szRCNAVIGATIONITEM, NULL, fTrue);
    }
    CloseOutput();
  }

  free(navigationItems);
}

/*-----------------------------------------------------------------------------
|	OpenInputFile
-------------------------------------------------------------WESC------------*/
static VOID
OpenInputFile(const char *szIn)
{
  vIn.file.szFilename = FindAndOpenFile(szIn, "rt", &vIn.file.fh);
  vIn.file.line = 0;
  vIn.buffer[0] = '\0';
  vIn.pch = vIn.buffer;
  FInitLexer(NULL, fTrue);
  vIn.fPendingTok = fFalse;
}

/*-----------------------------------------------------------------------------
|	ParseCInclude
|	
|		Supports #define foo const and foo equ const
/      Added Support for #ifdef/#else/#endif/nested #includes
-------------------------------------------------------------WESC------------*/
static void
ParseCInclude(const char *szIncludeFile)
{
  INPUTCONTEXT inSav = vIn;
  int ifdefSkippingSav = ifdefSkipping;
  int ifdefLevelSav = ifdefLevel;

  char szId[256];
  int wIdVal;

  static int depth = 0;

  depth++;
  ifdefSkipping = 0;
  ifdefLevel = 0;

  OpenInputFile(szIncludeFile);

  while (FGetTok(&tok))
  {
    switch (tok.lex.lt)
    {
      case ltPound:
        {
          if (!FGetTok(&tok))
            ErrorLine("preprocessor directive expected");
          /*lint -e{788}*/
          switch (tok.rw)
          {
              /*
               * LDu 31-8-2001: process nested include files 
               * limit the depth to 32 files...
               */
            case rwInclude:
              {
                // if flag vfIFIH set by user we ignore all includes
                // files within header files.
                if (!vfIFIH && ifdefSkipping == 0)
                {
                  if (depth < 32)
                  {
                    char *pchFilename = PchGetString("include filename");
                    ParseCInclude(pchFilename);
                    free(pchFilename);
                  }
                  else
                  {
                    ErrorLine("too many nested files");
                  }
                }
                else
                  NextLine();                    // RMa in #include not parsed jump to next line
                break;
              }

            case rwDefine:
              {
                if (ifdefSkipping)
                {
                  NextLine();
                }
                else
                {
                  AddDefineSymbol();
                }
                break;
              }

            case rwUndef:
              {
                if (ifdefSkipping)
                {
                  NextLine();
                }
                else
                {
                  GetExpectLt(&tok, ltId, "identifier");
                  RemoveSym(tok.lex.szId);
                }
                break;
              }

            case rwIfdef:
              {
                ifdefLevel++;

                if (ifdefSkipping)
                {
                  ifdefSkipping++;
                }
                else
                {
                  SYM *psym;

                  GetExpectLt(&tok, ltId, "identifier");
                  strcpy(szId, tok.lex.szId);

                  psym = PsymLookup(szId);
                  if (psym == NULL)
                  {
                    ifdefSkipping = 1;
                  }
                }
                break;
              }

            case rwIfndef:
              {
                ifdefLevel++;
                if (ifdefSkipping)
                {

                  ifdefSkipping++;
                }
                else
                {
                  SYM *psym;

                  GetExpectLt(&tok, ltId, "identifier");
                  strcpy(szId, tok.lex.szId);

                  psym = PsymLookup(szId);
                  if (psym != NULL)
                  {
                    ifdefSkipping = 1;
                  }
                }
                break;
              }

            case rwElse:
              {
                /*
                 * Verify Else is valid 
                 */
                if (ifdefLevel)
                {
                  /*
                   * only do something if ifdefSkipping == 0 or 1 
                   */
                  if (ifdefSkipping == 1)
                  {
                    ifdefSkipping = 0;
                  }
                  else
                  {
                    if (ifdefSkipping == 0)
                    {
                      ifdefSkipping = 1;
                    }
                  }
                }
                else
                {
                  ErrorLine("no matching #ifdef for %s", tok.lex.szId);
                }
                break;
              }

            case rwEndif:
              {
                if (ifdefLevel)
                {
                  --ifdefLevel;
                  if (ifdefSkipping)
                  {
                    --ifdefSkipping;
                  }
                }
                else
                {
                  ErrorLine("no matching #ifdef for %s", tok.lex.szId);
                }
                break;
              }

            case rwError:
              {
                if (!ifdefSkipping)
                  ErrorLine("#error %s", PchLexerBuffer());
                break;
              }

            default:
              {
                NextLine();
                break;
              }
          }
          break;
        }
      case ltId:
        if ((ifdefSkipping) || (tok.rw == rwExtern))    /* RMa add case for 'extern' directive */
        {
          NextLine();
        }
        else
        {
          strcpy(szId, tok.lex.szId);
          GetExpectRw(rwEqu);
          wIdVal = WGetConstEx("Constant");
          AddSym(szId, wIdVal);
        }
        break;
      case ltSemi:
      case ltDoubleSlash:
      case ltRBrace:                            /* RMa add case for usage of include in C++ */
        {
          NextLine();
          break;
        }
      default:
        ErrorLine("Identifier expected");
        break;
    }
  }

  fclose(vIn.file.fh);
  free(vIn.file.szFilename);
  vIn = inSav;

  ifdefSkipping = ifdefSkippingSav;
  ifdefLevel = ifdefLevelSav;
  depth--;
}

/*-----------------------------------------------------------------------------
|	ParseJavaInclude
|	
|		Supports a file of the EXACT form:
|
    package <pkgname>;
	
	public class <classname>
	{
		public static final short <idname> = <idnumber>;
	}
-------------------------------------------------------------DAVE------------*/
static void
ParseJavaInclude(char *szIncludeFile)
{
  INPUTCONTEXT inSav = vIn;

  char szId[256];
  int wIdVal;

  OpenInputFile(szIncludeFile);

  /*
   * Skip to class xxx { 
   */
  while (FGetTok(&tok) && (tok.lex.lt != ltLBrace))
  {
  }

  if (tok.lex.lt != ltLBrace)
  {
    ErrorLine("expected class definition in java file");
  }

  while (FGetTok(&tok))
  {
    /*lint -e{788}*/
    switch (tok.rw)
    {
      case rwPublic:
      case rwShort:
      case rwInt:
      case rwStatic:
      case rwFinal:
        continue;
      default:
        break;
    }

    switch (tok.lex.lt)
    {
      case ltId:
        strcpy(szId, tok.lex.szId);
        GetExpectLt(&tok, ltAssign,
                    "[public] [static] [final] [short|int] Name = number;");

        wIdVal = WGetConstEx("Constant");
        AddSym(szId, wIdVal);
        break;
      case ltSemi:
      case ltDoubleSlash:
        NextLine();
        break;
      case ltRBrace:
        goto endOfClass;
      default:
        ErrorLine("Identifier definition expected");
        break;
    }
  }

endOfClass:
  fclose(vIn.file.fh);
  free(vIn.file.szFilename);
  vIn = inSav;
}

static BOOL
OverwriteIncFile( const char *inc_fn, const char *tmp_fn )
{
	FILE *inc_file, *tmp_file;
	int inc_char, tmp_char;
	BOOL overwrite = 1;

	if ( ! inc_fn || ! tmp_fn )
		return 1;

	inc_file = fopen( inc_fn, "r" );
	tmp_file = fopen( tmp_fn, "r" );
	if ( inc_file && tmp_file )
	{
		do
		{
			inc_char = getc( inc_file );
			tmp_char = getc( tmp_file );
			if ( inc_char == EOF || tmp_char == EOF )
			{
				overwrite = inc_char != tmp_char;
				break;
			}
		} while ( inc_char == tmp_char );
	}
	if ( inc_file )
		fclose( inc_file );
	if ( tmp_file )
		fclose( tmp_file );

	return overwrite;
}

static void
WriteIncFile(char *szFile)
{
  FILE *temp_file;
  SYM *psym;
  char *temp_name = MakeTempFilename();

  temp_file = fopen(temp_name, "w");
  if (temp_file == NULL)
    Error("Can't write to temporary file %s: %s", temp_name, strerror(errno));

  fprintf(temp_file, "/* pilrc generated file.  Do not edit!*/\n");

  for (psym = psymFirst; psym != NULL; psym = psym->psymNext)
    if (psym->fAutoId)
      fprintf(temp_file, "#define %s %d\n", psym->sz, psym->wVal);

  fclose(temp_file);

  if (OverwriteIncFile(szFile, temp_name))
  {
    /* CodeWarrior PilRC command line adapter depends on the next phrase's
       exact wording to detect that an include file was written, so don't
       remove this message or change it without informing Metrowerks - BLC */

    if (! vfQuiet)
      printf("writing include file: %s\n", szFile);

    remove(szFile);
    if (rename(temp_name, szFile) != 0)
    {
      /* Attempt to copy manually; rename() may have failed merely because
         the files are on different filesystems, for example.  */

      FILE *out_file;
      char buffer[4096];
      size_t n;

      if ((temp_file = fopen(temp_name, "r")) == NULL)
        Error("Can't open temporary file %s: %s", temp_name, strerror(errno));

      if ((out_file = fopen(szFile, "w")) == NULL)
        Error("Can't write to include file %s: %s", szFile, strerror(errno));

      while ((n = fread(buffer, 1, sizeof buffer, temp_file)) > 0)
        fwrite(buffer, 1, n, out_file);

      if (ferror(out_file))
        Error("Error writing to include file %s: %s", szFile, strerror(errno));

      fclose(out_file);
      fclose(temp_file);

      remove(temp_name);
    }
  }
  else
  {
    if (! vfQuiet)
      printf("No changes made to include file: %s\n", szFile);

    remove(temp_name);
  }

  free(temp_name);
}


static VOID
InitRcpfile(RCPFILE * prcpfile,
            int fontType)
{
  PlexInit(&prcpfile->plfrm, sizeof(FRM), 10, 10);
  InitFontMem(fontType);

  iidMenuMac = 0;
  iidAlertMac = 0;
  iidStringMac = 0;
  iidStringTableMac = 0;
  iidAISMac = 0;
  idAutoMac = vfAutoStartID;
}

/*-----------------------------------------------------------------------------
/	ParseRcpFile
/   original code extract from ParseFile
/   put in separate function to obtain recursive analyzis thru .rcp files
-------------------------------------------------------------LDu--------------*/
static void
ParseRcpFile(const char *szRcpIn, RCPFILE * prcpfile)
{
  INPUTCONTEXT inSav = vIn;
  int ifdefSkippingSav = ifdefSkipping;
  int ifdefLevelSav = ifdefLevel;

  static int depth = 0;

  depth++;
  ifdefSkipping = 0;
  ifdefLevel = 0;

  if (depth >= 32)
  {
    ErrorLine("too many nested files");
  }

  OpenInputFile(szRcpIn);

  if (!FGetTok(&tok))
    Error("bogus source file");
  do
  {
    if (ifdefSkipping)
    {
      if (tok.lex.lt == ltPound)
      {
        ParseDirectives(prcpfile);
      }
      else
      {
        NextLine();
      }
      continue;
    }
    /*lint -e{788}*/
    switch (tok.rw)
    {
      case rwForm:
        if (FParseForm(prcpfile))
          DumpForm(PlexGetElementAt
                   (&prcpfile->plfrm, PlexGetCount(&prcpfile->plfrm) - 1));
        break;
      case rwMenu:
        if (FParseMenu(prcpfile))
        {
          AssignMenuRects();
          DumpMenu();
          FreeMenu();
        }
        break;
      case rwAlert:
        ParseDumpAlert(prcpfile);
        break;
      case rwVersion:
        ParseDumpVersion();
        break;
      case rwStringTable:
        ParseDumpStringTable();
        break;
      case rwString:
        ParseDumpString();
        break;
      case rwCategories:
        ParseDumpCategories();
        break;
      case rwApplicationIconName:
        ParseDumpApplicationIconName();
        break;
      case rwApplication:
        ParseDumpApplication();
        break;
      case rwTranslation:
        ParseTranslation();
        break;

      case rwBitmapGrey:
      case rwBitmapGrey16:
      case rwBitmapColor16:
      case rwBitmapColor256:
      case rwBitmapColor16k:
      case rwBitmapColor24k:
      case rwBitmapColor32k:
      case rwBitmapFamily_special:
	ParseDumpBitmap(tok.rw, fFalse);
        break;

#ifdef PALM_INTERNAL
      case rwBootScreenFamily:
	ParseDumpBitmap(rwBootScreenFamily, fTrue);
	break;
#endif

      case rwBitmap:
      case rwBitmapFamily:
      case rwBitmapFamilyEx:
	ParseDumpBitmap(rwBitmap, fTrue);
	break;

      case rwIcon:
      case rwIconFamily:
      case rwIconFamilyEx:
	ParseDumpBitmap(rwIcon, fTrue);
	break;

      case rwIconSmall:
      case rwIconSmallFamily:
      case rwIconSmallFamilyEx:
	ParseDumpBitmap(rwIconSmall, fTrue);
	break;

      case rwInteger:
        ParseDumpInteger();
        break;
      case rwWordList:
        ParseDumpWordList();
        break;
      case rwLongWordList:
        ParseDumpLongWordList();
        break;
      case rwByteList:
        ParseDumpByteList();
        break;
      case rwTrap:
        ParseDumpTrap();
        break;
      case rwFont:
        ParseDumpFont();
        break;
      case rwFontFamily:
        ParseDumpFontFamily();
        break;
      case rwPaletteTable:
        ParseDumpPaletteTable();
        break;
      case rwMidi:
        ParseDumpMidi();
        break;
      case rwPalette:
        ParseDumpPalette();
        break;
      case rwLauncherCategory:                  // RMa add: support for this resource
        ParseDumpLauncherCategory();
        break;

#ifdef PALM_INTERNAL
      case rwGraffitiInputArea:
        ParseDumpGraffitiInputArea();
        break;
      case rwCountryLocalisation:
        ParseDumpCountry();
        break;
      case rwLocales:
        ParseDumpLocales();
        break;
      case rwFeature:
        ParseDumpFeature();
        break;
      case rwKeyboard:
        ParseDumpKeyboard();
        break;
      case rwFontIndex:
        ParseDumpFontIndex();
        break;
      case rwFontMap:
        ParseDumpFontMap();
        break;
      case rwHardSoftButtonDefault:
        ParseDumpHardSoftButtonSoft();
        break;
      case rwSysAppPrefs:
        ParseApplicationPreferences();
        break;
      case rwTableList:
        ParseDumpTableList();
        break;
      case rwCharsetList:
        ParseDumpCharsetList();
        break;
      case rwTextTable:
        ParseDumpTextTable();
        break;
      case rwSearchTable:
        ParseDumpSearchTable();
        break;
#endif

      case rwNavigation:
      	ParseNavigation();
      	break;
      case rwHex:
        ParseDumpHex();
        break;
      case rwData:
        ParseDumpData();
        break;
      case rwResetAutoID:
        ParseResetAutoID();
        break;
      case rwGenerateHeader:
        ParseGenerateHeader();
        break;

        /*
         * Add a rw here, remember to add to error message below 
         */
      default:
        if (tok.lex.lt == ltPound)
        {
          ParseDirectives(prcpfile);
        }
        else if (tok.lex.lt == ltDoubleSlash)
          NextLine();
        else
        {                                        /* RMa add error string more explicit */
          ErrorLine ("Unknown token : '%s' ("
                     "FORM, MENU, ALERT, VERSION, STRINGTABLE, STRING, CATEGORIES, "
                     "APPLICATIONICONNAME, APPLICATION, BITMAP, PALETTE, "
                     "BITMAPGREY, BITMAPGREY16, BITMAPCOLOR16, BITMAPCOLOR, "
                     "BITMAPCOLOR16K, BITMAPCOLOR24K, BITMAPCOLOR32K, BITMAPFAMILY, "
                     "BITMAPFAMILYSPECIAL, BITMAPFAMILYEX, "
#ifdef PALM_INTERNAL
                     "BOOTSCREENFAMILY, "
#endif
                     "ICON, ICONFAMILY, SMALLICON, SMALLICONFAMILY, ICONFAMILYEX, SMALLICONFAMILYEX, "
                     "LAUNCHERCATEGORY, BYTELIST, WORDLIST, LONGWORDLIST, MIDI, "
                     "SYSAPPLICATIONPREFERENCES, PALETTETABLE, TRAP, FONT, FONTFAMILY, HEX, DATA, INTEGER, "
#ifdef PALM_INTERNAL
                     "FONTINDEX, FONTMAP, GRAFFITIINPUTAREA, COUNTRYLOCALISATION, "
                     "LOCALES, FEATURE, KEYBOARD, HARDSOFTBUTTONDEFAULT, "
                     "SEARCHTABLE, TEXTTABLE, TABLELIST, CHARSETLIST, "
#endif
                     "RESETAUTOID, GENERATEHEADER, or TRANSLATION expected)",
                     tok.lex.szId);
        }
    }
  }
  while (FGetTok(&tok));

  fclose(vIn.file.fh);
  free(vIn.file.szFilename);
  vIn = inSav;

  ifdefSkipping = ifdefSkippingSav;
  ifdefLevel = ifdefLevelSav;
  depth--;
}

/*-----------------------------------------------------------------------------
|	ParseLineDirective
-------------------------------------------------------------JohnM-----------*/
static void
ParseLineDirective(BOOL fnameRequired)
{
  // The constant given is for the *following* line
  vIn.file.line = WGetConst("Line number constant") - 1;

  if (fnameRequired || FIsString(PeekTok()))
  {
    free(vIn.file.szFilename);
    vIn.file.szFilename = PchGetString("Input filename");
  }
}

/*-----------------------------------------------------------------------------
|	ParseDirectives
-------------------------------------------------------------DAVE------------*/
static void
ParseDirectives(RCPFILE * prcpfile)
{
  TOK tok;
  char szId[256];

  FGetTok(&tok);
  /*lint -e{788}*/
  switch (tok.rw)
  {
    case rwInclude:
      {
        // ignore include files when skipping
        if (ifdefSkipping == 0)
        {
          char *szFileName = PchGetString("include filename");
          const char *pchExt;

          pchExt = strrchr(szFileName, '.');
	  if (pchExt)
	    pchExt++;
	  else
	    pchExt = "";

	  // If the extension is ".java" or ".jav", it's a Java include;
	  // otherwise, extensions starting with ".R" or ".r" (e.g. ".rcp")
	  // are included PilRC source code; and all others are C headers.

	  if (FSzEqI(pchExt, "java") || FSzEqI(pchExt, "jav"))
	    ParseJavaInclude(szFileName);
	  else if (tolower(pchExt[0]) == 'r')
	    ParseRcpFile(szFileName, prcpfile);
	  else
	    ParseCInclude(szFileName);

          free(szFileName);
        }
        break;
      }
    case rwDefine:
      {
          if (ifdefSkipping)
            NextLine();
          else
            AddDefineSymbol();
          break;
      }

    case rwUndef:
      {
        if (ifdefSkipping)
        {
          NextLine();
        }
        else
        {
          GetExpectLt(&tok, ltId, "identifier");
          RemoveSym(tok.lex.szId);
        }
        break;
      }

    case rwIfdef:
      {
        ifdefLevel++;

        if (ifdefSkipping)
        {

          ifdefSkipping++;
        }
        else
        {
          SYM *psym;

          GetExpectLt(&tok, ltId, "identifier");
          strcpy(szId, tok.lex.szId);

          psym = PsymLookup(szId);
          if (psym == NULL)
          {
            ifdefSkipping = 1;
          }
        }
        break;
      }

    case rwIfndef:
      {
        ifdefLevel++;

        if (ifdefSkipping)
        {

          ifdefSkipping++;
        }
        else
        {
          SYM *psym;

          GetExpectLt(&tok, ltId, "identifier");
          strcpy(szId, tok.lex.szId);

          psym = PsymLookup(szId);
          if (psym != NULL)
          {
            ifdefSkipping = 1;
          }
        }
        break;
      }

    case rwElse:
      {
        /*
         * Verify Else is valid 
         */
        if (ifdefLevel)
        {
          /*
           * only do something if ifdefSkipping == 0 or 1 
           */
          if (ifdefSkipping == 1)
          {
            ifdefSkipping = 0;
          }
          else
          {
            if (ifdefSkipping == 0)
            {
              ifdefSkipping = 1;
            }
          }
        }
        else
        {
          ErrorLine("no matching #ifdef for %s", tok.lex.szId);
        }
        break;
      }

    case rwEndif:
      {
        if (ifdefLevel)
        {
          --ifdefLevel;
          if (ifdefSkipping)
          {
            --ifdefSkipping;
          }
        }
        else
        {
          ErrorLine("no matching #ifdef for %s", tok.lex.szId);
        }

        break;
      }

    case rwLine:
      {
	// #line LINENO [FNAME]
        ParseLineDirective(fFalse);
        break;
      }

      /*
       * Add new directives here      
       */

    case rwError:
      {
        if (!ifdefSkipping)
          ErrorLine("#error %s", PchLexerBuffer());
        break;
      }

    default:
      if (tok.lex.lt == ltConst)
      {
	// # LINENO FNAME [FLAG...]
        UngetTok();
        ParseLineDirective(fTrue);
        // Skip any flags at the end of the linemarker
        NextLine();
      }
      else
        ErrorLine("directive not supported (try #include): %s", tok.lex.szId);
      break;
  }

}

/*-----------------------------------------------------------------------------
|	ParseFile
-------------------------------------------------------------WESC------------*/
RCPFILE *
ParseFile(const char *szIn,
          const char *szOutDir,
          const char *szResFile,
          const char *szIncFile,
          int fontType)
{
  RCPFILE *prcpfile;
  char *szMainOutput;

  if (szIncFile != NULL)
  {
    szOutputHeaderFile = MakeFilename("%s", szIncFile);
  }

  prcpfile = calloc(1, sizeof(RCPFILE));
  InitRcpfile(prcpfile, fontType);

  if (vfPrc)
  {
    if (strcmp(szOutDir, ".") == 0)
    {
      /*
       * PRC file format was selected, but no output filename was given.
       * Deduce one from the input filename.  
       */
      szMainOutput = MakeFilename("%s%e.ro", szIn, ".rcp");
    }
    else
    {
      /*
       * A filename was given.  Append an extension if it doesn't have one.  
       */
      char *dot = strrchr(szOutDir, '.');
      if (dot && (strchr(dot, '/') != NULL || strchr(dot, '\\') != NULL))
        dot = NULL;                              /* The dot was in a directory name, not the basename.  */

      szMainOutput = MakeFilename((dot)? "%s" : "%s.ro", szOutDir);
    }

    OpenResDBFile(szMainOutput);
  }
  else
  {
    SetOutFileDir(szOutDir);
    szMainOutput = MakeFilename("%s/%s", szOutDir, "%.bin");
  }

  /*
   * Predefined symbols: this is the .rcp language, and in particular
   * it's being read by PilRC.
   */
  AddSym("__rcp__", 1);
  AddSym("__PILRC__", 1);

  if (vfTrackDepends)
  {
    InitDependsList();
  }

  OpenResFile(szResFile);

  vIn.file.szFilename = NULL;

  ParseRcpFile(szIn, prcpfile);

  if (vfTrackDepends)
  {
    char *szDependsFile = MakeFilename("%s%e.d", szIn, ".rcp");
    FILE *dependsFile = fopen(szDependsFile, "w");
    if (dependsFile)
    {
        OutputDependsList(dependsFile, szMainOutput);
        if (szOutputHeaderFile)
            OutputDependsList(dependsFile, szOutputHeaderFile);
        fclose(dependsFile);
    }
    else
    {
        // FIXME: error
    }
    free(szDependsFile);
  }

  if (!vfInhibitOutput && szOutputHeaderFile)
  {
    WriteIncFile(szOutputHeaderFile);
  }

#ifdef PALM_INTERNAL
  if ((vfAutoAmdc) && (szDllNameP))
  {
    char *pchString = MakeFilename("%s.dll", szDllNameP);
    OpenOutput("amdc", 1);
    DumpBytes(pchString, strlen(pchString) + 1);
    CloseOutput();
    free(pchString);
  }
#endif

    
  free(szOutputHeaderFile);
  if (vfTrackDepends)
  {
    FreeDependsList();
  }
  FreeSymTable();
  FreeTranslations();
  CloseResFile();
  if (vfPrc)
    CloseResDBFile();
  else
    SetOutFileDir(NULL);
  free(szMainOutput);
  return prcpfile;
}
