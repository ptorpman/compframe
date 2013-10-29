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

#ifndef COMPFRAME_LOG_H
#define COMPFRAME_LOG_H

/*============================================================================*/
/* INCLUDES                                                                   */
/*============================================================================*/

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */
#if 0
}
#endif
/** @addtogroup Public API
 *  @{
 */
/*============================================================================*/
/* MACROS                                                                     */
/*============================================================================*/
/** Internal debug macro */
#define CF_DEBUG(a)                             \
    printf("%s:%d", __FILE__, __LINE__);        \
    printf a;
/*============================================================================*/
/* TYPES                                                                      */
/*============================================================================*/
/** Type used for CompFrame trace levels  */
typedef enum {
    CF_TRACE_OFF = 0,
    CF_TRACE_INFO = 1,
    CF_TRACE_DEBUG = 2,
    CF_TRACE_MASSIVE = 3
} CfTraceLevel;

/*============================================================================*/
/* PUBLIC FUNCTION DECLARATIONS                                               */
/*============================================================================*/

/** This function is used to log information.
    @param format  printf() format string
    @param ...     Variable arguments.
*/
void
cf_info_log(const char *format, ...);

/** This function is used to log errors.
    @param file    __FILE__
    @param line    __LINE__
    @param format  printf() format string
    @param ...     Variable arguments.
*/
void
cf_error_log(const char *file, int line, const char *format, ...);

/** This function is used to trace information to a log.
    @param file    __FILE__
    @param line    __LINE__
    @param level   The trace level.
    @param format  printf() format string
    @param ...     Variable arguments.
*/
void
cf_trace_log(const char *file, int line,
             CfTraceLevel level, const char *format, ...);

/** Sets the trace level 
 *  @param level New trace level 
 */
void
cf_trace_level_set(int level);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* COMPFRAME_LOG_H */
