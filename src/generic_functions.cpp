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

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <memory>

namespace uuid {

namespace modbus {

Request::Request(uint16_t device, uint8_t function_code, uint16_t timeout_ms,
		const std::shared_ptr<Response> &response)
		: device_(device), function_code_(function_code), timeout_ms_(timeout_ms),
		response_(response) {
}

uint16_t Request::encode(frame_buffer_t &frame) {
	frame[0] = device();
	frame[1] = function_code();
	return 2;
}

bool Response::check_length(frame_buffer_t &frame, uint16_t actual, uint16_t expected) {
	if (actual != expected) {
		logger.err(F("Length mismatch for function %02X from device %u, expected %u received %u"),
			frame[1], frame[0], expected, actual);
		return false;
	} else {
		return true;
	}
}

} // namespace modbus

} // namespace uuid
