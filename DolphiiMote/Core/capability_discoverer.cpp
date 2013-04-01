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

#include "capability_discoverer.h"

namespace dolphiimote {
  void capability_discoverer::data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data)
  {
    //TODO: Separate status requests and read reports.
  }

  void capability_discoverer::determine_capabilities(int wiimote_number)
  {
    //00 00 A6 20 00 05 at register address 0x(4)a600fa
  }

  void capability_discoverer::send_status_request(int wiimote_number)
  {
    //TODO: Send status report
  }

  void capability_discoverer::enable(int wiimote_number, wiimote_capabilities capabilities_to_enable)
  {
    std::array<u8, 21> data = { 0xa2, 0x16, 0x04, 0xA6, 0x00, 0xFE, 0x01, 0x04 };
    sender.send(wiimote_message(wiimote_number, data, 8));
  }
}