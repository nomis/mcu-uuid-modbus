/*
 * uuid-modbus - Microcontroller Modbus library
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

void test() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	client.read_exception_status(2);
	client.loop();

	TEST_ASSERT_EQUAL_INT(4, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x02, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x41, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[3]);
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();
	RUN_TEST(test);
	return UNITY_END();
}
