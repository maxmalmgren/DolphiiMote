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

#include "../Dolphin/Wiimote.h"
#include "../Dolphin/WiimoteReal.h"

typedef void (*update_callback_t)(unsigned int wiimote_number, struct dolphiimote_wiimote_data *data_struct, void *userdata);
typedef void (*connection_callback_t)(unsigned int wiimote_number, int connected);
typedef void (*capabilities_callback_t)(unsigned int wiimote_number, struct dolphiimote_wiimote_data *data_struct, void *userdata);

struct dolphiimote_callbacks
{  
  update_callback_t data_received;
  connection_callback_t connection_changed;
  capabilities_callback_t capabilities_changed;

  void *userdata;
};

int dolphiimote_init(dolphiimote_callbacks on_update, void *callback_userdata);
void dolphiimote_update();

/*
  Valid modes:

  0x30 - 0x3D
  Check http://wiibrew.org/wiki/Wiimote#Data_Reporting for more information

*/
void dolphiimote_set_reporting_mode(int wiimote_number, uint8_t mode);

void dolphiimote_brief_rumble(int wiimote_number);

/*
  These are the button states, as they are saved in dolphiimote_button_state
  and how they are sent by a Wiimote - but a Wiimote sends it in two
  consecutive bytes.
*/

#define dolphiimote_ACCELERATION_VALID 0x0002
#define dolphiimote_MOTIONPLUS_VALID 0x0004

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

typedef u16 dolphiimote_button_state;

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
struct dolphiimote_acceleration
{
  unsigned char x, y, z;
};

struct dolphiimote_motionplus
{
  u16 yaw_down_speed;
  u16 roll_left_speed;
  u16 pitch_left_speed;

  u8 slow_modes; //Yaw = 0x1, Roll = 0x2, Pitch = 0x4.
  u8 extension_connected;
};

struct dolphiimote_wiimote_data
{  
  dolphiimote_button_state button_state;

  unsigned int valid_data_flags;
  struct dolphiimote_acceleration acceleration;
  struct dolphiimote_motionplus motionplus;
};

#endif