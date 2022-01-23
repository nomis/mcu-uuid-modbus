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

/**
 * No response at all to a request should result in a timeout.
 */
void no_response() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	while (1) {
		client.loop();

		if (resp->done()) {
			break;
		}

		fake_millis++;
		TEST_ASSERT_LESS_THAN(15000, fake_millis);
	}

	TEST_ASSERT_EQUAL_INT(10000, fake_millis);

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_TIMEOUT, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->failed());
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	RUN_TEST(no_response);

	return UNITY_END();
}
