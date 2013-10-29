#ifndef CF_S_HH
#define CF_S_HH

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

//=============================================================================
//                        I N C L U D E S
//=============================================================================
#include "CFComponent.hh"
#include "IScheduler.hh"

#include <map>
using namespace std;

//=============================================================================
//                          M A C R O S 
//=============================================================================

//=============================================================================
//                           T Y P E S
//=============================================================================

// Scheduler states 
typedef enum {
    CF_S_IDLE = 0,
    CF_S_RUNNING = 1,
    CF_S_STOPPED = 2
} SchedulerState_t;

//=============================================================================
//                     E N U M E R A T I O N S
//=============================================================================

//=============================================================================
//                   G L O B A L  V A R I A B L E S
//=============================================================================

//=============================================================================
//                       C O N S T A N T S 
//=============================================================================


/** This class implements the S (Scheduler) component of CompFrame */
class CF_Scheduler :
	public CFComponent,
    public ISchedulerServer,
    public ISchedulerControl
{
public:
    // Constructor
    CF_Scheduler(const char *inst_name);
    // Destructor
    virtual ~CF_Scheduler();

    // 
    // ISchedulerServer methods
    int add(CFComponent *obj);
    int remove(CFComponent *obj);

    // 
    // ISchedulerControl methods
    int loop();
    /** Perform scheduling operations */
    int schedule();
    /** Start scheduling loop  */
    int start();
    /** Stop scheduling loop  */
    int stop();

private:
    // Instance name
    string mName;
    // State
    SchedulerState_t mState;
    // Time slice
    int mSlice;
    // Map of scheduled components
    map<CFComponent*,ISchedulerClient*> mClients;
};


#endif
