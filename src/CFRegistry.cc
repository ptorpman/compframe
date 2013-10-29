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
#include "CFRegistry.hh"
#include "CFComponent.hh"
#include "CFComponentLib.hh"
#include "compframe_log.h"

// Singleton
static CFRegistry inst;

IRegistry*
CFRegistry::instance()
{
	return (IRegistry*) &inst;
}

CFRegistry::CFRegistry()
{
}

CFRegistry::~CFRegistry()
{
}

int
CFRegistry::registerLibrary(CFComponentLib* lib)
{
	string name = lib->getName();
	map<string,CFComponentLib*>::iterator i = mCompLibraries.find(name);

	if (i != mCompLibraries.end()) {
		cf_error_log(__FILE__, __LINE__,
					 "Component name already used (%s)\n", name.c_str());
		return 1;
	}

	// Add to map
	mCompLibraries[name] = lib;
	
    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Component library- %s - registered!\n", name.c_str());

	return 0;
}

int
CFRegistry::deregisterLibrary(const char* name)
{
	string cName(name);

	map<string,CFComponent*>::iterator i = mInstances.find(cName);

	if (i != mInstances.end()) {
		cf_error_log(__FILE__, __LINE__,
					 "Cannot deregister class when instance exist! (%s)\n",
					 name);
		return 1;
	}

	map<string,CFComponentLib*>::iterator ii = mCompLibraries.find(cName);

	if (ii != mCompLibraries.end()) {
		cf_error_log(__FILE__, __LINE__,
					 "Cannot deregister class. No such class! (%s)\n",
					 name);
		return 1;
	}
	
	delete ii->second;
	mCompLibraries.erase(ii);

	return 0;
}

CFComponent*
CFRegistry::getCompObject(const char* inst_name)
{
	string iName(inst_name);

	map<string,CFComponent*>::iterator i = mInstances.find(iName);

	if (i == mInstances.end()) {
		return NULL;
	}
								   
	return i->second;
}

char*
CFRegistry::getCompName(CFComponent* c)
{
	map<CFComponent*,string>::iterator i = mInstancesReverse.find(c);

	if (i == mInstancesReverse.end()) {
		return NULL;
	}

	return (char*) i->second.c_str();
}

CFComponent*
CFRegistry::createComp(const char *name, const char *inst_name)
{
	map<string, CFComponentLib*>::iterator i = mCompLibraries.find(name);
	map<string, CFComponent*>::iterator ii = mInstances.find(inst_name);

	// Class registered?
	if (i == mCompLibraries.end()) {
        cf_error_log(__FILE__, __LINE__, "Class not registered! (%s)\n", name);
		return NULL;
	}
	
	// Instance created?
	if (ii != mInstances.end()) {
		cf_error_log(__FILE__, __LINE__, "Instance name already used (%s)\n",
					 inst_name);
		return NULL;
	}

	// Create component
	CFComponent* c = i->second->getCreateFunc()(inst_name);

	if (!c) {
        cf_error_log(__FILE__, __LINE__,
                     "Could not create (%s - %s)\n", name, inst_name);
		return NULL;
	}

	// Add to maps
	string iName(inst_name);
	mInstances[iName] = c;
	mInstancesReverse[c] = iName;
	
	// Setup the instance
    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Setting up component %s !\n", name);
	i->second->getSetupFunc()(c);
	
	return c;
}

int
CFRegistry::destroyComp(const char *inst_name)
{
	string iName(inst_name);

	map<string, CFComponent*>::iterator i = mInstances.find(inst_name);

	if (i == mInstances.end()) {
		cf_error_log(__FILE__, __LINE__,
					 "Could not destroy (%s). Not found!\n", inst_name);
		return 1;
	}

	map<CFComponent*, string>::iterator ii = mInstancesReverse.find(i->second);

	if (ii == mInstancesReverse.end()) {
		cf_error_log(__FILE__, __LINE__,
					 "Could not destroy (%s). Not found!\n", inst_name);
		return 1;
	}


	// Clean up maps
	mInstances.erase(i);
	mInstancesReverse.erase(ii);
	
	return 0;
}

int
CFRegistry::registerIface(CFComponent* comp, IBase* iface)
{  
	mCompInterfaces.insert(pair<CFComponent*, IBase*>(comp, iface));

    return 0;
}

int
CFRegistry::deregisterIfaces(CFComponent* comp)
{
    multimap<CFComponent*, IBase*>::iterator i;

    i = mCompInterfaces.find(comp);

    mCompInterfaces.erase(i);

    return 0;
}

IBase*
CFRegistry::getIface(CFComponent* comp, const char* iface_name)
{
    string iStr;
    iStr.assign(iface_name);

    multimap<CFComponent*, IBase*>::iterator it;
    pair<multimap<CFComponent*, IBase*>::iterator,multimap<CFComponent*, IBase*>::iterator> ret;
    
    ret = mCompInterfaces.equal_range(comp);

    for (it = ret.first; it != ret.second; ++it) {
		if ((*it).second->getName() == iStr) {
            return (IBase*)(*it).second;
        }
    }
    
    // Could not find it
	fprintf(stderr, "ERROR: Did not find interface - %s!!\n", iface_name);
    return NULL;
}

void 
CFRegistry::listInstances(void)
{
    fprintf(stdout, "%-32s %s\n", "Instance", "Class");
    fprintf(stdout, "------------------------------------------------------\n");
    
    map<string, CFComponent*>::iterator i = mInstances.begin();

    for (; i != mInstances.end(); ++i) {
        fprintf(stdout, "%-32s \n", i->first.c_str() /*tmp->className*/);
    }

}

void 
CFRegistry::listClasses(void)
{
}

void 
CFRegistry::listInterfaces(void)
{
}



IBase*
CFRegistry::getCompIface(string name, const char* iface_name)
{
	CFComponent* cObj = getCompObject(name.c_str());
	
	if (!cObj) {
		fprintf(stderr, "ERROR: Component not found!\n");
		return NULL;
	}

	return getIface(cObj, iface_name);
}


IRegistry* 
cfGetRegistry()
{
    return CFRegistry::instance();
}



