#include <Arduino.h>
#include <uuid/common.h>
#include <uuid/log.h>
#include <uuid/modbus.h>

class SerialLogHandler;

static uuid::modbus::SerialClient client{&Serial1};
static std::shared_ptr<const uuid::modbus::RegisterDataResponse> response;
static void setup_logging();

void setup() {
	Serial.begin(115200);
	Serial1.begin(19200, SERIAL_8E1);
	setup_logging();
}

void loop() {
	static uint16_t address = 0x00A0;

	uuid::loop();
	client.loop();

	if (!response) {
		if (address < 0x00C0) {
			Serial.print(F("Reading from device at address "));
			Serial.println(address);
			response = client.read_input_registers(123, address, 4);
			address += 4;
		}
	} else if (response->done()) {
		if (response->success()) {
			if (response->data().size() == 4) {
				Serial.print(F("Data: "));
				Serial.print(response->data()[0]);
				Serial.print(' ');
				Serial.print(response->data()[1]);
				Serial.print(' ');
				Serial.print(response->data()[2]);
				Serial.print(' ');
				Serial.println(response->data()[3]);
			} else {
				Serial.print(F("Invalid number of registers in response: "));
				Serial.println(response->data().size());
			}
		} else {
			Serial.println(F("Failed"));
		}

		response.reset();
		Serial.println();
	}
}

/* Basic logger so that the example has a way to output log messages. */
class SerialLogHandler: public uuid::log::Handler {
public:
	SerialLogHandler() = default;

	void start() {
		uuid::log::Logger::register_handler(this, uuid::log::Level::ALL);
	}

	/*
	 * It is not recommended to directly output from this function,
	 * this is only a simple example. Messages should normally be
	 * queued for later output when the application is less busy.
	 */
	void operator<<(std::shared_ptr<uuid::log::Message> message) {
		char temp[200] = { 0 };

		int ret = snprintf_P(temp, sizeof(temp), PSTR("%s %c [%S] %s\r\n"),
			uuid::log::format_timestamp_ms(message->uptime_ms).c_str(),
			uuid::log::format_level_char(message->level),
			message->name, message->text.c_str());

		if (ret > 0) {
			Serial.print(temp);
		}
	}
};

static SerialLogHandler log_handler;

static void setup_logging() {
	log_handler.start();
}
