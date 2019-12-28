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

#ifndef _NB_CONTACTS_H_INCLUDED
#define _NB_CONTACTS_H_INCLUDED

#define NATIONAL_NUMBER 129
#define NATIONAL_NUMBER_ALT 161
#define INTERNATIONAL_NUMBER 145
#define NETWORK_NUMBER 177

#include <Arduino.h>
#include "Modem.h"

class NBContacts {
public:
  NBContacts();
  ~NBContacts(); 

  int begin();

  // Add a new contact, returns 1 on success, 0 on failure
  // If name pointer is NULL use number as name
  // Default type is NATIONAL_NUMBER
  int add(const char* number, const char* name = nullptr, int type = NATIONAL_NUMBER);

  // Update a contact by index, see above
  int update(int index, const char* number, const char* name = nullptr, int type = NATIONAL_NUMBER);

  // Remove a contact by index, returns 1 on success, 0 on failure
  int remove(int index);

  // Retreive a contact by index, returns 1 on success, 0 on failure
  int get(int index, String* number = nullptr, String* name = nullptr, int* type = nullptr);

  // Search for a contact by name, returns index if found or -1 on failure
  int search(const char* name, String* number = nullptr, int* index = nullptr, int* type = nullptr);

  int maxContacts();
  int maxNameLength();
  int maxNumberLength();

private:
  int _maxContacts;
  int _maxNameLength;
  int _maxNumberLength;
  
  int parseResponse(int* index, String* number, String* name, int* type);
};

#endif
