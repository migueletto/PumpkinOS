#include <PalmOS.h>

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "sys.h"
#include "script.h"
#include "os_shell.h"
#include "debug.h"

#define MAX_CMD 128

// Based on:
// https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

int os_shell(int argc, char *argv[]) {
  SECURITY_ATTRIBUTES saAttr;
  HANDLE hChildStd_IN_Rd = NULL;
  HANDLE hChildStd_IN_Wr = NULL;
  HANDLE hChildStd_OUT_Rd = NULL;
  HANDLE hChildStd_OUT_Wr = NULL;
  TCHAR szCmdline[] = TEXT("C:\\WINDOWS\\system32\\cmd.exe");
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFO siStartInfo;
  DWORD dwWritten, dwRead, available;
  BOOL bSuccess;
  char ch, buf[128], cmd[MAX_CMD], *s;
  int i, n, icmd, ignore, cols, rows;

  debug(DEBUG_INFO, "WSHELL", "oshell start");

  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) {
    debug(DEBUG_ERROR, "WSHELL", "CreatePipe ChildStd_OUT failed");
    return -1;
  }

  if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
    debug(DEBUG_ERROR, "WSHELL", "SetHandleInformation ChildStd_OUT failed");
    CloseHandle(hChildStd_OUT_Rd);
    CloseHandle(hChildStd_OUT_Wr);
    return -1;
  }

  if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) {
    debug(DEBUG_ERROR, "WSHELL", "CreatePipe ChildStd_IN failed");
    CloseHandle(hChildStd_OUT_Rd);
    CloseHandle(hChildStd_OUT_Wr);
    return -1;
  }

  if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
    debug(DEBUG_ERROR, "WSHELL", "SetHandleInformation ChildStd_IN failed");
    CloseHandle(hChildStd_OUT_Rd);
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(hChildStd_IN_Rd);
    CloseHandle(hChildStd_IN_Wr);
    return -1;
  }

  ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.hStdError = hChildStd_OUT_Wr;
  siStartInfo.hStdOutput = hChildStd_OUT_Wr;
  siStartInfo.hStdInput = hChildStd_IN_Rd;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

  bSuccess = CreateProcess(NULL,
    szCmdline,     // command line
    NULL,          // process security attributes
    NULL,          // primary thread security attributes
    TRUE,          // handles are inherited
    CREATE_NO_WINDOW,             // creation flags
    NULL,          // use parent's environment
    NULL,          // use parent's current directory
    &siStartInfo,  // STARTUPINFO pointer
    &piProcInfo);  // receives PROCESS_INFORMATION

  CloseHandle(hChildStd_OUT_Wr);
  CloseHandle(hChildStd_IN_Rd);

  if (!bSuccess) {
    debug(DEBUG_ERROR, "WSHELL", "CreateProcess failed");
    CloseHandle(hChildStd_OUT_Rd);
    CloseHandle(hChildStd_IN_Wr);
    return -1;
  }

  CloseHandle(piProcInfo.hProcess);
  CloseHandle(piProcInfo.hThread);

  p = script_get_pointer(pe, SHELL_PROVIDER);
  p->window(shell, &cols, &rows);
  debug(DEBUG_INFO, "WSHELL", "window size is %dx%d", cols, rows);

  // clear screen
  buf[0] = 0x0C;
  pumpkin_write(buf, 1);

  for (icmd = 0, ignore = 0;;) {
    if ((n = pumpkin_haschar()) == -1) {
      break;
    }

    if (n > 0) {
      if ((buf[0] = pumpkin_getchar()) == -1) {
        break;
      }
      n = 1;

      if (n > 0) {
        for (i = 0; i < n; i++) {
          if (icmd < MAX_CMD-1) {
            switch (buf[i]) {
              case '\b':
                if (icmd) {
                  pumpkin_write(&buf[i], 1);
                  icmd--;
                }
                break;
              case '\r':
                cmd[icmd++] = '\n';
                if (!WriteFile(hChildStd_IN_Wr, cmd, icmd, &dwWritten, NULL)) {
                  debug(DEBUG_ERROR, "WSHELL", "WriteFile failed");
                }
                ignore = icmd;
                icmd = 0;
                buf[0] = '\r';
                buf[1] = '\n';
                pumpkin_write(buf, 2);
                break;
              default:
                if (buf[i] >= 32) {
                  cmd[icmd++] = buf[i];
                  pumpkin_write(&buf[i], 1);
                }
                break;
            }
          }
        }
      }
    }

    if (!PeekNamedPipe(hChildStd_OUT_Rd, NULL, 0, NULL, &available, NULL)) {
      debug(DEBUG_ERROR, "WSHELL", "PeekNamedPipe failed");
      break;
    }

    if (available) {
      if (!ReadFile(hChildStd_OUT_Rd, buf, sizeof(buf), &dwRead, NULL)) {
        debug(DEBUG_ERROR, "WSHELL", "ReadFile failed");
        break;
      }

      if (dwRead > 0) {
        for (s = buf, i = 0; ignore > 0 && i < dwRead; ignore--, i++) s++;
        if (dwRead > i) {
          pumpkin_write(s, dwRead - i);
        }
      }
    }
  }

  // clear screen
  buf[0] = 0x0C;
  pumpkin_write(buf, 1);

  CloseHandle(hChildStd_OUT_Rd);
  CloseHandle(hChildStd_IN_Wr);

  debug(DEBUG_INFO, "WSHELL", "oshell end");

  return 0;
}
