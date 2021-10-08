/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*!
  @file
    TLTFileUtils.h

  @brief
    TLT File Utils class
  @details
    
  
  @version 
    1.1.0

  @note
    Dependencies:
    ME310.h
    Arduino.h

  @author
    Cristina Desogus

  @date
    08/02/2021
*/

#ifndef __TLTFILEUTILS__H
#define __TLTFILEUTILS__H

#pragma once
/* Include files ================================================================================*/
#include <ME310.h>
#include <Arduino.h>
/* Using namespace ================================================================================*/
using namespace std;
using namespace me310;

/* Class definition ================================================================================*/

class TLTFileUtils
{
    public:
    
        TLTFileUtils(ME310* me310, bool debug = false);

        bool begin(const bool restart);
        bool begin() { return begin(true); };

        int existFile(const String filename);

        uint32_t fileCount() const { return _count; };
        size_t listFiles(String list[]) const;
        uint32_t listFile(const String filename);

        uint32_t downloadFile(const String filename,  char buf[], const uint32_t size, const bool append);
        uint32_t downloadFile(const String filename,  char buf[], const uint32_t size) { return downloadFile(filename, buf, size, false); };
        uint32_t downloadFile(const String filename, const String& buf) { return downloadFile(filename, (char*) buf.c_str(), buf.length()); }

        uint32_t createFile(const String filename, const char buf[], uint32_t size);

        uint32_t appendFile(const String filename, const String& buf)                     { return downloadFile(filename, (char*)buf.c_str(), buf.length(), true); }
        uint32_t appendFile(const String filename, const char buf[], const uint32_t size) { return downloadFile(filename, (char*)buf, size, true); }

        bool deleteFile(const String filename);
        int deleteFiles();

        uint32_t readFile(const String filename, String* content);
        uint32_t readFile(const String filename, uint8_t* content);

        uint32_t freeSpace();

    private:
        int _count;
        String _files;
        bool _debug;
        void _countFiles();
        int _getFileList();
        ME310* _me310;
        ME310::return_t _rc;
};

#endif //__TLTFILEUTILS__H