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

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	RUN_TEST(message_at_idle_1);
	RUN_TEST(message_at_idle_2);

	return UNITY_END();
}
