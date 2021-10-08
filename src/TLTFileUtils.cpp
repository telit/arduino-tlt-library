/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLTFileUtils.cpp

  @brief
   

  @details
     

  @version 
    1.1.0
  
  @note
    Dependencies:
    ME310.h
    TLTUtils.h

  @author
    

  @date
    08/02/2021
*/


#include <TLTFileUtils.h>

TLTFileUtils::TLTFileUtils(ME310* me310, bool debug) : _count(0), _files(""), _debug(debug)
{
    _me310 = me310;
}

bool TLTFileUtils::begin(const bool restart)
{
    int status;

    if (_debug)
    {
        _me310->report_mobile_equipment_error(2);
    }

    for (unsigned long start = millis(); (millis() - start) < 10000;)
    {
        status = _getFileList();
        if (status == 1)
        {
            _countFiles();
            return true;
        }
    }
    return false;
}

int TLTFileUtils::_getFileList()
{
    String response;
    int status = 0;

    while (!status)
    {
        _rc = _me310->m2m_list();
        if(_rc == ME310::RETURN_VALID)
        {
            status = 1;
            response = _me310->buffer_cstr_raw();
        }
        if (status)
        {
            _files = response;
        }
    }
    return status;
}

int TLTFileUtils::existFile(const String filename)
{
    _getFileList();
    _countFiles();

    String files[_count];

    int num = listFiles(files);

    for (int i = 0; i<num; i++)
    {
        if (files[i]==filename)
        {
            return 1;
        }
    }
    return 0;
}

void TLTFileUtils::_countFiles()
{
    String list = _files;
    size_t len = 0;

    if (list.length() > 0)
    {
        for (int index = list.indexOf('#M2MLIST: '); index != -1; index = list.indexOf('#M2MLIST: '))
        {
            list.remove(0, index + 1);
            ++len;
        }
        ++len;
    }
    _count = len;
}

size_t TLTFileUtils::listFiles(String files[]) const
{
    String list = _files;
    int index;

    if (_count == 0)
        return 0;

    size_t n = 0;

    for (index = list.indexOf('#M2MLIST: '); index != -1; index = list.indexOf('#M2MLIST: '))
    {
        String file = list.substring(1, index - 1);
        files[n++] = file;
        list.remove(0, index + 1);
    }
    files[n++] = list.substring(1, list.lastIndexOf("\""));

    return n;
}

uint32_t TLTFileUtils::downloadFile(String filename, char buf[], uint32_t size, const bool append)
{
    deleteFile(filename);

    int status = 0;

    while (!status)
    {
        _rc = _me310->m2m_write_file(filename.c_str(), size, 1, buf);
        if (_rc == ME310::RETURN_VALID)
        {
            status = 1;
        }
    }
    return size;
}

uint32_t TLTFileUtils::createFile(const String filename, const char buf[], uint32_t size)
{
    uint32_t sizeFile;
    sizeFile = listFile(filename);
    if (sizeFile)
    {
        return sizeFile;
    }
    return downloadFile(filename, (char*) buf, size);
}

uint32_t TLTFileUtils::readFile(const String filename, String* content)
{
    String response;

    if (!listFile(filename)) {
        return 0;
    }

    _me310->m2m_read(filename.c_str());
    response = _me310->buffer_cstr_raw();

    String data;
    data = response;
    return data.length();
}

uint32_t TLTFileUtils::readFile(const String filename, uint8_t* content)
{
    String response;

    if (listFile(filename) == 0)
    {
        return 0;
    }

    _me310->m2m_read(filename.c_str());
    response = _me310->buffer_cstr_raw();

    content = (uint8_t*) response.c_str();

    return response.length();
}

bool TLTFileUtils::deleteFile(const String filename)
{
    String response;
    int status = 0;

    _rc = _me310->m2m_delete(filename.c_str());
    if (_rc == ME310::RETURN_VALID)
    {
        status = 1;
    }
    
    if (status == 0)
        return false;

    _getFileList();
    _countFiles();

    return true;
}

int TLTFileUtils::deleteFiles()
{
    int n = 0;
    String files[_count];

    int num = listFiles(files);

    while (_count > 0)
    {
        n += deleteFile(files[_count - 1]);
    }

    return n;
}

uint32_t TLTFileUtils::listFile(const String filename)
{
    String response;
    int res;
    uint32_t size = 0;
    const char* fileN;
    fileN = filename.c_str();
    _rc = _me310->m2m_list(fileN);
    if(_rc == ME310::RETURN_VALID)
    {
        response = _me310->buffer_cstr_raw();        
        size = response.length();
    }

    return size / 2;
}

uint32_t TLTFileUtils::freeSpace()
{
    String response;
    int res;
    uint32_t size = 0;
    _rc = _me310->m2m_list();

    if (_rc == ME310::RETURN_VALID)
    {
        response = _me310->buffer_cstr_raw();
        String pos = response.substring(1, response.lastIndexOf("free bytes: "));
        size = pos.toInt();
    }
    else
    {
        size = 0;
    }

    return size;
}