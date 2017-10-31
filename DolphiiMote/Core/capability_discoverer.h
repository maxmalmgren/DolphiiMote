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
#include "wiimote_reader.h"

namespace dolphiimote
{
  class capability_discoverer : public wiimote_data_handler
  {
  public:
    
    capability_discoverer(std::map<int, wiimote> &wiimote_states, dolphiimote_callbacks callbacks, data_sender &sender, wiimote_reader &reader) : wiimote_states(wiimote_states), callbacks(callbacks), sender(sender), reader(reader)
    { }

    virtual void data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data);

    virtual void init_and_identify_extension_controller(int wiimote_number);
    virtual void send_status_request(int wiimote_number);
    virtual void enable(int wiimote_number, wiimote_capabilities::type capabilities_to_enable);
	virtual void handle_motion_plus_extension(int wiimote_number, bool extension_connected);

  protected:
    virtual void handle_status_report(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks);
    virtual void update_extension_type_from_id(int wiimote_number);
    virtual void handle_extension_id_message(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks);
	virtual void handle_extension_id_message_test(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks);
	virtual void handle_extension_id_message_test2(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks);
	virtual void handle_balanceboard_calibration1(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks);
	virtual void handle_balanceboard_calibration2(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks);
    virtual void handle_motionplus_id_message(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks);
    virtual void send_extension_id_read_message(int wiimote_number);
    virtual void dispatch_capabilities_changed(int wiimote, dolphiimote_callbacks callbacks);
    virtual void handle_motion_plus_and_extension_enabling(int wiimote_number, wiimote_capabilities::type capabilities_to_enable);
    virtual void handle_extension_controller_changed(bool extension_controller_connected, int wiimote, bool& changed);
	virtual void handle_motion_plus_passthrough_disable(int wiimote_number);
	virtual void handle_motion_plus_extension_id_message(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks);
    virtual void enable_motion_plus_no_passthrough(int wiimote_number);
    virtual void enable_motion_plus_extension_passthrough(int wiimote_number);
    virtual void enable_only_extension(int wiimote);
    virtual void handle_extension_connected(int wiimote);
    virtual void handle_extension_disconnected(int wiimote);

    u64 capability_discoverer::read_extension_id(checked_array<const u8> data);

    std::map<int, wiimote> &wiimote_states;
    dolphiimote_callbacks callbacks;
    data_sender &sender;
    wiimote_reader &reader;
  };
}
#endif