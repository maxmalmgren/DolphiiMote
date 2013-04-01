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

#ifndef DOLPHIIMOTE_DATA_SENDER_H
#define DOLPHIIMOTE_DATA_SENDER_H

#include "Util\collections.h"
#include "wiimote.h"
#include "dolphiimote.h"

namespace dolphiimote {
  class data_sender {
  public:
    data_sender(std::map<int, wiimote> &current_wiimote_state);
    void operator()();
    void send(const wiimote_message &message);

  private:
    void send_message(wiimote_message &message);

    timed_priority_queue<wiimote_message> messages;
    std::map<int, wiimote> &state;
  };
}

#endif