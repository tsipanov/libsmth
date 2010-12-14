/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-defs.c: private defs for smth.c
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

#ifndef __SMTH_DEFS_H__
#define __SMTH_DEFS_H__

/**
 * \internal
 * \file   smth-defs.h
 * \brief  private defs for smth.c
 * \author Stefano Sanfilippo
 * \date   11th July 2010
 */

/** \brief Holds the pseudofile handle for a given stream
 *
 *  This is redeclared as an opaque \c pointer in the public header file.
 */
typedef struct
{
	/** Manifest of the current Smooth Stream */
	Manifest manifest;
	/** Path to temporary dirs, one per stream. NULL terminated. */
	char **cachedirs;

} Handle;

typedef Handle SMTH_handle;

#endif /* __SMTH_DEFS_H__ */

/* vim: set ts=4 sw=4 tw=0: */
