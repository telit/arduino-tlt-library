/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    SendSMS_example.ino

  @brief
    SMS sender.

  @details
    In this example sketch, it is shown how to use SMS management, using TLTSMS library.\n
    In the loop section, it sends an SMS.

  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    25/08/2021
 */

#include <TLTMDM.h>

// initialize the library instance
ME310* myME310 = new ME310();
TLT TLTAccess(myME310);
TLTSMS sms(myME310);

void setup() {
  
  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  myME310->begin(115200);
  delay(1000);
  myME310->powerOn(ON_OFF);
  delay(5000);
  Serial.println(F("SMS Messages Sender"));

  // connection state
  bool connected = false;

  // Start connection
  Serial.print(F("Connecting..."));
  while (!connected)
  {
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
}

void loop() {

  Serial.print(F("Enter a mobile number: "));
  char remoteNum[20]; // telephone number to send SMS
  readSerial(remoteNum);
  Serial.println(remoteNum);

  // SMS text
  Serial.print("Now, enter SMS content: ");
  char txtMsg[200];
  readSerial(txtMsg);
  Serial.println("SENDING");
  Serial.println();
  Serial.println("Message:");
  Serial.println(txtMsg);

  // send the message
  sms.beginSMS(remoteNum);
  sms.print(txtMsg);
  sms.endSMS();
  Serial.println("\nCOMPLETE!\n");
  delay(5000);
  exit(0);
}

/*
  Read input serial
 */
int readSerial(char result[])
{
  int i = 0;
  while (1)
  {
    while (Serial.available() > 0)
    {
      char inChar = Serial.read();
      if (inChar == '\n')
      {
        result[i] = '\0';
        Serial.flush();
        return 0;
      }
      if (inChar != '\r')
      {
        result[i] = inChar;
        i++;
      }
    }
  }
}
