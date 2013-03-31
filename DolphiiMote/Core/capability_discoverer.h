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

#ifndef DOLPHIIMOTE_CAPABILITY_DISCOVERER_H
#define DOLPHIIMOTE_CAPABILITY_DISCOVERER_H

#include <vector>
#include <algorithm>
#include <functional>
#include <stdint.h>
#include "wiimote.h"

namespace dolphiimote
{
  class capability_discoverer : public wiimote_data_handler
  {
  public:
    
    capability_discoverer(std::map<int, wiimote> &wiimote_states) : wiimote_states(wiimote_states)
    { }

    virtual void data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data);

    void begin_determine_capabilities(int wiimote_number)
    {
      //send_packet(wiimote_number,

        //00 00 A6 20 00 05 at register address 0x(4)a600fa
    }

    void send_status_request(int wiimote_number)
    {

    }

    wiimote_capabilities check_capabilities(int wiimote_number)
    {
      return MotionPlus;
    }

  private:
    std::map<int, wiimote> &wiimote_states;

  };
}
#endif DOLPHIIMOTE_CAPABILITY_DISCOVERER_H