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
    status.extension_type = mote.extension_type;
    status.extension_id = mote.extension_id;
  }

  void capability_discoverer::dispatch_capabilities_changed(int wiimote, dolphiimote_callbacks callbacks)
  {
    dolphiimote_capability_status status = { 0 };

    fill_capabilities(status, wiimote_states[wiimote]);

    if(callbacks.capabilities_changed)
      callbacks.capabilities_changed(wiimote, &status, callbacks.userdata);
  }

  std::map<u64, wiimote_extensions> _id_to_extension_type;

  std::map<u64, wiimote_extensions>& id_to_extension_type()
  {
    if(_id_to_extension_type.size() == 0)
    {
      _id_to_extension_type[0x0000A4200000] = Nunchuck;
      _id_to_extension_type[0x0000A4200101] = ClassicController;
      _id_to_extension_type[0x0100A4200101] = ClassicControllerPro;
      _id_to_extension_type[0x0000A4200103] = GHGuitar;
      _id_to_extension_type[0x0100A4200103] = GHWorldTourDrums;
    }

    return _id_to_extension_type;
  }

  void capability_discoverer::update_extension_type_from_id(int wiimote_number)
  {
    auto id = wiimote_states[wiimote_number].extension_id;

    if(id_to_extension_type().find(id) != id_to_extension_type().end())
      wiimote_states[wiimote_number].extension_type = id_to_extension_type()[id];
  }

  u64 read_extension_id(checked_array<const u8> data)
  {
    u64 id = 0;

    for(unsigned int i = 0; i < 6; i++)
      id |= ((u64)data[5 - i]) << (i * 8);

    return id;
  }

  void capability_discoverer::handle_extension_id_message(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks)
  {
    u8 error_bit = read_error_bit(data);

    checked_array<const u8> extension_id = data.sub_array(7, 6);

    if(error_bit == 0 && data.valid())
      wiimote_states[wiimote_number].extension_id = read_extension_id(extension_id);
    else 
      wiimote_states[wiimote_number].extension_id = 0;

    update_extension_type_from_id(wiimote_number);
    dispatch_capabilities_changed(wiimote_number, callbacks);
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

  void capability_discoverer::send_extension_id_read_message(int wiimote_number)
  {
    reader.read(wiimote_number, 0XA400FA, 6, std::bind(&capability_discoverer::handle_extension_id_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    //According to wiibrew: Read 0xa400fa for 6 extension ID bytes.
  }

  void capability_discoverer::init_and_identify_extension_controller(int wiimote_number)
  {
    std::array<u8, 16> first_message = { 0x55 };
    std::array<u8, 16> second_message = { 0x00 };

    sender.write_register(wiimote_number, 0xA400F0, first_message, 1);
    sender.write_register(wiimote_number, 0xA400FB, second_message, 1, std::bind(&capability_discoverer::send_extension_id_read_message, this, std::placeholders::_1) );

    //According to wiibrew: init by writing 0x55 to 0x(4)A400F0, then writing 0x00 to 0x(4)A400FB
  }

  void capability_discoverer::determine_capabilities(int wiimote_number)
  {
    reader.read(wiimote_number, 0xA600FE, 0x02, std::bind(&capability_discoverer::handle_motionplus_id_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    init_and_identify_extension_controller(wiimote_number);
  }

  void capability_discoverer::send_status_request(int wiimote_number)
  {
    //TODO: Send status report
  }

  void capability_discoverer::enable(int wiimote_number, wiimote_capabilities capabilities_to_enable)
  {
    std::array<u8, 23> data = { 0xa2, 0x16, 0x04, 0xA6, 0x00, 0xFE, 0x01, 0x04 };
    sender.send(wiimote_message(wiimote_number, data, 8));
  }
}