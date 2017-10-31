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

#ifndef DOLPHIIMOTE_DOLPHIIMOTE_HOST_H
#define DOLPHIIMOTE_DOLPHIIMOTE_HOST_H

#include "../Dolphin/WiimoteReal.h"
#include "dolphiimote.h"
#include "Util/collections.h"
#include "wiimote.h"
#include "data_reporter.h"
#include "logging_capability_discoverer.h"
#include "rumbler.h"
#include "data_sender.h"
#include "wiimote_reader.h"

namespace dolphiimote {
  class dolphiimote_host : public WiimoteReal::wiimote_listener
  {
  public:
    dolphiimote_host(dolphiimote_callbacks callbacks);

	void do_rumble(int wiimote_number, bool enable);
	void set_leds(int wiimote_number, int leds);
    void do_brief_rumble(int wiimote_number);
	void request_status(int wiimote_number);
    void enable_capabilities(int wiimote_number, wiimote_capabilities::type capability);
    void request_reporting_mode(int wiimote_number, u8 mode);
    virtual void data_received(int wiimote_number, const u16 channel, const void* const data, const u32 size);
    virtual void wiimote_connection_changed(int wiimote_number, bool connected);
    void update();
    int number_of_wiimotes();

  private:
    dolphiimote_callbacks callbacks;

    /* Probable state - since dolphin sometimes alter for example LED itself we cannot be certain. */
    int init();
    std::map<int, wiimote> current_wiimote_state;    
    data_sender sender;
    data_reporter reporter;
    rumbler rumble;
    logging_capability_discoverer discoverer;
    wiimote_reader reader;
    std::vector<wiimote_data_handler*> handlers;
    std::vector<wiimote_data_handler*> init_handlers();
    int wiimotes_flag;
  };
}
#endif DOLPHIIMOTE_DOLPHIIMOTE_HOST_H