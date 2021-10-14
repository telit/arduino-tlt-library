/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLTGNSS.cpp

 @brief
    Management GNSS functionality using library ME310 Telit Modem.
    
  @details
    The class implements the typical functionalities of GNSS.\n
  
  @version
    1.1.0

  @note
    Dependencies:
    TLTGNSS
 
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

//! \brief Unsets GNSS Configuration
/*! \details
This method unsets GNSS configurations.
 * \return return 1 if alive, else 0.
 */
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

//! \brief Get GNSS Data information
/*! \details
This method gets GNSS data information about acquired position.
 * \return return GNSS data information.
 */
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

//! \brief Convert NMEA to Decimal
/*! \details
This method converts NMEA data to Decimal.
 * \return true if the conversion was successful, otherwise false.
 */
bool TLTGNSS::convertNMEA2Decimal(String lat, String lng, float *f_lat, float *f_lng)
{
  float tmp_f_lat = 0.0;
  float tmp_f_lng = 0.0;
  String tmp;
  int lat_sign;
  int lng_sign;

  int degrees;
  float minutes;

  if((lat.length() == 0) || (lng.length() == 0))
  {
    return false;
  }

  if(!f_lat || ! f_lng)
  {
    return false;
  }

  if( lat.endsWith("N"))
  {
    lat_sign = 1;
  }
  else if ( lat.endsWith("S"))
  {
    lat_sign = -1;
  }
  else
  {
    return false;
  }

  if( lng.endsWith("E"))
  {
    lng_sign = 1;
  }
  else if ( lng.endsWith("W"))
  {
    lng_sign = -1;
  }
  else
  {
    return false;
  }

  /*latitude */
  tmp = lat.substring(0, lat.length() -1 );

  tmp_f_lat = tmp.toFloat();
  tmp_f_lat *= lat_sign;
  degrees = trunc(tmp_f_lat / 100.0);
  minutes = tmp_f_lat - (degrees * 100.0);
  tmp_f_lat = (float)degrees + (minutes / 60.0);

  /*longitude */
  tmp = lng.substring(0, lng.length() -1 );
  tmp_f_lng = tmp.toFloat();
  tmp_f_lng *= lng_sign;
  degrees = trunc(tmp_f_lng / 100.0);
  minutes = tmp_f_lng - (degrees * 100.0);
  tmp_f_lng = (float)degrees + (minutes / 60.0);

  *f_lat = tmp_f_lat;
  *f_lng = tmp_f_lng;

  return true;
}

//! \brief Convert Decimal to DMS
/*! \details
This method converts decimal value to DMS data.
 * \return DMS data.
 */
DMS TLTGNSS::convertDecimal2DMS(float value)
{
  DMS result;
  float tmp_minutes;
  
  result.degrees = abs(trunc(value));
  tmp_minutes = (abs(value)- result.degrees) * 60.0;
  result.minutes = (int)(tmp_minutes);
  result.seconds = (tmp_minutes - result.minutes )*60;

  return result;
}
