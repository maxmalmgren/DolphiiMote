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
#include <concurrent_priority_queue.h>
#include "serialization.h"

namespace dolphiimote
{
  enum wiimote_capabilities { Unknown = 1, MotionPlus = 2 };

  struct range
  {
    range(u32 offset, u32 size) : offset(offset), size(size)
    { }

    range() : offset(), size()
    { }

    u32 offset;
    u32 size;
  };

  update_callback_t callback;
  void* callback_userdata;
  
  std::map<wiimote_capabilities, std::function<void(checked_array<const u8>, dolphiimote_wiimote_data&)>> extension_retrievers;
  std::vector<std::pair<u16, std::function<void(u8, checked_array<const u8>, dolphiimote_wiimote_data&)>>> standard_retrievers;
  std::map<u16, range> reporting_mode_extension_data_offset;

  /* Probable state - since dolphin sometimes alter for example LED itself we cannot be certain. */
  std::map<int, wiimote> current_wiimote_state;
  u16 brief_rumble_duration = 200;

  Concurrency::concurrent_priority_queue<wiimote_message> messages;

  template <typename T>
  std::string to_hex(T arg)
  {
    std::stringstream sstream;
    sstream << std::hex << std::uppercase << std::setw(2) << std::setfill('0')  << arg;
    return sstream.str();
  }

  void print_raw_data(int wiimote_number, u16 channelID, const u8* data, u32 size)
  {
    std::cout << wiimote_number << " ";

    for(size_t i = 0; i < size; i++)
    {
      auto string = to_hex((int)data[i]);
      std::cout << " " << string << " ";
    }

    std::cout << std::endl;
  }

  void retrieve_motion_plus(checked_array<const u8> extension_data, dolphiimote_wiimote_data &output)
  {
    u8 speed_mask = ~0x03;

    if(extension_data.size() >= 6)
    {
      output.valid_data_flags |= dolphiimote_MOTIONPLUS_VALID;

      output.motionplus.yaw_down_speed = extension_data[0] + ((u16)(extension_data[3] & speed_mask) << 6);
      output.motionplus.roll_left_speed = extension_data[1] + ((u16)(extension_data[4] & speed_mask) << 6);
      output.motionplus.pitch_left_speed = extension_data[2] + ((u16)(extension_data[5] & speed_mask) << 6);

      output.motionplus.slow_modes = (extension_data[3] & 0x03) << 1 | (extension_data[4] & 0x02) >> 1;
      output.motionplus.extension_connected = extension_data[4] & 0x01;
    }
  }

  void retrieve_button_state(u8 reporting_mode, checked_array<const u8> data, dolphiimote_wiimote_data &output)
  {
    if(data.size() > 4)
    {
      u8 first = data[2];
      u8 second = data[3];

      output.button_state = first << 8 | second;
    }
  }

  void retrieve_infrared_camera_data(u8 reporting_mode, checked_array<const u8> data, dolphiimote_wiimote_data &output)
  {
    
  }

  void retrieve_acceleration_data(u8 reporting_mode, checked_array<const u8> data, struct dolphiimote_wiimote_data &output)
  {
    if(data.size() > 7)
    {    
      output.valid_data_flags |= dolphiimote_ACCELERATION_VALID;

      output.acceleration.x = data[4];
      output.acceleration.y = data[5];
      output.acceleration.z = data[6];
    }
  }

  void setup_retrievers()
  {
    standard_retrievers.push_back(std::make_pair(0xFFFF, retrieve_button_state));
    standard_retrievers.push_back(std::make_pair(2 | 8 | 32 | 128, retrieve_acceleration_data));
    standard_retrievers.push_back(std::make_pair(8 | 64 | 128, retrieve_infrared_camera_data)); 

    extension_retrievers[MotionPlus] = retrieve_motion_plus;
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
    dolphiimote::setup_retrievers();
    dolphiimote::setup_extension_offsets();
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

  wiimote_capabilities check_enabled_extension(int wiimote_number)
  {
    return MotionPlus;
  }

  void retrieve_extension_data(int wiimote_number, checked_array<const u8> data, dolphiimote_wiimote_data &output)
  {
    wiimote_capabilities enabled_extension = check_enabled_extension(wiimote_number);

    if(extension_retrievers.find(enabled_extension) != extension_retrievers.end())
      extension_retrievers[enabled_extension](data, output);
  }

  void handle_data_reporting(int wiimote_number, u8 reporting_mode, checked_array<const u8> data)
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

    callback(wiimote_number, &wiimote_data, callback_userdata);
  }

  void response_received(int wiimote_number, u16 channel_id, const void* data, u32 size)
  {
    auto u8_data = checked_array<const u8>((const u8*)data, size);

    u8 hid_type = u8_data[0];
    u8 message_type = u8_data[1];

    if(message_type > 0x30 && message_type < 0x37)
      handle_data_reporting(wiimote_number, message_type, u8_data);
  }

  void begin_determine_capabilities(int wiimote_number)
  {

  }

  wiimote_capabilities check_capabilities(int wiimote_number)
  {
    return MotionPlus;
  }

  void enable(int wiimote_number, wiimote_capabilities capabilities_to_enable)
  {
      std::array<u8, 23> data = { 0xa2, 0x16, 0x04, 0xA6, 0x00, 0xFE, 0x01, 0x04 };
      WiimoteReal::InterruptChannel(wiimote_number, 65, &data[0], sizeof(data));

      std::array<u8, 10> reportingMode = { 0xa2, 0x12, 0x00, 0x35  };
      WiimoteReal::InterruptChannel(wiimote_number, 65, &reportingMode[0], sizeof(reportingMode));
  }

  void send_packet(int wiimote_number, const std::array<u8, 21>& data, size_t size, std::function<void(int)> callback)
  {
    messages.push(wiimote_message(wiimote_number, data, size, callback));
  }

  void send_packet(int wiimote_number, const std::array<u8, 21>& data, size_t size, steady_time_point future, std::function<void(int)> callback)
  {
    messages.push(wiimote_message(wiimote_number, future, data, size, callback));
  }

  std::vector<wiimote_message> get_expired_messages()
  {
    auto time = steady_time_point::clock::now();

    std::vector<wiimote_message> expired_messages;

    wiimote_message message;
    while(messages.try_pop(message))
    {
      if(time > message.send_time())
        expired_messages.push_back(message);
      else
      {
        messages.push(message);
        break;
      }
    }

    return expired_messages;
  }

  void send_messages()
  {
    auto expired_messages = get_expired_messages();

    for(auto& message : expired_messages)
    {
      WiimoteReal::InterruptChannel(message.wiimote(), 65, &message.message()[0], message.size());
      message.on_sent()(message.wiimote());
    }
  }

  void on_end_rumble(int wiimote_number)
  {
    current_wiimote_state[wiimote_number].end_brief_rumble();
  }

  void do_rumble(int wiimote_number)
  {
      dolphiimote::current_wiimote_state[wiimote_number].begin_brief_rumble();
      send_packet(wiimote_number, dolphiimote::serialization::start_rumble(), dolphiimote::serialization::rumble_size(), [](int wiimote_number) {
        auto time_to_end_vibration = dolphiimote::steady_time_point::clock::now() + std::chrono::milliseconds(brief_rumble_duration);
        send_packet(wiimote_number, dolphiimote::serialization::stop_rumble(), dolphiimote::serialization::rumble_size(), time_to_end_vibration, on_end_rumble);                         
      });      
  }

  class dolphiimote_wiimote_listener : public WiimoteReal::wiimote_listener
  {
  public:
    virtual void data_received(int wiimote_number, const u16 channel, const void* const data, const u32 size)
    {
      dolphiimote::response_received(wiimote_number, channel, data, size);
    }

    virtual void wiimote_connection_changed(int wiimote_number, bool connected)
    {

    }
  } listener;
}

int dolphiimote_init(update_callback_t _callback, void *userdata)
{
  dolphiimote::setup_data();

  dolphiimote::callback = _callback;
  dolphiimote::callback_userdata = userdata;

  WiimoteReal::listeners.add(&dolphiimote::listener);
  WiimoteReal::LoadSettings();
  WiimoteReal::Initialize();
  WiimoteReal::Refresh();

  auto num_wiimotes = WiimoteReal::NumFoundWiimotes();

  for(int wiimote = 0; wiimote < num_wiimotes; wiimote++)
  {
    dolphiimote::current_wiimote_state[wiimote] = dolphiimote::wiimote();
    dolphiimote::begin_determine_capabilities(wiimote);
    dolphiimote::enable(wiimote, dolphiimote::check_capabilities(wiimote));
  }

  return num_wiimotes;
}

void dolphiimote_brief_rumble(int wiimote_number)
{
  if(dolphiimote::current_wiimote_state.find(wiimote_number) != dolphiimote::current_wiimote_state.end() && !dolphiimote::current_wiimote_state[wiimote_number].rumble_active())
    dolphiimote::do_rumble(wiimote_number);
}

void dolphiimote_update()
{
  for(int i = 0; i < 4; i++)
  {
    dolphiimote::send_messages();
    WiimoteReal::Update(i);
  }
}