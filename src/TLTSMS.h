/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*!
  @file
    TLTSMS.h

  @brief
    Management SMS functionality using library ME310 Telit Modem
  @details
    The class implements the typical functionalities of SMS.\n
    In particular: send and read SMS.
  
  @version 
    1.1.0

  @note
    Dependencies:
    ME310.h
    string.h
    Arduino.h

  @author
    Cristina Desogus

  @date
    08/02/2021
*/

#ifndef __TLTSMS__H
#define __TLTSMS__H

/* Include files ================================================================================*/
#include <ME310.h>
#include <Arduino.h>
#include <Stream.h>
/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Define ========================================================================================*/

#define TLT_SMS_CLEAR_READ             (1)
#define TLT_SMS_CLEAR_READ_SENT        (2)
#define TLT_SMS_CLEAR_READ_SENT_UNSENT (3)
#define TLT_SMS_CLEAR_ALL              (4)
/* Class definition ================================================================================*/

/*! \class TLTSMS
    \brief Management SMS functionality
    \details 
    The class implements the SMS functionalities.\n
*/
class TLTSMS : public Stream
{
    public:
        TLTSMS(ME310* me310, bool synch = true);
        size_t write(uint8_t c);
        int setCharset(const char* charset = nullptr);
        int beginSMS(const char* to);
        int ready();
        int endSMS();
        int available();
        int remoteNumber(char* number, int nlength); 
        int read();
        int peek();
        void flush();
        void clean(int flag = TLT_SMS_CLEAR_READ_SENT);
        size_t print(const String &);
        bool setMessageFormat(int value);
        ~TLTSMS(){}
    
    private:

        int moduleReady();
        bool _synch;
        int _state;
        String _incomingBuffer;
        int _smsDataIndex;
        int _smsDataEndIndex;
        bool _smsTxActive;
        int _charset;
        char _bufferUTF8[4];
        int _indexUTF8;
        const char* _ptrUTF8;
        String _toBuffer;
        String _dataBuffer;
        ME310* _me310;
        ME310::return_t _rc;
};

#endif //__TLTSMS__H