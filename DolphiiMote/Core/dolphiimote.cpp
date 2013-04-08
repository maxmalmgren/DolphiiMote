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
extern "C"
{
  #include "dolphiimote.h"
}
#include "dolphiimote_host.h"

std::unique_ptr<dolphiimote::dolphiimote_host> host;

int dolphiimote_init(dolphiimote_callbacks _callback, void *userdata)
{
  host = std::unique_ptr<dolphiimote::dolphiimote_host>(new dolphiimote::dolphiimote_host(_callback));
  return host->number_of_wiimotes();
}

void dolphiimote_brief_rumble(unsigned int  wiimote_number)
{
  host->do_rumble(wiimote_number);
}

void dolphiimote_enable_capabilities(unsigned int wiimote_number, dolphiimote_capabilities capabilities)
{
  host->enable_capabilities(wiimote_number, (dolphiimote::wiimote_capabilities::type)capabilities);
}

void dolphiimote_determine_capabilities(unsigned int wiimote_number)
{
  host->determine_capabilities(wiimote_number);
}

void dolphiimote_set_reporting_mode(unsigned int wiimote_number, uint8_t mode)
{
  host->request_reporting_mode(wiimote_number, mode);
}

void dolphiimote_update()
{
  for(int i = 0; i < 4; i++)
  {
    host->update();
    WiimoteReal::Update(i);
  }
}