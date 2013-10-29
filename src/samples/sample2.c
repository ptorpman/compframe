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

/** This is a sample component too */

/** @addtogroup samplecomps Sample Component #2
 *  @{
 */


#include "compframe.h"
#include "compframe_interfaces.h"
#include "compframe_iface_m.h"
#include "compframe_iface_s.h"
#include "compframe_iface_c.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "iface.h"


#define TEST_UUID "433e76d0-77d1-460d-9321-e2dc8dc8bd59"


typedef struct TestComp {
    char *instName;               /* Instance name */


    cfi_m_server* m;              /* Pointer to M's server interface */
    void*         mObj;
} TestComp;



/** Variable for our interface implementation */
static iface_test2 myIface;
/* Predeclaration of function used later on. */
static void* 
create_me(const char* inst_name);
static void  
set_me_up(void* comp);
static int  
destroy_me(void* comp);

static void  
iface_func(void);

/** M client interface */
static cfi_m_client mifClient;
static int 
m_cli_connected(void* comp, void* conn, uint8_t chan,void* userData);
static int 
m_cli_disconnected(void* comp, void* conn, uint8_t chan,void* userData);
static int  
m_cli_message(void*          comp, 
              void*       conn, 
              uint8_t        chan, 
              int            len, 
              unsigned char* msg,
              void*          userData);

/** S client interface  */
static cfi_s_client sClient;
static void 
s_client_execute(void* obj, uint32_t slice, int* sliceRemain );


/** This function must reside in all component libraries.
    Here the component instance is 
*/
void 
dlopen_this(void)
{
    /* Register this component */
    cf_component_register("TESTCOMP2", create_me, set_me_up, destroy_me);
}


/** This function is used to create and initate a component. 
    @return Pointer to created instance or NULL 
*/
static void* 
create_me(const char* inst_name)
{
    TestComp* comp = malloc(sizeof(TestComp));

    /* Make sure we got a name */
    assert(inst_name!=NULL);

    /* Remember our name  */
    comp->instName = strdup(inst_name);

    return comp;
}

static int  
destroy_me(void* comp)
{
    if (!comp) {
        return 1;
    }

    TestComp* this = (TestComp*) comp;

    /* De-register our message receivers */
    this->m->mr_del(this->mObj, TEST_UUID,"sample2a");
    this->m->mr_del(this->mObj, TEST_UUID,"sample2b");

    /* Get out of scheduler */
    void* sObj = cf_component_get(CF_S_NAME);
    cfi_s_server* sIface = cf_interface_get(sObj, CF_S_SERVER_IFACE_NAME);

    sIface->remove(sObj,this);

    /* De-register our interfaces */
    cf_interface_deregister(comp);

    free(this->instName);
    free(this);

    return 0;
}


static void
set_me_up(void* comp)
{ 
    TestComp* this = (TestComp*) comp;


    /* Set up implementation of iface_test2  */
    myIface.printGoodbye = iface_func;


    /* Set up implementation of M client interface  */
    mifClient.connected = m_cli_connected;
    mifClient.disconnected = m_cli_disconnected;
    mifClient.message = m_cli_message;

    /* Scheduler client interface */
    sClient.execute = s_client_execute;

    /* Register our interfaces */
    cf_iface_reg_t iface[] = {
        {TEST2_IFACE_NAME, &myIface},
        {CF_M_CLIENT_IFACE_NAME, &mifClient},
        {CF_S_CLIENT_IFACE_NAME, &sClient},
        {NULL,NULL}
    };


    cf_trace_log(__FILE__,__LINE__, CF_TRACE_DEBUG,
                 "Adding interfaces...\n");

    /* Register interface*/
    cf_interface_register(comp, iface);


    /* Store reference to M */
    this->mObj = cf_component_get(CF_M_NAME);

    /* Get server interface of M */
    this->m = cf_interface_get(this->mObj, CF_M_SERVER_IFACE_NAME);

    if (!this->m) {
        cf_error_log(__FILE__,__LINE__,
                     "Could not get interface of M!\n");
        return;
    }

#ifdef SUPERTEST
    cf_trace_log(__FILE__,__LINE__, CF_TRACE_DEBUG,
                 "Adding receivers...\n");
    this->m->mr_add(this->mObj,this,TEST_UUID,"sample1","sample1");
    this->m->mr_add(this->mObj,this,TEST_UUID,"sample2","sample2");
    this->m->mr_add(this->mObj,this,TEST_UUID,"sample3","sample3");
    this->m->mr_add(this->mObj,this,TEST_UUID,"sample4","sample4");

    cf_trace_log(__FILE__,__LINE__, CF_TRACE_DEBUG,
                 "Removing receivers...\n");
    this->m->mr_del(this->mObj,TEST_UUID,"sample1");
    this->m->mr_del(this->mObj,TEST_UUID,"sample2");
    this->m->mr_del(this->mObj,TEST_UUID,"sample3");
    this->m->mr_del(this->mObj,TEST_UUID,"sample4");

    cf_trace_log(__FILE__,__LINE__, CF_TRACE_DEBUG,
                 "Adding receivers again...\n");

#endif  /* SUPERTEST */

    this->m->mr_add(this->mObj,this,TEST_UUID,"sample2a","sample2a");
    this->m->mr_add(this->mObj,this,TEST_UUID,"sample2b","sample2b");


    /* Add ourself to the S scheduling loop */

    void* sObj = cf_component_get(CF_S_NAME);

    cfi_s_server* sIface = cf_interface_get(sObj, CF_S_SERVER_IFACE_NAME);

    sIface->add(sObj,this);

}

/** Our implementation of iface_test2's function 'printGoodbye' */
static void  
iface_func(void)
{
    fprintf(stderr,"Good bye!\n");
}





/**************** M CLIENT INTERFACE IMPLEMENTATION      *************/
static int 
m_cli_connected(void* comp, void* conn, uint8_t chan,void* userData)
{
    TestComp* this = (TestComp*) comp;


    cf_info_log("Connected comp=%p conn=%p chan=%u \n",
                comp,
                conn,
                chan);

    /* Disable our receiver if we only support one connection per
       receiver */
    this->m->mr_disable(this->mObj,TEST_UUID,(char*)userData);



    return 1;
}

static int 
m_cli_disconnected(void* comp, void* conn, uint8_t chan,void* userData)
{
    TestComp* this = (TestComp*) comp;

    cf_info_log("Disconnected comp=%p conn=%p chan=%u \n",
                comp,
                conn,
                chan);

    /* Re-enable our receiver if we only support one connection per
       receiver */
    this->m->mr_enable(this->mObj,TEST_UUID,(char*)userData);


    return 1;
}
static int  
m_cli_message(void*          comp, 
              void*          conn, 
              uint8_t        chan, 
              int            len, 
              unsigned char* msg,
              void*          userData)
{
    TestComp* this = (TestComp*) comp;

    cf_info_log("Got message to %s: conn=%p chan=%u len=%d MSG=%s\n",
                (char*) userData,
                conn,
                chan,
                len,
                msg);

    char newMsg[256];

    strcpy(newMsg,"Hello, yourself!\n");

    this->m->send(this->mObj,conn,chan,strlen(newMsg)+1,(unsigned char*)newMsg);

    return 1;
}

/**************** M CLIENT INTERFACE IMPLEMENTATION END  *************/

/**************** S CLIENT INTERFACE IMPLEMENTATION START  *************/

static void 
s_client_execute(void* obj, uint32_t slice, int* sliceRemain )
{
    *sliceRemain = 0;

    cf_trace_log(__FILE__,__LINE__, CF_TRACE_MASSIVE,
                 "%s is scheduled with slice %u\n", 
                 cf_component_name_get(obj), slice);
}


/**************** S CLIENT INTERFACE IMPLEMENTATION END  *************/

/** @} */

