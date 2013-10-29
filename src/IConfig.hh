#ifndef ICONFIG_HH
#define ICONFIG_HH
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "IBase.hh"

/** @addtogroup Interfaces
 *  These are the public interfaces of CompFrame
 *  @{
 */

/** Textual name of the Cfg server interface, that is implemented by 
    Cfg (the configurator). */
#define ICONFIG_ID  "ea513ec5-4d35-47a8-982e-d7daef5343bc"

/** Interface of the Config client */
class IConfig  : public IBase
{
  public:
    /** Constructor */
    IConfig() : IBase("IConfig",ICONFIG_ID) {}
    /** Destructor */
    virtual ~IConfig() {};

    /** Sets an integer type variable
        @param cfgFile  Config file name
        @return 1 if OK, 0 if failure
    */
    virtual int parse(char* cfgFile) = 0;
};


/** Textual name of the Cfg client interface, that is implemented by 
    components that wants to be configured . */
#define ICONFIG_CLIENT_ID  "8d52b306-97e4-40bb-bd43-f8a811de4fc6"

/** Interface type */
class IConfigClient  : public IBase
{
  public:
    /** Constructor */
    IConfigClient() : IBase("IConfigClient", ICONFIG_CLIENT_ID) {}
    /** Destructor */
    virtual ~IConfigClient() {};

    /** Sets an integer type variable
        @param varName  Variable name
        @param varValue Variable value
        @return 1 if OK, 0 if failure
    */
    virtual int set(char* varName, char* varValue) = 0;
};

/** @} */


#endif
