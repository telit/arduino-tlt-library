/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */


/**
  @file
    TLTUDP.cpp

  @brief
   

  @details
     

  @version 
    1.3.0
  
  @note
    Dependencies:
    ME310.h
    TLTUDP.h
    Arduino.h

  @author
    

  @date
    08/02/2021
*/

#include <TLTUDP.h>

//! \brief Class Constructor
/*!
 * \param me310 pointer of ME310 class
 */
TLTUDP::TLTUDP(ME310* me310) :
  _socket(-1),
  _packetReceived(false),
  _txIp((uint32_t)0),
  _txHost(NULL),
  _txPort(0),
  _txSize(0),
  _rxIp((uint32_t)0),
  _rxPort(0),
  _rxSize(0),
  _rxIndex(0)
{
    _me310 = me310;
}

TLTUDP::~TLTUDP()
{}

//!\brief UDP begin.
/*! \details 
This method calls begin method setting connID and socket.
 * \param port RX port
 * \return 1 if the configuration is successful, 0 otherwise
 */
uint8_t TLTUDP::begin(uint16_t port)
{
    int socket = -1;
    _me310->socket_status();
    int i = 1;
    String strSocket;
    while(_me310->buffer_cstr(i) != NULL)
    {
        strSocket = _me310->buffer_cstr(i);
        if(strSocket.endsWith(",0"))
        {
            int pos = strSocket.lastIndexOf(",0");
            String subString = strSocket.substring(pos-1, pos);
            socket = atoi(subString.c_str());
            break;
        }
        i++;
    }
    return begin(1, socket, port);
}

//!\brief UDP begin.
/*! \details 
This method calls begin method setting connID and socket, sets configuration socket with SRECV in hex mode and send data mode in IRA mode.
 * \param connID connection ID
 * \param socket socket ID
 * \param port RX port
 * \return 1 if the configuration is successful, 0 otherwise
 */
uint8_t TLTUDP::begin(int connID, int socket, uint16_t port)
{
    _rc = _me310->socket_configuration(connID, socket);
    if (_rc != ME310::RETURN_VALID)
    {
        return 0;
    }

    _rc = _me310->socket_configuration_extended(1,0,1,0,0,1);
    if (_rc != ME310::RETURN_VALID)
    {
        return 0;
    }

    _rxPort = port;
    _socket = socket;
    return 1; 
}


//! \brief Socket Stop
/*! \details 
This method stops the socket connection.
 *\return void
 */
void TLTUDP::stop()
{
    if (_socket < 0)
    {
        return;
    }
    _me310->socket_shutdown(_socket);
    _socket = -1;
}

//!\brief UDP begin.
/*! \details 
This method sets packet value.
 * \param ip IP address
 * \param port TX port
 * \return 1 if the configuration is successful, 0 otherwise
 */
int TLTUDP::beginPacket(IPAddress ip, uint16_t port)
{
    if (_socket < 0)
    {
        return 0;
    }
    _txIp = ip;
    _txHost = NULL;
    _txPort = port;
    _txSize = 0;
    return 1;
}

//!\brief UDP begin.
/*! \details 
This method sets packet value.
 * \param host string of host
 * \param port TX port
 * \return 1 if the socket is valid, 0 otherwise
 */
int TLTUDP::beginPacket(const char *host, uint16_t port)
{
    if (_socket < 0)
    {
        return 0;
    }
    
    _txIp = (uint32_t)0;
    _txHost = host;
    _txPort = port;
    _txSize = 0;
    return 1;
}

//!\brief End Packet.
/*! \details 
This method sends data.
 * \return 1 if the data is sent successfully, 0 otherwise
 */
int TLTUDP::endPacket()
{
    String ipAddr;
    const char* p_ipAddr;
    if(_txHost == NULL)
    {
        ipAddr += _txIp[0];
        ipAddr += '.';
        ipAddr += _txIp[1];
        ipAddr += '.';
        ipAddr += _txIp[2];
        ipAddr += '.';
        ipAddr += _txIp[3];        
        p_ipAddr = ipAddr.c_str();
    }
    else
    {
        p_ipAddr = _txHost;
    }
    
    _me310->socket_status(_socket);
    _rc = _me310->socket_dial(_socket, 1, _txPort, p_ipAddr, 0, _rxPort, 1, 0, 0);
    if (_rc == ME310::RETURN_VALID)
    {
        uint8_t tmp_Buffer[1028];
        _me310->ConvertBufferToIRA(_txBuffer,tmp_Buffer,_txSize);
        _rc = _me310->socket_send_data_command_mode(_socket, (char*)tmp_Buffer);        
        if(_rc == ME310::RETURN_VALID)
        {
            return 1;
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

//! \brief Write a single character 
/*! \details 
This method writes a single character by calling the write method and passing it 1 as the value of size.
 *\param c single character 
 *\return size of written character.
 */
size_t TLTUDP::write(uint8_t b)
{
  return write(&b, sizeof(b));
}

//! \brief Write a buffer of uint8
/*! \details 
This method writes a buffer of uint8 by calling the write method and passing it buffer size as the value of size.
 *\param buf buffer of uint8
 *\return size of written character.
 */
size_t TLTUDP::write(const uint8_t *buffer, size_t size)
{
    if (_socket < 0)
    {
        return 0;
    }

    size_t spaceAvailable = sizeof(_txBuffer) - _txSize;

    if (size > spaceAvailable)
    {
        size = spaceAvailable;
    }
    memcpy(&_txBuffer[_txSize], buffer, size);
    _txSize += size;
    return size;
}

//! \brief Parse the received packets 
/*! \details 
This method checks if any packets have arrived and if so they are read and inserted into the read buffer
 *\return size of read buffer.
 */
int TLTUDP::parsePacket()
{
    _me310->wait_for_unsolicited(ME310::TOUT_200MS);
    _rc = _me310->socket_status(_socket);
    if(_rc != ME310::RETURN_VALID)
    {
        return 0;
    }

    if (_me310->buffer_cstr(1) == NULL)
    {
        return 0;
    }
    
    String strSocket = _me310->buffer_cstr(1);
    if(strSocket.endsWith(",0"))
    {
        return 0;
    }

    if (_socket < 0)
    {
        return 0;
    }
    
    _rc = _me310->socket_info();
    if(_rc ==  ME310::RETURN_VALID)
    {
        String socketInfo = _me310->buffer_cstr(1);
        int recvDataSize = CheckData(socketInfo);
        if(recvDataSize == 0)
        {
            return 0;
        }
        if(recvDataSize > 1500)
        {
            recvDataSize = 1500;
        }
        uint8_t* response;
        
        _rc = _me310->socket_receive_data_command_mode(_socket, recvDataSize, 1);
        if (_rc != ME310::RETURN_VALID && _rc != ME310::RETURN_CONTINUE)
        {
            return 0;
        }
        response = (uint8_t*)_me310->buffer_cstr_raw();
        _rxIndex = 0;
        _rxSize = recvDataSize;
        if(response != NULL)
        {
            memcpy(_rxBuffer, response, _rxSize);
            return _rxSize;
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

//! \brief Client UDP Available
/*! \details 
This method check if the client is available
 *\return 0 if the client is not available, else 1
 */
int TLTUDP::available()
{
    if (_socket < 0)
    {
        return 0;
    }
    return (_rxSize - _rxIndex);
}

//! \brief Read method
/*! \details 
This method creates a byte buffer and calls the method read
 *\return 0 if the socket is not connected, else 1
 */
int TLTUDP::read()
{
    byte b;
    if (read(&b, sizeof(b)) == 1)
    {
        return b;
    }
    return -1;
}

//! \brief Read method
/*! \details 
This method fills the buffer through the method of TLTSOCKETBUFFER class.
 *\param buf pointer of buffer
 *\param size the size of buffer
 *\return 0 if the socket is not connected, else 1
 */
int TLTUDP::read(unsigned char* buffer, size_t len)
{
    size_t readMax = available();

    if (len > readMax)
    {
        len = readMax;
    }
    memcpy(buffer, &_rxBuffer[_rxIndex], len);
    _rxIndex += len;
    return len;
}

//! \brief Socket Available
/*! \details 
This method check if the client is available
 *\return 0 if the client is not available, else 1
 */
int TLTUDP::peek()
{
    if (available() > 1)
    {
        return _rxBuffer[_rxIndex];
    }
    return -1;
}

void TLTUDP::flush()
{
}

//! \brief Get remote IP addres
/*! \details 
This method gets remote IP address
 *\return remote IP address
 */
IPAddress TLTUDP::remoteIP()
{
    return _rxIp;
}

//! \brief Get remote port
/*! \details 
This method gets remote port
 *\return remote port
 */
uint16_t TLTUDP::remotePort()
{
    return _rxPort;
}

//! \brief Check data size
/*! \details 
This method checks if any data has arrived and if is right, returns the size of the received data 
 * \param dataStr string of command received
 * \return size of data received
 */
int TLTUDP::CheckData(String dataStr)
{
    int firstComma = dataStr.indexOf(",");
    int secondComma = dataStr.indexOf(",", firstComma + 1);
    int thirdComma = dataStr.indexOf(",", secondComma + 1);
    int fourthComma = dataStr.indexOf(",", thirdComma + 1);
    String sizeData = dataStr.substring(thirdComma + 1, fourthComma);
    int size = sizeData.toInt();
    return size;
}