/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*!
  @file
    TLTUdp.h

  @brief
    TLT Udp class
  @details
    
  
  @version 
    1.1.0

  @note
    Dependencies:
    ME310.h
    string.h

  @author
    Cristina Desogus

  @date
    08/02/2021
*/

#ifndef __TLTUDP__H
#define __TLTUDP__H
/* Include files ================================================================================*/
#include <ME310.h>
#include <Arduino.h>
/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Class definition ================================================================================*/
class TLTUDP : public UDP
{
    public:

        TLTUDP(ME310* me310);
        virtual ~TLTUDP();

        virtual uint8_t begin(uint16_t);

        uint8_t begin(int connID, int socket, uint16_t port);  
        virtual void stop();

        virtual int beginPacket(IPAddress ip, uint16_t port);
        virtual int beginPacket(const char *host, uint16_t port);
        
        virtual int endPacket();
        
        virtual size_t write(uint8_t);
        virtual size_t write(const uint8_t *buffer, size_t size);

        using Print::write;

        virtual int parsePacket();
        
        virtual int available();
        
        virtual int read();
        
        virtual int read(unsigned char* buffer, size_t len);
        
        virtual int read(char* buffer, size_t len) { return read((unsigned char*)buffer, len); };
        
        virtual int peek();
        virtual void flush();	
        virtual IPAddress remoteIP();
        
        virtual uint16_t remotePort();

        //virtual void handleUrc(const String& urc);

    private:
      //void ConvertBufferToIRA(uint8_t* recv_buf, uint8_t* out_buf, int size);
      int CheckData(String data);

        int _socket;
        bool _packetReceived;

        IPAddress _txIp;
        const char* _txHost;
        uint16_t _txPort;
        size_t _txSize;
        uint8_t _txBuffer[512];
        
        IPAddress _rxIp;
        uint16_t _rxPort;
        size_t _rxSize;
        size_t _rxIndex;
        uint8_t _rxBuffer[512];

        ME310* _me310;
        ME310::return_t _rc;
};

#endif //__TLTUDP__H