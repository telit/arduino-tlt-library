/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*!
  @file
    TLT.h

  @brief
    TLT class
  @details
    This class managements the most important functionality of connection function.
  
  @version 
    1.1.0

  @note
    Dependencies:
    ME310.h
    string.h

  @author
    Cristina Desogus

  @date
    07/26/2021
*/

#ifndef __TLT__H
#define __TLT__H
/* Include files ================================================================================*/
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <ME310.h>
/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Class definition ================================================================================*/
/*! \enum Network status
    \brief Network status
*/
enum TLT_NetworkStatus_t
{ 
    ERROR, 
    IDLE, 
    CONNECTING, 
    READY, 
    GPRS_READY, 
    TRANSPARENT_CONNECTED, 
    OFF
};

class TLT
{
    public:
        TLT(ME310* me310, bool debug = false);

        TLT_NetworkStatus_t begin(const char* pin = 0, bool restart = true, bool synchronous = true);
        TLT_NetworkStatus_t begin(const char* pin, const char* apn, bool restart = true, bool synchronous = true);
        TLT_NetworkStatus_t begin(const char* pin, const char* ipProt, const char* apn, const char* username, const char* password, bool restart = true, bool synchronous = true);

        int isAccessAlive();
        bool shutdown();
        bool secureShutdown();
        int ready();
        void setTimeout(unsigned long timeout);

        unsigned long getTime();
        unsigned long getLocalTime();
        bool setTime(unsigned long const epoch, int const timezone = 0);

        TLT_NetworkStatus_t getStatus();

    private:

        struct tm parse_time(String time);
        int moduleReady();
        bool checkSetPhoneFunctionality(int value);

        bool TLTRestart(bool flag);

        TLT_NetworkStatus_t _state;
        int _readyState;
        const char* _ipProt;
        const char* _pin;
        const char* _apn;
        const char* _username;
        const char* _password;
        int _baund;
        String _response;
        unsigned long _timeout;
        ME310* _me310;
        ME310::return_t _rc;
};

#endif //__TLT__H