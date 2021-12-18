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

#ifndef UUID_MODBUS_H_
#define UUID_MODBUS_H_

#include <Arduino.h>

#include <cstdarg>
#include <cstdint>
#include <memory>
#include <vector>

#include <uuid/log.h>

namespace uuid {

/**
 * Asynchronous Modbus library.
 *
 * Provides a client for communication using the Modbus protocol. This library
 * is for single threaded applications and cannot be used from an interrupt
 * context.
 *
 * - <a href="https://github.com/nomis/mcu-uuid-modbus/">Git Repository</a>
 * - <a href="https://mcu-uuid-modbus.readthedocs.io/">Documentation</a>
 */
namespace modbus {

/**
 * Status of response messages.
 *
 * @since 0.1.0
 */
enum ResponseStatus : uint8_t {
	QUEUED, /*!< Waiting in queue. @since 0.1.0 */
	TRANSMIT, /*!< Request being transmitted. @since 0.1.0 */
	WAITING, /*!< Waiting for response. @since 0.1.0 */
	SUCCESS, /*!< Response received. @since 0.1.0 */
	TIMEOUT, /*!< Request timed out. @since 0.1.0 */
	EXCEPTION, /*!< Exception response received. @since 0.1.0 */
};

/**
 * Response message.
 *
 * This will be created when a request is submitted and then later updated with
 * the outcome. Poll the #status of the response to know when to access data.
 *
 * @since 0.1.0
 */
class Response {
	/**
	 * SerialClient needs to be able to set the status and store response data.
	 *
	 * @since 0.1.0
	 */
	friend class SerialClient;

public:
	virtual ~Response() = default;

	/**
	 * Determine if the request is complete.
	 *
	 * @return True if the request is finished, otherwise false.
	 * @since 0.1.0
	 */
	inline bool done() final const { return status >= ResponseStatus::SUCCESS; }

	/**
	 * Determine if the request is still pending.
	 *
	 * @return True if the request is in progress, otherwise false.
	 * @since 0.1.0
	 */
	inline bool pending() final const { return status < ResponseStatus::SUCCESS; }

	/**
	 * Determine if the request was successful.
	 *
	 * @return True if the request was successful, otherwise false.
	 * @since 0.1.0
	 */
	bool success() final const { return status_ == ResponseStatus::SUCCESS; }

	/**
	 * Determine if the request timed out.
	 *
	 * @return True if the request timed out, otherwise false.
	 * @since 0.1.0
	 */
	bool timeout() final const { return status_ == ResponseStatus::TIMEOUT; }

	/**
	 * Determine if the request returned an exception.
	 *
	 * @return True if the request returned an exception, otherwise false.
	 * @since 0.1.0
	 */
	bool exception() final const { return status_ == ResponseStatus::EXCEPTION; }

	/**
	 * Exception code from device response.
	 *
	 * Valid only if the status() is ResponseStatus::EXCEPTION.
	 *
	 * @since 0.1.0
	 */
	inline uint8_t exception_code() final const { return exception_code_; }

private:
	ResponseStatus status_; /*!< Status of response message. @since 0.1.0 */
	uint8_t exception_code_; /*!< Exception code from device response. @since 0.1.0 */
};

/**
 * Register response message.
 *
 * This will be created when a request is submitted and then later updated with
 * the outcome. Poll the #status of the response to know when to access data.
 *
 * @since 0.1.0
 */
class RegisterResponse: public Response {
	/**
	 * SerialClient needs to be able to set the status and store response data.
	 *
	 * @since 0.1.0
	 */
	friend class SerialClient;

public:
	/**
	 * Data from the device response, which may be fewer or more register values
	 * than requested.
	 *
	 * Valid only if the status() is ResponseStatus::SUCCESS.
	 *
	 * @return A reference to the data in the response.
	 * @since 0.1.0
	 */
	const std::vector<uint16_t>& data() const;

private:
	std::vector<uint16_t> data_; /*!< Data from device response. @since 0.1.0 */
};

/*
 * Request message.
 *
 * This will be created when a request is submitted and then discarded when the
 * response is updated with the outcome.
 *
 * @since 0.1.0
 */
class Request {
public:
	virtual ~Request() = default;

public:
	/**
	 * TODO
	 *
	 * @param[out] frame Message frame buffer.
	 * @return Size of message frame.
	 * @since 0.1.0
	 */
	uint16_t encode(uint8_t &frame[256]) = 0;

	/**
	 * Remote device address (0 to 247).
	 *
	 * @since 0.1.0
	 */
	const uint16_t device;

	/**
	 * Request message function code.
	 *
	 * @since 0.1.0
	 */
	const uint8_t function_code;

	/**
	 * Corresponding response object.
	 *
	 * @since 0.1.0
	 */
	const std::shared_ptr<Response> response;

protected:
	/**
	 * TODO
	 *
	 * @since 0.1.0
	 */
	Request(uint16_t device, uint8_t function_code, const std::shared_ptr<Response> &response);
};

/*
 * Request message for register functions.
 *
 * This will be created when a request is submitted and then discarded when the
 * response is updated with the outcome.
 *
 * @since 0.1.0
 */
class RegisterRequest: public Request {
public:
	/**
	 * TODO
	 *
	 * @since 0.1.0
	 */
	RegisterRequest(uint16_t device, uint8_t function_code, uint16_t address, uint16_t data);

	/**
	 * TODO
	 *
	 * @param[out] buffer Message frame data.
	 * @return Size of message frame.
	 * @since 0.1.0
	 */
	uint16_t encode(uint8_t &buffer[256]) = 0;

private:
	const uint16_t address_;
	const uint16_t data_;
};

/**
 * Serial client used to process requests.
 *
 * @since 0.1.0
 */
class SerialClient {
public:
	/**
	 * Create a new client.
	 *
	 * @param[in] serial Serial port object.
	 * @param[in] speed Baud rate.
	 * @param[in] config Configure data, parity and stop bits.
	 * @since 0.1.0
	 */
	SerialClient(::HardwareSerial *serial, unsigned long speed = 19200, uint8_t config = SERIAL_8E1);

	/**
	 * Create a new client.
	 *
	 * @param[in] stream Stream object.
	 * @param[in] speed Baud rate (for timeout calculations).
	 * @since 0.1.0
	 */
	SerialClient(::Stream *stream, unsigned long speed = 19200);

	~SerialClient() = default;

	/**
	 * Loop function that must be called regularly to send and receive messages.
	 *
	 * @since 0.1.0
	 */
	void loop();

	/**
	 * Read a contiguous block of holding registers from a remote device.
	 *
	 * @param[in] device Device address (1 to 247).
	 * @param[in] address Starting address (0x0000 to 0xFFFF).
	 * @param[in] size Quantity of registers (0x0001 to 0x007D).
	 * @return A response message that will contain the outcome and data in the
	 *         future when processing is complete.
	 * @since 0.1.0
	 */
	std::shared_ptr<RegisterResponse> read_holding_registers(uint16_t device, uint16_t address, uint16_t size);

	/**
	 * Read a contiguous block of input registers from a remote device.
	 *
	 * @param[in] device Device address (1 to 247).
	 * @param[in] address Starting address (0x0000 to 0xFFFF).
	 * @param[in] size Quantity of registers (0x0001 to 0x007D).
	 * @return A response message that will contain the outcome and data in the
	 *         future when processing is complete.
	 * @since 0.1.0
	 */
	std::shared_ptr<RegisterResponse> read_input_registers(uint16_t device, uint16_t address, uint16_t size);

	/**
	 * Write to a single holding register in a remote device.
	 *
	 * @param[in] device Device address (0 to 247).
	 * @param[in] address Register address (0x0000 to 0xFFFF).
	 * @param[in] value Register value.
	 * @return A response message that will contain the outcome and echoed data
	 *         in the future when processing is complete.
	 * @since 0.1.0
	 */
	std::shared_ptr<RegisterResponse> write_holding_register(uint16_t device, uint16_t address, uint16_t value);

private:
	::Stream *stream_; /*!< Serial port stream. @since 0.1.0 */
	std::vector<Request> requests_; /*!< Pending requests. @since 0.1.0 */

	uint8_t inter_char_timeout_ms_; /*!< Timeout between characters. @since 0.1.0 */
	uint8_t frame_timeout_ms_; /*!< Timeout between frames. @since 0.1.0 */
	uint8_t frame_[256]; /*!< Current message frame. @since 0.1.0 */

	uint16_t rx_frame_size_ = 0; /*!< Size of response frame that has been received. @since 0.1.0 */
	uint32_t last_rx_ms_ = 0; /*!< Time that the last character was received. @since 0.1.0 */

	uint16_t tx_frame_remaining_ = 0; /*!< Size of request frame that still has to be transmitted. @since 0.1.0 */
	uint32_t last_tx_ms_ = 0; /*!< Time that the last character was transmitted. @since 0.1.0 */
};

} // namespace log

} // namespace uuid

#endif
