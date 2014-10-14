#ifndef CF_CFG_HH
#define CF_CFG_HH
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

/** @addtogroup cfg Cfg - Configurator
 * @{
 */

#define WANT_TCL_COMMANDS 1

//=============================================================================
//                        I N C L U D E S
//=============================================================================
#include "CFComponent.hh"
#include "compframe.h"
#include "compframe_sockets.h"
#include <map>
using namespace std;

#include "IConfig.hh"
#include <stdio.h>


//=============================================================================
//                          M A C R O S 
//=============================================================================

//=============================================================================
//                           T Y P E S
//=============================================================================

//=============================================================================
//                     E N U M E R A T I O N S
//=============================================================================

//=============================================================================
//                   G L O B A L  V A R I A B L E S
//=============================================================================

//=============================================================================
//                       C O N S T A N T S 
//=============================================================================

class CF_Cfg :
  public CFComponent,
  public IConfig
{
public:
  CF_Cfg(const char *inst_name);
  virtual ~CF_Cfg();

  // IConfig methods
  int parse(char* cfgFile);
	
private:
  // Instance name
  string mName;

  int getLine(FILE * fp, char *buffer, int *length);

};



/** @}  Doxygen end marker */ 

#endif
