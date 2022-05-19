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
 * Write 1 holding register.
 */
void write_holding_1() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{device};

	auto resp = client.write_holding_register(7, 0x1234, 0xABCD);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x06, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0xAB, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0xCD, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x06, 0x12, 0x34, 0xAB, 0xCD, 0x73, 0xBF });

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
	TEST_ASSERT_EQUAL_INT(0x1234, resp->address());
	TEST_ASSERT_EQUAL_INT(0xABCD, resp->data()[0]);
}

/**
 * Response has the wrong length for the message data.
 */
void write_wrong_length_too_long() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{device};

	auto resp = client.write_holding_register(7, 0x1234, 0xABCD);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x06, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0xAB, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0xCD, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x06, 0x12, 0x34, 0xAB, 0xCD, 0xEE /* should not be present */, 0x7E, 0xA9 });

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

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * Response has the wrong length for the message data.
 */
void write_wrong_length_too_short() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{device};

	auto resp = client.write_holding_register(7, 0x1234, 0xABCD);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x06, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0xAB, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0xCD, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x06, 0x12, 0x34, 0xAB, /* missing: 0xCD, */ 0x66, 0x32 });

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

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * Response with an exception.
 */
void write_exception() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{device};

	auto resp = client.write_holding_register(7, 0x1234, 0xABCD);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x06, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0xAB, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0xCD, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x86, 0x04, 0xA3, 0xA2 });

	client.loop();
	fake_millis += uuid::modbus::INTER_FRAME_TIMEOUT_MS;
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::EXCEPTION, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->exception());
	TEST_ASSERT_FALSE(resp->success());
	TEST_ASSERT_FALSE(resp->failed());

	TEST_ASSERT_EQUAL_INT(0x04, resp->exception_code());
	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * Write to the broadcast device address.
 */
void write_holding_broadcast_device() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{device};

	auto resp = client.write_holding_register(0, 0x1234, 0xABCD);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	while (1) {
		client.loop();

		if (resp->done()) {
			break;
		}

		fake_millis++;
		TEST_ASSERT_LESS_THAN(2000, fake_millis);
	}

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x06, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0xAB, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0xCD, device.rx_[5]);

	TEST_ASSERT_EQUAL_INT(1000, fake_millis);

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::SUCCESS, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->address());
	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * Write to the broadcast device address with explicit delay.
 */
void write_holding_broadcast_device_explicit_delay() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{device};

	auto resp = client.write_holding_register(0, 0x1234, 0xABCD, 100);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	while (1) {
		client.loop();

		if (resp->done()) {
			break;
		}

		fake_millis++;
		TEST_ASSERT_LESS_THAN(200, fake_millis);
	}

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x06, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0xAB, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0xCD, device.rx_[5]);

	TEST_ASSERT_EQUAL_INT(100, fake_millis);

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::SUCCESS, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->address());
	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * Write to the broadcast device address with explicit delay.
 */
void write_holding_broadcast_device_implicit_default_delay() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{device};

	client.default_broadcast_delay_ms(100);

	auto resp = client.write_holding_register(0, 0x1234, 0xABCD);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	while (1) {
		client.loop();

		if (resp->done()) {
			break;
		}

		fake_millis++;
		TEST_ASSERT_LESS_THAN(200, fake_millis);
	}

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x06, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0xAB, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0xCD, device.rx_[5]);

	TEST_ASSERT_EQUAL_INT(100, fake_millis);

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::SUCCESS, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->address());
	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * Write to the broadcast device address with explicit delay.
 */
void write_holding_broadcast_device_explicit_default_delay() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{device};

	client.default_broadcast_delay_ms(100);

	auto resp = client.write_holding_register(0, 0x1234, 0xABCD, 0);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	while (1) {
		client.loop();

		if (resp->done()) {
			break;
		}

		fake_millis++;
		TEST_ASSERT_LESS_THAN(200, fake_millis);
	}

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x06, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0xAB, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0xCD, device.rx_[5]);

	TEST_ASSERT_EQUAL_INT(100, fake_millis);

	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::SUCCESS, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_TRUE(resp->success());

	TEST_ASSERT_EQUAL_INT(0, resp->address());
	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * Try to write to a reserved device address.
 */
void write_holding_reserved_device() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{device};

	auto resp = client.write_holding_register(248, 0x1234, 0xABCD);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_INVALID, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_FALSE(resp->success());
	TEST_ASSERT_TRUE(resp->failed());
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	RUN_TEST(write_holding_1);

	RUN_TEST(write_wrong_length_too_long);
	RUN_TEST(write_wrong_length_too_short);
	RUN_TEST(write_exception);

	RUN_TEST(write_holding_broadcast_device);
	RUN_TEST(write_holding_broadcast_device_explicit_delay);
	RUN_TEST(write_holding_broadcast_device_implicit_default_delay);
	RUN_TEST(write_holding_broadcast_device_explicit_default_delay);
	RUN_TEST(write_holding_reserved_device);

	return UNITY_END();
}
