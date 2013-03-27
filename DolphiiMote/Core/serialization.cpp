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
namespace dolphiimote { namespace serialization {

    std::array<u8, 21> _start_rumble = { 0xA2, 0x15, 0x01 };
    std::array<u8, 21> _stop_rumble = { 0xA2, 0x15, 0x00 };

    const std::array<u8, 21>& start_rumble()
    {
      return _start_rumble;
    }

    const std::array<u8, 21>& stop_rumble()
    {
      return _stop_rumble;
    }

    size_t rumble_size()
    {
      return 3;
    }
  }
}