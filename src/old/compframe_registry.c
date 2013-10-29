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

#include "compframe.h"
#include "compframe_registry.h"
#include "compframe_i.h"
#include "compframe_util.h"
#include <stdlib.h>
#include <string.h>

/*============================================================================*/
/* VARIABLES                                                                  */
/*============================================================================*/

/** Holds the pointer to the CompFrame registry*/
static cf_registry_t *cfRegistry = NULL;

/*============================================================================*/
/* FUNCTION DEFINITIONS                                                       */
/*============================================================================*/

int
cf_registry_init(void)
{
    cfRegistry = malloc(sizeof(cf_registry_t));

    memset(cfRegistry, 0, sizeof(cf_registry_t));

    return 1;
}

int
cf_component_register(const char *name,
                      cf_callback_create_t fp,
                      cf_callback_setup_t sf, cf_callback_destroy_t df)
{
    cf_class_container_t *tmp;

    /* Name already used? */
    for (tmp = cfRegistry->classHead; tmp != NULL; tmp = tmp->next) {
        if (!strcmp(name, tmp->name)) {
            cf_error_log(__FILE__, __LINE__,
                         "Component name already used (%s)\n", name);
            return 0;
        }
    }

    if (fp == NULL || sf == NULL || df == NULL) {
        cf_error_log(__FILE__, __LINE__, "Missing function! (%s)\n", name);
        return 0;
    }

    /* Not used... Store it in list */
    tmp = malloc(sizeof(cf_class_container_t));

    tmp->name = strdup(name);
    tmp->fp = fp;
    tmp->sf = sf;
    tmp->df = df;
    tmp->prev = NULL;

    /* Add to beginning */
    tmp->next = cfRegistry->classHead;

    if (cfRegistry->classHead) {
        cfRegistry->classHead->prev = tmp;
    }

    cfRegistry->classHead = tmp;

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Component - %s - registered!\n", name);

    return 1;
}

int
cf_component_deregister(const char *name)
{

    if (!name) {
        return 0;
    }

    /* If an instance still exists of this class, 
     * then we cannot remove it. */
    cf_inst_containter_t *inst = NULL;

    for (inst = cfRegistry->instHead; inst != NULL; inst = inst->next) {
        if (!strcmp(name, inst->className)) {
            /* Instance existed!! */
            cf_error_log(__FILE__, __LINE__,
                         "Cannot deregister class when instances exist! (%s::%s)\n",
                         name, inst->name);
            return 0;
        }
    }

    void *comp;

    comp = cf_component_get((char *) name);

    if (!comp) {
        return 0;
    }

    cf_class_container_t *c = (cf_class_container_t *) comp;

    CF_LIST_REMOVE(cfRegistry->classHead, c);

    free(c->name);
    free(c);

    return 1;
}

int
cf_component_create(const char *name, const char *inst_name)
{
    /* Class registered? */

    cf_class_container_t *tmp = NULL;

    /* Class registered? */
    for (tmp = cfRegistry->classHead; tmp != NULL; tmp = tmp->next) {
        if (!strcmp(name, tmp->name)) {
            /* Found it! */
            break;
        }
    }

    if (tmp == NULL) {
        cf_error_log(__FILE__, __LINE__, "Class not registered! (%s)\n", name);
        return 0;
    }

    /* Found it! Is instance name used? */
    cf_inst_containter_t *inst = NULL;

    for (inst = cfRegistry->instHead; inst != NULL; inst = inst->next) {
        if (!strcmp(inst_name, inst->name)) {
            /* Already used! */
            cf_error_log(__FILE__, __LINE__,
                         "Instance name already used (%s)\n", inst_name);
            return 0;
        }
    }

    /* Alright! Now we can start cooking! */
    void *created = tmp->fp(inst_name);

    if (!created) {
        cf_error_log(__FILE__, __LINE__,
                     "Could not create (%s - %s)\n", name, inst_name);
        return 0;
    }

    cf_inst_containter_t *newInst = malloc(sizeof(cf_inst_containter_t));

    memset(newInst, 0, sizeof(*newInst));

    newInst->name = strdup(inst_name);
    newInst->className = strdup(name);
    newInst->prev = NULL;
    newInst->instObject = created;
    newInst->ifaceHead = NULL;
    newInst->classObj = tmp;

    /* Add to beginning */
    CF_LIST_ADD(cfRegistry->instHead, newInst);

    /* Finally, allow component to set itself up.  */
    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Setting up component %p!\n", created);
    tmp->sf(created);

    return 1;
}

int
cf_component_destroy(const char *inst_name)
{
    /* Found it! Is instance name used? */
    cf_inst_containter_t *inst = NULL;

    for (inst = cfRegistry->instHead; inst != NULL; inst = inst->next) {
        if (strcmp(inst_name, inst->name) != 0) {
            continue;
        }

        CF_LIST_REMOVE(cfRegistry->instHead, inst);
        free(inst->name);
        free(inst->className);

        /* Call destructor */
        if (inst->classObj->df(inst->instObject) != 0) {
            /* Operation failed! */
            cf_error_log(__FILE__, __LINE__,
                         "Could not destroy (%s)\n", inst_name);
            return 1;
        }

        return 0;
    }

    /* Could not find it */
    cf_error_log(__FILE__, __LINE__,
                 "Could not destroy (%s). Not found!\n", inst_name);
    return 1;
}

void *
cf_component_get(char *inst_name)
{
    for (cf_inst_containter_t * inst = cfRegistry->instHead;
         inst != NULL; inst = inst->next) {
        if (!strcmp(inst->name, inst_name)) {
            /* Found it! */
            return inst->instObject;
        }
    }

    return NULL;
}

char *
cf_component_name_get(void *obj)
{
    for (cf_inst_containter_t * inst = cfRegistry->instHead;
         inst != NULL; inst = inst->next) {
        if (inst->instObject == obj) {
            return inst->name;
        }
    }

    return NULL;
}

int
cf_interface_register(void *comp, cf_iface_reg_t * ifaces)
{
    /* Find component in list */
    for (cf_inst_containter_t * tmp = cfRegistry->instHead;
         tmp != NULL; tmp = tmp->next) {

        if (tmp->instObject != comp) {
            /* Not this one. */
            continue;
        }

        /* Found it. */
        for (int i = 0; ifaces[i].name != NULL; i++) {
            cf_iface_containter_t *cont = malloc(sizeof(cf_iface_containter_t));

            cont->prev = NULL;
            cont->name = strdup(ifaces[i].name);
            cont->iface = ifaces[i].iface;

            cont->next = tmp->ifaceHead;

            if (tmp->ifaceHead) {
                tmp->ifaceHead->prev = cont;
            }

            tmp->ifaceHead = cont;

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "Interface - %s - added (%p)!\n",
                         tmp->ifaceHead->name, tmp->ifaceHead->iface);
        }

        return 1;
    }

    return 0;
}

int
cf_interface_deregister(void *comp)
{
    if (!comp) {
        return 0;
    }

    cf_iface_containter_t *i;

    for (cf_inst_containter_t * tmp = cfRegistry->instHead; tmp != NULL;
         tmp = tmp->next) {

        if (tmp->instObject != comp) {
            /* Not this one. */
            continue;
        }

        i = tmp->ifaceHead;

        while (i) {
            CF_LIST_REMOVE(tmp->ifaceHead, i);
            free(i->name);
            free(i);

            i = tmp->ifaceHead;
        }

        return 1;
    }

    return 0;
}

void *
cf_interface_get(void *comp, const char *iface_name)
{
    /* Find component in list */
    for (cf_inst_containter_t * tmp = cfRegistry->instHead;
         tmp != NULL; tmp = tmp->next) {
        if (tmp->instObject != comp) {
            /* Not this one. */
            continue;
        }

        /* Found it */
        for (cf_iface_containter_t * i = tmp->ifaceHead; i != NULL; i = i->next) {
            if (!strcmp(iface_name, i->name)) {
                /* Here it is! */
                return i->iface;
            }
        }
    }

    /* If we end up here something is baaad! */
    return NULL;
}

void
cf_instance_list(void)
{
    fprintf(stdout, "%-32s %s\n", "Instance", "Class");
    fprintf(stdout, "------------------------------------------------------\n");

    for (cf_inst_containter_t * tmp = cfRegistry->instHead; tmp != NULL;
         tmp = tmp->next) {
        fprintf(stdout, "%-32s %s\n", tmp->name, tmp->className);
    }
}

void
cf_interface_list(char *inst)
{
    for (cf_inst_containter_t * tmp = cfRegistry->instHead; tmp != NULL;
         tmp = tmp->next) {
        if (!strcmp(inst, tmp->name)) {
            fprintf(stdout, "Interfaces implemented by (%s)\n", inst);
            fprintf(stdout,
                    "------------------------------------------------------\n");
            for (cf_iface_containter_t * i = tmp->ifaceHead; i != NULL;
                 i = i->next) {
                fprintf(stdout, "%-32s\n", i->name);
            }
            return;
        }
    }

    cf_error_log(__FILE__, __LINE__, "No instance with named (%s)\n", inst);
}

void
cf_class_list(void)
{
    cf_class_container_t *tmp;

    fprintf(stdout, "Registered classes\n");
    fprintf(stdout, "------------------------------------------------------\n");
    for (tmp = cfRegistry->classHead; tmp != NULL; tmp = tmp->next) {
        fprintf(stdout, "%-32s\n", tmp->name);
    }
}
