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
#include <string.h>
#include <stdarg.h>

#define __COMPILING_LIBSMTH__

#include <smth-http.h>
#include <smth-manifest-parser.h>
#include <smth-common-defs.h>
#include <smth-dynlist.h>
#include <smth-defs.h>
#include <smth.h>

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

\section gamma Quick overview

\subsection abba Public API

The main API exposed by libsmth is composed of five functions:
\li \c SMTH_open : Opens a stream with the given url and params
\li \c SMTH_getinfo: Get various metadata about the playing stream
\li \c SMTH_EOS: signals whether the end of the selected stream has been reached
\li \c SMTH_read : Performs a read on the pseudofile object returned by SMTH_open
\li \c SMTH_close : Closes the handle.
Note that SMTHh is \e not a \c FILE like object: it \e must be destroyed with a
call to SMTH_close.

\subsection dadda Example

Here is a tiny example of how the lib may be used to read a single chunk from
a given stream:

\include smth-test.c

Usage is self-explanatory. Should you need more power, you can rely on the
internal API, as documented in this paper.
*/

/**
 * \brief Opens an url for a Smooth Stream and registers a handle, which will
 *        be used to fetch data with subsequent calls to \c SMTH_read
 *
 * \param url    The url from which to retrieve the Smooth Stream
 * \param params Optional \c GET params to make the request (e.g. authentication
 *               codes, pages, etc...), as an urlencoded string.
 * \return       A pseudofile handle to perform subsequent calls, or \c NULL
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

	if (!handle->manifest.streams)
	{
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
		streamh->EOS = false;

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

	if (handle->manifest.islive)
	{
		handle->url = strdup(url);
		handle->params = params? strdup(params): NULL;
	}
	else
	{
		handle->url = NULL;
		handle->params = NULL;
	}

	return handle;
}

/**
 * \brief Reads at most size bytes from \c Stream \c stream into \c buffer
 *        using \c Handle \c h
 *
 * Read past the end of a fragment will return \c 0, and subsequent reads
 * will return the next chunk. The input stream is empty when two or more subsequent
 * calls return a \c 0 value.
 *
 * \return the number of bytes effectively read (0, in case of error or \c EOS)
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
		if (!handle->manifest.streams[stream]->chunks[s->index])
		{
			s->EOS = true;
			return 0;
		}

		char filename[SMTH_MAX_FILENAME_LENGHT];
		snprintf(filename, SMTH_MAX_FILENAME_LENGHT, "%s/%lu", s->cachedir,
			handle->manifest.streams[stream]->chunks[s->index]->time);

		fcloseall(); /* XXX workaround... stupid CURLOPT_PRIVATE */

		FILE* input = fopen(filename, "rx"); /* FIXME ifdefined GLIBC */
		unlink(filename); /* will be removed after fclose() */
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
		rmdir(handle->streams[i]->cachedir); /* will delete empty cache dirs */
		free(handle->streams[i]->cachedir);
		free(handle->streams[i]);
	}

	if (handle->manifest.islive)
	{
		free(handle->url);
		if (handle->params) free(handle->params);
	}

	free(handle->streams);
	free(handle);
}

/**
 * \brief Signals whether a stream is over.
 *
 * \warning Note that a live stream will never reach this state!
 *
 * \param handle The handle to be tested
 * \param stream The index of the stream to be tested
 * \return       Non zero if the stream is over.
 */
int SMTH_EOS(Handle *handle, count_t stream)
{
	if (stream >= handle->streamsno) return 1;
	return handle->streams[stream]->EOS;
}

/**
 * \brief Gets the specified setting value for currently parsed stream and
 *        stores it into \c destination. If an invalid setting is requested,
 *        \c dest is nulled.
 *

 * \warning Unless \c what is \c SMTH_STREAMS_NO, a track number (as a \c size_t)
 * must be passed before the pointer to the receiving variable. If not, the call
 * will not work (and may possibly lead to a crash).
 *
 * \param what The setting to retrieve
 * \param handle The handle to be tested
 * \param [stream] The index of the stream to be tested
 * \param [dest] Pointer to an appropriate memory area to store the requested setting
 */
void SMTH_getinfo(SMTH_setting what, Handle *handle, ...)
{
	va_list args;

	count_t stream; /* Only if applicable */

	bitrate_t *dest_bitrate;
	unit_t *dest_unit;
	size_t *dest_size;
	chardata **dest_char;
	hexdata **dest_hex;
	metric_t **dest_metrics;
	flags_t *dest_flags;
	SMTH_type *dest_type;

	va_start(args, stream);

	if (what == SMTH_STREAMS_NO)
	{
		dest_size = va_arg(args, size_t*);
		*dest_size = handle->streamsno;
		goto end;
	}

	stream = va_arg(args, count_t);
	if (stream >= handle->streamsno) goto end;
	Stream *astream = handle->manifest.streams[stream];
	Track *atrack = astream->tracks[0]; //FIXME FIRST select correct one

	switch (what)
	{
		case SMTH_BITRATE:
			dest_bitrate  = va_arg(args, bitrate_t*);
			*dest_bitrate = atrack->bitrate;
			break;

		case SMTH_AUDIO_PACKET_SIZE:
			dest_bitrate  = va_arg(args, bitrate_t*);
			*dest_bitrate = atrack->packetsize;
			break;

		case SMTH_AUDIO_SAMPLE_RATE:
			dest_bitrate  = va_arg(args, bitrate_t*);
			*dest_bitrate = atrack->samplerate;
			break;

		case SMTH_AUDIO_TAG:
			dest_flags  = va_arg(args, flags_t*);
			*dest_flags = atrack->audiotag;
			break;

		case SMTH_AUDIO_CHANNELS:
			dest_unit = va_arg(args, unit_t*);
			*dest_unit = atrack->channelsno;
			break;

		case SMTH_AUDIO_SAMPLE_SIZE:
			dest_unit = va_arg(args, unit_t*);
			*dest_unit = atrack->bitspersample;
			break;

		case SMTH_NAL_UNIT_LENGTH:
			dest_unit = va_arg(args, unit_t*);
			*dest_unit = atrack->nalunitlength;
			break;

		case SMTH_FOURCC:
			dest_char = va_arg(args, chardata**);
			*dest_char = strdup(atrack->fourcc);
			break;

		case SMTH_SUBTYPE:
			dest_char = va_arg(args, chardata**);
			*dest_char = strdup(astream->subtype);
			break;

		case SMTH_HEADER:
			dest_hex = va_arg(args, hexdata**);
			*dest_hex = strdup(atrack->header);  //FIXME unhexlify
			break;

		case SMTH_SCREENSIZE:
			dest_metrics = va_arg(args, metric_t**);
			memcpy(*dest_metrics, &atrack->maxsize, sizeof (ScreenMetrics));
			break;

		case SMTH_BESTSIZE:
			dest_metrics = va_arg(args, metric_t**);
			memcpy(*dest_metrics, &astream->bestsize, sizeof (ScreenMetrics));
			break;

		case SMTH_TYPE:
			dest_type = va_arg(args, StreamType*);
			*dest_type = astream->type;
			break;

		case SMTH_NAME:
			dest_char  = va_arg(args, chardata**);
			*dest_char = strdup(astream->name? astream->name: SMTH_UNNAMED_STREAM);
			break;

		case SMTH_ISLIVE:
			dest_size = va_arg(args, size_t*);
			*dest_size = handle->manifest.islive;
			break;

		default:
			dest_char = va_arg(args, chardata**);
			*dest_char = NULL;
			return;
	}

end:

	va_end(args);

#if 0
	Manifest: TODO
	/** A UUID that uniquely identifies the Content Protection System.
	 *  For instance: \c {9A04F079-9840-4286-AB92E65BE0885F95}
	 */
	uuid_t armorID;
	/** Opaque data that can use to enable playback for authorized users */
	EmbeddedData *armor;
#endif

}

/* vim: set ts=4 sw=4 tw=0: */
