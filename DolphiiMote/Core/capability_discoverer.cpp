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

  std::map<u64, wiimote_extensions::type> _id_to_extension_type;

  std::map<u64, wiimote_extensions::type>& id_to_extension_type()
  {
    if(_id_to_extension_type.size() == 0)
    {
      _id_to_extension_type[0x0000A4200000] = wiimote_extensions::Nunchuck;
      _id_to_extension_type[0xFF00A4200000] = wiimote_extensions::Nunchuck; //WEIRD - had a nunchuk that always gave off this ID. Was there any difference?
      _id_to_extension_type[0x0000A4200101] = wiimote_extensions::ClassicController;
      _id_to_extension_type[0x0100A4200101] = wiimote_extensions::ClassicControllerPro;
      _id_to_extension_type[0x0000A4200103] = wiimote_extensions::GHGuitar;
      _id_to_extension_type[0x0100A4200103] = wiimote_extensions::GHWorldTourDrums;
      _id_to_extension_type[0x0000A4200405] = wiimote_extensions::MotionPlus;
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
    {
      wiimote_states[wiimote_number].extension_id = read_extension_id(extension_id);
      wiimote_states[wiimote_number].enabled_capabilities |= wiimote_capabilities::Extension;
    }
    else
    {
      wiimote_states[wiimote_number].extension_id = 0;
      wiimote_states[wiimote_number].enabled_capabilities &= ~wiimote_capabilities::Extension;
    }

    update_extension_type_from_id(wiimote_number);
    dispatch_capabilities_changed(wiimote_number, callbacks);
  }

  void capability_discoverer::handle_motionplus_id_message(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks)
  {
    u8 error_bit = read_error_bit(data);

    if(error_bit == 0)
    {
      wiimote_states[wiimote_number].available_capabilities |= wiimote_capabilities::MotionPlus;      
    }
    else
    {
      wiimote_states[wiimote_number].available_capabilities &= ~wiimote_capabilities::MotionPlus;      
    }

    dispatch_capabilities_changed(wiimote_number, callbacks);
  }

  void capability_discoverer::send_extension_id_read_message(int wiimote_number)
  {
    reader.read(wiimote_number, 0XA400FA, 6, std::bind(&capability_discoverer::handle_extension_id_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    //According to wiibrew: Read 0xa400fa for 6 extension ID bytes.
  }

  void capability_discoverer::init_and_identify_extension_controller(int wiimote_number)
  {
    sender.write_register(wiimote_number, 0xA400F0, 0x55, 1);
    sender.write_register(wiimote_number, 0xA400FB, 0x00, 1, std::bind(&capability_discoverer::send_extension_id_read_message, this, std::placeholders::_1));

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

  void enable_motion_plus_no_passthrough(int wiimote_number, data_sender &sender)
  {
    sender.write_register(wiimote_number, 0xA600FE, 0x4, 1);
  }

  void capability_discoverer::enable_only_extension(int wiimote)
  {
    if(is_set(wiimote_states[wiimote].enabled_capabilities, wiimote_capabilities::MotionPlus))
      sender.write_register(wiimote, 0xA400F0, 0x55, 1);  //According to Wiibrew: Writing 0x55 to 0x(4)A400F0 deactivates the MotionPlus      

    if(!is_set(wiimote_states[wiimote].enabled_capabilities, wiimote_capabilities::Extension))
      init_and_identify_extension_controller(wiimote);
  }

  void capability_discoverer::handle_motion_plus_and_extension_enabling(int wiimote_number, wiimote_capabilities::type capabilities_to_enable)
  {
    if(is_set(capabilities_to_enable, wiimote_capabilities::Extension) && is_set(capabilities_to_enable, wiimote_capabilities::MotionPlus))
    {
      enable_motion_plus_no_passthrough(wiimote_number, sender);
      //TODO: Implement passthrough mode for nunchuck atleast.
    }
    else if(is_set(capabilities_to_enable, wiimote_capabilities::Extension))
      enable_only_extension(wiimote_number);
    else if(is_set(capabilities_to_enable, wiimote_capabilities::MotionPlus))
      enable_motion_plus_no_passthrough(wiimote_number, sender);
    else
    {
      //Don't really need to disable; doesn't hurt.
    }
  }

  void capability_discoverer::enable(int wiimote_number, wiimote_capabilities::type capabilities_to_enable)
  {
    if(capabilities_to_enable == wiimote_states[wiimote_number].enabled_capabilities)
      return;

    if((capabilities_to_enable & wiimote_states[wiimote_number].available_capabilities) != capabilities_to_enable)
      return;

    handle_motion_plus_and_extension_enabling(wiimote_number, capabilities_to_enable);
  }
}