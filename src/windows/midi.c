#include <windows.h>

#include "sys.h"
#include "midi.h"
#include "debug.h"

DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName) {
  UINT wDeviceID;
  DWORD dwReturn;
  MCI_OPEN_PARMS mciOpenParms;
  MCI_PLAY_PARMS mciPlayParms;
  MCI_STATUS_PARMS mciStatusParms;
  //MCI_SEQ_SET_PARMS mciSeqSetParms;

  // Open the device by specifying the device and filename.
  // MCI will attempt to choose the MIDI mapper as the output port.
  mciOpenParms.lpstrDeviceType = "sequencer";
  mciOpenParms.lpstrElementName = lpszMIDIFileName;
  if ((dwReturn = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)(LPVOID)&mciOpenParms)) != 0) {
    // Failed to open device. Don't close it; just return error.
    debug(DEBUG_ERROR, "MIDI", "failed to open device (%ld)", dwReturn);
    return dwReturn;
  }

  // The device opened successfully; get the device ID.
  wDeviceID = mciOpenParms.wDeviceID;

  // Check if the output port is the MIDI mapper.
  mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;
  if ((dwReturn = mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)(LPVOID)&mciStatusParms)) != 0) {
    debug(DEBUG_ERROR, "MIDI", "failed to get device status (%ld)", dwReturn);
    mciSendCommand(wDeviceID, MCI_CLOSE, 0, 0);
    return dwReturn;
  }

  // The output port is not the MIDI mapper. 
  if (LOWORD(mciStatusParms.dwReturn) != MIDI_MAPPER) {
    debug(DEBUG_ERROR, "MIDI", "output is not MIDI mapper (%llu)", mciStatusParms.dwReturn);
    mciSendCommand(wDeviceID, MCI_CLOSE, 0, 0);
    return mciStatusParms.dwReturn;
  }

  // Begin playback. The window procedure function for the parent 
  // window will be notified with an MM_MCINOTIFY message when 
  // playback is complete. At this time, the window procedure closes the device.
  mciPlayParms.dwCallback = (DWORD_PTR)hWndNotify;
  if ((dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)(LPVOID)&mciPlayParms)) != 0) {
    debug(DEBUG_ERROR, "MIDI", "failed to start MIDI play (%ld)", dwReturn);
    mciSendCommand(wDeviceID, MCI_CLOSE, 0, 0);
    return dwReturn;
  }

  return 0;
}
