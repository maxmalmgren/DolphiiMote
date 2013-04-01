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

#include <map>
#include "wiimote.h"
#include "data_sender.h"

namespace dolphiimote
{
  class capability_discoverer : public wiimote_data_handler
  {
  public:
    
    capability_discoverer(std::map<int, wiimote> &wiimote_states, data_sender &sender) : wiimote_states(wiimote_states), sender(sender)
    { }

    virtual void data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data);

    void determine_capabilities(int wiimote_number);
    void send_status_request(int wiimote_number);
    void enable(int wiimote_number, wiimote_capabilities capabilities_to_enable);

  private:
    std::map<int, wiimote> &wiimote_states;
    data_sender &sender;
  };
}
#endif DOLPHIIMOTE_CAPABILITY_DISCOVERER_H