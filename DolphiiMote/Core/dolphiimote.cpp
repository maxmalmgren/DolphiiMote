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

std::shared_ptr<dolphiimote::dolphiimote_host> host;

int dolphiimote_init(dolphiimote_callbacks _callback)
{
  host = std::shared_ptr<dolphiimote::dolphiimote_host>(new dolphiimote::dolphiimote_host(_callback));
  WiimoteReal::listeners.add(std::weak_ptr<WiimoteReal::wiimote_listener>(host));
  return host->number_of_wiimotes();
}

void dolphiimote_brief_rumble(uint8_t  wiimote_number)
{
  if(!host)
    return;

  host->do_brief_rumble(wiimote_number);
}
void dolphiimote_set_rumble(uint8_t  wiimote_number, uint8_t enable)
{
	if (!host)
		return;

	host->do_rumble(wiimote_number, enable);
}
void dolphiimote_play_sound_pcm(uint8_t  wiimote_number, char* file, uint8_t volume)
{
	if (!host)
		return;

	host->play_sound_pcm(wiimote_number, file, volume);
}
void dolphiimote_stop_sound(uint8_t  wiimote_number)
{
	if (!host)
		return;

	host->stop_sound(wiimote_number);
}
void dolphiimote_set_leds(uint8_t  wiimote_number, uint8_t leds)
{
	if (!host)
		return;

	host->set_leds(wiimote_number, leds);
}
void dolphiimote_request_status(uint8_t  wiimote_number)
{
	if (!host)
		return;

	host->request_status(wiimote_number);
}
void dolphiimote_enable_capabilities(uint8_t wiimote_number, dolphiimote_capabilities capabilities)
{
  if(!host)
    return;

  host->enable_capabilities(wiimote_number, (dolphiimote::wiimote_capabilities::type)capabilities);
}

void dolphiimote_set_reporting_mode(uint8_t wiimote_number, uint8_t mode)
{
  if(!host)
    return;

  host->request_reporting_mode(wiimote_number, mode);
}

void dolphiimote_shutdown()
{
	if(!host)
	  return;
	host->stop_all_sounds();
	host.reset();
  WiimoteReal::Shutdown();
}

void dolphiimote_log_level(uint8_t level)
{
  dolphiimote::log_keeper::instance().set_level(level);
}

void dolphiimote_update()
{
  //If sound is playing, there is not enough bandwith available to update, so updating will freeze threads, stopping us from stopping audio.
  if(!host || host->sound_playing())
	  return;

  for(int i = 0; i < 4; i++)
  {
    host->update();
    WiimoteReal::Update(i);
  }
}