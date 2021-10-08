/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*!
  @file
    TLTClient.h

  @brief
    TLT Client class
  @details
    
  
  @version 
    1.1.0

  @note
    Dependencies:
    ME310.h
    string.h
    Client.h

  @author
    Cristina Desogus

  @date
    07/28/2021
*/

#ifndef __TLTCLIENT__H
#define __TLTCLIENT__H
/* Include files ================================================================================*/
#include <ME310.h>
#include <Client.h>

/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Class definition ================================================================================*/

class TLTClient : public Client
{
    public:
        TLTClient(ME310* me310, bool synch = true);
        TLTClient(ME310* me310, int socket, bool synch);
        virtual ~TLTClient();
        virtual int ready();

        int connect(IPAddress ip, uint16_t port);
        int connectSSL(IPAddress ip, uint16_t port);
        int connect(const char *host, uint16_t port);
        int connectSSL(const char *host, uint16_t port);

        void beginWrite(bool sync = false);
        size_t write(uint8_t c);
        size_t write(const uint8_t* buf, size_t);
        size_t write(const uint8_t *buf);
        void endWrite(bool sync = false);

        uint8_t connected();

        operator bool();

        int read(uint8_t *buf, size_t size);
        int read();
        int available();
        int peek();
        void flush();
        void stop();

        virtual void handleUrc(const String& urc);

    private:
        int connect();
        int moduleReady();
        bool _synch;
        int _socket;
        int _connected;

        int _state;
        IPAddress _ip;
        const char* _host;
        uint16_t _port;
        bool _ssl;

        bool _writeSync;
        String _response;

        ME310* _me310;
        ME310::return_t _rc;

};

#endif //__TLTCLIENT__H