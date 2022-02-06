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
#ifdef ENV_NATIVE

#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>

#include <string>

NativeConsole Serial;
ModbusDevice Serial1;

static unsigned long __millis = 0;

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
	setup();
	while (millis() <= 11 * 1000) {
		loop();
		delay(1);
	}
	return 0;
}

unsigned long millis() {
	return __millis;
}

void delay(unsigned long millis) {
	__millis += millis;
}

void yield(void) {

}

int snprintf_P(char *str, size_t size, const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	int ret = vsnprintf_P(str, size, format, ap);
	va_end(ap);

	return ret;
}

int vsnprintf_P(char *str, size_t size, const char *format, va_list ap) {
	std::string native_format;

	char previous = 0;
	for (size_t i = 0; i < strlen(format); i++) {
		char c = format[i];

		// This would be a lot easier if the ESP8266 platform
		// simply read all strings with 32-bit accesses instead
		// of repurposing %S (wchar_t).
		if (previous == '%' && c == 'S') {
			c = 's';
		}

		native_format += c;
		previous = c;
	}

	return vsnprintf(str, size, native_format.c_str(), ap);
}

void ModbusDevice::respond() {
	if (rx_[0] == 0x7B
			&& rx_[1] == 0x04
			&& rx_[2] == 0x00
			&& rx_[3] == 0xA0
			&& rx_[4] == 0x00
			&& rx_[5] == 0x04
			&& rx_[6] == 0xFA
			&& rx_[7] == 0x71) {
		rx_.erase(rx_.begin(), rx_.begin() + 8);
		tx_.insert(tx_.end(), { 0x7B, 0x04, 0x08, 0x12, 0x34, 0x56, 0x78,
			0x90, 0xAB, 0xCD, 0xEF, 0xBF, 0xC2 });
		delay(50);
	} else if (rx_[0] == 0x7B
			&& rx_[1] == 0x04
			&& rx_[2] == 0x00
			&& rx_[3] == 0xA4
			&& rx_[4] == 0x00
			&& rx_[5] == 0x04
			&& rx_[6] == 0xBB
			&& rx_[7] == 0xB0) {
		rx_.erase(rx_.begin(), rx_.begin() + 8);
		tx_.insert(tx_.end(), { 0x7B, 0x04, 0x08, 0x23, 0x45, 0x67, 0x89,
			0x0A, 0xBC, 0xDE, 0xF1, 0x76, 0x0D });
		delay(50);
	} else if (rx_[0] == 0x7B
			&& rx_[1] == 0x04
			&& rx_[2] == 0x00
			&& rx_[3] == 0xA8
			&& rx_[4] == 0x00
			&& rx_[5] == 0x04
			&& rx_[6] == 0x7B
			&& rx_[7] == 0xB3) {
		rx_.erase(rx_.begin(), rx_.begin() + 8);
		tx_.insert(tx_.end(), { 0x7B, 0x04, 0x08, 0x34, 0x56, 0x78, 0x90,
			0xAB, 0xCD, 0xEF, 0x12, 0x2C, 0x75 });
		delay(50);
	} else if (rx_[0] == 0x7B
			&& rx_[1] == 0x04
			&& rx_[2] == 0x00
			&& rx_[3] == 0xAC
			&& rx_[4] == 0x00
			&& rx_[5] == 0x04
			&& rx_[6] == 0x3A
			&& rx_[7] == 0x72) {
		rx_.erase(rx_.begin(), rx_.begin() + 8);
		tx_.insert(tx_.end(), { 0x7B, 0x04, 0x08, 0x45, 0x67, 0x89, 0x0A,
			0xBC, 0xDE, 0xF1, 0x23, 0xEE, 0xFF }); // Invalid CRC
		delay(50);
	} else if (rx_[0] == 0x7B
			&& rx_[1] == 0x04
			&& rx_[2] == 0x00
			&& rx_[3] == 0xB0
			&& rx_[4] == 0x00
			&& rx_[5] == 0x04
			&& rx_[6] == 0xFB
			&& rx_[7] == 0xB4) {
		rx_.erase(rx_.begin(), rx_.begin() + 8);
		tx_.insert(tx_.end(), { 0x7B, 0x84, 0x02, 0xE3, 0x18 }); // Exception
		delay(50);
	} else if (rx_[0] == 0x7B
			&& rx_[1] == 0x04
			&& rx_[2] == 0x00
			&& rx_[3] == 0xB4
			&& rx_[4] == 0x00
			&& rx_[5] == 0x04
			&& rx_[6] == 0xBA
			&& rx_[7] == 0x75) {
		rx_.erase(rx_.begin(), rx_.begin() + 8);
		tx_.insert(tx_.end(), { 0x7B, 0x04, 0x06, 0x56, 0x78, 0x9A, 0xBC,
			0xDE, 0xF0, 0x61, 0x15 }); // Unexpected number of registers
		delay(50);
	} else if (rx_[0] == 0x7B
			&& rx_[1] == 0x04
			&& rx_[2] == 0x00
			&& rx_[3] == 0xB8
			&& rx_[4] == 0x00
			&& rx_[5] == 0x04
			&& rx_[6] == 0x7A
			&& rx_[7] == 0x76) {
		rx_.erase(rx_.begin(), rx_.begin() + 8);
		tx_.insert(tx_.end(), { 0x7B, 0x04, 0x08, 0x67, 0x89, 0xAB, 0xCD,
			0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0x49, 0x92 }); // Length mismatch
		delay(50);
	}
}

#endif
