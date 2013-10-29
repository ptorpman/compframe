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
#ifndef COMPFRAME_H
#define COMPFRAME_H

/*===========================================================================*/
/* INCLUDES                                                                  */
/*===========================================================================*/

#include "compframe_debug.h"
#include "compframe_log.h"
#include "compframe_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
#if 0
}
#endif

/** @addtogroup Public API
 *  These functions are part of the public API of CompFrame
 *  @{
 */

/*===========================================================================*/
/* PUBLIC MACROS                                                             */
/*===========================================================================*/

/** Name of M (the message handler) */
#define CF_M_NAME "M"
/** Name of S (the scheduler) */
#define CF_S_NAME "S"
/** Name of C (the command handler) */
#define CF_C_NAME "C"
/** Name of Cfg (the configurator) */
#define CF_CFG_NAME "Cfg"


/*===========================================================================*/
/* PUBLIC FUNCTION DECLARATIONS                                              */
/*===========================================================================*/



/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif  /* COMPFRAME_H */
