/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLTFileUtils.cpp

  @brief


  @details


  @version
    1.3.0

  @note
    Dependencies:
    ME310.h
    TLTUtils.h

  @author


  @date
    08/02/2021
*/


#include <TLTFileUtils.h>
#include <vector>

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
    std::vector<String> *files = new std::vector<String>;
    int num = listFiles(files);
    for (int i = 0; i<num; i++)
    {
        if(files->at(i) == filename)
        {
            delete(files);
            return 1;
        }
    }
    delete(files);
    return 0;
}

void TLTFileUtils::_countFiles()
{
    _getFileList();
    String list = _files;
    size_t len = 0;

    if (list.length() > 0)
    {
        for (int index = list.indexOf("#M2MLIST: "); index != -1; index = list.indexOf("#M2MLIST: "))
        {
          int end = list.indexOf("\r\n", index);
          int comma;
          String line = list.substring(index + 1, end);
          comma = line.indexOf(",");
          if ( comma > 0)
          {
            ++len;
          }
          list.remove(0, index + 1);
        }
    }
    _count = len;
}

size_t TLTFileUtils::listFiles(std::vector<String> *files) const
{
    String list = _files;
    int index;
    int end = 0;
    if (_count == 0)
        return 0;

    size_t n = 0;
    for (index = list.indexOf(": "); index != -1; index = list.indexOf(": "))
    {
        end = list.indexOf("\r\n", index);
        int comma;
        String file = list.substring(index + 2, end);
        comma = file.indexOf(",");
        if ( comma > 0)
        {
          files->push_back(file.substring(1, comma-1)); /*start from 1 to comma - 1 to remove quotes */
          n++;
        }
        list.remove(0, end);
        
    }
    return n;
}


size_t TLTFileUtils::listFiles(String files[]) const
{
    String list = _files;
    int index;
    int end = 0;
    if (_count == 0)
        return 0;

    size_t n = 0;
    for (index = list.indexOf(": "); index != -1; index = list.indexOf(": "))
    {
        end = list.indexOf("\r\n", index);
        int comma;
        String file = list.substring(index + 2, end);
        comma = file.indexOf(",");
        if ( comma > 0)
        {
          files[n++] = file.substring(1, comma-1); /*start from 1 to comma - 1 to remove quotes */
        }
        list.remove(0, end);
    }
    return n;
}

uint32_t TLTFileUtils::downloadFile(String filename, char buf[], uint32_t size, const bool append)
{
    deleteFile(filename);
    int status = 0;

    while (!status)
    {
        if(filename.endsWith(".bin"))
        {
            _rc = _me310->m2m_write_file(filename.c_str(), size, 1, buf);
        }
        else
        {
            _rc = _me310->m2m_write_file(filename.c_str(), size, 0, buf);
        }

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
        deleteFile(filename);
    }
    return downloadFile(filename, (char*) buf, size);
}

uint32_t TLTFileUtils::readFile(const String filename, String &content)
{
    int size = listFile(filename);
    if (!size)
    {
        return 0;
    }

    _rc = _me310->m2m_read(filename.c_str());
    if(_rc == ME310::RETURN_CONTINUE )
    {
        content = _me310->buffer_cstr_raw();
        return content.length();
    }
    return 0;
}

uint32_t TLTFileUtils::readFile(const String filename, uint8_t *content)
{
    int size;
    size = listFile(filename);
    if (size == 0)
    {
        return 0;
    }
    _rc = _me310->m2m_read(filename.c_str());
    if(_rc == ME310::RETURN_CONTINUE )
    {
        if(_me310->buffer_cstr_raw() != NULL)
        {
            memcpy(content, _me310->buffer_cstr_raw(), size);
            return size;
        }
        else
        {
            return 0;
        }
    }
    return 0;
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
    if(_count != 0)
    {
        std::vector<String> *files = new std::vector<String>;
        int num = listFiles(files);
        if(num > 0)
        {
            while (_count > 0)
            {
                String filename = files->at(_count - 1);
                n += deleteFile(filename);
            }
        }
        delete (files);
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
    _rc = _me310->m2m_list();
    if(_rc == ME310::RETURN_VALID)
    {
        int index;
        response = _me310->buffer_cstr_raw();
        index = response.indexOf(fileN);

        if (index > 0)
        {
          /* add 2 to skip '",' */
          String fsize = response.substring(index + filename.length() + 2, response.indexOf("\r\n", index) );
          size = fsize.toInt();
        }
    }
    return size;
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
        int index = response.indexOf("free bytes: ");
        int end = response.indexOf("\r\n", index);

        String pos = response.substring(index + 12, end );

        size = pos.toInt();
    }
    else
    {
        size = 0;
    }

    return size;
}

uint32_t TLTFileUtils::fileCount()
{
    _countFiles();
    return _count;
}

void TLTFileUtils::printFiles()
{
    fileCount();
    std::vector<String> *files = new std::vector<String>;
    Serial.print(_count);
    Serial.print(_count == 1 ? " file" : " files");
    Serial.println(" found.");

    listFiles(files);
    for (int i = 0; i <_count; i++)
    {
        Serial.print("File: ");
        Serial.print(files->at(i));
        Serial.print(" - Size: ");
        Serial.print(listFile(files->at(i)));
        Serial.println();
    }
    delete (files);
}