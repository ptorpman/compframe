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

#define WANT_TCL_COMMANDS 1

/*===========================================================================*/
/* INCLUDES                                                                  */
/*===========================================================================*/

#include "compframe.h"
#include "compframe_sockets.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "compframe_interfaces.h"
#include "compframe_iface_s.h"
#include "compframe_iface_c.h"
#include "compframe_util.h"
#include "compframe_i.h"
#include <unistd.h>
#include "compframe_registry.h"

#ifdef WANT_TCL_COMMANDS
#include <tcl.h>
#endif

/** @addtogroup c C - Command Handler
 *  @{
 */

/*===========================================================================*/
/* MACROS                                                                    */
/*===========================================================================*/

/*===========================================================================*/
/* TYPES                                                                     */
/*===========================================================================*/
/** Type used for keeping commands in a list */
typedef struct cfc_command_t {
    struct cfc_command_t *next; /**< Pointer to next */
    struct cfc_command_t *prev; /**< Pointer to previous */
    char *name;                      /**< Command name */
    cfc_func_t func;              /**< Pointer to function */
    char *help;                      /**< Help string */
} cfc_command_t;

/** This is the command handler component of CompFrame */
typedef struct cfc_t {
    /** Instance name */
    char *inst_name;

    /** Timeslice used. The number of virtual milliseconds available for
        compoent when executing */
    uint32_t slice;

    /** List of installed commands */
    cfc_command_t *cmdList;

#ifdef WANT_TCL_COMMANDS
    /** Tcl interpreter  */
    Tcl_Interp *tclInterp;
#endif
} cfc_t;

/*===========================================================================*/
/* VARIABLES                                                                 */
/*===========================================================================*/

/** The C interface that we implement */
static cfi_c cInterface;

/* S client interface  */
static cfi_s_client sClient;

/*===========================================================================*/
/* FUNCTION DECLARATIONS                                                     */
/*===========================================================================*/

static void *create_me(const char *inst_name);
static void
set_me_up(void *comp);
static int
destroy_me(void *comp);

/* S client interface */
static void
s_client_execute(void *obj, uint32_t slice, int *sliceRemain);

static int
create_cmd(int argc, char **argv);
static int
remove_cmd(int argc, char **argv);
static int
list_cmd(int argc, char **argv);
static int
help_cmd(int argc, char **argv);
static int
connect_cmd(int argc, char **argv);

static void
free_argv(int argc, char **argv);

static int
command_add(void *obj, const char *name, cfc_func_t func, char *usage);
static int
command_remove(void *obj, const char *name);
static int
command_handle(void *obj, char *usr);

static void
commands_initiate(void);

static int
stdin_handle(void *comp, int sd, void *userData, cf_sock_event_t ev);

/*===========================================================================*/
/* FUNCTION DEFINITIONS                                                      */
/*===========================================================================*/

/** This function must reside in all component libraries.
    Here the component instance is 
*/
void
dlopen_this(void)
{
    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG, "dlopen_this C...\n");
    /* Use this function to get C into the Registry  */
    cf_component_register("C", create_me, set_me_up, destroy_me);
}

/** This function is used to create and initate a component. 
    @return Pointer to created instance or NULL 
*/
static void *
create_me(const char *inst_name)
{
    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG, "Creating C...\n");

    assert(inst_name != NULL);

    cfc_t *c = malloc(sizeof(cfc_t));

    memset(c, 0, sizeof(cfc_t));

    c->inst_name = strdup(inst_name);
    c->slice = 10;

    return c;
}

static int
destroy_me(void *comp)
{
    cfc_t *c = (cfc_t *) comp;

    if (!c) {
        return 1;
    }

    /* Remove all commands */
    cfc_command_t *cmd = c->cmdList;

    while (cmd) {
        CF_LIST_REMOVE(c->cmdList, cmd);
        free(cmd->name);
        free(cmd->help);
        free(cmd);

        cmd = c->cmdList;
    }

    /* De-register our interfaces */
    cf_interface_deregister(comp);

    free(c->inst_name);
    free(c);

    return 0;
}

/** Function called after S has been created */
static void
set_me_up(void *comp)
{
    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG, "Setting up C...\n");

    cfc_t *this = (cfc_t *) comp;

    /* Command handler interface  */
    cInterface.add = command_add;
    cInterface.remove = command_remove;
    cInterface.handle = command_handle;

    /* Scheduler client interface */
    sClient.execute = s_client_execute;

    /* Register our interfaces */
    cf_iface_reg_t iface[] = {
        {CF_S_CLIENT_IFACE_NAME, &sClient},
        {CF_C_IFACE_NAME, &cInterface},
        {NULL, NULL}
    };

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG, "Adding interfaces...\n");
    cf_interface_register(comp, iface);

    /* Initiate command handler */
    commands_initiate();

#ifdef WANT_TCL_COMMANDS
    this->tclInterp = Tcl_CreateInterp();
#endif

    /* Add ourself to the S scheduling loop */

    void *sObj = cf_component_get(CF_S_NAME);

    cfi_s_server *sIface = cf_interface_get(sObj, CF_S_SERVER_IFACE_NAME);

    sIface->add(sObj, this);
}

static void
commands_initiate(void)
{
    cfc_t *this = (cfc_t *) cf_component_get("C");

    command_add(this, "list", list_cmd, "Usage: list [-c | -i <name>]\n");

    command_add(this,
                "create", create_cmd, "Usage: create <class> <instname>\n");
    command_add(this, "remove", remove_cmd, "Usage: remove <instname>\n");

    command_add(this,
                "connect",
                connect_cmd,
                "Usage: connect <inst> <inst> IFACE <param1 param2 ...>\n");

    command_add(this, "help", help_cmd, "Usage: help <command>\n");
}

static int
command_handle(void *obj, char *usr)
{
    cfc_t *this = (cfc_t *) obj;
    char *lasts;
    char *tmp;
    int res;
    int argc = 0;
    char *argv[CF_MAX_COMMANDLEN];
    char *cmd;

    cmd = usr;

    if (!cmd) {
        cf_error_log(__FILE__, __LINE__, "Bad command!\n");
        return 0;
    }

    if (strlen(cmd) > CF_MAX_COMMANDLEN) {
        cf_error_log(__FILE__, __LINE__, "Command too long!\n");
        return 0;
    }

    if (strlen(cmd) == 0) {
        return 0;
    }

#ifdef WANT_TCL_COMMANDS
    if (cmd[0] == '%') {
        res = Tcl_Eval(this->tclInterp, cmd + 1);
        if (res != TCL_OK) {

            fprintf(stderr, "Tcl Error (%d): %s\n",
                    res, Tcl_GetStringResult(this->tclInterp));
        }
        return 0;
    }
#endif

    /* Take copy */
    tmp = cmd;

    argc = 0;
    while (1) {
        if (argc == 0) {
            tmp = strtok_r(tmp, " \t\n", &lasts);
        }
        else {
            tmp = strtok_r(NULL, " \t\n", &lasts);
        }

        if (!tmp) {
            /* End of commands */
            break;
        }

        argv[argc++] = strdup(tmp);
    }

    /* Supported command? */
    for (cfc_command_t * c = this->cmdList; c != NULL; c = c->next) {
        if (!strcmp(c->name, argv[0])) {
            res = c->func(argc, argv);
            free_argv(argc, argv);
            return res;
        }
    }

    free_argv(argc, argv);

    /* No, not there... */
    cf_info_log("Invalid command.\n");

    fprintf(stdout, "Available commands are:\n");
    for (cfc_command_t * c = this->cmdList; c != NULL; c = c->next) {
        fprintf(stdout, "  %s\n", c->name);
    }

    return 0;
}

static void
free_argv(int argc, char **argv)
{
    for (int i = 0; i < argc; i++) {
        if (argv[i]) {
            free(argv[i]);
        }
    }
}

static int
create_cmd(int argc, char **argv)
{
    if (argc != 3) {
        cf_error_log(__FILE__, __LINE__, "Usage: create <class> <instname>\n");
        return 0;
    }

    if (cf_component_create(argv[1], argv[2])) {
        return 1;
    }

    return 0;
}

static int
remove_cmd(int argc, char **argv)
{
    if (argc != 2) {
        cf_error_log(__FILE__, __LINE__, "Usage: remove <instname>\n");
        return 0;
    }

    if (cf_component_destroy(argv[1])) {
        return 1;
    }

    return 0;
}

static int
list_cmd(int argc, char **argv)
{
    (void) argv;                /* Avoid warnings */

    switch (argc) {

    case 1:
        /* No parameter */
        cf_instance_list();
        return 1;

    case 2:
        /* -c */
        if (strcmp(argv[1], "-c")) {
            cf_error_log(__FILE__, __LINE__, "Usage: list [-c | -i <name>]\n");
            return 0;
        }
        cf_class_list();
        return 1;

    case 3:
        /* -i <name> */
        if (strcmp(argv[1], "-i")) {
            cf_error_log(__FILE__, __LINE__, "Usage: list [-c | -i <name>]\n");
            return 0;
        }
        cf_interface_list(argv[2]);
        return 1;

    default:
        cf_error_log(__FILE__, __LINE__, "Usage: list [-c | -i <name>]\n");
        return 0;
    }

    return 0;
}

static int
help_cmd(int argc, char **argv)
{
    cfc_t *this = (cfc_t *) cf_component_get("C");

    if (argc != 2) {
        fprintf(stdout, "Available commands are:\n");
        for (cfc_command_t * c = this->cmdList; c != NULL; c = c->next) {
            fprintf(stdout, "  %s\n", c->name);
        }
        return 0;
    }

    cfc_command_t *c;

    for (c = this->cmdList; c != NULL; c = c->next) {
        if (!strcmp(c->name, argv[1])) {
	  fprintf(stdout, "%s", c->help);
	  return 1;
        }
    }

    cf_error_log(__FILE__, __LINE__, "Error: Invalid command.\n");
    return 0;
}

int
command_add(void *obj, const char *name, cfc_func_t func, char *usage)
{
    cfc_t *this = (cfc_t *) obj;
    cfc_command_t *c;

    if (!name) {
        return 0;
    }

    /* Already a command with that name? */
    for (c = this->cmdList; c != NULL; c = c->next) {
        if (!(strcmp(name, c->name))) {
            /* Already there */
            return 0;
        }
    }

    /* Allocate a new command structure and add it to list */
    c = malloc(sizeof(cfc_command_t));

    c->prev = NULL;
    c->name = strdup(name);
    c->func = func;
    c->help = strdup(usage);

    /* Always add to the beginning of the list */
    c->next = this->cmdList;

    if (this->cmdList) {
        this->cmdList->prev = c;
    }

    this->cmdList = c;

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Command - %s - registered!\n", name);

    return 1;
}

int
command_remove(void *obj, const char *name)
{
    cfc_t *this = (cfc_t *) obj;
    cfc_command_t *c;

    if (!name) {
        return 0;
    }

    /* A command with that name? */
    for (c = this->cmdList; c != NULL; c = c->next) {
        if (strcmp(name, c->name)) {
            /* Not this one */
            continue;
        }

        /* Found it */
        if (c->prev == NULL) {
            /* First in list */
            this->cmdList = c->next;

            if (this->cmdList) {
                this->cmdList->prev = NULL;
            }
        }
        else if (c->next == NULL) {
            /* Last in list */
            c->prev->next = NULL;
        }
        else {
            /* In the middle somewhere */
            c->prev->next = c->next;
            c->next->prev = c->prev;
        }

        /* Now free the memory */
        free(c->help);
        free(c->name);
        free(c);

        return 1;
    }

    /* Failure... */
    return 0;
}

static int
connect_cmd(int argc, char **argv)
{
    if (argc < 4) {
        cf_error_log(__FILE__, __LINE__,
                     "Usage: connect <inst> <inst> IFACE <param1 param2 ...>\n");
        return 0;
    }

    /* Get first component */
    void *comp1 = cf_component_get(argv[1]);

    if (!comp1) {
        cf_error_log(__FILE__, __LINE__, "No such instance (%s)\n", argv[1]);
        return 0;
    }

    /* Get other component */
    void *comp2 = cf_component_get(argv[2]);

    if (!comp2) {
        cf_error_log(__FILE__, __LINE__, "No such instance (%s)\n", argv[2]);
        return 0;
    }

    /* Get cfi_connect on first component */
    void *cfi = cf_interface_get(comp1, CF_CONNECT_IFACE_NAME);

    if (!cfi) {
        cf_error_log(__FILE__, __LINE__,
                     "Interface (%s) not implemented by (%s)\n",
                     CF_CONNECT_IFACE_NAME, argv[1]);
        return 0;
    }

    char **args = argv;

    if (argc == 4) {
        /* No parameters */
        return ((cfi_connect *) (cfi))->connect(comp1, comp2, argv[3], 0, NULL);
    }

    /* Get to the interface parameters */
    args += 4;

    return ((cfi_connect *) (cfi))->connect(comp1, comp2, argv[3], argc - 4,
                                            args);
}

/**************** S CLIENT INTERFACE IMPLEMENTATION START  *************/

static void
s_client_execute(void *obj, uint32_t slice, int *sliceRemain)
{
    static int showCopyStuff = 1;

    *sliceRemain = 0;

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_MASSIVE,
                 "%s is scheduled with slice %u\n",
                 cf_component_name_get(obj), slice);

    /* Do stuff that should be done once */
    if (showCopyStuff) {
        fprintf(stderr,
                "CompFrame " CF_VERSION " (c)2007-2010 Peter R. Torpman\n>> ");
        showCopyStuff = 0;

        /* Register stdin for polling */
        cf_socket_register(obj, fileno(stdin), stdin_handle, NULL);
    }
}

/**************** S CLIENT INTERFACE IMPLEMENTATION END  *************/

static int
stdin_handle(void *comp, int sd, void *userData, cf_sock_event_t ev)
{
    (void) userData;
    (void) ev;

    char readBuff[2048];
    ssize_t len;

    len = read(sd, readBuff, 2048);

    readBuff[len - 1] = 0;

    /* Want to quit? */
    if (!strcmp(readBuff, "q") || !strcmp(readBuff, "quit")) {
        exit(0);
    }

    command_handle(comp, readBuff);

    fprintf(stderr, ">> ");
    return 0;
}

#ifdef WANT_TCL_COMMANDS

#endif /* WANT_TCL_COMMANDS */

/** @} Doxygen end marker */
