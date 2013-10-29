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
#include "CF_C.hh"
#include "CFRegistry.hh"
#include "CFComponentLib.hh"
#include <assert.h>
#include "compframe_i.h"
#include <string.h>
#include <stdlib.h>
#include "IConnect.hh"
#include <unistd.h>

//=============================================================================
//                      G L O B A L  V A R I A B L E S
//=============================================================================

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
static int create_cmd(int argc, char **argv);
static int remove_cmd(int argc, char **argv);
static int list_cmd(int argc, char **argv);
static int help_cmd(int argc, char **argv);
static int connect_cmd(int argc, char **argv);

// The library container
static CFComponentLib theLib("C", create_me, set_me_up, destroy_me);



static void free_argv(int argc, char **argv);
static int
stdin_handle(void *comp, int sd, void *userData, cf_sock_event_t ev);

static void
free_argv(int argc, char **argv)
{
  for (int i = 0; i < argc; i++) {
    if (argv[i]) {
      free(argv[i]);
    }
  }
}


/** This function must reside in all component libraries.
    Here the component instance is 
*/
extern "C" void
dlopen_this(void)
{
  /* Use this function to get C into the Registry  */
  CFRegistry::instance()->registerLibrary(&theLib);
}

static CFComponent *
create_me(const char *inst_name)
{
  assert(inst_name != NULL);

  CF_CmdHandler* s = new CF_CmdHandler(inst_name);

  return s;
}

static int
destroy_me(CFComponent *comp)
{
  if (!comp) {
    return 1;
  }

  delete ((CF_CmdHandler*) comp);

  return 0;
}

/** Function called after S has been created */
static void
set_me_up(CFComponent *comp)
{
  CF_CmdHandler* cmdH =  (CF_CmdHandler*) comp;
	
  CFRegistry::instance()->registerIface(comp, (ICommand*) cmdH);
  CFRegistry::instance()->registerIface(comp, (ISchedulerClient*) cmdH);

  /* Add ourself to the S scheduling loop */

  ISchedulerServer* sIface = (ISchedulerServer*)
    CFRegistry::instance()->getCompIface("S", "ISchedulerServer");
                                                                
  sIface->add(cmdH);

  cmdH->add(comp, "list", list_cmd, "Usage: list [-c | -i <name>]\n");
  cmdH->add(comp,"create", create_cmd, "Usage: create <class> <instname>\n");
  cmdH->add(comp, "remove", remove_cmd, "Usage: remove <instname>\n");
  cmdH->add(comp,"connect",connect_cmd,
	    "Usage: connect <inst> <inst> IFACE <param1 param2 ...>\n");
  cmdH->add(comp, "help", help_cmd, "Usage: help <command>\n");
}




CF_CmdHandler::CF_CmdHandler(const char *inst_name) :
  CFComponent("C"),
  mName(inst_name),
  mSlice(0)
{
#ifdef WANT_TCL_COMMANDS
  mTclInterp = Tcl_CreateInterp();
#endif
}

CF_CmdHandler::~CF_CmdHandler()
{
}

int 
CF_CmdHandler::add(CFComponent* obj, const char* name, cfc_func_t func,
		   char* usage)
{
  Command_t* c = new Command_t;

  c->mName.assign(name);
  c->mFunc = func;
  c->mHelpString.assign(usage);
  c->mComp = obj;


  mCommands[c->mName] = c;

  return 0;
}

int 
CF_CmdHandler::remove(const char* name)
{
  map<string, Command_t*>::iterator i = mCommands.find(name);

  if (i == mCommands.end()) {
    return 1;
  }

  mCommands.erase(i);
  delete i->second;

  return 0;
}

int 
CF_CmdHandler::handle(char* cmdStr)
{
  char *lasts;
  char *tmp;
  int res;
  int argc = 0;
  char *argv[CF_MAX_COMMANDLEN];
  char *cmd;

  cmd = cmdStr;

  if (!cmd) {
    cf_error_log(__FILE__, __LINE__, "Bad command! 1\n");
    return 1;
  }


  if (strlen(cmd) > CF_MAX_COMMANDLEN) {
    cf_error_log(__FILE__, __LINE__, "Command too long!\n");
    return 0;
  }

  if (strlen(cmd) == 0) {
    return 0;
  }

#ifdef WANT_TCL_COMMANDS
  if (cmd[0] == '%') {
    res = Tcl_Eval(mTclInterp, cmd + 1);
    if (res != TCL_OK) {

      fprintf(stderr, "Tcl Error (%d): %s\n",
	      res, Tcl_GetStringResult(mTclInterp));
    }
    return 0;
  }
#endif

  /* Take copy */
  tmp = cmd;

  argc = 0;
  while (1) {
    if (argc == 0) {
      tmp = strtok_r(tmp, " \t\n", &lasts);
    }
    else {
      tmp = strtok_r(NULL, " \t\n", &lasts);
    }

    if (!tmp) {
      /* End of commands */
      break;
    }

    argv[argc++] = strdup(tmp);
  }

  /* Supported command? */
  map<string, Command_t*>::iterator i = mCommands.find(argv[0]);

  if (i == mCommands.end()) {
    cf_info_log("Invalid command.\n");
    return 1;
  }

  // Execute command
  res = i->second->mFunc(argc, argv);
  free_argv(argc, argv);
  return res;
}

void 
CF_CmdHandler::execute(uint32_t slice)
{
  static int showCopyStuff = 1;

  cf_trace_log(__FILE__, __LINE__, CF_TRACE_MASSIVE,
	       "%s is scheduled with slice %u\n",
	       CFRegistry::instance()->getCompName(this), slice);

  /* Do stuff that should be done once */
  if (showCopyStuff) {
    fprintf(stderr,
	    "CompFrame " CF_VERSION " (c)2007-2013 Peter R. Torpman\n>> ");
    showCopyStuff = 0;

    /* Register stdin for polling */
    cf_socket_register((CFComponent*)this, fileno(stdin), stdin_handle, this);
  }
}



static int
stdin_handle(void *comp, int sd, void *userData, cf_sock_event_t ev)
{
  CFComponent* c = (CFComponent*) comp;

  (void) userData;
  (void) ev;

  char readBuff[2048];
  ssize_t len;

  len = read(sd, readBuff, 2048);

  readBuff[len - 1] = 0;

  /* Want to quit? */
  if (!strcmp(readBuff, "q") || !strcmp(readBuff, "quit")) {
    exit(0);
  }

  ICommand* iface = (ICommand*)CFRegistry::instance()->getIface(c, "ICommand");

  iface->handle(readBuff);

  fprintf(stderr, ">> ");
  return 0;
}

static int
create_cmd(int argc, char **argv)
{
  if (argc != 3) {
    cf_error_log(__FILE__, __LINE__, "Usage: create <class> <instname>\n");
    return 0;
  }

  if (CFRegistry::instance()->createComp(argv[1], argv[2])) {
    return 1;
  }

  return 0;
}

static int
remove_cmd(int argc, char **argv)
{
  if (argc != 2) {
    cf_error_log(__FILE__, __LINE__, "Usage: remove <instname>\n");
    return 0;
  }

  // Do not allow removal of the core components.
  if (!strcmp(argv[1], "C") || !strcmp(argv[1], "Cfg") || !strcmp(argv[1], "S")) {
    cf_error_log(__FILE__, __LINE__, "Not allowed to remove core components\n");
    return 0;
  }



  if (CFRegistry::instance()->destroyComp(argv[1])) {
    return 1;
  }

  return 0;
}

static int
list_cmd(int argc, char **argv)
{
  (void) argv;                /* Avoid warnings */

  switch (argc) {

  case 1:
    /* No parameter */
    CFRegistry::instance()->listInstances();
    return 1;

  case 2:
    /* -c */
    if (strcmp(argv[1], "-c")) {
      cf_error_log(__FILE__, __LINE__, "Usage: list [-c | -i <name>]\n");
      return 0;
    }
    CFRegistry::instance()->listClasses();
    return 1;

  case 3:
    /* -i <name> */
    if (strcmp(argv[1], "-i")) {
      cf_error_log(__FILE__, __LINE__, "Usage: list [-c | -i <name>]\n");
      return 0;
    }
    CFRegistry::instance()->listInterfaces();
    return 1;

  default:
    cf_error_log(__FILE__, __LINE__, "Usage: list [-c | -i <name>]\n");
    return 0;
  }

  return 0;
}

static int
help_cmd(int argc, char **argv)
{
  CF_CmdHandler* c = 
    (CF_CmdHandler*)CFRegistry::instance()->getCompObject("C");

  if (argc != 2) {
    fprintf(stdout, "Available commands are:\n");
    c->listCommands();
    return 0;
  }

  c->printCommandHelp(argv[1]);

  return 0;
}

static int
connect_cmd(int argc, char **argv)
{
  if (argc < 4) {
    cf_error_log(__FILE__, __LINE__,
		 "Usage: connect <inst> <inst> IFACE <param1 param2 ...>\n");
    return 0;
  }
	
  /* Get first component */
  CFComponent *comp1 = CFRegistry::instance()->getCompObject(argv[1]);
	
  if (!comp1) {
    cf_error_log(__FILE__, __LINE__, "No such instance (%s)\n", argv[1]);
    return 0;
  }

  /* Get other component */
  CFComponent *comp2 = CFRegistry::instance()->getCompObject(argv[2]);

  if (!comp2) {
    cf_error_log(__FILE__, __LINE__, "No such instance (%s)\n", argv[2]);
    return 0;
  }

  /* Get cfi_connect on first component */
  IConnect *cfi = (IConnect*) CFRegistry::instance()->getIface(comp1, "IConnect");

  if (!cfi) {
    cf_error_log(__FILE__, __LINE__,
		 "Interface (%s) not implemented by (%s)\n",
		 ICONNECT_ID, argv[1]);
    return 0;
  }

  char **args = argv;

  if (argc == 4) {
    /* No parameters */
    return cfi->connect(comp2, argv[3], 0, NULL);
  }

  /* Get to the interface parameters */
  args += 4;

  return cfi->connect(comp2, argv[3], argc - 4, args);
}


void 
CF_CmdHandler::listCommands()
{
  map<string, Command_t*>::iterator ci = mCommands.begin();

  while (ci != mCommands.end()) {
    fprintf(stdout, "  %s\n", ci->first.c_str());
    ++ci;
  }
}

void 
CF_CmdHandler::printCommandHelp(char* cmd)
{
  map<string, Command_t*>::iterator ci = mCommands.find(cmd);

  if (ci == mCommands.end()) {
    cf_error_log(__FILE__, __LINE__,
		 "No help available for command (%s)\n", cmd);
    return;
  }

  fprintf(stdout, "  %s\n", ci->second->mHelpString.c_str());
}
