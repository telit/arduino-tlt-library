/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    - TLTMDM.h

  @brief
    Example of test on the use of the GNSS class of the TLT library.

  @details
    In this example sketch, it is shown how to use GNSS management using GNSS class of TLT library.\n
    GNSS configuration, GNSS controller power management, GNSS nmea configuration functions are shown.\n
    GPS positions are acquired and response is printed.
	NOTE:\n
	For the sketch to work correctly, GNSS should be tested in open sky conditions to allow a fix. The fix may take a few minutes.


  @version 
    1.0.0
  
  @note

  @author
    Fabio Pintus

  @date
    10/14/2021
 */


#include <TLTMDM.h>

ME310 *_me310 = new ME310();

/*When NMEA_DEBUG is false Unsolicited NMEA is disabled*/
/*NMEA is true*/
TLTGNSS gnss(_me310, true);

void setup()
{

  Serial.begin(115200);
  _me310->begin(115200);
  delay(3000);
  
  Serial.println("TLT GNSS example, enabling ME310 module");

  _me310->powerOn();
  delay(5000);
  Serial.print("Initializing GNSS");
  while (!gnss.setGNSSConfiguration())
  {
    Serial.print(".");
  }
  Serial.println(" is completed successfully");
}

void loop()
{
  DMS lat_dms, lng_dms;
  float lat, lng;
  GNSSInfo gnssInfo = gnss.getGNSSData();
  /*gnssInfo fields will have latitude, longitude and the other details in string format*/

  /*Fix 1.2 or 1.3 means valid fix*/
  if (gnssInfo.fix.toFloat() > 1.0)
  { 
    Serial.println("");
    Serial.print("Fix valid, converting...");
    if (gnss.convertNMEA2Decimal(gnssInfo.latitude, gnssInfo.longitude, &lat, &lng))
    {
      
      Serial.println("Conversion done!");
      Serial.println(lat, 6);
      Serial.println(lng, 6);

      lat_dms = gnss.convertDecimal2DMS(lat);
      lng_dms = gnss.convertDecimal2DMS(lng);

      Serial.println("");
      Serial.println("DMS coordinates: ");
      if (lat > 0)
      {
        Serial.print("N ");
      }
      else
      {
        Serial.print("S ");
      }
      Serial.print(lat_dms.degrees);
      Serial.print("° ");
      Serial.print(lat_dms.minutes);
      Serial.print("' ");
      Serial.print(lat_dms.seconds);
      Serial.println("\"");

      if (lng > 0)
      {
        Serial.print("E ");
      }
      else
      {
        Serial.print("W ");
      }
      Serial.print(lng_dms.degrees);
      Serial.print("° ");
      Serial.print(lng_dms.minutes);
      Serial.print("' ");
      Serial.print(lng_dms.seconds);
      Serial.println("\"");
    }
    else
    {
      Serial.println("Conversion failed!");
    }
  }
  else
  {
    Serial.println("Fix not valid yet.");
    Serial.print("Fix value: ");
    Serial.println(gnssInfo.fix.toFloat());
  }
  delay(10000);
}
