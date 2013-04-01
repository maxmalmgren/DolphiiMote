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
    u8 message_type = data[1];

    // TODO: Handle status requests.
  }

  u8 read_error_bit(checked_array<const u8> data)
  {
    return data[4] & 0x0F;
  }

  void fill_capabilities(dolphiimote_capability_status &status, wiimote &mote)
  {
    status.available_capabilities = mote.available_capabilities;
    status.enabled_capabilities = mote.enabled_capabilities;
    std::memcpy(&status.extension_id, mote.extension_id.data(), 6);
  }

  void capability_discoverer::dispatch_capabilities_changed(int wiimote, dolphiimote_callbacks callbacks)
  {
    dolphiimote_capability_status status = { 0 };

    fill_capabilities(status, wiimote_states[wiimote]);

    if(callbacks.capabilities_changed)
      callbacks.capabilities_changed(wiimote, &status, callbacks.userdata);
  }

  void capability_discoverer::handle_motionplus_id_message(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks)
  {
    u8 error_bit = read_error_bit(data);

    if(error_bit == 0 && !(wiimote_states[wiimote_number].available_capabilities & wiimote_capabilities::MotionPlus))
    {
      wiimote_states[wiimote_number].available_capabilities |= wiimote_capabilities::MotionPlus;
      dispatch_capabilities_changed(wiimote_number, callbacks);
    }
  }

  void capability_discoverer::determine_capabilities(int wiimote_number)
  {
    reader.read(wiimote_number, 0xA600FE, 0x02, std::bind(&capability_discoverer::handle_motionplus_id_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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