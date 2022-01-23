/*
 * uuid-modbus - Microcontroller Modbus library
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

#include <Arduino.h>
#include <unity.h>

#include <uuid/modbus.h>

static unsigned long fake_millis = 0;

unsigned long millis() {
	return fake_millis;
}

namespace uuid {

uint64_t get_uptime_ms() {
	static uint64_t millis = 0;
	return ++millis;
}

} // namespace uuid

void setUp() {
	fake_millis = 0;
}

/**
 * A response with only the device address is too short.
 */
void short_response_1() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07 });

	// Check that nothing happens until there is a timeout
	for (int i = 0; i < 100; i++) {
		client.loop();
		TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
		TEST_ASSERT_TRUE(resp->pending());
		TEST_ASSERT_FALSE(resp->done());
	}

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_TOO_SHORT, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * A response with only the device address and function code is too short.
 */
void short_response_2() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04 });

	// Check that nothing happens until there is a timeout
	for (int i = 0; i < 100; i++) {
		client.loop();
		TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
		TEST_ASSERT_TRUE(resp->pending());
		TEST_ASSERT_FALSE(resp->done());
	}

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_TOO_SHORT, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * A response with half the CRC is too short.
 */
void short_response_3() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0xFF });

	// Check that nothing happens until there is a timeout
	for (int i = 0; i < 100; i++) {
		client.loop();
		TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
		TEST_ASSERT_TRUE(resp->pending());
		TEST_ASSERT_FALSE(resp->done());
	}

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_TOO_SHORT, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * A response of 257 bytes is too long (by 1 byte).
 */
void long_response_257() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {0x07, 0x04, 0x7D});
	for (uint8_t i = 1; i <= 126; i++) {
		device.tx_.insert(device.tx_.end(), {0x00, i});
	}
	device.tx_.insert(device.tx_.end(), {0xFF, 0xFF});
	TEST_ASSERT_EQUAL_INT(257, device.tx_.size());

	// Check that nothing happens until there is a timeout
	for (int i = 0; i < 100; i++) {
		client.loop();
		TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
		TEST_ASSERT_TRUE(resp->pending());
		TEST_ASSERT_FALSE(resp->done());
	}

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_TOO_LONG, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * A response of 258 bytes is too long (by 2 bytes).
 */
void long_response_258() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {0x07, 0x04, 0x7D});
	for (uint8_t i = 1; i <= 126; i++) {
		device.tx_.insert(device.tx_.end(), {0x00, i});
	}
	device.tx_.insert(device.tx_.end(), {0x00});
	device.tx_.insert(device.tx_.end(), {0xFF, 0xFF});
	TEST_ASSERT_EQUAL_INT(258, device.tx_.size());

	// Check that nothing happens until there is a timeout
	for (int i = 0; i < 100; i++) {
		client.loop();
		TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
		TEST_ASSERT_TRUE(resp->pending());
		TEST_ASSERT_FALSE(resp->done());
	}

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_TOO_LONG, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * A response of 1000 bytes is too long (by 744 bytes).
 */
void long_response_1000() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {0x07, 0x04, 0x7D});
	for (int i = 1; i <= 1000 - 5; i++) {
		device.tx_.insert(device.tx_.end(), {(uint8_t)i});
	}
	device.tx_.insert(device.tx_.end(), {0xFF, 0xFF});
	TEST_ASSERT_EQUAL_INT(1000, device.tx_.size());

	// Check that nothing happens until there is a timeout
	for (int i = 0; i < 100; i++) {
		client.loop();
		TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
		TEST_ASSERT_TRUE(resp->pending());
		TEST_ASSERT_FALSE(resp->done());
	}

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_TOO_LONG, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * A response with the wrong CRC.
 */
void invalid_crc() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0x00, 0xFF, 0xFF });

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_CRC, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * A response from the wrong device.
 */
void wrong_device_address() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x08, 0x04, 0x00, 0xF2, 0xC2 });

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_ADDRESS, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * A response with the wrong function code.
 */
void wrong_function_code() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x05, 0x00, 0xC3, 0x51 });

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_FUNCTION, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	RUN_TEST(short_response_1);
	RUN_TEST(short_response_2);
	RUN_TEST(short_response_3);

	RUN_TEST(long_response_257);
	RUN_TEST(long_response_258);
	RUN_TEST(long_response_1000);

	RUN_TEST(invalid_crc);

	RUN_TEST(wrong_device_address);
	RUN_TEST(wrong_function_code);

	return UNITY_END();
}
