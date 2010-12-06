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

/** The number of simultaneous transfers allowed per \c Fetcher::handle */
#define FETCHER_MAX_TRANSFERS      1000L
/** The user agent string used by the fecther */
#define FETCHER_USERAGENT          "libsmth/0"
/** The template for the temp directory, one per \c Track */
#define FETCHER_DIRECTOTY_TEMPLATE "/tmp/smth.XXXXXX"

/** \brief Holds the Curl multi handle and fetcher metadata. */
typedef struct
{
	/** Handle to the active curl multi downloader. */
	CURLM *handle;
} Fetcher;

static error_t initfetcher(Fetcher *f, Manifest *m);
static error_t disposefetcher(Fetcher *f);
static error_t resetfetcher(Fetcher *f);
static error_t execfetcher(Fetcher *f);

static bool reinithandle(Fetcher *f, CURL *eh);

#endif /* __SMTH_HTTP_DEFS__ */

/* vim: set ts=4 sw=4 tw=0: */
