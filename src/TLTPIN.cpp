/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */


/**
  @file
    TLTPIN.cpp

  @brief
   

  @details
     

  @version 
    1.1.0
  
  @note
    Dependencies:
    ME310.h
    TLTPIN.h
    Arduino.h

  @author
    Cristina Desogus

  @date
    08/02/2021
*/

#include <ME310.h>
#include <TLTPIN.h>

//!\brief Class Constructor
/*! \details 
Constructor
 * \param me310 pointer of ME310 class.
 */
TLTPIN::TLTPIN(ME310* me310) : _pinUsed(false)
{
    _me310 = me310;
}

void TLTPIN::begin()
{
}

//!\brief Check PIN of SIM.
/*! \details 
This method checks if the SIM is inserted and if it needs the PIN.
 * \return 0 if not needs the PIN, else 1.
 */
int TLTPIN::isPIN()
{
    String response;

    for (unsigned long start = millis(); (millis() - start) < 1000;)
    {
        _rc = _me310->read_enter_pin();
        if (_rc == ME310::RETURN_VALID)
        {
            response = _me310->buffer_cstr(1);
            if (response.startsWith("+CPIN: "))
            {
                if (response.endsWith("READY"))
                {
                    return 0;
                }
                else if (response.endsWith("SIM PIN"))
                {
                    return 1;
                }
                else if (response.endsWith("SIM PUK"))
                {
                    return -1;
                }
                else
                {
                    return -2;
                }
            }
        }
        delay(100);
    }
    return -2;
}

//!\brief Check PIN.
/*! \details 
This method checks if the PIN is valid.
 * \param pin string of PIN
 * \return 0 if the PIN is right, else -1.
 */
int TLTPIN::checkPIN(String pin)
{
    const char *p_pin;
    p_pin = pin.c_str();
    _rc = _me310->enter_pin(p_pin);
    if (_rc == ME310::RETURN_VALID)
    {
        return 0;
    }
    return -1;
}

//!\brief Check PUK.
/*! \details 
This method checks if the PUK is valid.
 * \param puk string of PUK
 * \param pin string of PIN
 * \return 0 if the PUK and PIN are right, else -1.
 */
int TLTPIN::checkPUK(String puk, String pin)
{
    const char* p_pin;
    const char* p_puk;
    p_pin = pin.c_str();
    p_puk = puk.c_str();
    _rc = _me310->enter_pin(p_puk, p_pin);
    if (_rc == ME310::RETURN_VALID)
    {
        return 0;
    }
    return -1;
}

//!\brief Change PIN.
/*! \details 
This method sets the new PIN
 * \param old string of old PIN
 * \param pin string of new PIN
 * \return void.
 */
void TLTPIN::changePIN(String old, String pin)
{
    const char* p_old;
    const char* p_pin;
    p_old = old.c_str();
    p_pin = pin.c_str();
    _rc = _me310->change_facility_password("SC", p_old, p_pin);
    if (_rc == ME310::RETURN_VALID)
    {
        Serial.println("Pin changed successfully.");
    }
    else
    {
        Serial.println("ERROR");
    }
}

//!\brief Switch PIN.
/*! \details 
This method is used to lock a ME on a network facility.
 * \param pin string of PIN
 * \return void.
 */
void TLTPIN::switchPIN(String pin)
{
    String response;
    _rc = _me310->facility_lock_unlock("SC", 2);
    if (_rc != ME310::RETURN_VALID)
    {
        Serial.println("ERROR");
        return;
    }
    response = _me310->buffer_cstr(1);
    if (response == "+CLCK: 0")
    {
        const char* p_pin;
        p_pin = pin.c_str();
        _rc = _me310->facility_lock_unlock("SC", 1, p_pin);
        if (_rc == ME310::RETURN_VALID)
        {
            Serial.println("OK. PIN lock on.");
            _pinUsed = true;
        }
        else
        {
            Serial.println("ERROR");
            _pinUsed = false;
        }
    }
    else if (response == "+CLCK: 1")
    {
        const char* p_pin;
        p_pin = pin.c_str();
        _rc = _me310->facility_lock_unlock("SC", 0, p_pin);
        if (_rc == ME310::RETURN_VALID)
        {
            Serial.println("OK. PIN lock off.");
            _pinUsed = false;
        }
        else
        {
            Serial.println("ERROR");
            _pinUsed = true;
        }
    }
    else
    {
        Serial.println("ERROR");
    }
}

//!\brief Check Network registration status.
/*! \details 
This method checks network registration status.
 * \return 0 if the registration status is 1, 1 if registration status is 5, -1 otherwise.
 */
int TLTPIN::checkReg()
{
    for (unsigned long start = millis(); (millis() - start) < 10000L;)
    {
        _rc = _me310->read_network_registration_status();
        String response;

        if (_rc == ME310::RETURN_VALID)
        {
            response = _me310->buffer_cstr(1);
            if (response.startsWith("+CREG: "))
            {
                if (response.endsWith(",1"))
                {
                    return 0;
                }
                else if (response.endsWith(",5"))
                {
                    return 1;
                }
            }
        }
        delay(100);
    }
    return -1;
}

//!\brief Get if pin is used
/*! \details 
This method gets if PIN is used
 * \return  true if PIN is used, false otherwise.
 */
bool TLTPIN::getPINUsed()
{
    return _pinUsed;
}

//!\brief SET PIN used
/*! \details 
This method sets if PIN is used
 * \param used pin used value
 * \return  void
 */
void TLTPIN::setPINUsed(bool used)
{
    _pinUsed = used;
}