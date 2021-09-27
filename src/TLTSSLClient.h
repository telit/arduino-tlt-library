/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
    TLTSSLClient.h

  @brief
   TLT SSL Client class
  @details
    
  
  @version 
    1.0.0

  @note
    Dependencies:
    ME310.h
    TLTClient.h
    TLTRootCerts.h

  @author
    Cristina Desogus

  @date
    08/03/2021
*/
#ifndef __TLTSSLCLIENT__H
#define __TLTSSLCLIENT__H
/* Include files ================================================================================*/
#include "ME310.h"
#include <TLTCLient.h>
#include <TLTRootCerts.h>

/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Class definition ================================================================================*/
enum
{
  PROTOCOL_VERSION_TLS_1_2 = 3,
  PROTOCOL_VERSION_TLS_1_3 = 4
};

enum
{
  SNI_ON = 1,
  SNI_OFF = 0
};

class TLTSSLClient : public TLTClient
{
    public:

        TLTSSLClient(ME310* me310, bool synch = true);
        TLTSSLClient(ME310* me310, const TLTRootCert* myRCs, int myNumRCs, int version = PROTOCOL_VERSION_TLS_1_3, int SNI = SNI_OFF, bool synch = true);
        TLTSSLClient(ME310* me310, int version, int SNI,  bool synch = true);
        virtual ~TLTSSLClient();

        virtual int ready();
        virtual int iterateCerts();

        virtual int connect(IPAddress ip, uint16_t port);
        virtual int connect(const char* host, uint16_t port);
    
    private:

        const TLTRootCert* _RCs;
        int _numRCs;
        static bool _defaultRootCertsLoaded;
        bool _customRootCerts;
        bool _customRootCertsLoaded;
        int _certIndex;
        int _state;
        int _version;
        int _SNI;

        int moduleReady();


        ME310* _me310;
        ME310::return_t _rc;
};

#endif //__TLTSSLCLIENT__H