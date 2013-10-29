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
#include "compframe_iface_cfg.h"
#include "compframe_iface_c.h"
#include "compframe_util.h"
#include "compframe_i.h"

/** @addtogroup cfg Cfg - Configurator
 *  @{
 */

/*===========================================================================*/
/* MACROS                                                                    */
/*===========================================================================*/
/** Usage string from 'cfg' command */
#define CFG_CMD_USAGE                                               \
    "\nUsage: config -i <inst> -n <varnum> -t <type> -v <value>\n"

/*===========================================================================*/
/* TYPES                                                                     */
/*===========================================================================*/

/** This is the configurator component of CompFrame */
typedef struct cfcfg_t {
    /** Instance name */
    char *inst_name;

    /* Pointer to C component */
    void *cObj;
    /* Pointer to C interface */
    cfi_c *cIface;
} cfcfg_t;

/*===========================================================================*/
/* VARIABLES                                                                 */
/*===========================================================================*/

/** The Cfg server interface that we implement */
static cfi_cfg cfgIface;

/*===========================================================================*/
/* FUNCTION DECLARATIONS                                                     */
/*===========================================================================*/

static void *create_me(const char *inst_name);
static void set_me_up(void *comp);
static int destroy_me(void *comp);

static int cfg_cmd(int argc, char **argv);
static int cfg_config_parse(void *comp, char *file);
static int getLine(FILE * fp, char *buffer, int *length);

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
    cf_component_register("Cfg", create_me, set_me_up, destroy_me);
}

/** This function is used to create and initate a component. 
    @return Pointer to created instance or NULL 
*/
static void *
create_me(const char *inst_name)
{

    assert(inst_name != NULL);

    cfcfg_t *c = malloc(sizeof(cfcfg_t));

    memset(c, 0, sizeof(cfcfg_t));

    c->inst_name = strdup(inst_name);

    return c;
}

static int
destroy_me(void *comp)
{
    if (!comp) {
        return 1;
    }

    cfcfg_t *c = (cfcfg_t *) comp;

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
    cfcfg_t *c = (cfcfg_t *) comp;

    cfgIface.parse = cfg_config_parse;

    /* Register our interfaces */
    cf_iface_reg_t iface[] = {
        {CF_CFG_IFACE_NAME, &cfgIface},
        {NULL, NULL}
    };

    cf_interface_register(comp, iface);

    /* Register our commands */
    {
        c->cObj = cf_component_get(CF_C_NAME);
        c->cIface = cf_interface_get(c->cObj, CF_C_IFACE_NAME);

        c->cIface->add(c->cObj, "config", cfg_cmd, CFG_CMD_USAGE);
    }
}

/** The main 'Cfg' command.*/
static int
cfg_cmd(int argc, char **argv)
{
    if (argc != 4) {
        /* Wrong number of arguments */
        return 0;
    }

    /* Get first component */
    void *comp = cf_component_get(argv[1]);

    if (!comp) {
        cf_error_log(__FILE__, __LINE__,
                     "Instance not found (%s)!\n", argv[1]);
        return 0;
    }

    /* See if comp implements the right interface */
    void *cfi = cf_interface_get(comp, CF_CFG_CLIENT_IFACE_NAME);

    if (!cfi) {
        cf_error_log(__FILE__, __LINE__,
                     "Instance (%s) does not implement the cfi_cfg_client"
                     " interface!\n", argv[1]);
        return 0;
    }

    void* cfg = cf_component_get(CF_CFG_NAME);

    return ((cfi_cfg_client*)cfi)->set(cfg, comp, argv[2], argv[3]);
}

static int
cfg_config_parse(void *comp, char *file)
{
    cfcfg_t *c = (cfcfg_t *) comp;
    FILE *fp = fopen(file, "r");
    char line[1024];
    int length;
    int res;

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "CFG config file: %s\n", file);

    if (!fp) {
        cf_error_log(__FILE__, __LINE__, "Could not open file (%s)!\n", file);
        return 0;
    }

    length = 1024;

    while (getLine(fp, line, &length) != -1) {
        if (length != 0) {
            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "CFG config line: (%s)\n", line);

            if (!memcmp("create ", line, 7)) {
                /* Let the C component handle the 'create' command... */
                res = c->cIface->handle(c->cObj, line);

                if (!res) {
                    cf_error_log(__FILE__, __LINE__,
                                 "Command failed (%s)!\n", line);
                    return 0;
                }
            }
            else if (!memcmp("connect ", line, 8)) {
                /* Let the C component handle the 'connect' command... */
                res = c->cIface->handle(c->cObj, line);

                if (!res) {
                    cf_error_log(__FILE__, __LINE__,
                                 "Command failed (%s)!\n", line);
                    return 0;
                }
            }
            else if (!memcmp("config ", line, 7)) {
                /* We handle this ourselves */
                res = c->cIface->handle(c->cObj, line);

                if (!res) {
                    cf_error_log(__FILE__, __LINE__,
                                 "Command failed (%s)!\n", line);
                    return 0;
                }
            }
            else {
                /* Unknown command */
                cf_error_log(__FILE__, __LINE__,
                             "Unknown command (%s)!\n", line);
                return 0;
            }
        }

        length = 1024;
    }

    return 1;
}

static int
getLine(FILE * fp, char *buffer, int *length)
{
    char *line;
    char *tmp;
    char *lineCopy = malloc(*length);

    line = fgets(buffer, *length, fp);

    if (!line) {
        *length = 0;
        free(lineCopy);
        return -1;
    }

    if (line[0] == '#') {
        *length = 0;
        free(lineCopy);
        return 0;
    }

    strcpy(lineCopy, line);

    tmp = strtok(lineCopy, " \r\t\n");

    if (!tmp) {
        *length = 0;
        free(lineCopy);
        return 0;
    }

    *length = strlen(line);

    free(lineCopy);

    return 1;
}

/** @} */
