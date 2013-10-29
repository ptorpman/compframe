#ifndef CF_C_HH
#define CF_C_HH
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

/** @addtogroup c C - Command Handler
  * @{
  */

#define WANT_TCL_COMMANDS 1

//=============================================================================
//                        I N C L U D E S
//=============================================================================
#include "CFComponent.hh"
#include "compframe.h"
#include "compframe_sockets.h"
#include "ICommand.hh"
#include "IScheduler.hh"
#include <map>
#include <vector>
using namespace std;

#ifdef WANT_TCL_COMMANDS
#include <tcl.h>
#endif


//=============================================================================
//                          M A C R O S 
//=============================================================================

//=============================================================================
//                           T Y P E S
//=============================================================================

typedef struct Command_t 
{
    // Command name
    string mName;
    // Command function
    cfc_func_t mFunc;
    // Help string
    string mHelpString;
    // Component that implements the command
    CFComponent* mComp;
   
} Command_t;

//=============================================================================
//                     E N U M E R A T I O N S
//=============================================================================

//=============================================================================
//                   G L O B A L  V A R I A B L E S
//=============================================================================

//=============================================================================
//                       C O N S T A N T S 
//=============================================================================

class CF_CmdHandler :
    public CFComponent,
    public ICommand,
    public ISchedulerClient
{
public:
    /** Constructor */
    CF_CmdHandler(const char *inst_name);
    /** Destructor */
    virtual ~CF_CmdHandler();

    // ICommand methods

    int add(CFComponent* obj, const char* name, cfc_func_t func, char* usage);
    /** Remove a command from the handler */
    int remove(const char* name);
    /** Handle a command, based on a command string */
    int handle(char* cmdStr);

    // ISchedulerClient methods
    void execute(uint32_t slice);


#ifdef WANT_TCL_COMMANDS
    Tcl_Interp* getTcl() { return mTclInterp; }
#endif

    // Lists available commands
    void listCommands();
    // Prints command help
    void printCommandHelp(char* cmd);


private:
    // Instance name
    string mName;
    // Time slice
    int mSlice;
    // Map of commands (indexed on command name)
    map<string, Command_t*> mCommands;


#ifdef WANT_TCL_COMMANDS
    /** Tcl interpreter  */
    Tcl_Interp *mTclInterp;
#endif

};



/** @}  Doxygen end marker */ 

#endif
