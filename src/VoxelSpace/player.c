#include "sys.h"
#include "player.h"
#include "input.h"

void player_move(player_t* player) {
  // Move front or back (accounting for acceleration & deacceleration)
  if (is_key_pressed('w') && player->forward_vel < player->forward_max) {
    player->forward_vel += (player->forward_vel < 0) ? player->forward_brk : player->forward_acc;
  } else if (is_key_pressed('s') && player->forward_vel > -player->forward_max) {
    player->forward_vel -= (player->forward_vel > 0) ? player->forward_brk : player->forward_acc; 
  } else {
    if (player->forward_vel - player->forward_brk > 0) {
      player->forward_vel -= player->forward_brk;
    } else if (player->forward_vel + player->forward_brk < 0) {
      player->forward_vel += player->forward_brk;
    } else {
      player->forward_vel = 0;
    }
  }

  // Pitch down and up as we move forward and backwards (accounting for acceleration & deacceleration)
  if (is_key_pressed('w') && player->pitch_vel > -player->pitch_max) {
    player->pitch_vel -= (player->pitch_vel > 0) ? player->pitch_brk : player->pitch_acc;
  } else if (is_key_pressed('s') && player->pitch_vel < player->pitch_max - player->pitch_acc) {
    player->pitch_vel += (player->pitch_vel < 0) ? player->pitch_brk : player->pitch_acc;
  } else if (!is_key_pressed('w') && !is_key_pressed('s')) {
    if (player->pitch_vel - player->pitch_brk > 0) {
      player->pitch_vel -= player->pitch_brk;
    } else if (player->pitch_vel + player->pitch_brk < 0) {
      player->pitch_vel += player->pitch_brk;
    } else {
      player->pitch_vel = 0;
    }
  }

  // Yaw left or right (accounting for acceleration & deacceleration)
  if (is_key_pressed('a') && player->yaw_vel > -player->yaw_max) {
    player->yaw_vel -= (player->yaw_vel > 0) ? player->yaw_brk : player->yaw_acc;
  } else if (is_key_pressed('d') && player->yaw_vel < player->yaw_max) {
    player->yaw_vel += (player->yaw_vel < 0) ? player->yaw_brk : player->yaw_acc;
  } else {
    if (player->yaw_vel - player->yaw_brk > 0) {
      player->yaw_vel -= player->yaw_brk;
    } else if (player->yaw_vel + player->yaw_brk < 0) {
      player->yaw_vel += player->yaw_brk;
    } else {
      player->yaw_vel = 0;
    }
  }

  // Roll left or right (accounting for acceleration & deacceleration)
  if (is_key_pressed('d') && player->roll_vel > -player->roll_max) {
    player->roll_vel -= (player->roll_vel > 0) ? player->roll_brk : player->roll_acc;
  } else if (is_key_pressed('a') && player->roll_vel < player->roll_max - player->roll_acc) {
    player->roll_vel += (player->roll_vel < 0) ? player->roll_brk : player->roll_acc;
  } else if (!is_key_pressed('a') && !is_key_pressed('d') && !is_key_pressed('m') && !is_key_pressed('n')) {
    if (player->roll_vel - player->roll_brk > 0) {
      player->roll_vel -= player->roll_brk;
    } else if (player->roll_vel + player->roll_brk < 0) {
      player->roll_vel += player->roll_brk;
    } else {
      player->roll_vel = 0;
    }
  }

  // Move up and down (accounting for acceleration & deacceleration)
  if (is_key_pressed('u') && player->lift_vel < player->lift_max) {
    player->lift_vel += (player->lift_vel < 0) ? player->lift_brk : player->lift_acc;
  } else if (is_key_pressed('j') && player->lift_vel > -player->lift_max) {
    player->lift_vel -= (player->lift_vel > 0) ? player->lift_brk : player->lift_acc;
  } else {
    if (player->lift_vel - player->lift_brk > 0) {
      player->lift_vel -= player->lift_brk;
    } else if (player->lift_vel + player->lift_brk < 0) {
      player->lift_vel += player->lift_brk;
    } else {
      player->lift_vel = 0;
    }
  }

  // Strafe left and right (accounting for acceleration & deacceleration)
  if (is_key_pressed('m') && player->strafe_vel < player->strafe_max) {
    player->strafe_vel += (player->strafe_vel < 0) ? player->strafe_brk : player->strafe_acc;
  } else if (is_key_pressed('n') && player->strafe_vel > -player->strafe_max) {
    player->strafe_vel -= (player->strafe_vel > 0) ? player->strafe_brk : player->strafe_acc;
  } else {
    if (player->strafe_vel - player->strafe_brk > 0) {
      player->strafe_vel -= player->strafe_brk;
    } else if (player->strafe_vel + player->strafe_brk < 0) {
      player->strafe_vel += player->strafe_brk;
    } else {
      player->strafe_vel = 0;
    }
  }

  // Roll left or right as we strafe (accounting for acceleration & deacceleration)
  if (is_key_pressed('m') && player->roll_vel > -player->roll_max) {
    player->roll_vel -= (player->roll_vel > 0) ? player->roll_brk : player->roll_acc;
  } else if (is_key_pressed('n') && player->roll_vel < player->roll_max - player->roll_acc) {
    player->roll_vel += (player->roll_vel < 0) ? player->roll_brk : player->roll_acc;
  } else if (!is_key_pressed('a') && !is_key_pressed('d') && !is_key_pressed('m') && !is_key_pressed('n')) {
    if (player->roll_vel - player->roll_brk > 0) {
      player->roll_vel -= player->roll_brk;
    } else if (player->roll_vel + player->roll_brk < 0) {
      player->roll_vel += player->roll_brk;
    } else {
      player->roll_vel = 0;
    }
  }
  
  if (player->forward_vel) {
    // Update player x and y position based on forward velocity and angle
    player->x += sys_cos(player->angle) * player->forward_vel * 0.9;
    player->y += sys_sin(player->angle) * player->forward_vel * 0.9;
  }

  if (player->strafe_vel) {
    // Strafe left and right perpendicular to the player angle
    player->x += sys_cos(1.57 + player->angle) * player->strafe_vel * 0.5;
    player->y += sys_sin(1.57 + player->angle) * player->strafe_vel * 0.5;
  }

  // Update player angle based on its current yaw velocity
  player->angle += player->yaw_vel * 0.02;

  // Update player height based on lift velocity
  player->height += player->lift_vel * 1.4;

  // Pitch up and down as we move front and back
  player->pitch = (player->pitch_vel * 20.0) + 80.0;
}
