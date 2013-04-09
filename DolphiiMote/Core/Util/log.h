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

#ifndef DOLPHIIMOTE_LOG_H
#define DOLPHIIMOTE_LOG_H

#include <string>
#include <iostream>
#include <functional>
#include <stdint.h>
#include "typesafe.h"

namespace dolphiimote
{
  typedef typesafe<std::pair<uint8_t, std::string>> log_level;

  void log(const log_level &level, const char* fmt, ...);
  void log(const log_level &level, const char* fmt, va_list args);

  const log_level Debug = log_level(std::make_pair(0, "Debug"));
  const log_level Info = log_level(std::make_pair(1, "Info"));
  const log_level Warning = log_level(std::make_pair(2, "Warning"));
  const log_level Error = log_level(std::make_pair(3, "Error"));

  class log_keeper
  {
  public:
    static log_keeper& instance();

    void set_output(std::function<void(std::string&)> callback);
    void set_level(uint8_t level);
    void log(const log_level &level, const char* fmt, va_list args);

  private:
    uint8_t level;
    std::function<void(std::string&)> callback;
  };
}
#endif