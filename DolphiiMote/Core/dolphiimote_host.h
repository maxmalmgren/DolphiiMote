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
#include "capability_discoverer.h"
#include "rumbler.h"

namespace dolphiimote {
  class dolphiimote_host : public WiimoteReal::wiimote_listener
  {
  public:
    dolphiimote_host() : callbacks(callbacks),
                         current_wiimote_state(),
                         messages(std::bind(&dolphiimote_host::send_message, this, std::placeholders::_1)),
                         reporter(),
                         rumble(current_wiimote_state, messages),
                         discoverer(current_wiimote_state),
                         handlers(init_handlers())
    { }

    int init(dolphiimote_callbacks callbacks)
    {
      this->callbacks = callbacks;

      WiimoteReal::listeners.add(this);
      WiimoteReal::LoadSettings();
      WiimoteReal::Initialize();
      WiimoteReal::Refresh();

      auto wiimotes_flag = WiimoteReal::FoundWiimotesFlag();

      for(int i = 0, flag = wiimotes_flag; i < MAX_WIIMOTES; i++, flag >>= 1)
        if(flag & 0x01)
          current_wiimote_state[i] = dolphiimote::wiimote();

      return wiimotes_flag;
    }

    void do_rumble(int wiimote_number)
    {
      rumble.do_rumble(wiimote_number);
    }

    virtual void data_received(int wiimote_number, const u16 channel, const void* const data, const u32 size)
    {
      auto u8_data = checked_array<const u8>((const u8*)data, size);

      for(auto& handler : handlers)
        handler->data_received(callbacks, wiimote_number, u8_data);
    }

    void request_reporting_mode(int wiimote_number, u8 mode)
    {
      reporter.request_reporting_mode(wiimote_number, mode);
    }

    virtual void wiimote_connection_changed(int wiimote_number, bool connected)
    { }

    void update()
    {
      messages.dispatch_expired();
    }

  private:
    dolphiimote_callbacks callbacks;

    /* Probable state - since dolphin sometimes alter for example LED itself we cannot be certain. */
    std::map<int, wiimote> current_wiimote_state;    
    timed_priority_queue<wiimote_message> messages;
    data_reporter reporter;
    rumbler rumble;
    capability_discoverer discoverer;
    std::vector<wiimote_data_handler*> handlers;

    void send_message(wiimote_message &message)
    {
      if(message.preserve_rumble())
      {
        message.message()[2] &= ~(0x1);
        message.message()[2] |= (u8)current_wiimote_state[message.wiimote()].rumble_active();
      }

      WiimoteReal::InterruptChannel(message.wiimote(), 65, &message.message()[0], message.size());
      message.on_sent()(message.wiimote());
    } 

    std::vector<wiimote_data_handler*> init_handlers()
    {
      std::vector<wiimote_data_handler*> local_handlers;
      local_handlers.push_back(&reporter);
      local_handlers.push_back(&discoverer);
      return local_handlers;
    }

    void enable(int wiimote_number, wiimote_capabilities capabilities_to_enable)
    {
        std::array<u8, 23> data = { 0xa2, 0x16, 0x04, 0xA6, 0x00, 0xFE, 0x01, 0x04 };
        WiimoteReal::InterruptChannel(wiimote_number, 65, &data[0], sizeof(data));
    }
  };
}
#endif DOLPHIIMOTE_DOLPHIIMOTE_HOST_H