#ifndef player_H
#define player_H

#include <stdbool.h>
#include "input.h"

typedef struct {
  float x;            // current x coordinate on the map
  float y;            // current y coordinate on the map

  float height;       // current height (how much we offset the terrain up or down)
  float pitch;        // current pitch value (moves the horizon line up and down)
  float angle;        // current rotation angle where the player is pointing at (stored in radians)

  float forward_vel;  // forward velocity factor (value between -1 and 1)
  float forward_acc;  // forward acceleration (how fast the velocity increases)
  float forward_brk;  // forward brake (how fast the forward velocity decreases until it stops)
  float forward_max;  // maximum forward velocity (1.0)

  float pitch_vel;    // pitch velocity factor to move angle of attack up or down (value between -1 and 1)
  float pitch_acc;    // pitch acceleration (how fast the pitch increases)
  float pitch_brk;    // pitch brake (how fast the pitch velocity decreases until it stops)
  float pitch_max;    // maximum pitch velocity (1.0)

  float yaw_vel;      // yaw velocity factor used to smooth the movement of the player rotation angle (value between -1 and 1)
  float yaw_acc;      // yaw acceleration (how fast the rotation angle increases)
  float yaw_brk;      // yaw brake (how fast the angle of rotation speed decreases until it stops)
  float yaw_max;      // maximum yaw velocity (1.0)

  float lift_vel;     // lift velocity factor used to move the player height up and down (value between -1 and 1)
  float lift_acc;     // lift acceleration (how fast the player height moves up and down)
  float lift_brk;     // lift brake (how fast the lift velocity decreases until it stops)
  float lift_max;     // maximum lift velocity (1.0)

  float strafe_vel;   // strafe velocity factor used to move left and right perpendicular to the rotation angle (value between -1 and 1)
  float strafe_acc;   // strafe acceleration (how fast the player strafes left and right)
  float strafe_brk;   // strafe brake (how fast the strafe velocity decreases until it stops)
  float strafe_max;   // maximum strafe velocity (1.0)

  float roll_vel;     // roll velocity factor used to tilt the camera distorting the terrain left and right (value between -1 and 1)
  float roll_acc;     // roll acceleration (how fast the roll velocity increases)
  float roll_brk;     // roll brake (how fast the roll velocity decreases until it stops)
  float roll_max;     // maximum roll velocity (1.0)
} player_t;

void player_move(player_t *player);

#endif
