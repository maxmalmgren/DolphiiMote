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

#include "data_sender.h"

namespace dolphiimote {
    data_sender::data_sender(std::map<int, wiimote> &current_wiimote_state) : messages(std::bind(&data_sender::send_message, this, std::placeholders::_1)), state(current_wiimote_state)
    { }

    void data_sender::operator()()
    {
      messages.dispatch_expired();
    }

    void data_sender::send(const wiimote_message &message)
    {
      messages.push(message);
    }

    void data_sender::send_message(wiimote_message &message)
    {
      if(message.preserve_rumble())
      {
        message.message()[2] &= ~(0x1);
        message.message()[2] |= (u8)state[message.wiimote()].rumble_active();
      }

      WiimoteReal::InterruptChannel(message.wiimote(), 65, &message.message()[0], message.size());
      message.on_sent()(message.wiimote());
    }
}