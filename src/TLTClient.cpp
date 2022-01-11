/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLTClient.cpp

  @brief
   

  @details
     

  @version 
    1.3.0
  
  @note
    Dependencies:
    ME310.h
    TLTClient.h
    TLTSocketBuffer.h

  @author
    

  @date
    07/28/2021
*/
#include <TLTSocketBuffer.h>

#include <TLTClient.h>

/*! \enum Client state machine status
  \brief Enum of client status of internal state machine
*/
enum
{
  CLIENT_STATE_IDLE,
  CLIENT_STATE_CREATE_SOCKET,
  CLIENT_STATE_WAIT_CREATE_SOCKET_RESPONSE,
  CLIENT_STATE_CONNECT,
  CLIENT_STATE_WAIT_CONNECT_RESPONSE,
  CLIENT_STATE_CLOSE_SOCKET,
  CLIENT_STATE_WAIT_CLOSE_SOCKET,
  CLIENT_STATE_RETRIEVE_ERROR
};

//! \brief Class Constructor
/*!
 * \param me310 pointer of ME310 class
 * \param synch value of synchronous.
 */
TLTClient::TLTClient(ME310* me310, bool synch, bool debug):
  _synch(synch),
  _socket(-1),
  _connected(false),
  _state(CLIENT_STATE_IDLE),
  _ip((uint32_t)0),
  _host(NULL),
  _port(0),
  _ssl(false),
  _writeSync(true),
  _debug(debug)
{
  _me310 = me310;
}

//! \brief Class Constructor
/*!
 * \param me310 pointer of ME310 class
 * \param socket socket id value
 * \param synch value of synchronous.
 */
TLTClient::TLTClient(ME310* me310, int socket, bool synch, bool debug) :
  _synch(synch),
  _socket(socket),
  _connected(false),
  _state(CLIENT_STATE_IDLE),
  _ip((uint32_t)0),
  _host(NULL),
  _port(0),
  _ssl(false),
  _writeSync(true),
  _debug(debug)
{
  _me310 = me310;
}

TLTClient::~TLTClient()
{}

//! \brief Check internal state machine status
/*! \details
This method checks internal state machine status and call the specific command.
 * \return returns 0 if last command is still executing, 1 if last command is executed successful, >1 if last
 command is executed with error.
 */
int TLTClient::ready()
{
  int ready = moduleReady();
  if (ready == 0) 
  {
    return 0;
  }

  if(_debug)
  {
    printReadyState();
  }

  switch (_state) 
  {
    case CLIENT_STATE_IDLE:
    default:
    {
      break;
    }

    case CLIENT_STATE_CREATE_SOCKET:
    {
      _me310->socket_configuration(1, _socket);
      _response = _me310->buffer_cstr(1);
      _rc = _me310->socket_configuration_extended(1,0,0,0,0,0);
      _state = CLIENT_STATE_WAIT_CREATE_SOCKET_RESPONSE;
      ready = 0;
      break;
    }
    case CLIENT_STATE_WAIT_CREATE_SOCKET_RESPONSE: 
    {
      if (ready > 1 || !_response.startsWith("OK")) 
      {
        _state = CLIENT_STATE_IDLE;
      } 
      else
      {
        _state = CLIENT_STATE_CONNECT;
        ready = 0;
      }
      break;
    }
    
    case CLIENT_STATE_CONNECT:
    {
      if(!_ssl)
      {
        if(_host != NULL)
        {
          _rc = _me310->socket_dial(_socket, 0, _port, _host,  0, 0, 1, 0, 0, ME310::TOUT_1MIN);
        }
        else
        {
          String tmpIP;
          tmpIP += _ip[0];
          tmpIP += _ip[1];
          tmpIP += _ip[2];
          tmpIP += _ip[3];
          _host = tmpIP.c_str();
          _rc = _me310->socket_dial(_socket, 0, _port, _host,  0, 0, 1, 0, 0, ME310::TOUT_1MIN);
        }
        _state = CLIENT_STATE_WAIT_CONNECT_RESPONSE;
        ready = 0;
        break;
      }
      else
      {
        if(_host != NULL)
        {
          _rc = _me310->ssl_socket_open(_socket, _port, _host,  0, 1, 100, ME310::TOUT_1MIN); 
        }
        else
        {
          String tmpIP;
          tmpIP += _ip[0];
          tmpIP += _ip[1];
          tmpIP += _ip[2];
          tmpIP += _ip[3];
          _host = tmpIP.c_str();
          _rc = _me310->ssl_socket_open(_socket, _port, _host,  0, 1, 100, ME310::TOUT_1MIN);
        }
        _state = CLIENT_STATE_WAIT_CONNECT_RESPONSE;
        ready = 0;
        break;
      }
      
    }
    case CLIENT_STATE_WAIT_CONNECT_RESPONSE:
    {
      if (ready > 1)
      {
        _state = CLIENT_STATE_CLOSE_SOCKET;
        ready = 0;
      }
      else
      {
        /*TODO if not ssl ok else ssl socket status*/
        if(!_ssl)
        {
          _rc = _me310->socket_status(_socket);
          _connected = true;
        }
        else
        {
          _me310->ssl_socket_status(1);
          _connected = true;
        }
        
        _state = CLIENT_STATE_IDLE;
      }
      break;
    }
    case CLIENT_STATE_CLOSE_SOCKET:
    {
      _me310->socket_shutdown(_socket);
      _state = CLIENT_STATE_WAIT_CLOSE_SOCKET;
      ready = 0;
      break;
    }
    case CLIENT_STATE_WAIT_CLOSE_SOCKET:
    {
      _state = CLIENT_STATE_RETRIEVE_ERROR;
      _socket = -1;
      break;
    }
    case CLIENT_STATE_RETRIEVE_ERROR:
    {
      _me310->socket_detect_cause_disconnection(_socket);
      _state = CLIENT_STATE_IDLE;
      break;
    }
  }
  return ready;
}


//! \brief Connect Socket
/*! \details
This method calls connect method, sets ip address and port, SSL value is false.
 *\param ip IP Address 
 *\param port TX port
 *\return returns 0 if socket is connected, else 1.
 */
int TLTClient::connect(IPAddress ip, uint16_t port)
{
  _ip = ip;
  _host = NULL;
  _port = port;
  _ssl = false;
  return connect();
}

//! \brief Connect SSL Socket
/*! \details
This method calls connect method, sets ip address and port, SSL value is true.
 *\param ip IP Address 
 *\param port TX port
 *\return returns 0 if socket is connected, else 1.
 */
int TLTClient::connectSSL(IPAddress ip, uint16_t port)
{
  _ip = ip;
  _host = NULL;
  _port = port;
  _ssl = true;
  return connect();
}

//! \brief Connect Socket
/*! \details
This method calls connect method, sets host value and port, SSL value is false.
 *\param host string of host
 *\param port TX port
 *\return returns 0 if socket is connected, else 1.
 */
int TLTClient::connect(const char *host, uint16_t port)
{
 
  _ip = (uint32_t)0;
  _host = host;
  _port = port;
  _ssl = false;
  return connect();
}

//! \brief Connect SSL Socket
/*! \details
This method calls connect method, sets host value and port, SSL value is true.
 *\param host string of host 
 *\param port TX port
 *\return returns 0 if socket is connected, else 1.
 */
int TLTClient::connectSSL(const char *host, uint16_t port)
{
  _ip = (uint32_t)0;
  _host = host;
  _port = port;
  _ssl = true;
  return connect();
}

//! \brief Connect Socket
/*! \details
This method connects the socket, looks the first free socket and uses this ID to connect.
 *\return returns 0 if socket is connected, else 1.
 */
int TLTClient::connect()
{
  String strSocket;
  if(_ssl)
  {
    _rc = _me310->ssl_socket_status(1);
    strSocket = _me310->buffer_cstr(1);
    if(strSocket.endsWith(",0"))
    {
      _socket = 1;
    }
    if (_socket == -1)
    {
      stop();
    }
    if (_synch)
    {
      while (ready() == 0)
      {
        delay(1000);
      }
    }
    else if (ready() == 0)
    {
      return 0;
    }
    _socket = 1;
   _state = CLIENT_STATE_CREATE_SOCKET;
    return 1;
  }
  else
  {
    _me310->socket_status();
    int i = 0;
    
    while(_me310->buffer_cstr(i) != NULL)
    {
      strSocket = _me310->buffer_cstr(i);
      if(strSocket.endsWith(",0"))
      {
        int pos = strSocket.lastIndexOf(",0");
        String subString = strSocket.substring(pos-1, pos);
        _socket = atoi(subString.c_str());
        break;
      }
      i++;
    }
    if (_socket == -1)
    {
      stop();
    }
    if (_synch)
    {
      while (ready() == 0)
      {
        delay(5);
      }
    }
    else if (ready() == 0)
    {
      return 0;
    }
    _state = CLIENT_STATE_CREATE_SOCKET;
    if (_synch)
    {
      while (ready() == 0)
      {
        delay(5);
      }
      if (_socket == -1)
      {
        return 0;
      }
    }
    return 1;
  }
  
}

//! \brief Begin Write
/*! \details
This method sets write synchronous.
 *\param sync write synchronous value.
 *\return void
 */
void TLTClient::beginWrite(bool sync)
{
  _writeSync = sync;
}

//! \brief Write a single character 
/*! \details 
This method writes a single character by calling the write method and passing it 1 as the value of size.
 *\param c single character 
 *\return size of written character.
 */
size_t TLTClient::write(uint8_t c)
{
  return write(&c, 1);
}

//! \brief Write a buffer of uint8
/*! \details 
This method writes a buffer of uint8 by calling the write method and passing it buffer size as the value of size.
 *\param buf buffer of uint8
 *\return size of written character.
 */
size_t TLTClient::write(const uint8_t *buf)
{
  return write(buf, strlen((const char*)buf));
}

//! \brief Write a buffer of uint8
/*! \details 
This method writes a buffer of uint8.
 *\param buf buffer of uint8
 *\param size the size of buffer
 *\return size of written character.
 */
size_t TLTClient::write(const uint8_t* buf, size_t size)
{
  if (_writeSync)
  {
    while (ready() == 0)
    {
      delay(5);
    }
  }
  else if (ready() == 0)
  {
    return 0;
  }
  if (_socket == -1)
  {
    return 0;
  }
  size_t written = 0;
  if(!_ssl )
  {
    _rc = _me310->socket_send_data_command_mode(_socket, (char*) buf, 1);
    if (_writeSync)
    {
      String response;
      response = _me310->buffer_cstr(1);
      if (_rc != ME310::RETURN_VALID && response.indexOf("Operation not allowed") != -1 )
      {
        stop();
      }
    }
    written += size;
    return written;
  }
  else
  {
    _rc = _me310->ssl_socket_send_data_command_mode(1, size, (char*)buf);
    if (_writeSync)
    {
      String response;
      response = _me310->buffer_cstr(1);
      if (_rc != ME310::RETURN_VALID && response.indexOf("Operation not allowed") != -1 )
      {
        stop();
      }
    }
    written += size;
    return written;
  }
  
}

//! \brief End write
/*! \details 
This method ends the write method, set true the write synchronous.
 *\return void
 */
void TLTClient::endWrite(bool /*sync*/)
{
  _writeSync = true;
}

//! \brief Connected socket
/*! \details 
This method check if the socket is connected.
 *\return 0 if the socket is not connected, else 1
 */
uint8_t TLTClient::connected()
{
  if (_socket == -1)
  {
    return 0;
  }

  // call available to update socket state
  if (TLTSOCKETBUFFER.available(_socket, _ssl) < 0 || (_ssl && !_connected))
  {
    stop();
    return 0;
  }
  return 1;
}

TLTClient::operator bool()
{
  return (_socket != -1);
}

//! \brief Read method
/*! \details 
This method fills the buffer through the method of TLTSOCKETBUFFER class.
 *\param buf pointer of buffer
 *\param size the size of buffer
 *\return 0 if the socket is not connected, else 1
 */
int TLTClient::read(uint8_t *buf, size_t size)
{
  if (_socket == -1)
  {
    return 0;
  }

  if (size == 0)
  {
    return 0;
  }

  int avail = available();

  if (avail == 0)
  {
    return 0;
  }

  return TLTSOCKETBUFFER.read(_socket, buf, size, _ssl);
}

//! \brief Read method
/*! \details 
This method creates a byte buffer and calls the method read
 *\return 0 if the socket is not connected, else 1
 */
int TLTClient::read()
{
  byte b;

  if (read(&b, 1) == 1)
  {
    return b;
  }
  return -1;
}

//! \brief Client Available
/*! \details 
This method check if the client is available
 *\return 0 if the client is not available, else 1
 */
int TLTClient::available()
{
  if (_synch)
  {
    while (ready() == 0)
    {
      delay(5);
    }
  }
  else if (ready() == 0)
  {
    return 0;
  }

  if (_socket == -1)
  {
    return 0;
  }

  int avail = TLTSOCKETBUFFER.available(_socket, _ssl);

  if (avail < 0)
  {
    stop();
    return 0;
  }
  return avail;
}

//! \brief Socket Available
/*! \details 
This method check if the client is available
 *\return 0 if the client is not available, else 1
 */
int TLTClient::peek()
{
  if (available() > 0)
  {
    return TLTSOCKETBUFFER.peek(_socket, _ssl);
  }
  return -1;
}

void TLTClient::flush()
{
}

//! \brief Socket Stop
/*! \details 
This method stops the socket connection.
 *\return void
 */
void TLTClient::stop()
{
  _state = CLIENT_STATE_IDLE;
  if (_socket < 0)
  {
    return;
  }
  _me310->socket_shutdown(_socket);
  TLTSOCKETBUFFER.close(_socket);
  _socket = -1;
  _connected = false;
}

void TLTClient::handleUrc(const String& urc)
{
  if (urc.startsWith("#SRECV: "))
  {
    int socket = urc.charAt(9) - '0';
    if (socket == _socket)
    {
      if (urc.endsWith(",4294967295"))
      {
        _connected = false;
      }
    }
  } 
}

//!\brief Checks the module.
/*! \details 
This method checks the module.
 *\return 1 if the module is ready, 0 else.
 */
int TLTClient::moduleReady()
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
bool TLTClient::getDebug()
{
    return _debug;
}

//!\brief Set debug parameter value.
/*! \details 
This method sets debug parameter value.
 *\param debug true to enable debugging, false disable debugging.
 */
void TLTClient::setDebug(bool debug)
{
    _debug = debug;
}

//!\brief Get ready state.
/*! \details 
This method gets ready state.
 *\return ready state parameter value.
 */
int TLTClient::getReadyState()
{
    return _state;
}

//!\brief Print ready state string.
/*! \details 
This method prints ready state string.
 */
void TLTClient::printReadyState()
{
  switch (_state)
  {
    case CLIENT_STATE_IDLE:
        Serial.println("CLIENT_STATE_IDLE");
        break;
    case CLIENT_STATE_CREATE_SOCKET:
        Serial.println("CLIENT_STATE_CREATE_SOCKET");
        break;
    case CLIENT_STATE_WAIT_CREATE_SOCKET_RESPONSE:
        Serial.println("CLIENT_STATE_WAIT_CREATE_SOCKET_RESPONSE");
        break;
    case CLIENT_STATE_CONNECT:
        Serial.println("CLIENT_STATE_CONNECT");
        break;
    case CLIENT_STATE_CLOSE_SOCKET:
        Serial.println("CLIENT_STATE_CLOSE_SOCKET");
        break;
    case CLIENT_STATE_WAIT_CLOSE_SOCKET:
        Serial.println("CLIENT_STATE_WAIT_CLOSE_SOCKET");
        break;
    case CLIENT_STATE_RETRIEVE_ERROR:
        Serial.println("CLIENT_STATE_RETRIEVE_ERROR");
        break;
    default:
        break;
  }
}