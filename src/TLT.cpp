/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLT.cpp

  @brief
   

  @details
     

  @version 
    1.3.0
  
  @note
    Dependencies:
    ME310.h
    TLT.h

 @author
    Cristina Desogus

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
    _debug = debug;
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
  int i = 0;
  _rc = _me310->read_gprs_network_registration_status();
  if(_rc == ME310::RETURN_VALID)
  {
      char* resp = (char*) _me310->buffer_cstr(i);
      while(resp != NULL)
      {
        if((strcmp(resp, "+CREG: 0,1")!= 0) && (strcmp(resp, "+CREG: 0,5")!= 0))
        {
            return 1;
        }
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
        delay(2000);
        return 0;
    }
    if(_debug)
    {
        printReadyState();
    }
    switch (_readyState)
    {
        case READY_STATE_SET_ERROR_DISABLED:
        {
            _rc = _me310->report_mobile_equipment_error(2);
            if(_rc == ME310::RETURN_VALID)
            {
                _readyState = READY_STATE_WAIT_SET_ERROR_DISABLED;
                ready = 0;
            }
            else
            {
                ready = 2;
            } 
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
            if(checkSetPhoneFunctionality(1))
            {
                _readyState = READY_STATE_WAIT_SET_MINIMUM_FUNCTIONALITY_MODE;
                ready = 0;
                break;
            }
            else
            {
                _rc = _me310->set_phone_functionality(1);
                delay(5000);
                if(_rc == ME310::RETURN_VALID)
                {
                    _readyState = READY_STATE_WAIT_SET_MINIMUM_FUNCTIONALITY_MODE;
                    ready = 0;
                    break;
                }
                else
                {
                    ready = 2;
                    break;
                }
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
            int i = 0;
            _me310->read_enter_pin();
            char* resp = (char*) _me310->buffer_cstr(i);
            while(resp != NULL)
            {
                if(strcmp(resp, "OK") == 0)
                {
                    _response = _me310->buffer_cstr(1);
                }
                resp = (char*) _me310->buffer_cstr(i);
                i++;
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
            _rc = _me310->ps_attach_detach(0);
            if(_rc == ME310::RETURN_VALID)
            {
                _readyState = READY_STATE_WAIT_DETACH_DATA;
                ready = 0;
            }
            else
            {
                ready = 1;
            }
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
                if(_debug)
                {
                    Serial.println(_response.c_str());
                }
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
            _rc = _me310->context_activation(1,1);
            if(_rc == ME310::RETURN_VALID)
            {
                _response = _me310->buffer_cstr(1);
                _readyState = READY_STATE_WAIT_CHECK_CONTEXT_ACTIVATION;
                ready = 0;
                break;
            }
            else
            {
                ready = 2;
                break;
            }
            
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
    else
    {
        return 0;
    }
    now = parse_time(response);
    if (now.tm_year != 0)
    {
        time_t result = mktime(&now);
        time_t delta = ((response.charAt(26) - '0') * 10 + (response.charAt(27) - '0')) * (15 * 60);

        if(result != -1)
        {
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
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
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
        if(result != -1)
        {
            return result;
        }
        else
        {
            return 0;
        }
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
    struct tm now = {};
    string tmp_str;
    tmp_str = time.c_str();
    char expData[3] = {0};
    if(time.startsWith("+CCLK"))
    {
        char *timePtr = (char*) time.c_str();

        memcpy(expData, timePtr+8, 2);
        now.tm_year = atoi(expData);


        memset(expData, 0, sizeof(expData));
        memcpy(expData, timePtr+11, 2);
        int mon = atoi(expData);
        now.tm_mon = mon -1;


        memset(expData, 0, sizeof(expData));
        memcpy(expData,timePtr+14, 2);
        now.tm_mday = atoi(expData);

        memset(expData, 0, sizeof(expData));
        memcpy(expData, timePtr+17, 2);
        now.tm_hour = atoi(expData);

        memset(expData, 0, sizeof(expData));
        memcpy(expData, timePtr+20, 2);
        now.tm_min = atoi(expData);

        memset(expData, 0, sizeof(expData));
        memcpy(expData, timePtr+20, 2);
        now.tm_sec = atoi(expData);

    }
    else 
    {
        now.tm_year = 0;
        now.tm_mon = 0;
        now.tm_mday = 0;
        now.tm_hour = 0;
        now.tm_min = 0;
        now.tm_sec = 0;
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
    
    _rc = _me310->read_set_phone_functionality();
    if(_rc == ME310::RETURN_VALID)
    {
        char* tmp_buf = (char*) _me310->buffer_cstr(i);
        while(tmp_buf != NULL)
        {
            response = tmp_buf;
            size_t foundString = response.find(tmp_str);
            if(foundString != string::npos)
            {
                return true;
            }
            i++;
            tmp_buf = (char*) _me310->buffer_cstr(i);
        }
    }
    return false;
}

//!\brief Get debug parameter value.
/*! \details 
This method gets debug parameter value.
 *\return debug parameter value.
 */
bool TLT::getDebug()
{
    return _debug;
}

//!\brief Set debug parameter value.
/*! \details 
This method sets debug parameter value.
 *\param debug true to enable debugging, false disable debugging.
 */
void TLT::setDebug(bool debug)
{
    _debug = debug;
}

//!\brief Get ready state.
/*! \details 
This method gets ready state.
 *\return ready state parameter value.
 */
int TLT::getReadyState()
{
    return _readyState;
}

//!\brief Print ready state string.
/*! \details 
This method prints ready state string.
 */
void TLT::printReadyState()
{
    switch (_readyState)
    {
        case READY_STATE_SET_ERROR_DISABLED:
            Serial.println("READY_STATE_SET_ERROR_DISABLED");
            break;
        case READY_STATE_WAIT_SET_ERROR_DISABLED:
            Serial.println("READY_STATE_WAIT_SET_ERROR_DISABLED");
            break;
        case READY_STATE_SET_MINIMUM_FUNCTIONALITY_MODE:
            Serial.println("READY_STATE_SET_MINIMUM_FUNCTIONALITY_MODE");
            break;
        case READY_STATE_WAIT_SET_MINIMUM_FUNCTIONALITY_MODE:
            Serial.println("READY_STATE_WAIT_SET_MINIMUM_FUNCTIONALITY_MODE");
            break;
        case READY_STATE_CHECK_SIM:
            Serial.println("READY_STATE_CHECK_SIM");
            break;
        case READY_STATE_WAIT_CHECK_SIM_RESPONSE:
            Serial.println("READY_STATE_WAIT_CHECK_SIM_RESPONSE");
            break;
        case READY_STATE_UNLOCK_SIM:
            Serial.println("READY_STATE_UNLOCK_SIM");
            break;
        case READY_STATE_WAIT_UNLOCK_SIM_RESPONSE:
            Serial.println("READY_STATE_WAIT_UNLOCK_SIM_RESPONSE");
            break;
        case READY_STATE_DETACH_DATA:
            Serial.println("READY_STATE_DETACH_DATA");
            break;
        case READY_STATE_WAIT_DETACH_DATA:
            Serial.println("READY_STATE_WAIT_DETACH_DATA");
            break;
        case READY_STATE_SET_PREFERRED_MESSAGE_FORMAT:
            Serial.println("READY_STATE_SET_PREFERRED_MESSAGE_FORMAT");
            break;
        case READY_STATE_WAIT_SET_PREFERRED_MESSAGE_FORMAT_RESPONSE:
            Serial.println("READY_STATE_WAIT_SET_PREFERRED_MESSAGE_FORMAT_RESPONSE");
            break;
        case READY_STATE_SET_HEX_MODE:
            Serial.println("READY_STATE_SET_HEX_MODE");
            break;
        case READY_STATE_WAIT_SET_HEX_MODE_RESPONSE:
            Serial.println("READY_STATE_WAIT_SET_HEX_MODE_RESPONSE");
            break;
        case READY_STATE_SET_AUTOMATIC_TIME_ZONE:
            Serial.println("READY_STATE_SET_AUTOMATIC_TIME_ZONE");
            break;
        case READY_STATE_WAIT_SET_AUTOMATIC_TIME_ZONE_RESPONSE:
            Serial.println("READY_STATE_WAIT_SET_AUTOMATIC_TIME_ZONE_RESPONSE");
            break;
        case READY_STATE_SET_APN:
            Serial.println("READY_STATE_SET_APN");
            break;
        case READY_STATE_SET_APN_AUTH:
            Serial.println("READY_STATE_SET_APN_AUTH");
            break;
        case READY_STATE_WAIT_SET_APN_AUTH:
            Serial.println("READY_STATE_WAIT_SET_APN_AUTH");
            break;
        case READY_STATE_SET_FULL_FUNCTIONALITY_MODE:
            Serial.println("READY_STATE_SET_FULL_FUNCTIONALITY_MODE");
            break;
        case READY_STATE_WAIT_SET_FULL_FUNCTIONALITY_MODE:
            Serial.println("READY_STATE_WAIT_SET_FULL_FUNCTIONALITY_MODE");
            break;
        case READY_STATE_CHECK_REGISTRATION:
            Serial.println("READY_STATE_CHECK_REGISTRATION");
            break;
        case READY_STATE_WAIT_CHECK_REGISTRATION_RESPONSE:
            Serial.println("READY_STATE_WAIT_CHECK_REGISTRATION_RESPONSE");
            break;
        case READY_STATE_CHECK_CONTEXT_ACTIVATION:
            Serial.println("READY_STATE_CHECK_CONTEXT_ACTIVATION");
            break;
        case READY_STATE_WAIT_CHECK_CONTEXT_ACTIVATION:
            Serial.println("READY_STATE_WAIT_CHECK_CONTEXT_ACTIVATION");
            break;
        case READY_STATE_DONE:
            Serial.println("READY_STATE_DONE");
            break;
        default:
            break;
    }
}

//!\brief Get IMEI value.
/*! \details 
This method gets IMEI value.
 *\return string of IMEI value.
 */
String TLT::getIMEI()
{
    String IMEI;
    int i = 0;
    _me310->request_psn_identification_echo();
    while(_me310->buffer_cstr(i) != NULL)
    {
        String response = _me310->buffer_cstr(i);
        if(response.startsWith("#CGSN:"))
        {
            int pos = response.indexOf(":");
            IMEI = response.substring(pos + 2);
        }
        i++;
    }
    return IMEI;
}