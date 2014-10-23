Package name: 
=============
UDP

Desription: 
===========
Package contains support for UDP communication.

About:
======
- package consists of two entities:
---paiUDPService: a package that contains support for application independant UDP client and server logic
---appUDP program: a program that implements application specific UDP logic. This program utilizes services provided by the paiUDPService package.
IMPORTANT:
-updserv and udpclient tasks SHOULD be located above the udpApp task in the CPU task list. That way, custom UDP
configuration provided in the init section of the udpApp will have effect.

Dependencies:
=============
- B&R Libraries: AsUDP
- Other: bglib C utilities

Usage:
======
- UDP server configuration: modify code/settings in: paiUDPService/udpserv.c/udpservINIT()
- UDP client configuration: modify code/settings in: paiUDPService/udpclient.c/udpclientINIT()
- for application specific support, add needed code in appUDP (or elsewhere in the program)

Examples:
=============
TODO
