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

std::vector<std::string> test_messages;

void setUp() {
	test_messages.clear();
	fake_millis = 0;
}

/**
 * Read exception status.
 */
void read_exception_status() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_exception_status(11);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(4, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x0B, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[1]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x0B, 0x07, 0x6D, 0xC3, 0xDF });

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::SUCCESS, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->success());

	TEST_ASSERT_EQUAL_INT(0x6D, resp->data());
}

/**
 * Try to read from the broadcast device address.
 */
void read_exception_status_broadcast() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_exception_status(0);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_INVALID, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_FALSE(resp->success());
	TEST_ASSERT_TRUE(resp->failed());
}

/**
 * Try to read from a reserved device address.
 */
void read_exception_status_reserved_device() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_exception_status(248);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_INVALID, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_FALSE(resp->success());
	TEST_ASSERT_TRUE(resp->failed());
}

/**
 * Response has the wrong length.
 */
void read_exception_status_wrong_length_too_long() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_exception_status(11);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(4, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x0B, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[1]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x0B, 0x07, 0x6D, 0xFF, 0xDF, 0x11 });

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_LENGTH, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data());
}

/**
 * Response has the wrong length.
 */
void read_exception_status_wrong_length_too_short() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_exception_status(11);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(4, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x0B, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[1]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x0B, 0x07, 0x47, 0x42 });

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_LENGTH, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
	TEST_ASSERT_FALSE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->data());
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	RUN_TEST(read_exception_status);

	RUN_TEST(read_exception_status_broadcast);
	RUN_TEST(read_exception_status_reserved_device);

	RUN_TEST(read_exception_status_wrong_length_too_long);
	RUN_TEST(read_exception_status_wrong_length_too_short);

	return UNITY_END();
}
