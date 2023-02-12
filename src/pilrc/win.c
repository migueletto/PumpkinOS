
/*
 * @(#)win.c
 *
 * Copyright 1997-1999, Wes Cherry   (mailto:wesc@technosis.com)
 *           2000-2003, Aaron Ardiri (mailto:aaron@ardiri.com)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>

#define X86
#pragma pack(2)

#include "pilrc.h"
#include "resource.h"
#include "afxres.h"

#define dxScreen 160
#define dyScreen 160
// globals

HANDLE hinstApp;
HWND hwndApp;
char szClass[] = "pilrc";
char szAppName[] = "PilRC";
char szFileCur[128];
HFONT rghfont[8];

int ifrmCur = 0;
int wZoom = 1;

RCPFILE *vprcpfile;

void
SetRcFromPilotRc(RECT * prc,
                 RCRECT * prcPilot)
{
  prc->left = prcPilot->topLeft.x;
  prc->top = prcPilot->topLeft.y;
  prc->right = prcPilot->topLeft.x + prcPilot->extent.x;
  prc->bottom = prcPilot->topLeft.y + prcPilot->extent.y;
}

#define ptxLeft   0x0000
#define ptxCenter 0x0001
#define ptxInvert 0x0002
#define ptxNoExtent 0x0004
#define ptxCalcWidth 0x0008
void
PilotText(HDC hdc,
          char *sz,
          RCRECT * prc,
          int fontId,
          int ptx)
{
  RECT rcClip;
  int x;
  int y;
  SIZE size;
  int cch;
  int rgdx[256];
  int eto;
  HFONT hfontSav;

  cch = strlen(sz);

  hfontSav = SelectObject(hdc, rghfont[fontId]);
  GetTextExtentPoint(hdc, sz, cch, &size);
  size.cx = DxCalcRgdx(sz, fontId, rgdx);
  if (ptx & ptxCalcWidth)
    prc->extent.x = (short)size.cx;
  SetRcFromPilotRc(&rcClip, prc);
  x = prc->topLeft.x;
  y = prc->topLeft.y;
  if (ptx & ptxCenter)
  {
    x += (prc->extent.x - size.cx + 1) / 2;
    y += (prc->extent.y - size.cy + 1) / 2;
  }
  if (ptx & ptxInvert)
  {
    SetTextColor(hdc, RGB(0xff, 0xff, 0xff));
    SetBkColor(hdc, RGB(0, 0, 0));
  }
  eto = 0;
  if (!(ptx & ptxNoExtent))
    eto |= ETO_CLIPPED;
  if (ptx & ptxInvert)
    eto |= ETO_OPAQUE;
  ExtTextOut(hdc, x, y, eto, &rcClip, sz, cch, rgdx);
  if (ptx & ptxInvert)
  {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, RGB(0xff, 0xff, 0xff));
  }
  SelectObject(hdc, hfontSav);
}

void
DrawForm(HDC hdc,
         RCPFILE * prcpfile,
         int ifrm)
{
  HBRUSH hbr;
  HBRUSH hbrNull;
  HPEN hpenSav;
  HFONT hfontSav;
  int ilt;
  HPEN rghpen[5];
  RCFORM *pform;
  PLEXFORMOBJLIST *pplt;
  POINT ptSav;
  FRM *pfrm;

  if (prcpfile == NULL || ifrm >= PlexGetCount(&prcpfile->plfrm))
    return;
  pfrm = PlexGetElementAt(&prcpfile->plfrm, ifrm);

  pform = &pfrm->form;
  pplt = &pfrm->pllt;

  hbr = SelectObject(hdc, GetStockObject(WHITE_BRUSH));
  hbrNull = GetStockObject(NULL_BRUSH);
  rghpen[0] = CreatePen(PS_NULL, 0, 0L);
  rghpen[1] = CreatePen(PS_SOLID, 1, 0L);
  rghpen[2] = CreatePen(PS_SOLID, 2, 0L);
  rghpen[3] = CreatePen(PS_SOLID, 3, 0L);
  rghpen[4] = CreatePen(PS_DOT, 1, 0L);          // BUG! this isn't an every-other pixel pen

  rghfont[0] =
    CreateFont(-8 * GetDeviceCaps(hdc, LOGPIXELSY) / 72, 0, 0, 0, 400, 0,
               0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
               DEFAULT_QUALITY, VARIABLE_PITCH | FF_DONTCARE, "Arial Narrow");
  rghfont[1] =
    CreateFont(-8 * GetDeviceCaps(hdc, LOGPIXELSY) / 72, 0, 0, 0, 700, 0,
               0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
               DEFAULT_QUALITY, VARIABLE_PITCH | FF_DONTCARE, "Arial Narrow");
  rghfont[2] =
    CreateFont(-10 * GetDeviceCaps(hdc, LOGPIXELSY) / 72, 0, 0, 0, 700, 0,
               0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
               DEFAULT_QUALITY, VARIABLE_PITCH | FF_DONTCARE, "Arial Narrow");

  hfontSav = SelectObject(hdc, rghfont[0]);
  hpenSav = SelectObject(hdc, rghpen[pform->window.frameType.width]);
  if (pform->window.frameType.cornerDiam)
  {
    RoundRect(hdc,
              pform->window.windowBounds.topLeft.x,
              pform->window.windowBounds.topLeft.y,
              pform->window.windowBounds.topLeft.x +
              pform->window.windowBounds.extent.x + 1,
              pform->window.windowBounds.topLeft.y +
              pform->window.windowBounds.extent.y + 1,
              pform->window.frameType.cornerDiam,
              pform->window.frameType.cornerDiam);
  }
  else
  {
    Rectangle(hdc,
              pform->window.windowBounds.topLeft.x,
              pform->window.windowBounds.topLeft.y,
              pform->window.windowBounds.topLeft.x +
              pform->window.windowBounds.extent.x + 1,
              pform->window.windowBounds.topLeft.y +
              pform->window.windowBounds.extent.y + 1);
  }
  SetWindowOrgEx(hdc,
                 -(pform->window.windowBounds.topLeft.x -
                   pform->window.frameType.width),
                 -(pform->window.windowBounds.topLeft.y -
                   pform->window.frameType.width), &ptSav);
  SelectObject(hdc, rghpen[1]);
  SelectObject(hdc, hbrNull);
  for (ilt = 0; ilt < pform->numObjects; ilt++)
  {
    RCFORMOBJLIST *plt;
    RCFORMOBJECT *pobj;
    RCCONTROL ctl;
    RCFORMTITLE title;
    RCFIELD field;
    RCLIST list;

    //RCTABLE table;
    RCFORMBITMAP bitmap;
    RCFORMLABEL label;
    RCFORMPOPUP popup;
    RCFORMGRAFFITISTATE grfState;
    RCFORMGADGET gadget;
    RCRECT rc;
    char *pchText;
    HPEN hpenT;

    plt = PlexGetElementAt(pplt, ilt);
    pobj = &plt->u.object;
    pchText = NULL;
    switch (plt->objectType)
    {
      case frmFieldObj:
        field = *pobj->field;
        if (!field.attr.usable)
          break;
        if (field.attr.underlined)
        {
          int dyStep;

          dyStep = field.fontID == 2 ? 14 : 10;  // BUG! this is a guess on large font

          hpenT = SelectObject(hdc, rghpen[4]);
          if (field.attr.singleLine)
          {
            if (dyStep < field.rect.extent.y)
            {
              MoveToEx(hdc, field.rect.topLeft.x,
                       field.rect.topLeft.y + dyStep, NULL);
              LineTo(hdc, field.rect.topLeft.x + field.rect.extent.x,
                     field.rect.topLeft.y + dyStep);
            }
          }
          else
          {
            int dy;

            for (dy = dyStep; dy < field.rect.extent.y; dy += dyStep)
            {
              MoveToEx(hdc, field.rect.topLeft.x,
                       field.rect.topLeft.y + dy, NULL);
              LineTo(hdc, field.rect.topLeft.x + field.rect.extent.x,
                     field.rect.topLeft.y + dy);
            }
          }
          SelectObject(hdc, hpenT);
        }
        break;
      case frmControlObj:
        ctl = *pobj->control;
        if (!ctl.attr.usable)
          break;
        //                      ctl.text = (CharPtr) IbOut() + cbLt;
        pchText = pobj->control->u.text;         // RNi
        switch (ctl.style)
        {
          case buttonCtl:
            RoundRect(hdc, ctl.bounds.topLeft.x - 1,
                      ctl.bounds.topLeft.y - 1,
                      ctl.bounds.topLeft.x + ctl.bounds.extent.x + 1,
                      ctl.bounds.topLeft.y + ctl.bounds.extent.y + 1, 7, 7);
            PilotText(hdc, pchText, &ctl.bounds, ctl.font, ptxCenter);
            break;
          case selectorTriggerCtl:
            hpenT = SelectObject(hdc, rghpen[4]);
            // fall thru
          case repeatingButtonCtl:
          case pushButtonCtl:
            Rectangle(hdc, ctl.bounds.topLeft.x, ctl.bounds.topLeft.y,
                      ctl.bounds.topLeft.x + ctl.bounds.extent.x + 1,
                      ctl.bounds.topLeft.y + ctl.bounds.extent.y + 1);
            PilotText(hdc, pchText, &ctl.bounds, ctl.font, ptxCenter);
            if (plt->objectType == selectorTriggerCtl)
              SelectObject(hdc, hpenT);

            SelectObject(hdc, rghpen[1]);
            break;
          case checkboxCtl:
            {
              int y;

#define dxyCheck 8
              y = ctl.bounds.topLeft.y + 2;
              if (ctl.font == 2)
                y += 2;

              Rectangle(hdc, ctl.bounds.topLeft.x,
                        y, ctl.bounds.topLeft.x + dxyCheck, y + dxyCheck);
              ctl.bounds.topLeft.x += 14;
              ctl.bounds.extent.x -= 14;
              PilotText(hdc, pchText, &ctl.bounds, ctl.font, ptxLeft);
              // BUG! draw check
            }
            break;
          case popupTriggerCtl:
            {
              int x;
              int y;
              int dy;

              x = ctl.bounds.topLeft.x + 3;
              y =
                ctl.bounds.topLeft.y + (ctl.bounds.extent.y - 4) / 2 + 4 - 1;
              for (dy = 0; dy < 4; dy++)
              {
                PatBlt(hdc, x, y - dy, dy * 2 + 1, 1, BLACKNESS);
                x--;
              }
              ctl.bounds.topLeft.x += 10;
              ctl.bounds.extent.x -= 10;
              PilotText(hdc, pchText, &ctl.bounds, ctl.font, ptxLeft);  // BUG! ANCHOR
            }
            break;

        }
        break;
      case frmListObj:
        list = *pobj->list;
        if (!list.attr.usable)
          break;
        Rectangle(hdc, list.bounds.topLeft.x - 1,
                  list.bounds.topLeft.y - 1,
                  list.bounds.topLeft.x + list.bounds.extent.x + 1,
                  list.bounds.topLeft.y + list.bounds.extent.y + 1);

        //list.itemsText = (CharPtr *) IbOut()+cbLt;
        //SwapStruct(&list, "B12L1B12");
        break;
      case frmTableObj:
        //                      table = *pobj->table;
        //                      Assert(fFalse);
        break;
      case frmBitmapObj:
        bitmap = *pobj->bitmap;
        if (!bitmap.attr.usable)
          break;
        break;
        //              case frmLineObj:
        //                      break;
        //              case frmFrameObj:
        //                      break;
        //              case frmRectangleObj:
        //                      break;
      case frmLabelObj:
        label = *pobj->label;
        if (!label.attr.usable)
          break;
        pchText = pobj->label->text;
        rc.topLeft = label.pos;
        PilotText(hdc, pchText, &rc, label.fontID, ptxLeft | ptxNoExtent);
        break;
      case frmTitleObj:
        title = *pobj->title;
        pchText = pobj->title->text;
        //rc = pform->window.windowBounds;
        rc.topLeft.x = pform->window.frameType.width;
        rc.topLeft.y = pform->window.frameType.width;
        rc.extent.x = pform->window.windowBounds.extent.x;
        rc.extent.y = 12;
        if (pform->window.frameType.cornerDiam == 3 && pform->window.frameType.width == 2)      // dialogFrame
        {
          rc.topLeft.x++;
          rc.extent.x -= 2;
          PilotText(hdc, pchText, &rc, 1, ptxInvert | ptxCenter);
        }
        else
        {
          RCRECT rcT;

          rcT = rc;
          PilotText(hdc, pchText, &rcT, 1,
                    ptxInvert | ptxLeft | ptxCalcWidth);
          PatBlt(hdc, rc.topLeft.x, rc.extent.y, rc.extent.x, 2, BLACKNESS);
        }
        break;
      case frmPopupObj:
        popup = *pobj->popup;
        // no ui, ez
        break;
      case frmGraffitiStateObj:
        grfState = *pobj->grfState;
        rc.topLeft = grfState.pos;
        PilotText(hdc, "Caps", &rc, 0, ptxLeft | ptxNoExtent);

        hpenT = SelectObject(hdc, rghpen[2]);
        MoveToEx(hdc, grfState.pos.x + 7, grfState.pos.y + 4, NULL);
        LineTo(hdc, grfState.pos.x + 11, grfState.pos.y);

        MoveToEx(hdc, grfState.pos.x + 10, grfState.pos.y, NULL);
        LineTo(hdc, grfState.pos.x + 14, grfState.pos.y + 4);
        // body
        MoveToEx(hdc, grfState.pos.x + 11, grfState.pos.y, NULL);
        LineTo(hdc, grfState.pos.x + 11, grfState.pos.y + 10);
        SelectObject(hdc, hpenT);

        break;
      case frmGadgetObj:
        gadget = *pobj->gadget;
        if (!gadget.attr.usable)
          break;

        break;
    }
  }
  SelectObject(hdc, hfontSav);
  DeleteObject(rghfont[0]);
  DeleteObject(rghfont[1]);
  SelectObject(hdc, hbr);
  SelectObject(hdc, hpenSav);
  DeleteObject(rghpen[0]);
  DeleteObject(rghpen[1]);
  DeleteObject(rghpen[2]);
  DeleteObject(rghpen[3]);
  SetWindowOrgEx(hdc, ptSav.x, ptSav.y, NULL);
}

void
PilrcPaint(HWND hwnd)
{
  PAINTSTRUCT ps;
  RECT rc;
  HDC hdc;
  HBITMAP hbm;
  HBITMAP hbmSav;

  BeginPaint(hwnd, &ps);
  hdc = ps.hdc;
  GetClientRect(hwnd, &rc);
  if (wZoom != 1)
  {
    hdc = CreateCompatibleDC(ps.hdc);
    hbm = CreateCompatibleBitmap(ps.hdc, dxScreen, dyScreen);
    hbmSav = SelectObject(hdc, hbm);
    PatBlt(hdc, 0, 0, dxScreen, dyScreen, WHITENESS);
  }
  else
    PatBlt(hdc, 0, 0, rc.right, rc.bottom, WHITENESS);
  DrawForm(hdc, vprcpfile, ifrmCur);             //&rgfrm[ifrmCur]);
  if (wZoom != 1)
  {
    StretchBlt(ps.hdc, 0, 0, rc.right, rc.bottom, hdc, 0, 0, dxScreen,
               dyScreen, SRCCOPY);
    SelectObject(hdc, hbmSav);
    DeleteObject(hbm);
    DeleteDC(hdc);
  }
  //TextOut(ps.hdc, 0, 0, "Hi There", 8);
  EndPaint(hwnd, &ps);

}

BOOL
FPromptFileName(OPENFILENAME * pofn,
                char *szFileName,
                char *szTitle,
                char *szFilter,
                char *szDefExt,
                int ofn,
                BOOL fOpen)
{
  int w;

  szFileName[0] = '\000';

  /*
   * fill in non-variant fields of OPENFILENAME struct. 
   */
  pofn->lStructSize = sizeof(OPENFILENAME);
  pofn->hwndOwner = hwndApp;
  pofn->lpstrFilter = szFilter;
  pofn->lpstrCustomFilter = NULL;
  pofn->nMaxCustFilter = 0;
  pofn->nFilterIndex = 1;
  pofn->lpstrFile = szFileName;
  pofn->nMaxFile = 120;
  pofn->lpstrInitialDir = NULL;
  pofn->lpstrFileTitle = NULL;
  pofn->nMaxFileTitle = 120;
  pofn->lpstrTitle = szTitle;
  pofn->lpstrDefExt = szDefExt;
  pofn->Flags = ofn;
  pofn->lpfnHook = NULL;
  pofn->lCustData = 0;
  pofn->lpTemplateName = 0;

  if (fOpen)
    w = GetOpenFileName((LPOPENFILENAME) pofn);
  else
    w = GetSaveFileName(pofn);
  return w;
}

void
RedrawWin(HWND hwnd)
{
  InvalidateRect(hwnd, NULL, TRUE);
  UpdateWindow(hwnd);
}

void
Reload()
{
  char sz[256];
  extern BOOL vfErr;

  FreeRcpfile(vprcpfile);

  vfErr = fFalse;
  if (strlen(szFileCur) > 0)
  {
    vprcpfile = ParseFile(szFileCur, "c:\\tmp", NULL, NULL, fontDefault);
    wsprintf(sz, "PilRc - %s", szFileCur);
    SetWindowText(hwndApp, sz);
    RedrawWin(hwndApp);
  }
}

char szFilterSpecOpen[128] =                     /* file type filters */
  "RCP - Pilot Resource\0*.RCP\0All Files\0*.*\0";

VOID
PromptOpenFile()
{
  OPENFILENAME ofn;

  if (FPromptFileName
      (&ofn, szFileCur, "Open Pilot Resource Script...", szFilterSpecOpen,
       "RCP", OFN_FILEMUSTEXIST, fTrue))
  {
    Reload();
  }
}

void
SizeWin(HWND hwnd)
{
  RECT rc;

  rc.left = 0;
  rc.top = 0;
  rc.right = dxScreen * wZoom;
  rc.bottom = dyScreen * wZoom;
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

  SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
               SWP_NOZORDER | SWP_NOMOVE);
}

BOOL FAR PASCAL
About(HWND hdlg,
      UINT wm,
      WPARAM wParam,
      LPARAM lParam)
{
  switch (wm)
  {
    case WM_INITDIALOG:
      return (TRUE);

    case WM_COMMAND:
      if (wParam == IDOK || wParam == IDCANCEL)
      {
        EndDialog(hdlg, TRUE);
        return (TRUE);
      }
      break;
  }
  return (FALSE);
}

VOID
DoAbout(VOID)
{
  FARPROC lpprocAbout;

  lpprocAbout = MakeProcInstance(About, vhinstApp);

  DialogBox((HANDLE) hinstApp, MAKEINTRESOURCE(IDD_ABOUT), hwndApp,
            (DLGPROC) lpprocAbout);
  FreeProcInstance(lpprocAbout);
}

void
PilrcCmd(HWND hwnd,
         int wParam)
{
  switch (wParam)
  {
    case ID_FILE_EXIT:
      PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
      break;
    case ID_FILE_OPEN_PILRC:
      PromptOpenFile();
      break;
    case ID_FILE_RELOAD:
      Reload();
      break;
    case ID_OPTIONS_ZOOM:
      wZoom = wZoom == 1 ? 2 : 1;
      SizeWin(hwnd);
      break;
    case ID_FILE_ABOUT:
      DoAbout();
      break;
  }
  if (wParam >= ID_FORM0 && wParam < ID_FORMMAX)
  {
    ifrmCur = wParam - ID_FORM0;
    RedrawWin(hwnd);
  }
}

void
PilrcInitMenuPopup(HWND hwnd,
                   HMENU hmenu,
                   int imenu)
{
  switch (imenu)
  {
    case 1:                                     // Options
      CheckMenuItem(hmenu, ID_OPTIONS_ZOOM,
                    (wZoom == 1 ? MF_UNCHECKED : MF_CHECKED) | MF_BYCOMMAND);
      break;
    case 2:                                     // Form
      {
        int ifrm;

        // first delete everything
        while (DeleteMenu(hmenu, 0, MF_BYPOSITION)) ;
        if (vprcpfile != NULL)
        {
          int ifrmMac;

          ifrmMac = PlexGetCount(&vprcpfile->plfrm);
          for (ifrm = 0; ifrm < ifrmMac; ifrm++)
          {
            char sz[32];
            FRM *pfrm;

            pfrm = (FRM *) PlexGetElementAt(&vprcpfile->plfrm, ifrm);
            wsprintf(sz, "tFRM%04x", pfrm->form.formId);
            AppendMenu(hmenu, MF_STRING | MF_ENABLED, ID_FORM0 + ifrm, sz);
            if (ifrm == ifrmCur)
              CheckMenuItem(hmenu, ifrm, MF_BYPOSITION | MF_CHECKED);
          }
        }
      }
      break;
  }
}

int PASCAL
WinMain(HINSTANCE hinst,
        HINSTANCE hinstPrev,
        LPSTR lpszCmdLine,
        int nCmdShow)
{
  MSG msg;
  RECT rc;
  WNDCLASS wndclass;
  long FAR PASCAL PilrcWndProc(HWND hwnd,
                               unsigned wm,
                               WPARAM wParam,
                               LPARAM lParam);
  extern int __argc;
  extern char **__argv;
  extern int main(int iargMac,
                  char **rgszArg);

#ifdef THISDOESNTWORK
  if (__argc > 0 && strcmp(__argv[1], "-p") == 0)
  {
    int w;
    int hCrt;
    FILE *hf;
    STARTUPINFO si;

    GetStartupInfo(&si);

    //  AllocConsole();
    hCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
    hf = _fdopen(hCrt, "w");
    *stdout = *hf;
    setvbuf(stdout, NULL, _IONBF, 0);

    //              AllocConsole();

    //              stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    //stdin = GetStdHandle(STD_INPUT_HANDLE);
    //stderr = GetStdHandle(STD_ERROR_HANDLE);

    w = main(__argc, __argv);
    FreeConsole();
    return w;
  }
#endif
  vfWinGUI = fTrue;
  vfAutoId = fTrue;                              // ??? do I want to do this?
  hinstApp = hinst;
  //      InitWin(hinst);
  if (hinstPrev == (HANDLE) NULL)
  {
    wndclass.style = CS_VREDRAW | CS_HREDRAW;
    wndclass.lpfnWndProc = PilrcWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hinst;
    wndclass.hIcon = NULL;                       // LoadIcon(hinstApp, MAKEINTRESOURCE(idiPilrc));
    wndclass.hCursor = LoadCursor((HANDLE) NULL, IDC_ARROW);
    wndclass.hbrBackground = GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wndclass.lpszClassName = szClass;

    if (!RegisterClass(&wndclass))
      return FALSE;
  }

  rc.left = 0;
  rc.top = 0;
  rc.right = dxScreen;
  rc.bottom = dyScreen;
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
  hwndApp = CreateWindow(szClass, szAppName,
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
                         rc.bottom - rc.top, (HWND) NULL, (HMENU) NULL,
                         hinst, NULL);

  strcpy(szFileCur, lpszCmdLine);
  Reload();

  //      ReadIni();
  ShowWindow(hwndApp, nCmdShow);
  UpdateWindow(hwndApp);
  //SetTimer(hwndApp, 666, 10, NULL);

  while (GetMessage(&msg, (HWND) NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  //FreeBmps();
  // SaveIni();
  return msg.wParam;
}

long FAR PASCAL
PilrcWndProc(HWND hwnd,
             unsigned wm,
             WPARAM wParam,
             LPARAM lParam)
{
  extern BOOL vf32;

#define hmenu ((HMENU) (wParam))

  switch (wm)
  {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0L;
    case WM_COMMAND:
      PilrcCmd(hwnd, wParam);
      break;
    case WM_INITMENUPOPUP:
      if (!(HIWORD(lParam)))
        PilrcInitMenuPopup(hwnd, (HMENU) wParam, LOWORD(lParam));
      break;
    case WM_LBUTTONDOWN:
      Reload();
      break;
#ifdef LATER
    case WM_SIZE:
      PilrcSize(hwnd);
      break;

    case WM_KEYDOWN:
      PilrcKey(hwnd, wParam);
      break;
#endif
    case WM_PAINT:
      PilrcPaint(hwnd);
      return 0L;
      break;
  }
  return DefWindowProc(hwnd, wm, wParam, lParam);
}

#endif                                           // WINGUI
