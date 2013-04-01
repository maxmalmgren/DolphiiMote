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

#ifndef DOLPHIIMOTE_DATA_REPORTER_H
#define DOLPHIIMOTE_DATA_REPORTER_H

#include "wiimote.h"
#include "dolphiimote.h"
#include "data_sender.h"
#include "Util/collections.h"

namespace dolphiimote {  
  class data_reporter : public wiimote_data_handler
  {
  public:
    data_reporter(data_sender &sender) : sender(sender)
    { }
    void data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data);
    void request_reporting_mode(int wiimote_number, u8 reporting_mode);
  private:
    data_sender &sender;
  };
}
#endif DOLPHIIMOTE_DATA_REPORTER_H