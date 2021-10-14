/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    - UDPNtpClient_example.ino

  @brief
    Udp NTP Client

  @details
    In this example sketch,it gets the time from a Network Time Protocol (NTP) server.\n
    The use of UDP write and parsePacket is demonstrated.\n
    For the sketch to work it is necessary to install the external TimeLib library, available in:
    https://www.arduinolibraries.info/libraries/time

  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    25/08/2021
 */


#include <TLTMDM.h>
#include <TimeLib.h>


unsigned int localPort = 2500;      // local port to listen for UDP packets

const char timeServer[] = "0.it.pool.ntp.org";
unsigned short hostPort = 123; 

const int timeZone = 2;    
time_t prevDisplay = 0; // when the digital clock was displayed

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// initialize the library instance
ME310* myME310 = new ME310();
GPRS gprs(myME310);
TLT tltAccess(myME310);
TLTUDP Udp(myME310);

char APN[] = "APN";

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  myME310->begin(115200); 
  delay(1000);
  myME310->powerOn(ON_OFF);
  delay(5000);

  Serial.println(F("Starting Arduino UDP NTP client."));
  // connection state
  boolean connected = false;

  // attach the shield to the GPRS network with the APN, login and password
  Serial.print("Begin...");
  while (!connected)
  {
    if ((tltAccess.begin(0, APN, true) == READY) && (gprs.attachGPRS() == GPRS_READY))
    {
      connected = true;
    }
    else
    {
      Serial.print(F("."));
      delay(1000);
    }
  }

  Serial.println(F("Starting connection to server..."));
  Udp.begin(localPort); 
  Serial.println(F("Waiting for sync..."));
  setSyncProvider(getNtpTime);
}

void loop()
{
  if (timeStatus() != timeNotSet)
  {
    if (now() != prevDisplay)
    { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
  exit(0);
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(year());
  Serial.print("/");
  Serial.print(month());
  Serial.print("/");
  Serial.print(day()); 
  Serial.print(" "); 
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();  
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
  {
    Serial.print('0');
  }
  Serial.print(digits);
}

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request.");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response.");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(const char* address)
{
  
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

//memcpy(packetBuffer, "Hello World", 11);

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, hostPort); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
