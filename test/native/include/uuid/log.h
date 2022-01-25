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

#ifndef MOCK_UUID_LOG_H_
#define MOCK_UUID_LOG_H_

#include <cstdarg>
#include <cstdint>
#include <string>
#include <vector>

extern std::vector<std::string> test_messages;

namespace uuid {

namespace log {

enum class Level : int8_t {
	OFF = -1,
	EMERG = 0,
	ALERT,
	CRIT,
	ERR,
	WARNING,
	NOTICE,
	INFO,
	DEBUG,
	TRACE,
	ALL,
};

enum class Facility : uint8_t {
	DAEMON,
};

#if 0
static std::string format_timestamp_ms(uint64_t timestamp_ms, unsigned int days_width = 1) { return ""; }
static char format_level_char(Level level) { return ' '; }
static const __FlashStringHelper *format_level_uppercase(Level level) { return F(""); }
static const __FlashStringHelper *format_level_lowercase(Level level) { return F(""); }
#endif

struct Message {
	Message(uint64_t uptime_ms, Level level, Facility facility, const __FlashStringHelper *name, const std::string &&text);
	~Message() = default;

	const uint64_t uptime_ms;
	const Level level;
	const Facility facility;
	const __FlashStringHelper *name;
	const std::string text;
};

class Handler {
public:
	virtual ~Handler() = default;

	virtual void operator<<(std::shared_ptr<Message> message) = 0;

protected:
	Handler() = default;
};

class Logger {
public:
	Logger(const __FlashStringHelper *name, Facility facility) {};
	~Logger() = default;

	static void register_handler(Handler *handler, Level level) {}
	static void unregister_handler(Handler *handler) {}

	static Level get_log_level(const Handler *handler) { return Level::ALL; }

	static inline bool enabled(Level level) { return true; }
	void emerg(const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void emerg(const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }
	void alert(const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void alert(const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }
	void crit(const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void crit(const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }
	void err(const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void err(const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }
	void warning(const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void warning(const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }
	void notice(const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void notice(const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }
	void info(const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void info(const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }
	void debug(const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void debug(const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }
	void trace(const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void trace(const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }
	void log(Level level, Facility facility, const char *format, ...) const { va_list ap; va_start(ap, format); __vprintf(format, ap); va_end(ap); printf("\n"); }
	void log(Level level, Facility facility, const __FlashStringHelper *format, ...) const { va_list ap; va_start(ap, format); __vprintf(reinterpret_cast<const char *>(format), ap); va_end(ap); printf("\n"); }

private:
	static void __vprintf(const char *format, va_list &ap) {
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

		va_list ap_copy;
		va_copy(ap_copy, ap);
		vprintf(native_format.c_str(), ap);

		std::vector<char> text(1024);
		vsnprintf(text.data(), text.size(), native_format.c_str(), ap_copy);
		test_messages.emplace_back(text.data());
		va_end(ap_copy);
	}
};

} // namespace log

} // namespace uuid

#endif
