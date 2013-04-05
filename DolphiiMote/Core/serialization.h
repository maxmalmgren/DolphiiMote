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
#include "Util\collections.h"
#include "../Dolphin/CommonTypes.h"
#include "dolphiimote.h"

namespace dolphiimote { namespace serialization {
  const std::array<u8, 23>& start_rumble();
  const std::array<u8, 23>& stop_rumble();
  size_t rumble_size();
  void retrieve_motion_plus(checked_array<const u8> extension_data, dolphiimote_wiimote_data &output);
  void retrieve_button_state(u8 reporting_mode, checked_array<const u8> data, dolphiimote_wiimote_data &output);
  void retrieve_infrared_camera_data(u8 reporting_mode, checked_array<const u8> data, dolphiimote_wiimote_data &output);
  void retrieve_acceleration_data(u8 reporting_mode, checked_array<const u8> data, struct dolphiimote_wiimote_data &output);
  }
}

#endif