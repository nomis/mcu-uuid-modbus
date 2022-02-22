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

namespace uuid {

namespace modbus {

SerialClient::SerialClient(::HardwareSerial &serial) : serial_(serial) {
}

void SerialClient::loop() {
	if (requests_.empty() || idle_frame_) {
		idle();
		return;
	}

	auto &response = requests_.front()->response();

	if (response.status() == ResponseStatus::QUEUED) {
		idle();

		if (idle_frame_) {
			return;
		}

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

void SerialClient::idle() {
	uint32_t now_ms = input();

	if (frame_pos_ == 0) {
		return;
	} else {
		idle_frame_ = true;
	}

	if (now_ms - last_rx_ms_ >= INTER_FRAME_TIMEOUT_MS) {
		log_frame(F("<-"));
		logger.err(F("Received unexpected frame while idle from device %u"), frame_[0]);
		frame_pos_ = 0;
		idle_frame_ = false;
	}
}

void SerialClient::encode() {
	auto &request = *requests_.front().get();
	auto &response = request.response();

	frame_pos_ = request.encode(frame_);

	if (frame_pos_ > MAX_MESSAGE_SIZE - MESSAGE_CRC_SIZE) {
		response.status(ResponseStatus::FAILURE_INVALID);
		return;
	}

	uint16_t crc = calc_crc();
	frame_[frame_pos_++] = crc & 0xFF;
	frame_[frame_pos_++] = crc >> 8;

	tx_frame_size_ = frame_pos_;
	response.status(ResponseStatus::TRANSMIT);

	log_frame(F("->"));
	frame_pos_ = 0;
}

void SerialClient::transmit() {
	while (frame_pos_ < tx_frame_size_) {
		int available = serial_.availableForWrite();

		if (available <= 0) {
			return;
		}

		int len = std::min(available, tx_frame_size_ - frame_pos_);

		serial_.write(&frame_[frame_pos_], len);
		frame_pos_ += len;

		last_tx_ms_ = ::millis();
	}

	frame_pos_ = 0;
	requests_.front()->response().status(ResponseStatus::WAITING);
}

uint32_t SerialClient::input() {
	uint32_t now_ms = ::millis();
	int data = 0;

	do {
		int available = serial_.available();

		if (available <= 0) {
			break;
		}

		while (available-- > 0) {
			data = serial_.read();

			if (data == -1) {
				break;
			}

			if (frame_pos_ < frame_.size()) {
				frame_[frame_pos_++] = data;
			}

			now_ms = ::millis();
			last_rx_ms_ = now_ms;
		}
	} while (data != -1);

	return now_ms;
}

void SerialClient::receive() {
	uint32_t now_ms = input();

	if (frame_pos_ == 0) {
		auto &request = *requests_.front().get();

		if ((now_ms - last_tx_ms_) >= request.timeout_ms()) {
			if (request.device() == DeviceAddressType::BROADCAST) {
				request.response().status(ResponseStatus::SUCCESS);
			} else {
				request.response().status(ResponseStatus::FAILURE_TIMEOUT);
				logger.notice(F("Timeout waiting for response to function %02X from device %u"),
					frame_[1], frame_[0]);
			}
		}
	} else if (now_ms - last_rx_ms_ >= INTER_FRAME_TIMEOUT_MS) {
		complete();
		frame_pos_ = 0;
	}
}

void SerialClient::complete() {
	auto &request = *requests_.front().get();
	auto &response = request.response();

	log_frame(F("<-"));

	if (frame_pos_ < MESSAGE_HEADER_SIZE + MESSAGE_CRC_SIZE) {
		response.status(ResponseStatus::FAILURE_TOO_SHORT);
		logger.err(F("Received short frame from device %u"), frame_[0]);
		return;
	}

	if (frame_pos_ > MAX_MESSAGE_SIZE) {
		response.status(ResponseStatus::FAILURE_TOO_LONG);
		logger.err(F("Received oversized frame from device %u"), frame_[0]);
		return;
	}

	uint16_t act_crc = (frame_[frame_pos_ - 1] << 8) | frame_[frame_pos_ - 2];
	frame_pos_ -= MESSAGE_CRC_SIZE;
	uint16_t exp_crc = calc_crc();

	if (exp_crc != act_crc) {
		response.status(ResponseStatus::FAILURE_CRC);
		logger.err(F("Received frame with invalid CRC %04X from device %u with function %02X, expected %04X"),
			act_crc, frame_[0], frame_[1], exp_crc);
		return;
	}

	if (request.device() == DeviceAddressType::BROADCAST) {
		response.status(ResponseStatus::FAILURE_UNEXPECTED);
		logger.err(F("Received unexpected broadcast response with function code %02X from device %u"),
			frame_[1], frame_[0]);
		return;
	}

	if (frame_[0] != request.device()) {
		response.status(ResponseStatus::FAILURE_ADDRESS);
		logger.err(F("Received function %02X from device %u, expected device %u"),
			frame_[1], frame_[0], request.device());
		return;
	}

	if ((frame_[1] & ~0x80) != request.function_code()) {
		response.status(ResponseStatus::FAILURE_FUNCTION);
		logger.err(F("Received function %02X from device %u, expected function %02X"),
			frame_[1], frame_[0], request.function_code());
		return;
	}

	if (frame_[1] & 0x80) {
		if (frame_pos_ < 3) {
			response.status(ResponseStatus::FAILURE_LENGTH);
			logger.err(F("Exception with no code for function %02X from device %u"),
				frame_[1] & ~0x80, frame_[0]);
		} else {
			response.status(ResponseStatus::EXCEPTION);
			response.exception_code(frame_[2]);
			logger.notice(F("Exception code %02X for function %02X from device %u"),
				response.exception_code(), frame_[1] & ~0x80, frame_[0]);
		}
		return;
	}

	response.status(response.parse(frame_, frame_pos_));
}

void SerialClient::log_frame(const __FlashStringHelper *prefix) {
	if (logger.enabled(uuid::log::Level::TRACE)) {
		static constexpr uint8_t BYTES_PER_LINE = 16;
		static constexpr uint8_t CHARS_PER_BYTE = 3;
		std::vector<char> message(CHARS_PER_BYTE * BYTES_PER_LINE + 1);
		uint8_t pos = 0;

		for (uint16_t i = 0; i < frame_pos_; i++) {
			snprintf_P(&message[CHARS_PER_BYTE * pos++], CHARS_PER_BYTE + 1,
				PSTR("%c%02X"),
				(i == MESSAGE_HEADER_SIZE || i == frame_pos_ - MESSAGE_CRC_SIZE)
					? '\'' : ' ',
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

} // namespace modbus

} // namespace uuid
