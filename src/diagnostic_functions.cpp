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

#include <make_unique.cpp>

namespace uuid {

namespace modbus {

std::shared_ptr<const ExceptionStatusResponse> SerialClient::read_exception_status(
		uint16_t device, uint8_t timeout_s) {
	auto response = std::make_shared<ExceptionStatusResponse>();

	if (device < DeviceAddressType::MIN_UNICAST
			|| device > DeviceAddressType::MAX_UNICAST) {
		response->status(ResponseStatus::FAILURE_INVALID);
	} else {
		requests_.push_back(std::make_unique<Request>(device,
			FunctionCode::READ_EXCEPTION_STATUS, timeout_s, response));
	}

	return response;
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
