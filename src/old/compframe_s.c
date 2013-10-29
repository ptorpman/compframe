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
/*===========================================================================*/
/* INCLUDES                                                                  */
/*===========================================================================*/

#include "compframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "compframe_interfaces.h"
#include "compframe_iface_s.h"
#include "compframe_iface_c.h"
#include "compframe_util.h"
#include "compframe_i.h"
#include "compframe_sockets.h"

/** @addtogroup s S - Scheduler
 *  @{
 */

/*===========================================================================*/
/* MACROS                                                                    */
/*===========================================================================*/
/** Usage string from 's' command */
#define S_CMD_USAGE                                         \
    "\nUsage: s [-r | -h | -t <slice>] \n"                  \
    "  -r           Start scheduling components (Run)\n"    \
    "  -h           Stop scheduling components  (Halt)\n"   \
    "  -t <slice>   Set timeslice of scheduler\n"

/*===========================================================================*/
/* TYPES                                                                     */
/*===========================================================================*/
typedef enum {
    CF_S_IDLE = 0,
    CF_S_RUNNING = 1,
    CF_S_STOPPED = 2
} cfs_state_t;

/** Type used for scheduled components in a list  */
typedef struct cf_scheduled_t {
    /** Pointer to next  */
    struct cf_scheduled_t *next;
    /** Pointer to previous  */
    struct cf_scheduled_t *prev;

    /** Pointer to scheduled component */
    void *obj;
    /** Pointer to scheduled component's interface */
    cfi_s_client *schedIface;

    /** Current balance  */
    int sliceBalance;
} cf_scheduled_t;

/** This is the scheduler component of CompFrame */
typedef struct cfs_t {
    /** Instance name */
    char *inst_name;

    /** State  */
    cfs_state_t state;

    /** List of scheduled components  */
    cf_scheduled_t *head;

    /** Timeslice used. The number of virtual milliseconds available for
        compoent when executing */
    uint32_t slice;
} cfs_t;

/*===========================================================================*/
/* VARIABLES                                                                 */
/*===========================================================================*/

/** The S control interface that we implement */
static cfi_s_control sControlIface;

/** The S server interface that we implement */
static cfi_s_server sServerIface;

/*===========================================================================*/
/* FUNCTION DECLARATIONS                                                     */
/*===========================================================================*/

static void *create_me(const char *inst_name);
static void set_me_up(void *comp);
static int destroy_me(void *comp);

static int
scheduling_loop(void *obj);
static int
scheduling_perform(void *obj);
static int
scheduling_start(void *obj);
static int
scheduling_stop(void *obj);

static int
scheduled_add(void *sObj, void *obj);
static int
scheduled_remove(void *sObj, void *obj);

static int
s_cmd(int argc, char **argv);

/*===========================================================================*/
/* FUNCTION DEFINITIONS                                                      */
/*===========================================================================*/

/** This function must reside in all component libraries.
    Here the component instance is 
*/
void
dlopen_this(void)
{
    /* Use this function to get S into the Registry  */
    cf_component_register("S", create_me, set_me_up, destroy_me);
}

/** This function is used to create and initate a component. 
    @return Pointer to created instance or NULL 
*/
static void *
create_me(const char *inst_name)
{

    assert(inst_name != NULL);

    cfs_t *s = malloc(sizeof(cfs_t));

    memset(s, 0, sizeof(cfs_t));

    s->inst_name = strdup(inst_name);
    s->state = CF_S_IDLE;
    s->head = NULL;
    s->slice = 10;

    return s;
}

static int
destroy_me(void *comp)
{
    if (!comp) {
        return 1;
    }

    cfs_t *s = (cfs_t *) comp;

    /* Scheduled components? */
    if (s->head != NULL) {
        cf_error_log(__FILE__, __LINE__,
                     "Scheduled components still active! (%s)\n", s->inst_name);
        return 1;
    }

    /* De-register our interfaces */
    cf_interface_deregister(comp);

    free(s->inst_name);
    free(s);

    return 0;
}

/** Function called after S has been created */
static void
set_me_up(void *comp)
{
    /* S control interface */
    sControlIface.loop = scheduling_loop;
    sControlIface.do_it = scheduling_perform;
    sControlIface.start = scheduling_start;
    sControlIface.stop = scheduling_stop;

    /* S server interface */
    sServerIface.add = scheduled_add;
    sServerIface.remove = scheduled_remove;

    /* Register our interfaces */
    cf_iface_reg_t iface[] = {
        {CF_S_CONTROL_IFACE_NAME, &sControlIface},
        {CF_S_SERVER_IFACE_NAME, &sServerIface},
        {NULL, NULL}
    };

    cf_interface_register(comp, iface);
}

/** This function is the eternal loop of the system.
 *  @param obj This context
 *  @return 1 if OK, 0 if not.
 */
static int
scheduling_loop(void *obj)
{
    cfs_t *s = (cfs_t *) obj;

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Entering scheduler main loop...\n");

    /* First, make sure our command is registered. */
    /* Register our commands */
    void *cObj = cf_component_get(CF_C_NAME);
    cfi_c *cIface = cf_interface_get(cObj, CF_C_IFACE_NAME);

    cIface->add(cObj, "s", s_cmd, S_CMD_USAGE);

    /* Change state of scheduler to running... */
    s->state = CF_S_RUNNING;

    /* If the user issues 'q' on the command line we will get outta here... */
    while (1) {
        /* Poll the sockets ... Gotta do it somewhere.... */
        cf_sockets_poll();

        scheduling_perform(obj);
    }

    return 0;
}

/** This function will make sure all scheduled components gets to run.
 *  @param obj This context
 *  @return 1 if OK, 0 if not.
 */
static int
scheduling_perform(void *obj)
{
    cfs_t *s = (cfs_t *) obj;

    if (s->state != CF_S_RUNNING) {
        cf_trace_log(__FILE__, __LINE__, CF_TRACE_MASSIVE,
                     "Scheduler not started...\n");
        return 1;
    }

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_MASSIVE,
                 "Scheduling components...\n");

    int sliceLeft = 0;

    for (cf_scheduled_t * tmp = s->head; tmp != NULL; tmp = tmp->next) {

        /* Add to previously used up time (or time left) */
        tmp->sliceBalance += s->slice;

        if (tmp->sliceBalance <= 0) {
            /* No time to run this one... Add this round's slice and take next */
            continue;
        }

        cf_trace_log(__FILE__, __LINE__, CF_TRACE_MASSIVE,
                     "Scheduling component: %s\n",
                     cf_component_name_get(tmp->obj));

        tmp->schedIface->execute(tmp->obj, s->slice, &sliceLeft);

        /* Take care of time used up */
        tmp->sliceBalance += sliceLeft;
    }

    return 1;
}

/** Add a component to the scheduling loop
 *  @param sObj        Pointer to S component
 *  @param obj         Pointer to scheduled component
 *  @return 1 if OK, 0 if not.
 */
static int
scheduled_add(void *sObj, void *obj)
{
    cfs_t *s = (cfs_t *) sObj;

    /* See if it is already registered */
    for (cf_scheduled_t * tmp = s->head; tmp != NULL; tmp = tmp->next) {
        if (tmp->obj == obj) {
            /* Found it. Cannot add again.... */
            cf_error_log(__FILE__, __LINE__,
                         "Could not add component again to scheduler loop!\n");
            return 0;
        }
    }

    /* Make sure the component has the right interface */
    cfi_s_client *sIface = cf_interface_get(obj,
                                            CF_S_CLIENT_IFACE_NAME);

    if (!sIface) {
        cf_error_log(__FILE__, __LINE__,
                     "Component does not implement S client interface!\n");
        return 0;
    }

    /* Create a new one and insert into loop */
    cf_scheduled_t *tmp = malloc(sizeof(cf_scheduled_t));

    memset(tmp, 0, sizeof(cf_scheduled_t));

    tmp->sliceBalance = 0;
    tmp->schedIface = sIface;
    tmp->obj = obj;

    CF_LIST_ADD(s->head, tmp);

    return 1;
}

/** Remove a component from the scheduling loop
 *  @param sObj        Pointer to S component
 *  @param obj         Pointer to scheduled component
 *  @return 1 if OK, 0 if not.
 */
static int
scheduled_remove(void *sObj, void *obj)
{
    cfs_t *s = (cfs_t *) sObj;

    /* See if it is registered */
    for (cf_scheduled_t * tmp = s->head; tmp != NULL; tmp = tmp->next) {
        if (tmp->obj == obj) {
            /* Found it.  */

            return 1;
        }
    }

    cf_error_log(__FILE__, __LINE__,
                 "Could not find component in scheduler loop!\n");
    return 0;
}

/** This function starts the scheduler loop
 *  @param obj This context
 *  @return 1 if OK, 0 if not.
 */
static int
scheduling_start(void *obj)
{
    cfs_t *s = (cfs_t *) obj;

    cf_info_log("S scheduling commencing. Using timeslice %u.\n", s->slice);

    s->state = CF_S_RUNNING;

    return 1;
}

/** This function stops the scheduler loop
 *  @param obj This context
 *  @return 1 if OK, 0 if not.
 */
static int
scheduling_stop(void *obj)
{
    cfs_t *s = (cfs_t *) obj;

    cf_info_log("S scheduling stopped.\n");

    s->state = CF_S_STOPPED;

    return 1;
}

/** The main 'S' command.*/
static int
s_cmd(int argc, char **argv)
{
    cfs_t *this = cf_component_get(CF_S_NAME);

    switch (argc) {

    case 2:
        /* -r Start scheduler loop */
        if (!strcmp(argv[1], "-r")) {
            this->state = CF_S_RUNNING;
            return 1;
        }

        /* -h Stop scheduler loop */
        if (!strcmp(argv[1], "-h")) {
            this->state = CF_S_STOPPED;
            return 1;
        }

        break;

    case 3:

        if (!strcmp(argv[1], "-t")) {
            uint32_t slice;

            if (sscanf(argv[2], "%u", &slice) != 1) {
                cf_error_log(__FILE__, __LINE__,
                             "Bad timeslice value! (%d)\n", argv[2]);
                return 0;
            }

            this->slice = slice;

            cf_info_log("Scheduler timeslice set to %u.\n", slice);

            return 1;
        }
        else {
            cf_error_log(__FILE__, __LINE__, S_CMD_USAGE);
            return 0;
        }
    default:
        cf_error_log(__FILE__, __LINE__, S_CMD_USAGE);
        return 0;
    }

    return 0;
}

/** @} */
