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

#include "serialization.h"
#include "wiimote.h"
namespace dolphiimote { namespace serialization {
	// length between board sensors
	const int BSL = 43;

	// width between board sensors
	const int BSW = 24;
	const float KG2LB = 2.20462262f;
    std::array<u8, 23> _start_rumble = { 0xA2, 0x15, 0x01 };
    std::array<u8, 23> _stop_rumble = { 0xA2, 0x15, 0x00 };

    const std::array<u8, 23>& start_rumble()
    {
      return _start_rumble;
    }

    const std::array<u8, 23>& stop_rumble()
    {
      return _stop_rumble;
    }

    size_t rumble_size()
    {
      return 3;
    }
	/**
	* map value from istart, istop to ostart, ostop
	*/
	float map(float value, float istart, float istop, float ostart, float ostop) {
		return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
	}
	/**
	* interpolate a balance board sensor value between the three calibration values
	*/
	float interpolate_balance_board(short sensor, short kg0, short kg17, short kg34)
	{
		if (kg0 == kg17 || kg0 == kg34 || sensor == 0)
			return 0;
		if (sensor < kg17)
			return map(sensor, kg0, kg17, 0, 17);
		else
			return map(sensor, kg17, kg34, 17, 34);
	}
	void retrieve_balance_board(checked_array<const u8> extension_data, wiimote state, dolphiimote_wiimote_data &output)
	{
		if (extension_data.size() >= 8)
		{
			output.valid_data_flags |= dolphiimote_BALANCE_BOARD_VALID;
			output.balance_board.raw.top_right = extension_data[0] << 8 | extension_data[1];
			output.balance_board.raw.bottom_right = extension_data[2] << 8 | extension_data[3];
			output.balance_board.raw.top_left = extension_data[4] << 8 | extension_data[5];
			output.balance_board.raw.bottom_left = extension_data[6] << 8 | extension_data[7];
			output.balance_board.kg.top_right = interpolate_balance_board(output.balance_board.raw.top_right, state.calibrations.balance_board.kg0.top_right, state.calibrations.balance_board.kg17.top_right, state.calibrations.balance_board.kg34.top_right);
			output.balance_board.kg.bottom_right = interpolate_balance_board(output.balance_board.raw.bottom_right, state.calibrations.balance_board.kg0.bottom_right, state.calibrations.balance_board.kg17.bottom_right, state.calibrations.balance_board.kg34.bottom_right);
			output.balance_board.kg.top_left = interpolate_balance_board(output.balance_board.raw.top_left, state.calibrations.balance_board.kg0.top_left, state.calibrations.balance_board.kg17.top_left, state.calibrations.balance_board.kg34.top_left);
			output.balance_board.kg.bottom_left = interpolate_balance_board(output.balance_board.raw.bottom_left, state.calibrations.balance_board.kg0.bottom_left, state.calibrations.balance_board.kg17.bottom_left, state.calibrations.balance_board.kg34.bottom_left);

			output.balance_board.lb.top_right = KG2LB * output.balance_board.kg.top_right;
			output.balance_board.lb.bottom_right = KG2LB * output.balance_board.kg.bottom_right;
			output.balance_board.lb.top_left = KG2LB * output.balance_board.kg.top_left;
			output.balance_board.lb.bottom_left = KG2LB * output.balance_board.kg.bottom_left;

			output.balance_board.calibration_kg0.top_right = state.calibrations.balance_board.kg0.top_right;
			output.balance_board.calibration_kg0.bottom_right = state.calibrations.balance_board.kg0.bottom_right;
			output.balance_board.calibration_kg0.top_left = state.calibrations.balance_board.kg0.top_left;
			output.balance_board.calibration_kg0.bottom_left = state.calibrations.balance_board.kg0.bottom_left;

			output.balance_board.calibration_kg17.top_right = state.calibrations.balance_board.kg17.top_right;
			output.balance_board.calibration_kg17.bottom_right = state.calibrations.balance_board.kg17.bottom_right;
			output.balance_board.calibration_kg17.top_left = state.calibrations.balance_board.kg17.top_left;
			output.balance_board.calibration_kg17.bottom_left = state.calibrations.balance_board.kg17.bottom_left;

			output.balance_board.calibration_kg34.top_right = state.calibrations.balance_board.kg34.top_right;
			output.balance_board.calibration_kg34.bottom_right = state.calibrations.balance_board.kg34.bottom_right;
			output.balance_board.calibration_kg34.top_left = state.calibrations.balance_board.kg34.top_left;
			output.balance_board.calibration_kg34.bottom_left = state.calibrations.balance_board.kg34.bottom_left;

			output.balance_board.weight_kg = (output.balance_board.kg.top_right + output.balance_board.kg.bottom_right + output.balance_board.kg.top_left + output.balance_board.kg.bottom_left);
			output.balance_board.weight_lb = (output.balance_board.lb.top_right + output.balance_board.lb.bottom_right + output.balance_board.lb.top_left + output.balance_board.lb.bottom_left);
			float right = (output.balance_board.raw.top_right + output.balance_board.raw.bottom_right);
			float left = (output.balance_board.raw.top_left + output.balance_board.raw.bottom_left);
			float top = (output.balance_board.raw.top_left + output.balance_board.raw.top_right);
			float bot = (output.balance_board.raw.bottom_left + output.balance_board.raw.bottom_right);
			if (left == 0 || right == 0 || output.balance_board.weight_kg < 0) {
				output.balance_board.center_of_gravity_pos_x = BSL / 2;
				output.balance_board.center_of_gravity_x = 0;
			} 
			else {
				float Kx = left / right;
				float val = (Kx - 1) / (Kx + 1);
				output.balance_board.center_of_gravity_x = (val - 0.5) * 100;
				output.balance_board.center_of_gravity_pos_x = val * BSL;
			}
			if (top == 0 || bot == 0 || output.balance_board.weight_kg < 0) {
				output.balance_board.center_of_gravity_pos_y = BSW / 2;
				output.balance_board.center_of_gravity_y = 0;
			}
			else {
				float Ky = top / bot;
				float val = (Ky - 1) / (Ky + 1);
				output.balance_board.center_of_gravity_y = (val - 0.5) * 100;
				output.balance_board.center_of_gravity_pos_y = val * BSW;
			}
			
		}
	}
  void retrieve_motion_plus(checked_array<const u8> extension_data, wiimote state, dolphiimote_wiimote_data &output)
  {
    u8 speed_mask = ~0x03;

    if(extension_data.size() >= 6 && (extension_data[5] & 0x02))
    {
      output.valid_data_flags |= dolphiimote_MOTIONPLUS_VALID;

      output.motionplus.yaw_down_speed = extension_data[0] + ((u16)(extension_data[3] & speed_mask) << 6);
      output.motionplus.roll_left_speed = extension_data[1] + ((u16)(extension_data[4] & speed_mask) << 6);
      output.motionplus.pitch_left_speed = extension_data[2] + ((u16)(extension_data[5] & speed_mask) << 6);

      output.motionplus.slow_modes = (extension_data[3] & 0x03) << 1 | (extension_data[4] & 0x02) >> 1;
      output.motionplus.extension_connected = extension_data[4] & 0x01;
    }
  }

  void retrieve_nunchuck(checked_array<const u8> extension_data, wiimote state, dolphiimote_wiimote_data &output)
  {
    if(extension_data.size() >= 6)
    {    
      output.valid_data_flags |= dolphiimote_NUNCHUCK_VALID;

      output.nunchuck.stick_x = extension_data[0];
      output.nunchuck.stick_y = extension_data[1];
      output.nunchuck.x = (extension_data[2] << 2) | (extension_data[5] & 0x0C) >> 2;
      output.nunchuck.y = (extension_data[3] << 2) | (extension_data[5] & 0x30) >> 4;
      output.nunchuck.z = (extension_data[4] << 2) | (extension_data[5] & 0xC0) >> 6;

      output.nunchuck.buttons = ~(extension_data[5]) & 0x3;
    }
  }

  void retrieve_interleaved_nunchuck(checked_array<const u8> extension_data, wiimote state, dolphiimote_wiimote_data &output)
  {
    if(extension_data.size() >= 6 && !(extension_data[5] & 0x03))
    {    
      output.valid_data_flags |= dolphiimote_NUNCHUCK_VALID;

      output.nunchuck.stick_x = extension_data[0];
      output.nunchuck.stick_y = extension_data[1];
      output.nunchuck.x = (extension_data[2] << 1) | (extension_data[5] & 0x10) >> 4;
      output.nunchuck.y = (extension_data[3] << 1) | (extension_data[5] & 0x20) >> 5;
      output.nunchuck.z = ((extension_data[4] & ~0x1) << 1 ) | (extension_data[5] & 0xC0) >> 6;

      output.nunchuck.buttons = ~(extension_data[5] >> 2) & 0x3;
    }
  }

  void retrieve_classic_controller(checked_array<const u8> extension_data, wiimote state, dolphiimote_wiimote_data &output)
  {
	  if(extension_data.size() >= 6)
    { 	  
      output.valid_data_flags |= dolphiimote_CLASSIC_CONTROLLER_VALID;

      output.classic_controller.left_stick_x = extension_data[0] & 0x3F;
      output.classic_controller.left_stick_y = extension_data[1] & 0x3F;

      output.classic_controller.right_stick_x = ((extension_data[0] & 0xC0) >> 3) | ((extension_data[1] & 0xC0) >> 5) | ((extension_data[2] & 0xC0) >> 7);
      output.classic_controller.right_stick_y = extension_data[2] & 0x1F;

      output.classic_controller.left_trigger = ((extension_data[2] & 0x60) >> 2) | ((extension_data[3] & 0xE0) >> 5);
      output.classic_controller.right_trigger = extension_data[3] & 0x1F;

      output.classic_controller.buttons = ~((extension_data[4] << 8) | extension_data[5]);
    }
  }

  void retrieve_guitar(checked_array<const u8> extension_data, wiimote state, dolphiimote_wiimote_data &output)
  {
	  if (extension_data.size() >= 6)
	  {
		  output.valid_data_flags |= dolphiimote_GUITAR_VALID;

		  output.guitar.is_gh3 = (extension_data[0] & 0x80) == 0x80;
		  output.guitar.stick_x = extension_data[0] & 0x3F;
		  output.guitar.stick_y = extension_data[1] & 0x3F;
		  output.guitar.tap_bar = extension_data[2] & 0x1F;
		  output.guitar.whammy_bar = extension_data[3] & 0x1F;
		  output.guitar.buttons = ~((extension_data[4] << 8) | extension_data[5]);
	  }
  }
  void retrieve_interleaved_classic_controller(checked_array<const u8> extension_data, wiimote state, dolphiimote_wiimote_data &output)
  {
	  if(extension_data.size() >= 6)
    {
      if((extension_data[5] & 0x03) == 0)
      {
        output.valid_data_flags |= dolphiimote_CLASSIC_CONTROLLER_VALID;

        output.classic_controller.left_stick_x = extension_data[0] & 0x3E;
        output.classic_controller.left_stick_y = extension_data[1] & 0x3E;

        output.classic_controller.right_stick_x = ((extension_data[0] & 0xC0) >> 3) | ((extension_data[1] & 0xC0) >> 5) | ((extension_data[2] & 0xC0) >> 7);
        output.classic_controller.right_stick_y = extension_data[2] & 0x1F;

        output.classic_controller.left_trigger = ((extension_data[2] & 0x60) >> 2) | ((extension_data[3] & 0xE0) >> 5);
        output.classic_controller.right_trigger = extension_data[3] & 0x1F;

        output.classic_controller.buttons = ~(((extension_data[4] & 0xFE ) << 8) | (extension_data[5] & 0xFC) | ((extension_data[1] & 0x01) << 1) | (extension_data[0] & 0x01));
      }
    }
  }

  void retrieve_button_state(u8 reporting_mode, checked_array<const u8> data, dolphiimote_wiimote_data &output)
  {
    if(data.size() > 4)
    {
      u8 first = data[2];
      u8 second = data[3];

      output.button_state = first << 8 | second;
    }
  }

  void retrieve_infrared_camera_data(u8 reporting_mode, checked_array<const u8> data, dolphiimote_wiimote_data &output)
  {
    
  }

  void retrieve_acceleration_data(u8 reporting_mode, checked_array<const u8> data, struct dolphiimote_wiimote_data &output)
  {
    if(data.size() > 7)
    {    
      output.valid_data_flags |= dolphiimote_ACCELERATION_VALID;

      output.acceleration.x = (data[4] << 2) | (data[2] >> 5 & 0x3);
      output.acceleration.y = (data[5] << 2) | (data[3] >> 4 & 0x2);
      output.acceleration.z = (data[6] << 2) | (data[3] >> 5 & 0x2);
    }
  }
  }
}