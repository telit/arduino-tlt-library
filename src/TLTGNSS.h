/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
    TLTGNSS.h

  @brief
    Management GNSS functionality using library ME310 Telit Modem
  @details
    The class implements the typical functionalities of GNSS.\n
  
  @version 
    1.0.0

  @note
    Dependencies:
    ME310.h
    string.h
    Arduino.h

  @author
    Cristina Desogus

  @date
    09/30/2021
*/

#ifndef __TLTGNSS__H
#define __TLTGNSS__H

/* Include files ================================================================================*/
#include "ME310.h"
#include <Arduino.h>
#include <Stream.h>
/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Define ========================================================================================*/

/* Class definition ================================================================================*/

/*! \struct GNSS Information
    \brief GNSS information

*/
struct GNSSInfo
{
  String utc;
  String latitude;
  String longitude;
  String hdop;
  String altitude;
  String fix;
  String cog;
  String spkm;
  String spkn;
  String date;
  String num_sat;
};

/*! \class TLTGNSS
    \brief Management GNSS functionality
    \details 
    The class implements the GNSS functionalities.\n
*/
class TLTGNSS
{
  public:
      TLTGNSS(ME310* me310, bool nmea = false);        
      ~TLTGNSS(){}
      bool setGNSSConfiguration();
      bool unsetGNSSConfiguration();
      GNSSInfo getGNSSData();
  
  private:

      bool _nmea;
      ME310* _me310;
      ME310::return_t _rc;
};

#endif //__TLTGNSS__H