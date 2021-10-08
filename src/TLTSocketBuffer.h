/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*!
  @file
    TLTSocketBuffer.h

  @brief
    TLT Socket Buffer class
  @details
    
  
  @version 
    1.1.0

  @note
    Dependencies:
    ME310.h
    string.h

  @author
    Cristina Desogus

  @date
    07/28/2021
*/

#ifndef __TLTSOCKETBUFFER__H
#define __TLTSOCKETBUFFER__H
#include <ME310.h>
using namespace std;
using namespace me310;
class TLTSocketBuffer
{
    public:
        TLTSocketBuffer();
        virtual ~TLTSocketBuffer();

        void close(int socket);

        int available(int socket, bool ssl);
        int peek(int socket, bool ssl);
        int read(int socket, uint8_t* data, size_t length, bool ssl);

    private:

    ME310 _me310;
    ME310::return_t _rc;
    
        struct {
            uint8_t* data;
            uint8_t* head;
            int length;
        } _buffers[7];
};

extern TLTSocketBuffer TLTSOCKETBUFFER;

#endif //__TLTSOCKETBUFFER__H