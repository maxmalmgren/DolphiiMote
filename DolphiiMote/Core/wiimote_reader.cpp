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

#include "wiimote_reader.h"

namespace dolphiimote {
    wiimote_reader::wiimote_reader(data_sender &sender) : sender(sender), read_states()
    { }

    void insert_data(std::array<u8, 23> &data, uint32_t address, uint16_t size)
    {
      data[3] = (address >> 16) & 0xFF;
      data[4] = (address >> 8) & 0xFF;  //Requested data offset
      data[5] = address & 0xFF;

      data[6] = (size >> 8) & 0xFF;  //Requested data size
      data[7] = size & 0xFF;
    }

    u8 wiimote_reader::read_error_bit(checked_array<const u8> data)
    {
      return data[4] & 0x0F;
    }

    void wiimote_reader::operator()()
    {
      for(size_t i = 0; i < dolphiimote_MAX_WIIMOTES; i++)
      {
        auto& mote = read_states[i];

        if(mote.unfinished_request)
          return;

        if(!mote.requests.empty())
        {
          mote.unfinished_request = mote.requests.front();
          mote.requests.pop();

          std::array<u8, 23> request_data = { 0xA2, 0x17, 0x04 };
          insert_data(request_data, mote.unfinished_request.val().address, mote.unfinished_request.val().size);

          sender.send(wiimote_message(i, request_data, 8));
        }
      }
    }

    bool same_lowest_address(u8 first, u8 second, uint32_t address)
    {
      return (address & 0xFF) == first && ((address >> 8) & 0xFF) == second; 
    }

    void wiimote_reader::data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data)
    {
      if(data[1] == 0x21)
      {
        auto& state = read_states[wiimote_number];

        if(!state.unfinished_request || !same_lowest_address(data[6], data[5], state.unfinished_request.val().address))
          return;

        state.unfinished_request.val().callback(wiimote_number, data, callbacks);
        state.unfinished_request.invalidate();
      }
    }

    void wiimote_reader::read(int wiimote, uint32_t address, uint16_t size, std::function<void(int, checked_array<const u8>, dolphiimote_callbacks)> callback)
    {
      read_states[wiimote].requests.push(read_request(address, size, callback));
    }
}