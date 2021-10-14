/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    - TLTMDM.h

  @brief
    Scan Networks.

  @details
     This example sketch prints the IMEI number of the modem, then check if it is connected to an operator.\n
     It then scans nearby networks and prints their signal strength.
  @version 
    1.0.0
  
  @note

  @author
    Cristina Desogus

  @date
    09/23/2021
 */
// libraries

// initialize the library instance
#include "TLTMDM.h"

ME310* myME310 = new ME310();

TLT TLTAccess(myME310, true);     // include a 'true' parameter to enable debugging
TLTScanner scannerNetworks(myME310);

// Save data variables
String IMEI = "";

char APN[]= "apn";

void setup() {
  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  myME310->begin(115200);
  delay(1000);
  myME310->powerOn(ON_OFF);
  delay(5000);
  Serial.println("NB IoT/LTE Cat M1 networks scanner");
  scannerNetworks.begin();

  // connection state
  boolean connected = false;

  // Start module
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while (!connected)
  {
    if (TLTAccess.begin(NULL, APN, true) == READY)
    {
      connected = true;
    }
    else
    {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  // get modem parameters
  // IMEI, modem unique identifier
  Serial.print("Modem IMEI: ");
  IMEI = TLTAccess.getIMEI();
  Serial.println(IMEI.c_str());
}

void loop() {
  // currently connected carrier
  Serial.print("Current carrier: ");
  Serial.println(scannerNetworks.getCurrentCarrier());

  // returns strength and BER
  // signal strength in 0-31 scale. 31 means power > 51dBm
  // BER is the Bit Error Rate. 0-7 scale. 99=not detectable
  Serial.print("Signal Strength: ");
  Serial.print(scannerNetworks.getSignalStrength());
  Serial.println(" [0-31]");

  // scan for existing networks, displays a list of networks
  Serial.println("Scanning available networks. This may take some seconds.");
  Serial.println(scannerNetworks.readNetworks());
  // wait ten seconds before scanning again
  delay(10000);
}
