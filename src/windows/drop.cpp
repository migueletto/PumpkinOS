#define STRICT

#include <windows.h>

#include <PalmOS.h>

#include "sys.h"
#include "drop.h"
#include "thread.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

static bool DropData(HWND hwnd, IDataObject *pDataObject);

class FileDropTarget : public IDropTarget {

public:
  // IUnknown implementation
  HRESULT __stdcall QueryInterface(REFIID iid, void **ppvObject);
  ULONG __stdcall AddRef(void);
  ULONG __stdcall Release(void);

  // IDropTarget implementation
  HRESULT __stdcall DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
  HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
  HRESULT __stdcall DragLeave(void);
  HRESULT __stdcall Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

  // Constructor
  FileDropTarget(HWND hwnd);
  ~FileDropTarget();

private:
  bool QueryDataObject(IDataObject *pDataObject);

  LONG m_lRefCount;
  HWND m_hwnd;
  bool m_fAllowDrop;
};

FileDropTarget::FileDropTarget(HWND hwnd) {
  m_lRefCount = 1;
  m_hwnd = hwnd;
  m_fAllowDrop = false;
}

FileDropTarget::~FileDropTarget() {
}

HRESULT __stdcall FileDropTarget::QueryInterface(REFIID iid, void **ppvObject) {
  if (iid == IID_IDropTarget || iid == IID_IUnknown) {
    AddRef();
    *ppvObject = this;
    return S_OK;
  } else {
    *ppvObject = 0;
    return E_NOINTERFACE;
  }
}

ULONG __stdcall FileDropTarget::AddRef(void) {
  return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall FileDropTarget::Release(void) {
  LONG count = InterlockedDecrement(&m_lRefCount);

  if (count == 0) {
    delete this;
    return 0;
  } else {
    return count;
  }
}

bool FileDropTarget::QueryDataObject(IDataObject *pDataObject) {
  FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  HRESULT r;

  r = pDataObject->QueryGetData(&fmtetc);
  debug(DEBUG_TRACE, "WIN32", "QueryGetData: 0x%08X", r);

  return SUCCEEDED(r);
}

HRESULT __stdcall FileDropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
  debug(DEBUG_TRACE, "WIN32", "DragEnter");

  m_fAllowDrop = QueryDataObject(pDataObject);

  if (m_fAllowDrop) {
    debug(DEBUG_TRACE, "WIN32", "drop allowed");
    *pdwEffect = DROPEFFECT_COPY;
    SetFocus(m_hwnd);
  } else {
    debug(DEBUG_TRACE, "WIN32", "drop not allowed");
    *pdwEffect = DROPEFFECT_NONE;
  }

  return S_OK;
}

HRESULT __stdcall FileDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
  if (m_fAllowDrop) {
    *pdwEffect = DROPEFFECT_COPY;
  } else {
    *pdwEffect = DROPEFFECT_NONE;
  }

  return S_OK;
}

HRESULT __stdcall FileDropTarget::DragLeave(void) {
  return S_OK;
}

HRESULT __stdcall FileDropTarget::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
  debug(DEBUG_TRACE, "WIN32", "Drop");

  if (m_fAllowDrop) {
    if (DropData(m_hwnd, pDataObject)) {
      *pdwEffect = DROPEFFECT_COPY;
    } else {
      *pdwEffect = DROPEFFECT_NONE;
    }
  } else {
    *pdwEffect = DROPEFFECT_NONE;
  }

  return S_OK;
}

static int getName(char *src, char *dst, int max) {
  int len, i;

  len = strlen(src);
  if (len < 2) return -1;
  i = len-1;
  if (src[i] == '\\') return -1;

  for (;;) {
    if (i == 0) break;
    if (src[i] == '\\') break;
    i--;
  }

  if (src[i] != '\\') return -1;

  sys_snprintf(dst, max, "vfs/app_install/%s", &src[i+1]);
  return 0;
}

static bool deploy(char *file) {
  uint8_t *buf;
  char name[256];
  int nr, nw, rfd, wfd = 0;
  bool r = false;

  debug(DEBUG_INFO, "WIN32", "reading \"%s\"", file);
  if (getName(file, name, sizeof(name)-1) == 0) {
    debug(DEBUG_INFO, "WIN32", "writing \"%s\"", name);
    if ((rfd = sys_open(file, SYS_READ)) != -1) {
      if ((wfd = sys_create(name, SYS_WRITE | SYS_TRUNC, 0644)) != -1) {
        if ((buf = (uint8_t *)xmalloc(65536)) != NULL) {
          for (;;) {
            nr = sys_read(rfd, buf, 65536);
            if (nr <= 0) break;
            nw = sys_write(wfd, buf, nr);
            if (nw != nr) {
              nr = -1;
              break;
            }
          }
          r = (nr == 0);
          xfree(buf);
        }
        sys_close(wfd);
      }
      sys_close(rfd);
    }
  } else {
    debug(DEBUG_ERROR, "WIN32", "invalid name \"%s\"", file);
  }

  if (!r) {
    debug(DEBUG_ERROR, "WIN32", "deploy \"%s\" failed", file);
  }

  return r;
}

static bool DropData(HWND hwnd, IDataObject *pDataObject) {
  FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM stgmed;
  HDROP hDrop;
  HRESULT r;
  client_request_t creq;
  char *buf;
  int i, numFiles, size;
  bool ok = false;

  r = pDataObject->QueryGetData(&fmtetc);
  debug(DEBUG_TRACE, "WIN32", "DropData QueryGetData: 0x%08X", r);
  if (SUCCEEDED(r)) {
    debug(DEBUG_TRACE, "WIN32", "DropData query data ok");
    r = pDataObject->GetData(&fmtetc, &stgmed);
    debug(DEBUG_TRACE, "WIN32", "DropData GetData: 0x%08X", r);
    if (SUCCEEDED(r)) {
      debug(DEBUG_TRACE, "WIN32", "DropData get data ok");
      hDrop = (HDROP)stgmed.hGlobal;
      numFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
      debug(DEBUG_TRACE, "WIN32", "DropData numFiles: %d", numFiles);
      for (i = 0; i < numFiles; i++) {
        size = DragQueryFile(hDrop, i, NULL, 0);
        debug(DEBUG_TRACE, "WIN32", "DropData file %d size: %d", i, size);
        buf = (char *)xcalloc(size+1, sizeof(char));
        DragQueryFile(hDrop, i, buf, size+1);
        debug(DEBUG_TRACE, "WIN32", "DropData file %d name: \"%s\"", i, buf);
        if (deploy(buf)) {
          ok = true;
        }
        xfree(buf);
      }
      if (ok) {
        debug(DEBUG_TRACE, "WIN32", "send MSG_DEPLOY");
        xmemset(&creq, 0, sizeof(client_request_t));
        creq.type = MSG_DEPLOY;
        thread_client_write(pumpkin_get_spawner(), (uint8_t *)&creq, sizeof(client_request_t));
      }
      ReleaseStgMedium(&stgmed);
    }
  }

  return ok;
}

void RegisterDropWindow(HWND hwnd, IDropTarget **ppDropTarget) {
  FileDropTarget *pDropTarget = new FileDropTarget(hwnd);

  CoLockObjectExternal(pDropTarget, TRUE, FALSE);

  debug(DEBUG_TRACE, "WIN32", "RegisterDropWindow");
  RegisterDragDrop(hwnd, pDropTarget);

  *ppDropTarget = pDropTarget;
}

void UnregisterDropWindow(HWND hwnd, IDropTarget *pDropTarget) {
  RevokeDragDrop(hwnd);
  CoLockObjectExternal(pDropTarget, FALSE, TRUE);
  pDropTarget->Release();
}
