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
#ifndef IFACE_HH
#define IFACE_HH

/*============================================================================*/
/* INCLUDES                                                                   */
/*============================================================================*/
#include "IBase.hh"                             /* Base interface */


/** Interface textual name  */
#define TESTIFACE_ID  "1d6d2111-36fe-4661-93a6-1b3b647be110"

/** Interface type */
class ITest : public IBase
{
  public:
    /** Constructor */
    ITest() : IBase("ITest", TESTIFACE_ID) {}
    /** Destructor */
    virtual ~ITest() {};

    /** Samqple interface func */
    virtual void printHello(void) = 0;
    /** Sample interface func */
    virtual void print(char *str) = 0;
};

/** Interface textual name  */
#define TEST2IFACE_ID "1c1b04a2-b1b5-48ab-9aeb-4bb2d0bb8c18"

class ITest2 : public IBase
{
  public:
    /** Constructor */
    ITest2() : IBase("ITest2", TEST2IFACE_ID) {}
    /** Destructor */
    virtual ~ITest2() {};

    /** Sample interface func */
    virtual void printGoodbye(void) = 0;
};

#endif
