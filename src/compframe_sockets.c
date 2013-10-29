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
#include "compframe_util.h"
#include <sys/poll.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

/*============================================================================*/
/* MACROS                                                                     */
/*============================================================================*/

/** Maximum sockets to poll */
#define CF_MAX_SD 1024

/*============================================================================*/
/* TYPES                                                                      */
/*============================================================================*/

/** Used for keeping track of registered sockets */
typedef struct cf_socket_t {
    /** Pointer to next  */
    struct cf_socket_t *next;
    /** Pointer to previous  */
    struct cf_socket_t *prev;
    /** Socket descriptor  */
    int sd;
    /** Socket callback  */
    cf_sock_callback_t fp;
    /** Component pointer */
    void *comp;
    /** User data */
    void *userData;
} cf_socket_t;

/*============================================================================*/
/* VARIABLES                                                                  */
/*============================================================================*/

/** List of registered sockets  */
static cf_socket_t *socketHead = NULL;

/** Number of registered sockets  */
static int numRegistered = 0;

/** Poll array used for polling  */
static struct pollfd pollFD[CF_MAX_SD];

/*============================================================================*/
/* FUNCTION DECLARATIONS                                                      */
/*============================================================================*/

/*============================================================================*/
/* FUNCTION DEFINITIONS                                                       */
/*============================================================================*/

int
cf_socket_register(void *comp, int sd, cf_sock_callback_t fp, void *userData)
{
    if ((numRegistered + 1) == CF_MAX_SD) {
        /* Maximum number reached */
        cf_error_log(__FILE__, __LINE__, "Max number of sockets reached!\n");
        return 0;
    }

    cf_socket_t *s = malloc(sizeof(cf_socket_t));

    s->sd = sd;
    s->comp = comp;
    s->userData = userData;
    s->fp = fp;

    /* Add it to list */
    CF_LIST_ADD(socketHead, s);

    /* Rebuild the polling array */
    for (s = socketHead; s != NULL; s = s->next) {
        pollFD[numRegistered].fd = s->sd;
        pollFD[numRegistered].events = POLLIN;
        numRegistered++;
    }

    return 1;
}

int
cf_socket_deregister(int sd)
{
    int found = 0;

    for (cf_socket_t * s = socketHead; s != NULL; s = s->next) {
        if (s->sd == sd) {
            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "Removed socket %d.\n", sd);
            CF_LIST_REMOVE(socketHead, s);
            /* Free used memory */
            free(s);
            found = 1;
            break;
        }
    }

    if (!found) {
        cf_error_log(__FILE__, __LINE__, "Socket not found (%d)!\n", sd);
        return 0;
    }

    numRegistered = 0;
    /* Rebuild the polling array */
    for (cf_socket_t * s = socketHead; s != NULL; s = s->next) {
        cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                     "Socket %d is in list.\n", s->sd);
        pollFD[numRegistered].fd = s->sd;
        pollFD[numRegistered].events = POLLIN;
        numRegistered++;
    }

    return 1;
}

int
cf_sockets_poll(void)
{
    int res;

    res = poll(pollFD, numRegistered, 10);

    if (res <= 0) {
        return -1;
    }

    for (int i = 0; i < numRegistered; i++) {
        if (pollFD[i].revents == POLLIN || pollFD[i].revents == POLLHUP) {
            for (cf_socket_t * s = socketHead; s != NULL; s = s->next) {
                if (s->sd == pollFD[i].fd) {
                    /* Found the one! Call its callback */
                    if (pollFD[i].revents == POLLIN) {
                        s->fp(s->comp, s->sd, s->userData,
                              CF_SOCKET_STUFF_TO_READ);
                    }
                    else {
                        s->fp(s->comp, s->sd, s->userData, CF_SOCKET_CLOSED);
                    }
                    break;
                }
            }
        }
    }

    return res;
}

/*** Utility functions */

int
cf_socket_connect(char *host, int port, int *sd)
{
    struct sockaddr_in peer;    /* Address structures */
    struct hostent *hp;         /* Host data */

    /* Reset address structure */
    bzero((char *) &peer, sizeof(peer));

    /* Get internet address from host name */
    hp = gethostbyname(host);

    if (hp == NULL) {
        return -1;
    }

    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    bcopy((char *) hp->h_addr, (char *) &peer.sin_addr, hp->h_length);

    /* Create a socket in the Internet domain of type stream */
    *sd = socket(AF_INET, SOCK_STREAM, 0);

    if (*sd == -1) {
        return -1;
    }

    return connect(*sd, (struct sockaddr *) &peer, sizeof(peer));
}

int
cf_socket_server(int port)
{
    struct sockaddr_in serv_addr;   /* Address structures */
    int result;
    int size = sizeof(serv_addr);
    int newSocket = -1;

    /* Reset address structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    /* Create a socket in the Internet domain of type stream */
    newSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (newSocket == -1) {
        return -1;
    }

    result = bind(newSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (result == -1) {
        return -1;
    }

    result =
        getsockname(newSocket, (struct sockaddr *) &serv_addr,
                    (socklen_t *) & size);

    if (result == -1) {
        return -1;
    }

    result = listen(newSocket, 5);
    if (result == -1) {
        return -1;
    }
    /* Successful. Return the socket */
    return newSocket;
}

int
cf_socket_write(int sd, unsigned char *buf, size_t n)
{
    int result = -1;

    do {
        result = write(sd, buf, n);
    } while (result == -1 && errno == EAGAIN);

    return result;
}

int
cf_socket_name_get(int sd, char **host, int *port)
{
    struct sockaddr_in inA;     /* Address structures */
    int addr_len = sizeof(inA); /* size of address    */
    char tmphostname[1024];     /* Array for hostname */
    int result;

    if (getsockname(sd, (struct sockaddr *) &inA, (socklen_t *) & addr_len) !=
        0) {
        host = NULL;
        return -1;
    }

    result = gethostname(tmphostname, sizeof(tmphostname));

    if (result == -1) {
        host = NULL;
        return -1;
    }

    *port = ntohs(inA.sin_port);
    *host = strdup(tmphostname);
    return 0;
}

int
cf_socket_port_get(int sd)
{
    struct sockaddr_in host;
    int addr_len = sizeof(host);

    if (getsockname(sd, (struct sockaddr *) &host, (socklen_t *) & addr_len) !=
        0) {
        return -1;
    }

    return ntohs(host.sin_port);
}

int
cf_socket_bytes_to_read(int sd)
{
    int bytes = 0;
    int result = -1;

    result = ioctl(sd, FIONREAD, &bytes);

    if (result) {
        return bytes;
    }

    return result;
}

int
cf_socket_reusable(int sd)
{
    int flag = 1;

    return setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &flag,
                      sizeof(flag));
}
