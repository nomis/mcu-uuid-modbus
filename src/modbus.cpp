/*
 * uuid-modbus - Microcontroller asynchronous Modbus library
 * Copyright 2021-2022  Simon Arlott
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <uuid/modbus.h>

#include <Arduino.h>

#include <uuid/log.h>

namespace uuid {

namespace modbus {

static const char __pstr__loggername[] __attribute__((__aligned__(sizeof(int)))) PROGMEM = "modbus";
const uuid::log::Logger logger{reinterpret_cast<const __FlashStringHelper *>(__pstr__loggername), uuid::log::Facility::DAEMON};

} // namespace modbus

} // namespace uuid
