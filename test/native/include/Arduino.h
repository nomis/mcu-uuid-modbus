/*
 * uuid-modbus - Microcontroller Modbus library
 * Copyright 2021  Simon Arlott
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

#ifndef ARDUINO_H_
#define ARDUINO_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>

#define PROGMEM
#define PGM_P const char *
#define PSTR(s) (__extension__({static const char __c[] = (s); &__c[0];}))

class __FlashStringHelper;
#define FPSTR(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

#define snprintf_P snprintf
#define vsnprintf_P vsnprintf

#define pgm_read_byte(addr) (*reinterpret_cast<const char *>(addr))

unsigned long millis();

static __attribute__((unused)) void yield(void) {}

class Print {
public:
    virtual ~Print() = default;
    virtual size_t write(uint8_t c __attribute__((unused))) { return 1; }
    virtual size_t write(const uint8_t *buffer __attribute__((unused)), size_t size) { return size; }
    size_t print(char c) { return write((uint8_t)c); }
    size_t print(const char *data) { return write(reinterpret_cast<const uint8_t *>(data), strlen(data)); }
    size_t print(const __FlashStringHelper *data) { return print(reinterpret_cast<const char *>(data)); }
    size_t println() { return print("\r\n"); }
    size_t println(const char *data) { return print(data) + println(); }
    size_t println(const __FlashStringHelper *data) { return print(reinterpret_cast<const char *>(data)) + println(); }
    virtual void flush() { };
};

class Stream: public Print {
public:
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
};

class HardwareSerial: public Stream {
public:
    void begin(unsigned long baud __attribute__((unused)),
            uint8_t config __attribute__((unused))) {
    }

    virtual int availableForWrite(void) = 0;
    virtual size_t write(uint8_t c) = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
};

class ModbusDevice: public HardwareSerial {
public:
    int available() override {
        return tx_.size();
    }

    int read() override {
        if (!tx_.empty()) {
            uint8_t value = tx_.front();
            tx_.pop_front();
            return value;
        } else {
            return -1;
        }
    }

    int peek() override {
        if (!tx_.empty()) {
            return tx_.front();
        } else {
            return -1;
        }
    }

    int availableForWrite(void) override {
        return available_write_;
    }

    size_t write(uint8_t c) override {
        rx_.push_back(c);
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size) override {
        rx_.insert(rx_.end(), buffer, buffer + size);
        return size;
    }

    int available_write_ = 512;
    std::deque<uint8_t> rx_;
    std::deque<uint8_t> tx_;
};

#endif
