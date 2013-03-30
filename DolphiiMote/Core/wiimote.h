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
#include <array>

namespace dolphiimote
{

  typedef std::chrono::time_point<std::chrono::system_clock> steady_time_point;

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

    wiimote() : reporting_mode(), led_state(), rumble_state()
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

  private:
    u8 reporting_mode;
    u8 led_state;
    bool rumble_state;
  
  };

  class wiimote_message
  {
  public:  
    wiimote_message(int wiimote, std::array<u8, 21> message, size_t message_size, std::function<void(int)> callback) : wiimote_number(wiimote), _message(message), message_size(message_size), _send_time(steady_time_point::clock::now()), callback(callback)
    { }

    wiimote_message(int wiimote, steady_time_point send_time, std::array<u8, 21> message, size_t message_size, std::function<void(int)> callback) : wiimote_number(wiimote), _message(message), message_size(message_size), _send_time(send_time), callback(callback)
    { }

    wiimote_message() : wiimote_number(), _message(), message_size(), _send_time()
    { }

    int wiimote()
    {
      return wiimote_number;
    }

    const std::array<u8, 21>& message()
    {
      return _message;
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
    std::function<void(int)> callback;
    steady_time_point _send_time;
    int wiimote_number;
    std::array<u8, 21> _message;
    size_t message_size;
  };
}

#endif