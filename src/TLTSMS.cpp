/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */


/**
  @file
    TLTSMS.cpp

  @brief
   Management SMS functionality using library ME310 Telit Modem

  @details
     The class implements the typical functionalities of SMS.\n
     In particular: send and read SMS.

  @version 
    1.3.0
  
  @note
    Dependencies:
    TLTSMS.h

  @author
    

  @date
    08/02/2021
*/

#include <TLTSMS.h>

#define NYBBLETOHEX(x) ((x)<=9?(x)+'0':(x)-10+'A')
#define HEXTONYBBLE(x) ((x)<='9'?(x)-'0':(x)+10-'A')
#define ITOHEX(x) NYBBLETOHEX((x)&0xF)

#define SMS_CHARSET_IRA   'I'
#define SMS_CHARSET_GSM   'G'
#define SMS_CHARSET_NONE  'N'
#define SMS_CHARSET_UCS2  'U'

enum
{
    SMS_STATE_IDLE,
    SMS_STATE_LIST_MESSAGES,
    SMS_STATE_WAIT_LIST_MESSAGES_RESPONSE
};


struct gsm_mapping
{
  const unsigned char gsmc;
  const char *utf8;

  gsm_mapping(const char gsmc, const char *utf8) : gsmc(gsmc), utf8(utf8) {}
};

gsm_mapping _gsmUTF8map[] = {
    {0x00,"@"},{0x10,"Δ"},
    {0x40,"¡"},{0x60,"¿"},
    {0x01,"£"},{0x11,"_"},
    {0x02,"$"},{0x12,"Φ"},
    {0x03,"¥"},{0x13,"Γ"},
    {0x04,"è"},{0x14,"Λ"},
    {0x24,"¤"},
    {0x05,"é"},{0x15,"Ω"},
    {0x06,"ù"},{0x16,"Π"},
    {0x07,"ì"},{0x17,"Ψ"},
    {0x08,"ò"},{0x18,"Σ"},
    {0x09,"Ç"},{0x19,"Θ"},
/* Text mode SMS uses 0x1A as send marker so Ξ is not available. */
             //{0x1A,"Ξ"},
    {0x0B,"Ø"},                      {0x5B,"Ä"},{0x7B,"ä"},
    {0x0C,"ø"},{0x1C,"Æ"},           {0x5C,"Ö"},{0x7C,"ö"},
               {0x1D,"æ"},           {0x5D,"Ñ"},{0x7D,"ñ"},
    {0x0E,"Å"},{0x1E,"ß"},           {0x5E,"Ü"},{0x7E,"ü"},
    {0x0F,"å"},{0x1F,"É"},           {0x5F,"§"},{0x7F,"à"}};

/* Text mode SMS uses 0x1B as abort marker so extended set is not available. */
#if 0
gsm_mapping _gsmXUTF8map[] = {
    {0x40,"|"},
    {0x14,"^"},
    {0x65,"€"},
    {0x28,"{"},
    {0x29,"}"},
    {0x0A,"\f"},
    {0x1B,"\b"},
    {0x3C,"["},
    {0x0D,"\n"},
    {0x3D,"~"},
    {0x3E,"]"},
    {0x2F,"\\"}};
 //*/
#endif

//!\brief Class Constructor
/*! \details 
Constructor
 * \param me310 pointer of ME310
 * \param _synch synchronize the system
 */
TLTSMS::TLTSMS(ME310* me310, bool synch) : _synch(synch), _state(SMS_STATE_IDLE), _smsTxActive(false), _charset(SMS_CHARSET_NONE), _bufferUTF8{0,0,0,0}, _indexUTF8(0), _ptrUTF8("")
{
    _me310 = me310;
}

//!\brief Set charset
/*! \details 
Select specific SMS charset
 * \param charset character set, one of "IRA" (default), "GSM", or "UCS2", reads from modem if null.
 * \return first char of charset identifier on success and 0 on error
 */
int TLTSMS::setCharset(const char* charset)
{
    String readcharset(0);

    if ( charset == nullptr )
    {
        if ( _charset != SMS_CHARSET_NONE )
        {
            return _charset;
        }
    }
    else
    {
        _rc = _me310->select_te_character_set(charset);
        if(_rc != ME310::RETURN_VALID)
        {
            return 0;
        }
    }
    _rc = _me310->read_select_te_character_set();
    readcharset = _me310->buffer_cstr(1);
    if (_rc == ME310::RETURN_VALID && readcharset.startsWith("+CSCS: \""))
    {
        _charset = readcharset[8];
        return _charset;
    }
    return 0;
}

//!\brief Add a character in buffer of SMS message
/*! \details 
This method add a character in _dataBuffer SMS message that it will be sent.
 * \param c single character
 * \return size of character if SMS Tx is active, 0 else.
 */
size_t TLTSMS::write(uint8_t c)
{
    //add c into buffer (350) when call end() send AT command
    
    if (_smsTxActive)
    {
        if (_charset == SMS_CHARSET_GSM && (c >= 0x80 || c <= 0x24 || (c&0x1F) == 0 || (c&0x1F) >= 0x1B))
        {
            _bufferUTF8[_indexUTF8++]=c;
            if (_bufferUTF8[0] < 0x80 || (_indexUTF8==2 && (_bufferUTF8[0]&0xE0) == 0xC0) || (_indexUTF8==3 && (_bufferUTF8[0]&0xF0) == 0xE0) | _indexUTF8==4)
            {
                for (auto &gsmchar : _gsmUTF8map)
                {
                    if (strncmp(_bufferUTF8, gsmchar.utf8, _indexUTF8)==0)
                    {
                        _indexUTF8=0;
                        _dataBuffer += gsmchar.utf8;
                    }
                }
                
                for(c = 0; c < _indexUTF8; c++)
                {
                    _dataBuffer += c;
                }
                _indexUTF8 = 0;
            }
            return 1;
        }
        if (_charset == SMS_CHARSET_UCS2)
        {
            if (c < 0x80)
            {
                _dataBuffer += '0';
                _dataBuffer += '0';
                _dataBuffer += ITOHEX(c>>4);
            }
            else
            {
                _bufferUTF8[_indexUTF8++]=c;
                if (_indexUTF8==2 && (_bufferUTF8[0]&0xE0) == 0xC0)
                {
                    _dataBuffer += '0';
                    _dataBuffer += ITOHEX(_bufferUTF8[0]>>2);
                    _dataBuffer += ITOHEX((_bufferUTF8[0]<<2)|((c>>4)&0x3));
                }
                else if (_indexUTF8==3 && (_bufferUTF8[0]&0xF0) == 0xE0)
                {
                    _dataBuffer += ITOHEX(_bufferUTF8[0]);
                    _dataBuffer += ITOHEX(_bufferUTF8[1]>>2);
                    _dataBuffer += ITOHEX((_bufferUTF8[1]<<2)|((c>>4)&0x3));
                }
                else if (_indexUTF8==4)
                { // Missing in UCS2, output SPC
                    _dataBuffer += '0';
                    _dataBuffer += '2';
                    _dataBuffer += '2';
                    c = 0;
                }
                else
                {
                    return 1;
                }
            }
            _indexUTF8 = 0;
            c = ITOHEX(c);
        }
        return 1 ;
    }
    return 0;
}

//!\brief Begin a SMS to send it.
/*! \details 
This method add the destination address in _toBuffer.
 * \param to destination address
 * \return error command if it exists
 */
int TLTSMS::beginSMS(const char* to)
{
    setCharset();
    _toBuffer.remove(0, _toBuffer.length());
    if (_charset==SMS_CHARSET_UCS2 && *to == '+')
    {
        _toBuffer += '0';
        _toBuffer += '0';
        _toBuffer += '2';
        _toBuffer += 'B';
        to++;
    }
    while (*to!=0)
    {
        if (_charset==SMS_CHARSET_UCS2)
        {
            _toBuffer += '0';
            _toBuffer += '0';
            _toBuffer += '3';
        }
        _toBuffer += *to++;
    }
    _indexUTF8 = 0;
    _smsTxActive = true;

    return (_synch) ? 0 : 2;
}

//!\brief Get last command status.
/*! \details 
This method get last command status and read list of unreaded SMS.
 * \return returns 0 if last command is still executing, 1 success, >1 error
 */
int TLTSMS::ready()
{
    int ready;
    ready = moduleReady();
    if (ready == 0)
    {
        return 0;
    }
    switch(_state)
    {
        case SMS_STATE_IDLE:
        default:
        {
            break;
        }

        case SMS_STATE_LIST_MESSAGES:
        {
            _me310->list_messages("REC UNREAD");
            _incomingBuffer = _me310->buffer_cstr_raw();
            _state = SMS_STATE_WAIT_LIST_MESSAGES_RESPONSE;
            ready = 0;
            break;
        }
        case SMS_STATE_WAIT_LIST_MESSAGES_RESPONSE:
        {
            _state = SMS_STATE_IDLE;
            break;
        }
    }
    return ready;
}

//!\brief Send SMS
/*! \details 
This method ends SMS and sends it.
 * \return returns error command if it exists
 */
int TLTSMS::endSMS()
{
    int r;
    if (_smsTxActive)
    {
        _indexUTF8 = 0;
        char toda[] = "145";
        _rc = _me310->send_short_message((const char*)_toBuffer.c_str(), (const char*) toda, (char*) _dataBuffer.c_str(),ME310::TOUT_3SEC);
        
        return _rc;
    }
    else
    {
        return (_synch ? 0 : 2);
    }
}

//!\brief Availables SMS.
/*! \details 
This method checks if SMS available and prepare it to be read.
 * \return number of bytes in a received SMS.
 */
int TLTSMS::available()
{
    int nextMessageIndex = _incomingBuffer.indexOf("+CMGL: ");
    if (nextMessageIndex != -1)
    {
        _incomingBuffer.remove(0, nextMessageIndex);
    }
    else
    {
        _incomingBuffer = "";
    }

    if (_incomingBuffer.length() == 0)
    {
        int r;

        if (_state == SMS_STATE_IDLE)
        {
            setCharset();
            _state = SMS_STATE_LIST_MESSAGES;
        }

        if (_synch)
        {
            unsigned long start = millis();
            while ((r = ready()) == 0 && (millis() - start) < 3*60*1000)
            {
                delay(100);
            }
        }
        else
        {
            r = ready();
        }

        if (r != 1)
        {
            return 0;
        } 
    }
    if (_incomingBuffer.startsWith("+CMGL: "))
    {
        _incomingBuffer.remove(0, 7);
        _smsDataIndex = _incomingBuffer.indexOf('\n');
        _smsDataEndIndex = _incomingBuffer.indexOf("\r\n+CMGL: ",_smsDataIndex);
        if (_smsDataEndIndex == -1)
        {
            _smsDataEndIndex = _incomingBuffer.length() - 1;
        }

        return (_smsDataEndIndex - _smsDataIndex) + 1;
    }
    else
    {
        _incomingBuffer = "";
    }
    return 0;
}

//!\brief Reads remote number.
/*! \details 
This method reads sender number phone.
 * \param number buffer for save remote number phone.
 * \param nlength buffer length.
 * \return returns 1 if success, >1 if error.
 */
int TLTSMS::remoteNumber(char* number, int nlength)
{
    int phoneNumberStartIndex = _incomingBuffer.indexOf( "\"REC UNREAD\",\"");

    if (phoneNumberStartIndex != -1)
    {
        int i = phoneNumberStartIndex + sizeof( "\"REC UNREAD\",\"") - 1;

        if (_charset==SMS_CHARSET_UCS2 && _incomingBuffer.substring(i,i+4)=="002B")
        {
            *number++ = '+';
            i += 4;
        }
        while (i < (int)_incomingBuffer.length() && nlength > 1)
        {
            if (_charset==SMS_CHARSET_UCS2)
            {
                i += 3;
            }
            char c = _incomingBuffer[i];
            if (c == '"')
            {
                break;
            }
            *number++ = c;
            nlength--;
            i++;
        }
        *number = '\0';
        return 1;
    }
    else
    {
        *number = '\0';
    }
    return 2;
}

//!\brief Reads one char for SMS buffer.
/*! \details 
This method reads one char for SMS buffer in a circular buffer.
 * \return returns readed byte.
 */
int TLTSMS::read()
{
    if (*_ptrUTF8 != 0)
    {
        return *_ptrUTF8++;
    }
    if (_smsDataIndex < (signed)_incomingBuffer.length() && _smsDataIndex <= _smsDataEndIndex)
    {
        char c;
        if (_charset != SMS_CHARSET_UCS2)
        {
            c = _incomingBuffer[_smsDataIndex++];
            if (_charset == SMS_CHARSET_GSM && (c >= 0x80 || c <= 0x24 || (c&0x1F) == 0 || (c&0x1F) >= 0x1B))
            {
                for (auto &gsmchar : _gsmUTF8map)
                {
                    if (c == gsmchar.gsmc)
                    {
                        _ptrUTF8 = gsmchar.utf8;
                        return *_ptrUTF8++;
                    }
                }
            }
        }
        else
        {
            c = (HEXTONYBBLE(_incomingBuffer[_smsDataIndex+2])<<4) | HEXTONYBBLE(_incomingBuffer[_smsDataIndex+3]);
            if (strncmp(&_incomingBuffer[_smsDataIndex],"008",3)>=0)
            {
                _ptrUTF8 = _bufferUTF8+1;
                _bufferUTF8[2] = 0;
                _bufferUTF8[1] = (c&0x3F)|0x80;
                c = 0xC0 | (HEXTONYBBLE(_incomingBuffer[_smsDataIndex+1])<<2) | (HEXTONYBBLE(_incomingBuffer[_smsDataIndex+2])>>2);
                if (strncmp(&_incomingBuffer[_smsDataIndex],"08",2)>=0)
                {
                    _ptrUTF8 = _bufferUTF8;
                    _bufferUTF8[0] = c & (0x80|0x3F);
                    c = 0xE0 | HEXTONYBBLE(_incomingBuffer[_smsDataIndex]);
                }
            }
            _smsDataIndex += 4;
        }
        return c;
    }
    return -1;
}

//!\brief Reads a byte in SMS buffer.
/*! \details 
This method reads a byte without advance the buffer header.
 * \return returns readed byte.
 */
int TLTSMS::peek()
{
    if (*_ptrUTF8 != 0)
    {
        return *_ptrUTF8;
    }
    if (_smsDataIndex < (signed)_incomingBuffer.length() && _smsDataIndex <= _smsDataEndIndex)
    {
        char c = _incomingBuffer[_smsDataIndex++];
        if (_charset == SMS_CHARSET_GSM && (c >= 0x80 || c <= 0x24 || (c&0x1F) == 0 || (c&0x1F) >= 0x1B))
        {
            for (auto &gsmchar : _gsmUTF8map)
            {
                if (c == gsmchar.gsmc)
                {
                    return gsmchar.utf8[0];
                }
            }
        }
        if (_charset == SMS_CHARSET_UCS2)
        {
            c = (HEXTONYBBLE(_incomingBuffer[_smsDataIndex+2])<<4) | HEXTONYBBLE(_incomingBuffer[_smsDataIndex+3]);
            if (strncmp(&_incomingBuffer[_smsDataIndex],"008",3)>=0)
            {
                c = 0xC0 | (HEXTONYBBLE(_incomingBuffer[_smsDataIndex+1])<<2) | (HEXTONYBBLE(_incomingBuffer[_smsDataIndex+2])>>2);
                if (strncmp(&_incomingBuffer[_smsDataIndex],"08",2)>=0)
                {
                    c = 0xE0 | HEXTONYBBLE(_incomingBuffer[_smsDataIndex]);
                }
            }
        }
        return c;
    }
    return -1;
}

//!\brief Delete  SMS.
/*! \details 
This method deletes the SMS just read.
 */
void TLTSMS::flush()
{
    int smsIndexEnd = _incomingBuffer.indexOf(',');
    _ptrUTF8 = "";
    if (smsIndexEnd != -1)
    {
        while (moduleReady() == 0);

        if (_synch)
        {
            _me310->delete_message(smsIndexEnd);
        }
        else
        {
            _me310->delete_message(smsIndexEnd, ME310::TOUT_0MS);
        }
    }
}

//!\brief Cleans SMS.
/*! \details 
This method deletes all readed SMS.
 */
void TLTSMS::clean(int flag)
{
    _ptrUTF8 = "";

    while (moduleReady() == 0);

    if (flag<1 || flag>4)
    {
        flag = 2;
    }
    if (_synch)
    {
        _me310->delete_message(0, flag);
    }
    else
    {
        _me310->delete_message(0, flag, ME310::TOUT_0MS);
    }
}

size_t TLTSMS::print(const String &data)
{
    _dataBuffer = data;
    return _dataBuffer.length();
}

//!\brief Checks the module.
/*! \details 
This method checks the module.
 *\return 1 if the module is ready, 0 else.
 */
int TLTSMS::moduleReady()
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

//!\brief Sets message format.
/*! \details 
This method sets the format of SMS messages to be used.
 *\param value 0 if is in PDU mode, 1 if is in text mode.
 *\return returns true if success, false if error.
 */
bool TLTSMS::setMessageFormat(int value)
{
    _rc = _me310->message_format(value);
    if(_rc ==  ME310::RETURN_VALID)
    {
        return true;
    }
    else
    {
        return false;
    }
}