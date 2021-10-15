# TLT Library for Arduino

This library allows to use a Telit Charlie board to connect to the Internet over a NBIoT, LTE Cat M1 or 2G network(if fallback is available).

It is a loose porting effort of https://github.com/arduino-libraries/MKRNB Arduino library, providing the same interfaces when possible.

This library requires the ME310 Arduino Library **greater or equal than 2.6.0** version to work.


## Download and install the ME310 Library

Download the ME310 Arduino library from https://github.com/telit/arduino-me310-library, and place the folder in your Arduino libraries folder, or install the new library from the ZIP file.


## Contents

This Library will simplify the interactions with the ME310G1 Module.

### Classes

The library provides the following classes:

 - **TLT**:  _modem related operations (turn off, check status, enable connectivity, etc. )_
 - **TLTSMS**: _helper for SMS operations_
 - **GPRS**: _GPRS attach utilities_
 - **TLTClient**: _Client to exchange data over TCP/IP_
 - **TLTScanner**: _Utilities to analyze the cellular network such as carrier info, signal strength, etc._
 - **TLTPIN**: _Utilities for the SIM PIN management_
 - **TLTSSLClient**: _TLS/SSL client to exchange data in secure mode_
 - **TLTUDP**: _UDP client utilities_
 - **TLTFileUtils**: _Modem filesystem management_
 - **TLTGNSS**: _GNSS configuration and data management/conversion_


### Examples

The following examples are available:

 - **[ChooseRadioAccessTechnology_example](examples/ChooseRadioAccessTechnology_example/ChooseRadioAccessTechnology_example.ino)** : _Select a network operator and register the module via the ME310 library. with the preferred technology_
 - **[GPRS_example](examples/GPRS_example/GPRS_example.ino)** : _sest the device connectivity trying to communicate with a HTTP server_
 - **[PinManagement_example](examples/PinManagement_example/PinManagement_example.ino)** : _Insert or disable the SIM PIN_
 - **[ReceiveSMS_example](examples/ReceiveSMS_example/ReceiveSMS_example.ino)** : _SMS management, loop to receive an SMS message_
 - **[SendSMS_example](examples/SendSMS_example/SendSMS_example.ino)** : _SMS management, how to send SMS messages_
 - **[ScanNetworks_example](examples/ScanNetworks_example/ScanNetworks_example.ino)** : _Scan nearby network cells and provide info_
 - **[SSLWebClient_example](examples/SSLWebClient_example/SSLWebClient_example.ino)** : _Connect to a website using SSL_
 - **[TLTGNSS_example](examples/TLTGNSS_example/TLTGNSS_example.ino)** : _Configure the module in GNSS priority and then waits a fix, printing the retrieved coordinates (in decimal and DMS formats)_
 - **[UDPNtpClient_example](examples/UDPNtpClient_example/UDPNtpClient_example.ino)** : _UDP client used to retrieve NTP time_
 - **[WebClient_example](examples/WebClient_example/WebClient_example.ino)** : _Connects to an echo server and exchanges data_
 - **[FileUtils_example](examples/FileUtils_example/FileUtils_example.ino)** : _Shows how to perform file related operations_


## Support

If you need support, please open a ticket to our technical support by sending an email to:

 - ts-americas@telit.com if you are in the Americas region
 - ts-emea@telit.com if you are in EMEA region
 - ts-apac@telit.com if you are in APAC

 providing the following information:

 - module type
 - answer to the commands (you can use the ME310 library TrasparentBridge example to communicate with the modem)
   - AT#SWPKGV
   - AT+CPIN?
   - AT+CCID
   - AT+CGSN
   - AT+CGDCONT?

and add [Charlie][AppZone] in the e-mail object, and in the e-mail body refer to the opened issue on github.
