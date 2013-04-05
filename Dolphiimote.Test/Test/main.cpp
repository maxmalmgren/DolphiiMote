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

#include <iostream>
#include "../../Dolphiimote/Core/dolphiimote.h"
#include <array>
#include <thread>
#include <sstream>
#include <iomanip>
#include <iostream>

template <typename T>
std::string to_hex(T arg)
{
  std::stringstream sstream;
  sstream << std::hex << std::uppercase << std::setw(2) << std::setfill('0')  << arg;
  return sstream.str();
}

void on_data_received(unsigned int wiimote_number, struct dolphiimote_wiimote_data *data, void *userdata)
{
  std::cout << " wiimote " << wiimote_number << ": ";

  if(data->button_state & dolphiimote_BUTTON_A)
  {
    std::cout << "A ";
    dolphiimote_brief_rumble(wiimote_number);
  }

  if(data->button_state & dolphiimote_BUTTON_B)
    std::cout << "B ";
  if(data -> button_state & dolphiimote_BUTTON_DPAD_DOWN)
    std::cout << "Down ";
  if(data -> button_state & dolphiimote_BUTTON_DPAD_RIGHT)
    std::cout << "Right ";
  if(data -> button_state & dolphiimote_BUTTON_DPAD_LEFT)
    std::cout << "Left ";
  if(data -> button_state & dolphiimote_BUTTON_DPAD_UP)
    std::cout << "Up ";
  if(data -> button_state & dolphiimote_BUTTON_MINUS)
    std::cout << "Minus ";
  if(data -> button_state & dolphiimote_BUTTON_PLUS)
    std::cout << "Plus ";
  if(data -> button_state & dolphiimote_BUTTON_HOME)
    std::cout << "Home ";
  if(data -> button_state & dolphiimote_BUTTON_ONE)
    std::cout << "One ";
  if(data -> button_state & dolphiimote_BUTTON_TWO)
    std::cout << "Two ";

  //if(data->valid_data_flags & dolphiimote_ACCELERATION_VALID)
  //{
  //  std::cout << to_hex((int)data->acceleration.x);
  //  std::cout << to_hex((int)data->acceleration.y);
  //  std::cout << to_hex((int)data->acceleration.z) << std::endl;
  //}

  if(data->valid_data_flags & dolphiimote_MOTIONPLUS_VALID)
  {
    std::cout << to_hex((int)data->motionplus.yaw_down_speed);
    std::cout << to_hex((int)data->motionplus.yaw_down_speed);
    std::cout << to_hex((int)data->motionplus.yaw_down_speed) << std::endl;
  }
}

void on_capabilities_changed(unsigned int wiimote, dolphiimote_capability_status *status, void *userdata)
{
  std::cout << " wiimote " << wiimote << " capabilities:" << std::endl;
  std::cout << "Extension: ";

  switch(status->extension_type)
  {
    case dolphiimote_EXTENSION_NONE:
      std::cout << "None";
      break;
    case dolphiimote_EXTENSION_NUNCHUCK:
      std::cout << "Nunchuck";
      break;
    case dolphiimote_EXTENSION_CLASSIC_CONTROLLER:
      std::cout << "Classic Controller";
      break;
    case dolphiimote_EXTENSION_CLASSIC_CONTROLLER_PRO:
      std::cout << "Classic Controller Pro";
      break;
    case dolphiimote_EXTENSION_GUITAR_HERO_GUITAR:
      std::cout << "Guitar";
      break;
    case dolphiimote_EXTENSION_GUITAR_HERO_WORLD_TOUR_DRUMS:
      std::cout << "Drums";
      break;
  }

  std::cout << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  dolphiimote_set_reporting_mode(wiimote, 0x35);  
}

int main()
{
  dolphiimote_callbacks callbacks = { 0 };
  callbacks.data_received = on_data_received;
  callbacks.capabilities_changed = on_capabilities_changed;

  int wiimote_flags = dolphiimote_init(callbacks, NULL);

  for(int i = 0; i < MAX_WIIMOTES; i++, wiimote_flags >>= 1)
  {
    if(wiimote_flags & 0x01)
      dolphiimote_determine_capabilities(i);
  }

  while(true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    dolphiimote_update();
  }
}