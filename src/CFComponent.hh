#ifndef CFCOMPONENT_HH
#define CFCOMPONENT_HH
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


// This class can be used as a base class for any implemented component.

class CFComponent
{
public:
	/** Constructor
		@param className     Class name of component
	*/
	CFComponent(string className) :
		mName(className) {
	}
	
	// Returns the class name
    string getClassName() { return mName; }
	
private:
	// Component class name
	string mName;
};


/** Type used for class constructor. */
typedef CFComponent *(*cf_callback_create_t) (const char *name);

/** Type used for class setup. */
typedef void (*cf_callback_setup_t) (CFComponent *comp);

/** Type used for class destruction. 
    @note Must return 0 if operation is successful!
*/
typedef int (*cf_callback_destroy_t) (CFComponent *comp);





#endif
