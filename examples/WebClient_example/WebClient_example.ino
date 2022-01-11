/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    WebClient_example.ino

  @brief
    Web client

  @details
    This sketch connects to a server using sockets through the board.\n
    Specifically, this example connects to echo server "modules.telit.com" and prints the response to the Serial monitor.

  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    01/09/2021
 */
// libraries
#include <TLTMDM.h>

// initialize the library instance
ME310* myME310 = new ME310();
TLTClient client(myME310);
GPRS gprs(myME310);
TLT tltAccess(myME310);

// URL, path and port (for example: example.org)
char server[] = "modules.telit.com";
char path[] = "/";
int port = 10510; // port 80 is the default for HTTP
char APN[]="APN";

void setup() {
  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  myME310->begin(115200);
  delay(1000);
  myME310->powerOn(ON_OFF);
  delay(5000);

  Serial.println(F("Starting Arduino web client."));
  // connection state
  boolean connected = false;

  // After starting the modem with TLT.begin()
  // attach to the GPRS network with the APN, login and password
  // Start connection
  Serial.print(F("Begin..."));
  while (!connected)
  {
    if ((tltAccess.begin(0, APN, true) == READY) && (gprs.attachGPRS() == GPRS_READY))
    {
      connected = true;
      Serial.println(F(""));
    }
    else
    {
      Serial.print(F("."));
      delay(1000);
    }
  }

  Serial.println(F("connecting..."));

  // if you get a connection, report back via serial:
  if (client.connect(server, port))
  {
    Serial.println(F("connected"));
    // Make a HTTP request:
    client.println("GET / HTTP/1.1\r\nHost: telit.com\r\nConnection: close\r\n");
    Serial.println("Send is ok");
  }
  else
  {
    // if you didn't get a connection to the server:
    Serial.println(F("connection failed"));
  }
}
void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available())
  {
    Serial.print((char)client.read());
  }
  // if the server's disconnected, stop the client:
  if (!client.available() && !client.connected())
  {
    Serial.println();
    Serial.println(F("disconnecting."));
    client.stop();
    exit(0);
  }
}
