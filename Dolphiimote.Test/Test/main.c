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
int bKeepRunning = 1;
int bPause = 0;

void on_data_received(uint8_t wiimote_number, struct dolphiimote_wiimote_data *data, void *userdata)
{
	if (bPause == 0) {
		printf("wiimote %i: ", wiimote_number);
	}

  if(data->button_state & dolphiimote_BUTTON_A)
  {
    printf("A ");

    if(state != 1 + 10 * wiimote_number)
    {
      state = 1 + 10 * wiimote_number;
      dolphiimote_enable_capabilities(wiimote_number, dolphiimote_CAPABILITIES_MOTION_PLUS);
    }
  }

  if(data->button_state & dolphiimote_BUTTON_B)
  {
    printf("B ");
    if(state != 0 + 10 * wiimote_number)
    {
      state = 0 + 10 * wiimote_number;
      dolphiimote_enable_capabilities(wiimote_number, dolphiimote_CAPABILITIES_EXTENSION);
    }
  }

  if (data->button_state & dolphiimote_BUTTON_HOME)
  {
	  printf("Home ");
	  if (state != 2 + 10 * wiimote_number)
	  {
		  state = 2 + 10 * wiimote_number;
		  dolphiimote_enable_capabilities(wiimote_number, dolphiimote_CAPABILITIES_EXTENSION | dolphiimote_CAPABILITIES_MOTION_PLUS);
	  }
  }

  if ((data->button_state & dolphiimote_BUTTON_MINUS) && (data->button_state & dolphiimote_BUTTON_PLUS))
  {
	  bKeepRunning = 0;
  }

  if ((data->button_state & dolphiimote_BUTTON_ONE) && (data->button_state & dolphiimote_BUTTON_TWO))
  {
	  if (bPause == 1)
		  bPause = 0;
	  else
		  bPause = 1;
	  Sleep(500);
	  state = 2 + 10 * wiimote_number;
  }

  if (data->button_state & dolphiimote_BUTTON_DPAD_RIGHT) {
	  if (state != 5 + 10 * wiimote_number)
	  {
		  state = 5 + 10 * wiimote_number;
		  dolphiimote_set_rumble(wiimote_number, 1);
	  }
  }
  if (bPause == 0)
  {
	  if (data->button_state & dolphiimote_BUTTON_DPAD_DOWN)
	  {
		  printf("Down ");
		  dolphiimote_brief_rumble(wiimote_number);
	  }
	  if (data->button_state & dolphiimote_BUTTON_DPAD_RIGHT) {
		  dolphiimote_set_rumble(wiimote_number, 1);
		  printf("Right ");
	  }
	  if (data->button_state & dolphiimote_BUTTON_DPAD_LEFT) {
		  dolphiimote_set_rumble(wiimote_number, 0);
		  printf("Left ");
	  }
	  if (data->button_state & dolphiimote_BUTTON_DPAD_UP)
		  printf("Up ");
	  if (data->button_state & dolphiimote_BUTTON_MINUS)
		  printf("Minus ");
	  if (data->button_state & dolphiimote_BUTTON_PLUS)
		  printf("Plus ");
	  if (data->button_state & dolphiimote_BUTTON_ONE)
		  printf("One ");
	  if (data->button_state & dolphiimote_BUTTON_TWO)
		  printf("Two ");

    if (data->valid_data_flags & dolphiimote_BALANCE_BOARD_VALID)
  	  printf("Balance Board: Total: %02f, COGX: %02f, COGY: %02f", data->balance_board.weight_kg, data->balance_board.center_of_gravity_x, data->balance_board.center_of_gravity_y);

	  if (data->valid_data_flags & dolphiimote_ACCELERATION_VALID)
		  printf("Acc: %02d %02d %02d\t", data->acceleration.x, data->acceleration.y, data->acceleration.z);

	  if (data->valid_data_flags & dolphiimote_MOTIONPLUS_VALID)
		  printf("Motion Plus: %04X %04X %04X, Extension connected: %d\t", data->motionplus.yaw_down_speed, data->motionplus.pitch_left_speed, data->motionplus.roll_left_speed, data->motionplus.extension_connected);

	  if (data->valid_data_flags & dolphiimote_NUNCHUCK_VALID)
	  {
		  int c = data->nunchuck.buttons & dolphiimote_NUNCHUCK_BUTTON_C;
		  int z = data->nunchuck.buttons & dolphiimote_NUNCHUCK_BUTTON_Z;
		  printf("Nunchuck: C: %i, Z: %i, Acc: %02X%02X%02X Stick X: %i, Y: %i\t", c, z, data->nunchuck.x, data->nunchuck.y, data->nunchuck.z, data->nunchuck.stick_x, data->nunchuck.stick_y);
	  }

    if (data->valid_data_flags & dolphiimote_GUITAR_VALID)
    {
  	  printf("Guitar: %02d %02d %02d\t", data->guitar.stick_x, data->guitar.stick_y, data->guitar.whammy_bar);
	  if (data->guitar.is_gh3)
		  printf("Guitar hero three controller\t");
  	  if (data->guitar.buttons & dolphiimote_GUITAR_BUTTON_GREEN)
  		  printf("Green\t");
  	  if (data->guitar.buttons & dolphiimote_GUITAR_BUTTON_RED)
  		  printf("Red\t");
  	  if (data->guitar.buttons & dolphiimote_GUITAR_BUTTON_YELLOW)
  		  printf("Yellow\t");
  	  if (data->guitar.buttons & dolphiimote_GUITAR_BUTTON_BLUE)
  		  printf("Blue\t");
  	  if (data->guitar.buttons & dolphiimote_GUITAR_BUTTON_ORANGE)
  		  printf("Orange\t");

	  if (data->guitar.buttons & dolphiimote_GUITAR_BUTTON_PLUS)
		  printf("+\t");
	  if (data->guitar.buttons & dolphiimote_GUITAR_BUTTON_MINUS)
		  printf("-\t");
	  if (data->guitar.buttons & dolphiimote_GUITAR_BUTTON_STRUM_UP)
		  printf("Strum Down\t");
	  if (data->guitar.buttons & dolphiimote_GUITAR_BUTTON_STRUM_DOWN)
		  printf("Strum Up\t");
    }

	  if (data->valid_data_flags & dolphiimote_CLASSIC_CONTROLLER_VALID)
	  {

		  int lx = data->classic_controller.left_stick_x;
		  int ly = data->classic_controller.left_stick_y;
		  int rx = data->classic_controller.right_stick_x;
		  int ry = data->classic_controller.right_stick_y;
		  int lt = data->classic_controller.left_trigger;
		  int rt = data->classic_controller.right_trigger;

		  printf("Classic: L:%02d,%02d R:%02d,%02d T:%02d,%02d\t", lx, ly, rx, ry, lt, rt);

		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_A)
			  printf("A");
		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_B)
			  printf("B");
		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_X)
			  printf("X");
		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_Y)
			  printf("Y");

		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_DPAD_LEFT)
			  printf("DL");
		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_DPAD_RIGHT)
			  printf("DR");
		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_DPAD_DOWN)
			  printf("DD");
		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_DPAD_UP)
			  printf("DU");

		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_TRIGGER_LEFT)
			  printf("LT");
		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_TRIGGER_RIGHT)
			  printf("RT");

		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_Z_LEFT)
			  printf("LZ");
		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_Z_RIGHT)
			  printf("RZ");

		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_PLUS)
			  printf("+");
		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_MINUS)
			  printf("-");

		  if (data->classic_controller.buttons & dolphiimote_CLASSIC_CONTROLLER_BUTTON_HOME)
			  printf("H");
	  }

	  printf("\n");
  }
}

void on_capabilities_changed(uint8_t wiimote, dolphiimote_capability_status *status, void *userdata)
{
  printf("wiimote %i capabilities:\n", wiimote);
  printf("Extension: (0x%012llx) ", status->extension_id);
  switch(status->extension_type)
  {
    case dolphiimote_EXTENSION_NONE:
		state = 0 + 10 * wiimote;
      printf("None");
      break;
	case dolphiimote_EXTENSION_UNKNOWN:
		printf("Unknown");
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
    case dolphiimote_EXTENSION_MOTION_PLUS:
      printf("MotionPlus");
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

  dolphiimote_set_reporting_mode(wiimote, 0x35);
}

void on_connection_changed(uint8_t wiimote_number, int connected)
{
	bKeepRunning = 0;
}

void on_log_received(const char* str, uint32_t size)
{
  printf_s(str);
}

void init_dolphiimote(dolphiimote_callbacks callbacks)
{
  int wiimote_flags;
  int i;
  wiimote_flags = dolphiimote_init(callbacks);
}

int main()
{
  dolphiimote_callbacks callbacks = { 0 };
  int loop = 0;

  callbacks.data_received = on_data_received;
  callbacks.capabilities_changed = on_capabilities_changed;
  callbacks.connection_changed = on_connection_changed;
  callbacks.log_received = on_log_received;

  dolphiimote_log_level(dolphiimote_LOG_LEVEL_DEBUG);

  init_dolphiimote(callbacks);

  while(bKeepRunning)
  {
    dolphiimote_update();
    Sleep(10);
  }
  dolphiimote_shutdown();
}
