#ifndef ICOMMAND_HH
#define ICOMMAND_HH
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

class CFComponent;

/** @addtogroup Interfaces
 *  These are the public interfaces of CompFrame
 *  @{
 */



/** Type used for commands */
typedef int (*cfc_func_t)(int argc, char** argv);

/** Textual name of the C interface that is used to add commands
 *  to the command handler
 */
#define ICOMMAND_ID  "9dc8cdb6-7d05-4d09-9f59-4f70d16ae3c0"

/** Interface of the Config client */
class ICommand  : public IBase
{
public:
  /** Constructor */
  ICommand() : IBase("ICommand", ICOMMAND_ID) {}
  /** Destructor */
  virtual ~ICommand() {};

  /** Add a command to the handler */
  virtual 
  int add(CFComponent* obj, const char* name,
	  cfc_func_t func, char* usage) = 0;
  /** Remove a command from the handler */
  virtual int remove(const char* name) = 0;
  /** Handle a command, based on a command string */
  virtual int handle(char* cmdStr) = 0;
};

/** @} */

#endif
