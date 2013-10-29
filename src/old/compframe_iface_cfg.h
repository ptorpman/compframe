/* Copyright (c) 2007  Peter R. Torpman (peter at torpman dot se)

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
#ifndef COMPFRAME_IFACE_CFG_H
#define COMPFRAME_IFACE_CFG_H

#include "compframe_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
#if 0
}
#endif


/** @addtogroup ifaces CompFrame Interfaces 
    @{
*/
/** Textual name of the Cfg server interface, that is implemented by 
    Cfg (the configurator). */
#define CF_CFG_IFACE_NAME  "cfi_cfg"

/** Interface type */
typedef struct cfi_cfg {
  /** Sets an integer type variable
      @param cfg      Pointer to Cfg component
      @param cfgFile  Config file name
      @return 1 if OK, 0 if failure
  */
  int (*parse)(void* cfg, char* cfgFile);
} cfi_cfg;


/** Textual name of the Cfg client interface, that is implemented by 
    components that wants to be configured . */
#define CF_CFG_CLIENT_IFACE_NAME  "cfi_cfg_client"

/** Interface type */
typedef struct cfi_cfg_client {
  /** Sets an integer type variable
      @param cfg      Pointer to Cfg component
      @param comp     Pointer to own context
      @param varName  Variable name
      @param varValue Variable value
      @return 1 if OK, 0 if failure
  */
  int (*set)(void* cfg, 
             void* comp, 
             char* varName, 
             char* varValue);
} cfi_cfg_client;


/** @}   Doxygen end marker */

/*============================================================================*/
/* MACROS                                                                     */
/*============================================================================*/



#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif  /* COMPFRAME_IFACE_CFG_H */
