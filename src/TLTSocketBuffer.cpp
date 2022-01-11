/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */


/**
  @file
    TLTSocketBuffer.cpp

  @brief
   

  @details
     

  @version 
    1.3.0
  
  @note
    Dependencies:
    ME310.h
    TLTSocketBuffer.h

  @author
    

  @date
    07/28/2021
*/

#include <stdlib.h>
#include <string.h>
#include <TLTSocketBuffer.h>

using namespace me310;

#define TLT_SOCKET_NUM_BUFFERS (sizeof(_buffers) / sizeof(_buffers[0]))

#define TLT_SOCKET_BUFFER_SIZE 512

TLTSocketBuffer::TLTSocketBuffer()
{
    memset(&_buffers, 0x00, sizeof(_buffers));
}

TLTSocketBuffer::~TLTSocketBuffer()
{
    for (unsigned int i = 0; i < TLT_SOCKET_NUM_BUFFERS; i++)
    {
        close(i);
    }
}

void TLTSocketBuffer::close(int socket)
{
    if (_buffers[socket].data)
    {
        free(_buffers[socket].data);
        _buffers[socket].data = _buffers[socket].head = NULL;
        _buffers[socket].length = 0;
    }
}

int TLTSocketBuffer::available(int socket, bool ssl)
{
    if(!ssl)
    {
        if (_buffers[socket].length == 0)
        {
            if (_buffers[socket].data == NULL)
            {
                _buffers[socket].data = _buffers[socket].head = (uint8_t*)malloc(TLT_SOCKET_BUFFER_SIZE);
                _buffers[socket].length = 0;
            }

            String response;
            _rc = _me310.socket_receive_data_command_mode(socket, TLT_SOCKET_BUFFER_SIZE);
            response = _me310.buffer_cstr_raw();
            if (_rc == ME310::RETURN_ERROR)
            {
                return -1;
            }
            if(response == "\r\n")
            {
                return -1;
            }
            if(response.lastIndexOf("ERROR\r\n") !=-1)
            {
                return 0;
            }

            const char * p_response;
            p_response = response.c_str();
            _buffers[socket].data =(uint8_t *) p_response;
            _buffers[socket].head = (uint8_t *)p_response;
            _buffers[socket].length = response.length();
        }
        return  _buffers[socket].length;
    }
    else
    {
        if (_buffers[socket].length == 0)
        {
            if (_buffers[socket].data == NULL)
            {
                _buffers[socket].data = _buffers[socket].head = (uint8_t*)malloc(TLT_SOCKET_BUFFER_SIZE);
                _buffers[socket].length = 0;
            }

            String response;
            _rc = _me310.ssl_socket_receive_data(socket, TLT_SOCKET_BUFFER_SIZE);
            response = _me310.buffer_cstr_raw();
            if (_rc == ME310::RETURN_ERROR)
            {
                return -1;
            }
            if(response == "\r\n")
            {
                return -1;
            }
            if(response.lastIndexOf("ERROR\r\n") !=-1)
            {
                return 0;
            }

            const char * p_response;
            p_response = response.c_str();
            _buffers[socket].data =(uint8_t *) p_response;
            _buffers[socket].head = (uint8_t *)p_response;
            _buffers[socket].length = response.length();
        }
        return  _buffers[socket].length;
            
    }
    
}

int TLTSocketBuffer::peek(int socket, bool ssl)
{
    if (!available(socket, ssl))
    {
        return -1;
    }

    return *_buffers[socket].head;
}

int TLTSocketBuffer::read(int socket, uint8_t* data, size_t length, bool ssl)
{
    int avail = available(socket, ssl);

    if (avail == -1) 
    {
        return 0;
    }

    if (avail < (int)length) 
    {
        length = avail;
    }

    memcpy(data, _buffers[socket].head, length);
    _buffers[socket].head += length;
    _buffers[socket].length -= length;
    return length;
}

TLTSocketBuffer TLTSOCKETBUFFER;