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
#include <uuid/log.h>

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
 * No messages while idle.
 */
void nothing_at_idle() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	client.loop();

	fake_millis += 1;
	client.loop();

	fake_millis += 1;
	client.loop();

	fake_millis += 1;
	client.loop();

	fake_millis += 1;
	client.loop();

	TEST_ASSERT_EQUAL_INT(0, device.rx_.size());
	TEST_ASSERT_EQUAL_INT(0, test_messages.size());
}

/**
 * Read message while idle.
 */
void message_at_idle_1() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0x01, 0x56, 0x78, 0xFE, 0xB2 });

	client.loop();
	TEST_ASSERT_EQUAL_INT(0, device.tx_.size());

	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	client.loop();

	TEST_ASSERT_EQUAL_INT(2, test_messages.size());
	TEST_ASSERT_EQUAL_STRING("<- 07 04'01 56 78'FE B2", test_messages[0].c_str());
	TEST_ASSERT_EQUAL_STRING("Received unexpected frame while idle from device 7",
		test_messages[1].c_str());
}

/**
 * Read messages while idle.
 */
void message_at_idle_2() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0x01, 0x56, 0x78, 0xFE, 0xB2 });

	client.loop();
	TEST_ASSERT_EQUAL_INT(0, device.tx_.size());

	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	client.loop();

	device.tx_.insert(device.tx_.end(), {
		0x08, 0x04, 0x01, 0x56, 0x78, 0xFE, 0xB2 });

	client.loop();
	TEST_ASSERT_EQUAL_INT(0, device.tx_.size());

	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	client.loop();

	TEST_ASSERT_EQUAL_INT(4, test_messages.size());
	TEST_ASSERT_EQUAL_STRING("<- 07 04'01 56 78'FE B2", test_messages[0].c_str());
	TEST_ASSERT_EQUAL_STRING("Received unexpected frame while idle from device 7",
		test_messages[1].c_str());
	TEST_ASSERT_EQUAL_STRING("<- 08 04'01 56 78'FE B2", test_messages[2].c_str());
	TEST_ASSERT_EQUAL_STRING("Received unexpected frame while idle from device 8",
		test_messages[3].c_str());
}

/**
 * Read message while idle in parts.
 */
void message_at_idle_parts() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	device.tx_.insert(device.tx_.end(), { 0x07, 0x04, 0x01});

	client.loop();
	TEST_ASSERT_EQUAL_INT(0, device.tx_.size());

	device.tx_.insert(device.tx_.end(), { 0x56, 0x78, 0xFE, 0xB2 });

	fake_millis += 1;
	client.loop();
	TEST_ASSERT_EQUAL_INT(0, device.tx_.size());

	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	client.loop();

	TEST_ASSERT_EQUAL_INT(2, test_messages.size());
	TEST_ASSERT_EQUAL_STRING("<- 07 04'01 56 78'FE B2", test_messages[0].c_str());
	TEST_ASSERT_EQUAL_STRING("Received unexpected frame while idle from device 7",
		test_messages[1].c_str());
}

/**
 * Queue a message before reading a message at idle.
 */
void queue_request_before_message_at_idle() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	device.tx_.insert(device.tx_.end(), { 0x07, 0x04, 0x01});

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(0, device.tx_.size());

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());
	TEST_ASSERT_EQUAL_INT(0, device.rx_.size());

	device.tx_.insert(device.tx_.end(), { 0x56, 0x78, 0xFE, 0xB2 });

	fake_millis += 1;
	client.loop();
	TEST_ASSERT_EQUAL_INT(0, device.tx_.size());

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());
	TEST_ASSERT_EQUAL_INT(0, device.rx_.size());

	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	client.loop();

	TEST_ASSERT_EQUAL_INT(2, test_messages.size());
	TEST_ASSERT_EQUAL_STRING("<- 07 04'01 56 78'FE B2", test_messages[0].c_str());
	TEST_ASSERT_EQUAL_STRING("Received unexpected frame while idle from device 7",
		test_messages[1].c_str());

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());
	TEST_ASSERT_EQUAL_INT(0, device.rx_.size());

	client.loop();

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x04, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0x01, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0x01, 0x56, 0x78, 0xFE, 0xB2 });

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

	TEST_ASSERT_EQUAL_INT(1, resp->data().size());
	TEST_ASSERT_EQUAL_INT(0x5678, resp->data()[0]);
}


/**
 * Queue a message while reading a message at idle.
 */
void queue_request_while_message_at_idle() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	device.tx_.insert(device.tx_.end(), { 0x07, 0x04, 0x01});

	client.loop();
	TEST_ASSERT_EQUAL_INT(0, device.tx_.size());

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	device.tx_.insert(device.tx_.end(), { 0x56, 0x78, 0xFE, 0xB2 });

	fake_millis += 1;
	client.loop();
	TEST_ASSERT_EQUAL_INT(0, device.tx_.size());

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());
	TEST_ASSERT_EQUAL_INT(0, device.rx_.size());

	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	client.loop();

	TEST_ASSERT_EQUAL_INT(2, test_messages.size());
	TEST_ASSERT_EQUAL_STRING("<- 07 04'01 56 78'FE B2", test_messages[0].c_str());
	TEST_ASSERT_EQUAL_STRING("Received unexpected frame while idle from device 7",
		test_messages[1].c_str());

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());
	TEST_ASSERT_EQUAL_INT(0, device.rx_.size());

	client.loop();

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x04, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0x01, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0x01, 0x56, 0x78, 0xFE, 0xB2 });

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

	TEST_ASSERT_EQUAL_INT(1, resp->data().size());
	TEST_ASSERT_EQUAL_INT(0x5678, resp->data()[0]);
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	RUN_TEST(nothing_at_idle);
	RUN_TEST(message_at_idle_1);
	RUN_TEST(message_at_idle_2);
	RUN_TEST(message_at_idle_parts);
	RUN_TEST(queue_request_before_message_at_idle);
	RUN_TEST(queue_request_while_message_at_idle);

	return UNITY_END();
}
