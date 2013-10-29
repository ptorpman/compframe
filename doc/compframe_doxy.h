/**
   @mainpage CompFrame 
   @author Peter R. Torpman (ptorpman at sourcefourge.net)
   @version 0.4.0
   @date   September 13, 2007
   
   @note CompFrame is written in C. The standard used is the GNU implementation of C99.

   @ref taab @n
   @ref backg @n
   @ref intro @n
   @ref class @n
   @ref iface @n
   @ref complib @n
   @ref compreg @n
   @ref ifacereg @n
   @ref cmdreg @n
   @ref cmdline @n
   @ref mcomp @n
   @ref mcomplib @n
   @ref scomp @n
   @ref ccomp @n
   @ref envvars @n
   @ref legal @n

   @ref ifaces @n

   @section taab 0 Terms and Abbreviations

   <p>
   <b>Instance</b> - An occurence of a component (class).
   </p>
   
   <p>
   <b>Interface</b> - A set of functions used to manipulate the state of
   a component, or, to tell the component to perform a specific task.
   </p>
   <p>
   <b>Class</b> - A set of attributes and functions to manipulate those attributes.
   </p>
   <p>
   <b>Component</b> - A part of the component framework, e.g a class, interface etc.
   </p>
   <p>
   <b>Component library</b> - A set of components grouped together, e.g in a shared
   object library file.
   </p>

   @section backg 1 Background

   <p>
   Having worked in several projects that made use of some component framework,
   I came to see that it would really be beneficial for the OpenSource community
   to have a component framework. A component framework that would initially contain
   my experiences and, hopefully, not contain the pitfalls that I run into in the
   past with other component frameworks. In short, a straight-forward, easy to use
   piece of software that would empower developers instead of restraining them.
   </p>

   <p>
   I also happen to think that component based softwares are the right way to
   construct softwares. Why?! Because if you do it right, you can keep functionality
   separated and specialized. Furthermore, the maintenance will be easier. Also, 
   functionality can be added easily in separate containers instead of patching 
   some obscurely and loosly ordered source code files.
   </p>

   <p>
   My idea with CompFrame is that it can be used, extended
   and improved over time, without the need for re-inventing
   the wheel (or paying lots of cash) every time a component
   based software is supposed to be assembled. Thus, saving
   money and giving the poor developer time to address
   what's really important (and interesting).
   </p>


   @section intro 2 Introduction

   @note In CompFrame the textual name of a component must be unique. And,
         in CompFrame the textual name of an interface must be unique. 
         
   @subsection class 2.1 Component Class

   <p>
   A component class is realized using structs. 
   All values needed by the component should be kept within this struct. 
   Do not use static variables in the source code files, because they will be
   open to all instances of the component.
   </p>
   <p>
   The example below
   depicts a component class with two attributes - one for the instance
   name and one for the interface implemented by the component 'iface_test2'.
   </p>
   
   @verbatim
   typedef struct TestComp 
   {
     char*         instance_name:
     iface_test2*  test_iface2;
   } TestComp;
   @endverbatim

   
   @subsection iface 2.2 Interface

   <p>
   A CompFrame interface needs two things - a unique textual name and a 
   unique C type. 
   </p>
        
   <p>
   A component that wants to implement an interface must implement the functions
   implicated by the function pointers declared in the interface type. For example,
   a component that would like to implement the interface in the example below,
   would have to supply an implementation for the 'printHelloWorld' function.
   </p>

   @verbatim
   #define TEST2_IFACE_NAME "TEST2"

   typedef struct iface_test2 
   {
     void (*printHelloWorld)(void);
   } iface_test2;            
   @endverbatim


   @subsection complib 2.3 Component Library

   <p> 
   A CompFrame component library is a shared object library 
   (so-file), that may contain the implementation of one or many components.
   </p>
   <p>
   The library must have the following function defined:
   @verbatim
   void dlopen_this(void)
   @endverbatim
   
   <p> The library file will upon opening be scanned for the 
   <b>dlopen_this() </b> function.
   If the function cannot be found, it is not a valid CompFrame component library.
   </p>
   
   @subsection compreg 2.4 Component Registration

   <p> 
   A CompFrame component must register itself into the 
   <b>Registry</b> of CompFrame before it can be used.
   </p>
   <p>
   Registration can be done in the <b>dlopen_this()</b> function.
   </p>

   @verbatim
   cf_component_register("COMPNAME", create_me, set_me_up);
   @endverbatim
        
   <p>
   In this example the <b>COMPNAME</b> component is registered.
   The <i>create_me()</i> function will be called when a user
   wants to create an instance of the component. <i>set_me_up()</i>
   will be called after the instance has been created, in order to let it 
   set itself up and get ready for business.
   </p>

   <p>
   The library file will upon opening be scanned for the <i>dlopen_this</i> function.
   If the function cannot be found, it is not a valid CompFrame component library.
   </p>


   @subsection ifacereg 2.5 Interface Registration

   <p> 
   A component can implement no or many interfaces. Any interfaces implemented
   by the component must be registered to the <b>Registry</b>.
   </p>
   
   @verbatim
   static iface_test2 iTest2;

   iTest2.printHelloWorld = myHelloWorld;

   CfIfaceToReg iface[] = {
     {TEST2_IFACE_NAME, &amp;iTest2},
     {NULL,NULL}
   }

   cf_interface_register(comp,iface);
   @endverbatim

   <p>
   In the example above, the component registers that it implements the 
     <i>iface_test2</i> interface. And, it does so by the 
     <i>myHelloWorld</i> function.
   </p>

   @subsection cmdreg 2.6 Command Registration

   <p> 
   Commands that can do all sorts of things, may be registered into 
   CompFrame and made available on the CompFrame command line interface (CLI).
   </p>
   <p>
   The <b>C</b> system component is the Command Handler of CompFrame, and it
   is in that component we need to register our commands. That is done in the
   following way:
   </p>  
   @verbatim 

     static int
     my_cmd(int argc,char** argv);

     void* cObj    = cf_component_get(CF_C_NAME);
     cfi_c* cIface = cf_interface_get(cObj, CF_C_IFACE_NAME);

     cIface->add(cObj, "create", create_cmd, "Usage: create <class> <name>");

    @endverbatim

    <p>
    In the example the command <i>create</i> is registered.
    The <i>create_cmd()</i> is the function that is to perform all
    the work. Also, a help string is provided.
    </p>



    @section cmdline 3 Command Line
    
    <p>
    The command line of CompFrame is currently like this:
    
    @verbatim
    compframe -d <componentdir> 
             [-c <configuration>] 
             [-t <level>]
    @endverbatim
    
    <p><b>-d</b> is used to point out the directory where the component 
    libraries are located.
    </p>
    <p>
    <b>-c</b> is used to point out the configuration file.
    </p>
    <p>
    <b>-t</b> is used to specify trace level (0-3)
      </p>
    
    @subsection cmd_create 3.1 create
    
    <p>
    This command is used to create instances of components.

    @verbatim
    CompFrame 0.4.0 (c)2007 Peter R. Torpman
    >> create TESTCOMP x1
    @endverbatim
      
    <p>
    An instance of TESTCOMP with the name 'x1' is created.
    </p>


    @subsection cmd_list 3.2 list

    <p>
    This command is used to list many things, such as registered 
    classes, implemented interfaces etc.
    </p>
      
    @verbatim
    CompFrame 0.4.0 (c)2007 Peter R. Torpman
    >> list -c 
    @endverbatim
      
    <p>
        The registered classes will be printed.
    </p>

    @verbatim
    CompFrame 0.4.0 (c)2007 Peter R. Torpman
    >> list -i x1 
    @endverbatim
      
    <p>
        Lists the implemented interfaces of 'x1'. 
    </p>

    @subsection cmd_conn 3.3 connect

    <p>
       This command is used to connect instances of components.
    </p>
      
    @verbatim
    CompFrame 0.4.0 (c)2007 Peter R. Torpman
    >> connect x1 x2 TEST 1 2 "hello"
    @endverbatim
      
    <p>
      Connects instance 'x1' to instance 'x2' on its TEST 
      interface, passing 1, 2 and "hello" as parameters.
    </p>

    @subsection cmd_help 3.4 help

    <p>
       Will print the help text supplied to the 'create' command.
    </p>
      
    @verbatim
    CompFrame 0.4.0 (c)2007 Peter R. Torpman
    >> help create
    @endverbatim
      
    <p>
      Shows the help text for the create command.
    </p>

   @section scomp 4 S - Scheduler
   
   <p>
     The thought of a "common time" for all components is nice. A time 
     that is independent of the wall clock time. A virtual time, that 
     might mean that one component gets to execute 300 real seconds per
     cycle, while an other just uses up 10 real seconds. But, they both
     think they executed 1 second, or whatever the cycle time is set to.
   </p>
   <p>
     <b>S</b>, the CompFrame scheduler, makes sure that all components 
     <i>that chooses to be scheduled</i>, are scheduled in a round-robin
     type of fashion.
   </p>
   <p>
      A component that wishes to be scheduled, must implement the 
      <i>cfi_s_client</i> interface.
      interface:
   </p>
   @verbatim
   static cfi_s_client sClient;
   
   static void
   set_me_up(void* comp) 
   {
     ::
     // Scheduler client interface 
     sClient.execute = s_client_execute;

     // Register our interfaces 
     CfIfaceToReg iface[] = {
       {CF_S_CLIENT_IFACE_NAME, &sClient},
       {NULL,NULL}
     };

     cf_interface_register(comp, iface);

     // Add ourself to the S scheduling loop 
     
     void* sObj = cf_component_get(CF_S_NAME);
     
     cfi_s_server* sIface = cf_interface_get(sObj, CF_S_SERVER_IFACE_NAME);
     sIface->add(sObj,this);

     ::
   }
   
   @endverbatim
   <p>
     After that registration, we will be called int the <i>s_client_execute</i>
     function, when <b>S</b> decides that it is our turn to execute.
     In that function, we can do something like this: 
   </p>
   @verbatim
   static void 
   s_client_execute(void* obj, uint32_t slice, int* sliceRemain )
   {
     // do some stuff

     // Inform S that we have used up all our time
     *sliceRemain = 0;
   }

   @endverbatim
   <p>
     If we do not set <i>sliceRemain</i> to zero, we will get the remainder
     of the slice, plus the ordinary time slice, when we are called upon again.
   </p>

   @section mcomp 5 M - Message Handler

   <p>
     <b>M</b>, is the Message Handler of CompFrame. It is used to establish
     connections to components that reside within CompFrame, from other 
     applications that are somewhere else.
   </p>
   @verbatim
  
   CompFrame process
   +----------------------------+ 
   |                            |
   |             1)     +---+   |  2)  +---------+
   |           +------->| M |<-------- | ExtProc |
   |           |        +---+   |      +---------+
   |     +-------+        |     |            ^
   |     | comp  |<-------+     |            |
   |     +-------+    3)        |            |
   |         |                  |   4)       |
   |         +-------------------------------+  
   |                            |
   |                            |
   |                            |
   |                            |
   |                            |
   |                            |
   +----------------------------+

   @endverbatim

   <p>
     In the image above, the basics are shown for how a connection is 
     established between a component within CompFrame and an external
     program. First, in 1) the component registers itself within <b>M</b>.
     When doing that, the component passes parameters to make itself
     unique - a UUID and a name. The name does not make this, so-called
     <i>message receiver</i> unique, it is the UUID and name together
     that makes it unique.
   </p>
   <p>
     Second, the external program connects to <b>M</b> by using the 
     mechanisms provided by the <i>M client library</i> (see below).
     The program specifies which message receiver and UUID it is 
     interested in communicating with. 
   </p>
   <p>
     <b>M</b> then looks up the registered message receiver and informs 
     it that an external program is interested in some communication.
   </p>
   <p>
     After all this, in 4) the communication is established between the
     two entities.
   </p>
   <p>
     <b>M</b> opens up a server socket during CompFrame initialization,
     that is used by the <i>M client library</i>. You can get a hold of
     this port by looking for something like this:
   </p>
   @verbatim
   *** INFO # M server started on hammer:35545
   CompFrame 0.4.0 (c)2007 Peter R. Torpman
   >>  
   @endverbatim
   <p>
     Or, you can get the port by entering the following command on the CLI:
   </p>
   @verbatim
   CompFrame 0.4.0 (c)2007 Peter R. Torpman
   >>  m -l
   M server located at hammer:35545
   @endverbatim
   
   <b>Connections and Channels</b>
   <p>
    <b>M</b> uses the concept of <i>connections</i> and <i>channels</i>. By 
    definition, a connection is a socket connection between a CompFrame 
    internal component and an external entity. And, a channel is a logical
    path on that connection. <b>M</b> allows for 255 channels for each 
    connection.
   </p>
   <p>
     <b>Protocols</b>
   </p>
   <p>
     A protocol, in the world of <b>M</b>, is just a byte stream where the
     bytes have been given a specific meaning. Nothing revolutionary at all.
     In a protocol, over a channel between A and B, the bytes could mean one
     thing in one direction, and something completely different in the other.
   </p>
   <p>
     For example, this could be a protocol description (yes, you need it for
     your own good):
   </p>
   @verbatim
   +------+--------+--------+----------+---+
   | WHAT | LEN LB | LEN HB | PAYLOAD  | 0 |
   +------+--------+--------+----------+---+
   WHAT    - 1 byte  - Content selector. (What is carried..) 
   LEN LB  - 1 byte  - Total length low byte of payload
   LEN HB  - 1 byte  - Total length high byte of payload
   PAYLOAD - n bytes - The payload of info.
   @endverbatim  

   <p>And, this could be another:
   </p>
   @verbatim
   +----------+---+
   | PAYLOAD  | 0 |
   +----------+---+
   PAYLOAD - n bytes (NULL terminated) - Good information.
   @endverbatim  

   <p>
     In short, you decide, the important thing is that sender and receiver
     agrees on what the different bytes of the protocol means.
   </p>

   <p>
     <b>Interface</b>
   </p>
   <p>
     A component that wants to provide a <i>message receiver</i> needs to implement
     <i>cfi_m_client</i> inteface.
   </p>
   @verbatim
   // A sample protocol identifier... 
   #define TEST_UUID "433e76d0-77d1-460d-9321-e2dc8dc8bd59"

   static void
   set_me_up(void* comp)
   { 
     ::
     // Set up implementation of M client interface  
     mifClient.connected    = m_cli_connected;
     mifClient.disconnected = m_cli_disconnected;
     mifClient.message      = m_cli_message;

     CfIfaceToReg iface[] = {
       {CF_M_CLIENT_IFACE_NAME, &mifClient},
       {NULL,NULL}
     };

     // Register interface
     cf_interface_register(comp, iface);

     // Store reference to M
     void* mObj = cf_component_get(CF_M_NAME);

     // Get server interface of M 
     cfi_m_server* m = cf_interface_get(mObj, CF_M_SERVER_IFACE_NAME);

     // Add our message receiver
     m->mr_add(mObj,this,TEST_UUID,"sample1",NULL);
     
     ::
   }

   @endverbatim

   <p>
     In this example, the component registers the <i>sample1</i> message 
     receiver,that implements the TEST_UUID protocol. After this, the 
     component is reachable from the outside world.
   </p>

   <p>
     When a connection is established, the component will be called  in
     the <i>m_cli_connected</i> function. 
   </p>
   @verbatim
   static int 
   m_cli_connected(void* comp, CfMConn* conn, uint8_t chan,void* userData)
   {
     TestComp* this = (TestComp*) comp;
   
     cf_info_log("Connected comp=%p conn=%p chan=%u \n",comp,conn,chan);
     
     // Disable our receiver if we only support one connection per
     // receiver 
     this->m->mr_disable(this->mObj,TEST_UUID,(char*)userData);
     
     return 1;
   }
   @endverbatim   

   <p>
     And, when a connection is broken, the component will be called  in
     the <i>m_cli_disconnected</i> function. 
   </p>
   @verbatim
   static int 
   m_cli_disconnected(void* comp, CfMConn* conn, uint8_t chan,void* userData)
   {
     TestComp* this = (TestComp*) comp;
   
     cf_info_log("Disconnected comp=%p conn=%p chan=%u \n",comp,conn, chan);

     // Re-enable our receiver if we only support one connection per  receiver 
     this->m->mr_enable(this->mObj,TEST_UUID,(char*)userData);
   
     return 1;
   }
   @endverbatim   

   <p>
     The component will receive messages in the <i>m_cli_message</i> function.
   </p>
   @verbatim
   static int  
   m_cli_message(void*          comp, 
                 CfMConn*       conn, 
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
   
     unsigned char newMsg[256];
   
     strcpy(newMsg,"Hello, yourself!\n");
   
     this->m->send(this->mObj,conn,chan,strlen(newMsg)+1,newMsg);
     
     return 1;
   }
   @endverbatim   

   @section mcomplib 5.1 M Client Library


   <p> Coming Soon! </p>


   @section ccomp 6 C - Command Handler

   <p> Coming Soon! </p>


   @section envvars 7 Environment Variables

   <b>CF_COMP_DIR</b> - Can be used to point out the directory where 
   components are stored.
   @verbatim
     setenv CF_COMP_DIR dir1:dir2:dir3
   @endverbatim

   @section legal 8 Legal 

   Permission is granted to copy, distribute and/or modify this document
   under the terms of the GNU Free Documentation License (GFDL), Version 1.1 
   or any later version published by the Free Software Foundation with no 
   Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.  

   This manual is the CompFrame manual distributed under the GFDL.  If you 
   want to distribute this manual separately from the collection, you can 
   do so by adding a copy of the license to the manual, as described in
   section 6 of the license.

   DOCUMENT AND MODIFIED VERSIONS OF THE DOCUMENT ARE PROVIDED
   UNDER  THE TERMS OF THE GNU FREE DOCUMENTATION LICENSE
   WITH THE FURTHER UNDERSTANDING THAT:
   
   DOCUMENT IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTY OF ANY KIND, EITHER
   EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES THAT THE 
   DOCUMENT OR MODIFIED VERSION OF THEDOCUMENT IS FREE OF DEFECTS MERCHANTABLE, 
   FIT FOR A PARTICULAR PURPOSE OR NON-INFRINGING. THE ENTIRE RISK AS TO THE 
   QUALITY, ACCURACY, AND PERFORMANCE OF THE DOCUMENT OR MODIFIED VERSION OF THE
   DOCUMENT IS WITH YOU. SHOULD ANY DOCUMENT OR MODIFIED VERSION PROVE DEFECTIVE 
   IN ANY RESPECT,YOU (NOT THE INITIAL WRITER, AUTHOR OR ANY CONTRIBUTOR) ASSUME 
   THE COST OF ANY NECESSARY SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER
   OF WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS LICENSE. NO USE OF ANY 
   DOCUMENT OR MODIFIED VERSION OF THE DOCUMENT IS AUTHORIZED HEREUNDER EXCEPT 
   UNDER THIS DISCLAIMER; AND UNDER NO CIRCUMSTANCES AND UNDER NO LEGAL THEORY, 
   WHETHER IN TORT (INCLUDING NEGLIGENCE), CONTRACT, OR OTHERWISE, SHALL THE 
   AUTHOR, INITIAL WRITER, ANY CONTRIBUTOR, OR ANY DISTRIBUTOR OF THE DOCUMENT 
   OR MODIFIED VERSION OF THE DOCUMENT, OR ANY SUPPLIER OF ANY OF SUCH PARTIES, 
   BE LIABLE TO ANY PERSON FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
   CONSEQUENTIAL DAMAGES OF ANY CHARACTER INCLUDING, WITHOUT LIMITATION, DAMAGES 
   FOR LOSS OF GOODWILL, WORK STOPPAGE, COMPUTER FAILURE OR MALFUNCTION, OR ANY 
   AND ALL OTHER DAMAGES OR LOSSES ARISING OUT OF OR RELATING TO USE OF THE
   DOCUMENT AND MODIFIED VERSIONS OF THE DOCUMENT, EVEN IF SUCH PARTY SHALL HAVE 
   BEEN INFORMED OF THE POSSIBILITY OF SUCH DAMAGES.
*/
