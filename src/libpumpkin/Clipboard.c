#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

#define PALMOS_MODULE "Clipboard"

typedef struct {
  MemHandle textHandle;
  char text[cbdMaxTextLength*2];
} clp_module_t;

int ClpInitModule(void) {
  clp_module_t *module;

  if ((module = xcalloc(1, sizeof(clp_module_t))) == NULL) {
    return -1;
  }

  pumpkin_set_local_storage(clp_key, module);

  return 0;
}

int ClpFinishModule(void) {
  clp_module_t *module = (clp_module_t *)pumpkin_get_local_storage(clp_key);

  if (module) {
    if (module->textHandle) {
      MemHandleFree(module->textHandle);
    }
    xfree(module);
  }

  return 0;
}

static UInt16 ClipboardToUTF8(UInt8 *dst, UInt8 *src, UInt16 length) {
  UInt16 i, j;

  for (i = 0, j = 0; i < length && j < cbdMaxTextLength*2 - 4; i++) {
    switch (src[i]) {
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
      case 0x19:
        dst[j++] = ' ';
        break;
      case 0x80: // euro sign
        dst[j++] = 0xE2;
        dst[j++] = 0x82;
        dst[j++] = 0xAC;
        break;
      case 0x81:
        dst[j++] = ',';
        break;
      case 0x83:
        dst[j++] = 0xC6;
        dst[j++] = 0x92;
        break;
      case 0x18:
      case 0x85:
        dst[j++] = '.';
        dst[j++] = '.';
        dst[j++] = '.';
        break;
      case 0x89:
        dst[j++] = 0xE2;
        dst[j++] = 0x80;
        dst[j++] = 0xB0;
        break;
      case 0x8A:
        dst[j++] = 0xC5;
        dst[j++] = 0xA0;
        break;
      case 0x8C:
        dst[j++] = 0xC5;
        dst[j++] = 0x92;
        break;
      case 0x95:
        dst[j++] = 0xC2;
        dst[j++] = 0xB7;
        break;
      case 0x97:
        dst[j++] = 0xE2;
        dst[j++] = 0x80;
        dst[j++] = 0x94;
        break;
      case 0x99:
        dst[j++] = 0xE2;
        dst[j++] = 0x84;
        dst[j++] = 0xA2;
        break;
      case 0x9C:
        dst[j++] = 0xC5;
        dst[j++] = 0x93;
        break;
      default:
        if (src[i] >= 0x80 && src[i] < 0xA0) {
          dst[j++] = ' ';
        } else if (src[i] >= 0xA0 && src[i] < 0xC0) {
          dst[j++] = 0xC2;
          dst[j++] = src[i];
        } else if (src[i] >= 0xC0) {
          dst[j++] = 0xC3;
          dst[j++] = src[i];
        } else {
          dst[j++] = src[i];
        }
        break;
    }
  }

  return j;
}

// Clipboards for each type of data are separately maintained. That is,
// if you add a string of text to the clipboard, then add a bitmap, then
// ask to retrieve a clipboardText item from the clipboard, you will
// receive the string you added before the bitmap; the bitmap does not
// overwrite textual data and vice versa.

// The clipboard makes a copy of the data that you pass to this
// function. Thus, you may free any data that you’ve passed to the
// clipboard without destroying the contents of the clipboard. You may
// also add constant data or stack-based data to the clipboard.
// WARNING! You can’t add null-terminated strings to the clipboard.

void ClipboardAddItem(const ClipboardFormatType format, const void *ptr, UInt16 length) {
  clp_module_t *module = (clp_module_t *)pumpkin_get_local_storage(clp_key);

  switch (format) {
    case clipboardText:
      if (ptr && length) {
        length = ClipboardToUTF8((UInt8 *)module->text, (UInt8 *)ptr, length);
        if (pumpkin_clipboard_add_text(module->text, length) != 0) {
          debug(DEBUG_ERROR, PALMOS_MODULE, "ClipboardAddItem text error");
        }
      }
      break;

    case clipboardBitmap:
      if (ptr && length) {
        if (pumpkin_clipboard_add_bitmap((BitmapType *)ptr, length) != 0) {
          debug(DEBUG_ERROR, PALMOS_MODULE, "ClipboardAddItem bitmap error");
        }
      }
      break;

    default:
      debug(DEBUG_ERROR, PALMOS_MODULE, "ClipboardAddItem format %d not supported");
      break;
  }
}

Err ClipboardAppendItem(const ClipboardFormatType format, const void *ptr, UInt16 length) {
  clp_module_t *module = (clp_module_t *)pumpkin_get_local_storage(clp_key);
  Err err = memErrNotEnoughSpace;

  switch (format) {
    case clipboardText:
      if (ptr && length) {
        length = ClipboardToUTF8((UInt8 *)module->text, (UInt8 *)ptr, length);
        if (pumpkin_clipboard_append_text(module->text, length) != 0) {
          debug(DEBUG_ERROR, PALMOS_MODULE, "ClipboardAppendItem text error");
        } else {
          err = errNone;
        }
      }
      break;

    default:
      debug(DEBUG_ERROR, PALMOS_MODULE, "ClipboardAppendItem format %d not supported");
      break;
  }

  return err;
}

// The handle returned is a handle to the actual clipboard chunk. It is
// not suitable for passing to any API that modifies memory (such as
// FldSetTextHandle()). Consider this to be read-only access to the
// chunk. Copy the contents of the clipboard to your application’s own
// storage as soon as possible and use that reference instead of the
// handle returned by this function.
// Don't free the handle returned by this function; it is freed when a
// new item is added to the clipboard.
// Text retrieved from the clipboard does not have a null terminator.
// You must use the length parameter to determine the length in
// bytes of the string you’ve retrieved.

MemHandle ClipboardGetItem(const ClipboardFormatType format, UInt16 *length) {
  clp_module_t *module = (clp_module_t *)pumpkin_get_local_storage(clp_key);
  MemHandle h = NULL;
  char *s;
  int len;

  if (length) *length = 0;

  switch (format) {
    case clipboardText:
      if (module->textHandle) {
        MemHandleFree(module->textHandle);
        module->textHandle = NULL;
      }

      len = cbdMaxTextLength;
      if (pumpkin_clipboard_get_text(module->text, &len) == 0) {
        if ((h = MemHandleNew(len+1)) != NULL) {
          if ((s = MemHandleLock(h)) != NULL) {
            MemMove(s, module->text, len);
            MemHandleUnlock(h);
            module->textHandle = h;
            if (length) *length = len;
          }
        }
      } else {
        debug(DEBUG_ERROR, PALMOS_MODULE, "ClipboardGetItem error");
      }
      break;

    case clipboardBitmap:
      break;

    default:
      debug(DEBUG_ERROR, PALMOS_MODULE, "ClipboardGetItem format %d not supported");
      break;
  }

  return h;
}
