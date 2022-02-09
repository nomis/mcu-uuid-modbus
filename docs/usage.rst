Usage
=====

.. code:: c++

   #include <uuid/modbus.h>

Create a |uuid::modbus::SerialClient|_ with a configured ``HardwareSerial``
device and call |loop()|_ on the instance regularly.

Use the functions for `reading`_/`writing`_ registers to initiate a request and
then call |done()|_ on the returned response object to check for completion.
Progress can only be made when the |loop()|_ function is called.

Call |success()|_ to find out if communication was successful and then read
response data using |data()|_.

Example
-------

.. literalinclude:: ../examples/ReadRegisters.cpp

Output
~~~~~~

.. literalinclude:: ../examples/ReadRegisters.txt
   :language: none

.. |uuid::modbus::SerialClient| replace:: ``uuid::modbus::SerialClient``
.. _uuid::modbus::SerialClient: https://mcu-doxygen.uuid.uk/classuuid_1_1modbus_1_1SerialClient.html

.. |loop()| replace:: ``loop()``
.. _loop(): https://mcu-doxygen.uuid.uk/classuuid_1_1modbus_1_1SerialClient.html#a1799cf77470b87aa2da4cbd60600b009

.. read_holding_registers()
.. _reading: https://mcu-doxygen.uuid.uk/classuuid_1_1modbus_1_1SerialClient.html#afa0ea4afd8dc619d183d45d38c92ac78
.. write_holding_register()
.. _writing: https://mcu-doxygen.uuid.uk/classuuid_1_1modbus_1_1SerialClient.html#af22be2c872362e09987d3f50e9d3d630

.. |done()| replace:: ``done()``
.. _done(): https://mcu-doxygen.uuid.uk/classuuid_1_1modbus_1_1Response.html#a8b5e2e5189768c45fe5ddf85f5eee48b

.. |success()| replace:: ``success()``
.. _success(): https://mcu-doxygen.uuid.uk/classuuid_1_1modbus_1_1Response.html#a1a3988f01b56c03b8b94855c57f42967

.. |data()| replace:: ``data()``
.. _data(): https://mcu-doxygen.uuid.uk/classuuid_1_1modbus_1_1RegisterDataResponse.html#a2ebc9fe3d1d470235b715f8489c10662
