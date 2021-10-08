/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLTGNSS.cpp

  @brief


  @details


  @version
    1.0.0

  @note
    Dependencies:
    TLTGNSS
  @author


  @author
    Cristina Desogus

  @date
    09/30/2021
*/

#include <TLTGNSS.h>
//!\brief Class Constructor
/*! \details
Constructor
 * \param me310 pointer of ME310
 * \param nmea NMEA flag
 */
TLTGNSS::TLTGNSS(ME310* me310, bool nmea) : _nmea(nmea)
{
    _me310 = me310;
}

//! \brief Sets GNSS Configuration
/*! \details
This method sets GNSS configurations.
 * \return return 1 if alive, else 0.
 */
bool TLTGNSS::setGNSSConfiguration(void)
{
  _me310->read_gnss_configuration();
  _rc = _me310->gnss_configuration(2,1);
  if(_rc == ME310::RETURN_VALID)
  {
    _me310->module_reboot();
  }

  delay(10000);
  _rc = _me310->gnss_configuration(3,0);
  if(_rc == ME310::RETURN_VALID)
  {
    _me310->read_gnss_configuration();
    delay(5000);
    _rc = _me310->gnss_controller_power_management(1);

    if( _rc == ME310::RETURN_VALID )
    {
      if (_nmea)
      {
        _rc = _me310->gnss_nmea_extended_data_configuration(0,1,1,0,0,0,0,0,0,0,0,1,0);

        if(_rc ==  ME310::RETURN_VALID)
        {
          _rc = _me310->gnss_nmea_data_configuration(2,1,1,1,1,1,1);
          if(_rc ==  ME310::RETURN_VALID)
          {
            return true;
          }
          else
          {
            return false;
          }
        }
      }
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool TLTGNSS::unsetGNSSConfiguration()
{
  _rc = _me310->gnss_controller_power_management(0);
  if(_rc == ME310::RETURN_VALID)
  {
    _rc = _me310->gnss_configuration(3,1);
    delay(5000);
    return true;
  }
  else
  {
    return false;
  }
}


GNSSInfo TLTGNSS::getGNSSData()
{
  GNSSInfo tmpGNSSData;

  int i = 0;
  String tmpData;
  _rc = _me310->gps_get_acquired_position();
  if(_rc == ME310::RETURN_VALID)
  {
    while(_me310->buffer_cstr(i) != NULL)
    {
      tmpData = (char*)_me310->buffer_cstr(i);
      if(tmpData.startsWith("$GPSACP:"))
      {
        int pos1 = tmpData.indexOf(":");
        int pos2 = tmpData.indexOf(",", pos1 + 1);
        tmpGNSSData.utc = tmpData.substring(pos1, pos2);

        pos1 = tmpData.indexOf(",", pos2 + 1);
        tmpGNSSData.latitude = tmpData.substring(pos2+1, pos1);

        pos2 = tmpData.indexOf(",", pos1 + 1);
        tmpGNSSData.longitude = tmpData.substring(pos1+1, pos2);

        pos1 = tmpData.indexOf(",", pos2 + 1);
        tmpGNSSData.hdop = tmpData.substring(pos2+1, pos1);

        pos2 = tmpData.indexOf(",", pos1 + 1);
        tmpGNSSData.altitude = tmpData.substring(pos1+1, pos2);

        pos1 = tmpData.indexOf(",", pos2 + 1);
        tmpGNSSData.fix = tmpData.substring(pos2+1, pos1);

        pos2 = tmpData.indexOf(",", pos1 + 1);
        tmpGNSSData.cog = tmpData.substring(pos1+1, pos2);

        pos1 = tmpData.indexOf(",", pos2 + 1);
        tmpGNSSData.spkm = tmpData.substring(pos2+1, pos1);

        pos2 = tmpData.indexOf(",", pos1 + 1);
        tmpGNSSData.spkn = tmpData.substring(pos1+1, pos2);

        pos1 = tmpData.indexOf(",", pos2 + 1);
        tmpGNSSData.date = tmpData.substring(pos2+1, pos1);

        tmpGNSSData.num_sat = tmpData.substring(pos1 + 1);
        break;
      }
      i++;
    }
  }
  return tmpGNSSData;
}