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
#ifndef COMPFRAME_UTIL_H
#define COMPFRAME_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */
#if 0
}
#endif
/*============================================================================*/
/* MACROS                                                                     */
/*============================================================================*/
/** Use this for adding an element to a list
    @note The list element must have a 'next' and 'prev' pointer.
*/
#define CF_LIST_ADD(head,elem)                  \
do {                                            \
  (elem)->prev = NULL;                          \
  (elem)->next = (head);                        \
                                                \
  if ((head)) {                                 \
    (head)->prev = (elem);                      \
  }                                             \
                                                \
  (head) = (elem);                              \
                                                \
} while (0)
/** Use this for removing an element from a list 
    @note The list element must have a 'next' and 'prev' pointer.
*/
#define CF_LIST_REMOVE(head,elem)               \
  do {                                          \
    if ((elem)->prev == NULL) {                 \
      /* First in list */                       \
      (head) = (elem)->next;                    \
                                                \
      if ((head)) {                             \
        (head)->prev = NULL;                    \
      }                                         \
    }                                           \
    else if ((elem)->next == NULL) {            \
      /* Last in list */                        \
      (elem)->prev->next = NULL;                \
    }                                           \
    else {                                      \
      /* In the middle somewhere */             \
      (elem)->prev->next = (elem)->next;        \
      (elem)->next->prev = (elem)->prev;        \
    }                                           \
  } while (0)
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* COMPFRAME_UTIL_H */
