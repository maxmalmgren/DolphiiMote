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

#include "log.h"
#include <stdarg.h>
#include <chrono>
#include <time.h>

namespace dolphiimote
{
  void log(const log_level &level, const char* fmt, ...)
  {
    va_list args;
	  va_start(args, fmt);
    log_keeper::instance().log(level, fmt, args);
	  va_end(args);
  }

  void log(const log_level &level, const char* fmt, va_list args)
  {
    log_keeper::instance().log(level, fmt, args);
  }


  log_keeper _instance;

  log_keeper& log_keeper::instance()
  {
    return _instance;
  }
  
  void log_keeper::set_level(uint8_t level)
  {
    this->level = level;
  }

  void log_keeper::set_output(std::function<void(std::string&)> callback)
  {
    this->callback = callback;
  }

  size_t fill_time(char *buffer, size_t size)
  {
    time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tm time_struct; 
    localtime_s(&time_struct, &time);
    return strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &time_struct);
  }

  void log_keeper::log(const log_level &level, const char* fmt, va_list args)
  {
    if(level->first < this->level)
      return;

    char buffer[500];

    char* index = buffer;

    std::memcpy(index, level->second.c_str(), level->second.size());

    index += level->second.size();

    *index++ = '\t';

    index += fill_time(index, sizeof(buffer));    

    *index++ = '\t';
    *index++ = ' ';

    index += vsnprintf_s(index, sizeof(buffer) + buffer - index, sizeof(buffer) + buffer - index, fmt, args);

    *index++ = '\n';
    *index++ = '\0';

    if(callback)
      callback(std::string(buffer));
  }
}