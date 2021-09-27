/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLTScanner.cpp

  @brief
   

  @details
     

  @version 
    1.0.0
  
  @note
    Dependencies:
    TLTScanner.h
    Modem.h

  @author
    Cristina Desogus

  @date
    08/02/2021
*/
#include <ME310.h>

#include "TLTScanner.h"


//! \brief Class Constructor
/*!
 * \param me310 pointer of ME310 class
 * \param trace value of synchronous.
 */
TLTScanner::TLTScanner(ME310* me310, bool trace)
{ 
    _me310 = me310;
}

//!\brief Begin Scanner.
/*! \details 
This method returns IDLE status.
 * \return IDLE status
 */
TLT_NetworkStatus_t TLTScanner::begin()
{
    return IDLE;
}

//!\brief Get Current Carrier.
/*! \details 
This method reads the operator selection and return the string of this current carrier.
 * \return current carrier.
 */
String TLTScanner::getCurrentCarrier()
{
    String response;

    _rc = _me310->read_operator_selection();

    if (_rc == ME310::RETURN_VALID) 
    {
        response = _me310->buffer_cstr(1);
        int firstQuoteIndex = response.indexOf('"');
        int lastQuoteIndex = response.lastIndexOf('"');

        if (firstQuoteIndex != -1 && lastQuoteIndex != -1 && firstQuoteIndex != lastQuoteIndex)
        {
            return response.substring(firstQuoteIndex + 1, lastQuoteIndex);
        }
    }
    return "";
}

//!\brief Get Signal Stregth.
/*! \details 
This method reads the signal quality of service and return it.
 * \return signal strength.
 */
String TLTScanner::getSignalStrength()
{
    String response;
    _rc = _me310->signal_quality();
    if (_rc = ME310::RETURN_VALID)
    {
        response = _me310->buffer_cstr(1);
        int firstSpaceIndex = response.indexOf(' ');
        int lastCommaIndex = response.lastIndexOf(',');

        if (firstSpaceIndex != -1 && lastCommaIndex != -1)
        {
            return response.substring(firstSpaceIndex + 1, lastCommaIndex);
        }
    }
    return "";
}

//!\brief Read Networks list.
/*! \details 
This method reads the list of available networks.
 * \return list of available networks.
 */
String TLTScanner::readNetworks()
{
    String response;
    _rc = _me310->test_operator_selection();
    if (_rc = ME310::RETURN_VALID)
    {
        response = _me310->buffer_cstr(1);
        String result;
        unsigned int responseLength = response.length();

        for(unsigned int i = 0; i < responseLength; i++)
        {
            for (; i < responseLength; i++)
            {
                if (response[i] == '"')
                {
                    result += "> ";
                    break;
                }
            }
            for (i++; i < responseLength; i++)
            {
                if (response[i] == '"')
                {
                    result += '\n';
                    break;
                }

                result += response[i];
            }

            for (i++; i < responseLength; i++)
            {
                if (response[i] == ')')
                {
                    break;
                }
            }
        }
        return result;
    }
    return "";
}