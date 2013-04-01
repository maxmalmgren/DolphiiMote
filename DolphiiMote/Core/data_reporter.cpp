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

#include "data_reporter.h"
#include "serialization.h"

namespace dolphiimote {
    std::map<wiimote_capabilities, std::function<void(checked_array<const u8>, dolphiimote_wiimote_data&)>> extension_retrievers;
    std::vector<std::pair<u16, std::function<void(u8, checked_array<const u8>, dolphiimote_wiimote_data&)>>> standard_retrievers;
    std::map<u16, range> reporting_mode_extension_data_offset;
    bool init;

    void setup_retrievers()
    {
      standard_retrievers.push_back(std::make_pair(0xFFFF, serialization::retrieve_button_state));
      standard_retrievers.push_back(std::make_pair(2 | 8 | 32 | 128, serialization::retrieve_acceleration_data));
      standard_retrievers.push_back(std::make_pair(8 | 64 | 128, serialization::retrieve_infrared_camera_data)); 

      extension_retrievers[MotionPlus] = serialization::retrieve_motion_plus;
    }

    void setup_extension_offsets()
    {
      reporting_mode_extension_data_offset[0x32] = range(4, 8);
      reporting_mode_extension_data_offset[0x34] = range(4, 19);
      reporting_mode_extension_data_offset[0x35] = range(7, 16);
      reporting_mode_extension_data_offset[0x36] = range(14, 9);
      reporting_mode_extension_data_offset[0x37] = range(17, 6);
    }

    void setup_data()
    {
      setup_retrievers();
      setup_extension_offsets();
      init = true;
    }

    u16 to_maskable(u8 reporting_mode)
    {
      int iterator = reporting_mode - 0x30; // lowest reporting mode.
      u16 result = 1;

      for(int i = 0; i < iterator; i++)
        result *= 2;

      return result;
    }

    void retrieve_standard_data(u8 reporting_mode, checked_array<const u8> data, dolphiimote_wiimote_data& wiimote_data)
    {
      u16 maskable_reporting_mode = to_maskable(reporting_mode);
      std::function<bool(u16)> filter = [=](u16 mask) { return (maskable_reporting_mode & mask) > 0; };

      auto applicable_standard_retrievers = pair_filter(standard_retrievers, filter);
      for(auto& retriever : take_second(applicable_standard_retrievers))
        retriever(reporting_mode, data, wiimote_data);
    }

    void retrieve_extension_data(int wiimote_number, checked_array<const u8> data, dolphiimote_wiimote_data &output)
    {
      wiimote_capabilities enabled_extension = wiimote_capabilities::MotionPlus;

      if(extension_retrievers.find(enabled_extension) != extension_retrievers.end())
        extension_retrievers[enabled_extension](data, output);
    }

    void handle_data_reporting(dolphiimote_callbacks &callbacks, int wiimote_number, u8 reporting_mode, checked_array<const u8> data)
    {
      dolphiimote_wiimote_data wiimote_data = { 0 };

      retrieve_standard_data(reporting_mode, data, wiimote_data);

      if(reporting_mode_extension_data_offset.find(reporting_mode) != reporting_mode_extension_data_offset.end())
      {
        retrieve_extension_data(wiimote_number,
                                data.sub_array(reporting_mode_extension_data_offset[reporting_mode].offset,
                                               reporting_mode_extension_data_offset[reporting_mode].size),
                                wiimote_data);
      }

      callbacks.data_received(wiimote_number, &wiimote_data, callbacks.userdata);
    }

    void data_reporter::data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data)
    {
      if(!init)
        setup_data();

      u8 hid_type = data[0];
      u8 message_type = data[1];

      if(message_type > 0x30 && message_type < 0x37)
        handle_data_reporting(callbacks, wiimote_number, message_type, data);
    }

    void data_reporter::request_reporting_mode(int wiimote, u8 reporting_mode)
    {
      if(reporting_mode >= 0x30 && reporting_mode <= 0x37)
      {
        std::array<u8, 21> reportingMode = { 0xa2, 0x12, 0x00, reporting_mode };
        sender.send(wiimote_message(wiimote, reportingMode, 4));
      }
    }
  }