/* Copyright (c) 2007-2011  Peter R. Torpman (peter at torpman dot se)

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
#define _POSIX_SOURCE 1                           /* POSIX compliant */

//=============================================================================
//                              I N C L U D E S
//=============================================================================
#include "CF_M.hh"
#include "CFRegistry.hh"
#include "CFComponentLib.hh"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ICommand.hh"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
//=============================================================================
//                      G L O B A L  V A R I A B L E S
//=============================================================================
/** Number of bytes in a UUID */
#define CF_UUID_LEN 36

/** Usage string from 'm' command */
#define M_CMD_USAGE "Usage: m [-r | -l]\n"

//=============================================================================
//                        H E L P E R   C L A S S E S
//=============================================================================

//=============================================================================
//                        P U B L I C   M E T H O D S
//=============================================================================

//=============================================================================
//                       P R I V A T E   M E T H O D S
//=============================================================================
static CFComponent *create_me(const char *inst_name);
static void set_me_up(CFComponent *comp);
static int destroy_me(CFComponent *comp);
static int m_cmd(int argc, char **argv);
static int if_ok(const char *uuid);
static int handleServerSocketCallback(void *comp, int sd, void *userData, 
                                      cf_sock_event_t ev);

// The library container
static CFComponentLib theLib("M", create_me, set_me_up, destroy_me);


/** This function must reside in all component libraries.
    Here the component instance is 
*/
extern "C" void
dlopen_this(void)
{
    /* Use this function to get M into the Registry  */
	CFRegistry::instance()->registerLibrary(&theLib);
}


/** This function is used to create and initate a component. 
    @return Pointer to created instance or NULL 
*/
static CFComponent*
create_me(const char *inst_name)
{
    assert(inst_name != NULL);

    CF_M* m = new CF_M(inst_name);

    return  m;
}

static int
destroy_me(CFComponent* comp)
{
    if (!comp) {
        return 1;
    }

    delete ((CF_M*) comp);

    return 0;
}

/** Function called after M has been created */
static void
set_me_up(CFComponent* comp)
{
    CF_M* m = (CF_M*) comp;

    CFRegistry::instance()->registerIface(comp, (IMServer*) m);

    /* Get host name */
    char* hostName = getenv("HOST");

    if (!hostName) {
        hostName = getenv("HOSTNAME");
    }

    if (!hostName) {
        hostName = "localhost";
    }

    m->setHostName(hostName);


    /* Register our commands */
	ICommand* ifC = 
        (ICommand*) CFRegistry::instance()->getCompIface("C", "ICommand");

	ifC->add(comp, "m", m_cmd, M_CMD_USAGE);

    /* Start up the server socket */
    if (!m->initServerSocket()) {
        cf_error_log(__FILE__, __LINE__, "Could not start M server socket!\n");
        return;
    }
}

CF_M::CF_M(const char *inst_name) :
        CFComponent("M"),
        mName(inst_name)
{
}


CF_M::~CF_M()
{
}



/** Finds and returns a message receiver
    @param this Pointer to this context
    @param uuid Interface UUID
    @param name Name of receiver
    @return Pointer to receiver or NULL
*/
MReceiver *
CF_M::getReceiver(const char *uuid, char *name)
{
    string uid(uuid);

    map<string, MIface*>::iterator i =  mInterfaces.find(uuid);

    if (i == mInterfaces.end()) {
        cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                     "Found receiver name %s with IID %s \n",
                     name, uuid);
        return NULL;
    }

    vector<MReceiver*> v = i->second->mReceivers;
    vector<MReceiver*>::iterator ii = v.begin();

    string tmp(name);

    for (; ii != v.end(); ++ii) {
        if ((*ii)->mName == tmp) {
            return (*ii);
        }
    }

    return NULL;
}

/** Adds an interface to our list of interfaces */
MIface *
CF_M::addInterface(const char *uuid, char *name)
{
    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 0;
    }

    /* Already registered? */
    if (getInterface(uuid) != NULL) {
        /* Yes. */
        cf_error_log(__FILE__, __LINE__,
                     "Interface already registered (%s)\n", uuid);
        return NULL;
    }

    /* Create and add the interface */
    MIface* i = new MIface(uuid, name);

    mInterfaces[i->mUuid] = i;

    return i;
}

MIface* 
CF_M::getInterface(const char* uuid)
{
    map<string, MIface*>::iterator i = mInterfaces.find(uuid);
    
    if (i == mInterfaces.end()) {
        return NULL;
    }
    
    return i->second;   
}


/** Adds a receiver to our list of receivers */
int
CF_M::addReceiver(CFComponent *comp, const char *uuid, char *name, 
                  void *userData)
{
    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 0;
    }

    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return 0;
    }

    MIface *i = getInterface(uuid);

    if (i == NULL) {
        /* Not registered. So, let's register it! */
        i = addInterface(uuid, name);
    }

    /* Is it already there? */
    MReceiver* r = getReceiver(uuid, name);

    if (r) {
        cf_error_log(__FILE__, __LINE__,
                     "Receiver already registered! (%s %s)\n", uuid, name);
        return 0;
    }

    IMClient *iface = (IMClient*)
        CFRegistry::instance()->getIface(comp, "IMClient");

    if (!iface) {
        cf_error_log(__FILE__, __LINE__,
                     "Component (%s %s) does not implement the %s interface!\n",
                     uuid, name, "IMClient");
        return 0;
    }

    /* Create the new receiver */
    r = new MReceiver();
    r->mVisible = true;
    r->mName.assign(name);
    r->mClient = iface;
    r->mUserData = userData;

    // Add receiver to interface
    i->mReceivers.push_back(r);

    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                 "Added receiver %s %s...\n", uuid, name);

    return 1;
}

/** Marks a receiver as available for new connections  */
int
CF_M::enableReceiver(const char *uuid, char *name)
{
    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 1;
    }

    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return 1;
    }

    MReceiver *r = getReceiver(uuid, name);

    if (!r) {
        cf_error_log(__FILE__, __LINE__, "Receiver not found!\n");
        return 1;
    }

    r->mVisible = true;

    return 0;
}

/** Marks a receiver as unavailable for new connections  */
int
CF_M::disableReceiver(const char *uuid, char *name)
{
    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 1;
    }

    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return 1;
    }

    MReceiver *r = getReceiver(uuid, name);

    if (!r) {
        cf_error_log(__FILE__, __LINE__, "Receiver not found!\n");
        return 1;
    }

    r->mVisible = false;

    return 0;
}

/** Removes a receiver from our list */
int
CF_M::rmReceiver(const char *uuid, char *name)
{
    /* Sanity checks */
    if (!if_ok(uuid)) {
        return 1;
    }
    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return 1;
    }

    MIface *i = getInterface(uuid);
    MReceiver *r = getReceiver(uuid, name);

    if (!r) {
        cf_error_log(__FILE__, __LINE__,
                     "Invalid received! (%s %s)\n", uuid, name);
        return 1;
    }

    /** @todo We need to close down any open connections to this receiver */

    vector<MReceiver*>::iterator it = i->mReceivers.begin();
    
    for ( ; it != i->mReceivers.end(); ++i) {
        if (*it != r) {
            continue;
        }

        i->mReceivers.erase(it);
        delete r;

        if (i->mReceivers.size() == 0) {
            map<string, MIface*>::iterator it2 = mInterfaces.find(i->mName);

            mInterfaces.erase(it2);
            delete i;
        }

        return 0;
    }

    return 1;
}

/** Returns a list of receivers that begins with a certain name */
char *
CF_M::searchByName(char *name)
{
    if (!name) {
        cf_error_log(__FILE__, __LINE__, "Name missing!\n");
        return NULL;
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

    map<string, MIface*>::iterator ii =  mInterfaces.begin();

    for ( ; ii != mInterfaces.end(); ++ii) {
        posBefore = pos;

        vector<MReceiver*>::iterator ir = ii->second->mReceivers.begin();

        for ( ; ir != ii->second->mReceivers.end(); ++ir) {
            const char* recName = (*ir)->mName.c_str();

            if (!strncmp(recName, name, strlen(name))) {
                /* Found a winner */
                if (idAdded == 0) {
                    /* Add UUID first */
                    tmp = (char*) realloc(tmp, pos + CF_UUID_LEN + 1);

                    if (pos == 0) {
                        sprintf(tmp, "%s:", ii->first.c_str());
                    }
                    else {
                        sprintf(&tmp[pos], "%s%s:", tmp, ii->first.c_str());
                    }

                    pos += CF_UUID_LEN + 1;

                    idAdded = 1;
                }

                /* Add the name of the receiver */
                tmp = (char*) realloc(tmp, pos + strlen(recName) + 1);

                sprintf(&tmp[pos], "%s%s,", tmp, recName);

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
char *
CF_M::searchByIface(const char *uuid)
{
    /* FIXME! */
    (void) uuid;

    return NULL;
}

void
CF_M::printIfacesAndReceivers()
{
    fprintf(stdout,
            "------------------------------------------------------\n");
    fprintf(stdout, "- M: Registered Interfaces And Receivers\n");
    fprintf(stdout,
            "------------------------------------------------------\n");
    
    map<string, MIface*>::iterator ii =  mInterfaces.begin();

    int x = 0;
    for ( ; ii != mInterfaces.end(); ++ii) {
        fprintf(stdout, "%s\n\t", ii->first.c_str());

        vector<MReceiver*>::iterator ir = ii->second->mReceivers.begin();

        for ( ; ir != ii->second->mReceivers.end(); ++ir) {
            const char* recName = (*ir)->mName.c_str();
            fprintf(stdout, "%s (enabled=%d)  ", recName, (*ir)->mVisible);
            x++;
            
            if (x == 2) {
                fprintf(stdout, "\n\t");
                x = 0;
            }
        }
        fprintf(stdout, "\n");
    }
}

/** The main 'M' command.*/
static int
m_cmd(int argc, char **argv)
{
    CF_M *m = (CF_M*) CFRegistry::instance()->getCompObject("M");

    switch (argc) {
    case 2:
        /* -r */

        if (!strcmp(argv[1], "-r")) {
            m->printIfacesAndReceivers();
            return 0;
        }
        else if (!strcmp(argv[1], "-l")) {
            fprintf(stdout, "M server located at %s:%d\n",
                    m->getHostName().c_str(), m->getServerPort());
            return 0;
        }
        else {
            cf_error_log(__FILE__, __LINE__, M_CMD_USAGE);
            return 1;
        }

    default:
        cf_error_log(__FILE__, __LINE__, M_CMD_USAGE);
        return 1;
    }

    return 1;
}

/** Initiates the server socket of M */
int
CF_M::initServerSocket()
{
    int s;
    int res;

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

    mPort = ntohs(sa.sin_port);

    cf_info_log("M server started on %s:%d\n", mHostName.c_str(), mPort);

    /* Listen on socket */
    listen(s, 5);

    /* Remember the value */
    mSocket = s;

    /* Make socket non-blocking */
    fcntl(s, F_SETFL, O_NONBLOCK);

    /* Make sure our socket gets polled. We'll be called in 
     * m_server_socket_handle() if something happens */
    cf_socket_register(this, s, handleServerSocketCallback, this);

    return 1;
}

/** Return the connection for a specific socket descriptor
    @param sd Socket descriptor
    @return Pointer to connection or NULL
*/
MConn *
CF_M::getConnection(int sd)
{
    map<int,MConn*>::iterator i = mConnections.find(sd);
    
    if (i == mConnections.end()) {
        return NULL;
    }

    return i->second;
}

static int
handleServerSocketCallback(void *comp, int sd, void *userData, 
                           cf_sock_event_t ev)
{
    return ((CF_M*) comp)->handleServerSocket(sd, userData, ev);
}



/** Handle all our sockets
    @param comp Our context
    @param sd   Socket descriptor
    @param userData Own
*/
int
CF_M::handleServerSocket(int sd, void *userData, cf_sock_event_t ev)
{
    static unsigned char readBuff[CF_M_MAX_MESSAGE];

    (void) userData;            /* Avoid warnings */

    if (sd == mSocket) {
        if (ev != CF_SOCKET_STUFF_TO_READ) {
            /* Something bad has happened to the server socket. 
               Cannot continue */
            cf_error_log(__FILE__, __LINE__,
                         "M's server socket is dead! Panic!\n");
            return 1;
        }

        /* Time to accept some new connections */
        int remote;
        struct sockaddr_in remote_addr;
        socklen_t length = sizeof(struct sockaddr_in);

        remote = accept(mSocket,
                        (struct sockaddr *) &remote_addr, &length);

        if (remote < 0) {
            cf_error_log(__FILE__, __LINE__,
                         "Could not accept connection! errno=%d\n", errno);
            return 1;
        }

        /* Make it possible to reuse socket */
        int reuse = 1;

        setsockopt(remote, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse,
                   sizeof(reuse));

        /* Create structure for keeping this new connection */
        MConn* conn = new MConn();
        conn->mSocket = remote;

        mConnections[remote] = conn;

        /* a new connection has been established. Make sure we poll it. */
        cf_socket_register(this, remote, handleServerSocketCallback, this);

        return 0;
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

    MConn *conn = getConnection(sd);

    if (n == 0) {
        cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                     "Closed down socket %d.\n", sd);

        for (int i = 0; i < CFM_MAX_PEERS; i++) {
            if (conn->mPeer[i] == NULL) {
                continue;
            }

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "Informing peer/chan %d about disconnection...\n", i);
            MReceiver *rec = conn->mPeer[i]->mLocalReceiver;

            rec->mClient->disconnected(conn, i, rec->mUserData);
            delete conn->mPeer[i];
            conn->mPeer[i] = NULL;
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
        switch (conn->mState) {
        case CFM_CONN_INIT:
            conn->mChannel = readBuff[handled++];
            conn->mState = CFM_CONN_CHANNEL;

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "M message: Channel=%d (%x)\n",
                         conn->mChannel, readBuff[0]);

            break;
        case CFM_CONN_CHANNEL:
            conn->mMsgLen = readBuff[handled++];
            conn->mState = CFM_CONN_LENGTH1;

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "M message: Length=%d (1)\n", conn->mMsgLen);

            break;
        case CFM_CONN_LENGTH1:
            conn->mMsgLen = (readBuff[handled++] << 8) + conn->mMsgLen;
            conn->mState = CFM_CONN_LENGTH2;

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "M message: Length=%d (2)\n", conn->mMsgLen);

            break;
        case CFM_CONN_LENGTH2:
            /* Allocate message buffer */
            conn->mMsgBuff = (unsigned char*) malloc(conn->mMsgLen * 
                                                     sizeof(unsigned char));
            /* NULL terminate just to be safe */
            conn->mMsgBuff[0] = 0;
            conn->mMsgPos = 0;
            conn->mState = CFM_CONN_MSGBODY;

            break;
        case CFM_CONN_MSGBODY:
            conn->mMsgBuff[conn->mMsgPos++] = readBuff[handled++];

            if ((conn->mMsgPos + 3) == conn->mMsgLen) {
                /* All bytes are in there! */
                cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                             "M message: All bytes there\n");

                conn->mState = CFM_CONN_MSGREADY;
            }

            break;
        default:
            cf_error_log(__FILE__, __LINE__, "Error in message handling!\n");
            break;
        }

        /* Message ready to be handled? */
        if (conn->mState == CFM_CONN_MSGREADY) {
            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "M message ready.\n");

            if (conn->mChannel == CFM_M_CHANNEL) {
                cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                             "M control message.\n");

                if (conn->mIsM) {
                    /* A remote M component is connected */
                    handleRemoteMsg(conn, sd);
                }
                else {
                    /* A "normal" M client is connected here */
                    cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                                 "Handling M control message.\n");
                    handleClientMessage(conn, sd);
                }
            }
            else {
                cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                             "Passing message to client.\n");
                passMessageToClient(conn);
            }

            free(conn->mMsgBuff);
            conn->mMsgBuff = NULL;
            conn->mState = CFM_CONN_INIT;
        }

    }

    return 1;
}


/** Handles messages from remotely connected M servers
    @param this  This context
    @param conn  Connection to handle
    @param sd    Socket descriptor for response
    @return 1 if OK, 0 if failure
*/
int
CF_M::handleRemoteMsg(MConn* conn, int sd)
{
    /* FIXME: Add functionality for handling remote M server connections  */
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
int
CF_M::handleClientMessage(MConn * conn, int sd)
{
    unsigned char iid[CF_UUID_LEN + 1];
    unsigned char *msg = conn->mMsgBuff;
    char *name = NULL;
    MReceiver *rec = NULL;

    switch (msg[0]) {
    case CF_M_CHANNEL_OPEN:
    {
        memcpy(&iid[0], &msg[1], CF_UUID_LEN);

        iid[CF_UUID_LEN] = 0;

        name = (char *) &msg[37];

        rec = getReceiver((char *) iid, name);

        if (!rec) {
            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "CF_M_CHANNEL_OPEN to %s %s FAILED!\n", iid, name);

            sendResponse(sd,
                         CF_M_CHANNEL_OPEN,
                         CF_M_CHANNEL_OPEN_FAIL,
                         CF_M_COMP_NOT_FOUND, "Component not found!\n");
            return 0;
        }

        int newChan = -1;

        /* Component there.. get free channel */
        for (int i = 0; i < CFM_MAX_CHANNELS; i++) {
            if (conn->mPeer[i] == NULL) {
                /* Found empty slot */
                newChan = i;
                break;
            }
        }

        if (newChan == -1) {
            /* No new channel to be found */

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "CF_M_CHANNEL_OPEN to %s %s FAILED...\n", iid, name);

            sendResponse(sd,
                         CF_M_CHANNEL_OPEN,
                         CF_M_CHANNEL_OPEN_FAIL,
                         CF_M_OUT_OF_CHANNELS, "Out of channels!\n");
            return 0;
        }

        /* Create new peer and insert it. */
        MPeer *newPeer = new MPeer();

        conn->mPeer[newChan] = newPeer;
        newPeer->mChannel = newChan;
        newPeer->mSocket = sd;
        newPeer->mLocalReceiver = rec;

        /* Call open callback on receiver */
        int res = rec->mClient->connected(conn, newChan, rec->mUserData);

        if (res == 0) {
            /* Failed to open channel */

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "CF_M_CHANNEL_OPEN to %s %s FAILED...\n", iid, name);

            sendResponse(sd,
                         CF_M_CHANNEL_OPEN,
                         CF_M_CHANNEL_OPEN_FAIL,
                         CF_M_COULD_NOT_CONNECT, "Connection failed!\n");

            /* Remove the newly installed peer */
            conn->mPeer[newChan] = NULL;
            delete newPeer;
        }
        else {
            /* Managed to open channel */

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                         "CF_M_CHANNEL_OPEN to %s %s SUCCEEDED...\n", iid,
                         name);

            sendResponse(sd,
                         CF_M_CHANNEL_OPEN,
                         CF_M_CHANNEL_OPEN_OK,
                         CF_M_CHANNEL_OPEN_OK, "Channel open OK!!\n");
        }

        break;
    }
    case CF_M_CHANNEL_CLOSE:
    {
        int chan = msg[1];

        if (conn->mPeer[chan] == NULL) {
            sendResponse(sd,
                         CF_M_CHANNEL_CLOSE,
                         CF_M_CHANNEL_CLOSE_FAIL,
                         CF_M_COMP_NOT_FOUND,
                         "Component not found!!\n");

            return 0;
        }

        rec = (MReceiver *) conn->mPeer[chan]->mLocalReceiver;

        cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
                     "CF_M_CHANNEL_CLOSE to %s...\n", rec->mName.c_str());

        int res =
            rec->mClient->disconnected(conn, chan, rec->mUserData);

        if (res == 0) {
            /* Failed to close */
            sendResponse(sd,
                         CF_M_CHANNEL_CLOSE,
                         CF_M_CHANNEL_CLOSE_FAIL,
                         CF_M_CHANNEL_CLOSE_FAIL,
                         "Could not close!!\n");

        }
        else {
            sendResponse(sd,
                         CF_M_CHANNEL_CLOSE,
                         CF_M_CHANNEL_CLOSE_OK,
                         CF_M_CHANNEL_CLOSE_OK,
                         "Channel closed OK!!\n");
        }

        /* Remove the peer!  */
        delete conn->mPeer[chan];
        conn->mPeer[chan] = NULL;

        break;
    }
    default:
        /* Unknown message */
        sendResponse(sd,
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
int
CF_M::passMessageToClient(MConn * conn)
{
    MReceiver *rec = conn->mPeer[conn->mChannel]->mLocalReceiver;

    return rec->mClient->message(conn, conn->mChannel,
                                 conn->mMsgLen, conn->mMsgBuff, rec->mUserData);
}

/** Send a message to the other side */
int
CF_M::sendResponse(int sd, int order, int result, int response,
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

    int wSize = 0;

    if (len == 6) {
        wSize = write(sd, header, 5);
    }
    else {
        iov[0].iov_base = header;
        iov[0].iov_len = 6;
        iov[1].iov_base = responseText;
        iov[1].iov_len = strlen(responseText) + 1;

        wSize = writev(sd, iov, 2);
    }

    return 0;
}

int 
CF_M::sendToReceiver(void* c, uint8_t chan, int len, unsigned char *msg)
{
    MConn* conn = (MConn*) c;

    if (!conn) {
        cf_error_log(__FILE__, __LINE__, "Bad parameters!\n");
        return 0;
    }

    MPeer *p = conn->mPeer[chan];
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

    if (writev(p->mSocket, io, 2) < 0) {
        cf_error_log(__FILE__, __LINE__, "Failed to send message to client!\n");
        return 0;
    }

    return 1;

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
