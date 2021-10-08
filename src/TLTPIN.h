/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*!
  @file
    TLTPIN.h

  @brief
    TLT PIN class
  @details
    
  
  @version 
    1.1.0

  @note
    Dependencies:
    ME310.h
    Arduino.h

  @author
    Cristina Desogus

  @date
    08/02/2021
*/

#ifndef __TLTPIN__H
#define __TLTPIN__H
/* Include files ================================================================================*/
#include <ME310.h>
#include <Arduino.h>
/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Class definition ================================================================================*/

class TLTPIN
{
    public:

        TLTPIN(ME310* me310);
        void begin();
        int isPIN();
        int checkPIN(String pin);
        int checkPUK(String puk, String pin);
        void changePIN(String oldPIN, String newPIN);
        void switchPIN(String pin);
        int checkReg();
        bool getPINUsed();
        void setPINUsed(bool used);
    
    private:

        bool _pinUsed;
        ME310* _me310;
        ME310::return_t _rc;

};

#endif //__TLTPIN__H