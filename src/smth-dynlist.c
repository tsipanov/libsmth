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
 * \warning Allocation and deallocation of inserted items is up to the programmer.
 * \param item The item to be inserted into the list.
 * \param list The list in which to insert the \c item.
 * \return LIST_SUCCESS or LIST_NO_MEMORY, if there was no memory left. In this
 *         case, data is left untouched and the programmer may ignore this
 *         message, as appropriate.
 */
error_t addtolist(void *item, DynList *list)
{
	/* if too small, doubles the capiency. */
	if (list->index == list->slots)
	{	
		list->slots = list->slots? list->slots * 2: 3;
		void *tmp = realloc(list->list, list->slots * sizeof (list->list));
		if (!tmp) return LIST_NO_MEMORY;
		list->list = tmp;
	}

	list->list[list->index] = item;
	list->index++;

	return LIST_SUCCESS;
}

void preparelist(DynList *list)
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
 * \param list The list to be finalized.
 * \return LIST_SUCCESS or an appropriate error code.
 */
error_t finalizelist(DynList *list)
{
	void *tmp = realloc(list->list, list->index * sizeof (list->list)+1);
	if (!tmp) return LIST_NO_MEMORY;
	list->list = tmp;
	list->list[list->index] = NULL;
	return LIST_SUCCESS;
}

/**
 * \brief Destroys a dynamic list, appropriately.
 * \param list The list to be destroyed.
 */
void disposelist(DynList *list)
{	free(list->list);
}
