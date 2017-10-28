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

#include "wiimote.h"

namespace dolphiimote
{
  const wiimote_extensions::type wiimote_extensions::None = 0x00;
  const wiimote_extensions::type wiimote_extensions::Nunchuck = 0x01;
  const wiimote_extensions::type wiimote_extensions::ClassicController = 0x02;
  const wiimote_extensions::type wiimote_extensions::ClassicControllerPro = 0x04;
  const wiimote_extensions::type wiimote_extensions::GHGuitar = 0x08;
  const wiimote_extensions::type wiimote_extensions::GHWorldTourDrums = 0x10;
  const wiimote_extensions::type wiimote_extensions::MotionPlus = 0x20;
  const wiimote_extensions::type wiimote_extensions::BalanceBoard = 0x30;
  const wiimote_extensions::type wiimote_extensions::Passthrough = 0x40;

  const wiimote_capabilities::type wiimote_capabilities::None = 0;
  const wiimote_capabilities::type wiimote_capabilities::MotionPlus = 0x2;
  const wiimote_capabilities::type wiimote_capabilities::Extension = 0x4;
  const wiimote_capabilities::type wiimote_capabilities::IR = 0x8;
}