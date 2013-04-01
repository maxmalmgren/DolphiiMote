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

#ifndef DOLPHIIMOTE_RUMBLER_H
#define DOLPHIIMOTE_RUMBLER_H

#include "wiimote.h"
#include "serialization.h"
#include <map>
#include "data_sender.h"

namespace dolphiimote { 
  
  class rumbler
  {
  public:
    rumbler(std::map<int, wiimote> &current_wiimote_state, data_sender &sender) : sender(sender), current_wiimote_state(current_wiimote_state), brief_rumble_duration(200)
    { }

    void do_rumble(int wiimote_number)
    {
      if(current_wiimote_state.find(wiimote_number) != current_wiimote_state.end() && !current_wiimote_state[wiimote_number].rumble_active())
      {
        current_wiimote_state[wiimote_number].begin_brief_rumble();
        sender.send(wiimote_message(wiimote_number, serialization::start_rumble(), serialization::rumble_size(), [this](int wiimote_number) {
          auto time_to_end_vibration = steady_time_point::clock::now() + std::chrono::milliseconds(brief_rumble_duration);
          sender.send(wiimote_message(wiimote_number, time_to_end_vibration, serialization::stop_rumble(), serialization::rumble_size(), std::bind(&rumbler::on_end_rumble, this, std::placeholders::_1), false));                         
        }, false));
      }
    }

  private:
    data_sender &sender;
    std::map<int, wiimote> &current_wiimote_state;
    u16 brief_rumble_duration;

    void on_end_rumble(int wiimote_number)
    {
      current_wiimote_state[wiimote_number].end_brief_rumble();
    }
  };
}
#endif DOLPHIIMOTE_RUMBLER_H