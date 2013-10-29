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
#include "CF_Cfg.hh"
#include "CFRegistry.hh"
#include "CFComponentLib.hh"
#include <assert.h>
#include "compframe_i.h"
#include <string.h>
#include <stdlib.h>
#include "ICommand.hh"

//=============================================================================
//                      G L O B A L  V A R I A B L E S
//=============================================================================

#define CFG_CMD_USAGE							\
  "\nUsage: config -i <inst> -n <varnum> -t <type> -v <value>\n"

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
static int cfg_cmd(int argc, char **argv);

// The library container
static CFComponentLib theLib("Cfg", create_me, set_me_up, destroy_me);

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

  CF_Cfg* s = new CF_Cfg(inst_name);

  return s;
}

static int
destroy_me(CFComponent *comp)
{
  if (!comp) {
    return 1;
  }

  delete ((CF_Cfg*) comp);

  return 0;
}

/** Function called after S has been created */
static void
set_me_up(CFComponent *comp)
{
  CF_Cfg* c = (CF_Cfg*) comp;
  CFRegistry::instance()->registerIface(comp, (IConfig*) c);

  // Register commands
  ICommand* ifC = (ICommand*)
    CFRegistry::instance()->getCompIface("C", "ICommand");

  ifC->add(comp, "config", cfg_cmd, CFG_CMD_USAGE);
}

CF_Cfg::CF_Cfg(const char *inst_name) :
  CFComponent("Cfg"),
  mName(inst_name)
{
}

CF_Cfg::~CF_Cfg()
{
}

int
CF_Cfg::parse(char* cfgFile)
{
  FILE *fp = fopen(cfgFile, "r");
  char line[1024];
  int length;
  int res;
  cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
	       "CFG config file: %s\n", cfgFile);

  if (!fp) {
    cf_error_log(__FILE__, __LINE__, "Could not open file (%s)!\n",
		 cfgFile);
    return 0;
  }

  ICommand* ic = (ICommand*)
    CFRegistry::instance()->getCompIface("C", "ICommand");

	
  length = 1024;

  while (getLine(fp, line, &length) != -1) {
    if (length != 0) {
      cf_trace_log(__FILE__, __LINE__, CF_TRACE_DEBUG,
		   "CFG config line: (%s)\n", line);

      if (!memcmp("create ", line, 7) ||
	  !memcmp("connect ", line, 8) ||
	  !memcmp("config ", line, 7)) {
	/* Let the C component handle the command... */
	res = ic->handle(line);

	if (!res) {
	  cf_error_log(__FILE__, __LINE__,
		       "Command failed (%s)!\n", line);
	  return 0;
	}
      }
      else {
	/* Unknown command */
	cf_error_log(__FILE__, __LINE__,
		     "Unknown command (%s)!\n", line);
	return 0;
      }
    }

    length = 1024;
  }

  return 1;
}


int
CF_Cfg::getLine(FILE * fp, char *buffer, int *length)
{
  char *line;
  char *tmp;
  char *lineCopy = (char*) malloc(*length);

  line = fgets(buffer, *length, fp);

  if (!line) {
    *length = 0;
    free(lineCopy);
    return -1;
  }

  if (line[0] == '#') {
    *length = 0;
    free(lineCopy);
    return 0;
  }

  strcpy(lineCopy, line);

  tmp = strtok(lineCopy, " \r\t\n");

  if (!tmp) {
    *length = 0;
    free(lineCopy);
    return 0;
  }

  *length = strlen(line);

  free(lineCopy);

  return 1;
}

/** The main 'Cfg' command.*/
static int
cfg_cmd(int argc, char **argv)
{
  if (argc != 4) {
    /* Wrong number of arguments */
    return 0;
  }

  // Get IConfigClient interface
  IConfigClient* ifc = (IConfigClient*)
    CFRegistry::instance()->getCompIface(argv[1], "IConfigClient");
	

  if (!ifc) {
    cf_error_log(__FILE__, __LINE__,
		 "Instance (%s) does not implement the IConfigClient"
		 " interface!\n", argv[1]);
    return 0;
  }

  return ifc->set(argv[2], argv[3]);
}

