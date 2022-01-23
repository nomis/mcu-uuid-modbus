/*
 * uuid-modbus - Microcontroller asynchronous Modbus library
 * Copyright 2022  Simon Arlott
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

bool Response::check_length(frame_buffer_t &frame, uint16_t actual, uint16_t expected) {
	if (actual != expected) {
		logger.err(F("Length mismatch for function %02X from device %u, expected %u received %u"),
			frame[1], frame[0], expected, actual);
		return false;
	} else {
		return true;
	}
}

ResponseStatus RegisterDataResponse::parse(frame_buffer_t &frame, uint16_t len) {
	if (len < 3) {
		logger.err(F("Incomplete message for function %02X from device %u, expected 3+ received %u"),
			frame[1], frame[0], len);
		return ResponseStatus::FAILURE_LENGTH;
	} else if (!check_length(frame, len, 3 + 2 * frame[2])) {
		return ResponseStatus::FAILURE_LENGTH;
	}

	for (uint16_t i = 0; i < frame[2]; i++) {
		data_.emplace_back((frame[3 + i * 2] << 8) | frame[4 + i * 2]);
	}

	return ResponseStatus::SUCCESS;
}

ResponseStatus RegisterWriteResponse::parse(frame_buffer_t &frame, uint16_t len) {
	if (!check_length(frame, len, 6)) {
		return ResponseStatus::FAILURE_LENGTH;
	}

	address_ = (frame[2] << 8) | frame[3];
	data_.emplace_back((frame[4] << 8) | frame[5]);

	return ResponseStatus::SUCCESS;
}

ResponseStatus ExceptionStatusResponse::parse(frame_buffer_t &frame, uint16_t len) {
	if (!check_length(frame, len, 3)) {
		return ResponseStatus::FAILURE_LENGTH;
	}

	data_ = frame[2];

	return ResponseStatus::SUCCESS;
}

} // namespace modbus

} // namespace uuid
