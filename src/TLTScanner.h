/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
    TLTScanner.h

  @brief
    TLTScanner class
  @details
    
  
  @version 
    1.0.0

  @note
    Dependencies:
    ME310.h
    TLT.h

  @author
    Cristina Desogus

  @date
    08/02/2021
*/

#ifndef __TLTSCANNER__H
#define __TLTSCANNER__H

/* Include files ================================================================================*/
#include <ME310.h>
#include <TLT.h>

/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Class definition ================================================================================*/

class TLTScanner
{
    public:
        TLTScanner(ME310* me310, bool trace = false);
        TLT_NetworkStatus_t begin();
        String getCurrentCarrier();
        String getSignalStrength();
        String readNetworks();
    private:

      ME310* _me310;
      ME310::return_t _rc;

};

#endif //__TLTSCANNER__H