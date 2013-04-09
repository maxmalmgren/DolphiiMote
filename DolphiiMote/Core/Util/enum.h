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

#ifndef DOLPHIIMOTE_ENUM_H
#define DOLPHIIMOTE_ENUM_H

template <typename T, T def_value = 0>
class enumeration
{
public:
  typedef T enum_type;

  enumeration() : value(def_value)
  { }

  enumeration(enum_type value) : value(value)
  { }

  enumeration<T, def_value>& operator=(const enumeration<T, def_value>& other)
  {
    value = other.value;
    return *this;
  }

  enumeration<T, def_value>& operator=(enum_type other)
  {
    value = other;
    return *this;
  }

  operator enum_type()
  {
    return value;
  }

  enum_type val()
  {
    return value;
  }

private:
  enum_type value;  
};

  template <typename T, T def_value>
  inline void operator|=(enumeration<T, def_value> &a, enumeration<T, def_value> b)
  {
    a = (a.val() | b.val());
  }

  template <typename T>
  inline bool is_set(T type, T comparand)
  {
    return (type & comparand) == comparand;
  }

  template <typename T, T def_value>
  inline bool operator==(enumeration<T, def_value> a, enumeration<T, def_value> b)
  {
    return a.val() == b.val();
  }

  template <typename T, T def_value>
  inline enumeration<T, def_value> operator|(enumeration<T, def_value> a, enumeration<T, def_value> b)
  {
    return a.val() | b.val();
  }

  template <typename T, T def_value>
  inline bool operator<(enumeration<T, def_value> a, enumeration<T, def_value> b)
  {
    return a.val() < b.val();
  }

  template <typename T, T def_value>
  inline enumeration<T, def_value> operator~(enumeration<T, def_value> a)
  {
    return ~a.val();
  }

  template <typename T, T def_value>
  inline void operator&=(enumeration<T, def_value> &a, enumeration<T, def_value> b)
  {
    a = (a & b);
  }

  template <typename T, T def_value>
  inline enumeration<T, def_value> operator&(enumeration<T, def_value> a, enumeration<T, def_value> b)
  {
    return a.val() & b.val();
  }

#endif