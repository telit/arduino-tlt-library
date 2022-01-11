/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    GPRS.cpp

  @brief
   

  @details
     

  @version 
    1.3.0
  
  @note
    Dependencies:
    GPRS.h

  @author
    

  @date
    08/03/2021
*/
#include <GPRS.h>

/*! \enum GPRS status
  \brief Enum of GPRS status of internal state machine
*/
enum
{
    GPRS_STATE_IDLE,
    GPRS_STATE_ATTACH,
    GPRS_STATE_WAIT_ATTACH_RESPONSE,
    GPRS_STATE_CHECK_ATTACHED,
    GPRS_STATE_WAIT_CHECK_ATTACHED_RESPONSE,
    GPRS_STATE_DEATTACH,
    GPRS_STATE_WAIT_DEATTACH_RESPONSE
};

//! \brief Class Constructor
/*!
 * \param me310 pointer of ME310 class
 */
GPRS::GPRS(ME310* me310, bool debug) : _status(IDLE), _timeout(0), _debug(debug)
{
    _me310 = me310;
}

GPRS::~GPRS()
{
}

//!\brief Network Attach
/*! \details 
This method attachs the network.
 *\return network status .
 */
TLT_NetworkStatus_t GPRS::networkAttach()
{
    return attachGPRS();
};

//!\brief Network Detach
/*! \details
This method detachs the network
 *\return network status.
 */
TLT_NetworkStatus_t GPRS::networkDetach()
{ 
    return detachGPRS(); 
};

//!\brief GPRS Attach
/*! \details 
This method set the network state in GPRS_STATE_ATTACH and call ready function.
 *\param synchronous is a boolean value that synchronous the module
 *\return network status .
 */
TLT_NetworkStatus_t GPRS::attachGPRS(bool synchronous)
{
    _state = GPRS_STATE_ATTACH;
    _status = CONNECTING;
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
            delay(500);
        }
    }
    else
    {
        ready();
    }
    return _status;
}

//!\brief GPRS Detach
/*! \details 
This method set the network state in GPRS_STATE_DEATTACH and call ready function.
 *\param synchronous is a boolean value that synchronous the module
 *\return network status .
 */
TLT_NetworkStatus_t GPRS::detachGPRS(bool synchronous)
{
    _state = GPRS_STATE_DEATTACH;
    if (synchronous)
    {
        while (ready() == 0)
        {
            delay(100);
        }
    }
    else
    {
        ready();
    }
    return _status;
}

//! \brief Check internal state machine status
/*! \details
This method checks internal state machine status and call the specific command.
 * \return returns 0 if last command is still executing, 1 if last command is executed successful, >1 if last
 command is executed with error.
 */
int GPRS::ready()
{
    int ready = moduleReady();
    if (ready == 0)
    {
        return 0;
    }
    ready = 0;

    if(_debug)
    {
        printReadyState();
    }
    switch (_state)
    {
        case GPRS_STATE_IDLE:
        default:
        {
            break;
        }
        case GPRS_STATE_ATTACH:
        {
            _me310->ps_attach_detach(1);
            _state = GPRS_STATE_WAIT_ATTACH_RESPONSE;
            ready = 0;
            break;
        }
        case GPRS_STATE_WAIT_ATTACH_RESPONSE:
        {
            if (ready > 1)
            {
                _state = GPRS_STATE_IDLE;
                _status = ERROR;
            }
            else
            {
                _state = GPRS_STATE_CHECK_ATTACHED;
                ready = 0;
            }
            break;
        }
        case GPRS_STATE_CHECK_ATTACHED:
        {
            _me310->read_ps_attach_detach();
            _response = _me310->buffer_cstr(1);
            _state = GPRS_STATE_WAIT_CHECK_ATTACHED_RESPONSE;
            ready = 0;
            break;
        }
        case GPRS_STATE_WAIT_CHECK_ATTACHED_RESPONSE:
        {
            if (ready > 1)
            {
                _state = GPRS_STATE_IDLE;
                _status = ERROR;
            }
            else
            {
                if (_response.endsWith("1"))
                {
                    _state = GPRS_STATE_IDLE;
                    _status = GPRS_READY;
                    ready = 1;
                } 
                else
                {
                    _state = GPRS_STATE_WAIT_ATTACH_RESPONSE;
                    ready = 0;
                }
            }
            break;
        }
        case GPRS_STATE_DEATTACH:
        {
            _me310->ps_attach_detach(0);
            _state = GPRS_STATE_WAIT_DEATTACH_RESPONSE;
            ready = 0;
            break;
        }
        case GPRS_STATE_WAIT_DEATTACH_RESPONSE:
        {
            if (ready > 1)
            {
                _state = GPRS_STATE_IDLE;
                _status = ERROR;
            }
            else
            {
                _state = GPRS_STATE_IDLE;
                _status = IDLE;
                ready = 1;
            }
            break;
        }
  }
  return ready;
}

//! \brief Get internal IP Address
/*! \details
This method gets internal IP Address
 * \return IP Address if it is present else 0.0.0.0.
 */
IPAddress GPRS::getIPAddress()
{
    String response;
    _rc = _me310->show_pdp_address(1);
    if (_rc == ME310::RETURN_VALID)
    {
        response = _me310->buffer_cstr(1);
        if (response.startsWith("+CGPADDR: 1,"))
        {
            response.remove(0, 12);
            response.remove(response.length());

            IPAddress ip;

            if (ip.fromString(response))
            {
                return ip;
            }
        }
    }
    return IPAddress(0, 0, 0, 0);
}

//! \brief Set timeout
/*! \details
This method sets internal timeout
 *\param timeout timeout value
 * \return void
 */
void GPRS::setTimeout(unsigned long timeout)
{
    _timeout = timeout;
}

//! \brief Get network status
/*! \details
This method gets internal state machine status 
 *\param timeout timeout value
 * \return void
 */
TLT_NetworkStatus_t GPRS::status()
{
    return _status;
}


//!\brief Checks the module.
/*! \details 
This method checks the module.
 *\return 1 if the module is ready, 0 else.
 */
int GPRS::moduleReady()
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

/*DEBUG*/
//!\brief Get debug parameter value.
/*! \details 
This method gets debug parameter value.
 *\return debug parameter value.
*/
bool GPRS::getDebug()
{
    return _debug;
}

//!\brief Set debug parameter value.
/*! \details 
This method sets debug parameter value.
 *\param debug true to enable debugging, false disable debugging.
 */
void GPRS::setDebug(bool debug)
{
    _debug = debug;
}

//!\brief Get ready state.
/*! \details 
This method gets ready state.
 *\return ready state parameter value.
 */
int GPRS::getReadyState()
{
    return _state;
}

//!\brief Print ready state string.
/*! \details 
This method prints ready state string.
 */
void GPRS::printReadyState()
{
    switch (_state)
    {
        case GPRS_STATE_IDLE:
            Serial.println("GPRS_STATE_IDLE");
            break;
        case GPRS_STATE_ATTACH:
            Serial.println("GPRS_STATE_ATTACH");
            break;
        case GPRS_STATE_WAIT_ATTACH_RESPONSE:
            Serial.println("GPRS_STATE_WAIT_ATTACH_RESPONSE");
            break;
        case GPRS_STATE_CHECK_ATTACHED:
            Serial.println("GPRS_STATE_CHECK_ATTACHED");
            break;
        case GPRS_STATE_WAIT_CHECK_ATTACHED_RESPONSE:
            Serial.println("GPRS_STATE_WAIT_CHECK_ATTACHED_RESPONSE");
            break;
        case GPRS_STATE_DEATTACH:
            Serial.println("GPRS_STATE_DEATTACH");
            break;
        case GPRS_STATE_WAIT_DEATTACH_RESPONSE:
            Serial.println("GPRS_STATE_WAIT_DEATTACH_RESPONSE");
            break;
        default:
            break;
    }
}