#ifndef CFREGISTRY_HH
#define CFREGISTRY_HH
/* Copyright (c) 2007-2011  Peter R. Torpman (peter at torpman dot se)

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
#include <map>
#include <string>
using namespace std;

// INCLUDES
#include "compframe_types.h"
#include "CF_Interfaces.hh"

// FORWARD DECLARATIONS
class CFComponent;


// CLASS DECLARATION

// This class is the CompFrame Registry
class CFRegistry : public IRegistry
{
public:
	static IRegistry* instance();

	// Contructor
	CFRegistry();
	// Destructor
	virtual ~CFRegistry();

	// IRegistry methods
	IRegistry* getInstance() { return instance(); }
	int registerLibrary(CFComponentLib* lib);
	int deregisterLibrary(const char* name);
	CFComponent* getCompObject(const char* inst_name);
	char* getCompName(CFComponent* obj);
	CFComponent* createComp(const char *name, const char *inst_name);
	int destroyComp(const char *inst_name);
	int registerIface(CFComponent* comp, IBase* iface);
	int deregisterIfaces(CFComponent* comp);
	IBase* getIface(CFComponent* comp, const char* iface_name);
    void listInstances(void);
    void listClasses(void);
    void listInterfaces(void);
	IBase* getCompIface(string name, const char* iface_name);

	
private:

	// Map of registered components (indexed on name)
	map<string, CFComponentLib*> mCompLibraries;

	// Map of instantiated components (indexed on instance name)
	map<string, CFComponent*> mInstances;

	// Map of instantiated components (indexed on object)
	map<CFComponent*, string> mInstancesReverse;

    // Map of registered interfaces (indexed by interface)
    map<IBase*, void*> mInterfacesByName;
    
    // Map of interfaces implemented by modules
    multimap<CFComponent*, IBase*> mCompInterfaces;
	
};



#endif
