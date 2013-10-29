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

/** @addtogroup samplecomps1 Sample Component #1
 *  @{
 */

/** This is a sample component */
#include "compframe.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "iface.h"
#include "compframe_interfaces.h"
#include "compframe_iface_cfg.h"

/** Test component "class" */
typedef struct TestComp {
    char *instName;               /* Instance name */

    iface_test2* testIface;

} TestComp;

/** Variable for our interface implementation */
static iface_test myIface;
static cfi_connect myConnectIface;
static cfi_cfg_client myCfgIface;


/* Predeclaration of function used later on. */
static void* 
create_me(const char* inst_name);
static void  
set_me_up(void* comp);
static int  
destroy_me(void* comp);

static void  
iface_func(void);

static void  
iface_func2(char* str);

static int 
connect_me(void* obj, void* other, char* iface, int argc, char** argv);

static int 
disconnect_me(void* obj, void* other, char* iface);

static int config_me(void* cfg, void* comp, 
                     char* varName, char* varValue);

/** This function must reside in all component libraries.
    Here the component instance is 
*/
void 
dlopen_this(void)
{
    cf_component_register("TESTCOMP", create_me, set_me_up, destroy_me);
}


/** This function is used to create and initate a component. 
    @return Pointer to created instance or NULL 
*/
static void* 
create_me(const char* inst_name)
{
    TestComp* comp = malloc(sizeof(TestComp));

    assert(inst_name!=NULL);

    comp->instName = strdup(inst_name);
    comp->testIface = NULL;
    return comp;
}

static int  
destroy_me(void* comp)
{
    if (!comp) {
        return 1;
    }

    TestComp* t = (TestComp*) comp;

    /* De-register our interfaces */
    cf_interface_deregister(comp);

    free(t->instName);
    free(t);

    return 0;
}


static void
set_me_up(void* comp)
{ 
    /* Setup the function pointers for the interface we implement */
    myIface.printHello = iface_func;
    myIface.print      = iface_func2;

    myConnectIface.connect    = connect_me;
    myConnectIface.disconnect = disconnect_me;

    myCfgIface.set = config_me;

    /* Register our interfaces */
    cf_iface_reg_t iface[] = {
        {CF_CONNECT_IFACE_NAME, &myConnectIface}, /* Want to use 'connect' command */
        {TEST_IFACE_NAME, &myIface},
        {CF_CFG_CLIENT_IFACE_NAME, &myCfgIface},

        {NULL,NULL}
    };


    cf_interface_register(comp, iface);
}


static void  
iface_func(void)
{
    fprintf(stderr,"Hello!\n");
}

static void  
iface_func2(char* str)
{
    fprintf(stderr,"%s\n",str);
}


static int 
connect_me(void* obj, void* other, char* iface, int argc, char** argv)
{
    TestComp* this = (TestComp*) obj;

    (void) argc;                  /* Avoid warnings */
    (void) argv;                  /* Avoid warnings */

    if (this->testIface) {
        fprintf(stderr,"%s:%d Already connected\n",__FILE__, __LINE__);
        return 0;
    }

    this->testIface = cf_interface_get(other,iface);

    if (!this->testIface) {
        fprintf(stderr,
                "%s:%d Interface not implemented by peer (%p)\n",
                __FILE__, __LINE__, this->testIface);
        return 0;
    }

    fprintf(stderr," Connecting....: %d\n", argc);

    for (int i = 0; i < argc; i++) {
        fprintf(stderr,"      argument ARGV[%d]: (%s)\n", i, argv[i]);
    }

    /* Just call a function in the interface to show that it is possible */
    if (!strcmp(iface, TEST2_IFACE_NAME)) {
        this->testIface->printGoodbye();
    }

    return 1;
}

static int 
disconnect_me(void* obj, void* other, char* iface)
{
    (void) obj;
    (void) other;  
    (void) iface;
    return 1;
}

static int
config_me(void* cfg, void* comp, char* varName, char* varValue)
{
    (void) cfg;

    fprintf(stderr,"  %s: Setting %s to '%s' on %p\n",
            __FILE__, varName, varValue, comp);

    return 1;
}


/** @} */
