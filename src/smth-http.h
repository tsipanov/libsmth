/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-http.h : web transfer glue (public header)
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

#ifndef __SMTH_HTTP_H__
#define __SMTH_HTTP_H__

/**
 * \internal
 * \file   smth-http.h
 * \brief  Web transfer glue (public header).
 * \author Stefano Sanfilippo
 * \date   12th-13th June 2010 ~ 6 Dic 2010
 */

#include <smth-common-defs.h>
#include <smth-manifest-parser.h>

/** Everything is ok. */
#define FETCHER_SUCCESS                (0)
/** Could not set fd_set descriptors. */
#define FETCHER_FAILED_FDSET           (-26)
/** Connection to server has reached timeout latency. */
#define FETCHER_CONNECTION_TIMEOUT     (-27)
/** Could not multiplex fd_set with \c select(). */
#define FETCHER_NO_MULTIPLEX           (-28)
/** Could not initialize libcurl backend. */
#define FECTHER_FAILED_INIT		       (-29)
/** There was no memory enough to instantiate another handle. */
#define FECTHER_NO_MEMORY              (-30)
/** Could not add a Fragment handler to the global \c Fetcher::handle */
#define FECTHER_HANDLE_NOT_ADDED       (-31)
/** Could not destroy a \c Fetcher::handle */
#define FETCHER_HANDLE_NOT_CLEANED     (-32)
/** Could not properly initialise an handle */
#define FETCHER_HANDLE_NOT_INITIALISED (-33)
/** Could not transfer the file from the Network */
#define FETCHER_TRANFER_FAILED         (-34)
/** Could not destroy a \c Fetcher */
#define FETCHER_NOT_CLEANED            (-35)
/** Could not create a cache file for a chunk */
#define FETCHER_NO_FILE                (-36)
/** An appropriate url for \c Chunk retrieval was not specified */
#define FECTHER_NO_URL                 (-37)

/** Automatic quality setup */
#define FETCHER_QUALITY_AUTO           (0)

#if 0
/** \brief Holds metadata for fetched streams */
typedef struct
{
	/** A \c NULL terminated array pointing to the tracks indices
	 *  relative to each chunk
	 */
	count_t *track;
	/** The name of the temporary dir in which all files are stored */
	char *dirname;
} FetchedStream
#endif

char* SMTH_fetch(const char *url, Stream *stream, bitrate_t maxbitrate);
FILE* SMTH_fetchmanifest(const char *url, const char *params);

#endif /* __SMTH_HTTP_H__ */

/* vim: set ts=4 sw=4 tw=0: */
