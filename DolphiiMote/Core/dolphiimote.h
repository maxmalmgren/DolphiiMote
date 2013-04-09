// Copyright 2013 Max Malmgren

// This file is part of DolphiiMote.

// DolphiiMote is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// DolphiiMote is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with DolphiiMote.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _DOLPHIIMOTE_H
#define _DOLPHIIMOTE_H

#include <stdint.h>

typedef uint16_t dolphiimote_button_state;

/*
  'dolphiimote_capabilities' is a flag containing one or more of the following:

  MotionPlus = dolphiimote_CAPABILITIES_MOTION_PLUS
  Extension = dolphiimote_CAPABILITIES_EXTENSION
  IR = dolphiimote_CAPABILITIES_IR
*/
typedef uint16_t dolphiimote_capabilities;

/*
  Acceleration data format: Native Wiimote.

      (wiimote top) +Z  -Y (wiimote front)
                     |  /
                     | /
        +X  ---------|/-----------  -X
                     /
                    /|
                   / |
                  /  |
 (wiimote back) +Y  -Z (wiimote bottom)

*/
typedef struct dolphiimote_acceleration
{
  uint16_t x, y, z;
} dolphiimote_acceleration;

typedef struct dolphiimote_nunchuck
{
  uint8_t stick_x;
  uint8_t stick_y;

  uint16_t x, y, z;
  uint8_t buttons;

} dolphiimote_nunchuck;

typedef struct dolphiimote_motionplus
{
  uint16_t yaw_down_speed;
  uint16_t roll_left_speed;
  uint16_t pitch_left_speed;

  uint8_t slow_modes; //Yaw = 0x1, Roll = 0x2, Pitch = 0x4.
  uint8_t extension_connected;
} dolphiimote_motionplus;

typedef struct dolphiimote_wiimote_data
{  
  dolphiimote_button_state button_state;

  unsigned int valid_data_flags;

  dolphiimote_acceleration acceleration;
  dolphiimote_motionplus motionplus;
  dolphiimote_nunchuck nunchuck;
} dolphiimote_wiimote_data;

typedef struct dolphiimote_capability_status
{  
  uint64_t extension_id;
  uint32_t extension_type;

  dolphiimote_capabilities available_capabilities;
  dolphiimote_capabilities enabled_capabilities;
} dolphiimote_capability_status;

typedef void (*update_callback_t)(unsigned int wiimote_number, struct dolphiimote_wiimote_data *data_struct, void *userdata);
typedef void (*connection_callback_t)(unsigned int wiimote_number, int connected);
typedef void (*capabilities_callback_t)(unsigned int wiimote_number, struct dolphiimote_capability_status *capabilities, void *userdata);
typedef void (*log_callback_t)(const char* str, int size);

typedef struct dolphiimote_callbacks
{  
  update_callback_t data_received;
  connection_callback_t connection_changed;
  capabilities_callback_t capabilities_changed;
  log_callback_t log_received;

  void *userdata;
} dolphiimote_callbacks;


int dolphiimote_init(dolphiimote_callbacks on_update, void *callback_userdata);
void dolphiimote_update();

/*
  Valid modes:

  0x30 - 0x3D
  Check http://wiibrew.org/wiki/Wiimote#Data_Reporting for more information

*/
void dolphiimote_set_reporting_mode(unsigned int wiimote_number, uint8_t mode);

/*
  Rumble briefly - 200 ms
*/
void dolphiimote_brief_rumble(unsigned int wiimote_number);

/*
  Start a check for the current capabilities of the wiimote.
*/
void dolphiimote_determine_capabilities(unsigned int wiimote_number);

/*
  Enable different features of the wiimote.
*/
void dolphiimote_enable_capabilities(unsigned int wiimote_number, dolphiimote_capabilities capabilities);

/*
  Sets the level of the log calls that will be forwarded to the callback sent into dolphiimote_init.
  Log calls with level at or above the given level will be forwarded.
*/
void dolphiimote_log_level(uint8_t log_level);

/*
  These are the button states, as they are saved in dolphiimote_button_state
  and how they are sent by a Wiimote - but a Wiimote sends it in two
  consecutive bytes.
*/
#define dolphiimote_ACCELERATION_VALID 0x0002
#define dolphiimote_MOTIONPLUS_VALID 0x0004
#define dolphiimote_NUNCHUCK_VALID 0x0008

#define dolphiimote_BUTTON_DPAD_LEFT 0x0100
#define dolphiimote_BUTTON_DPAD_RIGHT 0x0200
#define dolphiimote_BUTTON_DPAD_DOWN 0x0400
#define dolphiimote_BUTTON_DPAD_UP 0x0800

#define dolphiimote_BUTTON_A 0x0008
#define dolphiimote_BUTTON_B 0x0004

#define dolphiimote_BUTTON_PLUS 0x1000
#define dolphiimote_BUTTON_MINUS 0x0010

#define dolphiimote_BUTTON_ONE 0x0002
#define dolphiimote_BUTTON_TWO 0x0001

#define dolphiimote_BUTTON_HOME 0x0080

#define dolphiimote_NUNCHUCK_BUTTON_Z 0x01
#define dolphiimote_NUNCHUCK_BUTTON_C 0x02

#define dolphiimote_EXTENSION_NONE 0x0000
#define dolphiimote_EXTENSION_NUNCHUCK 0x0001
#define dolphiimote_EXTENSION_CLASSIC_CONTROLLER 0x0002
#define dolphiimote_EXTENSION_CLASSIC_CONTROLLER_PRO 0x0003
#define dolphiimote_EXTENSION_GUITAR_HERO_GUITAR 0x0004
#define dolphiimote_EXTENSION_GUITAR_HERO_WORLD_TOUR_DRUMS 0x0005

#define dolphiimote_CAPABILITIES_MOTION_PLUS 0x0002
#define dolphiimote_CAPABILITIES_EXTENSION 0x0004
#define dolphiimote_CAPABILITIES_IR 0x0008

#define dolphiimote_LOG_LEVEL_DEBUG 0
#define dolphiimote_LOG_LEVEL_INFO 1
#define dolphiimote_LOG_LEVEL_WARNING 2
#define dolphiimote_LOG_LEVEL_ERROR 3

#define dolphiimote_MAX_WIIMOTES 4

#endif