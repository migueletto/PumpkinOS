//
// Copyright(C) 2022 by Ryan Krafnick
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DSDA Game Controller
//

#ifndef __DSDA_GAME_CONTROLLER__
#define __DSDA_GAME_CONTROLLER__

// Must match SDL
typedef enum {
  DSDA_CONTROLLER_BUTTON_A,
  DSDA_CONTROLLER_BUTTON_B,
  DSDA_CONTROLLER_BUTTON_X,
  DSDA_CONTROLLER_BUTTON_Y,
  DSDA_CONTROLLER_BUTTON_BACK,
  DSDA_CONTROLLER_BUTTON_GUIDE,
  DSDA_CONTROLLER_BUTTON_START,
  DSDA_CONTROLLER_BUTTON_LEFTSTICK,
  DSDA_CONTROLLER_BUTTON_RIGHTSTICK,
  DSDA_CONTROLLER_BUTTON_LEFTSHOULDER,
  DSDA_CONTROLLER_BUTTON_RIGHTSHOULDER,
  DSDA_CONTROLLER_BUTTON_DPAD_UP,
  DSDA_CONTROLLER_BUTTON_DPAD_DOWN,
  DSDA_CONTROLLER_BUTTON_DPAD_LEFT,
  DSDA_CONTROLLER_BUTTON_DPAD_RIGHT,
  DSDA_CONTROLLER_BUTTON_MISC1,
  DSDA_CONTROLLER_BUTTON_PADDLE1,
  DSDA_CONTROLLER_BUTTON_PADDLE2,
  DSDA_CONTROLLER_BUTTON_PADDLE3,
  DSDA_CONTROLLER_BUTTON_PADDLE4,
  DSDA_CONTROLLER_BUTTON_TOUCHPAD,
  DSDA_CONTROLLER_BUTTON_TRIGGERLEFT,
  DSDA_CONTROLLER_BUTTON_TRIGGERRIGHT,
  DSDA_CONTROLLER_BUTTON_MAX,
} dsda_game_controller_button_t;

const char* dsda_GameControllerButtonName(int button);
void dsda_PollGameController(void);
void dsda_InitGameController(void);

#endif
