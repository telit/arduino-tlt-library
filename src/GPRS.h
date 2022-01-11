/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*!
  @file
    GPRS.h

  @brief
   GPRS class
  @details
    
  
  @version 
    1.3.0

  @note
    Dependencies:
    ME310.h
    IPAddress.h
    TLT.h
  @author
    Cristina Desogus

  @date
    08/03/2021
*/
#ifndef __GPRS__H
#define __GPRS__H
/* Include files ================================================================================*/
#include <IPAddress.h>
#include <ME310.h>
#include <TLT.h>

/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Class definition ================================================================================*/

class GPRS
{
    public:

        GPRS(ME310* me310, bool debug = false);
        virtual ~GPRS();

        TLT_NetworkStatus_t networkAttach();
        TLT_NetworkStatus_t networkDetach();

        int ready();
        TLT_NetworkStatus_t attachGPRS(bool synchronous = true);
        TLT_NetworkStatus_t detachGPRS(bool synchronous = true);
        IPAddress getIPAddress();
        void setTimeout(unsigned long timeout);
        TLT_NetworkStatus_t status();

        bool getDebug();
        void setDebug(bool debug);

        int getReadyState();
        void printReadyState();

    private:

        int moduleReady();
        int _state;
        TLT_NetworkStatus_t _status;
        String _response;
        int _pingResult;
        unsigned long _timeout;
        bool _debug;

        ME310* _me310;
        ME310::return_t _rc;
};

#endif //__GPRS__H