#define STRICT

#include <windows.h>

#include <PalmOS.h>

#include "sys.h"
#include "drop.h"
#include "thread.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

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

  bool DropData(HWND hwnd, IDataObject *pDataObject);

  // Constructor
  FileDropTarget(HWND hwnd, void (*callback)(char *filename, void *data), void *data);
  virtual ~FileDropTarget() {}

private:
  bool QueryDataObject(IDataObject *pDataObject);

  LONG m_lRefCount;
  HWND m_hwnd;
  bool m_fAllowDrop;

  void (*drop_callback)(char *filename, void *data);
  void *drop_data;
};

FileDropTarget::FileDropTarget(HWND hwnd, void (*callback)(char *filename, void *data), void *data) {
  m_lRefCount = 1;
  m_hwnd = hwnd;
  m_fAllowDrop = false;

  drop_callback = callback;
  drop_data = data;
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

bool FileDropTarget::DropData(HWND hwnd, IDataObject *pDataObject) {
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
        drop_callback(buf, drop_data);
        xfree(buf);
        ok = true;
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

void RegisterDropWindow(HWND hwnd, IDropTarget **ppDropTarget, void (*callback)(char *filename, void *data), void *data) {
  FileDropTarget *pDropTarget = new FileDropTarget(hwnd, callback, data);

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
