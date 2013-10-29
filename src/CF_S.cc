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
#define _POSIX_SOURCE 1                           /* POSIX compliant */

//=============================================================================
//                              I N C L U D E S
//=============================================================================
#include "CF_S.hh"
#include "CFRegistry.hh"
#include "compframe_log.h"
#include "compframe_sockets.h"
#include "CFComponentLib.hh"
#include <assert.h>
//=============================================================================
//                      G L O B A L  V A R I A B L E S
//=============================================================================


//=============================================================================
//                        H E L P E R   C L A S S E S
//=============================================================================

//=============================================================================
//                        P U B L I C   M E T H O D S
//=============================================================================

//=============================================================================
//                       P R I V A T E   M E T H O D S
//=============================================================================
static CFComponent *create_me(const char *inst_name);
static void set_me_up(CFComponent *comp);
static int destroy_me(CFComponent *comp);

static CFComponentLib theLib("S", create_me, set_me_up, destroy_me);


CF_Scheduler::CF_Scheduler(const char *inst_name) :
	CFComponent("S"),
	mName(inst_name),
	mState(CF_S_IDLE),
	mSlice(10)
{
	
}

// Destructor
CF_Scheduler::~CF_Scheduler()
{
    if (mClients.size() != 0) {
        cf_error_log(__FILE__, __LINE__,
                     "Scheduled components still active!\n");
    }

    CFRegistry::instance()->deregisterIfaces(this);
}

// 
// CF_S_ServerIf methods
int 
CF_Scheduler::add(CFComponent *obj)
{
    map<CFComponent*,ISchedulerClient*>::iterator i = mClients.find(obj);

    if (i != mClients.end()) {
        cf_error_log(__FILE__, __LINE__,
                     "Could not add component again to scheduler loop!\n");
        return 1;
    }

    ISchedulerClient* iFace =  (ISchedulerClient*)
        CFRegistry::instance()->getIface(obj, "ISchedulerClient");

    if (!iFace) {
        cf_error_log(__FILE__, __LINE__,
                     "Component does not implement S client interface!\n");
        return 1;
    }

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_INFO,
                 "Added %s to scheduler...\n", obj->getClassName().c_str());

	
    // Store interface
    mClients[obj] = iFace;

    return 0;
}

int 
CF_Scheduler::remove(CFComponent *obj)
{
    map<CFComponent*,ISchedulerClient*>::iterator i = mClients.find(obj);

    if (i == mClients.end()) {
        cf_error_log(__FILE__, __LINE__,
                     "Could not find component in scheduler loop!\n");
        return 1;
    }

    // Remove client from scheduling
    mClients.erase(i);

    return 0;
}

// 
// CF_S_ControlIf methods
int 
CF_Scheduler::loop()
{
    mState = CF_S_RUNNING;

    while (1) {
        cf_sockets_poll();
        
        schedule();

    }

    return 0;
}

int 
CF_Scheduler::schedule()
{
    if (mState != CF_S_RUNNING) {
        cf_trace_log(__FILE__, __LINE__, CF_TRACE_MASSIVE,
                     "Scheduler not started...\n");
        return 1;
    }

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_MASSIVE,
                 "Scheduling components...\n");

    map<CFComponent*,ISchedulerClient*>::iterator i = mClients.begin();
    
    for ( ; i != mClients.end(); ++i) {
        i->second->execute(mSlice);
    }

    return 0;
}

int 
CF_Scheduler::start()
{
    cf_info_log("S scheduling commencing. Using timeslice %u.\n", mSlice);

    mState = CF_S_RUNNING;

    return 1;
}

int 
CF_Scheduler::stop()
{
    cf_info_log("S scheduling stopped.\n");

    mState = CF_S_STOPPED;
    
    return 0;
}


/** This function must reside in all component libraries.
    Here the component instance is 
*/
extern "C" void
dlopen_this(void)
{
    /* Use this function to get S into the Registry  */
	CFRegistry::instance()->registerLibrary(&theLib);
}

/** This function is used to create and initate a component. 
    @return Pointer to created instance or NULL 
*/
static CFComponent *
create_me(const char *inst_name)
{
    assert(inst_name != NULL);

    CF_Scheduler* s = new CF_Scheduler(inst_name);

    return s;
}

static int
destroy_me(CFComponent *comp)
{
    if (!comp) {
        return 1;
    }

    CF_Scheduler *s = (CF_Scheduler*) comp;

    delete s;

    return 0;
}

/** Function called after S has been created */
static void
set_me_up(CFComponent *comp)
{
	CF_Scheduler* s = (CF_Scheduler*) comp;

	CFRegistry::instance()->registerIface(comp,(ISchedulerServer*)s);
    CFRegistry::instance()->registerIface(comp,(ISchedulerControl*)s);
}


