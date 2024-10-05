#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <PalmOS.h>

#include "main.h"
#include "drop.h"
#include "midi.h"
#include "thread.h"
#include "script.h"
#include "sys.h"
#include "pwindow.h"
#include "audio.h"
#include "media.h"
#include "ptr.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

#define TAG_AUDIO  "audio"

#define MAX_ARGS   16
#define MAX_EVENTS 16

typedef struct {
  int ev, arg1, arg2;
} win_event_t;

typedef struct {
  BITMAPV5HEADER bmiHeader;
  DWORD bmiColors[3];
} V5BMPINFO;

typedef struct {
  HWND hwnd;
  IDropTarget *dt;
  V5BMPINFO bmi;
  HBITMAP bitmap;
  HDC hdcBitmap;
  uint16_t *bitmapBuffer;
  int width, height, size;
  win_event_t events[MAX_EVENTS];
  int num_ev, ie, oe, down;
  int x, y, buttons;
} win_window_t;

struct texture_t {
  int width, height, size;
  uint16_t *buffer;
};

typedef struct {
  char *tag;
  HWAVEOUT hwo;
  WAVEHDR hdr[2];
  int (*getaudio)(void *buffer, int len, void *data);
  void *data;
  uint8_t *buffer;
  uint32_t bsize;
} win_audio_t;

static const wchar_t CLASS_NAME[]  = L"PumpkinOS";
static const wchar_t WINDOW_PROP[] = L"window";

static window_provider_t wp;
static audio_provider_t ap;

static void putEvent(win_window_t *window, win_event_t *ev) {
  if (window->num_ev < MAX_EVENTS) {
    xmemcpy(&window->events[window->ie], ev, sizeof(win_event_t));
    window->ie++;
    if (window->ie == MAX_EVENTS) window->ie = 0;
    window->num_ev++;
  }
}

static int getEvent(win_window_t *window, win_event_t *ev) {
  if (window->num_ev > 0) {
    xmemcpy(ev, &window->events[window->oe], sizeof(win_event_t));
    window->oe++;
    if (window->oe == MAX_EVENTS) window->oe = 0;
    window->num_ev--;
    return 1;
  }

  return 0;
}

static int mapKey(int code) {
  switch (code) {
    case VK_BACK:   code = 8;  break;
    case VK_TAB:    code = 9;  break;
    case VK_RETURN: code = 13;  break;
    case VK_HOME:   code = WINDOW_KEY_HOME;  break;
    case VK_LEFT:   code = WINDOW_KEY_LEFT;  break;
    case VK_UP:     code = WINDOW_KEY_UP;    break;
    case VK_RIGHT:  code = WINDOW_KEY_RIGHT; break;
    case VK_DOWN:   code = WINDOW_KEY_DOWN;  break;
    case VK_F1:     code = WINDOW_KEY_F1;    break;
    case VK_F2:     code = WINDOW_KEY_F2;    break;
    case VK_F3:     code = WINDOW_KEY_F3;    break;
    case VK_F4:     code = WINDOW_KEY_F4;    break;
    case VK_F5:     code = WINDOW_KEY_F5;    break;
    case VK_F6:     code = WINDOW_KEY_F6;    break;
    case VK_F7:     code = WINDOW_KEY_F7;    break;
    case VK_F8:     code = WINDOW_KEY_F8;    break;
    case VK_F9:     code = WINDOW_KEY_F9;    break;
    case VK_F10:    code = WINDOW_KEY_F10;   break;
    case VK_F11:    code = WINDOW_KEY_F11;   break;
    case VK_F12:    code = WINDOW_KEY_F12;   break;
    default: code = 0;
  }

  return code;
}

static int mapChar(WPARAM wParam, LPARAM lParam) {
  UINT scandCode;
  WCHAR lBuffer[16]; 
  BYTE State[256];    
  int r, code = 0; 

  scandCode = (lParam >> 8) & 0xFFFFFF00; 
  GetKeyboardState(State);    
  if ((r = ToUnicode(wParam, scandCode, State, lBuffer, 16, 0)) == 1) {
    code = lBuffer[0];
  } else if (r < 0) {
    // dead key, "maybe" there is something in the buffer
    code = lBuffer[0];
    debug(DEBUG_TRACE, "Windows", "dead key %d", code);
  }

  return code;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  win_window_t *window;
  win_event_t ev;
  PAINTSTRUCT ps;
  HDC hdc;
  int code;

  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      pumpkin_set_finish(1);
      return 0;

    // XXX some dead keys are not mapped by this

    case WM_KEYDOWN:
      debug(DEBUG_TRACE, "Windows", "keyDown %d 0x%08X", wParam, lParam);
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);
      code = mapKey(wParam);
      debug(DEBUG_TRACE, "Windows", "keyDown mapKey %d", code);
      if (code == 0) {
        code = mapChar(wParam, lParam);
        debug(DEBUG_TRACE, "Windows", "keyDown mapChar %d", code);
      }
      if (code) {
        debug(DEBUG_TRACE, "Windows", "keyDown event");
        ev.ev = WINDOW_KEYDOWN;
        ev.arg1 = code;
        ev.arg2 = 0;
        putEvent(window, &ev);
      }
      return 0;

    case WM_KEYUP:
      debug(DEBUG_TRACE, "Windows", "keyUp %d 0x%08X", wParam, lParam);
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);
      code = mapKey(wParam);
      debug(DEBUG_TRACE, "Windows", "keyUp mapKey %d", code);
      if (code == 0) {
        code = mapChar(wParam, lParam);
        debug(DEBUG_TRACE, "Windows", "keyUp mapChar %d", code);
      }
      if (code) {
        debug(DEBUG_TRACE, "Windows", "keyUp event");
        ev.ev = WINDOW_KEYUP;
        ev.arg1 = code;
        ev.arg2 = 0;
        putEvent(window, &ev);
      }
      return 0;

    case WM_MOUSEMOVE:
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);
      if (window->down) {
        debug(DEBUG_TRACE, "Windows", "drag %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        ev.ev = WINDOW_MOTION;
        ev.arg1 = GET_X_LPARAM(lParam);
        ev.arg2 = GET_Y_LPARAM(lParam);
        window->x = ev.arg1;
        window->y = ev.arg2;
        putEvent(window, &ev);
      }
      return 0;

    case WM_LBUTTONDOWN:
      debug(DEBUG_TRACE, "Windows", "button 1 down %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);
      window->down = 1;
      ev.ev = WINDOW_MOTION;
      ev.arg1 = GET_X_LPARAM(lParam);
      ev.arg2 = GET_Y_LPARAM(lParam);
      window->x = ev.arg1;
      window->y = ev.arg2;
      window->buttons = 1;
      putEvent(window, &ev);
      ev.ev = WINDOW_BUTTONDOWN;
      ev.arg1 = 1;
      ev.arg2 = 0;
      putEvent(window, &ev);
      return 0;

    case WM_LBUTTONUP:
      debug(DEBUG_TRACE, "Windows", "button up %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);
      window->down = 0;
      ev.ev = WINDOW_BUTTONUP;
      ev.arg1 = 1;
      ev.arg2 = 0;
      window->buttons = 0;
      putEvent(window, &ev);
      return 0;

    case WM_RBUTTONDOWN:
      debug(DEBUG_TRACE, "Windows", "button 2 down %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);
      window->down = 1;
      ev.ev = WINDOW_MOTION;
      ev.arg1 = GET_X_LPARAM(lParam);
      ev.arg2 = GET_Y_LPARAM(lParam);
      window->x = ev.arg1;
      window->y = ev.arg2;
      putEvent(window, &ev);
      ev.ev = WINDOW_BUTTONDOWN;
      ev.arg1 = 2;
      ev.arg2 = 0;
      putEvent(window, &ev);
      return 0;

    case WM_RBUTTONUP:
      debug(DEBUG_TRACE, "Windows", "button 2 up %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);
      window->down = 0;
      ev.ev = WINDOW_BUTTONUP;
      ev.arg1 = 2;
      ev.arg2 = 0;
      putEvent(window, &ev);
      return 0;

    case WM_PAINT:
      window = (win_window_t *)GetProp(hwnd, WINDOW_PROP);
      hdc = BeginPaint(hwnd, &ps);
      debug(DEBUG_TRACE, "Windows", "paint %d %d %d %d", ps.rcPaint.left, ps.rcPaint.right, ps.rcPaint.top, ps.rcPaint.bottom);
      SetDIBits(hdc, window->bitmap, 0, window->height, window->bitmapBuffer, (BITMAPINFO *)&window->bmi, DIB_RGB_COLORS);
      BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right-ps.rcPaint.left+1, ps.rcPaint.bottom-ps.rcPaint.top+1, window->hdcBitmap, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
      EndPaint(hwnd, &ps);
      return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static window_t *window_create(int encoding, int *width, int *height, int xfactor, int yfactor, int rotate, int fullscreen, int software, char *driver, void *data) {
  win_window_t *window;
  HINSTANCE hInstance;
  BITMAPV5HEADER bmh;
  RECT rect;
  HDC hdc;

  debug(DEBUG_TRACE, "Windows", "window_create(%d,%d,%d,%d,%d,%d,%d,%d)", encoding, *width, *height, xfactor, yfactor, rotate, fullscreen, software);

  if ((window = xcalloc(1, sizeof(win_window_t))) != NULL) {
    hInstance = (HINSTANCE)data;

    rect.left = rect.top = 0;
    rect.right = *width;
    rect.bottom = *height;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    window->hwnd = CreateWindowEx(
      0,                              // Optional window styles.
      CLASS_NAME,                     // Window class
      L"PumpkinOS",                   // Window text
      WS_OVERLAPPEDWINDOW,            // Window style
      CW_USEDEFAULT, CW_USEDEFAULT,
      rect.right - rect.left, rect.bottom - rect.top,
      NULL,       // Parent window
      NULL,       // Menu
      hInstance,  // Instance handle
      NULL        // Additional application data
    );
    SetProp(window->hwnd, WINDOW_PROP, window);

    window->width = *width;
    window->height = *height;
    window->size = window->width * window->height * sizeof(uint16_t);

    hdc = GetDC(window->hwnd);
    window->hdcBitmap = CreateCompatibleDC(hdc);

    xmemset(&window->bmi, 0, sizeof(V5BMPINFO));
    bmh.bV5Size = sizeof(BITMAPV5HEADER);
    bmh.bV5Width = window->width;
    bmh.bV5Height = window->height;
    bmh.bV5Planes = 1;
    bmh.bV5BitCount = 16;
    bmh.bV5Compression = BI_BITFIELDS;
    bmh.bV5AlphaMask = 0x0000;
    bmh.bV5RedMask   = 0xF800;
    bmh.bV5GreenMask = 0x07E0;
    bmh.bV5BlueMask  = 0x001F;
    bmh.bV5CSType = LCS_WINDOWS_COLOR_SPACE;
    bmh.bV5Intent = LCS_GM_BUSINESS;
    window->bmi.bmiHeader = bmh;
    window->bmi.bmiColors[0] = 0x001F;
    window->bmi.bmiColors[1] = 0x07E0;
    window->bmi.bmiColors[2] = 0xF800;
    window->bitmap = CreateDIBSection(hdc, (BITMAPINFO *)&window->bmi, DIB_RGB_COLORS, (void **)&window->bitmapBuffer, 0, 0);

    SelectObject(window->hdcBitmap, window->bitmap);
    ReleaseDC(window->hwnd, hdc);

    RegisterDropWindow(window->hwnd, &window->dt);

    ShowWindow(window->hwnd, SW_NORMAL);
  }

  return (window_t *)window;
}

static int window_destroy(window_t *_window) {
  win_window_t *window = (win_window_t *)_window;

  debug(DEBUG_TRACE, "Windows", "window_destroy()");

  if (window) {
    if (window->hwnd) DestroyWindow(window->hwnd);
    xfree(window);
  }

  return 0;
}

static int window_render(window_t *_window) {
  win_window_t *window = (win_window_t *)_window;

  debug(DEBUG_TRACE, "Windows", "window_render()");

  if (window) {
    //debug(DEBUG_TRACE, "Windows", "render");
    //InvalidateRect(window->hwnd, NULL, false);
  }

  return 0;
}

static texture_t *window_create_texture(window_t *_window, int width, int height) {
  texture_t *texture;

  debug(DEBUG_TRACE, "Windows", "window_create_texture(%d,%d)", width, height);

  if ((texture = xcalloc(1, sizeof(texture_t))) != NULL) {
    texture->width = width;
    texture->height = height;
    texture->size = width * height * sizeof(uint16_t);
    texture->buffer = xmalloc(texture->size);
  }

  debug(DEBUG_TRACE, "Windows", "window_create_texture(%d,%d): %p", width, height, texture);
  return texture;
}

static int window_destroy_texture(window_t *window, texture_t *texture) {
  debug(DEBUG_TRACE, "Windows", "window_destroy_texture");

  if (texture) {
    if (texture->buffer) {
      xfree(texture->buffer);
    }
    xfree(texture);
  }

  return 0;
}

static int window_update_texture_rect(window_t *_window, texture_t *texture, uint8_t *src, int tx, int ty, int w, int h) {
  int i, j, tpitch, tindex;
  uint16_t *buffer;

  debug(DEBUG_TRACE, "Windows", "window_update_texture_rect(%d,%d,%d,%d)", tx, ty, w, h);

  if (texture && src) {
    buffer = (uint16_t *)src;
    tpitch = texture->width;
    tindex = ty * tpitch + tx;
    for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
        texture->buffer[tindex + j] = buffer[tindex + j];
      }
      tindex += tpitch;
    }
  }

  return 0;
}

static int window_update_texture(window_t *window, texture_t *texture, uint8_t *raw) {
  debug(DEBUG_TRACE, "Windows", "window_update_texture");

  if (texture && texture->buffer && raw) {
    xmemcpy(texture->buffer, raw, texture->size);
  }

  return 0;
}

static int window_draw_texture_rect(window_t *_window, texture_t *texture, int tx, int ty, int w, int h, int x, int y) {
  win_window_t *window = (win_window_t *)_window;
  int i, j, d, tpitch, wpitch, tindex, windex;
  RECT rect;

  debug(DEBUG_TRACE, "Windows", "window_draw_texture_rect(%d,%d,%d,%d,%d,%d)", tx, ty, w, h, x, y);

  if (texture && window->bitmapBuffer) {
    if (x < 0) {
      tx -= x;
      w += x;
      x = 0;
    }
    if (x+w >= window->width) {
      d = x+w - window->width;
      w -= d;
    }
    if (y < 0) {
      ty -= y;
      h += y;
      y = 0;
    }
    if (y+h >= window->height) {
      d = y+h - window->height;
      h -= d;
    }
    if (w > 0 && h > 0) {
      tpitch = texture->width;
      wpitch = window->width;
      tindex = ty * tpitch + tx;
      windex = (window->height - y - 1) * wpitch + x;
      GdiFlush();
      for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
          window->bitmapBuffer[windex + j] = texture->buffer[tindex + j];
        }
        tindex += tpitch;
        windex -= wpitch;
      }
      rect.left = x;
      rect.right = x + w;
      rect.top = y;
      rect.bottom = y + h;
      debug(DEBUG_TRACE, "Windows", "invalidate %d %d %d %d", rect.left, rect.right, rect.top, rect.bottom);
      InvalidateRect(window->hwnd, &rect, false);
    }
  }

  debug(DEBUG_TRACE, "Windows", "window_draw_texture_rect(%d,%d,%d,%d,%d,%d) end", tx, ty, w, h, x, y);

  return 0;
}

static int window_draw_texture(window_t *window, texture_t *texture, int x, int y) {
  debug(DEBUG_TRACE, "Windows", "window_draw_texture(%d,%d)", x, y);

  if (texture) {
    window_draw_texture_rect(window, texture, 0, 0, texture->width, texture->height, x, y);
  }

  return 0;
}

static int window_event2(window_t *_window, int wait, int *arg1, int *arg2) {
  win_window_t *window = (win_window_t *)_window;
  win_event_t event;
  MSG msg;
  int64_t t, dt, endt;
  uint32_t waitUs, us;
  int ev = 0;

  if (wait == 0) {
    while (PeekMessage(&msg, NULL, 0, 0, 0)) {
      if (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  } else if (wait < 0) {
    us = 100000;
    for (; !thread_must_end();) {
      while (PeekMessage(&msg, NULL, 0, 0, 0)) {
        if (GetMessage(&msg, NULL, 0, 0)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
      if (window->num_ev) break;
      sys_usleep(us);
    }
  } else {
    waitUs = wait * 1000;
    us = waitUs;
    if (us > 1000) us = 1000;
    endt = sys_get_clock() + waitUs;
    for (;;) {
      while (PeekMessage(&msg, NULL, 0, 0, 0)) {
        if (GetMessage(&msg, NULL, 0, 0)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
      if (window->num_ev) break;
      t = sys_get_clock();
      if (t >= endt) break;
      dt = endt - t;
      sys_usleep(dt < us ? dt : us);
    }
  }

  if (getEvent(window, &event)) {
    ev = event.ev;
    *arg1 = event.arg1;
    *arg2 = event.arg2;
  }

  return ev;
}

static void window_status(window_t *_window, int *x, int *y, int *buttons) {
  win_window_t *window = (win_window_t *)_window;
  *x = window->x;
  *y = window->y;
  *buttons = window->buttons;
}

static void window_show_cursor(window_t *window, int show) {
  ShowCursor(show ? TRUE : FALSE);
}

static int audio_mixer_init(void) {
  return 0;
}

static int audio_mixer_play(uint8_t *buf, uint32_t len, int volume) {
  char name[256 + L_tmpnam + 1];
  int fd, r = -1;

  if (buf) {
    if (sys_tmpname(name, sizeof(name)) == 0) {
      if ((fd = sys_create(name, SYS_WRITE, 0644)) != -1) {
        sys_write(fd, buf, len);
        sys_close(fd);
        r = playMIDIFile(NULL, name) == 0;
        sys_unlink(name);
      }
    }
  }

  return r;
}

static int audio_mixer_stop(void) {
  return 0;
}

static int sendBuffer(HWAVEOUT hwo, WAVEHDR *hdr, void *buffer, uint32_t len, void *user) {
  MMRESULT res;
  int r = -1;

  hdr->lpData = (LPSTR)buffer;
  hdr->dwBufferLength = len;
  hdr->dwUser = (DWORD_PTR)user;
  hdr->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
  hdr->dwLoops = 0;

  if ((res = waveOutPrepareHeader(hwo, hdr, sizeof(WAVEHDR))) == MMSYSERR_NOERROR) {
    debug(DEBUG_TRACE, "Windows", "sending %d bytes", len);
    if ((res = waveOutWrite(hwo, hdr, sizeof(WAVEHDR))) == MMSYSERR_NOERROR) {
      r = 0;
    } else {
      debug(DEBUG_ERROR, "Windows", "waveOutWrite failed (%d)", res);
      waveOutUnprepareHeader(hwo, hdr, sizeof(WAVEHDR));
    }
  } else {
    debug(DEBUG_ERROR, "Windows", "waveOutPrepareHeader failed (%d)", res);
  }

  return r;
}

static int sendAudio(win_audio_t *audio, int idx) {
  int len, r = -1;

  if ((len = audio->getaudio(audio->buffer, audio->bsize, audio->data)) > 0) {
    r = sendBuffer(audio->hwo, &audio->hdr[idx], audio->buffer, len, audio);
    if (r == 0) r = len;
  } else if (len == 0) {
    r = 0;
  } else {
    debug(DEBUG_ERROR, "Windows", "getaudio returned %d", len);
    audio->data = NULL;
  }

  return r;
}

static void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
  win_audio_t *audio;
  WAVEHDR *hdr;

  if (uMsg == WOM_DONE) {
    hdr = (WAVEHDR *)dwParam1;
    if (hdr) {
      audio = (win_audio_t *)hdr->dwUser;
      if (audio->data) {
        sendAudio(audio, hdr == &audio->hdr[0] ? 0 : 1);
      }
    } else {
      debug(DEBUG_INFO, "Windows", "waveOutCallback hdr NULL");
    }
  }
}

static void audio_destructor(void *p) {
  win_audio_t *audio = (win_audio_t *)p;

  if (audio) {
    if (audio->hwo) {
      if (audio->buffer) xfree(audio->buffer);
      waveOutClose(audio->hwo);
    }
    xfree(audio);
  }
}

static audio_t audio_create(int pcm, int channels, int rate, void *data) {
  win_audio_t *audio;
  WAVEFORMATEX fmt;
  MMRESULT res;
  int ptr = -1;

  debug(DEBUG_INFO, "Windows", "audio_create(%d,%d,%d)", pcm, channels, rate);

  if ((pcm >= PCM_S8 && pcm <= PCM_FLT) && (channels == 1 || channels == 2) && rate > 0) {
    if ((audio = xcalloc(1, sizeof(win_audio_t))) != NULL) {
      sys_memset(&fmt, 0, sizeof(WAVEFORMATEX));
      fmt.wFormatTag = WAVE_FORMAT_PCM;
      fmt.nChannels = channels;
      fmt.nSamplesPerSec = rate;
      switch (pcm) {
        case PCM_S8:
        case PCM_U8:
          fmt.wBitsPerSample = 8;
          audio->bsize = 1;
          break;
        case PCM_S16:
        case PCM_U16:
          fmt.wBitsPerSample = 16;
          audio->bsize = 2;
          break;
        case PCM_S32:
        case PCM_U32:
        case PCM_FLT:
          fmt.wBitsPerSample = 32;
          audio->bsize = 4;
          break;
      }
      audio->bsize *= channels;
      debug(DEBUG_INFO, "Windows", "sample size %d", audio->bsize);
      audio->bsize *= rate;
      audio->bsize /= 20;
      audio->buffer = xcalloc(1, audio->bsize);
      debug(DEBUG_INFO, "Windows", "buffer size %d", audio->bsize);

      fmt.nBlockAlign = (fmt.nChannels * fmt.wBitsPerSample) / 8;
      fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
      fmt.cbSize = 0;

      if ((res = waveOutOpen(&audio->hwo, WAVE_MAPPER, &fmt, (DWORD_PTR)waveOutCallback, 0, CALLBACK_FUNCTION)) == MMSYSERR_NOERROR) {
        audio->tag = TAG_AUDIO;
        if ((ptr = ptr_new(audio, audio_destructor)) == -1) {
          xfree(audio->buffer);
          waveOutClose(audio->hwo);
          xfree(audio);
        }
      } else {
        debug(DEBUG_ERROR, "Windows", "waveOutOpen failed (%d)", res);
        xfree(audio);
      }
    }
  }

  return ptr;
}

static int audio_start(audio_t _audio, int (*getaudio)(void *buffer, int len, void *data), void *data) {
  win_audio_t *audio;
  int r = -1;

  if ((audio = ptr_lock(_audio, TAG_AUDIO)) != NULL) {
    if (audio && audio->hwo) {
      audio->getaudio = getaudio;
      audio->data = data;

      r = sendAudio(audio, 0);
      if (r > 0) {
        r = sendAudio(audio, 1);
        if (r > 0) r = 0;
      }
    } else {
      debug(DEBUG_ERROR, "Windows", "audio_start invalid params");
    }
    ptr_unlock(_audio, TAG_AUDIO);
  }

  return r;
}

static int audio_destroy(audio_t audio) {
  return ptr_free(audio, TAG_AUDIO);
}

static void pit_callback(int pe, void *data) {
  script_set_pointer(pe, WINDOW_PROVIDER, &wp);
  script_set_pointer(pe, AUDIO_PROVIDER, &ap);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  WNDCLASS wc;
  char *argv[MAX_ARGS];
  char *cmdline;
  int i, s, argc;

  OleInitialize(0);

  xmemset(&wc, 0, sizeof(WNDCLASS));
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = CLASS_NAME;
  RegisterClass(&wc);

  xmemset(&wp, 0, sizeof(window_provider_t));
  wp.create = window_create;
  wp.destroy = window_destroy;
  wp.render = window_render;
  wp.create_texture = window_create_texture;
  wp.destroy_texture = window_destroy_texture;
  wp.update_texture = window_update_texture;
  wp.draw_texture = window_draw_texture;
  wp.update_texture_rect = window_update_texture_rect;
  wp.draw_texture_rect = window_draw_texture_rect;
  wp.event2 = window_event2;
  wp.status = window_status;
  wp.show_cursor = window_show_cursor;
  wp.data = hInstance;

  ap.create = audio_create;
  ap.start = audio_start;
  ap.destroy = audio_destroy;
  ap.mixer_init = audio_mixer_init;
  ap.mixer_play = audio_mixer_play;
  ap.mixer_stop = audio_mixer_stop;
  ap.data = hInstance;

  cmdline = xstrdup(lpCmdLine);
  argc = 0;
  argv[argc++] = "pit";
  s = 0;
  for (i = 0; cmdline[i] && argc < MAX_ARGS; i++) {
    switch (s) {
      case 0:
        if (cmdline[i] != ' ') {
          argv[argc++] = &cmdline[i];
          s = 1;
        }
        break;
      case 1:
        if (cmdline[i] == ' ') {
          cmdline[i] = 0;
          s = 0;
        }
        break;
    }
  }

  pit_main(argc, argv, pit_callback, NULL);
  xfree(cmdline);

  OleUninitialize();

  return 0;
}
