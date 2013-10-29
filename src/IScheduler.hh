#ifndef ISCHEDULER_HH
#define ISCHEDULER_HH
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
#include "IBase.hh"
#include <stdint.h>

class CFComponent;

/** @addtogroup Interfaces
 *  These are the public interfaces of CompFrame
 *  @{
 */

/** Textual name of the S interface that is used to let S do its
 * scheduling operations.
 */
#define ISCHEDULER_CONTROL_ID  "855cf332-9248-40e4-a48a-ec5fdb9d1981"

/** Control interface of the S component */
class ISchedulerControl  : public IBase
{
  public:
    /** Constructor */
    ISchedulerControl() : IBase("ISchedulerControl", ISCHEDULER_CONTROL_ID) {}
    /** Destructor */
    virtual ~ISchedulerControl() {};

    /** Enter eternal loop of scheduler. */
    virtual int loop() = 0;
    /** Perform scheduling operations */
    virtual int schedule() = 0;
    /** Start scheduling loop  */
    virtual int start() = 0;
    /** Stop scheduling loop  */
    virtual int stop() = 0;
};


/** Textual name of the S interface that is used by components to register
 *  within S, so that they may be put into the scheduling loop.
 *  @note Components must implement 'CF_S_ClientIf' if they want to be scheduled.
 */
#define ISCHEDULER_SERVER_ID "de04984d-d23c-448e-b1e8-b799067abd44"

/** S interface that is used by components to register
 *  within S, so that they may be put into the scheduling loop.
 *  @note Components must implement 'cfi_s_client' if they want to be scheduled.
 */
class ISchedulerServer : public IBase
{
  public:
    /** Constructor */
    ISchedulerServer() : IBase("ISchedulerServer", ISCHEDULER_SERVER_ID) {}
    /** Destructor */
    virtual ~ISchedulerServer() {};

    /** Register in S server.
     *  @param obj         Pointer to scheduled component
     *  @return 1 if OK, 0 if not.
     */
    virtual int add(CFComponent *obj) = 0;

    /** Deregister in S server.
     *  @param obj         Pointer to scheduled component
     *  @return 1 if OK, 0 if not.
     */
    virtual int remove(CFComponent *obj) = 0;
};

/** Textual name of the S interface used by S to tell scheduled components
 *  that it is their time to execute.
 */
#define ISCHEDULER_CLIENT_ID "ca3bef4f-19cf-4c32-ae01-0b5f71ff1d38"

/** Interface used by S to tell scheduled components
 *  that it is their time to execute.
 */
class ISchedulerClient : public IBase
{
  public:
    /** Constructor */
    ISchedulerClient() : IBase("ISchedulerClient", ISCHEDULER_CLIENT_ID) {}
    /** Destructor */
    virtual ~ISchedulerClient() {};

    /** Perform execution
     *  @param slice       Number of milliseconds available for execution
     */
    virtual void execute(uint32_t slice) = 0;
};
















/** @}   Doxygen end marker */

#endif 
