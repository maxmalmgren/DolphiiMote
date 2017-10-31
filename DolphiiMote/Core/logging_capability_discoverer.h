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

#ifndef DOLPHIIMOTE_LOGGING_CAPABILITY_DISCOVERER_H
#define DOLPHIIMOTE_LOGGING_CAPABILITY_DISCOVERER_H

#include "capability_discoverer.h"

namespace dolphiimote
{
  class logging_capability_discoverer : public capability_discoverer
  {
  public:    
    logging_capability_discoverer(std::map<int, wiimote> &wiimote_states, dolphiimote_callbacks callbacks, data_sender &sender, wiimote_reader &reader) : capability_discoverer(wiimote_states, callbacks, sender, reader)
    { }

	inline virtual void init_and_identify_extension_controller(int wiimote)
	{
		log(Info, "Wiimote #%i: Attempting to identify and initialize extension controller", wiimote);
		capability_discoverer::init_and_identify_extension_controller(wiimote);
	}
	inline virtual void send_status_request(int wiimote) {
		log(Info, "Wiimote #%i: Sending status request", wiimote);
		capability_discoverer::send_status_request(wiimote);
	}

  private:
    inline virtual void handle_extension_connected(int wiimote)
    {
      log(Info, "Wiimote #%i: Extension connected", wiimote);
      capability_discoverer::handle_extension_connected(wiimote);
    }

    inline virtual void handle_extension_disconnected(int wiimote)
    {
      log(Info, "Wiimote #%i: Extension disconnected", wiimote);
      capability_discoverer::handle_extension_disconnected(wiimote);
    }

    inline virtual void enable_motion_plus_no_passthrough(int wiimote)
    {
      log(Info, "Wiimote #%i: Enabling motionplus without passthrough", wiimote);
      capability_discoverer::enable_motion_plus_no_passthrough(wiimote);
    }

    inline virtual void enable_only_extension(int wiimote)
    {
      log(Info, "Wiimote #%i: Enabling only extension", wiimote);
      capability_discoverer::enable_only_extension(wiimote);
    }

    inline virtual void handle_motionplus_id_message(int wiimote, checked_array<const u8> data, dolphiimote_callbacks callbacks)
    {
      u8 error_bit = reader.read_error_bit(data);
	  if (is_set(wiimote_states[wiimote].available_capabilities, wiimote_capabilities::MotionPlus) != (error_bit == 0)) {
		  if (error_bit == 0)
			  log(Info, "Wiimote #%i: MotionPlus available", wiimote);
		  else log(Info, "Wiimote #%i: MotionPlus not available", wiimote);
	  }

      capability_discoverer::handle_motionplus_id_message(wiimote, data, callbacks);
    }

    virtual void handle_extension_id_message(int wiimote, checked_array<const u8> data, dolphiimote_callbacks callbacks)
    {
      u8 error_bit = reader.read_error_bit(data);

      if(error_bit == 0)
        log(Info, "Wiimote #%i: Extension available and enabled - id %06X", wiimote, read_extension_id(data.sub_array(7, 6)));
      else log(Info, "Wiimote #%i: Extension not available - could not read id", wiimote);

      capability_discoverer::handle_extension_id_message(wiimote, data, callbacks);
    }
  };
}
#endif