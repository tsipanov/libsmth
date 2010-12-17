/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth-http-defs.h : web transfer glue (private header)
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

#ifndef __SMTH_HTTP_DEFS_H__
#define __SMTH_HTTP_DEFS_H__

/**
 * \internal
 * \file   smth-http-defs.h
 * \brief  Web transfer glue (private header).
 * \author Stefano Sanfilippo
 * \date   13th June 2010
 */

#include <curl/multi.h>
#include <smth-http.h>
#include <smth-manifest-parser.h>

/** The user agent string used by the fecther */
#define FETCHER_USERAGENT             "libsmth/0"
/** The template for the temp directory, one per \c Track */
#define FETCHER_DIRECTOTY_TEMPLATE    "/tmp/smth.XXXXXX"
/** The template directory for a manifest file. */
#define FETCHER_MANIFEST_TEMPLATE     "/tmp/smth-manifest.XXXXXX"

/** The number of simultaneous transfers allowed per \c Fetcher::handle */
#define FETCHER_MAX_TRANSFERS         10L
/** The maximum length for a filename */
#define FETCHER_MAX_FILENAME_LENGTH   1024
/** The maximum length for a chunk url */
#define FETCHER_MAX_URL_LENGTH        2048
/** The maximum length of a replace format specifier */
#define FETCHER_REPLACE_FORMAT_LENGTH 8

/** The placeholder for \c Chunk::time */
#define FETCHER_START_TIME_PLACEHOLDER "{start time}"
/** The placeholder for \c Track::bitrate */
#define FETCHER_BITRATE_PLACEHOLDER    "{bitrate}"
 
/** \brief Holds the Curl multi handle and fetcher metadata. */
typedef struct
{
	/** Handle to the active curl multi downloader. */
	CURLM *handle;
	/** Handle to the active \c Stream */
	Stream *stream;
	/** Maximal stream bitrate. 0 = unlimited */
	bitrate_t maxbitrate;
	/** Pointer to the next \c Chunk to handle */
	Chunk *nextchunk;
	/** Index of the last parsed \c Chunk */
	count_t chunk_no;
	/** Model from which to build the retrieve url */
	url_t *urlmodel;
	/** The local path to the cache directory */
	chardata *cachedir;
	/** The time the last download took */
	double downloadtime;

} Fetcher;

static error_t initfetcher(Fetcher *f);
static error_t disposefetcher(Fetcher *f);
static error_t resetfetcher(Fetcher *f);
static error_t execfetcher(Fetcher *f);
static error_t reinithandle(Fetcher *f);

static bitrate_t getbitrate(Fetcher *f);

static char *compileurl(Fetcher *f, char *buffer);
static char *replace(char *buffer, size_t size, const char *source,
	char *search, const char *format, void *replace);

#endif /* __SMTH_HTTP_DEFS__ */

/* vim: set ts=4 sw=4 tw=0: */
