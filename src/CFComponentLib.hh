#ifndef CFCOMPONENTLIB_HH
#define CFCOMPONENTLIB_HH
/* Copyright (c) 2007-2010  Peter R. Torpman (peter at torpman dot se)

   This file is part of CompFrame (http://compframe.sourceforge.net)

   CompFrame is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   CompFrame is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.or/licenses/>.
*/
#include <string>
using namespace std;

#include "compframe_types.h"
#include "CFComponent.hh"

// This class can be used as a base class for any implemented component.

class CFComponentLib
{
public:
  /** Constructor
      @param name  Name of component library
      @param fp    Pointer to function used to create instance.
      @param sc    Pointer to function used to setup instance.
      @param df    Pointer to function used to delete instance.
  */
  CFComponentLib(string name, cf_callback_create_t fp,
		 cf_callback_setup_t sf, cf_callback_destroy_t df) :
    mName(name),
    mCreateFunc(fp),
    mSetupFunc(sf),
    mDestroyFunc(df) {
  }
		
  // Destructor
  ~CFComponentLib() { }

  // Returns the class name
  string getName() { return mName; }
  // Returns the create function
  cf_callback_create_t getCreateFunc() { return mCreateFunc; }
  // Returns the setup function
  cf_callback_setup_t getSetupFunc() { return mSetupFunc; }
  // Returns the destroy function
  cf_callback_destroy_t getDestroyFunc() { return mDestroyFunc; }

	
	
private:
  // Component class name
  string mName;
  // Function used to create an instance
  cf_callback_create_t  mCreateFunc;
  // Function to setup an instance
  cf_callback_setup_t   mSetupFunc;
  // Function used to delete instance
  cf_callback_destroy_t mDestroyFunc;
};


#endif
