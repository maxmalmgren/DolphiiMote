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

#include "../../Dolphiimote/Core/dolphiimote.h"
#include <stdio.h>
#include <Windows.h>

int state = 0;

void on_data_received(unsigned int wiimote_number, struct dolphiimote_wiimote_data *data, void *userdata)
{
  printf("wiimote %i:\t", wiimote_number);

  if(data->button_state & dolphiimote_BUTTON_A)
  {
    printf("A ");

    if(state == 0)
    {
      state = 1;
      dolphiimote_enable_capabilities(wiimote_number, dolphiimote_CAPABILITIES_MOTION_PLUS);
    }

    dolphiimote_brief_rumble(wiimote_number);
  }

  if(data->button_state & dolphiimote_BUTTON_B)
  {
    printf("B ");
    if(state == 1)
    {
      state = 0;
      dolphiimote_enable_capabilities(wiimote_number, dolphiimote_CAPABILITIES_EXTENSION);
    }
  }

  if(data -> button_state & dolphiimote_BUTTON_DPAD_DOWN)
    printf("Down ");
  if(data -> button_state & dolphiimote_BUTTON_DPAD_RIGHT)
    printf("Right ");
  if(data -> button_state & dolphiimote_BUTTON_DPAD_LEFT)
    printf("Left ");
  if(data -> button_state & dolphiimote_BUTTON_DPAD_UP)
    printf("Up ");
  if(data -> button_state & dolphiimote_BUTTON_MINUS)
    printf("Minus ");
  if(data -> button_state & dolphiimote_BUTTON_PLUS)
    printf("Plus ");
  if(data -> button_state & dolphiimote_BUTTON_HOME)
    printf("Home ");
  if(data -> button_state & dolphiimote_BUTTON_ONE)
    printf("One ");
  if(data -> button_state & dolphiimote_BUTTON_TWO)
    printf("Two ");

  if(data->valid_data_flags & dolphiimote_ACCELERATION_VALID)
    printf("Acc: %02X%02X%02X\t", data->acceleration.x, data->acceleration.y, data->acceleration.z);

  if(data->valid_data_flags & dolphiimote_MOTIONPLUS_VALID)
    printf("Motion Plus: %04X%04X%04X\t", data->motionplus.yaw_down_speed, data->motionplus.pitch_left_speed, data->motionplus.roll_left_speed);

  if(data->valid_data_flags & dolphiimote_NUNCHUCK_VALID)
  {
    int c = data->nunchuck.buttons & dolphiimote_NUNCHUCK_BUTTON_C;
    int z = data->nunchuck.buttons & dolphiimote_NUNCHUCK_BUTTON_Z;
    printf("Nunchuck: C: %i, Z: %i, Acc: %02X%02X%02X\n Stick X: %i, Y: %i\t", c, z, data->nunchuck.x, data->nunchuck.y, data->nunchuck.z, data->nunchuck.stick_x, data->nunchuck.stick_y);
  }

  printf("\n");
}

void on_capabilities_changed(unsigned int wiimote, dolphiimote_capability_status *status, void *userdata)
{
  printf("wiimote %i capabilities:\n", wiimote);
  printf("Extension: ");

  switch(status->extension_type)
  {
    case dolphiimote_EXTENSION_NONE:
      printf("None");
      break;
    case dolphiimote_EXTENSION_NUNCHUCK:
      printf("Nunchuck");
      break;
    case dolphiimote_EXTENSION_CLASSIC_CONTROLLER:
      printf("Classic Controller");
      break;
    case dolphiimote_EXTENSION_CLASSIC_CONTROLLER_PRO:
      printf("Classic Controller Pro");
      break;
    case dolphiimote_EXTENSION_GUITAR_HERO_GUITAR:
      printf("Guitar");
      break;
    case dolphiimote_EXTENSION_GUITAR_HERO_WORLD_TOUR_DRUMS:
      printf("Drums");
      break;
  }

  printf("\n");

  printf("Available:");

  if(status->available_capabilities & dolphiimote_CAPABILITIES_MOTION_PLUS)
    printf(" MotionPlus");

  if(status->available_capabilities & dolphiimote_CAPABILITIES_EXTENSION)
    printf(" Extension");

  if(status->available_capabilities & dolphiimote_CAPABILITIES_IR)
    printf(" IR");

  printf("\n");

  printf("Enabled:");

  if(status->enabled_capabilities & dolphiimote_CAPABILITIES_MOTION_PLUS)
    printf(" MotionPlus");

  if(status->enabled_capabilities & dolphiimote_CAPABILITIES_EXTENSION)
    printf(" Extension");

  if(status->enabled_capabilities & dolphiimote_CAPABILITIES_IR)
    printf(" IR");
  
  printf("\n");

  Sleep(500);
  dolphiimote_set_reporting_mode(wiimote, 0x35);  
}

void on_log_received(const char* str, int size)
{
  printf_s(str);
}

int main()
{
  dolphiimote_callbacks callbacks = { 0 };
  int wiimote_flags;
  int i;

  callbacks.data_received = on_data_received;
  callbacks.capabilities_changed = on_capabilities_changed;
  callbacks.log_received = on_log_received;

  wiimote_flags = dolphiimote_init(callbacks, NULL);

  for(i = 0; i < dolphiimote_MAX_WIIMOTES; i++, wiimote_flags >>= 1)
  {
    if(wiimote_flags & 0x01)
      dolphiimote_determine_capabilities(i);
  }

  while(1)
  {
    Sleep(10);
    dolphiimote_update();
  }
}