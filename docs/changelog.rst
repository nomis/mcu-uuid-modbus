Change log
==========

Unreleased_
-----------

0.2.0_ |--| 2022-02-10
----------------------

Support configuring the default timeout for requests.

Added
~~~~~

* Configuration of the default timeout for requests.

0.1.1_ |--| 2022-02-06
----------------------

Fix for register reads.

Fixed
~~~~~

* Handling of responses to the following Modbus functions:

  * Read Holding Registers
  * Read Input Registers

0.1.0_ |--| 2022-01-30
----------------------

First stable release.

Added
~~~~~

* Serial device client support for the following Modbus functions:

  * Read Holding Registers
  * Read Input Registers
  * Write Single Register
  * Read Exception Status

.. |--| unicode:: U+2013 .. EN DASH

.. _Unreleased: https://github.com/nomis/mcu-uuid-modbus/compare/0.2.0...HEAD
.. _0.2.0: https://github.com/nomis/mcu-uuid-modbus/compare/0.1.1...0.2.0
.. _0.1.1: https://github.com/nomis/mcu-uuid-modbus/compare/0.1.0...0.1.1
.. _0.1.0: https://github.com/nomis/mcu-uuid-modbus/commits/0.1.0
