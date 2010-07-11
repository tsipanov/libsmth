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

static error_t addtolist(void *item, DynList *list)
{
	/* if too small, doubles the capiency. */
	if (list->index == list->slots)
	{	
		list->slots = list->slots? list->slots * 2: 3;
		void *tmp = realloc(list->list, list->slots * sizeof (list->list));
		if (!tmp) return FRAGMENT_NO_MEMORY;
		list->list = tmp;
	}

	list->list[list->index] = item;
	list->index++;

	return FRAGMENT_SUCCESS;
}
