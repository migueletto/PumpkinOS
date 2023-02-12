#include <PalmOS.h>

#include <time.h>
#include <sys/time.h>

#include "thread.h"
#include "pwindow.h"
#include "sys.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

/*
The insertion point is a blinking indicator that shows where text is
inserted when users write Graffiti characters or paste clipboard text.
In general, an application doesn’t need to be concerned with the
insertion point; the Palm OS UI manages the insertion point.
*/

typedef struct {
  UInt32 lastBlink, tps;
  Boolean enabled, on;
  Int16 x, y, height;
} inspt_module_t;

extern thread_key_t *ins_key;

int InsPtInitModule(void) {
  inspt_module_t *module;

  if ((module = xcalloc(1, sizeof(inspt_module_t))) == NULL) {
    return -1;
  }

  module->tps = SysTicksPerSecond();
  thread_set(ins_key, module);

  return 0;
}

int InsPtFinishModule(void) {
  inspt_module_t *module = (inspt_module_t *)thread_get(ins_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

static void drawCursor(Boolean on) {
  inspt_module_t *module = (inspt_module_t *)thread_get(ins_key);
  IndexedColorType oldf;

  if (module->height > 0) {
    oldf = WinSetForeColor(UIColorGetTableEntryIndex(on ? UIFieldCaret : UIFieldBackground));
    WinDrawLine(module->x, module->y, module->x, module->y + module->height - 1);
    WinSetForeColor(oldf);
  }
}

// system use only

void InsPtInitialize(void) {
  debug(DEBUG_ERROR, "Field", "InsPtInitialize not implemented");
}

// Set the screen-relative position of the insertion point.
// The position passed to this function is the location of the top-left corner of the insertion point.
// This function should be called only by the Field functions.

void InsPtSetLocation(const Int16 x, const Int16 y) {
  inspt_module_t *module = (inspt_module_t *)thread_get(ins_key);

  if (module->enabled && module->on) {
    debug(DEBUG_TRACE, "Field", "InsPtSetLocation (%d,%d) erase caret", x, y);
    drawCursor(false);
    module->on = false;
  }
  module->x = x;
  module->y = y;
}

// Return the screen-relative position of the insertion point.
// This function is called by the Field functions. An application would not normally call this function.

void InsPtGetLocation(Int16 *x, Int16 *y) {
  inspt_module_t *module = (inspt_module_t *)thread_get(ins_key);

  if (x) *x = module->x;
  if (y) *x = module->y;
}

// Enable or disable the insertion point. When the insertion point is disabled, it’s invisible; when it’s enabled, it blinks.
// This function is called by the Form functions when a text field loses or gains the focus, and by the Window functions when a region of
// the display is copied (WinCopyRectangle).
// The sysNotifyInsPtEnableEvent is broadcast at the start of InsPtEnable.

void InsPtEnable(Boolean enableIt) {
  inspt_module_t *module = (inspt_module_t *)thread_get(ins_key);

  module->enabled = enableIt;
//debug(1, "XXX", "InsPtEnable %d", module->enabled);
  if (!module->enabled && module->on) {
    debug(DEBUG_TRACE, "Field", "InsPtEnable erase caret");
    drawCursor(false);
    module->on = false;
  }
}

// Return true if the insertion point is enabled or false if the insertion point is disabled.

Boolean InsPtEnabled(void) {
  inspt_module_t *module = (inspt_module_t *)thread_get(ins_key);

  return module->enabled;
}

// Set the height of the insertion point to match the character height of the font used in the field that the insertion point is in.
// When the current font is changed, the insertion point height should be set to the line height of the new font.
// If the insertion point is visible when its height is changed, it’s erased and redrawn with its new height.

void InsPtSetHeight(const Int16 height) {
  inspt_module_t *module = (inspt_module_t *)thread_get(ins_key);

  if (module->enabled && module->on) {
    debug(DEBUG_TRACE, "Field", "InsSetheight %d erase caret", height);
    drawCursor(false);
    module->on = false;
  }
  module->height = height;
}

// Returns the height of the insertion point, in pixels.

Int16 InsPtGetHeight(void) {
  inspt_module_t *module = (inspt_module_t *)thread_get(ins_key);

  return module->height;
}

// system use only

void InsPtCheckBlink(void) {
  inspt_module_t *module = (inspt_module_t *)thread_get(ins_key);
  UInt32 t;

  if (module->enabled) {
    t = TimGetTicks();

    if ((t - module->lastBlink) >= module->tps / 2) {
      module->on = !module->on;
//debug(1, "XXX", "InsPtCheckBlink %d", module->on);
      drawCursor(module->on);
      module->lastBlink = t;
    }
  }
}
