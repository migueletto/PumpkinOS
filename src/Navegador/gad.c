#include <PalmOS.h>

#include "gui.h"
#include "gps.h"
#include "app.h"

Boolean ProgressGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  if (FrmGetActiveFormID() == ProgressForm)
    return GadgetCallback(progressCtl, cmd, param);
  return true;
}

Boolean LogGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  switch (FrmGetActiveFormID()) {
    case MainForm:
    case SatForm:
    case MapForm: 
    case CompassForm: 
    case WaypointsForm:
    case SeekPointForm:
    case RoutesForm:
    case FollowRouteForm:
    case TracksForm:
    case TripForm:
    case AstroForm:
      return GadgetCallback(logCtl, cmd, param);
  }
  return true;
}

Boolean StatusGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  switch (FrmGetActiveFormID()) {
    case MainForm:
    case SatForm:
    case MapForm:
    case CompassForm:
    case WaypointsForm:
    case SeekPointForm:
    case RoutesForm:
    case FollowRouteForm:
    case TracksForm:
    case TripForm:
    case AstroForm:
      return GadgetCallback(statusCtl, cmd, param);
  }
  return true;
}

Boolean CenterGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return GadgetCallback(centerCtl, cmd, param);
}

Boolean ZoominGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return GadgetCallback(zoominCtl, cmd, param);
}

Boolean ZoomoutGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return GadgetCallback(zoomoutCtl, cmd, param);
}

Boolean LockGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return GadgetCallback(lockCtl, cmd, param);
}

Boolean SymbolGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return GadgetCallback(symbolTbl, cmd, param);
}

Boolean NearestGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return GadgetCallback(nearestCtl, cmd, param);
}
