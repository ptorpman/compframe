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
#ifndef COMPFRAME_IFACE_S_H
#define COMPFRAME_IFACE_S_H

#include "compframe_types.h"

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */
#if 0
}
#endif

/** @addtogroup ifaces CompFrame Interfaces 
    @{
*/


#if 0


/** Textual name of the S interface that is used to let S do its
 * scheduling operations.
 */
#define CF_S_CONTROL_IFACE_NAME  "cfi_s_control"
/** S control interface */ typedef struct cfi_s_control {
    /** Enter eternal loop of scheduler. */
    int (*loop) (void *obj);
    /** Perform scheduling operations */
    int (*do_it) (void *obj);
    /** Start scheduling loop  */
    int (*start) (void *obj);
    /** Stop scheduling loop  */
    int (*stop) (void *obj);
} cfi_s_control;


/** Textual name of the S interface that is used by components to register
 *  within S, so that they may be put into the scheduling loop.
 *  @note Components must implement 'cfi_s_client' if they want to be scheduled.
 */
#define CF_S_SERVER_IFACE_NAME  "cfi_s_server"

/** S server interface */
typedef struct cfi_s_server {
    /** Register in S server.
     *  @param sObj        Pointer to S component
     *  @param obj         Pointer to scheduled component
     *  @return 1 if OK, 0 if not.
     */
    int (*add) (void *sObj, void *obj);

    /** Deregister in S server.
     *  @param sObj        Pointer to S component
     *  @param obj         Pointer to scheduled component
     *  @return 1 if OK, 0 if not.
     */
    int (*remove) (void *sObj, void *obj);
} cfi_s_server;

/** Textual name of the S interface used by S to tell scheduled components
 *  that it is their time to execute.
 */
#define CF_S_CLIENT_IFACE_NAME  "cfi_s_client"

/** S client interface */
typedef struct cfi_s_client {
    /** Perform execution
     *  @param obj         Pointer to scheduled component
     *  @param slice       Number of milliseconds available for execution
     *  @param sliceRemain Number of milliseconds not used up.
     */
    void (*execute) (void *obj, uint32_t slice, int *sliceRemain);
} cfi_s_client;

#endif

/** @}   Doxygen end marker */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* COMPFRAME_IFACE_S_H */
