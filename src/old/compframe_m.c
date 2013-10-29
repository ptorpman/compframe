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
#include "compframe_sockets.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "iface.h"
#include "compframe_iface_m.h"
#include "compframe_iface_c.h"
#include "compframe_util.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "compframe_m.h"
#include <sys/uio.h>
#include <fcntl.h>
#include <errno.h>

/** @addtogroup m M - Message Transport
 *  @{
 */

/*===========================================================================*/
/* MACROS                                                                    */
/*===========================================================================*/
/** Number of bytes in a UUID */
#define CF_UUID_LEN 36

/** Usage string from 'm' command */
#define M_CMD_USAGE "Usage: m [-r | -l]\n"

/*===========================================================================*/
/* TYPES                                                                     */
/*===========================================================================*/

/** Type used for storing a message receiver */
typedef struct CfmReceiver {
    /** Pointer to next */
    struct CfmReceiver *next;
    /** Pointer to previous */
    struct CfmReceiver *prev;
    /** Flag if visible for connectors.  */
    int enabled;
    /** Name of receiver */
    char *name;
    /** Pointer to user context */
    void *comp;

    /** Pointer to user's data */
    void *userData;

    /** Pointer to M client interface in client */
    cfi_m_client *iface;
} CfmReceiver;

/** Type used for storing an interface */
typedef struct cfm_iface_t {
    /** Pointer to next */
    struct cfm_iface_t *next;
    /** Pointer to previous */
    struct cfm_iface_t *prev;

    /** UUID of interface  */
    char uuid[CF_UUID_LEN + 1];
    /** Name of interface  */
    char *name;

    /** Head of list of receiver for this interface  */
    CfmReceiver *head;

} cfm_iface_t;

/** This is the message transport component of CompFrame */
typedef struct cfm_t {
    /** Instance name */
    char *inst_name;
    /** Server socket */
    int server_socket;

    /** Server port  */
    int port;

    /** Head of interface list  */
    cfm_iface_t *if_head;
    /** Head of connections list  */
    cfm_conn_t *conn_head;

} cfm_t;

/*===========================================================================*/
/* VARIABLES                                                                 */
/*===========================================================================*/

/** The M server interface that we implement */
static cfi_m_server mIface;

/** Host name */
static char *hostName = NULL;

/*===========================================================================*/
/* FUNCTION DECLARATIONS                                                     */
/*===========================================================================*/

static void *create_me(const char *inst_name);
static void
set_me_up(void *comp);
static int
destroy_me(void *comp);

static cfm_iface_t *m_iface_add(void *m, const char *uuid, char *name);
static int
m_receiver_add(void *m, void *comp, const char *uuid, char *name, void *userData);
static int
m_receiver_enable(void *m, const char *uuid, char *name);
static int
m_receiver_disable(void *m, const char *uuid, char *name);
static int
m_receiver_del(void *m, const char *uuid, char *name);
static int
m_port_get(void *m);
static char *m_search_by_name(void *m, char *name);
static char *m_search_by_if(void *m, const char *uuid);
static int
m_message_send(void *m, void *conn, uint8_t chan,
               int len, unsigned char *msg);

static cfm_iface_t *if_get(cfm_t * this, const char *uuid);
static int
if_ok(const char *uuid);

static CfmReceiver *mr_get(cfm_t * this, const char *uuid, char *name);

static int
m_cmd(int argc, char **argv);

static int
m_server_socket_init(void *comp);

static int
m_server_socket_handle(void *comp, int sd, void *userData, cf_sock_event_t ev);

static cfm_conn_t *m_connection_get(cfm_t * this, int sd);

static int
m_remote_m_message_handle(cfm_t * this, cfm_conn_t * conn, int sd);
static int
m_client_message_handle(cfm_t * this, cfm_conn_t * conn, int sd);
static int
m_message_pass_to_client(cfm_t * this, cfm_conn_t * conn, int sd);

static int
m_message_response(int sd, int order, int result, int response, char *responseText);

/*===========================================================================*/
/* FUNCTION DEFINITIONS                                                      */
/*===========================================================================*/

/** This function must reside in all component libraries.
    Here the component instance is 
*/
void
dlopen_this(void)
{
    /* Use this function to get M into the Registry  */
    cf_component_register("M", create_me, set_me_up, destroy_me);
}

/** This function is used to create and initate a component. 
    @return Pointer to created instance or NULL 
*/
static void *
create_me(const char *inst_name)
{

    assert(inst_name != NULL);

    cfm_t *m = malloc(sizeof(cfm_t));

    m->inst_name = strdup(inst_name);
    m->server_socket = -1;
    m->port = 0;
    m->if_head = NULL;
    m->conn_head = NULL;

    return m;
}

static int
destroy_me(void *comp)
{
    cfm_t *m = (cfm_t *) comp;

    if (!m) {
        return 1;
    }

    if (m->if_head) {
        cf_error_log(__FILE__, __LINE__,
                     "Receivers still registered! (%s)\n", m->inst_name);
        return 1;
    }

    /* De-register our interfaces */
    cf_interface_deregister(comp);

    free(m->inst_name);
    free(m);

    return 0;
}

/** Checks if an interface is registered.
    @param this  Pointer to M context
    @param uuid  UUID string of interface
    @return Pointer to interface struct or NULL.
*/
static cfm_iface_t *
if_get(cfm_t * this, const char *uuid)
{
    for (cfm_iface_t * i = this->if_head; i != NULL; i = i->next) {
        if (memcmp(i->uuid, uuid, strlen(uuid)) == 0) {
            /* Found the interface */
            return i;
        }
    }

    /* Could not find it! */
    return NULL;
}

/** Performs some checks to see if an interface and a name seems 
    to be OK
    @param uuid  Interface UUID
    @return 1 if OK, 0 if not.
*/
static int
if_ok(const char *uuid)
{
    if (!uuid) {
        cf_error_log(__FILE__, __LINE__, "Interface UUID missing!\n");
        return 0;
    }
    if (strlen(uuid) != CF_UUID_LEN) {
        cf_error_log(__FILE__, __LINE__, "Bad UUID! (%s)\n", uuid);
        return 0;
    }

    /* A-OK! */
    return 1;
}

/** Finds and returns a message receiver
    @param this Pointer to this context
    @param uuid Interface UUID
    @param name Name of receiver
    @return Pointer to receiver or NULL
*/
static CfmReceiver *
mr_get(cfm_t * this, const char *uuid, char *name)
{
    cfm_iface_t *i;
    CfmReceiver *r;

    i = this->if_head;

    while (i) {
        if (memcmp(i->uuid, uuid, CF_UUID_LEN) == 0) {

            r = i->head;

            while (r) {

                /* Found the interface. Now look for the receiver */
                if (!strcmp(r->name, name)) {
                    /* Found it! */
                    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                                 "Found receiver name %s with IID %s \n",
                                 name, uuid);
                    return r;
                }

                r = r->next;
            }
        }

        i = i->next;
    }

    /* Nowhere to be found */
    return NULL;
}

/** Adds an interface to our list of interfaces */
static cfm_iface_t *
m_iface_add(void *m, const char *uuid, char *name)
{
    cfm_t *this = (cfm_t *) m;

    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 0;
    }

    /* Already registered? */
    if (if_get(this, uuid) != NULL) {
        /* Yes. */
        cf_error_log(__FILE__, __LINE__,
                     "Interface already registered (%s)\n", uuid);
        return NULL;
    }

    /* Create and add the interface */
    cfm_iface_t *i = malloc(sizeof(cfm_iface_t));

    i->prev = NULL;
    i->next = NULL;
    memcpy(i->uuid, uuid, CF_UUID_LEN);
    i->uuid[CF_UUID_LEN] = 0;
    i->name = strdup(name);
    i->head = NULL;

    CF_LIST_ADD(this->if_head, i);

    return i;
}

/** Adds a receiver to our list of receivers */
static int
m_receiver_add(void *m, void *comp, const char *uuid, char *name,
               void *userData)
{
    cfm_t *this = (cfm_t *) m;

    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 0;
    }

    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return 0;
    }

    cfm_iface_t *i = if_get(this, uuid);

    if (i == NULL) {
        /* Not registered. So, let's register it! */
        i = m_iface_add(m, uuid, name);
    }

    /* Is it already there? */
    CfmReceiver *r = mr_get(this, uuid, name);

    if (r) {
        cf_error_log(__FILE__, __LINE__,
                     "Receiver already registered! (%s %s)\n", uuid, name);
        return 0;
    }

    cfi_m_client *iface = cf_interface_get(comp, CF_M_CLIENT_IFACE_NAME);

    if (!iface) {
        cf_error_log(__FILE__, __LINE__,
                     "Component (%s %s) does not implement the %s interface!\n",
                     uuid, name, CF_M_CLIENT_IFACE_NAME);
        return 0;
    }

    /* Create the new receiver */
    r = malloc(sizeof(CfmReceiver));

    r->next = NULL;
    r->prev = NULL;
    r->enabled = 1;
    r->name = strdup(name);
    r->comp = comp;
    r->iface = iface;
    r->userData = userData;

    CF_LIST_ADD(i->head, r);

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Added receiver %s %s...\n", uuid, name);

    return 1;
}

/** Marks a receiver as available for new connections  */
static int
m_receiver_enable(void *m, const char *uuid, char *name)
{
    cfm_t *this = (cfm_t *) m;

    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 0;
    }

    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return 0;
    }

    CfmReceiver *r = mr_get(this, uuid, name);

    r->enabled = 1;

    return 1;
}

/** Marks a receiver as unavailable for new connections  */
static int
m_receiver_disable(void *m, const char *uuid, char *name)
{
    cfm_t *this = (cfm_t *) m;

    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 0;
    }

    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return 0;
    }

    CfmReceiver *r = mr_get(this, uuid, name);

    r->enabled = 0;

    return 1;
}

/** Removes a receiver from our list */
static int
m_receiver_del(void *m, const char *uuid, char *name)
{
    cfm_t *this = (cfm_t *) m;

    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 0;
    }
    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return 0;
    }

    cfm_iface_t *i = if_get(this, uuid);
    CfmReceiver *r = mr_get(this, uuid, name);

    if (!r) {
        cf_error_log(__FILE__, __LINE__,
                     "Invalid received! (%s %s)\n", uuid, name);
        return 0;
    }

    /*
     * FIXME!!!!
     * 
     * We need to close down any open connections to this
     * receiver
     */

    /* Detach it from the list */
    CF_LIST_REMOVE(i->head, r);

    free(r->name);

    /* Was it the last in the list? */
    if (i->head == NULL) {
        /* Yes. Remove the interface as well. */
        CF_LIST_REMOVE(this->if_head, i);
        free(i->name);
        free(i);
    }

    return 1;
}

/** Returns the server port of M*/
static int
m_port_get(void *m)
{
    cfm_t *this = (cfm_t *) m;

    return this->port;
}

/** Returns a list of receivers that begins with a certain name */
static char *
m_search_by_name(void *m, char *name)
{
    cfm_t *this = (cfm_t *) m;

    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return 0;
    }

    /*
     * If registered receivers are 
     * 46fa5287-f3e8-4040-9bcd-acc0ef064adc {TEST1 TEST2}
     * 5133ccbc-30f9-433d-b6c3-618b1e89b87a {X1 X2}
     * f2f0d34e-0191-47b9-817a-76f03d89e666 {TEMP TEMP2}
     * 
     * And 'name' is 'TE', then we will return the following
     * string:
     * 
     * 46fa5287-f3e8-4040-9bcd-acc0ef064adc:TEST1,TEST2; <no space>
     * f2f0d34e-0191-47b9-817a-76f03d89e666:TEMP,TEMP2;
     */

    char *tmp = NULL;
    int pos = 0;
    int idAdded = 0;
    int posBefore = 0;
    int posAfter = 0;

    for (cfm_iface_t * i = this->if_head; i != NULL; i = i->next) {
        posBefore = pos;

        for (CfmReceiver * r = i->head; r != NULL; r = r->next) {
            if (!strncmp(r->name, name, strlen(name))) {
                /* Found a winner */
                if (idAdded == 0) {
                    /* Add UUID first */
                    tmp = realloc(tmp, pos + CF_UUID_LEN + 1);

                    if (pos == 0) {
                        sprintf(tmp, "%s:", i->uuid);
                    }
                    else {
                        sprintf(&tmp[pos], "%s%s:", tmp, i->uuid);
                    }

                    pos += CF_UUID_LEN + 1;

                    idAdded = 1;
                }

                /* Add the name of the receiver */
                tmp = realloc(tmp, pos + strlen(r->name) + 1);

                sprintf(&tmp[pos], "%s%s,", tmp, r->name);

                pos += strlen(name) + 1;
            }
        }

        if (posAfter > posBefore) {
            /* Added some stuff.. Add the trailing semi-colon
             * (and overwrite any trailing comma) */
            tmp[pos] = ';';
        }

        /* Take next UUID */
        idAdded = 0;
    }

    return tmp;
}

/** Returns a list of receivers with a specific UUID (i.e interface) */
static char *
m_search_by_if(void *m, const char *uuid)
{
    cfm_t *this = (cfm_t *) m;

    /* FIXME! */
    (void) this;
    (void) uuid;

    return NULL;
}

/** The main 'M' command.*/
static int
m_cmd(int argc, char **argv)
{
    cfm_t *this = cf_component_get(CF_M_NAME);
    int x = 0;

    switch (argc) {

    case 2:
        /* -r */

        if (!strcmp(argv[1], "-r")) {
            fprintf(stdout,
                    "------------------------------------------------------\n");
            fprintf(stdout, "- M: Registered Interfaces And Receivers\n");
            fprintf(stdout,
                    "------------------------------------------------------\n");

            for (cfm_iface_t * i = this->if_head; i != NULL; i = i->next) {
                fprintf(stdout, "%s\n\t", i->uuid);

                for (CfmReceiver * r = i->head; r != NULL; r = r->next) {
                    fprintf(stdout, "%s (enabled=%d)  ", r->name, r->enabled);
                    x++;

                    if (x == 2) {
                        fprintf(stdout, "\n\t");
                        x = 0;
                    }
                }
                fprintf(stdout, "\n");
            }

            return 1;
        }
        else if (!strcmp(argv[1], "-l")) {
            fprintf(stdout, "M server located at %s:%d\n",
                    hostName, this->port);
            return 1;

        }
        else {
            cf_error_log(__FILE__, __LINE__, M_CMD_USAGE);
            return 0;
        }

    default:
        cf_error_log(__FILE__, __LINE__, M_CMD_USAGE);
        return 0;
    }

    return 0;
}

/** Initiates the server socket of M */
static int
m_server_socket_init(void *comp)
{
    int s;
    int res;
    cfm_t *this = (cfm_t *) comp;

    /* Open socket */
    s = socket(AF_INET, SOCK_STREAM, 0);

    if (s == -1) {
        cf_error_log(__FILE__, __LINE__, "Failed to open socket!\n");
        return 0;
    }

    /* Make it possible to reuse socket */
    int reuse = 1;

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse));

    /* Bind socket */
    struct sockaddr_in servaddr;

    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = 0;

    res = bind(s, (struct sockaddr *) &servaddr, sizeof(servaddr));

    if (res < 0) {
        cf_error_log(__FILE__, __LINE__, "Failed to bind socket!\n");
        shutdown(s, SHUT_RDWR);
        return 0;
    }

    struct sockaddr_in sa;

    socklen_t addrLen = sizeof(struct sockaddr_in);

    res = getsockname(s, (struct sockaddr *) &sa, &addrLen);

    this->port = ntohs(sa.sin_port);

    cf_info_log("M server started on %s:%d\n", hostName, this->port);

    /* Listen on socket */
    listen(s, 5);

    /* Remember the value */
    this->server_socket = s;

    /* Make socket non-blocking */
    fcntl(s, F_SETFL, O_NONBLOCK);

    /* Make sure our socket gets polled. We'll be called in 
     * m_server_socket_handle() if something happens */
    cf_socket_register(this, s, m_server_socket_handle, comp);

    return 1;
}

/** Return the connection for a specific socket descriptor
    @param sd Socket descriptor
    @return Pointer to connection or NULL
*/
static cfm_conn_t *
m_connection_get(cfm_t * this, int sd)
{
    for (cfm_conn_t * c = this->conn_head; c != NULL; c = c->next) {
        if (c->socket_fd == sd) {
            /* Found it! */
            return c;
        }
    }

    return NULL;
}

/** Handle all our sockets
    @param comp Our context
    @param sd   Socket descriptor
    @param userData Own
*/
static int
m_server_socket_handle(void *comp, int sd, void *userData, cf_sock_event_t ev)
{
    cfm_t *this = (cfm_t *) comp;
    static unsigned char readBuff[CF_M_MAX_MESSAGE];

    (void) userData;            /* Avoid warnings */

    if (sd == this->server_socket) {
        if (ev != CF_SOCKET_STUFF_TO_READ) {
            /* Something bad has happened to the server socket. Cannot continue */
            cf_error_log(__FILE__, __LINE__,
                         "M's server socket is dead! Panic!\n");
            return 0;
        }

        /* Time to accept some new connections */
        int remote;
        struct sockaddr_in remote_addr;
        socklen_t length = sizeof(struct sockaddr_in);

        remote = accept(this->server_socket,
                        (struct sockaddr *) &remote_addr, &length);

        if (remote < 0) {
            cf_error_log(__FILE__, __LINE__,
                         "Could not accept connection! errno=%d\n", errno);
            return 0;
        }

        /* Make it possible to reuse socket */
        int reuse = 1;

        setsockopt(remote, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse,
                   sizeof(reuse));

        /* Create structure for keeping this new connection */
        cfm_conn_t *conn = malloc(sizeof(cfm_conn_t));

        for (int i = 0; i < CFM_MAX_PEERS; i++) {
            conn->peer[i] = NULL;
        }

        conn->socket_fd = remote;
        conn->num_used = 0;
        conn->state = CFM_CONN_INIT;
        conn->isM = 0;

        CF_LIST_ADD(this->conn_head, conn);

        /* a new connection has been established. Make sure we poll it. */
        cf_socket_register(this, remote, m_server_socket_handle, comp);

        return 1;
    }

    /* It was a client socket. Here we need to take care of all the orders
     * that might come from the clients */

    int n;

    /* Read as much as possible */

    n = read(sd, readBuff, CF_M_MAX_MESSAGE);

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Read %d bytes from socket %d.\n", n, sd);

    if (n < 0) {
        /* Fault! */
        cf_error_log(__FILE__, __LINE__, "Read error (%d)!\n", n);
        return 0;
    }

    cfm_conn_t *conn = m_connection_get(this, sd);

    if (n == 0) {
        cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                     "Closed down socket %d.\n", sd);

        for (int i = 0; i < CFM_MAX_PEERS; i++) {
            if (conn->peer[i] == NULL) {
                continue;
            }

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "Informing peer/chan %d about disconnection...\n", i);
            CfmReceiver *rec = conn->peer[i]->localReceiver;

            rec->iface->disconnected(rec->comp, conn, i, rec->userData);
            free(conn->peer[i]);
            conn->peer[i] = NULL;
        }

        /* The socket is closed down. Remove it from polling */
        cf_socket_deregister(sd);
        close(sd);

        /* Go through all connections and see if someone was connected over this
         * socket. */

        if (!conn) {
            cf_error_log(__FILE__, __LINE__,
                         "Socket closed but not in lists! Fatal!\n", n);
            return 0;
        }

        return 1;
    }

    int handled = 0;

    while (handled < n) {
        switch (conn->state) {
        case CFM_CONN_INIT:
            conn->channel = readBuff[handled++];
            conn->state = CFM_CONN_CHANNEL;

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "M message: Channel=%d (%x)\n",
                         conn->channel, readBuff[0]);

            break;
        case CFM_CONN_CHANNEL:
            conn->msg_len = readBuff[handled++];
            conn->state = CFM_CONN_LENGTH1;

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "M message: Length=%d (1)\n", conn->msg_len);

            break;
        case CFM_CONN_LENGTH1:
            conn->msg_len = (readBuff[handled++] << 8) + conn->msg_len;
            conn->state = CFM_CONN_LENGTH2;

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "M message: Length=%d (2)\n", conn->msg_len);

            break;
        case CFM_CONN_LENGTH2:
            /* Allocate message buffer */
            conn->msg_buff = malloc(conn->msg_len * sizeof(unsigned char));
            /* NULL terminate just to be safe */
            conn->msg_buff[0] = 0;
            conn->msg_pos = 0;
            conn->state = CFM_CONN_MSGBODY;

            break;
        case CFM_CONN_MSGBODY:
            conn->msg_buff[conn->msg_pos++] = readBuff[handled++];

            if ((conn->msg_pos + 3) == conn->msg_len) {
                /* All bytes are in there! */
                cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                             "M message: All bytes there\n");

                conn->state = CFM_CONN_MSGREADY;
            }

            break;
        default:
            cf_error_log(__FILE__, __LINE__, "Error in message handling!\n");
            break;
        }

        /* Message ready to be handled? */
        if (conn->state == CFM_CONN_MSGREADY) {
            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "M message ready.\n");

            if (conn->channel == CFM_M_CHANNEL) {
                cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                             "M control message.\n");

                if (conn->isM) {
                    /* A remote M component is connected */
                    m_remote_m_message_handle(this, conn, sd);
                }
                else {
                    /* A "normal" M client is connected here */
                    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                                 "Handling M control message.\n");
                    m_client_message_handle(this, conn, sd);
                }
            }
            else {
                cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                             "Passing message to client.\n");
                m_message_pass_to_client(this, conn, sd);
            }

            free(conn->msg_buff);
            conn->msg_buff = NULL;
            conn->state = CFM_CONN_INIT;
        }

    }

    return 1;
}

/** Function called after M has been created */
static void
set_me_up(void *comp)
{
    /* Setup the function pointers for the interface we implement */
    mIface.mr_add = m_receiver_add;
    mIface.mr_disable = m_receiver_disable;
    mIface.mr_enable = m_receiver_enable;
    mIface.mr_del = m_receiver_del;
    mIface.port_get = m_port_get;
    mIface.search_by_name = m_search_by_name;
    mIface.search_by_if = m_search_by_if;
    mIface.send = m_message_send;

    /* Get host name */
    hostName = getenv("HOST");

    if (!hostName) {
        hostName = getenv("HOSTNAME");
    }

    if (!hostName) {
        hostName = "localhost";
    }

    /* Register our interfaces */
    cf_iface_reg_t iface[] = {
        {CF_M_SERVER_IFACE_NAME, &mIface}
        ,
        {NULL, NULL}
    };

    cf_interface_register(comp, iface);

    /* Register our commands */
    {
        void *cObj = cf_component_get(CF_C_NAME);
        cfi_c *cIface = cf_interface_get(cObj, CF_C_IFACE_NAME);

        cIface->add(cObj, "m", m_cmd, M_CMD_USAGE);
    }

    /* Start up the server socket */
    if (!m_server_socket_init(comp)) {
        cf_error_log(__FILE__, __LINE__, "Could not start M server socket!\n");
        return;
    }
}

/** Handles messages from remotely connected M servers
    @param this  This context
    @param conn  Connection to handle
    @param sd    Socket descriptor for response
    @return 1 if OK, 0 if failure
*/
static int
m_remote_m_message_handle(cfm_t * this, cfm_conn_t * conn, int sd)
{
    /* FIXME: Add functionality for handling remote M server connections  */
    (void) this;
    (void) conn;
    (void) sd;
    return 0;
}

/** Handles messages from connected M clients
    @param this  This context
    @param conn  Connection to handle
    @param sd    Socket descriptor for response
    @return 1 if OK, 0 if failure
*/
static int
m_client_message_handle(cfm_t * this, cfm_conn_t * conn, int sd)
{
    unsigned char iid[CF_UUID_LEN + 1];
    unsigned char *msg = conn->msg_buff;
    char *name = NULL;
    CfmReceiver *rec = NULL;

    switch (msg[0]) {
    case CF_M_CHANNEL_OPEN:

        memcpy(&iid[0], &msg[1], CF_UUID_LEN);

        iid[CF_UUID_LEN] = 0;

        name = (char *) &msg[37];

        rec = mr_get(this, (char *) iid, name);

        if (!rec) {
            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "CF_M_CHANNEL_OPEN to %s %s FAILED!\n", iid, name);

            m_message_response(sd,
                               CF_M_CHANNEL_OPEN,
                               CF_M_CHANNEL_OPEN_FAIL,
                               CF_M_COMP_NOT_FOUND, "Component not found!\n");
            return 0;
        }

        int newChan = -1;

        /* Component there.. get free channel */
        for (int i = 0; i < CFM_MAX_CHANNELS; i++) {
            if (conn->peer[i] == NULL) {
                /* Found empty slot */
                newChan = i;
                break;
            }
        }

        if (newChan == -1) {
            /* No new channel to be found */

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "CF_M_CHANNEL_OPEN to %s %s FAILED...\n", iid, name);

            m_message_response(sd,
                               CF_M_CHANNEL_OPEN,
                               CF_M_CHANNEL_OPEN_FAIL,
                               CF_M_OUT_OF_CHANNELS, "Out of channels!\n");
            return 0;
        }

        /* Create new peer and insert it. */
        cfm_peer_t *newPeer = malloc(sizeof(cfm_peer_t));

        conn->peer[newChan] = newPeer;
        newPeer->channel = newChan;
        newPeer->sd = sd;
        newPeer->localReceiver = rec;

        /* Call open callback on receiver */
        int res =
            rec->iface->connected(rec->comp, conn, newChan, rec->userData);

        if (res == 0) {
            /* Failed to open channel */

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "CF_M_CHANNEL_OPEN to %s %s FAILED...\n", iid, name);

            m_message_response(sd,
                               CF_M_CHANNEL_OPEN,
                               CF_M_CHANNEL_OPEN_FAIL,
                               CF_M_COULD_NOT_CONNECT, "Connection failed!\n");

            /* Remove the newly installed peer */
            conn->peer[newChan] = NULL;
            free(newPeer);
        }
        else {
            /* Managed to open channel */

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "CF_M_CHANNEL_OPEN to %s %s SUCCEEDED...\n", iid,
                         name);

            m_message_response(sd,
                               CF_M_CHANNEL_OPEN,
                               CF_M_CHANNEL_OPEN_OK,
                               CF_M_CHANNEL_OPEN_OK, "Channel open OK!!\n");
        }

        break;

    case CF_M_CHANNEL_CLOSE:
    {
        int chan = msg[1];

        if (conn->peer[chan] == NULL) {
            m_message_response(sd,
                               CF_M_CHANNEL_CLOSE,
                               CF_M_CHANNEL_CLOSE_FAIL,
                               CF_M_COMP_NOT_FOUND,
                               "Component not found!!\n");

            return 0;
        }

        rec = (CfmReceiver *) conn->peer[chan]->localReceiver;

        cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                     "CF_M_CHANNEL_CLOSE to %s...\n", rec->name);

        int res =
            rec->iface->disconnected(rec->comp, conn, chan, rec->userData);

        if (res == 0) {
            /* Failed to close */
            m_message_response(sd,
                               CF_M_CHANNEL_CLOSE,
                               CF_M_CHANNEL_CLOSE_FAIL,
                               CF_M_CHANNEL_CLOSE_FAIL,
                               "Could not close!!\n");

        }
        else {
            m_message_response(sd,
                               CF_M_CHANNEL_CLOSE,
                               CF_M_CHANNEL_CLOSE_OK,
                               CF_M_CHANNEL_CLOSE_OK,
                               "Channel closed OK!!\n");
        }

        /* Remove the peer!  */
        free(conn->peer[chan]);
        conn->peer[chan] = NULL;

        break;
    }
    default:
        /* Unknown message */
        m_message_response(sd,
                           CF_M_CHANNEL_ORDER_UNKNOWN,
                           CF_M_CHANNEL_ORDER_UNKNOWN,
                           CF_M_CHANNEL_ORDER_UNKNOWN, "Unknown command!!\n");
        break;
    }

    return 1;
}

/** Handles messages meant for registered message receivers
    @param this  This context
    @param conn  Connection to handle
    @param sd    Socket descriptor for response
    @return 1 if OK, 0 if failure
*/
static int
m_message_pass_to_client(cfm_t * this, cfm_conn_t * conn, int sd)
{
    (void) this;
    (void) sd;

    int res;
    CfmReceiver *rec = (CfmReceiver *) conn->peer[conn->channel]->localReceiver;

    res = rec->iface->message(rec->comp,
                              conn,
                              conn->channel,
                              conn->msg_len, conn->msg_buff, rec->userData);

    return 0;
}

/** Send a message to the other side */
static int
m_message_response(int sd, int order, int result, int response,
                   char *responseText)
{
    unsigned char header[6];
    int len = 5;
    struct iovec iov[2];

    if (responseText) {
        len += strlen(responseText) + 1;
    }

    header[0] = CFM_M_CHANNEL;
    header[1] = (len & 0xFF);
    header[2] = (len >> 8) & 0xFF;
    header[3] = order;
    header[4] = result;
    header[5] = response;

    if (len == 6) {
        write(sd, header, 5);
    }
    else {
        iov[0].iov_base = header;
        iov[0].iov_len = 6;
        iov[1].iov_base = responseText;
        iov[1].iov_len = strlen(responseText) + 1;

        writev(sd, iov, 2);
    }

    return 1;
}

static int
m_message_send(void *m, void *c, uint8_t chan, int len, unsigned char *msg)
{
    cfm_conn_t *conn = (cfm_conn_t *) c;

    if (!m || !conn) {
        cf_error_log(__FILE__, __LINE__, "Bad parameters!\n");
        return 0;
    }

    cfm_peer_t *p = conn->peer[chan];
    unsigned char newMsg[3];
    int tot_len = 3 + len;

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Sending on channel %u\n", chan);

    newMsg[0] = chan;
    newMsg[1] = tot_len & 0xFF;
    newMsg[2] = (tot_len >> 8) & 0xFF;

    struct iovec io[2];

    io[0].iov_base = newMsg;
    io[0].iov_len = 3;

    io[1].iov_base = msg;
    io[1].iov_len = len;

    if (writev(p->sd, io, 2) < 0) {
        cf_error_log(__FILE__, __LINE__, "Failed to send message to client!\n");
        return 0;
    }

    return 1;

}

/** @} */
