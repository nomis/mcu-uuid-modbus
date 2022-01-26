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

#include <make_unique.cpp>

namespace uuid {

namespace modbus {

std::shared_ptr<const RegisterDataResponse> SerialClient::read_holding_registers(
		uint16_t device, uint16_t address, uint16_t size, uint32_t timeout_ms) {
	auto response = std::make_shared<RegisterDataResponse>();

	if (device < DeviceAddressType::MIN_UNICAST
			|| device > DeviceAddressType::MAX_UNICAST
			|| size < 1 || size > 0x007D) {
		response->status(ResponseStatus::FAILURE_INVALID);
	} else {
		requests_.push_back(std::make_unique<RegisterRequest>(device,
			FunctionCode::READ_HOLDING_REGISTERS, timeout_ms, address, size,
			response));
	}

	return response;
}

std::shared_ptr<const RegisterDataResponse> SerialClient::read_input_registers(
		uint16_t device, uint16_t address, uint16_t size, uint32_t timeout_ms) {
	auto response = std::make_shared<RegisterDataResponse>();

	if (device < DeviceAddressType::MIN_UNICAST
			|| device > DeviceAddressType::MAX_UNICAST
			|| size < 1 || size > 0x007D) {
		response->status(ResponseStatus::FAILURE_INVALID);
	} else {
		requests_.push_back(std::make_unique<RegisterRequest>(device,
			FunctionCode::READ_INPUT_REGISTERS, timeout_ms, address, size,
			response));
	}

	return response;
}

std::shared_ptr<const RegisterWriteResponse> SerialClient::write_holding_register(
		uint16_t device, uint16_t address, uint16_t value, uint32_t timeout_ms) {
	auto response = std::make_shared<RegisterWriteResponse>();

	if (device > DeviceAddressType::MAX_UNICAST) {
		response->status(ResponseStatus::FAILURE_INVALID);
	} else {
		requests_.push_back(std::make_unique<RegisterRequest>(device,
			FunctionCode::WRITE_SINGLE_REGISTER, timeout_ms, address, value,
			response));
	}

	return response;
}

RegisterRequest::RegisterRequest(uint16_t device, uint8_t function_code,
		uint32_t timeout_ms, uint16_t address, uint16_t data,
		const std::shared_ptr<Response> &response)
		: Request(device, function_code, timeout_ms, response),
		address_(address), data_(data) {
}

uint16_t RegisterRequest::encode(frame_buffer_t &frame) {
	frame[0] = device();
	frame[1] = function_code();
	frame[2] = address() >> 8;
	frame[3] = address() & 0xFF;
	frame[4] = data() >> 8;
	frame[5] = data() & 0xFF;
	return 6;
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

} // namespace modbus

} // namespace uuid
