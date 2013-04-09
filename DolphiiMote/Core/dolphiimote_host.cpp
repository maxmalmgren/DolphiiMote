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

#include "dolphiimote_host.h"
#include "Util/log.h"

namespace dolphiimote {
  dolphiimote_host::dolphiimote_host(dolphiimote_callbacks callbacks) : callbacks(callbacks),
                                                                        current_wiimote_state(),
                                                                        sender(current_wiimote_state),
                                                                        reader(sender),
                                                                        reporter(current_wiimote_state, sender),
                                                                        rumble(current_wiimote_state, sender),
                                                                        discoverer(current_wiimote_state, callbacks, sender, reader),
                                                                        handlers(init_handlers()),
                                                                        wiimotes_flag(init())
  { }

  int dolphiimote_host::init()
  {
    log_keeper::instance().set_output([this](std::string &output)
    {
      if(callbacks.log_received)
        callbacks.log_received(output.c_str(), output.size());
    });

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

  void dolphiimote_host::do_rumble(int wiimote_number)
  {
    rumble.do_rumble(wiimote_number);
  }

  void dolphiimote_host::data_received(int wiimote_number, const u16 channel, const void* const data, const u32 size)
  {
    auto u8_data = checked_array<const u8>((const u8*)data, size);

    for(auto& handler : handlers)
      handler->data_received(callbacks, wiimote_number, u8_data);
  }

  void dolphiimote_host::determine_capabilities(int wiimote_number)
  {
    discoverer.determine_capabilities(wiimote_number);
  }

  int dolphiimote_host::number_of_wiimotes()
  {
    return wiimotes_flag;
  }

  void dolphiimote_host::enable_capabilities(int wiimote_number, wiimote_capabilities::type capability)
  {
    discoverer.enable(wiimote_number, capability);
  }

  void dolphiimote_host::request_reporting_mode(int wiimote_number, u8 mode)
  {
    reporter.request_reporting_mode(wiimote_number, mode);
  }

  void dolphiimote_host::wiimote_connection_changed(int wiimote_number, bool connected)
  { }

  void dolphiimote_host::update()
  {
    reader();
    sender();    
  }

  std::vector<wiimote_data_handler*> dolphiimote_host::init_handlers()
  {
    std::vector<wiimote_data_handler*> local_handlers;
    local_handlers.push_back(&reporter);
    local_handlers.push_back(&discoverer);
    local_handlers.push_back(&reader);
    return local_handlers;
  }
}