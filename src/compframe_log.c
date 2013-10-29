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

/*============================================================================*/
/* INCLUDES                                                                   */
/*============================================================================*/

#include "compframe_log.h"
#include <stdarg.h>

/*============================================================================*/
/* VARIABLES                                                                  */
/*============================================================================*/

/** This variable holds the trace level of CompFrame */
static CfTraceLevel cfTraceLevel = CF_TRACE_OFF;

/** This variable holds the names of the trace levels of CompFrame */
static char *cf_trace_level_names[] = {
    "OFF",
    "INFO",
    "DEBUG",
    "MASSIVE"
};

/*============================================================================*/
/* FUNCTION DEFINITIONS                                                       */
/*============================================================================*/

void
cf_info_log(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);

    fprintf(stderr, "*** INFO # ");
    vfprintf(stderr, format, ap);

    va_end(ap);
}

void
cf_error_log(const char *file, int line, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);

    fprintf(stderr, "*** ERROR %s:%d # ", file, line);
    vfprintf(stderr, format, ap);

    va_end(ap);
}

void
cf_trace_log(const char *file,
             int line, CfTraceLevel level, const char *format, ...)
{
    if (level > cfTraceLevel) {
        /* Too high trace level. Do nothing. */
        return;
    }

    va_list ap;

    va_start(ap, format);

    fprintf(stderr,
            "***[%-7s] %-20s:%-5d # ", cf_trace_level_names[level], file, line);
    vfprintf(stderr, format, ap);

    va_end(ap);
}

void
cf_trace_level_set(int level)
{
    cfTraceLevel = (CfTraceLevel) level;
}
