# screen-client

![](schematic.png?raw=true)
![](shot.jpg?raw=true)

This if the firmware for a device that displays information received
from `screen-server` (not yet pushed).  Written for ATmega 328 but it
doesn't use any fancy features so it should be easy to make it work
with other AVR chips, as long as the pins are configured correctly and
the chip has SPI capabilities.

I'm currently using it to get the ETA of the bus, but will display
whatever the server sends, so could also be used for weather or other
information.

## How it works

The push button closes the circuit, making it operational for as long
as the button is pressed. During this time the chip will send a
command "HELLO" to a server every second using an NRF24L01+
transceiver. The device will then show (through an HD44780-compatible
display) the payload of the acknowledgement packet from the server.