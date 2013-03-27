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

#ifndef _DOLPHIIMOTE_SERIALIZATION_H
#define _DOLPHIIMOTE_SERIALIZATION_H

#include <array>
#include "../Dolphin/CommonTypes.h"

namespace dolphiimote { namespace serialization {
    const std::array<u8, 21>& start_rumble();
    const std::array<u8, 21>& stop_rumble();
    size_t rumble_size();
  }
}

#endif