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

/** @addtogroup samplecomps1 Sample Component #1
 *  @{
 */

/** This is a sample component */
#include "compframe.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "IConnect.hh"
#include "IConfig.hh"
#include "CFComponentLib.hh"
#include "IRegistry.hh"
#include "iface.hh"                     // Test interfaces


/* Predeclaration of function used later on. */
static CFComponent* create_me(const char* inst_name);
static void         set_me_up(CFComponent* comp);
static int          destroy_me(CFComponent* comp);

// The library container
static CFComponentLib theLib("TESTCOMP", create_me, set_me_up, destroy_me);

// Test component
class TestComp : 
    public CFComponent,
    public ITest,
    public IConnect,
    public IConfigClient
{
public:
    TestComp(const char* instName) : 
            CFComponent(instName),
            mName(instName), mConnection(NULL) { 
    }
    virtual ~TestComp() { }

    // ITest methods
    void printHello(void);
    void print(char *str);
    
    // IConnect methods
    int connect(CFComponent *other, char *iface,
                int argc, char **argv);
    int disconnect(CFComponent *other, char *iface);

    // IConfigClient methods
    int set(char* varName, char* varValue);

private:
    // Instance name
    string mName;

    // Pointer to other side's interface
    ITest2* mConnection;
};



/** This function must reside in all component libraries.
    Here the component instance is 
*/
extern "C" void 
dlopen_this(void)
{
    /* Use this function to get M into the Registry  */
	cfGetRegistry()->registerLibrary(&theLib);
}


/** This function is used to create and initate a component. 
    @return Pointer to created instance or NULL 
*/
static CFComponent* 
create_me(const char* inst_name)
{
    assert(inst_name!=NULL);
    
    return new TestComp(inst_name);
}

static int  
destroy_me(CFComponent* comp)
{
    if (!comp) {
        return 1;
    }

    /* De-register our interfaces */
    cfGetRegistry()->deregisterIfaces(comp);

    delete (TestComp*)comp;

    return 0;
}


static void
set_me_up(CFComponent* comp)
{ 
    TestComp* t = (TestComp*) comp;

    cfGetRegistry()->registerIface(comp, (ITest*)t);
    cfGetRegistry()->registerIface(comp, (IConnect*)t);
    cfGetRegistry()->registerIface(comp, (IConfig*)t);
}


void  
TestComp::printHello(void)
{
    fprintf(stderr,"Hello!\n");
}

void  
TestComp::print(char* str)
{
    fprintf(stderr,"%s\n",str);
}


int 
TestComp::connect(CFComponent* other, char* iface, int argc, char** argv)
{
    (void) argc;                  /* Avoid warnings */
    (void) argv;                  /* Avoid warnings */

    if (mConnection) {
        fprintf(stderr,"%s:%d Already connected\n",__FILE__, __LINE__);
        return 0;
    }

    if (strcmp(iface, TEST2IFACE_ID)) {
        fprintf(stderr,
                "%s:%d Interface not implemented by this comp\n",
                __FILE__, __LINE__);
        return 1;
    }

    mConnection = (ITest2*) cfGetRegistry()->getIface(other,iface);

    if (!mConnection) {
        fprintf(stderr,
                "%s:%d Interface not implemented by peer\n",
                __FILE__, __LINE__);
        return 1;
    }

    fprintf(stderr," Connecting....: %d\n", argc);

    for (int i = 0; i < argc; i++) {
        fprintf(stderr,"      argument ARGV[%d]: (%s)\n", i, argv[i]);
    }

    /* Just call a function in the interface to show that it is possible */
    if (mConnection->getId() == TEST2IFACE_ID) {
        mConnection->printGoodbye();
    }

    return 0;
}

int
TestComp::disconnect(CFComponent* other, char* iface)
{
    (void) other;  
    (void) iface;
    return 1;
}

int
TestComp::set(char* varName, char* varValue)
{
    fprintf(stderr,"  %s: Setting %s to '%s'\n",
            __FILE__, varName, varValue);

    return 1;
}


/** @} */
