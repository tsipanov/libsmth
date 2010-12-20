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

#include <smth-fragment-parser.h>
#include <smth-manifest-parser.h>

/** Could not open a blocking file handle for the Manifest */
#define SMTH_NO_FILE_HANDLE (-38)
/** No more memory to allocate data */
#define SMTH_NO_MEMORY      (-39)

/** The maximum lenght admittable for a file name */
#define SMTH_MAX_FILENAME_LENGHT 2048
/** The string returned if a \c Stream has no name */
#define SMTH_UNNAMED_STREAM      "(no name)"

typedef struct
{
	/** The active \c Fragment structure */
	Fragment active;
	/** Whether a new fragment needs to be parsed */
	bool parsed;
	/** Active \s Chunk index in \c Stream */
	tick_t index;
	/** Path to temporary dir. \c NULL terminated. */
	char *cachedir;
	/** Cursor position into the payload */
	char* cursor;
	/** Remaining payload bytes count */
	size_t remaining;
	/** Whether the read is over */
	bool EOS;
} StreamHandle;

/** \brief Holds the pseudofile handle for a given stream
 *
 *  This is redeclared as an opaque \c pointer in the public header file.
 */
typedef struct
{
	/** Manifest of the current Smooth Stream */
	Manifest manifest;
	/** Streams handlers */
	StreamHandle **streams;
	/** Number of active streams (for safety) */
	count_t streamsno;

} Handle;

#endif /* __SMTH_DEFS_H__ */

/* vim: set ts=4 sw=4 tw=0: */
