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

#ifndef DOLPHIIMOTE_WIIMOTE_READER_H
#define DOLPHIIMOTE_WIIMOTE_READER_H

#include "wiimote.h"
#include "data_sender.h"
#include <queue>

namespace dolphiimote {
  class wiimote_reader : public wiimote_data_handler
  {
  public:
    wiimote_reader(data_sender &sender);
    void read(int wiimote, uint32_t address, uint16_t size, std::function<void(int, checked_array<const u8>, dolphiimote_callbacks)> callback);

    virtual void data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data);
    void operator()();
    u8 read_error_bit(checked_array<const u8> data);

  private:
    struct read_request
    {
      read_request(uint32_t address, uint16_t size, std::function<void(int, checked_array<const u8>, dolphiimote_callbacks)> &callback) : address(address), size(size), callback(callback)
      { }

      read_request() : address(), size(), callback()
      { }
      
      uint32_t address;
      uint16_t size;
      std::function<void(int, checked_array<const u8>, dolphiimote_callbacks)> callback;
    };

    struct wiimote_read_state
    {
      wiimote_read_state() : requests(), unfinished_request()
      { }

      std::queue<read_request> requests;
      optional<read_request> unfinished_request;
    };

    data_sender &sender;
    std::array<wiimote_read_state, dolphiimote_MAX_WIIMOTES> read_states;
  };
}

#endif