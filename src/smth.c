/*
 * Copyright (C) 2010 Stefano Sanfilippo
 *
 * smth.c : public API
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
 * \file   smth.c
 * \brief  Main loop and public API
 * \author Stefano Sanfilippo
 * \date   14th December 2010
 */

#include <stdlib.h>

#include <smth-http.h>
#include <smth-manifest-parser.h>
#include <smth-common-defs.h>
#include <smth-dynlist.h>
#include <smth-defs.h>

/**

\mainpage libsmth internals documentation

\section alfa Welcome
This is to document thouroughly the internals of libsmth. Note that only
internal developers should read this. If you simply wish to link against
this lib, you may want to check out \c README.

\section beta Brief introduction to Smooth Streaming key concepts

\subsection anna Packet (Box) structure

Each \c Fragment contains a chunk of audio, video or text data, with a
metadata header. Each functional block of the fragment was named \c Box.
Each \c Box has a fixed structure:

\verbatim
~-----------+------+-----------------+------------+-------------~
~ BoxLenght | Name | [BoxLongLenght] | BoxFields  | BoxChildren ~
~    4B     |  4B  |  8B (optional)  | {variable} |  {variable} ~
~-----------+------+-----------------+------------+-------------~
\endverbatim

where:

\li \c Length     is the length of the box in bytes, encoded in network format.
                  If the value of the field is 1, the BoxLongLength
                  field must be present. Otherwise, we assert that it is not
                  present. Size includes all the fields, even BoxLenght itself.
\li \c Name       is a 4B non null-terminated string identifying the type of
                  the block. There are seven different types, as it will be
                  explained in the appropriate section.
\li \c LongLenght is the length of the Box in bytes, encoded in network format. 
                  This field is present only if the size of the \c Box is larger
                  than BoxLenght max size (32B).

\li \c Fields&Children are respectively the attributes and the content of the
                       \c Box and may vary accordingly to the role of the
                       respective \c Box.

The syntax of each \c Box is a strict subset of the syntax of the respective
Fragment Box defined in [\c ISOFF].

\subsection bebba Fragment Structure

As stated before, a Fragment Response is composed of various \c Boxes,

\verbatim
~------------+------------~
~   MoofBox  |  MdatBox   ~
~ {variable} | {variable} ~
~------------+------------~
\endverbatim

Each \c Box must contain certain sub-Boxes, and it may contain others.
Any block may contain one or more \c VendorExtensionUUIDBox, as described later.

\subsection cedda Manifest Response

According to the specifications, the Manifest must be a Well-Formed \c XML
Document subject to the following constraints:

\li The Document's \c XML Declaration's major version is \c 1.
\li The Document’s \c XML Declaration's minor version is \c 0.
\li The Document does not use a Document Type Definition (\c DTD).
\li The Document uses an encoding that is supported by the Client.
\li The XML Elements specified in this document do not use \c XML Namespaces.

*/

/**
 * \brief Opens an url for a Smooth Stream and registers a handle, which will
 *        be used to fetch data with subsequent calls to \c SMTH_read
 *
 * \param url    The url from which to retrieve the Smooth Stream
 * \param params Optional \c GET params to make the request (e.g. authentication
 *               codes, pages, etc...), as an urlencoded string.
 * \return
 */
Handle *SMTH_open(const char *url, const char *params)
{
	DynList cachelist;
	count_t i;
	error_t error;

	FILE *mfile = SMTH_fetchmanifest(url, params);

	if (!mfile)
	{
		SMTH_error(SMTH_NO_FILE_HANDLE, stderr);
		return NULL;
	}

	Handle *handle = malloc(sizeof (Handle));

	if (!handle)
	{
		SMTH_error(SMTH_NO_MEMORY, stderr);
		return NULL;
	}

	error = SMTH_parsemanifest(&handle->manifest, mfile);
	fclose(mfile);

	if (error)
	{
		SMTH_error(error, stderr);
		return NULL;
	}

	SMTH_preparelist(&cachelist);

	for (i = 0; handle->manifest.streams[i]; ++i) /* if possible find something more efficient */ // thread
	{
		StreamHandle *streamh = malloc(sizeof (StreamHandle));
		if (!streamh)
		{
			SMTH_error(SMTH_NO_MEMORY, stderr); //will leak
			return NULL;
		}

		streamh->cachedir = SMTH_fetch(url, handle->manifest.streams[i], 0);
		if (!streamh->cachedir)
		{
			SMTH_error(SMTH_NO_MEMORY, stderr); //will leak
			return NULL;
		}
			
		streamh->index = 0;
		streamh->parsed = false;

		if (!SMTH_addtolist(streamh, &cachelist))
		{
			SMTH_error(SMTH_NO_MEMORY, stderr);
			return NULL;
		}
	}

	handle->streamsno =  cachelist.index;

	if (!SMTH_finalizelist(&cachelist))
	{
		SMTH_error(SMTH_NO_MEMORY, stderr);
		return NULL;
	}

	handle->streams = (StreamHandle**)cachelist.list;

	return handle;
}

/**
 * \brief Reads at most size bytes from \c Stream \c stream into \c buffer
 *        using \c Handle \c h
 *
 * \return the number of bytes effectively read (0, in case of error)
 */
size_t SMTH_read(void *buffer, size_t size, int stream, Handle *handle)
{
	size_t writtens = 0;

	if (stream >= handle->streamsno) return 0;
	
	StreamHandle *s = handle->streams[stream];

	/* If this is over... */
	if (!s->remaining && s->parsed)
	{
		SMTH_disposefragment(&s->active);
		s->parsed = false;
		return 0;
	}

	if (!s->parsed)
	{
		/* If everything is over... */
		if (!handle->manifest.streams[stream]->chunks[s->index]) return 0;

		char filename[SMTH_MAX_FILENAME_LENGHT];
		snprintf(filename, SMTH_MAX_FILENAME_LENGHT, "%s/%lu", s->cachedir,
			handle->manifest.streams[stream]->chunks[s->index]->time);

		fcloseall(); /* XXX workaround... stupid CURLOPT_PRIVATE */

		FILE* input = fopen(filename, "rx"); /* FIXME ifdefined GLIBC */
		if (!input) return 0;

		if (SMTH_parsefragment(&s->active, input) != FRAGMENT_SUCCESS)
			return 0;

		fclose(input);
		s->remaining = s->active.size;
		s->cursor = s->active.data;
		s->parsed = true;
		s->index++;
	}

	writtens = size < s->remaining? size: s->remaining;
	memcpy(buffer, s->cursor, writtens);
	s->cursor = &s->cursor[writtens]; /* seek the stream */
	s->remaining -= writtens;

	return writtens;
}

/**
 * \brief Closes a SMTHh handle.
 *
 * \param handle The handle to be disposed of properly.
 */
void SMTH_close(Handle *handle)
{
	int i;

	SMTH_disposemanifest(&handle->manifest);

	for (i = 0; i < handle->streamsno; ++i)
	{
		free(handle->streams[i]->cachedir);
		free(handle->streams[i]);
	}

	free(handle->streams);
	free(handle);
}

/* vim: set ts=4 sw=4 tw=0: */
