/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLT.cpp

  @brief
   

  @details
     

  @version 
    1.0.0
  
  @note
    Dependencies:
    ME310.h
    TLT.h
    Modem.h

  @author
    

  @date
    07/26/2021
*/

#include <string>
#include <TLT.h>
#include <time.h>

using namespace me310;
/*! \enum Ready status
  \brief Enum of ready status of internal state machine
*/
enum
{
    READY_STATE_SET_ERROR_DISABLED,
    READY_STATE_WAIT_SET_ERROR_DISABLED,
    READY_STATE_SET_MINIMUM_FUNCTIONALITY_MODE,
    READY_STATE_WAIT_SET_MINIMUM_FUNCTIONALITY_MODE,
    READY_STATE_CHECK_SIM,
    READY_STATE_WAIT_CHECK_SIM_RESPONSE,
    READY_STATE_UNLOCK_SIM,
    READY_STATE_WAIT_UNLOCK_SIM_RESPONSE,
    READY_STATE_DETACH_DATA,
    READY_STATE_WAIT_DETACH_DATA,
    READY_STATE_SET_PREFERRED_MESSAGE_FORMAT,
    READY_STATE_WAIT_SET_PREFERRED_MESSAGE_FORMAT_RESPONSE,
    READY_STATE_SET_HEX_MODE,
    READY_STATE_WAIT_SET_HEX_MODE_RESPONSE,
    READY_STATE_SET_AUTOMATIC_TIME_ZONE,
    READY_STATE_WAIT_SET_AUTOMATIC_TIME_ZONE_RESPONSE,
    READY_STATE_SET_APN,
    READY_STATE_WAIT_SET_APN,
    READY_STATE_SET_APN_AUTH,
    READY_STATE_WAIT_SET_APN_AUTH,
    READY_STATE_SET_FULL_FUNCTIONALITY_MODE,
    READY_STATE_WAIT_SET_FULL_FUNCTIONALITY_MODE,
    READY_STATE_CHECK_REGISTRATION,
    READY_STATE_WAIT_CHECK_REGISTRATION_RESPONSE,
    READY_STATE_CHECK_CONTEXT_ACTIVATION,
    READY_STATE_WAIT_CHECK_CONTEXT_ACTIVATION,
    READY_STATE_DONE
};


//! \brief Class Constructor
/*!
 * \param me310 pointer of ME310 class
 * \param debug determines debug mode.
 */
TLT::TLT(ME310* me310, bool debug) : _state(ERROR), _readyState(0), _pin(NULL), _apn(""), _username(""), _password(""), _timeout(0)
{
    _me310 = me310;
}

//! \brief Begin the modem.
/*! \details
Start the modem, attaching to the network
 * \param pin SIM PIN number, if is NULL the SIM has not configured with PIN
 * \param restart restart the modem, if is TRUE (default)
 * \param synchronous synchronous the modem, if is TRUE (default) return after the start is complete, else returns immediately.
 * \return return network status.
 */
TLT_NetworkStatus_t TLT::begin(const char* pin, bool restart, bool synchronous)
{
    return begin(pin, "", restart, synchronous);
}

//! \brief Begin the modem.
/*! \details
Start the modem, attaching to the network
 * \param pin SIM PIN number, if is NULL the SIM has not configured with PIN
 * \param apn (Access Point Name) a string parameter which is a logical name
 * \param restart restart the modem, if is TRUE (default)
 * \param synchronous synchronous the modem, if is TRUE (default) return after the start is complete, else returns immediately.
 * \return return network status.
 */
TLT_NetworkStatus_t TLT::begin(const char* pin, const char* apn, bool restart, bool synchronous)
{
    return begin(pin, "IP", apn, "", "", restart, synchronous);
}

//! \brief Begin the modem.
/*! \details
Start the modem, attaching to the network
 * \param pin SIM PIN number, if is NULL the SIM has not configured with PIN
 * \param ipProt (Packet Data Protocol type) a string parameter which specifies the type of packet data protocol
 * \param apn (Access Point Name) a string parameter which is a logical name
 * \param username User name for access to the IP network
 * \param password    Password for access to the IP network
 * \param restart restart the modem, if is TRUE (default)
 * \param synchronous synchronous the modem, if is TRUE (default) return after the start is complete, else returns immediately.
 * \return return network status.
 */
TLT_NetworkStatus_t TLT::begin(const char* pin, const char* ipProt, const char* apn, const char* username, const char* password, bool restart, bool synchronous)
{
    if(!TLTRestart(restart))
    {
        _state = ERROR;
    }
    else 
    {
        _pin = pin;
        _ipProt = ipProt;
        _apn = apn;
        _username = username,
        _password = password;
        _state = IDLE;
        _readyState = READY_STATE_SET_ERROR_DISABLED;
        restart = false;

        if (synchronous) 
        {
            unsigned long start = millis();

            while (ready() == 0) 
            {
                if (_timeout && !((millis() - start) < _timeout)) 
                {
                    _state = ERROR;
                    break;
                }
                delay(100);
            }
        }
        else 
        {
            return (TLT_NetworkStatus_t)0;
        }
    }
    return _state;
}

//! \brief Checks network access status
/*! \details
This method checks network access status calling gprs_network_registration_status() method in read mode.
 * \return return 1 if alive, else 0.
 */
int TLT::isAccessAlive()
{
  String response;
  int i = 0;
  _rc = _me310->read_gprs_network_registration_status();
  if(_rc == ME310::RETURN_VALID)
  {
      while(_me310->buffer_cstr(i) != NULL)
      {
        if((strcmp(_me310->buffer_cstr(i), "+CREG: 0,1")!= 0) && (strcmp(_me310->buffer_cstr(i), "+CREG: 0,5")!= 0))
        {
            return 1;
        }
        i++;
      }
  }  
  return 0;
}

//! \brief Shutdown the modem
/*! \details
This method calls software_shutdown() method to power off the module.
 * \return return true if successful, else false.
 */
bool TLT::shutdown()
{
    _rc = _me310->software_shutdown();
    if(_rc == ME310::RETURN_VALID)
    {
        return true;
    }
    return false;
}

//! \brief Shutdown the modem
/*! \details
This method calls software_shutdown() method to power off the module and sets state in OFF.
 * \return always return true
 */
bool TLT::secureShutdown()
{
    _me310->software_shutdown();
    _state = OFF;
    return true;
}

//! \brief Check last command status
/*! \details
This method checks the status and call the specific command.
 * \return returns 0 if last command is still executing, 1 if last command is executed successful, >1 if last
 command is executed with error.
 */
int TLT::ready()
{
    if (_state == ERROR)
    {
        return 2;
    }
    int ready = moduleReady();
    if (ready == 0) 
    {
        return 0;
    }
    switch (_readyState)
    {
        case READY_STATE_SET_ERROR_DISABLED:
        {
            _me310->report_mobile_equipment_error(2);
            _readyState = READY_STATE_WAIT_SET_ERROR_DISABLED;
            ready = 0;
            break;
        }
        case READY_STATE_WAIT_SET_ERROR_DISABLED:
        {
            if (ready > 1) 
            {
                _state = ERROR;
                ready = 2;
            } 
            else 
            {
                _readyState = READY_STATE_SET_MINIMUM_FUNCTIONALITY_MODE;
                ready = 0;
            }
            break;
        }
        case READY_STATE_SET_MINIMUM_FUNCTIONALITY_MODE:
        {
            if(checkSetPhoneFunctionality(0))
            {
                _readyState = READY_STATE_WAIT_SET_MINIMUM_FUNCTIONALITY_MODE;
                ready = 0;
                break;
            }
            else
            {
                _me310->set_phone_functionality(0);
                delay(5000);
                _readyState = READY_STATE_WAIT_SET_MINIMUM_FUNCTIONALITY_MODE;
                ready = 0;
                break;
            }
        }
        case READY_STATE_WAIT_SET_MINIMUM_FUNCTIONALITY_MODE:
        {
            if (ready > 1) 
            {
                _state = ERROR;
                ready = 2;
            }
            else 
            {
                _readyState = READY_STATE_CHECK_SIM;
                ready = 0;
            }
            break;
        }
        case READY_STATE_CHECK_SIM:
        {
            _me310->read_enter_pin();
            if(strcmp(_me310->buffer_cstr(2), "OK") == 0)
            {
                _response = _me310->buffer_cstr(1);
            }
            _readyState = READY_STATE_WAIT_CHECK_SIM_RESPONSE;
            ready = 0;
            break;
        }
        case READY_STATE_WAIT_CHECK_SIM_RESPONSE:
        {
            if (ready > 1)
            {
                _readyState = READY_STATE_CHECK_SIM;
                ready = 0;
            } 
            else 
            {
                if (_response.endsWith("READY"))
                {
                    _readyState = READY_STATE_SET_PREFERRED_MESSAGE_FORMAT;
                    ready = 0;
                }
                else if (_response.endsWith("SIM PIN"))
                {
                    _readyState = READY_STATE_UNLOCK_SIM;
                    ready = 0;
                } 
                else
                {
                    _state = ERROR;
                    ready = 2;
                }
            }
            break;
        }
        case READY_STATE_UNLOCK_SIM:
        {
            if (_pin != NULL)
            {
                _me310->enter_pin(_pin);
                _response = _me310->buffer_cstr(1);
                _readyState = READY_STATE_WAIT_UNLOCK_SIM_RESPONSE;
                ready = 0;
            }
            else
            {
                _state = ERROR;
                ready = 2;
            }
            break;
        }
        case READY_STATE_WAIT_UNLOCK_SIM_RESPONSE:
        {
            if (ready > 1)
            {
                _state = ERROR;
                ready = 2;
            } 
            else
            {
                _readyState = READY_STATE_DETACH_DATA;
                ready = 0;
            }
            break;
        }
        case READY_STATE_DETACH_DATA:
        {
            _me310->ps_attach_detach(0);
            _readyState = READY_STATE_WAIT_DETACH_DATA;
            ready = 0;
            break;
        }
        case READY_STATE_WAIT_DETACH_DATA:
        {
            if (ready > 1)
            {
                _state = ERROR;
                ready = 2;
            } 
            else
            {
                _readyState = READY_STATE_SET_PREFERRED_MESSAGE_FORMAT;
                ready = 0;
            }
            break;
        }
        case READY_STATE_SET_PREFERRED_MESSAGE_FORMAT:
        {
            _me310->message_format(1);
            _readyState = READY_STATE_WAIT_SET_PREFERRED_MESSAGE_FORMAT_RESPONSE;
            ready = 0;
            break;
        }
        case READY_STATE_WAIT_SET_PREFERRED_MESSAGE_FORMAT_RESPONSE:
        {
            if (ready > 1)
            {
                _state = ERROR;
                ready = 2;
            }
            else
            {
                _readyState = READY_STATE_SET_AUTOMATIC_TIME_ZONE;
                ready = 0;
            }
            break;
        }
        case READY_STATE_SET_AUTOMATIC_TIME_ZONE:
        {
            _me310->automatic_time_zone_update(1);
            _readyState = READY_STATE_WAIT_SET_AUTOMATIC_TIME_ZONE_RESPONSE;
            ready = 0;
            break;
        }
        case READY_STATE_WAIT_SET_AUTOMATIC_TIME_ZONE_RESPONSE:
        {
            if (ready > 1)
            {
                _state = ERROR;
                ready = 2;
            } 
            else
            {
                _readyState = READY_STATE_SET_APN;
                ready = 0;
            }
            break;
        }
        case READY_STATE_SET_APN:
        {
            _me310->define_pdp_context(1, _ipProt, _apn);
            _readyState = READY_STATE_WAIT_SET_APN;
            ready = 0;
            break;
        }
        case READY_STATE_WAIT_SET_APN:
        {
            if (ready > 1)
            {
                _state = ERROR;
                ready = 2;
            }
            else
            {
                _readyState = READY_STATE_SET_APN_AUTH;
                ready = 0;
            }
            break;
        }
        case READY_STATE_SET_APN_AUTH:
        {
            if (strlen(_username) > 0 || strlen(_password) > 0)
            {
                _me310->define_pdp_context_auth_params(1,2,_username, _password);
            }
            else
            {
                _me310->define_pdp_context_auth_params(1,0,"","");
            }
            _readyState = READY_STATE_WAIT_SET_APN_AUTH;
            ready = 0;
            break;
        }
        case READY_STATE_WAIT_SET_APN_AUTH:
        {
            if (ready > 1)
            {
                _state = ERROR;
                ready = 2;
            }
            else
            {
                _readyState = READY_STATE_SET_FULL_FUNCTIONALITY_MODE;
                ready = 0;
            }
            break;
        }
        case READY_STATE_SET_FULL_FUNCTIONALITY_MODE:
        {
            if(checkSetPhoneFunctionality(0))
            {
                _readyState = READY_STATE_WAIT_SET_MINIMUM_FUNCTIONALITY_MODE;
                ready = 0;
                break;
            }
            else
            {
                _me310->set_phone_functionality(1);
                _readyState = READY_STATE_WAIT_SET_FULL_FUNCTIONALITY_MODE;
                delay(5000);
                ready = 0;
                break;
            }
        }
        case READY_STATE_WAIT_SET_FULL_FUNCTIONALITY_MODE:
        {
            if (ready > 1)
            {
                _state = ERROR;
                ready = 2;
            }
            else
            {
                _readyState = READY_STATE_CHECK_REGISTRATION;
                ready = 0;
            }
            break;
        }
        case READY_STATE_CHECK_REGISTRATION:
        {
            _me310->read_operator_selection();
            _response = _me310->buffer_cstr(1);
            _readyState = READY_STATE_WAIT_CHECK_REGISTRATION_RESPONSE;
            ready = 0;
            break;
        }
        case READY_STATE_WAIT_CHECK_REGISTRATION_RESPONSE:
        {
            if (ready > 1)
            {
                _state = ERROR;
                ready = 2;
            }
            else
            {
                if(_response.endsWith("0"))
                {
                    _me310->read_gprs_network_registration_status();
                    _response = _me310->buffer_cstr(1);
                    if(_response.endsWith("1") || _response.endsWith("5"))
                    {
                        _readyState = READY_STATE_CHECK_CONTEXT_ACTIVATION;
                        ready = 0;
                    }
                    else if (_response.endsWith("0") || _response.endsWith("4"))
                    {
                        _readyState = READY_STATE_CHECK_REGISTRATION;
                        ready = 0;
                    }
                    else if(_response.endsWith("2"))
                    {
                        _readyState = READY_STATE_CHECK_REGISTRATION;
                        _state = CONNECTING;
                        ready = 0;
                    }
                    else if (_response.endsWith("3"))
                    {
                        _state = ERROR;
                        ready = 2;
                    }
                }
                else if(_response.endsWith("8") || _response.endsWith("9"))
                {
                    _me310->read_eps_network_registration_status();
                    _response = _me310->buffer_cstr(1);
                    int status = _response.charAt(_response.length() - 1) - '0';
                    if (status == 0 || status == 4)
                    {
                        _readyState = READY_STATE_CHECK_REGISTRATION;
                        ready = 0;
                    }
                    else if (status == 1 || status == 5 || status == 8)
                    {
                        _readyState = READY_STATE_CHECK_CONTEXT_ACTIVATION;
                        ready = 0;
                    }
                    else if (status == 2)
                    {
                        _readyState = READY_STATE_CHECK_REGISTRATION;
                        _state = CONNECTING;
                        ready = 0;
                    } 
                    else if (status == 3)
                    {
                        _state = ERROR;
                        ready = 2;
                    }
                }
            }
            break;
        }
        case READY_STATE_CHECK_CONTEXT_ACTIVATION:
        {
            _me310->context_activation(1,1);
            _response = _me310->buffer_cstr(1);
            _readyState = READY_STATE_WAIT_CHECK_CONTEXT_ACTIVATION;
            ready = 0;
            break;
        }
        case READY_STATE_WAIT_CHECK_CONTEXT_ACTIVATION:
        {
            if (ready > 1)
            {
                _state = ERROR;
                ready = 2;
            }
            else
            {
                _readyState = READY_STATE_DONE;
                _state = READY;
                ready = 1;
            }
        }
        case READY_STATE_DONE:
            break;
    }
    return ready;
}

//! \brief Sets timeout
/*! \details
This method sets timeout
 */
void TLT::setTimeout(unsigned long timeout)
{
  _timeout = timeout;
}

//! \brief Get time
/*! \details
This method calls clock_management() method in read mode, to read real-time clock of the module
 * \return returns real-time clock of the module
 */
unsigned long TLT::getTime()
{
    String response;
    _rc = _me310->read_clock_management();
    if(_rc != ME310::RETURN_VALID)
    {
        return 0;
    }
    response = _me310->buffer_cstr(1);
    struct tm now;

    int dashIndex = response.lastIndexOf('-');
    if (dashIndex != -1)
    {
        response.remove(dashIndex);
    }
    now = parse_time(response);
    if (now.tm_year != 0)
    {
        time_t result = mktime(&now);
        time_t delta = ((response.charAt(26) - '0') * 10 + (response.charAt(27) - '0')) * (15 * 60);

        if (response.charAt(25) == '-')
        {
            result += delta;
        }
        else if (response.charAt(25) == '+')
        {
            result -= delta;
        }
        return result;
    }
    return 0;
}

//! \brief Get local time
/*! \details
This method calls clock_management() method in read mode, to read real-time clock of the module
 * \return returns real-time clock of the module
 */
unsigned long TLT::getLocalTime()
{
    String response;
    _rc = _me310->read_clock_management();
    if(_rc != ME310::RETURN_VALID)
    {
        return 0;
    }
    response = _me310->buffer_cstr(1);

    struct tm now;

    now = parse_time(response);
    if (now.tm_year != 0)
    {
        time_t result = mktime(&now);
        return result;
    }
    return 0;
}

//! \brief Set time
/*! \details
This method calls clock_management() method to set real-time clock of the module
 * \return returns true if successful, else false
 */
bool TLT::setTime(unsigned long const epoch, int const timezone)
{
    String hours, date, completeData;
    const uint8_t daysInMonth [] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30 };
    unsigned long unix_time = epoch - 946684800UL; /* Subtract seconds from 1970 to 2000 */

    if (((unix_time  % 86400L) / 3600) < 10 )
    {
        hours = "0";
    }

    hours += String((unix_time  % 86400L) / 3600) + ":";
    if ( ((unix_time  % 3600) / 60) < 10 )
    {
        hours = "0";
    }

    hours += String((unix_time  % 3600) / 60) + ":";
    if ((unix_time % 60) < 10 )
    {
        hours += "0";
    }

    hours += String(unix_time % 60)+ "+";
    if (timezone < 10)
    {
        hours += "0";
    }

    hours += String(timezone);
    int days = unix_time / (24 * 3600);
    int leap;
    int year = 0;
    while (1)
    {
        leap = year % 4 == 0;
        if (days < 365 + leap)
        {
            if (year < 10)
            {
                date += "0";
            }
        break;
        }
        days -= 365 + leap;
        year++;
    }

    date += String(year) + "/";
    int month;
    for (month = 1; month < 12; month++)
    {
        uint8_t daysPerMonth = daysInMonth[month - 1];
        if (leap && month == 2)
        {
            daysPerMonth++;
        }
        if (days < daysPerMonth)
        {
            if (month < 10)
            {
                date += "0";
            }
            break;
        }
        days -= daysPerMonth;
    }
    date += String(month) + "/";

    if ((days + 1) < 10)
    {
        date += "0";
    }
    date +=  String(days + 1) + ",";
    completeData = date +","+hours;
    _rc = _me310->clock_management(completeData.c_str());
    if(_rc != ME310::RETURN_VALID)
    {
        return false;
    }
    return true;
}

//! \brief Get network status
/*! \details
This method gets network stutus
 * \return enum network status
 */
TLT_NetworkStatus_t TLT::getStatus()
{
  return _state;
}

//! \brief Parse time
/*! \details
This method converts the time string in tm struct.
 *\param time string of time
 * \return tm struct
 */
struct tm TLT::parse_time(String time)
{
    struct tm now;
    string tmp_str;
    tmp_str = time.c_str();
    char expData[2];
    if(time.startsWith("+CCLK"))
    {
        int len = tmp_str.copy(expData, 2, 8);
        if(len != 0)
        {
          expData[len] = '\0';
          now.tm_year = atoi(expData);          
        }
        memset(expData, 0, 2);
        len = tmp_str.copy(expData, 2, 11);        
        if(len != 0)
        {
          expData[len] = '\0';
          int mon = atoi(expData);
          now.tm_mon = mon -1;
        }
        memset(expData, 0, 2);
        len = tmp_str.copy(expData, 2, 14);        
        if(len != 0)
        {          
          expData[len] = '\0';
          now.tm_mday = atoi(expData);
        }
        memset(expData, 0, 2);
        len = tmp_str.copy(expData, 2, 17);        
        if(len != 0)
        {
          expData[len] = '\0';
          now.tm_hour = atoi(expData);
        }
        memset(expData, 0, 2);
        len = tmp_str.copy(expData, 2, 20);        
        if(len != 0)
        {
          expData[len] = '\0';
          now.tm_min = atoi(expData);
        }
        memset(expData, 0, 2);
        len = tmp_str.copy(expData, 2, 23);        
        if(len != 0)
        {
          expData[len] = '\0';
          now.tm_sec = atoi(expData);
        }
    }
    else 
    {
        now.tm_year = 0;
        now.tm_mon = 0;
        now.tm_mday = 0;
        now.tm_hour = 0;
        now.tm_min = 0;
        now.tm_sec = 0;
        return now;
    }
    return now;
}

//! \brief Restart module
/*! \details
This method calls reboot method if flag is true
 * \param flag if is true reboot the module 
 * \return returns true if reboot is successful, else false
 */
bool TLT::TLTRestart(bool flag)
{
    digitalWrite(LED_BUILTIN, LOW);
    if (flag) 
    {
        _me310->module_reboot();
        delay(6000);
        return true;
    }
    return false;
}

//!\brief Checks the module.
/*! \details 
This method checks the module.
 *\return 1 if the module is ready, 0 else.
 */
int TLT::moduleReady()
{
    _rc = _me310->attention();
    if(_rc == ME310::RETURN_VALID)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//!\brief Check Set Phone Functionality.
/*! \details 
This method checks set phone functionality.
 *\param  value is the level of functionality in the ME
 *\return true if the value is found, false else.
 */
bool TLT::checkSetPhoneFunctionality(int value)
{
    int i = 0;
    string response;
    string tmp_str = "+CFUN: ";
    tmp_str.append(std::to_string(value));
    _me310->read_set_phone_functionality();
    while(_me310->buffer_cstr(i) != NULL)
    {
      response = _me310->buffer_cstr(i);
      size_t foundString = response.find(tmp_str);
      if(foundString != string::npos)
      {
         return true;
      } 
      i++;
    }
    return false;
}