/*
  This file is part of the MKR NB library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

// Timeouts from https://www.sparkfun.com/datasheets/Cellular%20Modules/AT_Commands_Reference_Guide_r0.pdf

#include "NBContacts.h"

NBContacts::NBContacts() : 
  _maxContacts(0),
  _maxNameLength(0),
  _maxNumberLength(0)
{
  //MODEM.debug();
}

NBContacts::~NBContacts() 
{
}

int NBContacts::begin()
{
  int start;
  String response;
  response.reserve(30);

  MODEM.send("AT+CPBS=\"SM\""); // Choose SIM phone book
  if(MODEM.waitForResponse(5000) != 1) {
    return 0;
  }
  MODEM.send("AT+CPBR=?");      // Query phone book max sizes 
  if(MODEM.waitForResponse(20000,&response) != 1) {
    return 0;
  }
  start = response.indexOf("-");
  if (start > 0) {
    response.remove(0,start+1);
    _maxContacts = response.toInt();
  } else {
    return 0;
  }
  start = response.indexOf(",");
  if (start > 0) {
    response.remove(0,start+1);
    _maxNameLength = response.toInt();
  } else {
    return 0;
  }
  start = response.indexOf(",");
  if (start > 0) {
    response.remove(0,start+1);
    _maxNumberLength = response.toInt();
  } else {
    return 0;
  }
  
  return 1;
}

int NBContacts::maxContacts()
{
  return _maxContacts;
}

int NBContacts::maxNameLength()
{
  return _maxNameLength;
}

int NBContacts::maxNumberLength() 
{
  return _maxNumberLength;
}

int NBContacts::remove(int index) 
{
  MODEM.sendf("AT+CPBW=%i",index);
  if(MODEM.waitForResponse(20000) != 1) {
    return 0;
  }
  return 1;
}

int NBContacts::update(int index, const char* number, const char* name, int type) 
{
  if( index < 0 or index >= _maxContacts ) {
    return -1; 
  }
  if (name == nullptr) {
    name = number;
  }
  if(index > 0) {
    MODEM.sendf("AT+CPBW=%i,\"%s\",%i,\"%s\"",index, number, type, name);
  } else {
    MODEM.sendf("AT+CPBW=,\"%s\",%i,\"%s\"", number, type, name);
  }
  if(MODEM.waitForResponse(20000) != 1) {
    return 0;
  }
  return 1;
}

int NBContacts::add(const char* name, const char* number, int type) 
{
  return update(0, name, number, type);
}

int NBContacts::get(int index, String* number, String* name, int* type)
{
  MODEM.sendf("AT+CPBR=%i",index);
  return parseResponse(nullptr, number, name, type);
}

int NBContacts::search(const char* name, String* number, int* index, int* type)
{  
  MODEM.sendf("AT+CPBF=\"%s\"",name);
  return parseResponse(index, number, nullptr, type);
}

int NBContacts::parseResponse(int* index, String* number, String* name, int* type)
{
  String response((char*)0);
  response.reserve(30+_maxNameLength+_maxNumberLength);

  if (MODEM.waitForResponse(20000, &response)==1 
      && (response.startsWith("+CPBF: ") || response.startsWith("+CPBR: "))) {
    response.remove(0,7);
    if (index != nullptr) {
      *index = response.toInt();
    }
    int start = response.indexOf("\"")+1;       // number starts after first "
    int stop = response.indexOf("\"",start);    // number stops at second "
    if (number != nullptr) {
      *number = response.substring(start,stop);
    }
    response.remove(0,stop+2);                  // type starts after subsequent comma    
    if (type != nullptr) {
      *type = response.toInt();
    }
    if (name != nullptr) {
      start = response.indexOf("\"")+1;         // name starts after third "
      stop = response.indexOf("\"",start);      // name stops at fourth "
      *name = response.substring(start,stop);
    }
    return 1;
  }
  return 0;
}
