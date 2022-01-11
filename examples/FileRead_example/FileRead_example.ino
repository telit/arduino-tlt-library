/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    TLTMDM.h
    string.h
    stdio.h

  @brief
    Sample test of the use of FileUtils class in TLT library.

  @details
    In this example sketch, it is shown to use file system management, using FileUtils library.\n
    Add file in directory, get list of directory, delete files and directory functions are shown.\n


  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    10/08/2021
 */
// libraries
#include <TLTMDM.h>
using namespace me310;

// initialize the library instance
ME310* myME310 = new ME310();
TLTFileUtils fileUtils(myME310, true);

char filename[] = "testFile.txt";
char fileBin[] = "testBin.bin";


void setup() {
  Serial.begin(115200);
  myME310->begin(115200);

  delay(5000);
  Serial.println("Starting Arduino file demo.");

  myME310->powerOn(ON_OFF);

  fileUtils.begin();
  Serial.print("Free space: ");
  Serial.print(fileUtils.freeSpace());
  Serial.println(" Bytes.");

  Serial.print("Count File: ");
  Serial.println(fileUtils.fileCount());

  Serial.println("Files list before new file creation.");
  fileUtils.printFiles();

  String buf = "Hello World";
  Serial.println("Create File");
  fileUtils.createFile(filename, buf.c_str(), buf.length());

  Serial.println("Files List after create file.");
  fileUtils.printFiles();


  Serial.println("Check file.");
  Serial.print(filename);
  Serial.print(": ");
  if (fileUtils.existFile(filename))
  {
    Serial.println("File present!");

    uint32_t size = fileUtils.listFile(filename);
    Serial.print("Size file: ");
    Serial.println(size);

    String contentFile;
    Serial.println("Read File: <");
    fileUtils.readFile(filename, contentFile);
    Serial.println(contentFile.c_str());
    Serial.println(">");

    uint8_t contentFileUint[size+1] = {0};
    Serial.println("Read File Uint8: <");
    fileUtils.readFile(filename, contentFileUint);
    Serial.println((char*)contentFileUint);
    Serial.println(">");

    Serial.println("Delete File.");
    fileUtils.deleteFile(filename);

    Serial.println("Files List after delete file.");
    fileUtils.printFiles();
  }
  else
  {
    Serial.println("File does not present!");
  }

  Serial.println("Create a binary file");
  uint8_t bufferBin[] = {0,1,2,3,4,5,6,7,8,9};

  fileUtils.createFile(fileBin, (char*)bufferBin, 10);
  Serial.println("Files List after create binary file.");
  fileUtils.printFiles();

  Serial.println("Check file.");
  Serial.print(fileBin);
  Serial.print(": ");
  if (fileUtils.existFile(fileBin))
  {
    Serial.println("File present!");

    uint32_t size = fileUtils.listFile(fileBin);
    Serial.print("Size file: ");
    Serial.println(size);

    if(size > 0)
    {
      uint8_t contentFileUint[size+1] = {0};
      if(((size*2)+1) < INT8_MAX)
      {
         int tmp_size = (size*2)+1;
        uint8_t tmp_contFileUint[tmp_size] = {0};
        Serial.println("Read File Uint8: <");
        fileUtils.readFile(fileBin, contentFileUint);
        myME310->ConvertBufferToIRA(contentFileUint, tmp_contFileUint, size);
        Serial.println((char*)tmp_contFileUint);
        Serial.println(">");
      }
    }
    Serial.println("Delete File.");
    fileUtils.deleteFile(fileBin);

    Serial.println("Files List after delete file.");
    fileUtils.printFiles();
  }
  else
  {
    Serial.println("File does not present!");
  }


  Serial.println("end");
}

void loop() {
}