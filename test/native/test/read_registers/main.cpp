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
 * Read 0 input registers.
 */
void read_input_0() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 0);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

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
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0x00, 0xC2, 0xC1 });

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

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * Read 1 input register.
 */
void read_input_1() {
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
 * Read 2 input registers.
 */
void read_input_2() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 2);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

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
	TEST_ASSERT_EQUAL_UINT8(0x02, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0x02, 0xAB, 0xCD, 0xEF, 0x12, 0x68, 0x62 });

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

	TEST_ASSERT_EQUAL_INT(2, resp->data().size());
	TEST_ASSERT_EQUAL_INT(0xABCD, resp->data()[0]);
	TEST_ASSERT_EQUAL_INT(0xEF12, resp->data()[1]);
}

/**
 * Read 125 input registers.
 */
void read_input_125() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 125);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

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
	TEST_ASSERT_EQUAL_UINT8(0x7D, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {0x07, 0x04, 0x7D});
	for (uint8_t i = 1; i <= 125; i++) {
		device.tx_.insert(device.tx_.end(), {0x00, i});
	}
	device.tx_.insert(device.tx_.end(), {0xF5, 0x2C});

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

	TEST_ASSERT_EQUAL_INT(125, resp->data().size());
	for (uint16_t i = 1; i <= 125; i++) {
		TEST_ASSERT_EQUAL_INT(i, resp->data()[i - 1]);
	}
}

/**
 * Try to read too many input registers.
 */
void read_input_126() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 126);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_INVALID, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_FALSE(resp->success());
	TEST_ASSERT_TRUE(resp->failed());
}

/**
 * Response has the wrong length for the message data.
 */
void read_wrong_length_too_long() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 2);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

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
	TEST_ASSERT_EQUAL_UINT8(0x02, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0x03 /* should be 0x02 */, 0xAB, 0xCD, 0xEF, 0x12, 0x55, 0xA2 });

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
void read_wrong_length_too_short() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 2);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

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
	TEST_ASSERT_EQUAL_UINT8(0x02, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x04, 0x01 /* should be 0x02 */, 0xAB, 0xCD, 0xEF, 0x12, 0x2C, 0x62 });

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
void read_exception() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_input_registers(7, 0x1234, 2);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

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
	TEST_ASSERT_EQUAL_UINT8(0x02, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x84, 0x04, 0xA2, 0xC2 });

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
 * Read 0 holding registers.
 */
void read_holding_0() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_holding_registers(7, 0x1234, 0);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x03, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x03, 0x00, 0xC0, 0xF1 });

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

	TEST_ASSERT_EQUAL_INT(0, resp->data().size());
}

/**
 * Read 1 holding register.
 */
void read_holding_1() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_holding_registers(7, 0x1234, 1);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x03, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0x01, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x03, 0x01, 0x56, 0x78, 0xFF, 0xC6 });

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
 * Read 2 holding registers.
 */
void read_holding_2() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_holding_registers(7, 0x1234, 2);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x03, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0x02, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {
		0x07, 0x03, 0x02, 0xAB, 0xCD, 0xEF, 0x12, 0x69, 0xD5 });

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

	TEST_ASSERT_EQUAL_INT(2, resp->data().size());
	TEST_ASSERT_EQUAL_INT(0xABCD, resp->data()[0]);
	TEST_ASSERT_EQUAL_INT(0xEF12, resp->data()[1]);
}

/**
 * Read 125 holding registers.
 */
void read_holding_125() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_holding_registers(7, 0x1234, 125);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::QUEUED, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	client.loop();
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::WAITING, resp->status());
	TEST_ASSERT_TRUE(resp->pending());
	TEST_ASSERT_FALSE(resp->done());

	TEST_ASSERT_EQUAL_INT(8, device.rx_.size());
	TEST_ASSERT_EQUAL_UINT8(0x07, device.rx_[0]);
	TEST_ASSERT_EQUAL_UINT8(0x03, device.rx_[1]);
	TEST_ASSERT_EQUAL_UINT8(0x12, device.rx_[2]);
	TEST_ASSERT_EQUAL_UINT8(0x34, device.rx_[3]);
	TEST_ASSERT_EQUAL_UINT8(0x00, device.rx_[4]);
	TEST_ASSERT_EQUAL_UINT8(0x7D, device.rx_[5]);

	device.rx_.clear();
	device.tx_.insert(device.tx_.end(), {0x07, 0x03, 0x7D});
	for (uint8_t i = 1; i <= 125; i++) {
		device.tx_.insert(device.tx_.end(), {0x00, i});
	}
	device.tx_.insert(device.tx_.end(), {0x0D, 0x67});

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

	TEST_ASSERT_EQUAL_INT(125, resp->data().size());
	for (uint16_t i = 1; i <= 125; i++) {
		TEST_ASSERT_EQUAL_INT(i, resp->data()[i - 1]);
	}
}

/**
 * Try to read too many holding registers.
 */
void read_holding_126() {
	ModbusDevice device;
	uuid::modbus::SerialClient client{&device};

	auto resp = client.read_holding_registers(7, 0x1234, 126);
	TEST_ASSERT_EQUAL_INT(uuid::modbus::ResponseStatus::FAILURE_INVALID, resp->status());
	TEST_ASSERT_FALSE(resp->pending());
	TEST_ASSERT_TRUE(resp->done());
	TEST_ASSERT_FALSE(resp->success());
	TEST_ASSERT_TRUE(resp->failed());
}

int main(int argc, char *argv[]) {
	UNITY_BEGIN();

	RUN_TEST(read_input_0);
	RUN_TEST(read_input_1);
	RUN_TEST(read_input_2);
	RUN_TEST(read_input_125);
	RUN_TEST(read_input_126);

	RUN_TEST(read_wrong_length_too_long);
	RUN_TEST(read_wrong_length_too_short);
	RUN_TEST(read_exception);

	RUN_TEST(read_holding_0);
	RUN_TEST(read_holding_1);
	RUN_TEST(read_holding_2);
	RUN_TEST(read_holding_125);
	RUN_TEST(read_holding_126);

	return UNITY_END();
}
