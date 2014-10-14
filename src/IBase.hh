#ifndef IBASE_HH
#define IBASE_HH

/* Copyright (c) 2007-2010  Peter R. Torpman (peter at torpman dot se)

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
#include <string>
using namespace std;

/** @addtogroup Interfaces
 *  These are the public interfaces of CompFrame
 *  @{
 */


/** Interface ID string */
#define IBASE_ID "00000000-0000-0000-0000-000000000000"

/** Base Class used to represent an interface */
class IBase
{
public:
  //! Constructor
  IBase(string name, string id) {
    mName = name;
    mId = id;
  }

  // Destructor
  ~IBase() { }

  /** Returns interface name */
  string getName() { return mName; }	
  /** Returns interface ID */
  string getId() { return mId; }

	
private:
  // String representation of interface (i.e interface name)
  string mName;
  // Interface ID
  string mId;
};

/** @} */

#endif
