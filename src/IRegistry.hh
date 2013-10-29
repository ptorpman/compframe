#ifndef IREGISTRY_HH
#define IREGISTRY_HH

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

#include "IBase.hh"


class CFComponentLib;
class CFComponent;

/** @addtogroup Interfaces
 *  These are the public interfaces of CompFrame
 *  @{
 */

/** Textual name of the IRegistry interface */
#define IREGISTRY_ID  "a9367764-e807-4eb3-acec-43df3364edbe"

// Registry Interface
// Used by: CF Users
// Implemented by: The CF Registry
class IRegistry :
	public IBase
{
public:
	/** Constructor */
	IRegistry() : IBase("IRegistry", IREGISTRY_ID) {}

	/** Returns pointer to Regigistry singleton */
	virtual IRegistry* getInstance() = 0;
	
	/** This function is used to register a component library into the
		framework. The component must have a unique class name.
	 *  @param lib   Pointer to component library info 
	 */
	virtual int registerLibrary(CFComponentLib* lib) = 0;
	
	/** This function is used to deregister from the framework.
		@param name  Class name of component 
	*/
	virtual int deregisterLibrary(const char* name) = 0;
	
	/** This function returns a pointer to a component with a specific name
	 *  @param inst_name   Instance name
	 *  @return Pointer or NULL
	 */
	virtual CFComponent* getCompObject(const char* inst_name) = 0;

	/** This function returns the name of a specific component instance
	 *  @param obj   Instance object
	 *  @return Name or NULL
	 */
	virtual char* getCompName(CFComponent* obj) = 0;

	/** This function is used to create a component
		@param name       Class name
		@param inst_name  Instance name
		@return Pointer to component object or NULL
	*/
	virtual CFComponent* createComp(const char *name, const char *inst_name) = 0;

	/** This function is used to destroy a component
		@param inst_name  Instance name
	*/
	virtual int destroyComp(const char *inst_name) = 0;

	/** This function is used to register the interfaces of the component.
	 *  @param comp        Pointer to component
	 *  @param iface       Interface to add
	 *  
	 *  Note: NULL terminate iface_names array to indicate end of list.
	 *        There must be the same amount of entries in iface_names
	 *        and iface_ptrs.
	 */
	virtual int registerIface(CFComponent* comp, IBase* iface) = 0;

	/** This function is used to deregister the interfaces of a component.
	 *  @param comp         Pointer to component
	 */ 
	virtual int deregisterIfaces(CFComponent* comp) = 0;

	/** Returns a pointer to an interface for a specific component
		@param comp        Pointer to component instance
		@param iface_name  Name of interface
	*/
	virtual IBase* getIface(CFComponent* comp, const char* iface_name) = 0;

	/** Prints the active instances to stdout */
    virtual void listInstances(void) = 0;
	/** Prints the available classes to stdout */
    virtual void listClasses(void) = 0;
	/** Prints the available interfaces to stdout */
    virtual void listInterfaces(void) = 0;

	/** Returns component interface or NULL */
	virtual IBase* getCompIface(string name, const char* iface_name) = 0;
	
};

/** Used to get the registry singleton */
extern IRegistry* cfGetRegistry();

/** @} */



#endif
