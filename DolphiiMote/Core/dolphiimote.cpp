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

#include "dolphiimote.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <array>
#include <map>
#include <vector>
#include <functional>
#include "Util/collections.h"
#include "wiimote.h"
#include "serialization.h"
#include "data_reporter.h"
#include "capability_discoverer.h"
#include "rumbler.h"
#include <memory>

namespace dolphiimote
{
  dolphiimote_callbacks callbacks;

  /* Probable state - since dolphin sometimes alter for example LED itself we cannot be certain. */
  std::map<int, wiimote> current_wiimote_state;

  std::vector<wiimote_data_handler*> handlers;

  data_reporter reporter;
  std::unique_ptr<rumbler> rumble;
  std::unique_ptr<capability_discoverer> discoverer;
  

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

  timed_priority_queue<wiimote_message> messages(send_message);

  wiimote_capabilities check_enabled_extension(int wiimote_number)
  {
    return MotionPlus;
  }

  void init_handlers()
  {
    discoverer = std::unique_ptr<capability_discoverer>(new capability_discoverer(current_wiimote_state));
    rumble = std::unique_ptr<rumbler>(new rumbler(current_wiimote_state, messages));
    
    handlers.push_back(&reporter);
    handlers.push_back(&(*discoverer));
  }

  void enable(int wiimote_number, wiimote_capabilities capabilities_to_enable)
  {
      std::array<u8, 23> data = { 0xa2, 0x16, 0x04, 0xA6, 0x00, 0xFE, 0x01, 0x04 };
      WiimoteReal::InterruptChannel(wiimote_number, 65, &data[0], sizeof(data));
  }

  class dolphiimote_wiimote_listener : public WiimoteReal::wiimote_listener
  {
  public:
    virtual void data_received(int wiimote_number, const u16 channel, const void* const data, const u32 size)
    {
      auto u8_data = checked_array<const u8>((const u8*)data, size);

      for(auto& handler : handlers)
        handler->data_received(callbacks, wiimote_number, u8_data);
    }

    virtual void wiimote_connection_changed(int wiimote_number, bool connected)
    { }
  } listener;
}

int dolphiimote_init(dolphiimote_callbacks _callback, void *userdata)
{
  dolphiimote::callbacks = _callback;

  dolphiimote::init_handlers();

  WiimoteReal::listeners.add(&dolphiimote::listener);
  WiimoteReal::LoadSettings();
  WiimoteReal::Initialize();
  WiimoteReal::Refresh();

  auto wiimotes_flag = WiimoteReal::FoundWiimotesFlag();

  for(int i = 0, flag = wiimotes_flag; i < MAX_WIIMOTES; i++, flag >>= 1)
    if(flag & 0x01)
      dolphiimote::current_wiimote_state[i] = dolphiimote::wiimote();

  return wiimotes_flag;
}

void dolphiimote_brief_rumble(int wiimote_number)
{
  dolphiimote::rumble->do_rumble(wiimote_number);
}

void dolphiimote_set_reporting_mode(int wiimote_number, uint8_t mode)
{
  dolphiimote::reporter.request_reporting_mode(wiimote_number, mode);
}

void dolphiimote_update()
{
  for(int i = 0; i < 4; i++)
  {
    dolphiimote::messages.dispatch_expired();
    WiimoteReal::Update(i);
  }
}