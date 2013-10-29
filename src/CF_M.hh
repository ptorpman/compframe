#ifndef CF_M_HH
#define CF_M_HH

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

#include "IMServer.hh"
#include "IMClient.hh"
#include "CFComponent.hh"
#include "compframe.h"
#include "compframe_sockets.h"
#include <map>
#include <vector>
using namespace std;

/** @addtogroup m M - Message Transport
 *  @{
 */

// Used for storing receivers
class MReceiver 
{
public:
    MReceiver() : mVisible(false), mName(""), mUserData(NULL), mClient(NULL) {}
   // Flag if visible
    bool mVisible;
    // Name
    string mName;
    // User data
    void* mUserData;
    // Pointer to client
    IMClient* mClient;
};

// Type used for storing an interface 
class MIface 
{
public:
    // Constructor
    MIface(const char *uuid, char *name) : mUuid(uuid), mName(name) { }
    // UUID of interface
    string mUuid;
    // Name
    string mName;
    // List of receivers for this interface
    vector<MReceiver*> mReceivers;

};

/** Used for keeping track of users of a specified connection */
class MPeer 
{
public:
    MPeer() : mSocket(-1), mChannel(-1),
              mLocalReceiver(NULL), mOpen(NULL),
              mClose(NULL), mError(NULL), mMsg(NULL),
              mUserData(NULL) {
    }

    /** Socket descriptor */
    int mSocket;
    /**  Channel used */
    int mChannel;
    /** Pointer to locally connected receiver  */
    MReceiver *mLocalReceiver;
    /** Will be called when connection is opened */
    cfm_callback_open_t mOpen;
    /** Will be called when connection is closed */
    cfm_callback_close_t mClose;
    /** Will be called when connection has a fault */
    cfm_callback_error_t mError;
    /** Will be called when message is received */
    cfm_callback_msg_t mMsg;
    /** Will be returned to user in callbacks */
    void *mUserData;
};

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

/** Used for M connections */
class MConn 
{
public:
    MConn() : mHost(NULL), mPort(-1),
              mSocket(-1), mNumUsed(0),
              mIsM(false), mChannel(-1),
              mMsgLen(0), mMsgBuff(NULL),
              mMsgPos(0), mState(CFM_CONN_INIT) {
    }

    /** Host name*/
    char* mHost;
    /** Port number */
    int mPort;
    /** Socket descriptor  */
    int mSocket;
    /** Number of used channels (Range 0..255) */
    uint8_t mNumUsed;
    /** Flag if remote connection is a M compoent  */
    bool mIsM;
    /** Current channel handled  */
    int mChannel;
    /** message length */
    int mMsgLen;
    /** Message buffer  */
    unsigned char *mMsgBuff;
    /** Message position */
    int mMsgPos;
    /** State of buffer  */
    cfm_conn_state_t mState;
    /** Peers on this connection  */
    MPeer* mPeer[CFM_MAX_PEERS];
};





class CF_M :
	public CFComponent,
    public IMServer
{
public:
    /** Constructor */
    CF_M(const char *inst_name);
    /** Destructor */
    virtual ~CF_M();

    // IMServer methods
    int addReceiver(CFComponent *comp, const char *uuid,
               char *name, void *userData);
    int enableReceiver(const char *uuid, char *name);
    int disableReceiver(const char *uuid, char *name);
	int rmReceiver(const char *uuid, char *name);
    int getServerPort() { return mPort; }
    char* searchByName(char *name);
    char *searchByIface(const char *uuid);
    int sendToReceiver(void *conn, uint8_t chan, int len,
             unsigned char *msg);


    // Set host name
    void setHostName(char* h) { mHostName.assign(h); }
    // Return host name
    string getHostName() { return mHostName; }

    /** Initiates the server socket of M */
    int initServerSocket();

    // Prints registered interfaces and receivers
    void printIfacesAndReceivers();
    // Handles stuff on the server socket
    int handleServerSocket(int sd, void *userData, cf_sock_event_t ev);

private:
    // Instance name
    string mName;
    // Host name
    string mHostName;
    // Port number
    int mPort;
    // Server socket
    int mSocket;
    // Map of interfaces
    map<string, MIface*> mInterfaces;
    // Map of connections
    map<int,MConn*> mConnections;

    // Returns a message receiver
    MReceiver* getReceiver(const char *uuid, char *name);
    // Add an interface
    MIface * addInterface(const char *uuid, char *name);
    // Returns an interface
    MIface* getInterface(const char* uuid);
    // Returns connection pointer for socket
    MConn* getConnection(int sd);
    // Handle remote message
    int handleRemoteMsg(MConn* conn, int sd);
    // Handle client message
    int handleClientMessage(MConn * conn, int sd);
    // Send message to client
    int passMessageToClient(MConn * conn);
    // Send response to peer
    int sendResponse(int sd, int order, int result, int response,
                     char *responseText);



};

/** @} */
#endif
