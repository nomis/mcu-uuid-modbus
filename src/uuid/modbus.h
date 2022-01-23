/*
 * uuid-modbus - Microcontroller asynchronous Modbus library
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

#ifndef UUID_MODBUS_H_
#define UUID_MODBUS_H_

#include <Arduino.h>

#include <cstdarg>
#include <cstdint>
#include <array>
#include <deque>
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

constexpr uint16_t MAX_MESSAGE_SIZE = 256; /*!< Maximum size of a message. @since 0.1.0 */
constexpr uint32_t INTER_FRAME_TIMEOUT_MS = 2; /*!< Timeout between frames (in milliseconds). @since 0.1.0 */
constexpr uint32_t DEFAULT_TIMEOUT_MS = 10000; /*!< Default time to wait for a response (in milliseconds). @since 0.1.0 */

extern const uuid::log::Logger logger; /*!< uuid::log::Logger instance for Modbus library. @since 0.1.0 */

using frame_buffer_t = std::array<uint8_t, MAX_MESSAGE_SIZE + 1>; /*!< Buffer for receiving frames. @since 0.1.0 */

/**
 * Device address types.
 *
 * All other values are reserved.
 *
 * @since 0.1.0
 */
enum DeviceAddressType : uint8_t {
	BROADCAST = 0, /*!< Broadcast device address. @since 0.1.0 */
	MIN_UNICAST = 1, /*!< Minimum device address. @since 0.1.0 */
	MAX_UNICAST = 247, /*!< Maximum device address. @since 0.1.0 */
};

/**
 * Function codes.
 *
 * @since 0.1.0
 */
enum FunctionCode : uint8_t {
	READ_HOLDING_REGISTERS = 0x03, /*!< Read holding registers. @since 0.1.0 */
	READ_INPUT_REGISTERS = 0x04, /*!< Read input registers. @since 0.1.0 */
	WRITE_SINGLE_REGISTER = 0x06, /*!< Write single register. @since 0.1.0 */
	READ_EXCEPTION_STATUS = 0x07, /*!< Read exception status. @since 0.1.0 */
};

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
	EXCEPTION, /*!< Exception response received. @since 0.1.0 */
	FAILURE_INVALID, /*!< Invalid request parameters. @since 0.1.0 */
	FAILURE_CRC, /*!< Invalid CRC in response. @since 0.1.0 */
	FAILURE_TIMEOUT, /*!< Request timed out. @since 0.1.0 */
	FAILURE_TOO_SHORT, /*!< Response too short. @since 0.1.0 */
	FAILURE_TOO_LONG, /*!< Response too long. @since 0.1.0 */
	FAILURE_ADDRESS, /*!< Response from another device. @since 0.1.0 */
	FAILURE_FUNCTION, /*!< Unexpected function code in response. @since 0.1.0 */
	FAILURE_LENGTH, /*!< Incorrect response length. @since 0.1.0 */
};

/**
 * Response message.
 *
 * This will be created when a request is submitted and then later updated with
 * the outcome. Poll the status() of the response to know when to access data.
 *
 * @since 0.1.0
 */
class Response {
public:
	virtual ~Response() = default;

	/**
	 * Determine if the request is complete.
	 *
	 * @return True if the request is finished, otherwise false.
	 * @since 0.1.0
	 */
	inline bool done() const { return status_ >= ResponseStatus::SUCCESS; }

	/**
	 * Determine if the request is still pending.
	 *
	 * @return True if the request is in progress, otherwise false.
	 * @since 0.1.0
	 */
	inline bool pending() const { return status_ < ResponseStatus::SUCCESS; }

	/**
	 * Determine if the request was successful.
	 *
	 * @return True if the request was successful, otherwise false.
	 * @since 0.1.0
	 */
	bool success() const { return status_ == ResponseStatus::SUCCESS; }

	/**
	 * Determine if the request returned an exception.
	 *
	 * @return True if the request returned an exception, otherwise false.
	 * @since 0.1.0
	 */
	bool exception() const { return status_ == ResponseStatus::EXCEPTION; }

	/**
	 * Determine if the request failed for a reason other than an exception.
	 *
	 * @return True if the request failed (without receiving an exception),
	 *         otherwise false.
	 * @since 0.1.0
	 */
	bool failed() const { return status_ > ResponseStatus::EXCEPTION; }

	/**
	 * Get the status of the response message.
	 *
	 * @return Status of the response message.
	 * @since 0.1.0
	 */
	inline ResponseStatus status() const { return status_; }

	/**
	 * Set the status of the response message.
	 *
	 * @param[in] status Status of the response message.
	 * @since 0.1.0
	 */
	inline void status(ResponseStatus status) { status_ = status; }

	/**
	 * Get the exception code from the device response.
	 *
	 * Valid only if the status() is ResponseStatus::EXCEPTION or exception()
	 * returns true.
	 *
	 * @return Exception code from the device response.
	 * @since 0.1.0
	 */
	inline uint8_t exception_code() const { return exception_code_; }

	/**
	 * Set the exception code from the device response.
	 *
	 * Valid only if the status() is ResponseStatus::EXCEPTION or exception()
	 * returns true.
	 *
	 * @param[in] exception_code Exception code from the device response.
	 * @since 0.1.0
	 */
	inline void exception_code(uint8_t exception_code) { exception_code_ = exception_code; }

	/**
	 * Parse a message frame buffer and store the outcome in this response.
	 *
	 * @param[in] frame Message frame buffer.
	 * @param[in] len Size of message frame.
	 * @return The status result of message parsing.
	 * @since 0.1.0
	 */
	virtual ResponseStatus parse(frame_buffer_t &frame, uint16_t len) = 0;

protected:
	Response() = default;

	/**
	 * Check the length of the message frame is correct and log an error if it
	 * is not.
	 *
	 * @param[in] frame Message frame buffer.
	 * @param[in] actual Actual length of the message frame.
	 * @param[in] expected Expected length of the message frame.
	 * @return True if the message frame length matches, otherwise false.
	 * @since 0.1.0
	 */
	bool check_length(frame_buffer_t &frame, uint16_t actual, uint16_t expected);

private:
	ResponseStatus status_ = ResponseStatus::QUEUED; /*!< Status of response message. @since 0.1.0 */
	uint8_t exception_code_ = 0; /*!< Device exception response. @since 0.1.0 */
};

/**
 * Register data response message.
 *
 * This will be created when a request is submitted and then later updated with
 * the outcome. Poll the status() of the response to know when to access data.
 *
 * @since 0.1.0
 */
class RegisterDataResponse: public Response {
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
	inline const std::vector<uint16_t>& data() const { return data_; };

	/**
	 * Parse a message frame buffer and store the outcome in this response.
	 *
	 * @param[in] frame Message frame buffer.
	 * @param[in] len Size of message frame.
	 * @return The status result of message parsing.
	 * @since 0.1.0
	 */
	ResponseStatus parse(frame_buffer_t &frame, uint16_t len) override;

protected:
	std::vector<uint16_t> data_; /*!< Data from device response. @since 0.1.0 */
};

/**
 * Register write response message.
 *
 * This will be created when a request is submitted and then later updated with
 * the outcome. Poll the status() of the response to know when to access data.
 *
 * @since 0.1.0
 */
class RegisterWriteResponse: public RegisterDataResponse {
public:
	/**
	 * Parse a message frame buffer and store the outcome in this response.
	 *
	 * @param[in] frame Message frame buffer.
	 * @param[in] len Size of message frame.
	 * @return The status result of message parsing.
	 * @since 0.1.0
	 */
	ResponseStatus parse(frame_buffer_t &frame, uint16_t len) override;

	/**
	 * Get the address from the device response, which should match the address
	 * that was requested.
	 *
	 * Valid only if the status() is ResponseStatus::SUCCESS.
	 *
	 * @return Address from the response.
	 * @since 0.1.0
	 */
	uint16_t address() const { return address_; }

private:
	uint16_t address_; /*!< Address from device response. @since 0.1.0 */
};

/**
 * Exception status response message.
 *
 * This will be created when a request is submitted and then later updated with
 * the outcome. Poll the status() of the response to know when to access data.
 *
 * @since 0.1.0
 */
class ExceptionStatusResponse: public Response {
public:
	/**
	 * Parse a message frame buffer and store the outcome in this response.
	 *
	 * @param[in] frame Message frame buffer.
	 * @param[in] len Size of message frame.
	 * @return The status result of message parsing.
	 * @since 0.1.0
	 */
	ResponseStatus parse(frame_buffer_t &frame, uint16_t len) override;

	/**
	 * Get the output data from the device response.
	 *
	 * Valid only if the status() is ResponseStatus::SUCCESS.
	 *
	 * @return Output data from the response.
	 * @since 0.1.0
	 */
	inline uint8_t data() const { return data_; };

private:
	uint8_t data_; /*!< Output data from device response. @since 0.1.0 */
};

/**
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
	 * Create a new request message (not directly useful).
	 *
	 * @param[in] device Destination device address.
	 * @param[in] function_code Function code of the request.
	 * @param[in] timeout_ms Timeout to wait for a response in milliseconds.
	 * @param[in] response Response object.
	 * @since 0.1.0
	 */
	Request(uint16_t device, uint8_t function_code, uint32_t timeout_ms,
		const std::shared_ptr<Response> &response);

	/**
	 * Encode this request and store it in a message frame buffer.
	 *
	 * @param[out] frame Message frame buffer.
	 * @return Size of message frame.
	 * @since 0.1.0
	 */
	virtual uint16_t encode(frame_buffer_t &frame);

	/**
	 * Get the destination device address.
	 *
	 * @return Remote device address.
	 * @since 0.1.0
	 */
	inline uint16_t device() const { return device_; };

	/**
	 * Get the function code of the request.
	 *
	 * @return Request message function code.
	 * @since 0.1.0
	 */
	inline uint8_t function_code() const { return function_code_; };

	/**
	 * Get the timeout to wait for a response in milliseconds.
	 *
	 * @return Request timeout.
	 * @since 0.1.0
	 */
	inline uint32_t timeout_ms() const { return timeout_ms_; };

	/**
	 * Get the response object.
	 *
	 * @return Corresponding response object.
	 * @since 0.1.0
	 */
	inline Response& response() const { return *response_.get(); };

private:
	const uint16_t device_; /*!< Remote device address. @since 0.1.0 */
	const uint8_t function_code_; /*!< Request message function code. @since 0.1.0 */
	const uint32_t timeout_ms_; /*!< Request timeout. @since 0.1.0 */
	const std::shared_ptr<Response> response_; /*!< Corresponding response object. @since 0.1.0 */
};

/**
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
	 * Create a new register request message (not directly useful).
	 *
	 * @param[in] device Destination device address.
	 * @param[in] function_code Function code of the request.
	 * @param[in] timeout_ms Timeout to wait for a response in milliseconds.
	 * @param[in] address Register address.
	 * @param[in] data Number of registers to read or register value to write.
	 * @param[in] response Response object.
	 * @since 0.1.0
	 */
	RegisterRequest(uint16_t device, uint8_t function_code, uint32_t timeout_ms,
		uint16_t address, uint16_t data,
		const std::shared_ptr<Response> &response);

	/**
	 * Encode this request and store it in a message frame buffer.
	 *
	 * @param[out] frame Message frame data.
	 * @return Size of message frame.
	 * @since 0.1.0
	 */
	uint16_t encode(frame_buffer_t &frame) override;

	/**
	 * Get the register address.
	 *
	 * @return Register address.
	 * @since 0.1.0
	 */
	inline uint16_t address() const { return address_; };

	/**
	 * Get the register size or value.
	 *
	 * @return Register size or value.
	 * @since 0.1.0
	 */
	inline uint16_t data() const { return data_; };

private:
	const uint16_t address_; /*!< Register address. @since 0.1.0 */
	const uint16_t data_; /*!< Register size or value. @since 0.1.0 */
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
	 * @param[in] serial Serial port device.
	 * @since 0.1.0
	 */
	SerialClient(::HardwareSerial *serial);

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
	 * The response message contains the register values returned.
	 *
	 * @param[in] device Device address (DeviceAddressTypes::MIN_UNICAST to DeviceAddressTypes::MAX_UNICAST).
	 * @param[in] address Starting address (0x0000 to 0xFFFF).
	 * @param[in] size Quantity of registers (0x0001 to 0x007D).
	 * @param[in] timeout_ms Timeout to wait for a response in milliseconds.
	 * @return A response message that will contain the outcome and data in the
	 *         future when processing is complete.
	 * @since 0.1.0
	 */
	std::shared_ptr<const RegisterDataResponse> read_holding_registers(uint16_t device,
		uint16_t address, uint16_t size, uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);

	/**
	 * Read a contiguous block of input registers from a remote device.
	 *
	 * The response message contains the register values returned.
	 *
	 * @param[in] device Device address (DeviceAddressTypes::MIN_UNICAST to DeviceAddressTypes::MAX_UNICAST).
	 * @param[in] address Starting address (0x0000 to 0xFFFF).
	 * @param[in] size Quantity of registers (0x0001 to 0x007D).
	 * @param[in] timeout_ms Timeout to wait for a response in milliseconds.
	 * @return A response message that will contain the outcome and data in the
	 *         future when processing is complete.
	 * @since 0.1.0
	 */
	std::shared_ptr<const RegisterDataResponse> read_input_registers(uint16_t device,
		uint16_t address, uint16_t size, uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);

	/**
	 * Write to a single holding register in a remote device.
	 *
	 * The response message contains the register address followed by the
	 * register value returned.
	 *
	 * @param[in] device Device address (DeviceAddressTypes::BROADCAST to DeviceAddressTypes::MAX_UNICAST).
	 * @param[in] address Register address (0x0000 to 0xFFFF).
	 * @param[in] value Register value.
	 * @param[in] timeout_ms Timeout to wait for a response in milliseconds.
	 * @return A response message that will contain the outcome and echoed data
	 *         in the future when processing is complete.
	 * @since 0.1.0
	 */
	std::shared_ptr<const RegisterWriteResponse> write_holding_register(uint16_t device,
		uint16_t address, uint16_t value, uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);

	/**
	 * Read exception status from a remote device.
	 *
	 * @param[in] device Device address (DeviceAddressTypes::MIN_UNICAST to DeviceAddressTypes::MAX_UNICAST).
	 * @param[in] timeout_ms Timeout to wait for a response in milliseconds.
	 * @return A response message that will contain the outcome and output data
	 *         in the future when processing is complete.
	 * @since 0.1.0
	 */
	std::shared_ptr<const ExceptionStatusResponse> read_exception_status(uint16_t device,
		uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);

private:
	/**
	 * Encode the request message at the top of the queue.
	 *
	 * @since 0.1.0
	 */
	void encode();

	/**
	 * Transmit the current message frame.
	 *
	 * @since 0.1.0
	 */
	void transmit();

	/**
	 * Receive a message frame.
	 *
	 * @since 0.1.0
	 */
	void receive();

	/**
	 * Finish current request and populate response.
	 *
	 * @since 0.1.0
	 */
	void complete();

	/**
	 * Log the contents of the current message frame.
	 *
	 * @param[in] prefix Message prefix ("<-" or "->").
	 * @since 0.1.0
	 */
	void log_frame(const __FlashStringHelper *prefix);

	/**
	 * Calculate CRC for the current frame;
	 *
	 * @return CRC value.
	 * @since 0.1.0
	 */
	uint16_t calc_crc() const;

	::HardwareSerial *serial_; /*!< Serial port device. @since 0.1.0 */
	std::deque<std::unique_ptr<Request>> requests_; /*!< Pending requests. @since 0.1.0 */

	frame_buffer_t frame_; /*!< Current message frame. @since 0.1.0 */
	uint16_t frame_pos_ = 0; /*!< Position in message frame. @since 0.1.0 */

	uint32_t last_rx_ms_ = 0; /*!< Time that the last character was received. @since 0.1.0 */

	uint16_t tx_frame_size_ = 0; /*!< Size of request frame to transmit. @since 0.1.0 */
	uint32_t last_tx_ms_ = 0; /*!< Time that the last character was transmitted. @since 0.1.0 */
};

} // namespace modbus

} // namespace uuid

#endif
