/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLTSSLClient.cpp

  @brief
   

  @details
     

  @version 
    1.0.0
  
  @note
    Dependencies:
    ME310.h
    TLTRootCerts.h
    TLTSSLClient.h

  @author
    

  @date
    08/03/2021
*/

#include "TLTRootCerts.h"

#include "TLTSSLClient.h"


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
TLTSSLClient::TLTSSLClient(ME310* me310, bool synch) :
  TLTClient(me310, synch),
  _RCs(TLT_ROOT_CERTS),
  _numRCs(TLT_NUM_ROOT_CERTS),
  _customRootCerts(false),
  _version(4),
  _SNI(1)
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
TLTSSLClient::TLTSSLClient(ME310* me310, int version, int SNI,  bool synch) :
  TLTClient(me310, synch),
  _RCs(TLT_ROOT_CERTS),
  _numRCs(TLT_NUM_ROOT_CERTS),
  _customRootCerts(false),
  _version(version),
  _SNI(SNI)
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
TLTSSLClient::TLTSSLClient(ME310* me310, const TLTRootCert* myRCs, int myNumRCs, int version, int SNI, bool synch) :
  TLTClient(me310, synch),
  _RCs(myRCs),
  _numRCs(myNumRCs),
  _customRootCerts(true),
  _customRootCertsLoaded(false),
  _version(version),
  _SNI(SNI)
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
      _state = SSL_CLIENT_STATE_WAIT_MANAGE_PROFILE_RESPONSE;
      ready = 0;
      break;
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
        if (_RCs[_certIndex].size)
        {
          // load the next root cert
          _rc = _me310->ssl_security_data(1,1, 1, _RCs[_certIndex].size, 0, (char*) _RCs[_certIndex].data, ME310::TOUT_1MIN);
          if (_rc != ME310::RETURN_VALID)
          {
              // failure
              ready = -1;
          }
          else
          {
              _state = SSL_CLIENT_STATE_WAIT_LOAD_ROOT_CERT_RESPONSE;
              ready = 0;
          }
        }
        else
        {
          // remove the next root cert name
          _rc = _me310->ssl_security_data(1,0,_RCs[_certIndex].dataType);
          _state = SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE;
          ready = 0;
        }
        break;
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
  _certIndex++;
  if (_certIndex == _numRCs)
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
  }
  else
  {
    // load next
    _state = SSL_CLIENT_STATE_LOAD_ROOT_CERT;
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