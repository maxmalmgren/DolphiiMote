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
  typedef typesafe<std::string> log_level;

  void log(const log_level &level, const char* fmt, ...);
  void log(const log_level &level, const char* fmt, va_list args);

  const log_level Debug = log_level("Debug");
  const log_level Info = log_level("Info");
  const log_level Warning = log_level("Warning");
  const log_level Error = log_level("Error");

  class log_keeper
  {
  public:
    static log_keeper& instance();

    void set_output(std::function<void(std::string&)> callback);
    void log(const log_level &level, const char* fmt, va_list args);

  private:
    std::function<void(std::string&)> callback;
  };
}
#endif