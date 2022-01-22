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

#include <uuid/log.h>

#ifndef __cpp_lib_make_unique
namespace std {

template<typename _Tp, typename... _Args>
inline unique_ptr<_Tp> make_unique(_Args&&... __args) {
		return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
}

} // namespace std
#endif

namespace uuid {

namespace modbus {

SerialClient::SerialClient(::HardwareSerial *serial) : serial_(serial) {
}

void SerialClient::loop() {
	if (requests_.empty()) {
		return;
	}

	auto &response = *requests_.front()->response_.get();

	if (response.status() == ResponseStatus::QUEUED) {
		encode();
	}

	if (response.status() == ResponseStatus::TRANSMIT) {
		transmit();
	}

	if (response.status() == ResponseStatus::WAITING) {
		receive();
	}

	if (response.done()) {
		requests_.pop_front();
	}
}

void SerialClient::encode() {
	auto &request = *requests_.front().get();
	auto &response = *request.response_.get();

	frame_pos_ = request.encode(frame_);

	if (frame_pos_ + 2 > MAX_MESSAGE_SIZE) {
		response.status_ = ResponseStatus::FAILURE_INVALID;
		return;
	}

	uint16_t crc = calc_crc();
	frame_[frame_pos_++] = crc & 0xFF;
	frame_[frame_pos_++] = crc >> 8;

	tx_frame_size_ = frame_pos_;
	response.status_ = ResponseStatus::TRANSMIT;

	log_frame(F("->"));
	frame_pos_ = 0;
}

void SerialClient::transmit() {
	while (frame_pos_ < tx_frame_size_) {
		int available = serial_->availableForWrite();

		if (!available) {
			return;
		}

		int len = std::min(available, tx_frame_size_ - frame_pos_);

		serial_->write(&frame_[frame_pos_], len);
		frame_pos_ += len;

		last_tx_ms_ = ::millis();
	}

	frame_pos_ = 0;
	rx_frame_size_ = 0;
	requests_.front()->response_->status_ = ResponseStatus::WAITING;
}

void SerialClient::receive() {
	auto &request = *requests_.front().get();
	auto &response = *request.response_.get();
	uint32_t now_ms = millis();

	if (frame_pos_ == 0) {
		if ((now_ms - last_tx_ms_) >= request.timeout_s_ * 1000) {
			response.status_ = ResponseStatus::FAILURE_TIMEOUT;
			logger.notice(F("Timeout waiting for response to function %02X from device %u"),
				frame_[1], frame_[0]);
			return;
		}
	} else if (now_ms - last_rx_ms_ >= INTER_FRAME_TIMEOUT_MS) {
		if (!response.done()) {
			complete();
		}
		return;
	}

	while (1) {
		int available = serial_->available();

		if (!available) {
			break;
		}

		while (available-- > 0) {
			int data = serial_->read();

			if (data == -1) {
				return;
			}

			if (frame_pos_ < frame_.size()) {
				frame_[frame_pos_++] = data;
			}
		}
	}
}

void SerialClient::complete() {
	auto &request = *requests_.front().get();
	auto &response = *request.response_.get();

	log_frame(F("<-"));

	if (frame_pos_ < 4) {
		response.status_ = ResponseStatus::FAILURE_TOO_SHORT;
		logger.err(F("Received short frame from device %u"), frame_[0]);
		return;
	}

	if (frame_pos_ > MAX_MESSAGE_SIZE) {
		response.status_ = ResponseStatus::FAILURE_TOO_LONG;
		logger.err(F("Received oversized frame from device %u"), frame_[0]);
		return;
	}

	uint16_t act_crc = (frame_[frame_pos_ - 1] << 8) | frame_[frame_pos_ - 2];
	frame_pos_ -= 2;
	uint16_t exp_crc = calc_crc();

	if (exp_crc != act_crc) {
		response.status_ = ResponseStatus::FAILURE_CRC;
		logger.err(F("Received frame with invalid CRC %04X from device %u with function %02X, expected %04X"),
			act_crc, frame_[0], frame_[1], exp_crc);
		return;
	}

	if (frame_[0] != request.device_) {
		response.status_ = ResponseStatus::FAILURE_ADDRESS;
		logger.err(F("Received function %02X from device %u, expected device %u"),
			frame_[1], frame_[0], request.device_);
		return;
	}

	if ((frame_[1] & ~0x80) != request.function_code_) {
		response.status_ = ResponseStatus::FAILURE_FUNCTION;
		logger.err(F("Received function %02X from device %u, expected function %02X"),
			frame_[1], frame_[0], request.function_code_);
		return;
	}

	if (frame_[1] & 0x80) {
		response.status_ = ResponseStatus::EXCEPTION;
		if (frame_pos_ < 3) {
			response.exception_code_ = 0;
		} else {
			response.exception_code_ = frame_[2];
		}
		logger.notice(F("Exception code %02X for function %02X from device %u"),
			response.exception_code_, frame_[1] & ~0x80, frame_[0]);
		return;
	}

	response.status_ = response.parse(frame_, frame_pos_);
}

void SerialClient::log_frame(const __FlashStringHelper *prefix) {
	if (logger.enabled(uuid::log::Level::TRACE)) {
		static constexpr uint8_t BYTES_PER_LINE = 16;
		std::vector<char> message(3 * BYTES_PER_LINE + 1);
		uint8_t pos = 0;

		for (uint16_t i = 0; i < frame_pos_; i++) {
			snprintf_P(&message[3 * pos++], 4, PSTR("%c%02X"),
				(i == 2 || i == frame_pos_ - 2) ? '\'' : ' ',
				frame_[i]);

			if (pos == BYTES_PER_LINE || i == frame_pos_ - 1) {
				logger.trace(F("%S%s"), prefix, message.data());
				pos = 0;
				prefix = F("  ");
			}
		}
	}
}

uint16_t SerialClient::calc_crc() const {
	uint16_t crc = 0xFFFF;

	for (uint16_t i = 0; i < frame_pos_; i++) {
		crc = crc ^ frame_[i];

		for (uint8_t b = 0; b < 8; b++) {
			if (crc & 0x0001) {
				crc >>= 1;
				crc ^= 0xA001;
			} else {
				crc >>= 1;
			}
		}
	}

	return crc;
}

std::shared_ptr<RegisterDataResponse> SerialClient::read_holding_registers(
		uint16_t device, uint16_t address, uint16_t size, uint8_t timeout_s) {
	auto response = std::make_shared<RegisterDataResponse>();

	if (device < DeviceAddressType::MIN_UNICAST
			|| device > DeviceAddressType::MAX_UNICAST
			|| size > 0x007D) {
		response->status_ = ResponseStatus::FAILURE_INVALID;
	} else {
		requests_.push_back(std::make_unique<RegisterRequest>(device,
			FunctionCode::READ_HOLDING_REGISTERS, timeout_s, address, size,
			response));
	}

	return response;
}

std::shared_ptr<RegisterDataResponse> SerialClient::read_input_registers(
		uint16_t device, uint16_t address, uint16_t size, uint8_t timeout_s) {
	auto response = std::make_shared<RegisterDataResponse>();

	if (device < DeviceAddressType::MIN_UNICAST
			|| device > DeviceAddressType::MAX_UNICAST
			|| size > 0x007D) {
		response->status_ = ResponseStatus::FAILURE_INVALID;
	} else {
		requests_.push_back(std::make_unique<RegisterRequest>(device,
			FunctionCode::READ_INPUT_REGISTERS, timeout_s, address, size,
			response));
	}

	return response;
}

std::shared_ptr<RegisterWriteResponse> SerialClient::write_holding_register(
		uint16_t device, uint16_t address, uint16_t value, uint8_t timeout_s) {
	auto response = std::make_shared<RegisterWriteResponse>();

	if (device > DeviceAddressType::MAX_UNICAST) {
		response->status_ = ResponseStatus::FAILURE_INVALID;
	} else {
		requests_.push_back(std::make_unique<RegisterRequest>(device,
			FunctionCode::WRITE_SINGLE_REGISTER, timeout_s, address, value,
			response));
	}

	return response;
}

std::shared_ptr<ExceptionStatusResponse> SerialClient::read_exception_status(
		uint16_t device, uint8_t timeout_s) {
	auto response = std::make_shared<ExceptionStatusResponse>();

	if (device < DeviceAddressType::MIN_UNICAST
			|| device > DeviceAddressType::MAX_UNICAST) {
		response->status_ = ResponseStatus::FAILURE_INVALID;
	} else {
		requests_.push_back(std::make_unique<Request>(device,
			FunctionCode::READ_EXCEPTION_STATUS, timeout_s, response));
	}

	return response;
}

} // namespace modbus

} // namespace uuid
