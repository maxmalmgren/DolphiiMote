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

#ifndef _DOLPHIIMOTE_WIIMOTE_H
#define _DOLPHIIMOTE_WIIMOTE_H

#include "../Dolphin/CommonTypes.h"
#include "dolphiimote.h"
#include <chrono>
#include <array>
#include "Util/collections.h"
#include "Util/enum.h"

namespace dolphiimote
{
  typedef std::chrono::time_point<std::chrono::system_clock> steady_time_point;
  
  class wiimote_capabilities
  {    
  public:
    typedef enumeration<uint16_t> type;
    static const type None, MotionPlus, Extension, IR;
  };
  class trigger_calibration {
  public:
	  u8 min;
	  u8 max;
  };
  class stick_calibration {
  public:
	  u8 max_x;
	  u8 min_x;
	  u8 mid_X;
	  u8 max_y;
	  u8 min_y;
	  u8 mid_y;
  };
  class accel_calibration {
  public:
	  u8 X0;
	  u8 y0;
	  u8 Z0;
	  u8 XG;
	  u8 YG;
	  u8 ZG;
  };
  class calibration_classic_controller {
  public:
	  stick_calibration left_st;
	  stick_calibration right_st;
	  trigger_calibration left_tr;
	  trigger_calibration right_tr;
  };
  class calibration_nunchuck {
  public:
	  accel_calibration accel;
	  stick_calibration stick;
  };

  class calibration_balance_board 
  {
  public:
	  dolphiimote_balance_board_sensor_raw kg0;
	  dolphiimote_balance_board_sensor_raw kg17;
	  dolphiimote_balance_board_sensor_raw kg34;
  };
  class wiimote_calibrations
  {
  public:
	  calibration_balance_board balance_board;
	  //calibration_classic_controller classic_controller;
	  //calibration_nunchuck nunchuck;
	  //accel_calibration wiimote;
  };
  class wiimote_extensions
  { 
  public:
    typedef enumeration<uint32_t, 0> type;
    static const type None, Nunchuck, ClassicController, ClassicControllerPro, GHGuitar, GHWorldTourDrums, MotionPlus, Passthrough, BalanceBoard;
  };

  class wiimote_data_handler
  {
  public:
    virtual void data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data) = 0;
    virtual ~wiimote_data_handler() { }
  };

  struct timed_rumble
  {
    timed_rumble(std::chrono::milliseconds duration) : duration(duration), start()
    { }

    timed_rumble() : duration(), start()
    { }

    std::chrono::milliseconds duration;
    optional<steady_time_point> start;
  };

  class wiimote
  {
  public:

    wiimote() : extension_id(), available_capabilities(), enabled_capabilities(), extension_type(), reporting_mode(), led_state(), rumble_state()
    { }

    void begin_brief_rumble()
    {
      rumble_state = true;
    }

    void end_brief_rumble()
    {
      rumble_state = false;
    }

    bool rumble_active()
    {
      return rumble_state;
    }

    void set_extension_disabled()
    {
      enabled_capabilities &= ~wiimote_capabilities::Extension;
	  available_capabilities &= ~wiimote_capabilities::Extension;
      extension_id = 0;
      extension_type = 0;
    }

    u64 extension_id;
    wiimote_capabilities::type available_capabilities;
    wiimote_capabilities::type enabled_capabilities;
    wiimote_extensions::type extension_type;
    u8 reporting_mode;
    u8 led_state;
    bool rumble_state;  
	wiimote_calibrations calibrations;
  };

  class wiimote_message
  {
  public:  
    wiimote_message(int wiimote, std::array<u8, 23> message, size_t message_size, std::function<void(int)> callback, bool preserve_rumble = true) : wiimote_number(wiimote), _message(message), message_size(message_size), _send_time(steady_time_point::clock::now()), callback(callback), _preserve_rumble(preserve_rumble)
    { }

    wiimote_message(int wiimote, std::array<u8, 23> message, size_t message_size, bool preserve_rumble = true) : wiimote_number(wiimote), _message(message), message_size(message_size), _send_time(steady_time_point::clock::now()), callback([](int x) { }), _preserve_rumble(preserve_rumble)
    { }

    wiimote_message(int wiimote, steady_time_point send_time, std::array<u8, 23> message, size_t message_size, std::function<void(int)> callback,  bool preserve_rumble = true) : wiimote_number(wiimote), _message(message), message_size(message_size), _send_time(send_time), callback(callback), _preserve_rumble(preserve_rumble)
    { }

    wiimote_message() : wiimote_number(), _message(), message_size(), _send_time()
    { }

    int wiimote()
    {
      return wiimote_number;
    }

    std::array<u8, 23>& message()
    {
      return _message;
    }

    bool preserve_rumble()
    {
      return _preserve_rumble;
    }

    size_t size()
    {
      return message_size;
    }

    steady_time_point send_time()
    {
      return _send_time;
    }

    std::function<void(int)>& on_sent()
    {
      return callback;
    }

    bool operator<(const wiimote_message& other) const
    {
      return other._send_time < _send_time;
    }

  private:
    bool _preserve_rumble;
    std::function<void(int)> callback;
    steady_time_point _send_time;
    int wiimote_number;
    std::array<u8, 23> _message;
    size_t message_size;
  };
}

#endif