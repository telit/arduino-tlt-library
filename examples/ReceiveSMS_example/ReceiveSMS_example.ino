/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ReceiveSMS_example.ino

  @brief
    SMS receiver

  @details
    In this example sketch, it is shown how to use SMS management, using TLTSMS library.\n
    In the loop section, it waits for receipt of an SMS.

  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    25/08/2021
 */


#include "TLTMDM.h"

// initialize the library instances
ME310* myME310 = new ME310();

TLT TLTAccess(myME310);
TLTSMS sms(myME310);

// Array to hold the number an SMS is retrieved from
char senderNumber[20];

void setup() {

  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(ON_OFF, LOW);
  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  myME310->begin(115200);
  delay(2000);
  Serial.println(F("SMS Messages Receiver"));
  // connection state
  bool connected = false;
  // Start connection
  Serial.print(F("Connecting..."));
  while (!connected) {
    if (TLTAccess.begin() == READY)
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
  Serial.println(F("TLT initialized"));
  Serial.println(F("Waiting for messages"));
}

void loop() {
  int c;
  // If there are any SMSs available()
  if (sms.available())
  {
    Serial.println(F("Message received from: "));

    // Get remote number
    sms.remoteNumber(senderNumber, 20);
    Serial.println(senderNumber);
    // An example of message disposal
    // Any messages starting with # should be discarded
    if (sms.peek() == '#')
    {
      Serial.println(F("Discarded SMS"));
      sms.flush();
    }
    // Read message bytes and print them
    Serial.print(F("Message text: "));
    while ((c = sms.read()) != -1)
    {
      Serial.print((char)c);
    }
    Serial.println(F("\nEND OF MESSAGE"));
    // Delete message from modem memory
    sms.flush();
    Serial.println(F("MESSAGE DELETED"));
    exit(0);
  }
  delay(1000);
}
