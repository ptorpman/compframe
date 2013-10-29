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
#ifndef COMPFRAME_C_H
#define COMPFRAME_C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#if 0
}
#endif

/** @addtogroup types CompFrame Types
 *  @{
 *
 */

//*============================================================================*/
/* MACROS                                                                     */
/*============================================================================*/

/*============================================================================*/
/* TYPES                                                                      */
/*============================================================================*/

/** Type used when registering interfaces */
typedef struct cf_iface_reg_t {
    char *name;                 /**< Name of interface */
    void *iface;                /**<Interface pointer */
} cf_iface_reg_t;

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* COMPFRAME_C_H */
