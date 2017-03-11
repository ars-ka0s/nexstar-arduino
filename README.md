# nexstar-arduino

Library for communicating with telescope mounts based on the Celestron NexStar 4/5SE hardware.

## Notes

This library was developed based mainly on information published by Andre Paquette in his [NexStar AUX Command Set](http://www.paquettefamily.ca/nexstar/NexStar_AUX_Commands_10.pdf) and some additional probing of the interface on the specific model I was playing with.

Not all of the commands detailed in that document are implemented here.  I did not personally have a need for features like PEC and index-finding, so those functions are not implemented because I didn't want to waste time debugging them.

The code was tested against a pair of newer GT-series mounts which internally have 4/5SE boards, thus it may not work with other models, or even similar ones of an older vintage.

## Pinout

This library is designed to be used for communicating directly to the mount, using an Aux or HC (hand control) port.  It is not directly compatible with the PC port (which requires an RS-232 level converter) or the serial port found on the bottom of hand controllers.

The pinout of the Aux/HC ports is as follows:

1. NC
2. Serial data
3. +12 VDC passthrough
4. Serial data
5. Ground
6. Select

The two serial pins are internally wired together, so either can be used.  The serial data is passed on a half-duplex line and the select line is used to avoid collisions.  The select line idles high and is driven low to start a message.  After the serial data is sent, the line should be released and will return high.  The mount will then pull the select line low to begin its reply.

## Using the library

Three selectable Arduino pins are used for communicating with the mount.  The serial and select lines are pulled high inside the mount, so only need to be driven low.  The serial data wire from the mount should go to the chosen receive pin, ground wire to ground, and select wire to the select pin.  The chosen transmit pin should be connected to the receive pin with a small signal diode (think 1N4148) so the transmit pin can pull the serial line low.

Beyond that it is as simple as creating a NexStar instance and calling functions on the azimuth or elevation controllers.  A console example for testing is in the examples folder.

## Trademarks

Celestron and NexStar are registered trademarks of Celestron Acqusition, LLC and are used solely to refer to compatible hardware.  Use of these trademarks does not express or imply any affiliation with or endorsement by the holders of said trademarks.