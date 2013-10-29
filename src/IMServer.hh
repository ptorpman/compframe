#ifndef CF_MSERVER_HH
#define CF_MSERVER_HH
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

/** Textual name of the M (Message Transport Component) server interface,
    that is implemented by the M component. */
#define IMSERVER_ID  "5f715f86-3e01-11e0-811e-00219b221678"

/** IMServer interaface - implemented by the M component */
class IMServer : public IBase
{
public:
	// Constructor
	IMServer() : IBase("IMServer", IMSERVER_ID) {}
	// Destructor
	virtual ~IMServer() {}

    /** Add a message receiver
        @param comp     Pointer to own context
        @param uuid     UUID string for the interface 
        @param name     Name of message receiver
        @param userData User's data
        @return 1 if OK, 0 if failure
    */
    virtual int addReceiver(CFComponent *comp, const char *uuid,
					   char *name, void *userData) = 0;

    /** Enable message receiver, i.e allow connections to this receiver.
        @note Receivers are enabled by default.
        @param uuid UUID string for the interface 
        @param name Name of message receiver
        @return 1 if OK, 0 if failure
    */
    virtual int enableReceiver(const char *uuid, char *name) = 0;

    /** Disable message receiver, i.e do not allow any more connections
        to this receiver.
        @param uuid UUID string for the interface 
        @param name Name of message receiver
        @return 1 if OK, 0 if failure
    */
    virtual int disableReceiver(const char *uuid, char *name) = 0;

    /** Remove message receiver
        @param uuid UUID string of the interface 
        @param name Name of message receiver
        @return 1 if OK, 0 if failure
    */
	virtual int rmReceiver(const char *uuid, char *name) = 0;

    /** Returns the port of the M server 
        @return the port number of -1 if not started.
    */
    virtual int getServerPort() = 0;

    /** Returns a colon separated string with message receivers
        with specific names (name="M" will return all that start
        with 'M')
        @param name String with name or prefix.
        @return the port number of -1 if not started.
    */
    virtual char* searchByName(char *name) = 0;

    /** Returns a colon separated string with message receivers
        with a specific interface.
        @param uuid UUID string of interface.
        @return the port number of -1 if not started.
    */
    virtual char *searchByIface(const char *uuid) = 0;

    /** Send a message to a receiver */
    virtual int sendToReceiver(void *conn, uint8_t chan, int len,
					 unsigned char *msg) = 0;

};












/** @} */
#endif
