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
   along with this program.  If not, see <http://www.gnu.or/licenses/>.
*/
#ifndef COMPFRAME_IFACE_C_H
#define COMPFRAME_IFACE_C_H

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

/** Type used for commands */
typedef int (*cfc_func_t)(int argc, char** argv);

/** Textual name of the C interface that is used to add commands
 *  to the command handler
 */
#define CF_C_IFACE_NAME  "cfi_c"

/** C command interface */
typedef struct cfi_c {
    /** Add a command to the handler */
    int (*add)(void* obj, const char* name, cfc_func_t func, char* usage);
    /** Remove a command from the handler */
    int (*remove)(void* obj, const char* name);
    /** Handle a command, based on a command string */
    int (*handle)(void* obj, char* cmdStr);
} cfi_c;


/** @}   Doxygen end marker */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif  /* COMPFRAME_IFACE_C_H */
