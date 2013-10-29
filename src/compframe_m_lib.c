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

#include "compframe_m_lib.h"
#include "compframe_util.h"
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*===========================================================================*/
/* MACROS                                                                    */
/*===========================================================================*/

/*===========================================================================*/
/* TYPES                                                                     */
/*===========================================================================*/
/** Type used for keeping track of the stuff going on for a specific
    connection */
typedef enum {
    CFM_CONN_INIT = 0,
    CFM_CONN_CHANNEL = 1,
    CFM_CONN_LENGTH1 = 2,
    CFM_CONN_LENGTH2 = 3,
    CFM_CONN_MSGBODY = 4,
    CFM_CONN_MSGREADY = 5
} cfm_conn_state_t;

/** Used for keeping track of users of a specified connection */
typedef struct cfm_peer_t {
    /** Socket descriptor */
    int sd;
    /**  Channel used */
    int channel;
    /** Pointer to locally connected receiver  */
    void *localReceiver;
    /** Will be called when connection is opened */
    cfm_callback_open_t callback_open;
    /** Will be called when connection is closed */
    cfm_callback_close_t callback_close;
    /** Will be called when connection has a fault */
    cfm_callback_error_t callback_error;
    /** Will be called when message is received */
    cfm_callback_msg_t callback_msg;
    /** Will be returned to user in callbacks */
    void *userData;
} cfm_peer_t;

/** Used for M connections */
struct cfm_conn_t {
    /** Pointer to next  */
    struct cfm_conn_t *next;
    /** Pointer to previous  */
    struct cfm_conn_t *prev;

    /** Host name*/
    char *host;
    /** Port number */
    int port;

    /** Socket descriptor  */
    int socket_fd;
    /** Number of used channels (Range 0..255) */
    uint8_t num_used;
    /** Peers on this connection  */
    cfm_peer_t *peer[CFM_MAX_PEERS];
    /** State of buffer  */
    cfm_conn_state_t state;

    /** Flag if remote connection is a M compoent  */
    int isM;
    /** Current channel handled  */
    int channel;
    /** message length */
    int msg_len;
    /** Message buffer  */
    unsigned char *msg_buff;
    /** Message position */
    int msg_pos;
};

/*===========================================================================*/
/* VARIABLES                                                                 */
/*===========================================================================*/

/** List of open connections */
static cfm_conn_t *connHead = NULL;

/* Callback functions used when setting up a channel */
static cfm_callback_open_t callback_open = NULL;
static cfm_callback_close_t callback_close = NULL;
static cfm_callback_error_t callback_error = NULL;
static cfm_callback_msg_t callback_msg = NULL;
static void *curr_user_data = NULL;
static int channel_being_closed = -1;

/*===========================================================================*/
/* FUNCTION DECLARATIONS                                                     */
/*===========================================================================*/

static int
cfm_control_msg_handle(void *conn);

/*===========================================================================*/
/* FUNCTION DEFINITIONS                                                      */
/*===========================================================================*/

void *
cfm_connection_open(char *host, int port)
{
    if (!host || port < 0) {
        fprintf(stderr, "ERROR: Bad parameters\n");
        return NULL;
    }

    /* Reuse a connection if possible */
    if (connHead) {
        for (cfm_conn_t * tmp = connHead; tmp != NULL; tmp = tmp->next) {
            if (strcmp(tmp->host, host) == 0 && tmp->port == port) {
                /* Already a connection up for this one */
                return tmp;
            }
        }
    }

    struct hostent *he;
    struct in_addr inAddr;

    he = gethostbyname(host);

    if (!he) {
        /* See if it is a dotted IP address  */
        if (inet_aton(host, &inAddr) == 0) {
            fprintf(stderr, "ERROR: Unknown host ($%s)\n", host);
            return NULL;
        }
    }
    else {
        inAddr = *((struct in_addr *) he->h_addr);
    }
    /* Open up a socket to be used for this connection */
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    if (sd < 0) {
        fprintf(stderr, "ERROR: Could not open socket!\n");
        return NULL;
    }

    /* Make it possible to reuse socket */
    int reuse = 1;

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse));

    /* Connect the socket to the M server */

    struct sockaddr_in sa;

    sa.sin_family = AF_INET;
    sa.sin_addr = inAddr;
    sa.sin_port = htons(port);
    memset(&(sa.sin_zero), '\0', 8);

    if (connect(sd, (struct sockaddr *) &sa, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "ERROR: Could not connect socket!\n");
        return NULL;
    }

    cfm_conn_t *c = malloc(sizeof(cfm_conn_t));

    memset(c, 0, sizeof(cfm_conn_t));

    /* Remember some values */
    c->socket_fd = sd;
    c->host = strdup(host);
    c->port = port;

    /* Add connection to our list */
    CF_LIST_ADD(connHead, c);

    return c;
}

int
cfm_connection_sd_get(void *conn)
{
    return ((cfm_conn_t *) conn)->socket_fd;
}

void *
cfm_connection_find(int sd)
{
    cfm_conn_t *c;

    for (c = connHead; c != NULL; c = c->next) {
        if (c->socket_fd == sd) {
            return c;
        }
    }

    return NULL;
}

int
cfm_connection_close(void *c)
{
    cfm_conn_t *conn = (cfm_conn_t *) c;

    if (!connHead || !conn) {
        return 0;
    }

    CF_LIST_REMOVE(connHead, conn);

    for (int i = 0; i < CFM_MAX_PEERS; i++) {
        if (conn->peer[i] == NULL) {
            continue;
        }

        /* Make sure all peers are informed of the fact that 
         * the connection is going down... */

        conn->peer[i]->callback_close(conn, i, conn->peer[i]->userData);

        free(conn->peer[i]);
        conn->peer[i] = NULL;
    }

    /* Finally, shut down the socket */
    shutdown(conn->socket_fd, SHUT_RDWR);

    free(conn);

    return 1;
}

int
cfm_channel_open(void *cx,
                 const char *uiid,
                 char *name,
                 cfm_callback_open_t openCB,
                 cfm_callback_close_t closeCB,
                 cfm_callback_msg_t msgCB,
                 cfm_callback_error_t errorCB, void *userData)
{
    cfm_conn_t *conn = (cfm_conn_t *) cx;

    /* Valid parameters? */
    if (!connHead || !conn || !name) {
        return 0;
    }

    fprintf(stderr, "DEBUG: Open channel to %s (%s)..\n", name, uiid);

    if (callback_open) {
        /* Connection in progress... Cannot open yet */
        fprintf(stderr, "Error: Connection in progress. Please, try later..\n");
        return 0;
    }

    cfm_conn_t *c;

    for (c = connHead; c != NULL; c = c->next) {
        if (c == conn) {

            /* Send an open channel request to the other side */
            unsigned char newMsg[2048];

            int len = 4 + 36 + strlen(name) + 1;

            newMsg[0] = CFM_M_CHANNEL;
            newMsg[1] = len & 0xFF;
            newMsg[2] = ((len >> 8) & 0xFF);
            newMsg[3] = CF_M_CHANNEL_OPEN;
            memcpy(&newMsg[4], uiid, 36);
            memcpy(&newMsg[40], name, strlen(name) + 1);

            len = write(conn->socket_fd, newMsg, len);

            if (len <= 0) {
                /* Socket does not feel OK... */
                return 0;
            }

            /*  Remember callbacks */
            callback_open = openCB;
            callback_close = closeCB;
            callback_msg = msgCB;
            callback_error = errorCB;
            curr_user_data = userData;

            fprintf(stderr, "DEBUG: Sent CF_M_CHANNEL_OPEN !\n");

            return 1;
        }
    }

    fprintf(stderr, "Error: Connection not found!\n");
    return 0;
}

int
cfm_channel_close(void *c, int chan)
{
    cfm_conn_t *conn = (cfm_conn_t *) c;

    /* Valid parameters? */
    if (!connHead || !conn) {
        return 0;
    }

    if (channel_being_closed != -1) {
        fprintf(stderr,
                "Error: Could not close channel. Pending close going on...\n");
    }

    /* Remember channel that is being closed */
    channel_being_closed = chan;

    fprintf(stderr, "DEBUG: Close channel %d\n", chan);

    if (conn->peer[chan] == NULL) {
        fprintf(stderr, "Error: Could not close channel. It was not open!\n");
        return 0;
    }

    unsigned char newMsg[5];

    int len;

    newMsg[0] = CFM_M_CHANNEL;
    newMsg[1] = 5;
    newMsg[2] = 0;
    newMsg[3] = CF_M_CHANNEL_CLOSE;
    newMsg[4] = chan;

    len = write(conn->socket_fd, newMsg, 5);

    if (len <= 0) {
        /* Socket does not feel OK... */
        fprintf(stderr, "Error: Write error!\n");
        return 0;
    }

    fprintf(stderr, "DEBUG: Sent CF_M_CHANNEL_CLOSE !\n");

    return 1;
}

int
cfm_message_send(void *c, int chan, int len, unsigned char *msg)
{
    cfm_conn_t *conn = (cfm_conn_t *) c;
    unsigned char newMsg[3];
    int tot_len = 3 + len;

    newMsg[0] = chan;
    newMsg[1] = tot_len & 0xFF;
    newMsg[2] = (tot_len >> 8) & 0xFF;

    struct iovec io[2];

    io[0].iov_base = newMsg;
    io[0].iov_len = 3;

    io[1].iov_base = msg;
    io[1].iov_len = len;

    if (writev(conn->socket_fd, io, 2) < 0) {
        return 0;
    }
    else {
        return 1;
    }
}

static int
cfm_control_msg_handle(void *c)
{
    cfm_conn_t *conn = (cfm_conn_t *) c;
    int res;
    int slot = 0;

    switch (conn->msg_buff[1]) {
    case CF_M_CHANNEL_OPEN_OK:
        fprintf(stderr, "DEBUG: CF_M_CHANNEL_OPEN_OK.\n");

        /* Find open peer slot */
        for (slot = 0; slot < CFM_MAX_PEERS; slot++) {
            if (conn->peer[slot] == NULL) {
                conn->peer[slot] = malloc(sizeof(cfm_peer_t));

                /* Take this one! */
                conn->peer[slot]->callback_open = callback_open;
                conn->peer[slot]->callback_close = callback_close;
                conn->peer[slot]->callback_msg = callback_msg;
                conn->peer[slot]->callback_error = callback_error;
                conn->peer[slot]->channel = slot;
                conn->peer[slot]->userData = curr_user_data;

                /* No connection establishment is going on... Reset static data */

                res = callback_open(conn, slot, curr_user_data);

                callback_open = NULL;
                callback_close = NULL;
                callback_msg = NULL;
                callback_error = NULL;
                curr_user_data = NULL;

                return res;
            }
        }

        /* Could not open */
        res = callback_error(conn, "FAILED!", curr_user_data);

        callback_open = NULL;
        callback_close = NULL;
        callback_msg = NULL;
        callback_error = NULL;
        curr_user_data = NULL;

        return res;

    case CF_M_CHANNEL_OPEN_FAIL:
        fprintf(stderr, "DEBUG: CF_M_CHANNEL_OPEN_FAIL.\n");

        res = callback_error(conn, "FAILED!", curr_user_data);

        callback_open = NULL;
        callback_close = NULL;
        callback_msg = NULL;
        callback_error = NULL;
        curr_user_data = NULL;

        return res;

    case CF_M_CHANNEL_CLOSE_OK:
        res = conn->peer[channel_being_closed]->callback_close(conn,
                                                               channel_being_closed,
                                                               conn->
                                                               peer
                                                               [channel_being_closed]->
                                                               userData);

        /* Reset... */
        channel_being_closed = -1;

        fprintf(stderr, "DEBUG: CF_M_CHANNEL_CLOSE_OK.\n");
        return res;

    case CF_M_CHANNEL_CLOSE_FAIL:
        fprintf(stderr, "DEBUG: CF_M_CHANNEL_CLOSE_FAIL.\n");
        return conn->peer[conn->channel]->callback_close(conn,
                                                         conn->channel,
                                                         conn->peer[conn->
                                                                    channel]->
                                                         userData);
        break;
    case CF_M_CHANNEL_ORDER_UNKNOWN:
        fprintf(stderr, "DEBUG: CF_M_CHANNEL_ORDER_UNKNOWN.\n");
        break;
    default:
        fprintf(stderr, "Error: Unknown message.(%u)\n", conn->msg_buff[1]);
        break;
    }

    return 1;
}

int
cfm_sockets_handle(int fd)
{
    cfm_conn_t *conn;
    unsigned char buf[0xFFFF];

    /* Find socket in list */
    for (conn = connHead; conn != NULL; conn = conn->next) {
        if (conn->socket_fd != fd) {
            continue;
        }

        /* Found it! */
        int res;
        int handled = 0;

        res = read(fd, buf, 0xFFFF);

        if (res <= 0) {
            /* Socket has gone down! */
            return -1;
        }

        while (handled < res) {
            switch (conn->state) {
            case CFM_CONN_INIT:
                conn->channel = buf[handled++];
                conn->state = CFM_CONN_CHANNEL;

                fprintf(stderr, "DEBUG: M message: Channel (%d)\n",
                        conn->channel);

                break;
            case CFM_CONN_CHANNEL:
                conn->msg_len = buf[handled++];
                conn->state = CFM_CONN_LENGTH1;
                break;
            case CFM_CONN_LENGTH1:
                conn->msg_len = (buf[handled++] << 8) + conn->msg_len;
                conn->state = CFM_CONN_LENGTH2;

                fprintf(stderr, "DEBUG: M message: Length (%d)\n",
                        conn->msg_len);

                break;
            case CFM_CONN_LENGTH2:
                conn->msg_buff = malloc(conn->msg_len * sizeof(unsigned char));
                /* NULL terminate just to be safe */
                conn->msg_buff[0] = 0;
                conn->msg_pos = 0;
                conn->state = CFM_CONN_MSGBODY;

                break;
            case CFM_CONN_MSGBODY:
                conn->msg_buff[conn->msg_pos++] = buf[handled++];
                if ((conn->msg_pos + 3) == conn->msg_len) {
                    /* All bytes are in there! */
                    fprintf(stderr, "DEBUG: M message: All bytes there (%d)\n",
                            conn->msg_pos);

                    conn->state = CFM_CONN_MSGREADY;
                }

                break;
            default:
                fprintf(stderr, "Error in message handling!\n");
                break;
            }

            if (conn->state != CFM_CONN_MSGREADY) {
                continue;
            }

            /* Re-init... */
            conn->state = CFM_CONN_INIT;

            if (conn->channel == CFM_M_CHANNEL) {
                /* Got a message on the control channel. Handle it here... */

                return cfm_control_msg_handle(conn);
            }

            /* Message for the client... */
            return conn->peer[conn->channel]->callback_msg(conn,
                                                           conn->channel,
                                                           conn->msg_len,
                                                           conn->msg_buff,
                                                           conn->peer[conn->
                                                                      channel]->
                                                           userData);
        }
    }

    return 0;
}
