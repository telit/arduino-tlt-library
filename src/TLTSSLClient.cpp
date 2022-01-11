/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLTSSLClient.cpp

  @brief
   

  @details
     

  @version 
    1.3.0
  
  @note
    Dependencies:
    ME310.h
    TLTRootCerts.h
    TLTSSLClient.h

  @author
    

  @date
    08/03/2021
*/
#include <TLTSSLClient.h>

enum
{
  SSL_CLIENT_STATE_ENABLE,
  SSL_CLIENT_STATE_WAIT_ENABLE_RESPONSE,
  SSL_CLIENT_STATE_LOAD_ROOT_CERT,
  SSL_CLIENT_STATE_WAIT_LOAD_ROOT_CERT_RESPONSE,
  SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE,
  SSL_CLIENT_STATE_MANAGE_PROFILE,
  SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE,
  SSL_CLIENT_STATE_MANAGE_PROFILE_2,
  SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE_2,
  SSL_CLIENT_STATE_CLOSE_SOCKET, 
  SSL_CLIENT_STATE_CONNECT,
  SSL_CLIENT_STATE_WAIT_CONNECT
};



bool TLTSSLClient::_defaultRootCertsLoaded = false;

//!\brief Class Constructor
/*! \details 
Constructor
 * \param me310 pointer of ME310 class.
 * \param _synch synchronize the system
 */
TLTSSLClient::TLTSSLClient(ME310* me310, bool synch, bool debug) :
  TLTClient(me310, synch, debug),
  _RCs((TLTRootCert*)TLT_ROOT_CERTS),
  _numRCs(TLT_NUM_ROOT_CERTS),
  _customRootCerts(false),
  _version(4),
  _SNI(1), 
  _debug(debug)
{
  _me310 = me310;
}

//!\brief Class Constructor
/*! \details 
Constructor
 * \param me310 pointer of ME310 class.
 * \param version version of SSL
 * \param SNI SNI value
 * \param _synch synchronize the system
 */
TLTSSLClient::TLTSSLClient(ME310* me310, int version, int SNI,  bool synch, bool debug) :
  TLTClient(me310, synch, debug),
  _RCs(TLT_ROOT_CERTS),
  _numRCs(TLT_NUM_ROOT_CERTS),
  _customRootCerts(false),
  _version(version),
  _SNI(SNI),
  _debug(debug)
{
  _me310 = me310;
}

//!\brief Class Constructor
/*! \details 
Constructor
 * \param me310 pointer of ME310 class.
 * \param myRCs root certificate
 * \param myNumRCs number of RCs
 * \param _synch synchronize the system
 * \param version version of SSL
 * \param SNI SNI value
 * \param _synch synchronize the system
 */
TLTSSLClient::TLTSSLClient(ME310* me310, const TLTRootCert* myRCs, int myNumRCs, int version, int SNI, bool synch, bool debug) :
  TLTClient(me310, synch, debug),
  _RCs((TLTRootCert*)myRCs),
  _numRCs(myNumRCs),
  _customRootCerts(true),
  _customRootCertsLoaded(false),
  _version(version),
  _SNI(SNI),
  _debug(debug)
{
  _me310 = me310;
}

TLTSSLClient::~TLTSSLClient()
{
}

//! \brief Check internal state machine status
/*! \details
This method checks internal state machine status and call the specific command.
 * \return returns 0 if last command is still executing, 1 if last command is executed successful, >1 if last
 command is executed with error.
 */
int TLTSSLClient::ready()
{
  if ((!_customRootCerts && _defaultRootCertsLoaded) || (_customRootCerts && (_numRCs == 0 || _customRootCertsLoaded))) 
  {
    // root certs loaded already, continue to regular TLTClient
    return TLTClient::ready();
  }
  int ready = moduleReady();
  
  if (ready == 0)
  {
      // a command is still running
      return 0;
  }

  if(_debug)
  {
      printReadyState();
  }

  switch (_state)
  {
    case SSL_CLIENT_STATE_ENABLE:
    {
      _me310->ssl_enable(1,1);
      _state = SSL_CLIENT_STATE_WAIT_ENABLE_RESPONSE;
      ready = 0;
      break;
    }
    case SSL_CLIENT_STATE_WAIT_ENABLE_RESPONSE:
    {
      if (ready > 1)
      {
        _state = SSL_CLIENT_STATE_CLOSE_SOCKET;
      }
      else
      {
        _state = SSL_CLIENT_STATE_MANAGE_PROFILE;
      }
      ready = 0;
      break;
    }
    case SSL_CLIENT_STATE_MANAGE_PROFILE:
    {
      _rc = _me310->ssl_configure_security_param(1,0,1);
      if(_rc == ME310::RETURN_VALID)
      {
        _state = SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE;
        ready = 0;
        break;
      }
      else
      {
        ready = 2;
        break;
      }
    }
    case SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE:
    {
      if (ready > 1)
      {
        _state = SSL_CLIENT_STATE_ENABLE;
      }
      else
      {   
        _state = SSL_CLIENT_STATE_MANAGE_PROFILE_2;
      }
      ready = 0;
      break;
    }
    case SSL_CLIENT_STATE_MANAGE_PROFILE_2:
    {
      _rc = _me310->ssl_additional_parameters(1, _version, _SNI);
      _state = SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE_2;
      ready = 0;
      break;
    }
    case SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE_2:
    {
      if (ready > 1)
      {
        _state = SSL_CLIENT_STATE_ENABLE;
      }
      else
      {   
        _state = SSL_CLIENT_STATE_LOAD_ROOT_CERT;
      }
      ready = 0;
      break;
    }
    case SSL_CLIENT_STATE_LOAD_ROOT_CERT:
    {     
      if(_certIndex < _numRCs)
      {
        if (_RCs[_certIndex].size > 0)
        {
          // load the next root cert
          if((char*) _RCs[_certIndex].data != NULL)
          {
            _rc = _me310->ssl_security_data(1, 1, 1, _RCs[_certIndex].size, 0, (char*) _RCs[_certIndex].data, ME310::TOUT_1MIN);
            if (_rc != ME310::RETURN_VALID)
            {
                // failure
                _state = SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE;
                ready = 2;
            }
            else
            {
                _state = SSL_CLIENT_STATE_WAIT_LOAD_ROOT_CERT_RESPONSE;
                ready = 0;
            }
          }
          else
          {
              // failure
              _state = SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE;
              ready = 2;
          }
        }
        else
        {
          // remove the next root cert name
          _rc = _me310->ssl_security_data(1,0,_RCs[_certIndex].dataType);
          if(_rc != ME310::RETURN_VALID)
          {
            // failure
            _state = SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE;
            ready = 2;
          }
          else
          {
            _state = SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE;
            ready = 0;
          }          
        }
        break;
      }
      else
      {
        _state = SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE;
        ready = 0;
        break;
      }
    }
    case SSL_CLIENT_STATE_WAIT_LOAD_ROOT_CERT_RESPONSE:
    {
        if (ready > 1)
        {
          /*TODO ERROR*/
        }
        else
        {
          ready = iterateCerts();
        }
        break;
    }
    case SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE:
    {
        // ignore ready response, root cert might not exist
        ready = iterateCerts();
        break;
    }
    case SSL_CLIENT_STATE_CLOSE_SOCKET:
    {
      ready = 0;
      break;
    }
    default:
    {
      break;
    }
  }
  return ready;
}

//! \brief Connect SSL Socket
/*! \details
This method calls connect method, sets ip address and port, SSL value is true.
 *\param ip IP Address 
 *\param port TX port
 *\return returns 0 if socket is connected, else 1.
 */
int TLTSSLClient::connect(IPAddress ip, uint16_t port)
{
  _certIndex = 0;
  _state = SSL_CLIENT_STATE_ENABLE;
  return connectSSL(ip, port);
}

//! \brief Connect SSL Socket
/*! \details
This method calls connect method, sets host value and port, SSL value is true.
 *\param host string of host 
 *\param port TX port
 *\return returns 0 if socket is connected, else 1.
 */
int TLTSSLClient::connect(const char* host, uint16_t port)
{
  _certIndex = 0;
  _state = SSL_CLIENT_STATE_ENABLE;
  return connectSSL(host, port);
}

//! \brief Iterate the list of certificates
/*! \details
This method iterates the list of certificates.
 *\return always 0.
 */
int TLTSSLClient::iterateCerts()
{
  if(_numRCs > 0)
  {
    if (_certIndex == _numRCs-1)
    {
       // all certs loaded
      if (_customRootCerts)
      {
        _customRootCertsLoaded = true;
      }
      else
      {
        _defaultRootCertsLoaded = true;
      }
      
      _certIndex = 0;
    }
    else
    {
      _certIndex++;
        // load next
      _state = SSL_CLIENT_STATE_LOAD_ROOT_CERT;
    }
  }
  else
  {
    if (_customRootCerts)
    {
      _customRootCertsLoaded = true;
    }
    else
    {
      _defaultRootCertsLoaded = true;
    }
    
    _certIndex = 0;
  }
  return 0;
}

//!\brief Checks the module.
/*! \details 
This method checks the module.
 *\return 1 if the module is ready, 0 else.
 */
int TLTSSLClient::moduleReady()
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
bool TLTSSLClient::getDebug()
{
    return _debug;
}

//!\brief Set debug parameter value.
/*! \details 
This method sets debug parameter value.
 *\param debug true to enable debugging, false disable debugging.
 */
void TLTSSLClient::setDebug(bool debug)
{
    _debug = debug;
}

//!\brief Get ready state.
/*! \details 
This method gets ready state.
 *\return ready state parameter value.
 */
int TLTSSLClient::getReadyState()
{
    return _state;
}
//!\brief Print ready state string.
/*! \details 
This method prints ready state string.
 */
void TLTSSLClient::printReadyState()
{
    switch (_state)
    {
        case SSL_CLIENT_STATE_ENABLE:
            Serial.println("SSL_CLIENT_STATE_ENABLE");
            break;
        case SSL_CLIENT_STATE_WAIT_ENABLE_RESPONSE:
            Serial.println("SSL_CLIENT_STATE_WAIT_ENABLE_RESPONSE");
            break;
        case SSL_CLIENT_STATE_LOAD_ROOT_CERT:
            Serial.println("SSL_CLIENT_STATE_LOAD_ROOT_CERT");
            break;
        case SSL_CLIENT_STATE_WAIT_LOAD_ROOT_CERT_RESPONSE:
            Serial.println("SSL_CLIENT_STATE_WAIT_LOAD_ROOT_CERT_RESPONSE");
            break;
        case SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE:
            Serial.println("SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE");
            break;
        case SSL_CLIENT_STATE_MANAGE_PROFILE:
            Serial.println("SSL_CLIENT_STATE_MANAGE_PROFILE");
            break;
        case SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE:
            Serial.println("SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE");
            break;
        case SSL_CLIENT_STATE_MANAGE_PROFILE_2:
            Serial.println("SSL_CLIENT_STATE_MANAGE_PROFILE_2");
            break;
        case SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE_2:
            Serial.println("SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE_2");
            break;
        case SSL_CLIENT_STATE_CLOSE_SOCKET:
            Serial.println("SSL_CLIENT_STATE_CLOSE_SOCKET");
            break;
        case SSL_CLIENT_STATE_CONNECT:
            Serial.println("SSL_CLIENT_STATE_CONNECT");
            break;
        case SSL_CLIENT_STATE_WAIT_CONNECT:
            Serial.println("SSL_CLIENT_STATE_WAIT_CONNECT");
            break;
        default:
            break;
    }
}