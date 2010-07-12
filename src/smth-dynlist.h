/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-dynlist.h: Simple dynamic list implementation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __SMTH_DYNLIST_H__
#define __SMTH_DYNLIST_H__

/**
 * \internal
 * \file   smth-dynlist.h
 * \brief  simple dynamic list implementation
 * \author Stefano Sanfilippo
 * \date   11th July 2010
 */

#include <smth-common-defs.h>
#include <stdbool.h>

/**
 * \brief Holds dynamic list metadata and queue pointer.
 *
 * \warning To achieve universality, the list exploits a \c void pointer,
 * so that the compiler will not complain about the type of the assignement.
 * Triple check your assignements to \c DynList::list or you will introduce a bug!
 *
 * \sa addtolist()
 */
typedef struct
{   count_t slots; /**< The number of allocated slots.   */
	count_t index; /**< The number of filled slots.      */
	void **list;   /**< Pointer to the head of the list. */
} DynList;

bool addtolist(void *item, DynList *list);
bool finalizelist(DynList *list);
void preparelist(DynList *list);
void disposelist(DynList *list);

#endif /* __SMTH_DYNLIST_H__ */

/* vim: set ts=4 sw=4 tw=0: */
