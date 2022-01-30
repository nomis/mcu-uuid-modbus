#include <Arduino.h>
#include <uuid/common.h>
#include <uuid/log.h>
#include <uuid/modbus.h>

static uuid::log::PrintHandler log_handler{Serial};
static uuid::modbus::SerialClient client{Serial1};

void setup() {
	Serial.begin(115200);
	Serial1.begin(19200, SERIAL_8E1);
	uuid::log::Logger::register_handler(&log_handler, uuid::log::Level::ALL);
}

void loop() {
	static uuid::log::Logger logger{F("example")};
	static std::shared_ptr<const uuid::modbus::RegisterDataResponse> response;
	static uint16_t address = 0x00A0;

	uuid::loop();
	client.loop();

	if (!response) {
		if (address < 0x00C0) {
			logger.info(F("Reading from device at address %04X"), address);
			response = client.read_input_registers(123, address, 4);
			address += 4;
		}
	} else if (response->done()) {
		if (response->success()) {
			if (response->data().size() == 4) {
				logger.info(F("Data: %04X %04X %04X %04X"),
					response->data()[0],
					response->data()[1],
					response->data()[2],
					response->data()[3]);
			} else {
				logger.err(F("Invalid number of registers in response: %u"),
					response->data().size());
			}
		} else {
			logger.err(F("Failed"));
		}

		response.reset();
		log_handler.loop();
		Serial.println();
	}

	log_handler.loop();
}
