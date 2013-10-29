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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compframe.h"
#include "compframe_i.h"
#include <dlfcn.h>              /* dlopen() */
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include "IScheduler.hh"
#include "IConfig.hh"
#include "CFRegistry.hh"

/*============================================================================*/
/* FUNCTION DECLARATIONS                                                      */
/*============================================================================*/

static void
cf_init_sofile(char *file);
static void
print_usage(void);
static void
print_version(void);

/*============================================================================*/
/* VARIABLES                                                                  */
/*============================================================================*/

static CFComponent* sCompObj = NULL;
static ISchedulerControl* sIface = NULL;
static CFComponent* cfgObj = NULL;
static char* cfgFile = NULL;

/*============================================================================*/
/* FUNCTION DEFINITIONS                                                       */
/*============================================================================*/

static void
cf_init_sofile(char *file)
{
    void *handle;
    char *error;

    /* Make sure we have a .so file  */
    int len = strlen(file);

    if (len < 4) {
        /* x.so is the shortest allowed  */
        return;
    }

    if (!(file[len - 3] == '.' && file[len - 2] == 's' && file[len - 1] == 'o')) {
        /* Not a .so file. */
        return;
    }

    /* Open the component library */
    handle = dlopen(file, RTLD_LAZY);

    if (!handle) {
        cf_error_log(__FILE__, __LINE__,
                     "Could not open %s! (Error=%s)\n", file, dlerror());

        return;
    }

    /* Get the symbol 'dlopen_this', and call it to let the compoent register
     * itself. */
    void (*dlopen_this) (void);

    dlerror();                  /* Clear any existing error. */
    *(void**)(&dlopen_this) = dlsym(handle, "dlopen_this");

    if ((error = dlerror()) != NULL) {
        return;
    }

    dlopen_this();
}

static void
print_usage(void)
{
    fprintf(stderr,
            "Usage: compframe [options]\n"
            "Options:\n"
            " -d <dirlist>       Use component directory list a-la LD_LIBRARY_PATH\n"
            " -f <file>          Use Configuration file 'file'\n"
            " -t <level>         Use debug trace (levels 0 to 3)\n"
            " -h, --help         Display this information.\n"
            " -v, --version      Display version information\n\n"
            "For bug reporting and suggestions, mail peter@torpman.se\n");
}

static void
print_version(void)
{
    fprintf(stderr,
            "compframe %s\n"
            "Copyright (C) 2007-2013 Peter R. Torpman (peter@torpman.se)\n"
            "This is free software.  \nYou may redistribute copies of it under "
            "the terms of the GNU General Public License \n"
            "<http://www.gnu.org/licenses/gpl.html>.\n"
            "There is NO WARRANTY, to the extent permitted by law.\n",
            CF_VERSION);
}

int
main(int argc, char **argv)
{

    /* Usage: compframe [-d <componentdir>] [-f <configuration>] */
    char *compDir = NULL;

    int i = 1;

    while (i < argc) {

        /* Component Directory  */
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            print_version();
            return 0;
        }
        else if (!strcmp(argv[i], "-d")) {
            cf_trace_log(__FILE__, __LINE__, CF_TRACE_INFO,
                         "Handling -d \n", argv[i + 1]);

            if (argv[i + 1] == NULL) {
                print_usage();
                return 1;
            }

            compDir = argv[i + 1];

            i += 2;
        }

        /* Configuration file  */
        else if (!strcmp(argv[i], "-f")) {
            cf_trace_log(__FILE__, __LINE__, CF_TRACE_INFO,
                         "Handling -c \n", argv[i + 1]);

            if (argv[i + 1] == NULL) {
                print_usage();
                return 1;
            }

            /* Parse configuration file */
            cfgFile = argv[i + 1];

            i += 2;
        }

        /* Trace level  */
        else if (!strcmp(argv[i], "-t")) {

            cf_trace_log(__FILE__, __LINE__, CF_TRACE_INFO,
                         "Handling -t \n", argv[i + 1]);

            if (argv[i + 1] == NULL) {
                print_usage();
                return 1;
            }

            int level;

            if (sscanf(argv[i + 1], "%d", &level) != 1) {
                cf_error_log(__FILE__, __LINE__,
                             "Bad trace level! (%s)\n", argv[i + 1]);
                return 1;
            }

            if (level > 3 || level < 0) {
                cf_error_log(__FILE__, __LINE__,
                             "Bad trace level! (%s)\n", argv[i + 1]);
                return 1;
            }

            cf_trace_level_set(level);

            i += 2;
        }
        else {
            cf_error_log(__FILE__, __LINE__, "Bad parameter! (%s)\n", argv[i]);
            print_usage();
            return 1;
        }
    }

    if (compDir == NULL) {
        /* Check if environment variable is set. */
        compDir = getenv("CF_COMP_DIR");
    }

    /* Component directory should now be in argv[i+1] */
    char *tmp = compDir;
    char *lasts;

    if (!compDir) {
        cf_error_log(__FILE__, __LINE__,
                     "No component directory specified! (CF_COMP_DIR)\n");
        return 1;
    }

    tmp = strtok_r(tmp, ":\n\t ", &lasts);

    while (tmp) {
        DIR *dir = opendir(tmp);

        if (!dir) {
            cf_error_log(__FILE__, __LINE__,
                         "Bad component directory! (%s)\n", tmp);
            return 1;
        }

        struct dirent *de = NULL;
        char full_path[1024];

        cf_trace_log(__FILE__, __LINE__, CF_TRACE_INFO,
                     "Looking for component libraries in - %s \n", tmp);

        while ((de = readdir(dir)) != NULL) {
            sprintf(full_path, "%s/%s", tmp, de->d_name);

			if (strstr(full_path, ".so")) {
				cf_trace_log(__FILE__, __LINE__, CF_TRACE_INFO,
							 " Init lib - %s \n", full_path);
				cf_init_sofile(full_path);
			}
        }

        tmp = strtok_r(NULL, ":\n\t", &lasts);
    }

    /* ---------------------------------------------------------------------- */
    /* Create system components and initialize services                       */
    /* ---------------------------------------------------------------------- */

    /* Create our scheduler */
    sCompObj = CFRegistry::instance()->createComp("S", "S");

    /* Create our command handler */
    CFRegistry::instance()->createComp("C", "C");

    /* Create our message Handler */
    CFRegistry::instance()->createComp("M", "M");

    /* Create our configurator */
    CFRegistry::instance()->createComp("Cfg", "Cfg");

    /* Get control interface of S */
    sIface = (ISchedulerControl*) 
        CFRegistry::instance()->getCompIface("S", "ISchedulerControl");

    /* If configuration file was specified, let Cfg handle it */
    if (cfgFile) {
        int res;

        cfgObj = CFRegistry::instance()->getCompObject("Cfg");
        IConfig* cfgIface = 
            (IConfig*) CFRegistry::instance()->getIface(cfgObj, "IConfig");

        res = cfgIface->parse(cfgFile);

        if (!res) {
            cf_error_log(__FILE__, __LINE__, "Configuration file error!\n");
            return res;
        }
    }

    /* Start scheduler loop (eternal loop) */
    sIface->loop();

    return 0;
}
