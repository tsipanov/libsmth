/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-dynlist.c: Simple dynamic list implementation.
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

/**
 * \internal
 * \file   smth-dynlist.c
 * \brief  simple dynamic list implementation
 * \author Stefano Sanfilippo
 * \date   11th July 2010
 */

#include <stdlib.h>
#include <string.h>
#include <smth-dynlist.h>

/**
 * \brief Adds a \c item pointer to a DynList.
 * \warning    Allocation and deallocation of inserted items is up to the
 *             programmer.
 * \param item The item to be inserted into the list.
 * \param list The list in which to insert the \c item.
 * \return     \c true on success or \c false if there was no memory left.
 *             In this case, data is left untouched and the programmer may
 *             ignore this message, as appropriate.
 */
bool SMTH_addtolist(const void *item, DynList *list)
{
	/* if too small, doubles the capiency. */
	if (list->index == list->slots)
	{	
		list->slots = list->slots? list->slots * 2: 3;
		const void **tmp = realloc(list->list, list->slots * sizeof (list->list));
		if (!tmp) return false;
		list->list = tmp;
	}

	list->list[list->index] = item;
	list->index++;

	return true;
}

//FIXME macro...
void SMTH_preparelist(DynList *list)
{	memset(list, 0x0, sizeof(DynList));
}

/**
 * \brief Throws away empty slots and sigils the array.
 *
 * After a call to \c finalizelist(), \c DynList::list can be assigned to any
 * pointer or reused. This function is useful to save memory by eliminating
 * unused pointer slots, but it is not necessary to call it before any operation
 * on the list. The list of pointers is closed by a NULL pointer.
 *
 * \warning Finalizing an uninitialised array (i.e. not processed via
 *          \c preparelist() may have unexpected results.
 *
 * \param list The list to be finalized.
 * \return     \c true on success or \c false.
 */
bool SMTH_finalizelist(DynList *list)
{
	const void **tmp = realloc(list->list, (list->index + 1) * sizeof (void*));
	if (!tmp) return false;
	list->list = tmp;
	list->list[list->index] = NULL;

	return true;
}

/**
 * \brief Destroys a dynamic list, appropriately.
 * \param list The list to be destroyed.
 */
void SMTH_disposelist(DynList *list)
{	free(list->list);
}

/* vim: set ts=4 sw=4 tw=0: */
